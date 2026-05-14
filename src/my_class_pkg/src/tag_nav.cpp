#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <iostream>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>

#include "tf2_ros/transform_listener.h"
#include "geometry_msgs/TransformStamped.h"
#include <std_msgs/String.h>

using namespace std;

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

// ================= 封装识别并播报的通用函数 =================
void detectAndSpeak(tf2_ros::Buffer& buffer, ros::Publisher& voice_pub, const string& tag_name) {
    ROS_INFO("正在监听坐标变换，等待识别到 %s (10秒超时)...", tag_name.c_str());
    try {
        // 将超时时间从 30 秒改成了 10 秒
        geometry_msgs::TransformStamped tfs = buffer.lookupTransform("arm_base_link", tag_name, ros::Time(0), ros::Duration(1));

        // 成功获取到坐标，说明识别到了目标
        std_msgs::String voice_msg;
        voice_msg.data = "已找到目标";
        voice_pub.publish(voice_msg);
        ROS_INFO("成功识别到 %s，语音播报指令已发送！", tag_name.c_str());
        
    } catch (tf2::TransformException &ex) {
        // 10秒内没看到Tag，播报“未找到目标”
        std_msgs::String voice_msg;
        voice_msg.data = "未找到目标";
        voice_pub.publish(voice_msg);
        ROS_WARN("在10秒内未能识别到 %s: %s", tag_name.c_str(), ex.what());
    }
    
    // 播报完成后等待 2 秒，确保语音完整播放
    ros::Duration(2.0).sleep();
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "nav_tag_speech_node");
    ros::NodeHandle nh;

    // ================= 导航与功能初始化 =================
    MoveBaseClient ac("move_base", true);
    ROS_INFO("Waiting for move_base server...");
    ac.waitForServer();
    ROS_INFO("Connected to move_base server.");

    ros::Publisher voice_pub = nh.advertise<std_msgs::String>("/robot_voice/llm/result", 10);
    tf2_ros::Buffer buffer;
    tf2_ros::TransformListener listener(buffer);
    ROS_INFO("初始化完成，准备开始任务...");

    move_base_msgs::MoveBaseGoal goal1, goal2, goal3;
    tf2::Quaternion quaternion;

    // ===================== Goal 1 =====================
    quaternion.setRPY(0, 0, 1.531);
    goal1.target_pose.pose.position.x = 2.647;
    goal1.target_pose.pose.position.y = 1.675;
    goal1.target_pose.pose.orientation = tf2::toMsg(quaternion);
    goal1.target_pose.header.frame_id = "map";
    goal1.target_pose.header.stamp = ros::Time::now();

    ac.sendGoal(goal1);
    ROS_INFO("Sending Goal 1...");
    ac.waitForResult();

    if (ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
    {
        ROS_INFO("Goal 1 reached successfully! 准备开始识别Tag...");
        ros::Duration(1.5).sleep(); 
        detectAndSpeak(buffer, voice_pub, "tag_1");
    }
    else
    {
        ROS_WARN("Failed to reach Goal 1. State: %s", ac.getState().toString().c_str());
        ROS_WARN("由于未到达目标点1，跳过Tag识别，直接尝试前往目标点2...");
    }


// =====================  Goal 1.5 =====================
 /*   quaternion.setRPY(0, 0, 0);
    goal3.target_pose.pose.position.x = 0.1;
    goal3.target_pose.pose.position.y = 2.388;
    goal3.target_pose.pose.orientation = tf2::toMsg(quaternion);
    goal3.target_pose.header.frame_id = "map";
    goal3.target_pose.header.stamp = ros::Time::now();

    ac.sendGoal(goal3);
    ROS_INFO("Sending Final Goal...");
    ac.waitForResult();

    if (ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
    {
        ROS_INFO("Final goal reached! Task finished.");
    }
    else
    {
        ROS_WARN("Failed to reach Final goal. State: %s", ac.getState().toString().c_str());
    }
        */


    // ===================== Goal 2 =====================
    quaternion.setRPY(0, 0, -1.5603);
    goal2.target_pose.pose.position.x = 2.6;
    goal2.target_pose.pose.position.y = 3.1;
    goal2.target_pose.pose.orientation = tf2::toMsg(quaternion);
    goal2.target_pose.header.frame_id = "map";
    goal2.target_pose.header.stamp = ros::Time::now();

    ac.sendGoal(goal2);
    ROS_INFO("Sending Goal 2...");
    ac.waitForResult();

    if (ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
    {
        ROS_INFO("Goal 2 reached successfully! 准备开始识别Tag...");
        ros::Duration(1.5).sleep();
        detectAndSpeak(buffer, voice_pub, "tag_2");
    }
    else
    {
        ROS_WARN("Failed to reach Goal 2. State: %s", ac.getState().toString().c_str());
    }
    
    // ===================== Final Goal =====================
    quaternion.setRPY(0, 0, 0);
    goal3.target_pose.pose.position.x = -0.404;
    goal3.target_pose.pose.position.y = 2.388;
    goal3.target_pose.pose.orientation = tf2::toMsg(quaternion);
    goal3.target_pose.header.frame_id = "map";
    goal3.target_pose.header.stamp = ros::Time::now();

    ac.sendGoal(goal3);
    ROS_INFO("Sending Final Goal...");
    ac.waitForResult();

    if (ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
    {
        ROS_INFO("Final goal reached! Task finished.");
    }
    else
    {
        ROS_WARN("Failed to reach Final goal. State: %s", ac.getState().toString().c_str());
    }

    return 0;
}
    
    

/*    // ===================== Final Goal =====================
    quaternion.setRPY(0, 0, 0);
    goal3.target_pose.pose.position.x = -0.6;
    goal3.target_pose.pose.position.y = 2.388;
    goal3.target_pose.pose.orientation = tf2::toMsg(quaternion);
    goal3.target_pose.header.frame_id = "map";
    goal3.target_pose.header.stamp = ros::Time::now();

    ac.sendGoal(goal3);
    ROS_INFO("Sending Final Goal...");
    ac.waitForResult();

    if (ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
    {
        ROS_INFO("Final goal reached! Task finished.");
    }
    else
    {
        ROS_WARN("Failed to reach Final goal. State: %s", ac.getState().toString().c_str());
    }

    return 0;  
}                        */
