#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <getopt.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <string>
using namespace std;

#include "Scheduler.h"
#include "SchedulerUI.h"
#include "SchedulerTalker.h"
#include "VehicleTalker.h"
#include <station_path.h>

//------------------------------------------------------------------------------
// Global constants

/// Although the system is intended to support several vehicles, not everything
/// has been implemented, so we will use vehicle 0 only for now.
const unsigned DEFAULT_VEHICLE_ID = 0;

/// Number of station
const int NUM_STATIONS = 3;

/// Number of seconds in minutes (for testing purpose)
const int NUM_SECONDS_IN_MIN = 1;

/// Number of seconds between two successive task info
const int NUM_SEC_TASK_INFO_SENT = 10;



//------------------------------------------------------------------------------
// Program options

int optVerbosityLevel = 0;
int optNoGUI = 0;
int optNoMobile = 0;
int optSimulateMobile = 0;
int optSimulateVehicle = 0;
int optSimulateTime = 0;
int optNoOP = 0;
string optHostName = "localhost";
int optMobilePort = 4440;
int optVehiclePort = 8888;


//------------------------------------------------------------------------------
// Global variables

// Whether the operator has quit (i.e. with Ctrl-C)
volatile sig_atomic_t gQuit = 0;

// Scheduler and talkers
Scheduler* gScheduler = NULL;
SchedulerTalker* gSchedulerTalker = NULL;
VehicleTalker* gVehicleTalker = NULL;
SchedulerUI* gSchedulerUI = NULL;

// Threads and mutex
pthread_t gOperatorThread, gTalkerThread, gVehicleThread;
pthread_mutex_t gSchedulerMutex;

// Current task
Task gCurrentTask;
time_t gPreviousTime;

/// The name of this program
const char * gProgramName;

// Log file
FILE* gLogFile = NULL;

StationList stationList;


//------------------------------------------------------------------------------
enum OperatorOption
{
    OPERATOR_OPTION_FIRST = 0,
    OPERATOR_ADD_TASK = 1,
    OPERATOR_REMOVE_TASK = 2,
    OPERATOR_VIEW_TASK_LIST = 3,
    OPERATOR_UPDATE = 4,
    OPERATOR_QUIT = 5,
    OPERATOR_OPTION_LAST = 6
};


//------------------------------------------------------------------------------
// Function declarations

// Error handling
#define MSG(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#define ERROR(fmt, ...) fprintf(stderr, "ERROR %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)


void sigintHandler(int sig)
{
    // if the user presses CTRL-C three or more times, and the program still
    // doesn't terminate, abort
    if (gQuit++ > 1) abort();
}

/// To be used with pthread_create --> creates the function pointer called by the thread.
template<class T, void(T::*mem_fn)()>
void* thunk(void* p)
{
    (static_cast<T*>(p)->*mem_fn)();
    return 0;
}


/// Prints usage information for this program to STREAM (typically stdout
/// or stderr), and exit the program with EXIT_CODE. Does not return.
void print_usage (FILE* stream, int exit_code);

void parseOptions(int argc, char **argv);
void openLogFile();
void launchThreads();
void terminateThreads();

/// Send task status, e.g. waiting time to mobile phones
void sendMobileTaskStatus();

/// Checks if there is a new task from the mobile customer. If there is, add it
/// to the scheduler.
void addMobileTask();

/// Update all the information
void update();

/// A thread that get command from operator
void *operatorLoop(void*);



//------------------------------------------------------------------------------
// Main function

