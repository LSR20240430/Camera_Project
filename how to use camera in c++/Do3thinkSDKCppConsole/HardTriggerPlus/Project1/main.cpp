#define NOMINMAX

#include <iostream>
#include <thread>
#include <direct.h>
#include <atomic>
#include <mutex>
#include <string>
#include <limits>

#include "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\include\\DvpCamera.h"
#include "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\include\\dvpir.h"
#include "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\include\\dvpParam.h"

#ifdef _M_X64
#pragma comment(lib, "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\lib\\x64\\DVPCamera64.lib")
#else
#pragma comment(lib, "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\lib\\x86\\DVPCamera.lib")
#endif

// 修改后的Log宏定义
#define Log(msg) std::cout << msg << std::endl
#define Log_Error(msg, err) std::cout << msg << " failed with err: " << err << std::endl

// 回调上下文结构
struct CallbackContext {
    dvpHandle hCamera;
    std::string cameraName;
    std::atomic<int> frameCounter;
    std::mutex frameMutex;
    bool shouldStop;
};

void createDirectory(const char* dir) {
    _mkdir(dir);
}

void saveImage(const dvpFrame* frame, const void* pBuffer, const std::string& name, int index) {
    char PicName[128];
    sprintf_s(PicName, sizeof(PicName), "pic/test-%s_pic_%d.jpg", name.c_str(), index);

    dvpStatus status = dvpSavePicture(frame, pBuffer, PicName, 90);
    if (status == DVP_STATUS_OK) {
        std::cout << "Image saved successfully to: " << PicName << std::endl;
    }
    else {
        Log_Error("Failed to save image", status);
    }
}

// 触发事件回调
static dvpInt32 OnEventFLAGTRIGGERLAUNCH(dvpHandle handle, dvpEvent event,
    void* pContext, dvpInt32 param,
    struct dvpVariant* pVariant) {
    CallbackContext* ctx = static_cast<CallbackContext*>(pContext);
    if (!ctx || ctx->shouldStop) return -1;

    std::lock_guard<std::mutex> lock(ctx->frameMutex);

    dvpFrame Frame;
    void* pBuffer = nullptr;
    dvpStatus Status = dvpGetFrame(ctx->hCamera, &Frame, &pBuffer, 3000);

    if (Status != DVP_STATUS_OK) {
        Log_Error("dvpGetFrame in callback", Status);
        return -1;
    }

    printf("%s, frame:%lld, timestamp:%lld, %d*%d, %dbytes, format:%d\n",
        ctx->cameraName.c_str(),
        Frame.uFrameID,
        Frame.uTimestamp,
        Frame.iWidth,
        Frame.iHeight,
        Frame.uBytes,
        Frame.format);

    saveImage(&Frame, pBuffer, ctx->cameraName, ctx->frameCounter++);

    return 0;
}

void testSoftTrigger(void* p) {
    dvpStatus Status;
    dvpHandle h;
    char* Name = (char*)p;
    void* pBuffer;
    dvpFrame Frame;

    createDirectory("pic");

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

            printf("%s, frame:%lld, timestamp:%lld, %d*%d, %dbytes, format:%d\n",
                Name,
                Frame.uFrameID,
                Frame.uTimestamp,
                Frame.iWidth,
                Frame.iHeight,
                Frame.uBytes,
                Frame.format);

            saveImage(&Frame, pBuffer, Name, i);
        }

        Status = dvpStop(h);
        if (Status != DVP_STATUS_OK) {
            Log_Error("dvpStop", Status);
        }

    } while (0);

    Status = dvpClose(h);
    if (Status != DVP_STATUS_OK) {
        Log_Error("dvpClose", Status);
    }
}

void testHardTrigger(void* p) {
    dvpStatus Status;
    dvpHandle h;
    char* Name = (char*)p;

    // 创建回调上下文
    CallbackContext ctx;
    ctx.cameraName = Name;
    ctx.frameCounter = 0;
    ctx.shouldStop = false;

    createDirectory("pic");

    do {
        Status = dvpOpenByName(Name, OPEN_NORMAL, &h);
        if (Status != DVP_STATUS_OK) {
            Log_Error("dvpOpenByName", Status);
            break;
        }

        ctx.hCamera = h;

        Status = dvpSetTriggerState(h, true);
        if (Status != DVP_STATUS_OK) {
            Log_Error("dvpSetTriggerState", Status);
            break;
        }

        Status = dvpSetTriggerSource(h, TRIGGER_SOURCE_LINE1);
        if (Status != DVP_STATUS_OK) {
            Log_Error("dvpSetTriggerSource", Status);
            break;
        }

        Status = dvpRegisterEventCallback(h, OnEventFLAGTRIGGERLAUNCH,
            EVENT_FLAG_TRIGGER_LAUNCH, &ctx);
        if (Status != DVP_STATUS_OK) {
            Log_Error("dvpRegisterEventCallback", Status);
            break;
        }

        Status = dvpStart(h);
        if (Status != DVP_STATUS_OK) {
            Log_Error("dvpStart", Status);
            break;
        }

        Log("Hard trigger mode started. Press Enter to stop...");
        std::cin.ignore();  // 等待用户输入停止
        std::cin.get();

        ctx.shouldStop = true;

        Status = dvpStop(h);
        if (Status != DVP_STATUS_OK) {
            Log_Error("dvpStop", Status);
        }

    } while (0);

    Status = dvpClose(h);
    if (Status != DVP_STATUS_OK) {
        Log_Error("dvpClose", Status);
    }
}

int main() {
    dvpUint32 Count = 0;
    dvpCameraInfo Info[2];

    dvpRefresh(&Count);
    if (Count > 2) {
        Count = 2;
    }

    if (Count == 0) {
        Log("No device found");
        return 1;
    }

    for (dvpUint32 i = 0; i < Count; i++) {
        if (dvpEnum(i, &Info[i]) == DVP_STATUS_OK) {
            printf("[%d]-Camera FriendlyName :%s\n", i, Info[i].FriendlyName);
        }
    }

    if (Count > 0) {
        int Num = -1;
        while (Num < 0 || Num >= static_cast<int>(Count)) {
            Log("Please enter the number of camera you want to open (0-" + std::to_string(Count - 1) + "):");
            std::cin >> Num;
            if (std::cin.fail()) {
                std::cin.clear();
                std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
                Num = -1;
            }
        }

        int triggerType = -1;
        while (triggerType != 0 && triggerType != 1) {
            Log("Enter 0 for Soft Trigger, 1 for Hard Trigger:");
            std::cin >> triggerType;
            if (std::cin.fail()) {
                std::cin.clear();
                std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
                triggerType = -1;
            }
        }

        std::thread task;
        if (triggerType == 0) {
            task = std::thread(testSoftTrigger, (void*)Info[Num].FriendlyName);
        }
        else {
            task = std::thread(testHardTrigger, (void*)Info[Num].FriendlyName);
        }

        task.join();
    }

    Log("Program finished. Press Enter to exit...");
    std::cin.ignore();
    std::cin.get();
    return 0;
}