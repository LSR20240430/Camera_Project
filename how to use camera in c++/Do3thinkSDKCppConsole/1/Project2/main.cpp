#include <iostream>
#include <thread>
#include <direct.h>
#include <io.h>      
#include <memory>

#include "E:\\DO3THINK\\DVP2 SDK CN\\library\\Visual C++\\include\\DvpCamera.h"

#ifdef _M_X64
#pragma comment(lib, "E:\\DO3THINK\\DVP2 SDK CN\\library\\Visual C++\\lib\\x64\\DVPCamera64.lib")
#else
#pragma comment(lib, "E:\\DO3THINK\\DVP2 SDK CN\\library\\Visual C++\\lib\\x86\\DVPCamera.lib")
#endif

#define Log(X) std::cout << X << std::endl
#define Log_Error(msg, err) std::cout << msg << " failed with err: " << err << std::endl

bool CheckStatus(dvpStatus status, const char* errorMsg) {
    if (status != DVP_STATUS_OK) {
        Log_Error(errorMsg, status);
        return false;
    }
    return true;
}

void createDirectory(const char* dir) {
    if (_access(dir, 0) != 0) {
        _mkdir(dir);
    }
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

bool InitializeCameraSettings(dvpHandle h) {
    dvpRegion SetRegion = { 0, 0, 640, 480 };
    float SetAnalogGain = 8.0f;
    double SetExposure = 10000.0f;
    float AnalogGain;
    double Exposure;
    dvpRegion Region;
    float SetrGain = 0.9f, SetgGain = 1.0f, SetbGain = 0.9f;
    float rGain, gGain, bGain;


    if (!CheckStatus(dvpSetAnalogGain(h, SetAnalogGain), "dvpSetAnalogGain")) return false;
    if (!CheckStatus(dvpGetAnalogGain(h, &AnalogGain), "dvpGetAnalogGain")) return false;
    printf("Analog Gain: %f\n", AnalogGain);

    if (!CheckStatus(dvpSetExposure(h, SetExposure), "dvpSetExposure")) return false;
    if (!CheckStatus(dvpGetExposure(h, &Exposure), "dvpGetExposure")) return false;
    printf("Exposure: %f\n", Exposure);

    if (!CheckStatus(dvpSetRoi(h, SetRegion), "dvpSetRoi")) return false;
    if (!CheckStatus(dvpGetRoi(h, &Region), "dvpGetRoi")) return false;
    printf("Region - X: %d, Y: %d, Width: %d, Height: %d\n", Region.X, Region.Y, Region.W, Region.H);

    if (!CheckStatus(dvpSetRgbGain(h, SetrGain, SetgGain, SetbGain), "dvpSetRgbGain")) return false;
    if (!CheckStatus(dvpGetRgbGain(h, &rGain, &gGain, &bGain), "dvpGetRgbGain")) return false;
    printf("rGain: %.2f, gGain: %.2f, bGain: %.2f\n", rGain, gGain, bGain);

    return true;
}

void testTrigger(const char* Name, bool isSoftTrigger) {
    dvpHandle h;
    dvpFrame Frame;
    void* pBuffer;
    int triggerCount;
    dvpAwbOperation SetAwbOperation = AWB_OP_CONTINUOUS;
    dvpAwbOperation GetAwbOperation;
    createDirectory("pic");

    if (!CheckStatus(dvpOpenByName(Name, OPEN_NORMAL, &h), "dvpOpenByName")) return;

    if (!InitializeCameraSettings(h) ||
        !CheckStatus(dvpSetTriggerState(h, true), "dvpSetTriggerState") ||
        !CheckStatus(dvpSetTriggerSource(h, isSoftTrigger ? TRIGGER_SOURCE_SOFTWARE : TRIGGER_SOURCE_LINE1), "dvpSetTriggerSource") ||
        !CheckStatus(dvpStart(h), "dvpStart")) {
        dvpClose(h);
        return;
    }
    if (!CheckStatus(dvpSetAwbOperation(h, SetAwbOperation), "dvpSetAwbOperation")) return ;
    if (CheckStatus(dvpGetAwbOperation(h, &GetAwbOperation), "dvpGetAwbOperation")) {
        switch (GetAwbOperation) {
        case AWB_OP_OFF:
            printf("AwbOperation: OFF\n");
            break;
        case AWB_OP_ONCE:
            printf("AwbOperation: ONCE\n");
            break;
        case AWB_OP_CONTINUOUS:
            printf("AwbOperation: CONTINUOUS\n");
            break;
        default:
            printf("AwbOperation: Unknown(%d)\n", GetAwbOperation);
            break;
        }
    }

    Log("Please enter the number of trigger executions:");
    std::cin >> triggerCount;

    for (int i = 0; i < triggerCount; ++i) {
        if (!isSoftTrigger && !CheckStatus(dvpSetTriggerInputType(h, TRIGGER_HIGH_LEVEL), "dvpSetTriggerInputType")) break;
        if (!CheckStatus(dvpTriggerFire(h), "dvpTriggerFire")) break;
        if (!CheckStatus(dvpGetFrame(h, &Frame, &pBuffer,100000), "dvpGetFrame")) break;

        printf("%s, frame:%lld, timestamp:%lld, %d*%d, %dbytes, format:%d\r\n",
            Name, Frame.uFrameID, Frame.uTimestamp, Frame.iWidth, Frame.iHeight, Frame.uBytes, Frame.format);

        saveImage(&Frame, pBuffer, Name, i);
       
    }

    CheckStatus(dvpStop(h), "dvpStop");
    CheckStatus(dvpClose(h), "dvpClose");
}

int main() {
    

    dvpUint32 Count, Num;
    dvpCameraInfo Info[2];
    dvpRefresh(&Count);



    if (Count == 0) {
        Log("No device found");
        return 1;
    }

    for (dvpUint32 i = 0; i < Count; i++) {
        if (dvpEnum(i, &Info[i]) == DVP_STATUS_OK) {
            printf("[%d]-Camera FriendlyName :%s\r\n", i, Info[i].FriendlyName);
        }
    }

    Log("Please enter the number of camera you want to open:");
    std::cin >> Num;
    if (Num >= Count) {
        Log("Invalid camera number.");
        return 1;
    }

    Log("Enter 0 for Soft Trigger, 1 for Hard Trigger:");
    int triggerType;
    std::cin >> triggerType;

    if (triggerType == 0 || triggerType == 1) {
        std::thread task(testTrigger, Info[Num].FriendlyName, triggerType == 0);
        task.join();
    }
    else {
        Log("Invalid trigger type.");
        return 1;
    }

    return 0;
}