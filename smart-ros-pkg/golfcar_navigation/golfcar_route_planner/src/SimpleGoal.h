#ifndef __ROUTE_PLANNER_VEHICLE_SIMPLE_GOAL__H__
#define __ROUTE_PLANNER_VEHICLE_SIMPLE_GOAL__H__

#include <math.h>

#include <ros/ros.h>

#include <golfcar_ppc/speed_contribute.h>

#include "RoutePlanner.h"


/// Drives the vehicle from A to B by sending origin and destination station
/// number to the global planner:
/// publish a geometry_msgs::PoseStamped on "move_base_simple/goal"
class SimpleGoal : public RoutePlanner
{
public:
    SimpleGoal(const StationPaths & sp);

private:
    ros::NodeHandle nh;
    ros::Publisher goal_pub_;
    ros::Subscriber speed_status_sub_;

    bool has_reached_;

    bool goToDest();
    void initDest();
    void speedStatusCallBack(const golfcar_ppc::speed_contribute &);
};

#endif
