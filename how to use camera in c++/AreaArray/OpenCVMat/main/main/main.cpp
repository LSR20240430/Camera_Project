#include <iostream>
#include <mutex>
#include <atomic>
#include <thread>
#include <opencv2/opencv.hpp>

// DVP SDK 头文件
#include "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\include\\DvpCamera.h"
#include "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\include\\dvpir.h"
#include "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\include\\dvpParam.h"

// 链接 SDK 库
#ifdef _M_X64
#pragma comment(lib, "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\lib\\x64\\DVPCamera64.lib")
#else
#pragma comment(lib, "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\lib\\x86\\DVPCamera.lib")
#endif

// 全局数据
static std::mutex        g_frameMutex;
static cv::Mat           g_latestFrame;
static std::atomic<bool> g_hasFrame{ false };

// 简单实现 gcd (支持 C++11)
int gcd(int a, int b) {
    return b == 0 ? a : gcd(b, a % b);
}

// 获取图像格式字符串
const char* GetFormatString(dvpImageFormat format) 
{
    static const char* formatStr[] = {
        "MONO(黑白)", "BAYER_BG", "BAYER_GB", "BAYER_GR", "BAYER_RG",
        "", "", "", "", "",
        "BGR24", "BGR32", "BGR48", "BGR64", "RGB24",
        "RGB32", "RGB48", "", "", "",
        "YUV411", "YUV422", "YUV444", "YUV420", "",
        "", "", "", "", "",
        "", "", "", "", "",
        "B8_G8_R8", "", "", "", "B16_G16_R16",
        "", "", "", "", "",
        "", "", "", "", "",
        "BGRW8", "RGBW8"
    };
    return (format <= FORMAT_RGBW8) ? formatStr[format] : "Unknown";
}

// 估算每行字节数（pitch）
int EstimatePitch(const dvpFrame* pFrame) {
    int w = pFrame->iWidth;
    switch (pFrame->format) {
    case FORMAT_MONO:    return w * 1;
    case FORMAT_BGR24:
    case FORMAT_RGB24:   return w * 3;
    default:             return w * 3;
    }
}

// 打印帧信息
void PrintFrameInfo(const dvpFrame* pFrame) {
    if (!pFrame) {
        std::cout << "[错误] 帧数据指针为空！\n";
        return;
    }
    std::cout << "\n===== 帧信息 =====\n";
    std::cout << "格式: " << GetFormatString(pFrame->format) << "\n";
    std::cout << "尺寸: " << pFrame->iWidth << "x" << pFrame->iHeight << "\n";
    std::cout << "字节数: " << pFrame->uBytes << " Bytes\n";
    std::cout << "帧ID: " << pFrame->uFrameID << "\n";
    std::cout << "时间戳: " << pFrame->uTimestamp << " ns\n";
    std::cout << "==================\n\n";
}

// 检查图像方向（Top-Down 或 Bottom-Up）
void CheckBufferDirection(const dvpFrame* pFrame, void* pBuffer, int pitch) {
    if (!pFrame || !pBuffer) return;
    auto ptr = reinterpret_cast<unsigned char*>(pBuffer);
    int h = pFrame->iHeight;

    const unsigned char* firstRow = ptr;
    const unsigned char* lastRow = ptr + (h - 1) * pitch;

    int sumFirst = 0, sumLast = 0;
    int checkLen = (pitch < 32 ? pitch : 32);
    for (int i = 0; i < checkLen; ++i) {
        sumFirst += firstRow[i];
        sumLast += lastRow[i];
    }

    std::string dir = (sumFirst >= sumLast)
        ? "从上到下 (正常)"
        : "从下到上 (可能倒置)";
    std::cout << "[方向推测] 图像排列方向: " << dir << "\n";
}

// 回调函数
static dvpInt32 OnGetFrame(
    dvpHandle      h,
    dvpStreamEvent e,
    void* ctx,
    dvpFrame* f,
    void* buf)
{
    if (!f || !buf) return 0;

    PrintFrameInfo(f);

    // 对齐分析
    int pitch = EstimatePitch(f);
    int expectedTotal = pitch * f->iHeight;
    int actualPitch = f->uBytes / f->iHeight;
    int padding = actualPitch - pitch;
    int alignStep = (padding > 0 ? gcd(actualPitch, padding) : 0);
    std::cout << "[对齐分析] 紧凑行长: " << pitch
        << ", 实际行长: " << actualPitch
        << ", padding: " << padding
        << ", 对齐步长: " << alignStep << " 字节\n";

    CheckBufferDirection(f, buf, actualPitch);

    // 拷贝图像
    auto ptr = reinterpret_cast<unsigned char*>(buf);
    cv::Mat tmp;
    switch (f->format) {
    case FORMAT_MONO:
        tmp = cv::Mat(f->iHeight, f->iWidth, CV_8UC1, ptr).clone();
        break;
    case FORMAT_BGR24:
        tmp = cv::Mat(f->iHeight, f->iWidth, CV_8UC3, ptr).clone();
        break;
    case FORMAT_RGB24: {
        cv::Mat rgb(f->iHeight, f->iWidth, CV_8UC3, ptr);
        cv::Mat bgr;
        cv::cvtColor(rgb, bgr, cv::COLOR_RGB2BGR);
        tmp = bgr.clone();
        break;
    }
    default:
        return 0;
    }

    {
        std::lock_guard<std::mutex> lk(g_frameMutex);
        g_latestFrame = std::move(tmp);
        g_hasFrame = true;
    }
    return 0;
}

int main() {
    dvpUint32 count = 0;
    dvpRefresh(&count);
    if (count == 0) {
        std::cerr << "No camera found!\n";
        return -1;
    }
    if (count > 8) count = 8;

    dvpCameraInfo info[8];
    for (dvpUint32 i = 0; i < count; ++i)
        if (dvpEnum(i, &info[i]) == DVP_STATUS_OK)
            std::cout << "[" << i << "] " << info[i].FriendlyName << "\n";

    dvpUint32 choice;
    std::cout << "Select camera index (0-" << count - 1 << "): ";
    std::cin >> choice;
    if (choice >= count) return -1;

    dvpHandle h;
    if (dvpOpenByName(info[choice].FriendlyName, OPEN_NORMAL, &h) != DVP_STATUS_OK) return -1;
    if (dvpRegisterStreamCallback(h, OnGetFrame, STREAM_EVENT_FRAME_THREAD, nullptr) != DVP_STATUS_OK) return -1;
    if (dvpStart(h) != DVP_STATUS_OK) return -1;

    cv::namedWindow("Camera Stream", cv::WINDOW_AUTOSIZE);
    std::cout << "Press ESC to exit\n";
    while (true) {
        if (g_hasFrame) {
            std::lock_guard<std::mutex> lk(g_frameMutex);
            if (!g_latestFrame.empty()) {
                cv::imshow("Camera Stream", g_latestFrame);
                g_hasFrame = false;
            }
        }
        if (cv::waitKey(1) == 27) break;
    }

    dvpStop(h);
    dvpClose(h);
    cv::destroyAllWindows();
    return 0;
}
