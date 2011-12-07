#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <getopt.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <math.h>

#include <iostream>
#include <sstream>
#include <string>
using namespace std;

#include "Scheduler.h"
#include "SchedulerUI.h"
#include "DBTalker.h"

#include <golfcar_route_planner/station_path.h>


/** BUG list
 * Apparently the time of tasks (twait) is wrong. Ask Nok to check.
 */

/** TODO
 * Support more than one vehicle (low priority).
 */


//------------------------------------------------------------------------------
// Global constants

/// Although the system is intended to support several vehicles, not everything
/// has been implemented, so we will use vehicle 0 only for now.
const unsigned DEFAULT_VEHICLE_ID = 0;

/// Number of seconds between two successive task info
const int NUM_SEC_TASK_INFO_SENT = 1;



//------------------------------------------------------------------------------
// Program options

int optVerbosityLevel = 0;
int optNoGUI = 0;
int optSimulateMobile = 0;
int optSimulateVehicle = 0;
int optSimulateTime = 0;
int optNoOP = 0;
string optHostName = "localhost";
float optTimeFactor = 1; ///< control the speed of time in simulation mode
float optNewTaskProba = 0.001; ///< control the probability of a new task in optSimulateMobile mode



//------------------------------------------------------------------------------
// Global variables

// Whether the operator has quit (i.e. with Ctrl-C)
volatile sig_atomic_t gQuit = 0;

// Scheduler and talkers
Scheduler* gScheduler = NULL;
DBTalker* gDBTalker = NULL;
SchedulerUI* gSchedulerUI = NULL;

time_t gPreviousTime = 0;

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
// Macros declarations


#define LOG(fmt, ...) do { \
        fprintf(gLogFile, "%s#%d, time %u, " fmt "\n", \
                __FUNCTION__, __LINE__, (unsigned)time(NULL), ##__VA_ARGS__); \
        fflush(gLogFile); \
    } while(0)

#define MSG(fmt, ...) do { \
        if (optVerbosityLevel > 0) { \
            fprintf(stderr, "%s#%d: " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            fflush(stderr); \
            }\
    } while(0)

#define MSGLOG(fmt, ...) do { MSG(fmt,##__VA_ARGS__); LOG(fmt,##__VA_ARGS__); } while(0)

#define MSGLOGQUIET(fmt, ...) do { if(optNoGUI) MSG(fmt,##__VA_ARGS__); LOG(fmt,##__VA_ARGS__); } while(0)

#define ERROR(fmt, ...) do { \
        fprintf(stderr, "ERROR %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
        fflush(stderr); \
    } while(0)


//------------------------------------------------------------------------------
// Function declarations

/// Prints usage information for this program to STREAM (typically stdout
/// or stderr), and exit the program with EXIT_CODE. Does not return.
void print_usage (FILE* stream, int exit_code);

void parseOptions(int argc, char **argv);
void openLogFile();

/// Checks if there is a new task from the mobile customer. If there is, add it
/// to the scheduler.
void addMobileTask();

/// Update all the information
void update();

/// get commands from operator in text mode
void textUI();


void sigintHandler(int sig)
{
    //First time: raise the QUIT flag
    //second time: abort
    if( ++gQuit > 1 )
        abort();
}



//------------------------------------------------------------------------------
// Main function