int main(int argc, char **argv)
{
    /*
    // catch CTRL-C and exit cleanly, if possible
    signal(SIGINT, sigintHandler);
    // and SIGTERM, i.e. when someone types "kill <pid>" or the like
    signal(SIGTERM, sigintHandler);
    signal(SIGHUP, sigintHandler);
    */

    parseOptions(argc, argv);

    //cout << "Connecting to " << optHostName << ":" << optMobilePort << endl;

    gScheduler = new Scheduler(optVerbosityLevel);
    gCurrentTask = gScheduler->getVehicleCurrentTask(DEFAULT_VEHICLE_ID);

    openLogFile();

    launchThreads();

    time_t prevTimeTaskInfo = time(NULL);
    gPreviousTime = time(NULL);

    while(gQuit == 0)
    {
        if (!optNoOP && optNoGUI)
            pthread_mutex_lock(&gSchedulerMutex);
        else if (!optNoGUI)
            gSchedulerUI->updateConsole();

        if( ! optNoMobile )
            addMobileTask();

        // Update vehicle status, waiting time and send new task to vehicle if it has completes the previous task.
        update();

        // Report waiting time
        if (time(NULL) - prevTimeTaskInfo > NUM_SEC_TASK_INFO_SENT) {
            prevTimeTaskInfo = time(NULL);
            sendMobileTaskStatus();
        }

        if (!optNoOP && optNoGUI)
            pthread_mutex_unlock(&gSchedulerMutex);

        if (!optNoGUI && gSchedulerUI->getSchedulerStatus() == SchedulerUI::SCHEDULER_QUIT) {
            gQuit = 1;
        if (!optNoMobile && !optSimulateMobile)
            gSchedulerTalker->quit();
        if (!optSimulateVehicle)
            gVehicleTalker->quit();
        }
        usleep(100000);
    }

    fclose (gLogFile);

    terminateThreads();

    delete gScheduler;
} //main()


//------------------------------------------------------------------------------
// Helper functions

void print_usage (FILE* stream, int exit_code)
{
    fprintf( stream, "Usage:  %s [options]\n", gProgramName );
    fprintf( stream, "  --host hostname   Specify the IP address of the server "
            "for talking to mobile phone. The default is localhost.\n");
    fprintf( stream, "  --mport mport     Specify the port for talking to mobile "
            "phones (any number between 2000 and 65535). The default is 4440.\n");
    fprintf( stream, "  --vport vport     Specify the port for talking to the "
            "vehicle (any number between 2000 and 65535). The default is 4444.\n");
    fprintf( stream, "  --verbose         See the status of the program at every step.\n");
    fprintf( stream, "  --nogui           Run in plain text mode.\n");
    fprintf( stream, "  --nomobile        Do not receive request from mobile phones.\n");
    fprintf( stream, "  --simmobile       Simulate mobile phone users.\n");
    fprintf( stream, "  --simvehicle      Simulate vehicles' status.\n");
    fprintf( stream, "  --simtime         Simulate remaining time of current task.\n");
    fprintf( stream, "  --noop            Do not interact with operator when --nogui option is given.\n");
    fprintf( stream, "  --help            Display this message.\n");
    exit(exit_code);
}


void parseOptions(int argc, char **argv)
{
    int ch;
    const char* const short_options = "hp:H:";

    /* An array describing valid long options. */
    static struct option long_options[] =
    {
        // first: long option (--option) string
        // second: 0 = no_argument, 1 = required_argument, 2 = optional_argument
        // third: if pointer, set variable to value of fourth argument
        //        if NULL, getopt_long returns fourth argument
        {"verbose",     no_argument,       &optVerbosityLevel,        1},
        {"nogui",       no_argument,       &optNoGUI,                  1},
        {"nomobile",    no_argument,       &optNoMobile,               1},
        {"simmobile",   no_argument,       &optSimulateMobile,        1},
        {"simvehicle",  no_argument,       &optSimulateVehicle,       1},
        {"simtime",     no_argument,       &optSimulateTime,          1},
        {"noop",        no_argument,       &optNoOP,                   1},
        {"host",        required_argument, 0,                       'H'},
        {"mport",       required_argument, 0,                       'm'},
        {"vport",       required_argument, 0,                       'v'},
        {"help",        no_argument,       0,                       'h'},
        {0,0,0,0}
    };

    /* Remember the name of the program, to incorporate in messages.
        The name is stored in argv[0]. */
    gProgramName = argv[0];

    // Loop through and process all of the command-line input options.
    while((ch = getopt_long(argc, argv, short_options, long_options, NULL)) != -1)
        //while((ch = getopt_long_only(argc, argv, "", long_options, &option_index)) != -1)
    {
        switch(ch)
        {
        case 'h':
            /* User has requested usage information. Print it to standard
            output, and exit with exit code zero (normal
            termination). */
            print_usage(stdout, 0);
            break;

        case '?':
            /* The user specified an invalid option. */
            /* Print usage information to standard error, and exit with exit
            code one (indicating abnormal termination). */
            print_usage(stderr, 1);
            break;

        case 'H':
            optHostName = optarg;
            break;

        case 'm':
            optMobilePort = atoi(optarg);
            break;

        case 'v':
            optVehiclePort = atoi(optarg);
            break;

        case -1: /* Done with options. */
            break;
        }
    }
}


