#using <System.dll>
#include <direct.h>
#include <string>
#include <conio.h>
#include <SMObject.h>
#include <smstructs.h>

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;


int main()
{
	// Tele-operation
	//Declarations + Initializations
	SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	array<String^>^ ModuleList = gcnew array<String^>{"Laser", "Display", "Vehicle", "GPS", "Camera"};
	array<int>^ Critical = gcnew array<int>(ModuleList->Length) { 0, 0, 0, 0, 0 };
	array<Process^>^ ProcessList = gcnew array<Process^>(ModuleList->Length);

	//SM Creation and seeking access
	PMObj.SMCreate();
	PMObj.SMAccess();

	ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;


	for (int i = 0; i < ModuleList->Length; i++)
	{
		if (Process::GetProcessesByName(ModuleList[i])->Length == 0)
		{
			ProcessList[i] = gcnew Process;

			std::string cwd("\0", FILENAME_MAX + 1);
			std::string currDir = getcwd(&cwd[0], cwd.capacity());
			currDir.append("\\..\\Debug");
			String^ dirString = gcnew String(currDir.c_str());
			ProcessList[i]->StartInfo->WorkingDirectory = dirString;

			ProcessList[i]->StartInfo->FileName = ModuleList[i];
			ProcessList[i]->Start();
			Console::WriteLine("The process " + ModuleList[i] + ".exe started");
		}
	}



	// Main loop
	while (!_kbhit())
	{

		Thread::Sleep(25);
	}

	PMData->Shutdown.Status = 0xFF;
	// Clearing and shutdown

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