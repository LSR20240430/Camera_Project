#include <iostream>
#include <thread>

#include <direct.h>  // 用于 _mkdir

#include "E:\\DO3THINK\\DVP2 SDK CN\\library\\Visual C++\\include\\DvpCamera.h"

#ifdef _M_X64
#pragma comment(lib, "E:\\DO3THINK\\DVP2 SDK CN\\library\\Visual C++\\lib\\x64\\DVPCamera64.lib")
#else
#pragma comment(lib, "E:\\DO3THINK\\DVP2 SDK CN\\library\\Visual C++\\lib\\x86\\DVPCamera.lib")
#endif

#define Log(X) std::cout << X << std::endl
#define Log_Error(msg, err) std::cout << msg << " failed with err: " << err << std::endl

void test(void* p);

// 用于创建目录的函数
void createDirectory(const char* dir) {
#if defined(_WIN32) || defined(_WIN64)
    _mkdir(dir);  // Windows 环境
#else
    mkdir(dir, 0777);  // Linux/macOS
#endif
}

// 用于保存图像的函数
void saveImage(const dvpFrame* pFrame, const void* pBuffer, const char* name, int index, int quality) {
    char PicName[128];
    createDirectory("pic");  // 创建用于保存图像的目录


    sprintf_s(PicName, sizeof(PicName), "pic/test-%s_pic_%d.jpg", name, index);

    // 保存图像
    dvpStatus status = dvpSavePicture(pFrame, pBuffer, PicName, quality);
    if (status == DVP_STATUS_OK) {
        Log("Saved image to: " << PicName);
    }
    else {
        Log_Error("dvpSavePicture", status);
    }
}

int main()
{
    dvpUint32 Count, Num;
    dvpCameraInfo Info[2];
    dvpRefresh(&Count);

    if (Count > 2) {
        Count = 2;
    }

    if (Count == 0) {
        Log("No device found");
        return 0;
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

    std::thread task(test, (void*)Info[Num].FriendlyName);
    task.join();
    std::cin.get();
    return 0;
}

void test(void* p)
{
    dvpStatus Status;
    dvpHandle h;
    dvpFrame Frame;
    char* Name = (char*)p;
    void* pBuffer;

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
        bool FlipHorizontalState;
        Status = dvpGetFlipHorizontalState(h, &FlipHorizontalState);
        printf("FlipHorizontalState: %s\n", FlipHorizontalState ? "true" : "false");


        dvpUserIoInfo userIoInfo;
        
       Status =  dvpGetUserIoInfo(h, &userIoInfo);
       if (Status == 0) { // 检查状态
           printf("Input Valid: %d\n", userIoInfo.inputValid);
           printf("Output Valid: %d\n", userIoInfo.outputValid);
           printf("Reserved: ");
           for (int i = 0; i < sizeof(userIoInfo.reserved); i++) {
               printf("%02X ", (unsigned char)userIoInfo.reserved[i]);
           }
           printf("\n");
       }
       else {
           printf("Error retrieving user IO info.\n");
       }

       dvpLine line = LINE_1; // 选择引脚1
       dvpLineMode lineMode;

       // 获取引脚的 I/O 模式
        Status = dvpGetLineMode(h, line, &lineMode);

       // 打印结果
       if (Status == DVP_STATUS_OK) { // 检查状态
           printf("Line: %d, Mode: %s\n", line, lineMode == LINE_MODE_INPUT ? "Input" : "Output");
       }
       else {
           printf("Error retrieving line mode.\n");
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



            Status = dvpGetUserIoInfo(h, &userIoInfo);
            if (Status == 0) { // 检查状态
                printf("Input Valid: %d\n", userIoInfo.inputValid);
                printf("Output Valid: %d\n", userIoInfo.outputValid);
                printf("Reserved: ");
                for (int i = 0; i < sizeof(userIoInfo.reserved); i++) {
                    printf("%02X ", (unsigned char)userIoInfo.reserved[i]);
                }
                printf("\n");
            }
            else {
                printf("Error retrieving user IO info.\n");
            }

            saveImage(&Frame, pBuffer, Name, i, 90);
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