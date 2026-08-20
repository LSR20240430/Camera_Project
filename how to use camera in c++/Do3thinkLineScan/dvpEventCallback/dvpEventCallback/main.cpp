#include<iostream>
#include<thread>


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

const int frame_count = 1000;

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

// 事件回调 — 接收到帧信号
static dvpInt32 OnEventFLAGTRIGGERLAUNCH(dvpHandle handle, dvpEvent event, void* pContext, dvpInt32 param, struct dvpVariant* pVariant)
{
    
    
    
    printf("OnEventFLAGTRIGGERLAUNCH\n");
    return 0;
}

// 事件回调 — 帧开始传输
static dvpInt32 OnEVENT_FRAME_START(dvpHandle handle, dvpEvent event, void* pContext, dvpInt32 param, struct dvpVariant* pVariant)
{
    printf("OnEVENT_FRAME_START\n");
    return 0;
}

// 事件回调 — 帧开始传输
static dvpInt32 OnEVENT_FRAME_END(dvpHandle handle, dvpEvent event, void* pContext, dvpInt32 param, struct dvpVariant* pVariant)
{
    printf("OnEVENT_FRAME_END\n");
    return 0;
}




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

        /*
        // 关闭行触发
        status = dvpSetBoolValue(h, "LineTrigEnable", false);
        if (status != DVP_STATUS_OK)
        {
            printf("dvpSetBoolValue LineTrigEnable failed with err:%d\r\n", status);
            break;
        }
        */
        //关闭帧触发
        status = dvpSetTriggerState(h, false);
        if (status != DVP_STATUS_OK)
        {
            printf("dvpSetTriggerState failed with err:%d\r\n", status);
            break;
        }
        //设置帧触发源为Line3
        status = dvpSetTriggerSource(h, TRIGGER_SOURCE_LINE3);

        /* 2025 2.12谢工测试

        // 设置为连续模式
        status = dvpSetTriggerState(h, false);
        if (status != DVP_STATUS_OK)
        {
            printf("dvpSetTriggerState failed with err:%d\r\n", status);
            break;
        }

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

        //注册回调事件——接受到帧信号
        status = dvpRegisterEventCallback(h, OnEventFLAGTRIGGERLAUNCH, EVENT_FLAG_TRIGGER_LAUNCH, NULL);
        if (status != DVP_STATUS_OK)
        {
            printf("dvpRegisterEventCallback OnEventReconnected failed with err:%d\r\n", status);
            break;
        }

        // 注册回调事件——帧开始传输
        status = dvpRegisterEventCallback(h, OnEVENT_FRAME_START, EVENT_FRAME_START, NULL);
        if (status != DVP_STATUS_OK)
        {
            printf("dvpRegisterEventCallback OnEventReconnected failed with err:%d\r\n", status);
            break;
        }

        // 注册回调事件——帧结束传输
        status = dvpRegisterEventCallback(h, OnEVENT_FRAME_END, EVENT_FRAME_END, NULL);
        if (status != DVP_STATUS_OK)
        {
            printf("dvpRegisterEventCallback OnEventReconnected failed with err:%d\r\n", status);
            break;
        }



        /* 帧信息 */
        dvpFrame frame;
        /* 帧数据首地址，用户不需要申请释放内存 */
        void* z;

        status = dvpStart(h);
        if (status = DVP_STATUS_OK)
        {

            for (int j = 0; j < frame_count; j++)
            {
                /* 当前案例没有设置相机的曝光增益等参数，只展示在默认的ROI区域显示帧信息 */
                status = dvpGetFrame(h, &frame, &z, 4000);
                if (status != DVP_STATUS_OK)
                {
                    printf("dvpGetFrame failed with err:%d\r\n", status);


                }

                /* 显示帧数和帧率 */
                dvpFrameCount framecount;
                status = dvpGetFrameCount(h, &framecount);
                if (status != DVP_STATUS_OK)
                {
                    printf("get framecount failed\n");
                }
                printf("framecount: %d, framerate: %f\n", framecount.uFrameCount, framecount.fFrameRate);

                /* 显示帧信息 */
                printf("%s, frame:%lld, timestamp:%lld, %d*%d, %dbytes, format:%d\r\n",
                    name,
                    frame.uFrameID,
                    frame.uTimestamp,
                    frame.iWidth,
                    frame.iHeight,
                    frame.uBytes,
                    frame.format);
            }

           
        }

        /* 停止视频流 */
        status = dvpStop(h);
        if (status != DVP_STATUS_OK)
        {
            printf("dvpGetFrame failed with err:%d\r\n", status);
            break;
        }

    } while (0);
    // 关闭设备

    status = dvpStop(h);
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

