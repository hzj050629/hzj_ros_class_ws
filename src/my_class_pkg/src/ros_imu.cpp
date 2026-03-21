#include <ros/ros.h>
#include <sensor_msgs/Imu.h>
#include <geometry_msgs/Twist.h>
#include <tf/transform_datatypes.h>
#include <cmath>

// 全局变量（简化逻辑，确保启动即旋转）
ros::Publisher vel_pub;          // 运动指令发布器
double initial_yaw = 0.0;        // 初始偏航角（旋转前的基准角度）
double current_yaw = 0.0;        // 当前偏航角
double target_angle = M_PI;      // 180° = π 弧度
double rotate_angular_vel = 0.6; // 旋转角速度（0.6rad/s，适中不易超调）
bool has_initial_yaw = false;    // 是否获取到初始角度
bool rotation_complete = false;  // 旋转是否完成

// 弧度转角度（方便打印）
inline double rad2deg(double rad) {
    return rad * 180.0 / M_PI;
}

// 角度归一化到 [-π, π] 范围（解决角度跳变问题）
double normalize_angle(double angle) {
    while (angle > M_PI) angle -= 2 * M_PI;
    while (angle < -M_PI) angle += 2 * M_PI;
    return angle;
}

// IMU回调函数：解析/ros/imu数据，计算旋转角度
void imuCallback(const sensor_msgs::Imu::ConstPtr& msg) {
    // 1. 四元数转欧拉角（仅关注yaw：绕z轴旋转角度）
    tf::Quaternion quat;
    tf::quaternionMsgToTF(msg->orientation, quat);
    double roll, pitch, yaw;
    tf::Matrix3x3(quat).getRPY(roll, pitch, yaw);
    current_yaw = yaw;

    // 2. 记录初始角度（仅第一次回调时）
    if (!has_initial_yaw) {
        initial_yaw = yaw;
        has_initial_yaw = true;
        ROS_INFO("初始偏航角已记录：%.1f°", rad2deg(initial_yaw));
        return;
    }

    // 3. 计算已旋转的角度（相对初始角度的差值）
    double rotated_angle = normalize_angle(current_yaw - initial_yaw);
    // 取绝对值（无论顺时针/逆时针，只看旋转角度）
    double rotated_angle_abs = fabs(rotated_angle);

    // 实时打印调试信息
    ROS_INFO_THROTTLE(0.1, "已旋转角度：%.1f° | 目标：180.0°", rad2deg(rotated_angle_abs));

    // 4. 达到180°（误差±3°），停止旋转
    if (rotated_angle_abs >= M_PI - 0.05 && !rotation_complete) {
        ROS_WARN("✅ 旋转180°完成！立即停止");
        // 发布停止指令
        geometry_msgs::Twist stop_cmd;
        stop_cmd.linear.x = 0.0;
        stop_cmd.angular.z = 0.0;
        vel_pub.publish(stop_cmd);
        rotation_complete = true;
    }
}

int main(int argc, char** argv) {
    // 1. 初始化ROS节点
    ros::init(argc, argv, "imu_rotate_180_node");
    ros::NodeHandle nh;

    // 2. 创建运动指令发布器（/cmd_vel是差分底盘通用话题）
    vel_pub = nh.advertise<geometry_msgs::Twist>("/cmd_vel", 10);

    // 3. 订阅IMU话题（核心：/ros/imu）
    ros::Subscriber imu_sub = nh.subscribe("/ros/imu", 50, imuCallback); // 增大队列避免数据丢失

    ROS_INFO("=== IMU旋转180°实验启动 ===");
    ROS_INFO("IMU话题：/ros/imu | 旋转角速度：%.1f rad/s", rotate_angular_vel);

    // 4. 等待获取初始IMU角度（最多等3秒，避免卡死）
    ros::Rate wait_rate(100);
    int wait_count = 0;
    while (!has_initial_yaw && ros::ok() && wait_count < 300) {
        ros::spinOnce();
        wait_rate.sleep();
        wait_count++;
    }
    if (!has_initial_yaw) {
        ROS_ERROR("❌ 3秒内未获取到/ros/imu数据，请检查传感器连接！");
        return -1;
    }

    // 5. 立即发布旋转指令（启动即转，无延迟）
    ROS_INFO("🚀 开始旋转180°...");
    geometry_msgs::Twist rotate_cmd;
    rotate_cmd.linear.x = 0.0;                // 线速度为0，仅旋转
    rotate_cmd.angular.z = rotate_angular_vel; // 顺时针旋转（负值=逆时针）
    vel_pub.publish(rotate_cmd);

    // 6. 持续发布旋转指令（每秒10次，确保小车持续旋转）
    ros::Rate loop_rate(10);
    while (ros::ok() && !rotation_complete) {
        // 持续发布旋转指令（防止指令中断）
        if (!rotation_complete) {
            vel_pub.publish(rotate_cmd);
        }
        ros::spinOnce();
        loop_rate.sleep();
    }

    ROS_INFO("=== 实验结束 ===");
    return 0;
}
