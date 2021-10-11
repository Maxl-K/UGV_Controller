#define MAX_WAIT_CYCLES 10
#using <System.dll>
#include <Windows.h>
#include <conio.h>

#include <SMObject.h>
#include <smstructs.h>

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;

int main()
{
	//Declaration
	SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	//SM Creation and seeking access
	double TimeStamp;
	__int64 Frequency, Counter;
	int Shutdown = 0x00;
	int WaitCounter = 0;

	//Part of Windows.h 
	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);

	PMObj.SMAccess();
	ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;

	while (1)
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
		TimeStamp = (double)Counter / (double)Frequency * 1000; // ms
		Console::WriteLine("Vehicle time stamp    : {0,12:F3}", TimeStamp);

		if (PMData->Heartbeat.Flags.Vehicle == 0) {
			PMData->Heartbeat.Flags.Vehicle = 1;
			WaitCounter = 0;
		};
		if (PMData->Heartbeat.Flags.Vehicle == 1) {

			if (WaitCounter++ > MAX_WAIT_CYCLES)
			{
				PMData->Shutdown.Status = 0xFF;
			}
		}

		Thread::Sleep(25);
		if (PMData->Shutdown.Flags.Vehicle)
			break;
		if (_kbhit())
			break;
	}

	return 0;
}