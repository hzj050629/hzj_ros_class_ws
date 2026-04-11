#include "tf2_ros/transform_listener.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.h"
#include "geometry_msgs/TransformStamped.h"
#include "geometry_msgs/PointStamped.h"

#include "upros_message/ArmPosition.h"
#include "std_srvs/Empty.h"
#include <ros/ros.h>

// 坐标安全限制
int limit(int val, int min, int max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

void sleep(double second)
{
    ros::Duration(second).sleep();
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "mgrab_test");
    ros::AsyncSpinner spinner(1);
    spinner.start();

    ros::NodeHandle nh;

    tf2_ros::Buffer buffer;
    tf2_ros::TransformListener listener(buffer);

    // 服务客户端
    ros::ServiceClient arm_move_open_client = nh.serviceClient<upros_message::ArmPosition>("/upros_arm_control/arm_pos_service_open");
    ros::ServiceClient arm_zero_client = nh.serviceClient<std_srvs::Empty>("/upros_arm_control/zero_service");
    ros::ServiceClient arm_grab_client = nh.serviceClient<std_srvs::Empty>("/upros_arm_control/grab_service");
    ros::ServiceClient arm_release_client = nh.serviceClient<std_srvs::Empty>("/upros_arm_control/release_service");

    ROS_INFO("Waiting for arm services...");
    arm_move_open_client.waitForExistence();
    arm_release_client.waitForExistence();
    arm_grab_client.waitForExistence();
    arm_zero_client.waitForExistence();
    ROS_INFO("All services connected!");

    // 回零
    ROS_INFO("GO HOME FIRST!");
    std_srvs::Empty empty_srv;
    arm_zero_client.call(empty_srv);
    sleep(3.0);

    // 等待TF
    ROS_INFO("Waiting for TF transform...");
    sleep(1.0);

    geometry_msgs::TransformStamped tfs_1;
    try {
        tfs_1 = buffer.lookupTransform("arm_base_link", "tag_1", ros::Time(0), ros::Duration(3.0));
    } catch (tf2::TransformException &ex) {
        ROS_ERROR("TF ERROR: %s", ex.what());
        return 1;
    }

    // ==============================================
    // 🚀 终极加大：向前 + 向下 拉到最大安全距离
    // ==============================================
    int x = -int(tfs_1.transform.translation.y * 1000);
    
    // Y 轴 向前：原来+30 → 现在 +80mm（大幅往前伸）
    int y = int(tfs_1.transform.translation.x * 1000) + 80;  
    
    // Z 轴 向下：原来-50 → 现在 -100mm（大幅往下压）
    int z = int(tfs_1.transform.translation.z * 1000) - 100; 
    // ==============================================

    // 安全限位完全放开（保证能走到）
    x = limit(x, -200, 200);
    y = limit(y, 20, 300);  // 向前最大范围
    z = limit(z, 20, 300);  // 向下最大范围

    ROS_INFO("TARGET POS: X=%d  Y=%d  Z=%d", x, y, z);

    // 抓取流程
    upros_message::ArmPosition move_srv;
    move_srv.request.x = x;
    move_srv.request.y = y;
    move_srv.request.z = z;

    // 打开夹爪
    arm_release_client.call(empty_srv);
    sleep(2.0);

    // 移动到抓取点
    arm_move_open_client.call(move_srv);
    sleep(7.0);

    // 抓取
    arm_grab_client.call(empty_srv);
    sleep(3.0);

    // 回零
    arm_zero_client.call(empty_srv);
    sleep(5.0);

    ROS_INFO("GRAB COMPLETED SUCCESSFULLY!");
    ros::shutdown();
    return 0;
}
