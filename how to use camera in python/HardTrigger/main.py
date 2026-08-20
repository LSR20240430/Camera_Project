from dvp import *
import numpy as np
import cv2


def set_HardTrigger(camera):
    camera.TriggerState = True
    camera.TriggerInputType = TriggerInputType.TRIGGER_POS_EDGE
    camera.TriggerSource = TriggerSource.TRIGGER_SOURCE_LINE1




def set_exposure(camera, exposure_time: int):
    """获取当前曝光时间，设置相机曝光时间（手动模式）"""
    current_exposure = camera.Exposure  # 获取当前曝光时间
    print(f"当前曝光时间: {current_exposure} 微秒")

    camera.AeOperation = AeOperation.AE_OP_OFF  # 关闭自动曝光
    camera.Exposure = exposure_time  # 设置曝光时间（单位：微秒）
    print(f"曝光时间已设置为 {exposure_time} 微秒")

def main():
    # 获取相机列表
    camera_list = Refresh()
    if not camera_list:
        print("未找到相机设备")
        return

    # 选择相机
    for idx, cam in enumerate(camera_list):
        print(f"{idx} -> {cam.FriendlyName}")
    index = int(input("请输入相机索引号 (0,1,2...): "))
    camera = Camera(index)  # 打开相机
    print(f"已打开相机: {camera}")

    try:
        # 设置曝光并启动视频流
        camera.TriggerState = False
        set_exposure(camera, 20000)  # 设置曝光时间为1000微秒
        #set_hardtrigger(camera)
        #camera.SetConfigString("Exposure", "1000")  #DVP_STATUS_NOT_SUPPORTED



        camera.Start()  # 启动视频流
        print("视频流已启动，按 ESC 退出预览...")

        # 实时预览
        while cv2.waitKey(1) != 27:  # 按ESC退出
            try:
                frame = camera.GetFrame(2000)  # 获取帧，超时2秒
                mat = np.frombuffer(frame[1], dtype=np.uint8)  # 转换为numpy数组
                mat = mat.reshape(frame[0].iHeight, frame[0].iWidth, -1)  # 调整维度
                cv2.imshow("Camera Preview", mat)
            except dvpException as e:
                if e.Status != Status.DVP_STATUS_TIME_OUT:
                    print(f"采集帧失败: {e.Status}")
                    break
    finally:
        # 清理资源
        cv2.destroyAllWindows()
        camera.Stop()
        camera.Close()
        print("相机已关闭")


#在Python中，每个脚本变量都有一个内置的变量__name__
#如果这个脚本是直接运行的，那么__name__的值会被自动设置为"__main__"
#如果这个脚本是被其它脚本导入的，那么 __name__ 的值会被设置为脚本的文件名（不包括 .py 后缀）
# __name__ == "__main__" ，满足这个条件才会执行main()
if __name__ == "__main__":
    main()