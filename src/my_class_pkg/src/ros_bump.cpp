#include "ros/ros.h"
#include "std_msgs/Int16MultiArray.h"
#include "geometry_msgs/Twist.h"
#include <unistd.h>

// 全局发布者
ros::Publisher cmd_vel_pub;
// 碰撞标志位
bool is_bumped = false;
// 速度指令消息（全局复用，减少创建开销）
geometry_msgs::Twist vel_msg;

/**
 * 碰撞传感器回调函数
 * 触发1号传感器→后退20cm→恢复前进；未触发→持续前进
 */
void bumpCallback(const std_msgs::Int16MultiArray::ConstPtr& msg)
{
    // 校验传感器数据完整性
    if (msg->data.size() < 3)
    {
        ROS_WARN("Bump sensor data incomplete!");
        return;
    }

    // 1号正前方传感器触发碰撞
    if (msg->data[1] == 1 && !is_bumped)
    {
        ROS_INFO("===== Front bumper (No.1) triggered! Start reversing 20cm =====");
        is_bumped = true;

        // 第一步：停止前进
        vel_msg.linear.x = 0;
        cmd_vel_pub.publish(vel_msg);
        usleep(500000); // 暂停0.5秒，确保停止

        // 第二步：后退20cm（速度-0.1m/s，持续2秒）
        vel_msg.linear.x = -0.1;
        cmd_vel_pub.publish(vel_msg);
        sleep(2); // 后退持续2秒（0.1×2=0.2m=20cm）

        // 第三步：停止后退
        vel_msg.linear.x = 0;
        cmd_vel_pub.publish(vel_msg);
        usleep(500000); // 暂停0.5秒，确保停止

        ROS_INFO("===== Reversed 20cm completed! Resume moving forward =====");
        // 重置碰撞标志位，恢复前进
        is_bumped = false;
    }
}

int main(int argc, char **argv)
{
    // 初始化ROS节点
    ros::init(argc, argv, "bump_avoid_node");
    ros::NodeHandle n;

    // 创建速度指令发布者（主题/cmd_vel，队列大小10）
    cmd_vel_pub = n.advertise<geometry_msgs::Twist>("/cmd_vel", 10);
    // 订阅碰撞传感器数据
    ros::Subscriber sub = n.subscribe("/robot/bump_sensor", 1000, bumpCallback);

    // 初始化速度指令：默认持续前进（0.1m/s，无旋转）
    vel_msg.linear.x = 0.1;   // 前进速度（可根据小车调整，比如0.2）
    vel_msg.linear.y = 0;
    vel_msg.linear.z = 0;
    vel_msg.angular.x = 0;
    vel_msg.angular.y = 0;
    vel_msg.angular.z = 0;

    ROS_INFO("Bumper avoid node started! Moving forward...");

    // 循环发布前进指令（10Hz），碰撞时被回调函数中断
    ros::Rate rate(10); // 10Hz发布频率，确保小车持续前进
    while (ros::ok())
    {
        // 未碰撞时持续发布前进指令
        if (!is_bumped)
        {
            cmd_vel_pub.publish(vel_msg);
        }
        // 处理回调函数（传感器数据）
        ros::spinOnce();
        // 按照10Hz频率循环
        rate.sleep();
    }

    return 0;
}