void openLogFile()
{
    char timestr[64];
    time_t t = time(NULL);
    strftime(timestr, sizeof(timestr), "%F-%a-%H-%M", localtime(&t));

    ostringstream oss;
    oss  << "log_scheduler" << "." << timestr << ".log";
    string logFileName = oss.str();

    // if it exists already, append .1, .2, .3 ...
    string suffix = "";
    struct stat st;
    for (int i = 1; stat((logFileName + suffix).c_str(), &st) == 0; i++)
    {
        ostringstream tmp;
        tmp << '.' << i;
        suffix = tmp.str();
    }
    logFileName += suffix;
    gLogFile = fopen(logFileName.c_str(), "w");
}


void launchThreads()
{
    if (!optNoOP && optNoGUI)
    {
        pthread_mutex_init(&gSchedulerMutex, NULL);
        int ret = pthread_create(&gOperatorThread, NULL, &operatorLoop, NULL);
        if (ret != 0)
            ERROR("return code from pthread_create(): %d", ret);
    }
    else if (!optNoGUI)
    {
        gSchedulerUI = new SchedulerUI(*gScheduler);
        gSchedulerUI->initConsole();
        gSchedulerUI->updateConsole();
    }

    if (!optNoMobile && !optSimulateMobile)
    {
        gSchedulerTalker = new SchedulerTalker(optHostName, optMobilePort, optVerbosityLevel);
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        int ret = pthread_create(&gTalkerThread, &attr,
                                thunk<SchedulerTalker, &SchedulerTalker::runMobileReceiver>,
                                gSchedulerTalker);
        if (ret != 0)
        ERROR("return code from pthread_create(): %d", ret);
    }

    if (!optSimulateVehicle)
    {
        MSG("Waiting for connection from the vehicle.");
        if (!optNoGUI)
            gSchedulerUI->updateConsole();
        gVehicleTalker = new VehicleTalker(optVehiclePort, optVerbosityLevel);
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        int ret = pthread_create(&gVehicleThread, &attr,
                                thunk<VehicleTalker, &VehicleTalker::runVehicleReceiver>,
                                gVehicleTalker);
        MSG("Vehicle ready.");
    }
}


void terminateThreads()
{
    if (!optNoOP && optNoGUI) {
        pthread_join(gOperatorThread, 0);
        pthread_mutex_destroy(&gSchedulerMutex);
    }
    else if (!optNoGUI) {
        gSchedulerUI->finishConsole();
        delete gSchedulerUI;
    }
    if (!optNoMobile && !optSimulateMobile) {
        pthread_join(gTalkerThread, 0);
        delete gSchedulerTalker;
    }
    if (!optSimulateVehicle) {
        pthread_join(gVehicleThread, 0);
        delete gVehicleTalker;
    }
}


bool checkMobileTask()
{
    if (!optSimulateMobile) {
        return gSchedulerTalker->checkTask();
    }
    else {
        srand ( time(NULL) );
        if (rand() % 45 != 0)
            return false;
        else
            return true;
    }
}


Task getMobileTask()
{
    Task task;
    task.id = 0;
    if (optVerbosityLevel > 0)
        MSG("\nGetting task from mobile phone...");

    if (!optSimulateMobile)
    {
        gSchedulerTalker->recvTask(task.customerID, task.taskID, task.pickup, task.dropoff);
    }
    else
    {
        //srand ( time(NULL) );
        task.customerID = 1234;
        task.taskID = 1;
        task.pickup = stationList(rand() % stationList.size());
        do
            task.dropoff = stationList(rand() % stationList.size());
        while (task.dropoff == task.pickup);

    }

    MSG("Got task <%u,%u,%s,%s> from mobile phone",
        task.customerID, task.taskID, task.pickup.c_str(), task.dropoff.c_str());
    fprintf(gLogFile, "\n%d: Got task <%u,%u,%s,%s> from mobile phone",
            (int) time(NULL), task.customerID, task.taskID,
            task.pickup.c_str(), task.dropoff.c_str());

    return task;
}


