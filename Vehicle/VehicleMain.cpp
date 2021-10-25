#define MAX_WAIT_CYCLES 100
#include "Vehicle.h"

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
	Vehicle vehicle;
	vehicle.setupSharedMemory();
	Console::WriteLine("Process Awake");
	while (!vehicle.getShutdownFlag())
	{
		vehicle.setHeartbeat(MAX_WAIT_CYCLES);
		vehicle.getData();
		Thread::Sleep(25);
	}

	return 0;
}