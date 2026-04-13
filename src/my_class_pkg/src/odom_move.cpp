#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <cmath>
#include <sensor_msgs/Imu.h>
#include <tf/transform_datatypes.h>

bool got_finish = false;
bool has_record_start = false;
double start_x = 0.0, start_y = 0.0;
double current_x = 0.0, current_y = 0.0;

bool turn_finish = false;
double yaw = 0.0;
double start_yaw = 0.0;
bool need_turn = false;
const double TARGET_ANGLE = 90.0;

int loop_cnt = 0;
const int MAX_LOOP = 4;

// 超时时间（秒）
const double MOVE_TIMEOUT = 15.0;
const double TURN_TIMEOUT = 15.0;
ros::Time action_start_time;

void odom_callback(const nav_msgs::OdometryConstPtr &odom_msg)
{
    current_x = odom_msg->pose.pose.position.x;
    current_y = odom_msg->pose.pose.position.y;

    if (!has_record_start)
    {
        start_x = current_x;
        start_y = current_y;
        has_record_start = true;
        ROS_INFO("起点：(%.2f, %.2f)，目标前进1米", start_x, start_y);
        return;
    }

    double dist = sqrt(pow(current_x - start_x, 2) + pow(current_y - start_y, 2));

    // ====================== 改成 1.0 米 ======================
    if (dist >= 1.0 && !got_finish)
    {
        got_finish = true;
        ROS_INFO("前进完成！总距离：%.2f米", dist);
    }
}

void imu_callback(const sensor_msgs::ImuConstPtr &imu_msg)
{
    double roll, pitch;
    tf::Quaternion q(
        imu_msg->orientation.x,
        imu_msg->orientation.y,
        imu_msg->orientation.z,
        imu_msg->orientation.w);
    tf::Matrix3x3(q).getRPY(roll, pitch, yaw);
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "square_controller");
    ros::NodeHandle nh;

    ros::Subscriber odom_sub = nh.subscribe("/odom", 10, odom_callback);
    ros::Subscriber imu_sub = nh.subscribe("/imu/data", 10, imu_callback);
    ros::Publisher cmd_pub_ = nh.advertise<geometry_msgs::Twist>("/cmd_vel", 10);

    geometry_msgs::Twist cmd_vel;
    ros::Rate rate(20);

    action_start_time = ros::Time::now();

    while (ros::ok())
    {
        // 前进阶段
        if (!got_finish)
        {
            if ((ros::Time::now() - action_start_time).toSec() > MOVE_TIMEOUT)
            {
                ROS_WARN("前进超时，强制进入转弯");
                got_finish = true;
            }
            else
            {
                cmd_vel.linear.x = 0.2;
                cmd_vel.angular.z = 0.0;
            }
        }
        // 转弯阶段
        else
        {
            cmd_vel.linear.x = 0.0;

            if (!need_turn)
            {
                need_turn = true;
                start_yaw = yaw;
                action_start_time = ros::Time::now();
                ROS_INFO("=== 开始左转90° ===");
            }

            if (need_turn && !turn_finish)
            {
                if ((ros::Time::now() - action_start_time).toSec() > TURN_TIMEOUT)
                {
                    ROS_WARN("转弯超时，强制结束");
                    turn_finish = true;
                }

                double angle_diff_rad = yaw - start_yaw;
                angle_diff_rad = atan2(sin(angle_diff_rad), cos(angle_diff_rad));
                double angle_diff = angle_diff_rad * 57.29578;

                if (angle_diff < TARGET_ANGLE - 1.0)
                {
                    cmd_vel.angular.z = 0.3;
                }
                else
                {
                    cmd_vel.angular.z = 0.0;
                    turn_finish = true;
                    ROS_INFO("转弯90°完成");

                    loop_cnt++;
                    if (loop_cnt >= MAX_LOOP)
                    {
                        ROS_INFO("=== 1x1 米正方形路径完成！===");
                        break;
                    }

                    got_finish = false;
                    has_record_start = false;
                    need_turn = false;
                    turn_finish = false;
                    action_start_time = ros::Time::now();
                    ros::Duration(0.5).sleep();
                }
            }
        }

        cmd_pub_.publish(cmd_vel);
        ros::spinOnce();
        rate.sleep();
    }

    cmd_vel.linear.x = 0.0;
    cmd_vel.angular.z = 0.0;
    cmd_pub_.publish(cmd_vel);
    ROS_INFO("节点退出，机器人已停止");

    return 0;
}
