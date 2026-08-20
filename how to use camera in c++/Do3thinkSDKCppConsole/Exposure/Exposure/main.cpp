#include<iostream>
#include<thread>
#include <direct.h>

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
    dvpUint32 Count = 0, Num = 0;
    dvpCameraInfo Info[2];
    dvpRefresh(&Count);
    double Exposure = 0;
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

    dvpRegion Region, SetRegion;
    SetRegion.X = 0;
    SetRegion.Y = 0;
    SetRegion.W = 500;
    SetRegion.H = 400;
    dvpStatus Status;
    dvpHandle h;
    dvpFrame Frame;
    char* Name = (char*)p;
    void* pBuffer;
    double Exposure = 0;
    double SetExposure = 1000.0f;

    do {
        Status = dvpOpenByName(Name, OPEN_NORMAL, &h);
        if (Status != DVP_STATUS_OK)
        {
            Log_Error("dvpOpenByName", Status);
            break;
        }
       
        /* dvpSetRoi(h, SetRegion);
        if (Status != DVP_STATUS_OK)
        {
            Log_Error("dvpSetRoi", Status);
            break;
        }*/

        /*float exposure = 20;*/

        //Status = dvpSetConfigString(h,"TriggerMode","true");
        //Status = dvpSetFloatValue(h, " Exposure", 2000);

        /*Status = dvpSetTriggerState(h, true);
        if (Status != DVP_STATUS_OK) {
            Log_Error("dvpSetTriggerState", Status);
            break;
        }*/

        //Status = dvpSetTriggerSource(h, TRIGGER_SOURCE_SOFTWARE);
       /* if (Status != DVP_STATUS_OK)
        {
            Log_Error("dvpSetTriggerSource", Status);
            break;
        }*/


        /*Status = dvpSetExposure(h, SetExposure);
        if (Status != DVP_STATUS_OK)
        {
            Log_Error("dvpGetExposure", Status);
            break;
        }

        Status = dvpGetExposure(h, &SetExposure);
        if (Status != DVP_STATUS_OK)
        {
            Log_Error("dvpGetExposure", Status);
            break;
        }*/


        // //自动曝光模式

        dvpAeMode AeMode;
        Status = dvpSetAeMode(h, AE_MODE_AE_AG);
        if (Status != DVP_STATUS_OK)
        {
            Log_Error("dvpSetAeMode", Status);
            break;
        }




        /*Status = dvpGetAeMode(h, &AeMode);
        if (Status == DVP_STATUS_OK)
        {
            switch (AeMode) {
            case AE_MODE_AE_AG:
                printf("AeMode: AE_MODE_AE_AG (自动曝光，自动增益同时开启，曝光优先)\n");
                break;
            case AE_MODE_AG_AE:
                printf("AeMode: AE_MODE_AG_AE (自动曝光，自动增益同时开启，增益优先)\n");
                break;
            case AE_MODE_AE_ONLY:
                printf("AeMode: AE_MODE_AE_ONLY (自动曝光)\n");
                break;
            case AE_MODE_AG_ONLY:
                printf("AeMode: AE_MODE_AG_ONLY (自动增益)\n");
                break;
            default:
                printf("AeMode: Unknown (%d)\n", AeMode);
                break;
            }
        }
        else
        {
            Log_Error("dvpGetAeMode", Status);
            break;

        }*/


        //自动曝光操作方式

        dvpAeOperation GetAeOperation;


        Status = dvpSetAeOperation(h, AE_OP_CONTINUOUS);
        if (Status != DVP_STATUS_OK)
        {
            Log_Error("dvpSetAeOperation", Status);
            break;
        }
        //   
        /*Status = dvpGetAeOperation(h, &GetAeOperation);
        if (Status != DVP_STATUS_OK)
        {
            Log_Error("dvpSetAeRoi", Status);
            break;
        }
        if (Status == DVP_STATUS_OK)
        {
            switch (GetAeOperation)
            {
            case AE_OP_OFF:
                printf("GetAeOperation: AE_OP_OFF\n");
                break;
            case AE_OP_ONCE:
                printf("GetAeOperation: AE_OP_ONCE\n");
                break;
            case AE_OP_CONTINUOUS:
                printf("GetAeOperation: AE_OP_CONTINUOUS\n");
                break;
            default:
                printf("GetAeOperation: Unknown (%d)\n", GetAeOperation);
                break;
            }
        }*/


            //   
            //   
            //   //自动曝光统计区域
            //   dvpRegion AeRoi;
            //   dvpRegion SetAeRoi{ 0,0,500,500 };
            //   
            //   Status = dvpSetAeRoi(h, SetAeRoi);
            //   if (Status != DVP_STATUS_OK) {
            //       Log_Error("dvpSetAeRoi", Status);
            //       break;
            //   }
            //   
            //   
            //   Status = dvpGetAeRoi(h, &AeRoi);
            //   if (Status != DVP_STATUS_OK) {
            //       Log_Error("SetAeTarget", Status);
            //       break;
            //   }
            //   printf(" AeRoi: x:%d, y:%d, w:%d, h:%d\r\n", AeRoi.X, AeRoi.Y, AeRoi.W, AeRoi.H);
            //   
            //   
            //   dvpInt32 AeTarget;
            dvpInt32 SetAeTarget = 100;//自动曝光亮度值
            Status = dvpSetAeTarget(h, SetAeTarget);
            if (Status != DVP_STATUS_OK) {
                Log_Error("SetAeTarget", Status);
                break;
            }
            //   Status = dvpGetAeTarget(h, &AeTarget);
            //   if (Status != DVP_STATUS_OK) {
            //       Log_Error("dvpGetAeTarget", Status);
            //       break;
            //   }
            //   printf("AeTarget: %d\n", AeTarget);

            //dvpSetRoi(h, SetRegion);
            //if (Status != DVP_STATUS_OK)
            //{
            //    Log_Error("dvpSetRoi", Status);
            //    break;
            //}
            //

            /*Status = dvpGetRoi(h, &Region);
            if (Status != DVP_STATUS_OK)
            {
                Log_Error("dvpGetRoi", Status);
                break;*/
                //}

            Status = dvpStart(h);
            if (Status != DVP_STATUS_OK)
            {
                Log_Error("dvpStart", Status);
                break;
            }


            for (int i = 0; i < 10; i++)
            {
                Status = dvpTriggerFire(h);
                if (Status != DVP_STATUS_OK)
                {
                    Log_Error("dvpTriggerFire", Status);
                    break;
                }

                Status = dvpGetFrame(h, &Frame, &pBuffer, 3000);
                if (Status != DVP_STATUS_OK)
                {
                    Log_Error("dvpGetFrame", Status);
                    break;
                }

                printf("Exposure:%lf\r\n", Exposure);
                /*printf(" region: x:%d, y:%d, w:%d, h:%d\r\n", Region.X, Region.Y, Region.W, Region.H);*/
                printf("%s, frame:%lld, timestamp:%lld, %d*%d, %dbytes, format:%d\r\n",
                    Name,
                    Frame.uFrameID,
                    Frame.uTimestamp,
                    Frame.iWidth,
                    Frame.iHeight,
                    Frame.uBytes,
                    Frame.format);

                saveImage(&Frame, pBuffer, Name, i, 90);
            }

            Status = dvpStop(h);
            if (Status != DVP_STATUS_OK)
            {
                Log_Error("dvpStop", Status);
                break;
            }

        } while (0);

        Status = dvpClose(h);
        if (Status != DVP_STATUS_OK)
        {
            Log_Error("dvpClose", Status);
        }
    }

