第一周
1.小车1x1走方格：rosrun my_class_pkg odom_move


第二周
1.cpp发布消息：rosrun my_class_pkg ros_publisher_node
2.cpp订阅消息：rosrun my_class_pkg ros_subscriber_node
3.py发布消息：rosrun my_class_pkg ros_publisher_node.py
4.py订阅消息：rosrun my_class_pkg ros_subscriber_node.py
5.自定义消息：rosmsg show my_class_pkg/MyMessage
6.自定义消息cpp发布：rosrun my_class_pkg msg_publisher_node
7.自定义消息cpp订阅：rosrun my_class_pkg msg_subscriber_node
8.launch文件同时启动节点：roslaunch my_class_pkg bringup_topic.launch
9.自定义服务：rossrv show my_class_pkg/MyServiceMsg
10.cpp发布服务：rosrun my_class_pkg ros_server_node
11.cpp接收服务：rosrun my_class_pkg ros_client_node
12.py发布服务：rosrun my_class_pkg ros_server.py
13.py接收服务：rosrun my_class_pkg ros_client.py
14.cpp发送动作：rosrun my_class_pkg ros_action_server
15.cpp接收动作：rosrun my_class_pkg ros_action_client
16.py发送动作：rosrun my_class_pkg ros_action_server.py
17.py接收动作：rosrun my_class_pkg ros_action_client.py

18.碰撞传感器避障：rosrun my_class_pkg ros_bump_node
19.超声tof避障：rosrun my_class_pkg ros_sonic_node
20.imu自旋：rosrun my_class_pkg ros_imu_node


第三周
1.cpp设置读取参数：rosrun my_class_pkg ros_param
2.py设置读取参数：rosrun my_class_pkg ros_param.py
3.launch文件配置：roslaunch my_class_pkg parameter.launch
4.配置动态参数：rosrun my_class_pkg dynamic_reconfigure_node
5.rqt修改参数：rosrun rqt_reconfigure rqt_reconfigure
6.动态配置小车速度：rosrun my_class_pkg ros_dynamic_speed_node

7.cpp实现log节点：rosrun my_class_pkg ros_log
8.py实现log节点：rosrun my_class_pkg ros_log.py


第四周
1.两轮小车gazebo仿真：roslaunch hzj_robot_description gazebo.launch
2.小车控制：rosrun teleop_twist_keyboard teleop_twist_keyboard.py
3.机械臂仿真：roslaunch hzj_robot_description gazebo_w2a_arm.launch
4.机械臂控制：rosrun rqt_joint_trajectory_controller rqt_joint_trajectory_controller


第五周
1.使用 OpenCV 处理 ROS 获取的图像：rosrun my_class_pkg get_ros_image.py                启动rqt：rosrun rqt_image_view rqt_image_view
2.巡线：rosrun my_class_pkg follow_line.py            可视化：rostopic pub -1 /enable_move std_msgs/Int16 "data: 1"
3.手势识别：rosrun my_class_pkg gesture_movement.py
4.视觉跟踪：rosrun my_class_pkg apriltag_follow.py
5.视觉抓取：roslaunch upros_arm recognize_apriltag.launch                     rosrun my_class_pkg tag_grab_node


第六周
1.激光雷达获取信息：rosrun my_class_pkg ros_scan_node
2.激光雷达避障：rosrun my_class_pkg ros_avoid_node
3.自主导航：roslaunch upros_bringup bringup_w2a.launch      roslaunch upros_navigation navigation.launch     roslaunch upros_navigation view_nav.launch    rosrun my_class_pkg movebase_client_node


第七周
1.语音导航：roslaunch upros_chat speech_to_word.launch      roslaunch upros_chat speech_to_word.launch      rosrun my_class_pkg voice_nav_node


期末考试：
roslaunch upros_bringup bringup_w2a.launch
roslaunch upros_arm recognize_apriltag.launch
roslaunch upros_chat word_to_speech.launch
roslaunch upros_navigation navigation.launch
roslaunch upros_navigation view_nav.launch
rosrun my_class_pkg tag_nav_node