int main(int argc, char **argv)
{
    int exit_code = 0;

    // catch CTRL-C and exit cleanly, if possible
    signal(SIGINT, sigintHandler);
    // and SIGTERM, i.e. when someone types "kill <pid>" or the like
    signal(SIGTERM, sigintHandler);
    signal(SIGHUP, sigintHandler);

    srand ( time(NULL) );
    parseOptions(argc, argv);

    openLogFile();
    time_t prevTimeTaskInfo = time(NULL);

    try
    {
        gScheduler = new Scheduler();
        gScheduler->setVerbosityLevel(optVerbosityLevel);
        gScheduler->setLogFile(gLogFile);

        string username = "fmauto", passwd = "smartfm", dbname = "fmauto";
        gDBTalker = new DBTalker(optHostName, username, passwd, dbname);
        gDBTalker->setLogFile(gLogFile);
        gDBTalker->setVerbosityLevel(optVerbosityLevel);

        if( optSimulateVehicle )
            gDBTalker->createVehicleEntry("golfcart1");

        if (!optNoGUI)
        {
            gSchedulerUI = new SchedulerUI(*gScheduler, *gDBTalker);
            gSchedulerUI->initConsole();
            gSchedulerUI->updateConsole();
        }

        while(gQuit == 0)
        {
            if( optNoGUI ) {
                if( ! optNoOP )
                    textUI();
            }
            else {
                gSchedulerUI->updateConsole();
            }

            addMobileTask();

            // Update vehicle status, waiting time and send new task to vehicle if
            // it has completes the previous task.
            update();

            // Report waiting time
            if (time(NULL) - prevTimeTaskInfo > NUM_SEC_TASK_INFO_SENT) {
                prevTimeTaskInfo = time(NULL);
                gDBTalker->update(gScheduler->vehicles);
            }

            if (!optNoGUI && gSchedulerUI->getSchedulerStatus() == SchedulerUI::SCHEDULER_QUIT)
                gQuit = 1;

            usleep( (unsigned)(0.1 *1e6) );

        } //while(gQuit == 0)

    } //try
    catch( StationDoesNotExistException & e ) {
        MSGLOG("ERROR: Caught StationDoesNotExistException: station %s does not exist.", e.what());
        exit_code = 1;
    }
    catch( SchedulerException & e ) {
        MSGLOG("ERROR: Caught SchedulerException: %s.", e.what());
        exit_code = 1;
    }
    catch( exception & e )
    {
        MSGLOG("ERROR: Caught exception: %s", e.what());
        exit_code = 1;
    }

    fclose (gLogFile);

    if (!optNoGUI)
    {
        gSchedulerUI->finishConsole();
        delete gSchedulerUI;
    }

    delete gScheduler;
    if( gDBTalker!=NULL )
        delete gDBTalker;

    return exit_code;
} //main()


//------------------------------------------------------------------------------
// Helper functions

void print_usage (FILE* stream, int exit_code)
{
    stringstream ss;
    ss <<"Usage:  " <<gProgramName <<" [options]" <<endl;
    ss <<"  --host hostname   Specify the IP address of the server. The default is localhost." <<endl;
    ss <<"  --verbose lvl     Set verbosity level to lvl." <<endl;
    ss <<"  --nogui           Run in plain text mode (as opposed to using the NCurse interface)." <<endl;
    ss <<"  --simmobile       Do not intend to communicate with mobile phones. Simulate mobile phone users instead (automatically generate random tasks)." <<endl;
    ss <<"  --taskproba p     Sets the probability of a new task (only used when simmobile is on, default value is " <<optNewTaskProba <<")." <<endl;
    ss <<"  --simvehicle      Simulate vehicles' status." <<endl;
    ss <<"  --timefactor tf   Control speed of time when simulating the vehicle (default is 1)." <<endl;
    ss <<"  --simtime         Simulate remaining time of current task." <<endl;
    ss <<"  --noop            No interaction with user. (daemon mode)." <<endl;
    ss <<"  --help            Display this message." <<endl;

    fprintf( stream, "%s", ss.str().c_str());

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
        {"nogui",       no_argument,       &optNoGUI,                 1},
        {"simmobile",   no_argument,       &optSimulateMobile,        1},
        {"simvehicle",  no_argument,       &optSimulateVehicle,       1},
        {"simtime",     no_argument,       &optSimulateTime,          1},
        {"noop",        no_argument,       &optNoOP,                  1},
        {"timefactor",  required_argument, 0,                       'F'},
        {"taskproba",   required_argument, 0,                       'P'},
        {"host",        required_argument, 0,                       'H'},
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

        case 'F':
            optTimeFactor = atof(optarg);
            break;

        case 'P':
            optNewTaskProba = atof(optarg);
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
    strftime(timestr, sizeof(timestr), "%F-%a-%H-%M-%S", localtime(&t));

    ostringstream oss;
    oss  << "log_scheduler" << "." << timestr <<".log";
    string logFileName = oss.str();

    gLogFile = fopen(logFileName.c_str(), "w");
}


