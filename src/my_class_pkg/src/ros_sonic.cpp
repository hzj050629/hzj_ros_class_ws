#include <ros/ros.h>
#include <sensor_msgs/Range.h>
#include <geometry_msgs/Twist.h>
#include <ros/duration.h>

// 全局变量：运动指令发布器、ROS循环频率（控制指令发布频率）
ros::Publisher vel_pub;
ros::Rate* loop_rate;
// 状态标志：是否正在后退（防止中断后退流程）
bool is_backing = false;
// 运动参数配置（可直接调整）
const double FORWARD_SPEED = 0.2;    // 前进速度：0.2m/s（持续发布）
const double BACK_SPEED = -0.2;      // 后退速度：0.2m/s（负值为后退）
const double BACK_DISTANCE = 0.2;    // 后退距离：20cm（0.2m）
const double TRIGGER_DISTANCE = 0.4; // 触发后退的距离阈值
// 后退持续时间（精准控制20cm）：时间=距离/速度
const double BACK_DURATION = BACK_DISTANCE / fabs(BACK_SPEED);

// 通用函数：发布运动指令
void publish_vel(double linear_x) {
    geometry_msgs::Twist vel_msg;
    // 差分底盘仅需设置线速度x（前后），角速度全为0
    vel_msg.linear.x = linear_x;
    vel_msg.linear.y = 0.0;
    vel_msg.linear.z = 0.0;
    vel_msg.angular.x = 0.0;
    vel_msg.angular.y = 0.0;
    vel_msg.angular.z = 0.0;
    vel_pub.publish(vel_msg);
}

// TOF2传感器回调函数：仅更新状态，不处理运动逻辑（避免回调阻塞）
double current_tof_distance = 0.0; // 存储当前传感器距离
void tof2_callback(const sensor_msgs::Range::ConstPtr& msg) {
    current_tof_distance = msg->range;
    ROS_INFO_THROTTLE(0.5, "TOF2当前距离：%.2f m", current_tof_distance); // 每0.5s打印一次，避免刷屏
}

int main(int argc, char** argv) {
    // 1. 初始化ROS节点
    ros::init(argc, argv, "tof2_sonic_control");
    ros::NodeHandle nh;

    // 2. 创建发布器（运动指令话题：/cmd_vel，队列大小10）
    vel_pub = nh.advertise<geometry_msgs::Twist>("/cmd_vel", 10);
    // 3. 创建订阅器（TOF2传感器话题：/us/tof2）
    ros::Subscriber tof2_sub = nh.subscribe("/us/tof2", 10, tof2_callback);
    // 4. 设置循环频率（10Hz：每秒发布10次指令，确保持续前进）
    loop_rate = new ros::Rate(10);

    ROS_INFO("=== TOF2传感器小车控制节点启动 ===");
    ROS_INFO("规则：距离≥0.4m → 0.2m/s持续前进 | 距离<0.4m → 停止前进+后退20cm");

    // 主循环：持续检测距离并发布运动指令（核心逻辑）
    while (ros::ok()) {
        // 先处理传感器回调（更新当前距离）
        ros::spinOnce();

        // 逻辑1：正在后退时，跳过所有运动指令（确保后退流程不中断）
        if (is_backing) {
            loop_rate->sleep();
            continue;
        }

        // 逻辑2：距离<0.4m → 停止前进 + 后退20cm
        if (current_tof_distance < TRIGGER_DISTANCE && current_tof_distance > 0) { // 排除传感器异常值
            ROS_WARN("检测到障碍物（距离=%.2fm < 0.4m）→ 停止前进，开始后退20cm", current_tof_distance);
            
            // 第一步：立即停止前进
            publish_vel(0.0);
            is_backing = true; // 标记为后退中

            // 第二步：后退20cm（持续发布后退指令，确保精准）
            for (int i = 0; i < BACK_DURATION * 10; i++) { // 按10Hz发布后退指令
                publish_vel(BACK_SPEED);
                loop_rate->sleep();
            }

            // 第三步：后退完成，停止小车
            publish_vel(0.0);
            ROS_INFO("后退20cm完成，等待距离恢复≥0.4m继续前进");
            is_backing = false; // 复位后退标志

        // 逻辑3：距离≥0.4m → 持续发布前进指令（10Hz）
        } else if (current_tof_distance >= TRIGGER_DISTANCE) {
            publish_vel(FORWARD_SPEED);
        }

        // 控制循环频率（10Hz）
        loop_rate->sleep();
    }

    // 程序退出前停止小车
    publish_vel(0.0);
    delete loop_rate;
    return 0;
}
