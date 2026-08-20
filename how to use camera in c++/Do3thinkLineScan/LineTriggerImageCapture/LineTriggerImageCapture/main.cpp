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
    for (int i = 0; i < 16 ; ++i) {
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
   
    dvpUint32 count = 0 , num = 0;
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
    dvpInt64 width = 0 , height = 0, FrameTimeout = 0;
    dvpIntDescr w = { 0 }, H = { 0 } , FTimeout = { 0 };
    char* name = (char*)p;
    
    
    //打开相机
    CheckStatus( status = dvpOpenByName(name, OPEN_NORMAL, &h), "dvpOpenByName");
    
    //设置行宽
    CheckStatus( status = dvpSetIntValue(h, "Width", 4032), "dvpSetIntValue Width");
    CheckStatus(status = dvpGetIntValue(h, "Width", &width, &w), "dvpGetIntValue Width");
    printDvpInt64("width" , width);
    printDvpIntDescr("width" , & w);

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


    //关闭帧触发 （该项目只用行触发，不使用帧+行触发）
    CheckStatus(status = dvpSetTriggerState(h, false), "dvpSetTriggerState");
   
    //行触发使能
    bool* LineTrigEnable = nullptr;
    LineTrigEnable = new bool;
    
     
    /*该接口用rgl4k3c实测，设置LineTrigEnable时比dvpSetBoolValue好用，后者无法让LineTrigEnable为false,
    测试TriggerMode帧触发时也只能是true，无法设置为false，设置false结果变成true*/
    
    CheckStatus(status = dvpSetConfigString(h, "LineTrigEnable", "true"), "dvpSetConfigString");
    CheckStatus(status = dvpGetBoolValue(h, "LineTrigEnable", LineTrigEnable), "dvpGetBoolValue");
    
    if (LineTrigEnable != nullptr) {
        std::cout << "LineTrigEnable: " << (*LineTrigEnable ? "true" : "false") << std::endl;
    }
    else {
        std::cout << "LineTrigEnable is null!" << std::endl;
    }
    delete LineTrigEnable;

    //行触发源
    //char* LineTrigSource = new char[100]; 
    /*为该指针分配内存后不报错*/
    CheckStatus(status = dvpSetEnumValueByString(h, "LineTrigSource", "EncoderB"), "dvpSetEnumValueByString： LineTrigSource");
    CheckStatus(status = dvpGetEnumValueByString(h, "LineTrigSource", LineTrigSource), "dvpGetEnumValueByString： LineTrigSource");


    if (LineTrigSource != nullptr) {
        std::cout << "LineTrigSource: " << LineTrigSource << std::endl;
    }
    else {
        std::cout << "LineTrigSource is null!" << std::endl;
    }


    //编码器方向
    char* EncoderDirection = new char[100];
    status = dvpSetEnumValueByString(h, "EncoderDirection", "AnyDirection");
    status = dvpGetEnumValueByString(h, "EncoderDirection", EncoderDirection);
    if (EncoderDirection != nullptr) {
        std::cout << "EncoderDirection: " << EncoderDirection << std::endl;
    }
    else {
        std::cout << "EncoderDirection is null!" << std::endl;
    }
    
    //编码器方向信号源
    char* EncoderDirSource = new char[100];
    status = dvpSetEnumValueByString(h, "EncoderDirSource", "Encoder");
    status = dvpGetEnumValueByString(h, "EncoderDirSource", EncoderDirSource);
    if (EncoderDirSource != nullptr) {
        std::cout << "EncoderDirSource: " << EncoderDirSource << std::endl;
    }
    else {
        std::cout << "EncoderDirSource is null!" << std::endl;
    }

    //编码器信号源A
    char* EncoderSourceA = new char[100];
    status = dvpSetEnumValueByString(h, "EncoderSourceA", "Line1");
    status = dvpGetEnumValueByString(h, "EncoderSourceA", EncoderSourceA);
    if (EncoderSourceA != nullptr) {
        std::cout << "EncoderSourceA: " << EncoderSourceA << std::endl;
    }
    else {
        std::cout << "EncoderSourceA is null!" << std::endl;
    }

    //编码器信号源B
    char* EncoderSourceB = new char[100];
    status = dvpSetEnumValueByString(h, "EncoderSourceB", "Line2");
    status = dvpGetEnumValueByString(h, "EncoderSourceB", EncoderSourceB);
    if (EncoderSourceB != nullptr) {
        std::cout << "EncoderSourceB: " << EncoderSourceB << std::endl;
    }
    else {
        std::cout << "EncoderSourceB is null!" << std::endl;
    }

    
    
    

    //开启视频流
    CheckStatus(status = dvpStart(h), "dvpStart");
    

    int triggerCount = 0;
    Log("Please enter the number of trigger executions:");
    std::cin >> triggerCount;
    

    dvpInt64 a = 0 , g = 0;
    

    //获取图像和获取编码器计数以及编码器向前计数
    dvpFrame frame ;
    void* buffer;
    for (int i = 0; i < triggerCount; ++i)
    {

        CheckStatus(dvpGetFrame(h, &frame, &buffer, 4000), "dvpGetFrame");
         
        printf("%s, frame:%lld, timestamp:%lld, %d*%d, %dbytes, format:%d\r\n",
            name, frame.uFrameID, frame.uTimestamp, frame.iWidth, frame.iHeight, frame.uBytes, frame.format);

        status = dvpGetIntValueSync(h, "EncoderValue", &a, nullptr);
        printDvpInt64("EncoderValue", a);
       
        status = dvpGetIntValueSync(h, "EncoderForwardCounter", &g, nullptr);
        printDvpInt64("EncoderForwardCounter", g);

    }


    //关闭相机
    status = dvpClose(h);

    printf("test quit, %s, status:%d\r\n", name, status);

}
