#define MAX_WAIT_CYCLES 100
#include "GPS.h"

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
	GPS gps;
	gps.setupSharedMemory();
	Console::WriteLine("Process Awake");
	while (!(_kbhit() || gps.getShutdownFlag()))
	{
		gps.setHeartbeat(MAX_WAIT_CYCLES);
		Thread::Sleep(25);
	}

	return 0;
}