#include <iostream>
#include <thread>


#include "E:\\Do3thinkSdk\\DVP2 SDK CN\\library\\Visual C++\\include\\DvpCamera.h"

#ifdef _M_X64
#pragma comment(lib, "E:\\Do3thinkSdk\\DVP2 SDK CN\\library\\Visual C++\\lib\\x64\\DVPCamera64.lib")
#else
#pragma comment(lib, "E:\\Do3thinkSdk\\DVP2 SDK CN\\library\\Visual C++\\lib\\x86\\DVPCamera.lib")
#endif

#define Log(X)   std::cout << X << std::endl
#define Log_Error(msg, err)   std::cout << msg << " failed with err: " << err << std::endl

void testTrigger(const char* Name, bool isSoftTrigger);
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
void testTrigger(const char* Name, bool isSoftTrigger)
{
    dvpHandle h;
    dvpFrame Frame;
    void* pBuffer;
    int triggerCount, Auto, AutoExposure, Overturn, Rotate;

    dvpOpenByName(Name, OPEN_NORMAL, &h);







}