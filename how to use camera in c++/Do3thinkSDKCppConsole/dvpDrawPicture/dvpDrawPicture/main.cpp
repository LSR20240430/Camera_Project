#include <iostream>
#include <thread>
#include <windows.h>
#include <conio.h>

#include "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\include\\DvpCamera.h"
#include "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\include\\dvpir.h"
#include "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\include\\dvpParam.h"

#ifdef _M_X64
#pragma comment(lib, "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\lib\\x64\\DVPCamera64.lib")
#else
#pragma comment(lib, "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\lib\\x86\\DVPCamera.lib")
#endif

#define Log(X) std::wcout << X << std::endl
#define Log_Error(msg, err) std::wcout << msg << L" failed with err: " << err << std::endl

HWND g_hWnd = NULL; // 用于显示相机图像的窗口句柄




bool GetUserChoice(const std::string& prompt)
{
    int choice;
   
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




bool Rotate1(dvpHandle h)
{
    bool GetRotateState, SetRotateState = true;
    bool GetRotateState1, SetRotateState1 = true;
    int RotateAngle = 1;

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






// 视频流回调函数
static dvpInt32 OnGetFrame(dvpHandle handle, dvpStreamEvent event, void* pContext, dvpFrame* pFrame, void* pBuffer)
{
    if (!pFrame || !pBuffer)
    {
        wprintf(L"OnGetFrame: Invalid frame or buffer.\n");
        return -1;
    }

    if (g_hWnd == NULL)
    {
        wprintf(L"OnGetFrame: No valid window handle.\n");
        return -1;
    }

    // 显示图像
    dvpStatus status = dvpDrawPicture(pFrame, pBuffer, g_hWnd, NULL, NULL);
    if (status != DVP_STATUS_OK)
    {
        wprintf(L"dvpDrawPicture failed with err: %d\n", status);
    }

    return 0;
}

// 线程函数，持续获取相机数据
void test(void* p)
{
    dvpStatus status;
    dvpHandle h;
    char* name = (char*)p;

    do
    {
        // 打开设备
        status = dvpOpenByName(name, OPEN_NORMAL, &h);
        if (status != DVP_STATUS_OK)
        {
            printf("dvpOpenByName failed with err:%d\r\n", status);
            break;
        }

        // 设置为连续模式
        status = dvpSetTriggerState(h, false);
        if (status != DVP_STATUS_OK)
        {
            printf("dvpSetTriggerState failed with err:%d\r\n", status);
            break;
        }
        printf("Enter 1 to Rotate Image, 0 to skip:");
        if (GetUserChoice("Enter 1 to Rotate Image, 0 to skip:"))
        {
            if (!Rotate1(h))
            {
                Log("Rotate Image failed, exiting loop.");
                break;
            }
        }








        // 注册视频流回调函数
        status = dvpRegisterStreamCallback(h, OnGetFrame, STREAM_EVENT_FRAME_THREAD, NULL);
        if (status != DVP_STATUS_OK)
        {
            printf("dvpRegisterStreamCallback failed with err:%d\r\n", status);
            break;
        }

        dvpFrame frame;
        void* buffer;

        // 开启视频流
        status = dvpStart(h);
        if (status != DVP_STATUS_OK)
        {
            printf("dvpStart failed with err:%d\r\n", status);
            break;
        }

        printf("Press ESC to stop streaming...\n");

        // 无限循环，持续获取帧
        while (true)
        {
            // 监听 ESC 键，按下后退出循环
            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
            {
                printf("ESC pressed, stopping stream...\n");
                break;
            }

           /* status = dvpGetFrame(h, &frame, &buffer, 4000);
            if (status != DVP_STATUS_OK)
            {
                printf("dvpGetFrame failed with err:%d\r\n", status);
                continue;
            }*/

            // 获取并显示帧信息
            dvpFrameCount framecount;
            status = dvpGetFrameCount(h, &framecount);
            if (status != DVP_STATUS_OK)
            {
                printf("get framecount failed\n");
            }
            printf("framecount: %d, framerate: %f\n", framecount.uFrameCount, framecount.fFrameRate);
        }

        // 停止视频流
        status = dvpStop(h);
        if (status != DVP_STATUS_OK)
        {
            printf("dvpStop failed with err:%d\r\n", status);
        }

    } while (0);
}

// 创建窗口用于显示相机画面
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// 创建并显示窗口
HWND CreateDisplayWindow()
{
    const wchar_t CLASS_NAME[] = L"CameraDisplayWindow";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, L"Camera Display", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, GetModuleHandle(NULL), NULL);

    if (hwnd)
    {
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
    }

    return hwnd;
}

int main()
{
    wprintf(L"start...\n");

    dvpUint32 count = 0, num = -1;
    dvpCameraInfo info[8];

    // 创建窗口用于显示图像
    g_hWnd = CreateDisplayWindow();
    if (!g_hWnd)
    {
        wprintf(L"Failed to create display window!\n");
        return -1;
    }

    // 枚举设备
    dvpRefresh(&count);
    if (count > 8)
        count = 8;

    for (int i = 0; i < (int)count; i++)
    {
        if (dvpEnum(i, &info[i]) == DVP_STATUS_OK)
        {
            wprintf(L"[%d]-Camera FriendlyName : %S\n", i, info[i].FriendlyName);
        }
    }

    // 没发现设备
    if (count == 0)
    {
        wprintf(L"No device found!\n");
        return 0;
    }

    while (num < 0 || num >= count)
    {
        wprintf(L"Please enter the number of the camera you want to open: \n");
        wscanf_s(L"%d", &num);
    }

    // 启动相机处理线程
    std::thread task(test, (void*)info[num].FriendlyName);
    task.detach(); // 让线程后台运行

    // 进入消息循环，保持窗口运行
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}