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

void test(void* p);

// 打印 dvpInt64 的函数
void printDvpInt64(const char* name, dvpInt64 value) {
    printf("Printing dvpInt64: %s\n", name);
    printf("  Value: %lld\n", value); // 使用 %lld 打印 64 位整数
}

// 打印 dvpIntDescr 的函数
void printDvpIntDescr(const char* name, const dvpIntDescr* descr) {
    if (!descr) {
        printf("Invalid dvpIntDescr pointer\n");
        return;
    }
    printf("Printing dvpIntDescr: %s\n", name);
    printf("  Step: %d\n", descr->iStep);
    printf("  Min: %d\n", descr->iMin);
    printf("  Max: %d\n", descr->iMax);
    printf("  Default: %d\n", descr->iDefault);
    printf("  Reserved: ");
    for (int i = 0; i < 16; ++i) {
        printf("%02X ", descr->reserved[i]);
    }
    printf("\n");
}

bool CheckStatus(dvpStatus Status, const char* errorMsg)
{
    if (Status != DVP_STATUS_OK)
    {
        Log_Error(errorMsg, Status);
        return false;
    }
    return true;
}

int main()
{

    dvpUint32 count = 0, num = 0;
    dvpCameraInfo info[8];

    dvpRefresh(&count);


    for (int i = 0; i < (int)count; i++)
    {
        if (dvpEnum(i, &info[i]) == DVP_STATUS_OK)
        {
            printf("[%d]-Camera FriendlyName : %s\r\n", i, info[i].FriendlyName);
        }
    }

    /* 没发现设备 */
    if (count == 0)
    {
        printf("No device found!\n");
        return 0;
    }





    printf("Please enter the number of the camera you want to open: \r\n");
    scanf_s("%d", &num);


    std::thread task(test, (void*)info[num].FriendlyName);
    task.join();

    system("pause");
    return 0;

}


void test(void* p)
{

    dvpHandle h;
    dvpStatus status;
    dvpInt64 width = 0, height = 0, FrameTimeout = 0;
    dvpIntDescr w = { 0 }, H = { 0 }, FTimeout = { 0 };
    char* name = (char*)p;


    //打开相机
    CheckStatus(status = dvpOpenByName(name, OPEN_NORMAL, &h), "dvpOpenByName");

    //设置行宽
    CheckStatus(status = dvpSetIntValue(h, "Width", 4032), "dvpSetIntValue Width");
    CheckStatus(status = dvpGetIntValue(h, "Width", &width, &w), "dvpGetIntValue Width");
    printDvpInt64("width", width);
    printDvpIntDescr("width", &w);

    //设置行高
    CheckStatus(status = dvpSetIntValue(h, "Height", 1000), "dvpSetIntValue Heigh");
    CheckStatus(status = dvpGetIntValue(h, "Height", &height, &H), "dvpGetIntValue Heigh");
    printDvpInt64("height", height);
    printDvpIntDescr("height", &H);

    //设置帧超时
    CheckStatus(status = dvpSetIntValue(h, "FrameTimeout", 100), "dvpSetIntValue FrameTimeout");
    CheckStatus(status = dvpGetIntValue(h, "FrameTimeout", &FrameTimeout, &FTimeout), "dvpGetIntValue FrameTimeout");
    printDvpInt64("FrameTimeout", FrameTimeout);
    printDvpIntDescr("FrameTimeout", &FTimeout);


    /*status = dvpSetEnumValueByString(h, "PartialFrameSubmitMode", "PartialFrameOutput");
    PartialFrameSubmitMode是要设置的属性，我要怎么查找这个属性可以设置成哪些值比如"PartialFrameOutput"*/




    //设置帧触发
    bool triggerValue = false; // 栈分配的布尔变量
    bool* FrameTrigger = &triggerValue;
    
    CheckStatus(dvpSetTriggerState(h, true), "dvpSetTriggerState");
    
    status = dvpGetBoolValueSync(h, "TriggerMode", FrameTrigger);
    if (status == DVP_STATUS_OK) {
        std::cout << "FrameTrigger: " << (*FrameTrigger ? "true" : "false") << std::endl;
    }
    else {
        std::cout << "Failed to get FrameTrigger value. Status: " << status << std::endl;
    }

    //设置帧触发源为TRIGGER_SOURCE_SOFTWARE 软触发
    dvpTriggerSource TriggerSource;
    CheckStatus(status = dvpSetTriggerSource(h, TRIGGER_SOURCE_SOFTWARE), "dvpSetTriggerSource");
    CheckStatus(status = dvpGetTriggerSource(h, &TriggerSource), "dvpGetTriggerSource");
    

    printf("TriggerSource: %d", TriggerSource);


    //开启视频流
    CheckStatus(status = dvpStart(h), "dvpStart");


    int triggerCount = 0;
    Log("Please enter the number of trigger executions:");
    std::cin >> triggerCount;

    //获取图像
    dvpFrame frame;
    void* buffer;
    for (int i = 0; i < triggerCount; ++i)
    {
        
        
        
        CheckStatus(status = dvpTriggerFire(h), "dvpTriggerFire");
       
        
        
        CheckStatus(status = dvpGetFrame(h, &frame, &buffer, 5000), "dvpGetFrame");

       

        

        printf("%s, frame:%lld, timestamp:%lld, %d*%d, %dbytes, format:%d\r\n",
            name, frame.uFrameID, frame.uTimestamp, frame.iWidth, frame.iHeight, frame.uBytes, frame.format);

    }


    //关闭相机
    status = dvpClose(h);

    printf("test quit, %s, status:%d\r\n", name, status);

}
