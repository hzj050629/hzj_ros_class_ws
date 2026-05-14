#include "tf2_ros/transform_listener.h"
#include "geometry_msgs/TransformStamped.h"
#include <ros/ros.h>
#include <std_msgs/String.h> // 保留语音播报需要的头文件

int main(int argc, char **argv)
{
    ros::init(argc, argv, "tag_detection_speech"); // 修改节点名，更贴合当前功能
    ros::AsyncSpinner spinner(1);
    spinner.start();
    ros::NodeHandle nh;

    // 创建语音播报的 Publisher
    ros::Publisher voice_pub = nh.advertise<std_msgs::String>("/robot_voice/llm/result", 10);

    tf2_ros::Buffer buffer;
    tf2_ros::TransformListener listener(buffer);
    ROS_INFO("正在监听坐标变换，等待识别到 tag_1...");

    try {
        // 尝试获取 tag_1 的坐标变换，设置超时时间为 100 秒
        // 如果能成功执行到这里，说明已经识别到了目标
        geometry_msgs::TransformStamped tfs_1 = buffer.lookupTransform("arm_base_link", "tag_1", ros::Time(0), ros::Duration(100));

        // 成功获取到坐标，触发语音播报
        std_msgs::String voice_msg;
        voice_msg.data = "已找到目标";
        voice_pub.publish(voice_msg);
        
        ROS_INFO("成功识别到 tag_1，语音播报指令已发送！");
        
        // 播报完成后等待 2 秒，确保语音节点有足够时间接收并处理消息
        ros::Duration(2.0).sleep();

    } catch (tf2::TransformException &ex) {
        // 如果超时或发生其他错误，打印错误信息
        ROS_WARN("获取坐标变换失败: %s", ex.what());
    }

    ros::shutdown();
    return 0;
}
