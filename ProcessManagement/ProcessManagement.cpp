#using <System.dll>
#include <direct.h>
#include <string>
#include <conio.h>
#include <SMObject.h>
#include <smstructs.h>
//#define MAX_WAIT_CYCLES 20
//^^ for home computer
#include <bitset>
#include <typeinfo>

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;

int main()
{
	//Timestamp Initialisations
	double TimeStamp;
	__int64 Frequency, Counter;
	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);

	//Get current directory
	std::string cwd("\0", FILENAME_MAX + 1);
	std::string currDir = getcwd(&cwd[0], cwd.capacity());
	currDir.append("\\..\\Debug");
	String^ dirString = gcnew String(currDir.c_str());

	//Process setup
	SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	array<String^>^ ModuleList = gcnew array<String^>{"Laser", "Display", "Vehicle", "GPS", "Camera"};
	array<int>^ Critical = gcnew array<int>(ModuleList->Length) { 0, 0, 0, 0, 0 };
	array<Process^>^ ProcessList = gcnew array<Process^>(ModuleList->Length);

	array<int>^ WaitCounter = gcnew array<int>(ModuleList->Length) { 0, 0, 0, 0, 0 };
	array<int>^ MaxCounts = gcnew array<int>(ModuleList->Length) { 500, 500, 500, 500, 500 };

	//Timestamp setup
	SMObject TStamps(TEXT("TStamps"), sizeof(TimeStamps));

	//SM Creation and seeking access
	PMObj.SMCreate();
	PMObj.SMAccess();
	TStamps.SMCreate();
	TStamps.SMAccess();
	if (PMObj.SMCreateError || TStamps.SMAccessError) {
		Console::WriteLine("Shared memory creation failed. Terminating.");
		return -1;
	}

	//Casting pointers
	ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;
	TimeStamps* TSData = (TimeStamps*)TStamps.pData;

	//Initialise shutdown flags
	PMData->Shutdown.Status = 0x00;

	//Initialise heartbeats flag will, be pulled up by other process
	PMData->Heartbeat.Status = 0x00;

	for (int i = 0; i < ModuleList->Length; i++)
	{
		if (Process::GetProcessesByName(ModuleList[i])->Length == 0)
		{
			ProcessList[i] = gcnew Process;
			ProcessList[i]->StartInfo->WorkingDirectory = dirString;
			ProcessList[i]->StartInfo->FileName = ModuleList[i];
			ProcessList[i]->Start();
			Console::WriteLine("The process " + ModuleList[i] + ".exe started");
		}
	}

	// Main loop
	while (!(_kbhit() || PMData->Shutdown.Flags.PM))
	{
		//Get PM Timestamp
		QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
		TimeStamp = (double)Counter / (double)Frequency * 1000; // ms
		TSData->PMTimeStamp = TimeStamp;

		WaitCounter[0] = 0;
		for (int i = 0; i < 5; i++)
		{
			if (((PMData->Heartbeat.Status) & (1 << i)) != 0)
			{
				PMData->Heartbeat.Status = (PMData->Heartbeat.Status) ^ (1 << i);
				WaitCounter[i] = 0;
			}
			else
			{
				if ((i == 0) && (TSData->LaserTimeStamp == NULL))
				{
					continue;
				}
				if (WaitCounter[i]++ > MaxCounts[i])
				{
					if (Critical[i] == 1)
					{
						//shutdown all
						Console::WriteLine("Critical process " + ModuleList[i] + ".exe unresponsive");
						PMData->Shutdown.Status = 0xFF;
						break;
					}
					else
					{
						Console::WriteLine("Non-critical process " + ModuleList[i] + ".exe unresponsive");
						if (Process::GetProcessesByName(ModuleList[i])->Length == 0)
						{
							// re-start
							ProcessList[i] = gcnew Process;
							ProcessList[i]->StartInfo->WorkingDirectory = dirString;
							ProcessList[i]->StartInfo->FileName = ModuleList[i];
							ProcessList[i]->Start();
							Console::WriteLine("The process " + ModuleList[i] + ".exe was restarted");
							WaitCounter[i] = 0;
						}
						else
						{
							// kill GPS
							// restart GPS
							ProcessList[i]->Kill();
							ProcessList[i] = gcnew Process;
							ProcessList[i]->StartInfo->WorkingDirectory = dirString;
							ProcessList[i]->StartInfo->FileName = ModuleList[i];
							ProcessList[i]->Start();
							Console::WriteLine("A new process " + ModuleList[i] + ".exe was started");
							WaitCounter[i] = 0;
						}
					}
				}
			}
		}

		Thread::Sleep(25);
	}

	// Clearing and shutdown
	PMData->Shutdown.Status = 0xFF;
	Console::WriteLine("Process management terminated normally. Press any key to close");
	Console::ReadKey();

	return 0;
}


/*
#using <System.dll>
#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <iostream>
#include <conio.h>

#include "SMStructs.h"
#include "SMObject.h"

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

#define NUM_UNITS 3

bool IsProcessRunning(const char* processName);
void StartProcesses();

//defining start up sequence
TCHAR Units[10][20] = //
{
	TEXT("GPS.exe"),
	TEXT("Camera.exe"),
	TEXT("Display.exe"),
	TEXT("Laser.exe"),
	TEXT("VehicleControl.exe")
};

int main()
{
	//start all 5 modules
	StartProcesses();
	return 0;
}


//Is process running function
bool IsProcessRunning(const char* processName)
{
	bool exists = false;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry))
		while (Process32Next(snapshot, &entry))
			if (!_stricmp((const char *)entry.szExeFile, processName))
				exists = true;

	CloseHandle(snapshot);
	return exists;
}


void StartProcesses()
{
	STARTUPINFO s[10];
	PROCESS_INFORMATION p[10];

	for (int i = 0; i < NUM_UNITS; i++)
	{
		if (!IsProcessRunning((const char *)Units[i]))
		{
			ZeroMemory(&s[i], sizeof(s[i]));
			s[i].cb = sizeof(s[i]);
			ZeroMemory(&p[i], sizeof(p[i]));

			if (!CreateProcess(NULL, Units[i], NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &s[i], &p[i]))
			{
				printf("%s failed (%d).\n", Units[i], GetLastError());
				_getch();
			}
			std::cout << "Started: " << Units[i] << std::endl;
			Sleep(100);
		}
	}
}

*/