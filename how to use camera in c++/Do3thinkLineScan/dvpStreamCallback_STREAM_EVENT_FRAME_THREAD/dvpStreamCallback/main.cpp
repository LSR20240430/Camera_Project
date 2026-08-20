#include<iostream>
#include<thread>
#include <conio.h>

#include "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\include\\DvpCamera.h"
#include "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\include\\dvpir.h"
#include "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\include\\dvpParam.h"

#ifdef _M_X64
#pragma comment(lib, "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\lib\\x64\\DVPCamera64.lib")
#else
#pragma comment(lib, "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\lib\\x86\\DVPCamera.lib")
#endif

#define Log(X) std::cout << X << std::endl
#define Log_Error(msg, err) std::cout << msg << " failed with err: " << err << std::endl



/* 视频流回调函数 */
static dvpInt32 OnGetFrame(dvpHandle handle, dvpStreamEvent event, void* pContext, dvpFrame* pFrame, void* pBuffer)
{
    
    dvpStatus status;

    /* 显示帧数和帧率 */
    dvpFrameCount framecount;

    printf("OnGetFrame\n");

    status = dvpGetFrameCount(handle, &framecount);
    if (status != DVP_STATUS_OK)
    {
        printf("get framecount failed\n");
    }

    /* 显示帧信息 */
    printf("Frame ID:%lld, timestamp:%lld, %d*%d, %dbytes, format:%d\n\r",
        pFrame->uFrameID, pFrame->uTimestamp, pFrame->iWidth,
        pFrame->iHeight, pFrame->uBytes, pFrame->format);

    printf("uFrameCount: %u, uFrameDrop: %u, uFrameIgnore: %u, uFrameError: %u, uFrameOK: %u, uFrameOut: %u, uFrameResend: %u, uFrameProc: %u, fFrameRate: %.2f, fProcRate: %.2f\n",
        framecount.uFrameCount, framecount.uFrameDrop, framecount.uFrameIgnore,
        framecount.uFrameError, framecount.uFrameOK, framecount.uFrameOut,
        framecount.uFrameResend, framecount.uFrameProc, framecount.fFrameRate,
        framecount.fProcRate);




    /* 通常返回0即可，没有特殊含义 */
    return 0;
}






/*
// 事件回调 - 断开连接事件
static dvpInt32 OnEventLostConnection(dvpHandle handle, dvpEvent event, void* pContext, dvpInt32 param, struct dvpVariant* pVariant)
{
    printf("OnEventLostConnection triggered\n");
    return 0;
}

// 事件回调 - 重连事件
static dvpInt32 OnEventReconnected(dvpHandle handle, dvpEvent event, void* pContext, dvpInt32 param, struct dvpVariant* pVariant)
{
    printf("OnEventReconnected triggered\n");
    return 0;
}
*/

void test(void* p)
{
    dvpStatus status;
    dvpHandle h;
    char* name = (char*)p;

    do
    {
        // 打开设备
        status = dvpOpenByName(name, OPEN_NORMAL, &h);
        if (status != DVP_STATUS_OK)
        {
            printf("dvpOpenByName failed with err:%d\r\n", status);
            break;
        }

       


        // 设置为连续模式
        status = dvpSetTriggerState(h, false);
        if (status != DVP_STATUS_OK)
        {
            printf("dvpSetTriggerState failed with err:%d\r\n", status);
            break;
        }

        /*2025.2.13 在测试视频流
        // 在打开相机之后，开启视频流之前注册事件回调函数
        status = dvpRegisterEventCallback(h, OnEventLostConnection, EVENT_LOST_CONNECTION, NULL);
        if (status != DVP_STATUS_OK)
        {
            printf("dvpRegisterEventCallback OnEventLostConnection failed with err:%d\r\n", status);
            break;
        }

        status = dvpRegisterEventCallback(h, OnEventReconnected, EVENT_RECONNECTED, NULL);
        if (status != DVP_STATUS_OK)
        {
            printf("dvpRegisterEventCallback OnEventReconnected failed with err:%d\r\n", status);
            break;
        }
        */

        ///* 在打开相机之后，开启视频流之前注册事件回调函数，启动一个专门的线程以dvpGetFrame（同步采集）的方式获取图像 */
        //status = dvpRegisterStreamCallback(h, OnGetFrame, STREAM_EVENT_FRAME_THREAD, NULL);
        //if (status != DVP_STATUS_OK)
        //{
        //    printf("dvpRegisterStreamCallback failed with err:%d\r\n", status);
        //    break;
        //}


        // 注册视频流回调函数，以独立的线程开启dvpGetFrame采集图像;这个打印帧率只能在回调函数中打印
        
        status = dvpRegisterStreamCallback(h, OnGetFrame, STREAM_EVENT_FRAME_THREAD, NULL);
        
        
        


        
        
        status = dvpStart(h);
        /* 主线程等待 */
        int key;
        printf("\nPress ESC to exit!\n");
        
       
        

        
        while (1)
        {
            //getch会阻塞主线程
            key = _getch();
            if (key == 27)
            {
                /* 关闭视频流 */
                status = dvpStop(h);
                if (status != DVP_STATUS_OK)
                {
                    break;
                }
                break;
            }
        }


       

    } while (0);
    // 关闭设备

    status = dvpClose(h);
    if (status != DVP_STATUS_OK)
    {
        printf("dvpClose failed with err:%d\r\n", status);

    }

}

int main()
{


    dvpCamera camera;


    printf("start...\r\n");

    dvpUint32 count = 0, num = -1;
    dvpCameraInfo info[8];

    // 枚举设备
    dvpRefresh(&count);
    if (count > 8)
        count = 8;

    for (int i = 0; i < (int)count; i++)
    {
        if (dvpEnum(i, &info[i]) == DVP_STATUS_OK)
        {
            printf("[%d]-Camera FriendlyName : %s\r\n", i, info[i].FriendlyName);
        }
    }

    // 没发现设备
    if (count == 0)
    {
        printf("No device found!\n");
        return 0;
    }

    while (num < 0 || num >= count)
    {
        printf("Please enter the number of the camera you want to open: \r\n");
        scanf_s("%d", &num);
    }

    // 启动测试线程
    std::thread task(test, (void*)info[num].FriendlyName);
    task.join();

    system("pause");
    return 0;
}

