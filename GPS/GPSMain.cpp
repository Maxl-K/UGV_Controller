#define MAX_WAIT_CYCLES 100
#include "GPS.h"

#using <System.dll>
#include <Windows.h>
#include <conio.h>

#include <SMObject.h>
#include <smstructs.h>

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;
using namespace System::Diagnostics;
using namespace System::Threading;

int main()
{
	Console::WriteLine("Process Awake.");
	GPS gps;
	while(gps.setupSharedMemory());
	while(gps.connect(gps.Ip, gps.PortNumber) == ERROR) {
		if (gps.getShutdownFlag())
		{
			break;
		}
	}

	while (!(_kbhit() || gps.getShutdownFlag()))
	{
		gps.setHeartbeat(MAX_WAIT_CYCLES);
		gps.getData();
		if (gps.checkData() != SUCCESS)
		{
			continue;
		}
		gps.sendDataToSharedMemory();
		Thread::Sleep(25);
	}
	return 0;
}