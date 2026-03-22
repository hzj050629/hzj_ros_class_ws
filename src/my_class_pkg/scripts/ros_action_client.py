#!/usr/bin/env python3
import rospy
import actionlib
# 修复1：补全from...import完整语法（核心语法错误）
from my_class_pkg.msg import MyActionAction, MyActionGoal, MyActionResult, MyActionFeedback

# 反馈回调函数：接收服务端的进度反馈
def feedback_cb(feedback):
    rospy.loginfo('Progress: {}%'.format(feedback.progress))

if __name__ == '__main__':  # 修复2：if和__name__之间加空格
    try:
        rospy.init_node('my_client')
        # 创建Action客户端，连接名为"my_action"的服务端
        client = actionlib.SimpleActionClient('my_action', MyActionAction)
        
        # 等待服务端启动（超时10秒，避免无限等待）
        rospy.loginfo("Waiting for action server...")
        if not client.wait_for_server(rospy.Duration(10.0)):
            rospy.logerr("Action server not found!")
            exit(1)
        
        # 创建动作目标并设置参数
        goal = MyActionGoal()
        goal.object_name = 'world'  # 给目标设置自定义参数
        
        # 发送目标并注册反馈回调
        rospy.loginfo('Sending goal...')  # 修复3：Sendinggoal → Sending goal（空格）
        client.send_goal(goal, feedback_cb=feedback_cb)
        
        # 等待动作执行完成
        client.wait_for_result()
        
        # 获取并输出执行结果
        result = client.get_result()
        if result.success:
            rospy.loginfo('Action succeeded')
        else:
            rospy.loginfo('Action failed')
    
    # 捕获ROS中断异常（如Ctrl+C）
    except rospy.ROSInterruptException:
        rospy.logerr("Client interrupted by user")
    # 捕获其他未知异常
    except Exception as e:
        rospy.logerr(f"Error occurred: {str(e)}")