/// Add new task to gScheduler and returns its id
unsigned addTask(Task task)
{
    if (optVerbosityLevel > 0 && optNoGUI)
        cout << endl;

    unsigned taskid = gScheduler->addTask(task.customerID, task.taskID,
                                          task.pickup, task.dropoff);

    fprintf (gLogFile, "\n%d: Added task %u:%u:%s:%s to the scheduler",
             (int) time(NULL), task.customerID, task.taskID,
             task.pickup.c_str(), task.dropoff.c_str());

    if (optVerbosityLevel > 0 && optNoGUI)
    {
        cout << "After adding task..." << endl;
        cout << "Task queue:" << endl;
        gScheduler->printTasks();
    }

    return taskid;
}


void addMobileTask()
{
    Task task;

    if( checkMobileTask() )
    {
        try
        {
            // Get a new task from mobile phone and add to the scheduler
            task = getMobileTask();
            unsigned taskid = addTask(task);
            task = gScheduler->getTask(taskid);

            if (!optSimulateMobile)
            {
                gSchedulerTalker->sendTaskStatus(task.customerID, task.taskID, task.twait, task.vehicleID);
                fprintf (gLogFile, "\n%d: Sending status: %d:%d:%d:%d to the server",
                        (int) time(NULL), task.customerID, task.taskID, task.twait, task.vehicleID);
            }

            /* NOTEBRICE: When pickup and dropoff are -1, it means remove the task.
            else if (ret >= 0 && task.pickup == -1 && task.dropoff == -1 && !optSimulateMobile) {
                gSchedulerTalker->sendTaskStatus(task.customerID, task.taskID, -1, TASK_REMOVED);
                fprintf (gLogFile, "\n%d: Sending status: %d:%d:%d:%d to the server",
                        (int) time(NULL), task.customerID, task.taskID, -1, TASK_REMOVED);
            }
            */
        }
        catch( StationDoesNotExistException & e )
        {
            ERROR("Task %d <%s,%s> from customer %d is invalid: station %s does not exist.", task.id,
                  task.pickup.c_str(), task.dropoff.c_str(), task.customerID, e.what());
        }
        catch( SchedulerException & e )
        {
            /* NOTEBRICE: is this necessary?
            if (!optNoMobile && !optSimulateMobile)
            {
                gSchedulerTalker->sendTaskStatus(task.customerID, task.taskID,
                                                -1, errorCode);
                fprintf (gLogFile, "\n%d: Sending status: %d:%d:%d:%d to the server",
                        (int) time(NULL), task.customerID, task.taskID, -1, errorCode);
            }
            */
            ERROR("Task %d <%s,%s> from customer %d is invalid: %s", task.id,
                  task.pickup.c_str(), task.dropoff.c_str(), task.customerID, e.what());
        }
    }
}


void sendMobileTaskStatus()
{
    if (!optNoMobile && !optSimulateMobile)
    {
        Scheduler::VehicleStatus vehStatus = gScheduler->getVehicleStatus(DEFAULT_VEHICLE_ID);
        list<Task> tasks = gScheduler->getVehicleRemainingTasks(DEFAULT_VEHICLE_ID);

        if( vehStatus == Scheduler::VEHICLE_ON_CALL )
        {
            gSchedulerTalker->sendTaskStatus(gCurrentTask.customerID, gCurrentTask.taskID,
                                             gCurrentTask.tpickup, gCurrentTask.vehicleID);
            fprintf (gLogFile, "\n%d: Sending status: %d:%d:%d:%d to the server",
                     (int) time(NULL), gCurrentTask.customerID, gCurrentTask.taskID,
                     gCurrentTask.tpickup, gCurrentTask.vehicleID);
        }

        for ( list<Task>::iterator it=tasks.begin(); it != tasks.end(); it++ )
        {
            gSchedulerTalker->sendTaskStatus(it->customerID, it->taskID, it->twait, it->vehicleID);
            fprintf (gLogFile, "\n%d: Sending status: %d:%d:%d:%d to the server",
                     (int) time(NULL), it->customerID, it->taskID, it->twait, it->vehicleID);
        }
    }
}


