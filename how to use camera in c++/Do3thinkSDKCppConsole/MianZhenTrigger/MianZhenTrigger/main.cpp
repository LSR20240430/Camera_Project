#include <iostream>
#include <thread>
#include <direct.h>
#include <io.h>      

#include "E:\\DO3THINK\\DVP2 SDK CN\\library\\Visual C++\\include\\DvpCamera.h"

#ifdef _M_X64
#pragma comment(lib, "E:\\DO3THINK\\DVP2 SDK CN\\library\\Visual C++\\lib\\x64\\DVPCamera64.lib")
#else
#pragma comment(lib, "E:\\DO3THINK\\DVP2 SDK CN\\library\\Visual C++\\lib\\x86\\DVPCamera.lib")
#endif

#define Log(X)   std::cout << X << std::endl
#define Log_Error(msg, err)   std::cout << msg << " failed with err: " << err << std::endl


/*只能键盘输入0或者1。输入1返回true，0返回false*/
bool GetUserChoice(const std::string& prompt)
{
    int choice;
    Log(prompt);
    std::cin >> choice;
    if (choice != 0 && choice != 1)
    {
        Log("Invalid input, exiting program.");
        exit(1); 
    }
    return choice == 1;

}


/*检查api返回值，若api调用失败，打印调用错误的api和其返回值。错误的返回值具体信息可在DvpCamera.h查看*/
bool CheckStatus(dvpStatus Status, const char* errorMsg)
{
    if (Status != DVP_STATUS_OK)
    {
        Log_Error(errorMsg, Status);
        return false;

    }
    return true;

}


/*检查目录（保存图片）是否存在，不存在则创建它*/
void createDirectory(const char* dir)
{ 
    if (_access(dir, 0) != 0) 
    {   
        _mkdir(dir);

    }

}

/*保存图片*/
void saveImage(const dvpFrame* frame, const void* pBuffer, const char* name, int index)
{
    char PicName[128];
    sprintf_s(PicName, sizeof(PicName), "pic/test-%s_pic_%d.jpg", name, index);
    if (!CheckStatus(dvpSavePicture(frame, pBuffer, PicName, 100), "dvpSavePicture"))
    {
        return;
    }
    else
    {
        printf("Image saved successfully to: %s\n", PicName);


    }

}

/*自定义一个初始化相机设置，包括曝光，模拟增益，数字增益，Roi*/
bool InitializeCameraSettings(dvpHandle h) 
{
    double SetExposure = 30000;
    float AnalogGain, SetAnalogGain = 8;
    double Exposure;
    dvpRegion Region, SetRegion = { 0,0,2100,2100 };
    float SetrGain = 0.9, SetgGain = 1, SetbGain = 0.9;
    float rGain, gGain, bGain;
    bool GetRgbGainState;

    /*设置模拟增益*/
    if (!CheckStatus(dvpSetAnalogGain(h, SetAnalogGain), "dvpSetAnalogGain")) return false;
    if (!CheckStatus(dvpGetAnalogGain(h, &AnalogGain), "dvpGetAnalogGain")) return false;
    printf("Analog Gain: %f\n", AnalogGain);

    /*设置曝光*/
    if (!CheckStatus(dvpSetExposure(h, SetExposure), "dvpSetExposure")) return false;
    if (!CheckStatus(dvpGetExposure(h, &Exposure), "dvpGetExposure")) return false;
    printf("Exposure: %f\n", Exposure);

    /*设置Roi*/
    if (!CheckStatus(dvpSetRoi(h, SetRegion), "dvpSetRoi")) return false;
    if (!CheckStatus(dvpGetRoi(h, &Region), "dvpGetRoi")) return false;
    printf("Region - X: %d, Y: %d, Width: %d, Height: %d\n", Region.X, Region.Y, Region.W, Region.H);

    /*设置数字增益使能*/
    if (!CheckStatus(dvpSetRgbGainState(h, true), "dvpSetRgbGainState")) return false;
    if (!CheckStatus(dvpGetRgbGainState(h, &GetRgbGainState), "dvpGetRgbGainState")) return false;
    printf("GetRgbGainState: %s\n", GetRgbGainState ? "true" : "false");

    /*设置数字增益*/
    if (!CheckStatus(dvpSetRgbGain(h, SetrGain, SetgGain, SetbGain), "dvpSetRgbGain")) return false;
    if (!CheckStatus(dvpGetRgbGain(h, &rGain, &gGain, &bGain), "dvpGetRgbGain")) return false;
    printf("rGain: %.2f, gGain: %.2f, bGain: %.2f\n", rGain, gGain, bGain);

    return true;
}


