# -*- coding: utf-8 -*-

# 执行 help(Camera) 可以查看相应的帮助信息
# 更多的帮助信息请参考DVPCamera.chm，并结合BasedCam的“开发者模式”
# 其中的dvpSet...和dvpGet...等函数在python都以属性赋值的形式出现
# 比如dvpGetGamma和dvpSetGamma，对应于variable = camera.Gamma和camera.Gamma = 100


from time import sleep
from dvp import *  # 将对应操作系统的dvp.pyd或dvp.so放入python安装目录下的Lib目录或者工程目录
import numpy as np  # 用pip命令安装numpy库
import cv2  # 用pip命令安装opencv-python库


def callbackFunc(pbuffer):
    print("Enter Callback")
    return 1



# 定义主函数
def main():
    cameraInfo = Refresh();  # 刷新并获取相机列表
    if (len(cameraInfo) == 0):  # 没有任何设备则退出
        print(u"没有找到设备")
        return

    for k, v in enumerate(cameraInfo):  # 打印相机索引和名称
        print(k, "->", v.FriendlyName)

    while (True):  # 循环直到打开一台相机
        try:
            str = input("请选定将要打开的相机索引号(0,1,2...):")
            index = (int)(str)  # 输入的索引号字符串转换为整数
            camera = Camera(index)  # 以索引号的方式打开相机
            # camera = Camera(cameraInfo[index].FriendlyName)#或以名称的方式打开相机
            print(camera);  # 打印相机信息
            break
        except dvpException as e:
            print(u"打开相机失败:", e.Status)  # 如果是DVP的标准异常
        except BaseException as e:
            print(u"非法的索引号:", str)  # 其他异常

    try:
        camera.TriggerState = True  # 从触发模式切换到连续出图模式
        # setCameraParams(camera)                             #设置其他的相机的参数
        status = camera.RegisterStreamCallback(callbackFunc, StreamEvent.STREAM_EVENT_FRAME_THREAD)
        print(status)
        camera.Start()  # 启动视频流
    except dvpException as e:
        print(u"操作相机出错:", e.Status)

    while (1):  # 按ESC键则退出循环
        try:
            sleep(1)
            # camera.TriggerFire = 1 #软触发一帧
            print(camera.FrameCount)  # 打印帧统计信息
            # frame = camera.GetFrame(4000)                   #从相机采集图像数据，超时时间为4000毫秒
        except dvpException as e:
            print(u"采集图像数据失败:", e.Status)
            if (e.Status == Status.DVP_STATUS_TIME_OUT):
                continue  # 如果只是超时错误，则继续采集
            break  # 其他错误则中止采集

    camera.Stop()  # 停止视频流
    camera.Close()  # 关闭相机


# 执行主函数
main()