/// TODO: Get the status of the vehicle
VehicleInfo getVehicleInfo(unsigned vehicleID)
{
    if (optVerbosityLevel > 0 && optNoGUI)
        MSG("Getting vehicle information...");

    VehicleInfo info;
    info.vehicleID = vehicleID;
    time_t currentTime = time(NULL);
    time_t timeDiff = (currentTime - gPreviousTime)/NUM_SECONDS_IN_MIN;

    if (timeDiff > 0)
        gPreviousTime = currentTime;

    if (!optSimulateVehicle)
    {
        info = gVehicleTalker->getVehicleInfo();
        if (optSimulateTime)
        {
            if (info.status == Scheduler::VEHICLE_AVAILABLE)
            {
                info.tremain = 0;
            }
            else if (info.status == Scheduler::VEHICLE_ON_CALL)
            {
                info.tremain = gCurrentTask.tpickup - timeDiff;
                if (info.tremain < 0)
                    info.tremain = 0;
            }
            else if (info.status == Scheduler::VEHICLE_POB)
            {
                info.tremain = gCurrentTask.ttask - timeDiff;
                if (info.tremain < 0)
                    info.tremain = 0;
            }
            else if (info.status == Scheduler::VEHICLE_BUSY)
            {
                if (gCurrentTask.tpickup > 0) {
                    info.status = Scheduler::VEHICLE_ON_CALL;
                    info.tremain = gCurrentTask.tpickup - timeDiff;
                }
                else {
                    info.status = Scheduler::VEHICLE_POB;
                    info.tremain = gCurrentTask.ttask - timeDiff;
                }
                if (info.tremain < 0)
                    info.tremain = 0;
            }
        }
    }
    else // if(!optSimulateVehicle)
    {
        info.isNew = true;
        if (gCurrentTask.id < 0 || gCurrentTask.ttask <= 0)
        {
            info.status = Scheduler::VEHICLE_AVAILABLE;
            info.tremain = 0;
        }
        else if (gCurrentTask.tpickup > 0)
        {
            info.status = Scheduler::VEHICLE_ON_CALL;
            info.tremain = gCurrentTask.tpickup - timeDiff;
            if (info.tremain < 0)
                info.tremain = 0;
        }
        else
        {
            info.status = Scheduler::VEHICLE_POB;
            info.tremain = gCurrentTask.ttask - timeDiff;
            if (info.tremain < 0)
                info.tremain = 0;
        }
    }

    return info;
}


void sendTaskToVehicle(Task task)
{
    MSG("Sending task %d: <%s,%s> from customer %d to vehicle",
        task.id, task.pickup.c_str(), task.dropoff.c_str(), task.customerID);
    fprintf (gLogFile, "\n%d: Sending task %d: <%s,%s> from customer %d to vehicle",
             (int) time(NULL), task.id, task.pickup.c_str(), task.dropoff.c_str(), task.customerID);

    if (!optSimulateVehicle)
        gVehicleTalker->sendNewTask(task.customerID, task.pickup, task.dropoff);

    fprintf (gLogFile, "\n%d: Task sent", (int) time(NULL));
}


