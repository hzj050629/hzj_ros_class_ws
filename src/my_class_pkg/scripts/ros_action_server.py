#!/usr/bin/env python3
import rospy
import actionlib
# 修复：第4行语法错误，补全from...import完整语法
from my_class_pkg.msg import MyActionAction, MyActionGoal, MyActionResult, MyActionFeedback

class MyActionServer(object):
    # 修复：def和__init__之间加空格
    def __init__(self, name):
        self._action_name = name
        # 修复：换行后保持4空格缩进，代码更清晰
        self._as = actionlib.SimpleActionServer(
            self._action_name, 
            MyActionAction,
            execute_cb=self.executeCB,
            auto_start=False
        )
        self._as.start()
        self._feedback = MyActionFeedback()
        self._result = MyActionResult()

    def executeCB(self, goal):
        r = rospy.Rate(1)  # 1Hz频率循环
        success = True

        # 执行动作：模拟进度从10%到100%
        # 修复：for循环内代码统一缩进4空格
        for i in range(1, 11):
            # 检查是否被客户端抢占
            if self._as.is_preempt_requested():
                rospy.loginfo('{}: Preempted'.format(self._action_name))
                self._as.set_preempted()
                success = False
                break  # 抢占后退出循环
            
            # 更新进度并发布反馈
            self._feedback.progress = i * 10
            rospy.loginfo('{}: Executing, progress = {}%'.format(self._action_name, self._feedback.progress))
            self._as.publish_feedback(self._feedback)
            r.sleep()  # 按照1Hz频率休眠

        # 发送最终结果
        if success:
            self._result.success = True
            # 修复：拼写错误（r ospy → rospy）
            rospy.loginfo('{}: Succeeded'.format(self._action_name))
            self._as.set_succeeded(self._result)

if __name__ == '__main__':
    rospy.init_node('my_server')
    server = MyActionServer('my_action')
    rospy.spin()
