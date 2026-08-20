#include<iostream>
#include<thread>

#include "E:\\DO3THINK\\DVP2 SDK CN\\library\\Visual C++\\include\\DvpCamera.h"

#ifdef _M_X64
#pragma comment(lib, "E:\\DO3THINK\\DVP2 SDK CN\\library\\Visual C++\\lib\\x64\\DVPCamera64.lib")
#else
#pragma comment(lib, "E:\\DO3THINK\\DVP2 SDK CN\\library\\Visual C++\\lib\\x86\\DVPCamera.lib")
#endif

#define Log(X)   std::cout << X << std::endl
#define Log_Error(msg, err)   std::cout << msg << " failed with err: " << err << std::endl
void test(void* p);


int main()
{
	
	dvpUint32 Count, Num;
	dvpCameraInfo Info[2];
	dvpRefresh(&Count);
	if (Count > 2)
	{
		Count = 2;
	}
	if (Count == 0)
	{
		Log("No device found");
	}
	for (dvpUint32 i = 0; i < Count; i++)
	{
		if (dvpEnum(i, &Info[i]) == DVP_STATUS_OK)
		{
			printf("[%d]-Camera FriendlyName :%s\r\n", i, Info[i].FriendlyName);
		}
	}
	if (Count > 0)
	{
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
	do 
	{
		Status = dvpOpenByName(Name, OPEN_NORMAL, &h);
		if (Status != DVP_STATUS_OK)
		{
			Log_Error("dvpOpenByName", Status);
			break;
		}
		
		Status = dvpSetTriggerState(h, true);
		if (Status != DVP_STATUS_OK)
		{
			Log_Error("dvpSetTriggerState", Status);
			break;
		}

		Status = dvpSetTriggerSource(h, TRIGGER_SOURCE_SOFTWARE);
		if (Status != DVP_STATUS_OK)
		{
			Log_Error("dvpSetTriggerSource", Status);
			break;
		}
		Status = dvpStart(h);
		if (Status != DVP_STATUS_OK)
		{
			Log_Error("dvpStart", Status);
			break;
		}
		/*Status = dvpSetSoftTriggerLoolState(h,true);
          Status = dvpGetSoftTriggerLoopDescr(dvpHandle handle, dvpDoubleDescr *pSoftTriggerLoopDescr);	
		  Status = dvpSetSoftTriggerLoop(dvpHandle handle, double SoftTriggerLoop);
		*/
		
		
		
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

			printf("%s, frame:%lld, timestamp:%lld, %d*%d, %dbytes, format:%d\r\n",
				Name,
				Frame.uFrameID,
				Frame.uTimestamp,
				Frame.iWidth,
				Frame.iHeight,
				Frame.uBytes,
				Frame.format);

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