void update()
{
    bool vehicleIsAvailable = false;

    // Check the vehicle status
    if (optVerbosityLevel > 0 && optNoGUI)
        cout << endl;

    VehicleInfo vehInfo = getVehicleInfo(DEFAULT_VEHICLE_ID);
    Scheduler::VehicleStatus vehStatus = vehInfo.status;

    fprintf (gLogFile, "\n%d: got status: %d:%d:%d",
             (int) time(NULL), vehInfo.status, vehInfo.tremain, vehInfo.isNew);

    // Get the next task and send to the vehicle if the vehicle is available
    if (vehStatus == Scheduler::VEHICLE_AVAILABLE && (vehicleIsAvailable || vehInfo.isNew))
    {
        gScheduler->updateVehicleStatus(DEFAULT_VEHICLE_ID, vehStatus);
        if (gScheduler->hasPendingTasks(DEFAULT_VEHICLE_ID))
        {
            if (optVerbosityLevel > 0 && optNoGUI)
                cout << endl;

            gCurrentTask = gScheduler->getVehicleNextTask(DEFAULT_VEHICLE_ID);
            sendTaskToVehicle(gCurrentTask);
            vehicleIsAvailable = false;

            if (optVerbosityLevel > 0 && optNoGUI) {
                cout << endl;
                cout << "After task sent..." << endl;
                cout << "Task queue:" << endl;
                gScheduler->printTasks();
            }
        }
        else
        {
            gScheduler->updateTPickupCurrent(DEFAULT_VEHICLE_ID, 0);
            gScheduler->updateTTaskCurrent(DEFAULT_VEHICLE_ID, 0);
            vehicleIsAvailable = true;
        }
    }
    // Otherwise, update the remaining task time (POB) or pickup time (other status)
    else if (vehStatus != Scheduler::VEHICLE_AVAILABLE)
    {
        gScheduler->updateVehicleStatus(DEFAULT_VEHICLE_ID, vehStatus);
        gScheduler->updateTCurrent(DEFAULT_VEHICLE_ID, vehInfo.tremain);
        gCurrentTask = gScheduler->getVehicleCurrentTask(DEFAULT_VEHICLE_ID);
        fprintf (gLogFile, "\n%d: Updated vehicle status to %d", (int) time(NULL), vehStatus);

        if (optVerbosityLevel > 0 && optNoGUI) {
            cout << endl;
            cout << "Updated remaining time to " << vehInfo.tremain << endl;
        }
    }

    // Update waiting time
    if (optVerbosityLevel > 0 && optNoGUI)
        cout << endl;

    gScheduler->updateWaitTime(DEFAULT_VEHICLE_ID);

    if (optVerbosityLevel > 0 && optNoGUI)
    {
        cout << "After updating waiting time..." << endl;
        cout << "Task queue:" << endl;
        gScheduler->printTasks();
    }
}


// TODO: Add a mechanism to escape from the loop.
int getNumeric(int min=0, int max=INT_MAX)
{
    int val = -1;

    while ( true )
    {
        if( cin >>val )
        {
            if( val >= min && val <= max )
                break;
            else
                MSG("Please enter a value between %d and %d", min, max);
        }
        else
        {
            MSG("Please enter a numeric value.");
            cin.clear();
            cin.ignore(80, '\n');
        }
    }

    return val;
}


void printMenu()
{
    printf("Menu:\n");
    printf ("  (%d) Add task\n", OPERATOR_ADD_TASK);
    printf ("  (%d) Cancel task\n", OPERATOR_REMOVE_TASK);
    printf ("  (%d) View task list\n", OPERATOR_VIEW_TASK_LIST);
    printf ("  (%d) Update\n", OPERATOR_UPDATE);
    printf ("  (%d) Quit\n", OPERATOR_QUIT);
}


void *operatorLoop(void* arg)
{
    while (gQuit == 0)
    {
        printMenu();

        // Options for operator
        if (optNoGUI && !optNoOP)
        {
            OperatorOption opOption = (OperatorOption) getNumeric(OPERATOR_OPTION_FIRST+1, OPERATOR_OPTION_LAST-1);

            pthread_mutex_lock(&gSchedulerMutex);
            if (opOption == OPERATOR_ADD_TASK)
            {
                Task task;
                task.id = 0;
                stationList.print();
                task.pickup = stationList.prompt("  Enter the pick-up location: ");
                task.dropoff = stationList.prompt("  Enter the drop-off location: ");
                task.customerID = 0;
                task.taskID = 1;
                addTask(task);
            }
            else if (opOption == OPERATOR_REMOVE_TASK)
            {
                printf ("  Enter id of task to be removed: \n");
                gScheduler->removeTask((unsigned) getNumeric());

                if (optVerbosityLevel > 0 && optNoGUI) {
                    cout << "After removing task..." << endl;
                    cout << "Task queue:" << endl;
                    gScheduler->printTasks();
                }
            }
            else if (opOption == OPERATOR_VIEW_TASK_LIST)
            {
                cout << endl;
                gScheduler->printTasks();
            }
            else if (opOption == OPERATOR_QUIT)
            {
                gQuit = 1;
                break;
            }
            update();
            pthread_mutex_unlock(&gSchedulerMutex);
        }
    }
    cout << "Operator thread stops\n";
    return 0;
}
