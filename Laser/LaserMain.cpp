#using <System.dll>
#include <Windows.h>
#include <conio.h>
#include <SMObject.h>
#include <smstructs.h>
#include "Laser.h"
#define MAX_WAIT_CYCLES 100

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;
using namespace System::Diagnostics;
using namespace System::Threading;

int main()
{
	Console::WriteLine("Process awake.");

	Laser laser;
	while(laser.setupSharedMemory());
	while (laser.connect(laser.Ip, laser.PortNumber) == ERROR) {
		if (laser.getShutdownFlag())
		{
			break;
		}
	}

	while (!(_kbhit() || laser.getShutdownFlag()))
	{
		laser.setHeartbeat(MAX_WAIT_CYCLES);
		laser.getData();
		if (laser.checkData() != SUCCESS)
		{
			continue;
		}
		
		laser.sendDataToSharedMemory();
		Thread::Sleep(25);
	}

	//Console::WriteLine("Press any key to quit");
	//Console::ReadKey();

	return 0;
}