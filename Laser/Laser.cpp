#using <System.dll>
#include <Windows.h>
#include <conio.h>
#include <SMObject.h>
#include <smstructs.h>
#define MAX_WAIT_CYCLES 10

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;

int main()
{
	//SM Creation and seeking access
	//Process Management
	SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	PMObj.SMAccess();

	//Timestamp setup
	SMObject TStamps(TEXT("TStamps"), sizeof(TimeStamps));
	TStamps.SMAccess();

	//Declaration
	__int64 Frequency, Counter;
	int Shutdown = 0x00;
	int WaitCounter = 0;

	//Part of Windows.h 
	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);

	TimeStamps* TSData = (TimeStamps*)TStamps.pData;
	ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;

	while (1)
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
		TSData->LaserTimeStamp = (double)Counter / (double)Frequency * 1000; // ms
		Console::WriteLine("Laser time stamp    : {0,12:F3} {1,12:X2}", TSData->LaserTimeStamp, Shutdown);
		
		if (PMData->Heartbeat.Flags.Laser == 0) {
			PMData->Heartbeat.Flags.Laser = 1;
			WaitCounter = 0;
		};
		if (PMData->Heartbeat.Flags.Laser == 1) {
			
			if (WaitCounter++ > MAX_WAIT_CYCLES)
			{
				PMData->Shutdown.Status = 0xFF;
			}
		}
		
		Thread::Sleep(25);
		if (PMData->Shutdown.Flags.Laser)
			break;
		if (_kbhit())
			break;
	}


	return 0;
}