/// Add new task to gScheduler and returns it
Task addTask(Task task)
{
    task = gScheduler->addTask(task);
    gDBTalker->update(gScheduler->vehicles);

    MSGLOG("Added task %u:%s:%s:%s to the scheduler", task.taskID,
        task.customerID.c_str(), task.pickup.c_str(), task.dropoff.c_str());
    MSGLOGQUIET("After adding task...\nTask queue:\n%s", gScheduler->toString().c_str());

    return task;
}

double random_(double min, double max)
{
    return (((double)rand())/RAND_MAX) * (max-min) + min;
}

double random_()
{
    return random_(0,1);
}

unsigned randu_(unsigned max)
{
    return (unsigned) random_(0,max);
}

/// Gets any new tasks from the server. Alternatively, if optSimulateMobile is
/// set, generates some random tasks.
vector<Task> getMobileTask()
{
    if( optSimulateMobile )
    {
        /* Adding some radom tasks to the database.
         * The proba of a random task is equal to optNewTaskProba multiplied by
         * optTimeFactor (to scale for fast simulations).
         * Several tasks can be issued. The probability of a new task is half
         * the probability of the previous task.
         * For each task, pickup and dropoff stations are picked at random.
         */
        unsigned i = 0;
        while( random_() < optNewTaskProba*optTimeFactor/pow(2,i++) )
        {
            unsigned s1 = randu_(stationList.size());
            unsigned s2;
            do
                s2 = randu_(stationList.size());
            while (s1==s2);
            gDBTalker->makeBooking("cust1", stationList(s1), stationList(s2));
        }
    }
    return gDBTalker->getRequestedBookings();
}


///TODO: Add a mechanism to report error to the mobile phone customer.
void addMobileTask()
{
    vector<Task> tasks = getMobileTask();
    if( tasks.empty() )
        return;

    for( unsigned i=0; i<tasks.size(); i++ )
    {
        Task & task = tasks[i];
        try
        {
            task = addTask( task );
        }
        catch( StationDoesNotExistException & e )
        {
            ERROR("Task %d <%s,%s> from customer %s is invalid: station %s does not exist.", task.taskID,
                    task.pickup.c_str(), task.dropoff.c_str(), task.customerID.c_str(), e.what());
        }
        catch( SchedulerException & e )
        {
            ERROR("Task %d <%s,%s> from customer %s is invalid: %s", task.taskID,
                    task.pickup.c_str(), task.dropoff.c_str(), task.customerID.c_str(), e.what());
        }
    }
}


pair<VehicleStatus, Duration> getVehicleInfo(unsigned vehicleID)
{
    pair<VehicleStatus, Duration> info;
    MSGLOGQUIET("Getting vehicle information...");

    time_t currentTime = time(NULL);
    if( gPreviousTime==0 ) gPreviousTime = time(NULL);
    time_t timeDiff = (currentTime - gPreviousTime);
    Task & curTask = gScheduler->getVehicleCurrentTask(vehicleID);

    if (timeDiff > 0)
        gPreviousTime = currentTime;

    if( optSimulateVehicle )
    {
        Duration dt = timeDiff * optTimeFactor;
        if ( curTask.ttask <= dt)
        {
            info.first = VEHICLE_AVAILABLE;
            info.second = 0;
        }
        else if (curTask.tpickup > dt)
        {
            info.first = VEHICLE_ON_CALL;
            info.second = curTask.tpickup - dt;
            if (info.second < dt)
                info.second = 0;
        }
        else
        {
            info.first = VEHICLE_POB;
            info.second = curTask.ttask - dt;
            if (info.second < dt)
                info.second = 0;
        }
    }
    else // if(optSimulateVehicle)
    {
        info = gDBTalker->getVehicleStatus(vehicleID);
        Duration dt = timeDiff;
        if (optSimulateTime)
        {
            if (info.first == VEHICLE_AVAILABLE)
            {
                info.second = 0;
            }
            else if (info.first == VEHICLE_ON_CALL)
            {
                info.second = curTask.tpickup - timeDiff;
                if (info.second < dt)
                    info.second = 0;
            }
            else if (info.first == VEHICLE_POB)
            {
                info.second = curTask.ttask - timeDiff;
                if (info.second < dt)
                    info.second = 0;
            }
        }
    }

    return info;
}

