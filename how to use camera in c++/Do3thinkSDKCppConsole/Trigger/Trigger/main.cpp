#include <iostream>
#include <thread>
#include <direct.h>  // 用于创建目录


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

void createDirectory(const char* dir) {
    _mkdir(dir);  // 创建目录
}

void saveImage(const dvpFrame* frame, const void* pBuffer, const char* name, int index) {
    char PicName[128];
    sprintf_s(PicName, sizeof(PicName), "pic/test-%s_pic_%d.jpg", name, index);

    // 保存图像
    dvpStatus status = dvpSavePicture(frame, pBuffer, PicName, 90); // 质量设置为90
    if (status == DVP_STATUS_OK) {
        Log("Image saved successfully to: " << PicName);
    }
    else {
        Log_Error("Failed to save image", status);
    }
}

void testSoftTrigger(void* p) {
    dvpStatus Status;
    dvpHandle h;
    dvpFrame Frame;
    char* Name = (char*)p;
    void* pBuffer;

    createDirectory("pic"); // 创建保存图像的目录

    do {
        Status = dvpOpenByName(Name, OPEN_NORMAL, &h);
        if (Status != DVP_STATUS_OK) {
            Log_Error("dvpOpenByName", Status);
            break;
        }

        Status = dvpSetTriggerState(h, true);
        if (Status != DVP_STATUS_OK) {
            Log_Error("dvpSetTriggerState", Status);
            break;
        }

        Status = dvpSetTriggerSource(h, TRIGGER_SOURCE_SOFTWARE);
        if (Status != DVP_STATUS_OK) {
            Log_Error("dvpSetTriggerSource", Status);
            break;
        }

        Status = dvpStart(h);
        if (Status != DVP_STATUS_OK) {
            Log_Error("dvpStart", Status);
            break;
        }

        for (int i = 0; i < 10; i++) {
            Status = dvpTriggerFire(h);
            if (Status != DVP_STATUS_OK) {
                Log_Error("dvpTriggerFire", Status);
                break;
            }

            Status = dvpGetFrame(h, &Frame, &pBuffer, 3000);
            if (Status != DVP_STATUS_OK) {
                Log_Error("dvpGetFrame", Status);
                break;
            }

            printf("%s, frame:%lld, timestamp:%lld, %d*%d, %dbytes, format:%d\r\n",
                Name,
                Frame.uFrameID,
                Frame.uTimestamp,
                Frame.iWidth,
                Frame.iHeight,
                Frame.uBytes,
                Frame.format);

            // 保存图片
            saveImage(&Frame, pBuffer, Name, i);
        }

        Status = dvpStop(h);
        if (Status != DVP_STATUS_OK) {
            Log_Error("dvpStop", Status);
            break;
        }

    } while (0);

    Status = dvpClose(h);
    if (Status != DVP_STATUS_OK) {
        Log_Error("dvpClose", Status);
    }
}



// 事件回调 — 接收到帧信号
static dvpInt32 OnEventFLAGTRIGGERLAUNCH(dvpHandle handle, dvpEvent event, void* pContext, dvpInt32 param, struct dvpVariant* pVariant)
{
    
    printf("OnEventFLAGTRIGGERLAUNCH\n");

    return;
    
    

    



}




void testHardTrigger(void* p)
{
    dvpStatus Status;
    dvpHandle h;
    dvpFrame Frame;
    char* Name = (char*)p;
    void* pBuffer;

    createDirectory("pic"); // 创建保存图像的目录

    do {
        Status = dvpOpenByName(Name, OPEN_NORMAL, &h);
        if (Status != DVP_STATUS_OK) {
            Log_Error("dvpOpenByName", Status);
            break;
        }

        Status = dvpSetTriggerState(h, true);
        if (Status != DVP_STATUS_OK) {
            Log_Error("dvpSetTriggerState", Status);
            break;
        }

        Status = dvpSetTriggerSource(h, TRIGGER_SOURCE_LINE1); // 硬触发设置
        if (Status != DVP_STATUS_OK) {
            Log_Error("dvpSetTriggerSource", Status);
            break;
        }
        
        Status = dvpRegisterEventCallback(h, OnEventFLAGTRIGGERLAUNCH, EVENT_FLAG_TRIGGER_LAUNCH, NULL);
        if (Status != DVP_STATUS_OK)
        {
            printf("dvpRegisterEventCallback OnEventReconnected failed with err:%d\r\n", Status);
            break;
        }







        Status = dvpStart(h);
        if (Status != DVP_STATUS_OK) {
            Log_Error("dvpStart", Status);
            break;
        }

        for (int i = 0; i < 10; i++) {
            Status = dvpSetTriggerInputType(h, TRIGGER_HIGH_LEVEL); // 设置高电平触发
            if (Status != DVP_STATUS_OK) {
                Log_Error("dvpSetTriggerInputType", Status);
                break;
            }

            Status = dvpGetFrame(h, &Frame, &pBuffer, 3000);
            if (Status != DVP_STATUS_OK) {
                Log_Error("dvpGetFrame", Status);
                break;
            }

            printf("%s, frame:%lld, timestamp:%lld, %d*%d, %dbytes, format:%d\r\n",
                Name,
                Frame.uFrameID,
                Frame.uTimestamp,
                Frame.iWidth,
                Frame.iHeight,
                Frame.uBytes,
                Frame.format);

            // 保存图片
            saveImage(&Frame, pBuffer, Name, i);
        }

        Status = dvpStop(h);
        if (Status != DVP_STATUS_OK) {
            Log_Error("dvpStop", Status);
            break;
        }

    } while (0);

    Status = dvpClose(h);
    if (Status != DVP_STATUS_OK) {
        Log_Error("dvpClose", Status);
    }
}

int main() {
    dvpUint32 Count, Num;
    dvpCameraInfo Info[2];
    dvpRefresh(&Count);

    if (Count > 2) {
        Count = 2;
    }

    if (Count == 0) {
        Log("No device found");
        return 1; // 直接返回错误
    }

    for (dvpUint32 i = 0; i < Count; i++) {
        if (dvpEnum(i, &Info[i]) == DVP_STATUS_OK) {
            printf("[%d]-Camera FriendlyName :%s\r\n", i, Info[i].FriendlyName);
        }
    }

    if (Count > 0) {
        Log("Please enter the number of camera you want to open");
        scanf_s("%d", &Num);
    }

    Log("Enter 0 for Soft Trigger, 1 for Hard Trigger:");
    int triggerType;
    std::cin >> triggerType;

    std::thread task;
    if (triggerType == 0) {
        task = std::thread(testSoftTrigger, (void*)Info[Num].FriendlyName);
    }
    else if (triggerType == 1) {
        task = std::thread(testHardTrigger, (void*)Info[Num].FriendlyName);
    }
    else {
        Log("Invalid trigger type.");
        return 1;
    }

    task.join();
    std::cin.get();
    return 0;
}