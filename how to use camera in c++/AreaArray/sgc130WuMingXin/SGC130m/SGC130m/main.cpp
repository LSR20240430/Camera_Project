/*****************************************************************************
* @FileName:dvp2getframe.cpp
* @CreatTime: 2020/8/19 11:58
* @Descriptions: 简单的应用程序
* @Version: ver 1.0
* @Copyright(c) 2020 Do3Think All Rights Reserved.
*****************************************************************************/

#include <iostream>
#include <thread>
using namespace std;

// DVP API 依赖
#include "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\include\\DvpCamera.h"
#include "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\include\\dvpir.h"
#include "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\include\\dvpParam.h"

#ifdef _M_X64
#pragma comment(lib, "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\lib\\x64\\DVPCamera64.lib")
#else
#pragma comment(lib, "D:\\SoftWare\\Do3Think\\SDK\\DVP2 SDK CN\\library\\Visual C++\\lib\\x86\\DVPCamera.lib")
#endif

#pragma warning(disable:4996)

/* 打开下面两个注释切换为触发模式 */
//#define TEST_TRIG			/* 设置触发模式宏定义，打开注释，即可开启软触发模式 */
//#define SOFT_TRIG			/* 软触发宏定义，打开注释，即可进行软触发 */
#define GRABCOUNT 20		/* 抓帧次数 */

