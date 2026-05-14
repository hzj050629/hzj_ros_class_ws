#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <iostream>
#include <tf2/LinearMath/Quaternion.h>
#include <geometry_msgs/Quaternion.h>
#include <cmath>

#include "upros_message/TagCommand.h"

using namespace std;

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

void sendGoal(int target)
{
    MoveBaseClient ac("move_base", true);
    ac.waitForServer();

    move_base_msgs::MoveBaseGoal goal;
    tf2::Quaternion quat;

    if (target == 1)
    {
        quat.setRPY(0, 0, 0.01);
        goal.target_pose.pose.position.x = 1.146;
        goal.target_pose.pose.position.y = -1.121;
        goal.target_pose.pose.orientation.z = quat.z();
        goal.target_pose.pose.orientation.w = quat.w();
    }
    else if (target == 2)
    {
        quat.setRPY(0, 0, 0.13);
        goal.target_pose.pose.position.x = 2.516;
        goal.target_pose.pose.position.y = -1.68;
        goal.target_pose.pose.orientation.z = quat.z();
        goal.target_pose.pose.orientation.w = quat.w();
    }
    else
    {
        ROS_WARN("invalid target");
        goal.target_pose.pose.position.x = 0.0;
        goal.target_pose.pose.position.y = 0.0;
        goal.target_pose.pose.orientation.w = 1.0;
        return;
    }

    goal.target_pose.header.frame_id = "map";
    goal.target_pose.header.stamp = ros::Time::now();

    ROS_INFO("going to target: %d", target);
    ac.sendGoal(goal);
    ac.waitForResult();

    if (ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
        ROS_INFO("arrived at target: %d", target);
    else
        ROS_ERROR("navigation failed");
}

void callback(const upros_message::TagCommand::ConstPtr& msg)
{
    if (msg->intent != "go_to") {
        ROS_INFO("ignore command: %s", msg->intent.c_str());
        return;
    }

    int target = msg->target;
    ROS_INFO("------------------------------");
    ROS_INFO("received command");
    ROS_INFO("intent: %s", msg->intent.c_str());
    ROS_INFO("target: %d", target);
    ROS_INFO("------------------------------");

    sendGoal(target);
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "voice_nav_node");
    ros::NodeHandle nh;

    ros::Subscriber sub = nh.subscribe("/voice_control", 10, callback);

    ROS_INFO("voice nav node started");
    ROS_INFO("subscribed to: /voice_control");

    ros::spin();
    return 0;
}