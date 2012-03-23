/** A node that prompts the passenger (text mode) whether it is safe to go
 * at a junction. Stopping points are hard coded.
 *
 */

#include <string>
#include <cmath>

#include <ros/ros.h>

#include <tf/transform_listener.h>
#include <tf/transform_datatypes.h>

#include <std_msgs/Bool.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/Point.h>
#include <geometry_msgs/PolygonStamped.h>
#include <nav_msgs/Odometry.h>
#include <nav_msgs/Path.h>

#include <fmutil/fm_math.h>


using namespace std;


class StopJunction
{
public:
    StopJunction();

    ros::NodeHandle n;
    tf::TransformListener tf_;
    ros::Publisher junction_pub;
    ros::Subscriber global_plan_;

private:
    vector<geometry_msgs::Point> stoppingPoint_;
    double stopping_distance_;
    int lastStop_;

    bool getRobotGlobalPose(tf::Stamped<tf::Pose>& odom_pose) const;
    void globalPlanCallback(nav_msgs::Path path);
    void UILoop();
};


StopJunction::StopJunction()
{
    junction_pub = n.advertise<std_msgs::Bool>("/nav_junction", 1);
    global_plan_ = n.subscribe("/global_plan", 1,
            &StopJunction::globalPlanCallback, this);

    ros::NodeHandle nh("~");
    nh.param("stopping_distance", stopping_distance_, 8.0);

    lastStop_ = -1;



    //-------------------------------------------------------------------------
    // hard coded stopping points

    geometry_msgs::Point p;
    p.x = 322; p.y = 2300; stoppingPoint_.push_back(p);
    p.x = 636; p.y = 538; stoppingPoint_.push_back(p);

    //-------------------------------------------------------------------------

    StopJunction::UILoop();
}

void StopJunction::UILoop()
{
    ros::Rate loop(10);
    double res = 0.1;
    int y_pixels = 3536;

    while( ros::ok() )
    {
        tf::Stamped<tf::Pose> robot_pose;
        getRobotGlobalPose(robot_pose);
        double robot_x = robot_pose.getOrigin().x();
        double robot_y = robot_pose.getOrigin().y();

        std_msgs::Bool msg;

        for( unsigned i=0; i<stoppingPoint_.size(); i++ )
        {
            double sp_x = stoppingPoint_[i].x * res;
            double sp_y = (y_pixels - stoppingPoint_[i].y) * res;
            double d = fmutil::distance(robot_x,robot_y,sp_x, sp_y);
            if( d<=stopping_distance_ && lastStop_!=i )
            {
                lastStop_ = i;

                msg.data = true;
                junction_pub.publish(msg);
                ros::spinOnce();

                string temp = "";
                cout <<"Clear to go ?" <<endl;
                getline(cin, temp);
                cout <<"Continue" <<endl;
            }
        }

        msg.data = false;
        junction_pub.publish(msg);

        ros::spinOnce();
        loop.sleep();
    }
}

void StopJunction::globalPlanCallback(nav_msgs::Path path)
{
    //listen to any new path. Simply reset the last stopping record so
    //that all stops can be reevaluated again
    lastStop_ = -1;
}

bool StopJunction::getRobotGlobalPose(tf::Stamped<tf::Pose>& odom_pose) const
{
    odom_pose.setIdentity();
    tf::Stamped<tf::Pose> robot_pose;
    robot_pose.setIdentity();
    robot_pose.frame_id_ = "/base_link";
    robot_pose.stamp_ = ros::Time();
    ros::Time current_time = ros::Time::now(); // save time for checking tf delay later

    try {
        tf_.transformPose("/map", robot_pose, odom_pose);
    }
    catch(tf::LookupException& ex) {
        ROS_ERROR("No Transform available Error: %s\n", ex.what());
        return false;
    }
    catch(tf::ConnectivityException& ex) {
        ROS_ERROR("Connectivity Error: %s\n", ex.what());
        return false;
    }
    catch(tf::ExtrapolationException& ex) {
        ROS_ERROR("Extrapolation Error: %s\n", ex.what());
        return false;
    }
    return true;
}


int main(int argc, char **argv)
{
    ros::init(argc, argv, "stop_junction");
    StopJunction sa;
    ros::spin();
    return 0;
}
