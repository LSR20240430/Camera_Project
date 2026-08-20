#include <iostream>
#include <thread>

/*这个头文件提供了 Windows 特有的文件和目录操作函数，如 _mkdir()。在你的代码中，确实有调用 _mkdir() 来创建目录 
(createDirectory() 函数中)。因此，这个头文件是必须保留的，因为它提供了该函数。*/
#include <direct.h>
/*这个头文件提供了与文件和目录访问相关的函数，如 _access()，用于检查目录是否存在。在你的代码中，你使用了 _access() 来检查目录是否存在(createDirectory() 函数中)。
因此，这个头文件也是必须的，因为它提供了文件检查的功能。*/
#include <io.h>      
/*#include <memory>
用于智能指针管理的，可以去掉*/

#include "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\include\\DvpCamera.h"
#include "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\include\\dvpir.h"
#include "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\include\\dvpParam.h"

#ifdef _M_X64
#pragma comment(lib, "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\lib\\x64\\DVPCamera64.lib")
#else
#pragma comment(lib, "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\lib\\x86\\DVPCamera.lib")
#endif

#define Log(X)   std::cout << X << std::endl
#define Log_Error(msg, err)   std::cout << msg << " failed with err: " << err << std::endl


/*const std::string& prompt 表示一个对std::string类型常量的引用，& 表示传引用，传引用意味着 prompt 并不是
传入字符串的副本，而是传入实际字符串的引用（即地址）。在字符串很长时，避免再拷贝一次，传引用的效率远大于传值*/
bool GetUserChoice(const std::string& prompt)
{
    int choice;
    Log(prompt);
    std::cin >> choice;//等待用户输入一个值，并赋给choice
    if (choice != 0 && choice != 1)
    {
        Log("Invalid input, exiting program.");
        exit(1); // 非法输入，退出程序
    }
    return choice == 1;//判断用户输出的值是否等于1，1返回真，0返回假
}


/*检查api返回值。若api调用失败，打印调用错误的api和其返回值。错误的返回值具体信息可在DvpCamera.h查看*/
bool CheckStatus(dvpStatus Status, const char* errorMsg) 
{
    if (Status != DVP_STATUS_OK) 
    {
        Log_Error(errorMsg, Status);
        return false;
    }
    return true;
}
/*检查目录是否存在，不存在则创建它*/
void createDirectory(const char* dir) 
{
    /*如果 dir 目录存在，_access 返回 0，表示目录已经存在。如果 dir 目录不存在，_access 返回非 0 值，表示目录不存在。*/
    if (_access(dir, 0) != 0) //检查dir目录是否存在，如果存在
    {
        /*_mkdir 是一个创建目录的函数，通常用于 Windows 环境（在 Linux 上类似的函数是 mkdir）。
        如果目录创建成功，_mkdir 返回 0。
        如果创建失败（例如，目录已经存在或路径无效），则会返回 -1，并设置 errno 来指示错误类型。*/
        _mkdir(dir);
    }
}

/*保存图片*/
void saveImage(const dvpFrame* frame, const void* pBuffer, const char* name, int index)
{
    char PicName[128];
    /*这里使用 sprintf_s 函数生成文件名，并将其存储到 PicName 字符数组中。文件名的格式为：pic / test - {name}_pic_{index}.jpg
    例如，如果 name 是 "sample" 且 index 是 1，生成的文件名将是 pic/test-sample_pic_1.jpg。*/
    sprintf_s(PicName, sizeof(PicName), "pic/test-%s_pic_%d.jpg", name, index);
   
    /*std::string PicName = "pic/test-" + name + "_pic_" + std::to_string(index) + ".jpg"; 
    使用std:string c++标准库11的方式存入数组名*/
    if(!CheckStatus(dvpSavePicture(frame, pBuffer, PicName, 100), "dvpSavePicture"))
    {
        return;
    }
    else
    {
        printf("Image saved successfully to: %s\n", PicName);

    }

    /*在标准 C++ 中，<< 常用于向输出流（如 std::cout）写入数据，std::cout，它能够接受多个输入并将它们拼接成一条日志信息。
    "Image saved successfully to: " 是一个字符串常量。
    << PicName 将 PicName 的内容追加到这个字符串后面，使日志内容包含具体的文件名。
    << 的作用就是逐步拼接多个输出项，将它们作为一个整体传递给 Log 函数*/
       
    
}