void update()
{
    if( gScheduler->getVehicleTasks(DEFAULT_VEHICLE_ID).empty() )
        return;

    pair<VehicleStatus, Duration> info = getVehicleInfo(DEFAULT_VEHICLE_ID);
    MSGLOGQUIET("Got status: %d:%u", info.first, info.second);

    // Get the next task and send to the vehicle if the vehicle is available
    if( info.first == VEHICLE_AVAILABLE )
    {
        gScheduler->updateVehicleStatus(DEFAULT_VEHICLE_ID, VEHICLE_AVAILABLE);
        gDBTalker->update(gScheduler->vehicles);

        if( gScheduler->hasPendingTasks(DEFAULT_VEHICLE_ID) )
        {
            gScheduler->vehicleSwitchToNextTask(DEFAULT_VEHICLE_ID);
            if( optNoGUI )
                MSGLOGQUIET("\nAfter task sent...\nTask queue:\n%s", gScheduler->toString().c_str());
        }
        else
        {
            gScheduler->getVehicleTasks(DEFAULT_VEHICLE_ID).clear();
            gPreviousTime = 0;
        }
        gDBTalker->update(gScheduler->vehicles);
    }
    // Otherwise, update the remaining task time (POB) or pickup time (other status)
    else
    {
        gScheduler->updateVehicleStatus(DEFAULT_VEHICLE_ID, info.first);
        gScheduler->updateTCurrent(DEFAULT_VEHICLE_ID, info.second);
        MSGLOGQUIET("Updated vehicle status to %d and remaining time to %u", info.first, info.second);
    }

    if( info.first != VEHICLE_AVAILABLE )
        gScheduler->updateWaitTime(DEFAULT_VEHICLE_ID);

    MSGLOGQUIET("After updating waiting time...\nTask queue:\n%s", gScheduler->toString().c_str());
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
                cout <<"Please enter a value between " <<min <<"and " <<max <<".\n";
        }
        else
        {
            cout <<"Please enter a numeric value." <<endl;
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
    printf ("Your choice: ");
}


void textUI()
{
    printMenu();
    OperatorOption opOption = (OperatorOption) getNumeric(OPERATOR_OPTION_FIRST+1, OPERATOR_OPTION_LAST-1);

    if (opOption == OPERATOR_ADD_TASK)
    {
        stationList.print();
        Task task;
        task.customerID = "cust1";
        task.pickup = stationList.prompt("  Enter the pick-up location: ");
        task.dropoff = stationList.prompt("  Enter the drop-off location: ");
        unsigned id = gDBTalker->makeBooking(task.customerID, task.pickup, task.dropoff);
        task.taskID = id;
        task = addTask(task);
    }
    else if (opOption == OPERATOR_REMOVE_TASK)
    {
        printf ("  Enter id of task to be removed: ");

        try
        {
            gScheduler->removeTask((unsigned) getNumeric());

            if (optVerbosityLevel > 0 && optNoGUI)
                MSGLOGQUIET("After removing task...\nTask queue:\n%s", gScheduler->toString().c_str());
        }
        catch( SchedulerException & e )
        {
            if( e.type()==SchedulerException::TASK_DOES_NOT_EXIST )
                ERROR("This task does not exist.");
            else if( e.type()==SchedulerException::TASK_CANNOT_BE_CANCELLED )
                ERROR("This task cannot be removed (current task).");
        }
    }
    else if (opOption == OPERATOR_VIEW_TASK_LIST)
    {
        MSGLOGQUIET("\n%s", gScheduler->toString().c_str());
    }
    else if (opOption == OPERATOR_QUIT)
    {
        gQuit = 1;
        return;
    }
    update();
}