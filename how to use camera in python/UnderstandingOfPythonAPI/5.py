# -*- coding: utf-8 -*-

import sys
import numpy as np
import cv2
from dvp import *  # 确保 dvp.pyd 在 lib 或项目目录下

# 将帧数据转换为 numpy 矩阵
def frame2mat(frameBuffer):
    frame, buffer = frameBuffer
    bits = np.uint8 if frame.bits == Bits.BITS_8 else np.uint16
    shape = None

    if ImageFormat.FORMAT_MONO <= frame.format <= ImageFormat.FORMAT_BAYER_RG:
        shape = 1
    elif frame.format in [ImageFormat.FORMAT_BGR24, ImageFormat.FORMAT_RGB24]:
        shape = 3
    elif frame.format in [ImageFormat.FORMAT_BGR32, ImageFormat.FORMAT_RGB32]:
        shape = 4
    else:
        return None

    mat = np.frombuffer(buffer, bits)
    mat = mat.reshape(frame.iHeight, frame.iWidth, shape)
    return mat

# 主程序
def main():
    cameraInfo = Refresh()
    if not cameraInfo:
        print("No device")
        return

    for k, v in enumerate(cameraInfo):
        print(k, "->", v.FriendlyName)

    while True:
        try:
            #str_index = input("Please select the camera index number to be opened (0,1,2...): ")
            #index = int(str_index)
            camera = Camera(0)
            break
        except dvpException as e:
            print("Open camera Failed:", e.Status)
        # except ValueError:
        #     print("Invalid index number:", str_index)

    camera.Start()
    cv2.namedWindow("Image", cv2.WINDOW_NORMAL)
    cv2.resizeWindow("Image", 800, 600)

    while True:
        try:
            frame = camera.GetFrame(4000)  # 解包元组




        except dvpException as e:
            print("Acquire image data Failed:", e.Status)
            if e.Status == Status.DVP_STATUS_TIME_OUT:
                continue
            break

        mat = frame2mat(frame)
        cv2.imshow("Image", mat)

        # 用 OpenCV 检测键盘输入
        if cv2.waitKey(1) & 0xFF == ord('q'):
            print("Press 'q' to exit")
            break

    cv2.destroyAllWindows()
    camera.Stop()
    camera.Close()
    print("Camera closed.")

if __name__ == "__main__":
    main()