/*自定义一个初始化相机设置，包括曝光，模拟增益，数字增益，*/
bool InitializeCameraSettings(dvpHandle h){                                       
    double SetExposure = 4000;                                         
    float AnalogGain , SetAnalogGain =8;
    double Exposure;
    dvpRegion Region, SetRegion = {0,0,640,480};
    float SetrGain = 0.9, SetgGain = 1, SetbGain = 0.9;                                                                              
    float rGain, gGain, bGain;                                                  
    bool GetRgbGainState;

    /*dvpSetAnalogGain在前，dvpGetAnalogGain在后，它们的传入参数可用作同一个，dvpGetAnaloGain会刷新传入的参数，把这时候相机的AnaloGain
    赋值给被传入参数*/
    if (!CheckStatus(dvpSetAnalogGain(h, SetAnalogGain), "dvpSetAnalogGain")) return false;
    if (!CheckStatus(dvpGetAnalogGain(h, &AnalogGain), "dvpGetAnalogGain")) return false;
    printf("Analog Gain: %f\n", AnalogGain);

    if (!CheckStatus(dvpSetExposure(h, SetExposure), "dvpSetExposure")) return false;
    if (!CheckStatus(dvpGetExposure(h, &Exposure), "dvpGetExposure")) return false;
    printf("Exposure: %f\n", Exposure);

    if (!CheckStatus(dvpSetRoi(h, SetRegion), "dvpSetRoi")) return false;
    if (!CheckStatus(dvpGetRoi(h, &Region), "dvpGetRoi")) return false;
    printf("Region - X: %d, Y: %d, Width: %d, Height: %d\n", Region.X, Region.Y, Region.W, Region.H);
    

    if (!CheckStatus(dvpSetRgbGainState(h, true), "dvpSetRgbGainState")) return false;
    if (!CheckStatus(dvpGetRgbGainState(h, &GetRgbGainState), "dvpGetRgbGainState")) return false;
    printf("GetRgbGainState: %s\n", GetRgbGainState ? "true" : "false");

    if (!CheckStatus(dvpSetRgbGain(h, SetrGain, SetgGain, SetbGain), "dvpSetRgbGain")) return false;
    if (!CheckStatus(dvpGetRgbGain(h, &rGain, &gGain, &bGain), "dvpGetRgbGain")) return false;
    printf("rGain: %.2f, gGain: %.2f, bGain: %.2f\n", rGain, gGain, bGain);

    return true;
}

