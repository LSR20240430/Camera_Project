#include <iostream>
#include <thread>
#include <direct.h>  

#include "E:\\DO3THINK\\DVP2 SDK CN\\library\\Visual C++\\include\\DvpCamera.h"

#ifdef _M_X64
#pragma comment(lib, "E:\\DO3THINK\\DVP2 SDK CN\\library\\Visual C++\\lib\\x64\\DVPCamera64.lib")
#else
#pragma comment(lib, "E:\\DO3THINK\\DVP2 SDK CN\\library\\Visual C++\\lib\\x86\\DVPCamera.lib")
#endif

#define Log(X) std::cout << X << std::endl
#define Log_Error(msg, err) std::cout << msg << " failed with err: " << err << std::endl

void createDirectory(const char* dir) {
    _mkdir(dir);  
}

void saveImage(const dvpFrame* frame, const void* pBuffer, const char* name, int index) {
    char PicName[128];
    sprintf_s(PicName, sizeof(PicName), "pic/test-%s_pic_%d.jpg", name, index);

    
    dvpStatus status = dvpSavePicture(frame, pBuffer, PicName, 90); 
    if (status == DVP_STATUS_OK) {
        Log("Image saved successfully to: " << PicName);
    }
    else {
        Log_Error("Failed to save image", status);
    }
}

void testSoftTrigger(const char* name) {
    dvpStatus Status;
    dvpHandle h;
    dvpFrame Frame;
    void* pBuffer;

    createDirectory("pic"); 

    do {
        Status = dvpOpenByName(name, OPEN_NORMAL, &h);
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
                name,
                Frame.uFrameID,
                Frame.uTimestamp,
                Frame.iWidth,
                Frame.iHeight,
                Frame.uBytes,
                Frame.format);

            saveImage(&Frame, pBuffer, name, i);
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
    dvpUint32 Count;
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
            printf("[%d]-Camera FriendlyName :%s\r\n", i, Info[i].FriendlyName);
        }
    }

    Log("Enter 0 for Soft Trigger, 1 for Hard Trigger:");
    int triggerType;
    std::cin >> triggerType;

    std::thread task1, task2;

    if (triggerType == 0) {
        if (Count > 0) {
            task1 = std::thread(testSoftTrigger, Info[0].FriendlyName);
        }
        if (Count > 1) {
            task2 = std::thread(testSoftTrigger, Info[1].FriendlyName);
        }
    }
    else {
        Log("Invalid trigger type.");
        return 1;
    }
    
    if (task1.joinable()) task1.join();
    if (task2.joinable()) task2.join();
    
    std::cin.get();
    return 0;
}