/*设置白平衡*/
bool AwbOperation(dvpHandle h)
{
    dvpAwbOperation SetAwbOperation = AWB_OP_CONTINUOUS;
    dvpAwbOperation GetAwbOperation;
    dvpRegion GetAwbRoi, SetAwbRoi = { 0 ,0 , 2000, 2000 };
    
    
   /*设置白平衡操作方式*/
    if (!CheckStatus(dvpSetAwbOperation(h, SetAwbOperation), "dvpSetAwbOperation"))
    {
        return false;
    }

    /*获取白平衡操作方式*/
    if (CheckStatus(dvpGetAwbOperation(h, &GetAwbOperation), "dvpGetAwbOperation"))
    {
        switch (GetAwbOperation)
        {
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
    else
    {
        return false;
    }

    /*设置白平衡Roi，以这个区域为计算，再应用到全部图片*/
    if (!CheckStatus(dvpSetAwbRoi(h, SetAwbRoi), "dvpSetAwbRoi")) return false;
    if (!CheckStatus(dvpGetAwbRoi(h, &GetAwbRoi), "dvpGetAwbRoi")) return false;
    printf(" GetAwbRoi: x:%d, y:%d, w:%d, h:%d\r\n", GetAwbRoi.X, GetAwbRoi.Y, GetAwbRoi.W, GetAwbRoi.H);

    return true;
}

/*设置自动曝光*/
bool AutoExposure1(dvpHandle h)
{
    dvpAeMode AeMode;
    dvpAeOperation GetAeOperation;
    dvpAeOperation SetAeOperation = AE_OP_CONTINUOUS;
    dvpRegion AeRoi;
    dvpRegion SetAeRoi = { 0, 0,2000,2000 };
    dvpInt32 AeTarget;
    dvpInt32 SetAeTarget = 100;

    /*设置自动曝光模式*/
    if (!CheckStatus(dvpSetAeMode(h, AE_MODE_AE_AG), "dvpSetAeMode")) return false;
    if (!CheckStatus(dvpGetAeMode(h, &AeMode), "dvpGetAeMode")) return false;
    switch (AeMode)
    {
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
    
    /*设置自动曝光操作方式*/
    if (!CheckStatus(dvpSetAeOperation(h, SetAeOperation), "dvpSetAeOperation")) return false;
    if (!CheckStatus(dvpGetAeOperation(h, &GetAeOperation), "dvpGetAeOperation")) return false;
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

    /*设置自动曝光Roi*/
    if (!CheckStatus(dvpSetAeRoi(h, SetAeRoi), "dvpSetAeRoi")) return false;
    if (!CheckStatus(dvpGetAeRoi(h, &AeRoi), "dvpGetAeRoi")) return false;
    printf(" AeRoi: x:%d, y:%d, w:%d, h:%d\r\n", AeRoi.X, AeRoi.Y, AeRoi.W, AeRoi.H);
  
    /*设置自动曝光亮度*/
    if (!CheckStatus(dvpSetAeTarget(h, SetAeTarget), "dvpSetAeTarget")) return false;
    if (!CheckStatus(dvpGetAeTarget(h, &AeTarget), "dvpGetAeTarget")) return false;
    printf("AeTarget: %d\r\n", AeTarget);

    return true;
}

/*设置图片翻转。先水平翻转再垂直翻转。设置使能后，执行翻转*/
bool Overturn1(dvpHandle h)
{
    bool GetFlipHorizontalState, SetFlipHorizontalState = true;
    bool GetFlipVerticalState, SetFlipVerticalState = true;

    /*设置水平翻转*/
    if (!CheckStatus(dvpSetFlipHorizontalState(h, SetFlipHorizontalState), "dvpSetFlipHorizontalState")) return false;
    if (!CheckStatus(dvpGetFlipHorizontalState(h, &GetFlipHorizontalState), "dvpGetFlipHorizontalState")) return false;
    printf("FlipHorizontalState: %s\n", GetFlipHorizontalState ? "true" : "false");

    /*设置垂直翻转*/
    if (!CheckStatus(dvpSetFlipVerticalState(h, SetFlipVerticalState), "dvpSetFlipVerticalState")) return false;
    if (!CheckStatus(dvpGetFlipVerticalState(h, &GetFlipVerticalState), "dvpGetFlipVerticalState")) return false;
    printf("FlipVerticalState: %s\n", GetFlipVerticalState ? "true" : "false");

    return true;
}

/*设置图片旋转。设置旋转使能后，执行旋转，顺时针旋转180°*/
bool Rotate1(dvpHandle h)
{
    bool GetRotateState, SetRotateState = true;
    bool GetRotateState1, SetRotateState1 = true;
    int RotateAngle = 2;

    /*设置图片旋转*/
    if (!CheckStatus(dvpSetRotateState(h, SetRotateState), "dvpSetRotateState")) return false;
    if (!CheckStatus(dvpGetRotateState(h, &GetRotateState), "dvpGetRotateState")) return false;
    printf(" GetRotateState: %s\n", GetRotateState ? "true" : "false");

    /*设置图像旋转方向的标志
      true代表逆时针，false顺时针，头文件注释和DemoCam3开发者模式都是反的，默认是false*/
    if (!CheckStatus(dvpSetRotateOpposite(h, SetRotateState1), "dvpSetRotateOpposite")) return false;
    if (!CheckStatus(dvpGetRotateOpposite(h, &GetRotateState1), "dvpGetRotateOpposite")) return false;
    printf(" 图像旋转方向标志： %s\n", GetRotateState1 ? "true" : "false");
    
    /*设置图片旋转180°*/
    if (!CheckStatus(dvpRotateImage(h, RotateAngle), "dvpRotateImage")) return false;

    return true;
}



/*设置触发模式，采集图像，保存图像*/
void testTrigger(const char* Name, bool isSoftTrigger)
{
    dvpHandle h;
    dvpFrame Frame;
    void* pBuffer;
    int triggerCount, Auto, AutoExposure, Overturn, Rotate;

    /*创建保存图像目录*/
    createDirectory("pic");
    do {
        /*打开相机*/
        if (!CheckStatus(dvpOpenByName(Name, OPEN_NORMAL, &h), "dvpOpenByName")) break;
        
        
        if (!InitializeCameraSettings(h) ||
            /*设置触发使能*/
            !CheckStatus(dvpSetTriggerState(h, true), "dvpSetTriggerState") ||
            /*设置触发源。硬触发为Line1*/
            !CheckStatus(dvpSetTriggerSource(h, isSoftTrigger ? TRIGGER_SOURCE_SOFTWARE : TRIGGER_SOURCE_LINE1), "dvpSetTriggerSource") ||
            /*开启视频流*/
            !CheckStatus(dvpStart(h), "dvpStart"))
        {
            break;
        }

        /*调用白平衡函数*/
        if (GetUserChoice("Enter 1 to enable Auto Adjust (白平衡), 0 to disable:"))
        {
            if (!AwbOperation(h))
            {
                Log("Auto Adjust failed, exiting loop.");
                break;
            }
        }


        /*调用自动曝光函数*/
        if (GetUserChoice("Enter 1 to enable Auto Exposure, 0 to disable:"))
        {
            if (!AutoExposure1(h))
            {
                Log("Auto Exposure failed, exiting loop.");
                break;
            }
        }

        /*调用翻转函数*/
        if (GetUserChoice("Enter 1 for Horizontal and Vertical Flip, 0 to skip:"))
        {
            if (!Overturn1(h))
            {
                Log("Horizontal and Vertical Flip failed, exiting loop.");
                break;
            }
        }

        /*调用旋转函数*/
        if (GetUserChoice("Enter 1 to Rotate Image, 0 to skip:"))
        {
            if (!Rotate1(h))
            {
                Log("Rotate Image failed, exiting loop.");
                break;
            }
        }

        Log("Please enter the number of trigger executions:");
        std::cin >> triggerCount;

        for (int i = 0; i < triggerCount; ++i)
        {
            if (!isSoftTrigger) 
            {
                /*设置硬触发为高电平*/
                if (!CheckStatus(dvpSetTriggerInputType(h, TRIGGER_HIGH_LEVEL), "dvpSetTriggerInputType")) {
                    break; 
                }
            }

            if (isSoftTrigger)
            {
                /*发送软触发信号*/
                if (!CheckStatus(dvpTriggerFire(h), "dvpTriggerFire")) 
                {
                    break; 
                }
            }

           /*同步采集图像*/
            if (!CheckStatus(dvpGetFrame(h, &Frame, &pBuffer, 3000), "dvpGetFrame")) 
            {
                break; 
            }

            
            printf("%s, frame:%lld, timestamp:%lld, %d*%d, %dbytes, format:%d\r\n",
                   Name, Frame.uFrameID, Frame.uTimestamp, Frame.iWidth, Frame.iHeight, Frame.uBytes, Frame.format);

            /*保存图像*/
            saveImage(&Frame, pBuffer, Name, i);

        }
        
        /*停止视频流*/
        CheckStatus(dvpStop(h), "dvpStop");
    } while (0);
   
    /*关闭相机*/
    CheckStatus(dvpClose(h), "dvpClose");
}

int main()
{


    dvpUint32 Count, Num;
    dvpCameraInfo Info[2];
    
    /*枚举相机*/
    dvpRefresh(&Count);

    if (Count == 0)
    {
        Log("No device found");
        return 1;
    }

    for (dvpUint32 i = 0; i < Count; i++)
    {
        if (dvpEnum(i, &Info[i]) == DVP_STATUS_OK)
        {
            printf("[%d]-Camera FriendlyName :%s\r\n", i, Info[i].FriendlyName);

        }
    }

    Log("Please enter the number of camera you want to open:");
    std::cin >> Num;
    if (Num >= Count)
    {
        Log("Invalid camera number.");

        return 1;
    }

    Log("Enter 0 for Soft Trigger, 1 for Hard Trigger:");
    int triggerType;
    std::cin >> triggerType;

    if (triggerType == 0 || triggerType == 1)
    {
        /*当 triggerType 为 0 时，选用 软触发 模式。
          当 triggerType 为 1 时，选用 硬触发 模式。*/
        std::thread task(testTrigger, Info[Num].FriendlyName, triggerType == 0);

        task.join();
    }
    else
    {
        Log("Invalid trigger type!Please enter 0 or 1.");

        return 1;
    }

    return 0;
}