/*设置白平衡*/
bool AutoAdjust(dvpHandle h)
{
    dvpAwbOperation SetAwbOperation = AWB_OP_CONTINUOUS;
    dvpAwbOperation GetAwbOperation;
    dvpRegion GetAwbRoi , SetAwbRoi = {0 ,0 , 2000, 2000};
    /*白平衡和自动曝光的Roi没什么作用，无法做到让指定区域白平衡或者自动曝光
      即使设定了白平衡和自动曝光Roi，也是在全分辨率或者dvpSetRoi设置后的Roi区域生效*/
    if (!CheckStatus(dvpSetAwbOperation(h, SetAwbOperation), "dvpSetAwbOperation")) 
    {
        return false;
    }

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
     
    if (!CheckStatus(dvpSetAwbRoi(h, SetAwbRoi), "dvpSetAwbRoi")) return false;
    if (!CheckStatus(dvpGetAwbRoi(h, &GetAwbRoi), "dvpGetAwbRoi")) return false;
    printf(" GetAwbRoi: x:%d, y:%d, w:%d, h:%d\r\n", GetAwbRoi.X, GetAwbRoi.Y, GetAwbRoi.W, GetAwbRoi.H);
    
    return true;
}
bool AutoExposure1(dvpHandle h)
{
    dvpAeMode AeMode;
    dvpAeOperation GetAeOperation ;
    dvpAeOperation SetAeOperation = AE_OP_CONTINUOUS;
    dvpRegion AeRoi;
    dvpRegion SetAeRoi = { 0, 0,2000,2000 };
    dvpInt32 AeTarget;
    dvpInt32 SetAeTarget = 100;
    
    if (!CheckStatus(dvpSetAeMode(h, AE_MODE_AE_AG), "dvpSetAeMode")) return false ;
    if (!CheckStatus(dvpGetAeMode(h, &AeMode), "dvpGetAeMode")) return false ;
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

    if (!CheckStatus(dvpSetAeRoi(h, SetAeRoi), "dvpSetAeRoi")) return false;
    if (!CheckStatus(dvpGetAeRoi(h, &AeRoi), "dvpGetAeRoi")) return false;
    printf(" AeRoi: x:%d, y:%d, w:%d, h:%d\r\n", AeRoi.X, AeRoi.Y, AeRoi.W, AeRoi.H);
    /*经测试偶尔会出现Bug，若调用dvpSetRoi,则即使调用dvpSetAeRoi，dvpGetAeRoi获取的ROI区域仍是dvpSetRoi;若不调用dvpSetRoi，则dvpSetAeRoi
    和dvpGetAeRoi
    */

    if (!CheckStatus(dvpSetAeTarget(h, SetAeTarget), "dvpSetAeTarget")) return false;
    if (!CheckStatus(dvpGetAeTarget(h, &AeTarget), "dvpGetAeTarget")) return false;
    printf("AeTarget: %d\r\n", AeTarget);

    return true;
}

bool Overturn1(dvpHandle h)
{
    bool GetFlipHorizontalState , SetFlipHorizontalState = true;
    bool GetFlipVerticalState, SetFlipVerticalState = true;
    
    if (!CheckStatus(dvpSetFlipHorizontalState(h, SetFlipHorizontalState), "dvpSetFlipHorizontalState")) return false;
    if (!CheckStatus(dvpGetFlipHorizontalState(h, &GetFlipHorizontalState), "dvpGetFlipHorizontalState")) return false;
    printf("FlipHorizontalState: %s\n", GetFlipHorizontalState ? "true" : "false");

    if (!CheckStatus(dvpSetFlipVerticalState(h, SetFlipVerticalState), "dvpSetFlipVerticalState")) return false;
    if (!CheckStatus(dvpGetFlipVerticalState(h, &GetFlipVerticalState), "dvpGetFlipVerticalState")) return false;
    printf("FlipVerticalState: %s\n", GetFlipVerticalState ? "true" : "false");

    return true;
}

bool Rotate1(dvpHandle h)
{
    bool GetRotateState, SetRotateState = true;
    bool GetRotateState1, SetRotateState1 = true;
    int RotateAngle = 2;

    if (!CheckStatus(dvpSetRotateState(h, SetRotateState), "dvpSetRotateState")) return false;
    if (!CheckStatus(dvpGetRotateState(h, &GetRotateState), "dvpGetRotateState")) return false;
    printf(" GetRotateState: %s\n", GetRotateState ? "true" : "false");

    if (!CheckStatus(dvpSetRotateOpposite(h, SetRotateState1), "dvpSetRotateOpposite")) return false;
    if (!CheckStatus(dvpGetRotateOpposite(h, &GetRotateState1), "dvpGetRotateOpposite")) return false;
    printf(" 图像旋转方向标志： %s\n", GetRotateState1 ? "true" : "false");
    //图像旋转方向的标志，true代表逆时针，false顺时针，头文件注释和软件开发手册都是反的，默认是false

    if (!CheckStatus(dvpRotateImage(h, RotateAngle), "dvpRotateImage")) return false;

    return true;
}