void test(void* p)
{
	dvpStatus status;
	dvpHandle h;
	bool trigMode = false;

	char* name = (char*)p;

	printf("Test start,camera is %s\r\n", name);
	do
	{
		/* 打开设备 */
		status = dvpOpenByName(name, OPEN_NORMAL, &h);
		if (status != DVP_STATUS_OK)
		{
			printf("dvpOpenByName failed with err:%d\r\n", status);
			break;
		}

		// 华封参数设置 

		// 设置触发模式为On
		status = dvpSetBoolValue(h, "TriggerMode", true); // 1通常代表On，具体值需查枚举定义
		if (status != DVP_STATUS_OK)
		{
			printf("dvpSetBoolValue TriggerMode true failed with err:%d\r\n", status);
			break;
		}

		// 设置曝光时间为1100.0
		status = dvpSetFloatValue(h, "ExposureTime", 1100.0f);
		if (status != DVP_STATUS_OK)
		{
			printf("dvpSetFloatValue ExposureTime failed with err:%d\r\n", status);
			break;
		}

		// 设置触发选择器为FrameBurstStart
		status = dvpSetEnumValueByString(h, "TriggerSelector", "FrameBurstStart"); // 2假设为FrameBurstStart
		if (status != DVP_STATUS_OK) {
			printf("dvpSetEnumValue TriggerSelector failed with err:%d\r\n", status);
			break;
		}

		// 设置触发源为Software
		status = dvpSetEnumValueByString(h, "TriggerSource", "Software"); // 7假设为Software
		if (status != DVP_STATUS_OK) {
			printf("dvpSetEnumValue TriggerSource failed with err:%d\r\n", status);
			break;
		}

		// 设置采集模式为Continuous
		status = dvpSetEnumValueByString(h, "AcquisitionMode", "Continuous"); // 2假设为Continuous
		if (status != DVP_STATUS_OK) {
			printf("dvpSetEnumValue AcquisitionMode failed with err:%d\r\n", status);
			break;
		}

		// 设置闪光输出为LevelHigh
		//status = dvpSetEnumValueByString(h, "StrobeOutput", "LevelHigh"); // 1假设为LevelHigh
		if (status != DVP_STATUS_OK) {
			printf("dvpSetEnumValue StrobeOutput failed with err:%d\r\n", status);
			break;
		}

		// 设置线路选择为Line2
		status = dvpSetEnumValueByString(h, "LineSelector", "Line2"); // 2假设为Line2
		if (status != DVP_STATUS_OK) {
			printf("dvpSetEnumValue LineSelector failed with err:%d\r\n", status);
			break;
		}

		// 设置线路模式为Output
		status = dvpSetEnumValueByString(h, "LineMode", "Output"); // 1假设为Output
		if (status != DVP_STATUS_OK) {
			printf("dvpSetEnumValue LineMode failed with err:%d\r\n", status);
			break;
		}

		// 设置线路源为Strobe
		status = dvpSetEnumValueByString(h, "LineSource", "Strobe"); // 3假设为Strobe
		if (status != DVP_STATUS_OK) {
			printf("dvpSetEnumValue LineSource failed with err:%d\r\n", status);
			break;
		}

		// 设置LineInverter为0
		status = dvpSetBoolValue(h, "LineInverter", 0);
		if (status != DVP_STATUS_OK) {
			printf("dvpSetIntValue LineInverter failed with err:%d\r\n", status);
			break;
		}

		// 设置StrobeSource为ExposureDuration
		status = dvpSetEnumValueByString(h, "StrobeSource", "ExposureDuration"); // 2假设为ExposureDuration
		if (status != DVP_STATUS_OK) {
			printf("dvpSetEnumValue StrobeSource failed with err:%d\r\n", status);
			break;
		}

		/* 帧信息 */
		dvpFrame frame;
		/* 帧数据首地址，用户不需要申请释放内存 */
		void* p;

		/* 开始视频流 */
		status = dvpStart(h);
		if (status != DVP_STATUS_OK)
		{
			break;
		}

		/* 抓帧 */
		for (int j = 0; j < GRABCOUNT; j++)
		{
#ifdef SOFT_TRIG
			if (trigMode)
			{
				// trig a frame
				status = dvpTriggerFire(h);
				if (status != DVP_STATUS_OK)
				{
					printf("Fail to trig a frame\r\n");
				}
			}
#endif
			/* 当前案例没有设置相机的曝光增益等参数，只展示在默认的ROI区域显示帧信息 */
			status = dvpGetFrame(h, &frame, &p, 3000);
			if (status != DVP_STATUS_OK)
			{
				if (trigMode)
				{
					printf("Fail to get a frame in trigger mode \r\n");
					continue;
				}
				else
				{
					printf("Fail to get a frame in continuous mode \r\n");
					break;
				}
			}

			/* 显示帧数和帧率 */
			dvpFrameCount framecount;
			status = dvpGetFrameCount(h, &framecount);
			if (status != DVP_STATUS_OK)
			{
				printf("get framecount failed\n");
			}
			printf("framecount: %d, framerate: %f\n", framecount.uFrameCount, framecount.fFrameRate);

			/* 显示帧信息 */
			printf("%s, frame:%lld, timestamp:%lld, %d*%d, %dbytes, format:%d\r\n",
				name,
				frame.uFrameID,
				frame.uTimestamp,
				frame.iWidth,
				frame.iHeight,
				frame.uBytes,
				frame.format);

			/* 需要创建pic目录保存图片 */
			/*char PicName[64];
			sprintf(PicName, "pic/test-%s_pic_%d.jpg", name, j);
			status = dvpSavePicture(&frame, p, PicName, 90);
			if (status == DVP_STATUS_OK)
			{
				printf("Save to %s OK\r\n", PicName);
			}*/

		}

		/* 停止视频流 */
		status = dvpStop(h);
		if (status != DVP_STATUS_OK)
		{
			break;
		}

	} while (0);

	status = dvpClose(h);

	printf("test quit, %s, status:%d\r\n", name, status);
}

int main()
{
	printf("start...\r\n");

	dvpUint32 count = 0, num = -1;
	dvpCameraInfo info[8];

	/* 枚举设备 */
	dvpRefresh(&count);
	if (count > 8)
		count = 8;

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

	while (num < 0 || num >= count)
	{
		printf("Please enter the number of the camera you want to open: \r\n");
		scanf("%d", &num);
	}

	thread task(test, (void*)info[num].FriendlyName);
	task.join();

	system("pause");
	return 0;
}