void testTrigger(const char* Name, bool isSoftTrigger) 
{
    dvpHandle h;
    dvpFrame Frame;
    void* pBuffer;
    int triggerCount , Auto , AutoExposure , Overturn , Rotate;
    
    createDirectory("pic");
    do {
        if (!CheckStatus(dvpOpenByName(Name, OPEN_NORMAL, &h), "dvpOpenByName")) break;

        if (!InitializeCameraSettings(h) ||
            !CheckStatus(dvpSetTriggerState(h, true), "dvpSetTriggerState") ||
            /*isSoftTrigger ? TRIGGER_SOURCE_SOFTWARE : TRIGGER_SOURCE_LINE1：
             这是一个三元条件运算符，根据 isSoftTrigger 的值选择触发源。
             isSoftTrigger 为 true 时，选择 TRIGGER_SOURCE_SOFTWARE（软触发模式）。
             isSoftTrigger 为 false 时，选择 TRIGGER_SOURCE_LINE1（硬触发模式，通常通过外部触发线来控制采集）。*/
            !CheckStatus(dvpSetTriggerSource(h, isSoftTrigger ? TRIGGER_SOURCE_SOFTWARE : TRIGGER_SOURCE_LINE1), "dvpSetTriggerSource") ||
            !CheckStatus(dvpStart(h), "dvpStart"))
        {
            break;
        }

        if (GetUserChoice("Enter 1 to enable Auto Adjust (白平衡), 0 to disable:"))
        {
            if (!AutoAdjust(h)) 
            {
                Log("Auto Adjust failed, exiting loop.");
                break; 
            }
        }
       


        if (GetUserChoice("Enter 1 to enable Auto Exposure, 0 to disable:"))
        {
            if (!AutoExposure1(h))
            {
                Log("Auto Exposure failed, exiting loop.");
                break;
            }
        }

        if (GetUserChoice("Enter 1 for Horizontal and Vertical Flip, 0 to skip:"))
        {
            if (!Overturn1(h))
            {
                Log("Horizontal and Vertical Flip failed, exiting loop.");
                break;
            }
        }

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
            // 硬触发模式下，设置触发输入类型
            if (!isSoftTrigger) {
                // 硬触发模式下，需要设置触发输入类型
                if (!CheckStatus(dvpSetTriggerInputType(h, TRIGGER_HIGH_LEVEL), "dvpSetTriggerInputType")) {
                    break; // 如果设置触发输入类型失败，退出循环
                }
            }

            // 软触发模式下，发送触发信号
            if (isSoftTrigger) {
                // 软触发模式下，发送触发信号
                if (!CheckStatus(dvpTriggerFire(h), "dvpTriggerFire")) {
                    break; // 如果发送触发信号失败，退出循环
                }
            }

            // 获取图像帧
            if (!CheckStatus(dvpGetFrame(h, &Frame, &pBuffer, 2000), "dvpGetFrame")) {
                break; // 如果获取图像失败，退出循环
            }

            // 打印当前帧的相关信息
            printf("%s, frame:%lld, timestamp:%lld, %d*%d, %dbytes, format:%d\r\n",
                Name, Frame.uFrameID, Frame.uTimestamp, Frame.iWidth, Frame.iHeight, Frame.uBytes, Frame.format);

            // 保存图像
            saveImage(&Frame, pBuffer, Name, i);
        }

        CheckStatus(dvpStop(h), "dvpStop");
    } while (0);
   

    
    CheckStatus(dvpClose(h), "dvpClose");
}

int main() 
{


    dvpUint32 Count, Num;
    dvpCameraInfo Info[2];
    dvpRefresh(&Count);/*枚举相机。Count = 0 作为行参，在这个api内部Count被刷新为电脑连上相机的个数*/



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
        /*当 triggerType 为 0 时，该表达式结果为 true，表示选用 软触发 模式。
          当 triggerType 为 1 时，该表达式结果为 false，表示选用 硬触发 模式。*/
        std::thread task(testTrigger, Info[Num].FriendlyName, triggerType == 0);
        
        /*join() 是 std::thread 类的一个成员函数，它会阻塞主线程，直到调用它的线程（即 task）运行完毕。
          这样可以确保 testTrigger 函数的任务完成后，主线程才继续往下执行。
          通过 join()，可以避免多线程运行时可能产生的资源冲突或未完成的任务等问题，确保线程的生命周期和数据安全。*/
        task.join();
    }
    else 
    {
        Log("Invalid trigger type!Please enter 0 or 1.");
        return 1;
    }

    
    

    return 0;
}
