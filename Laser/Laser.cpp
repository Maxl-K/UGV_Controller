//Compile in a C++ CLR empty project
#using <System.dll>
#include <Windows.h>
#include <conio.h>
#include <SMObject.h>
#include <smstructs.h>
#define MAX_WAIT_CYCLES 10

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;
using namespace System::Diagnostics;
using namespace System::Threading;

int main()
{
	SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SMObject TStamps(TEXT("TStamps"), sizeof(TimeStamps));
	SMObject LaserObj(_TEXT("Laserobj"), sizeof(SM_Laser));

	PMObj.SMAccess();
	if (PMObj.SMAccessError) {
		Console::WriteLine("Shared memory access failed");
		return -1;
	}
	TStamps.SMAccess();
	if (PMObj.SMAccessError) {
		Console::WriteLine("Shared memory access failed");
		return -1;
	}
	LaserObj.SMAccess();
	if (LaserObj.SMAccessError) {
		Console::WriteLine("Shared memory access failed");
		return -1;
	}

	//Declaration
	__int64 Frequency, Counter;
	int Shutdown = 0x00;
	int WaitCounter = 0;

	//Part of Windows.h 
	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);

	TimeStamps* TSData = (TimeStamps*)TStamps.pData;
	ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;
	SM_Laser* LSData = (SM_Laser*)LaserObj.pData;
	PMData->Heartbeat.Flags.Laser = 1;

	// LMS151 port number must be 23000
	int PortNumber = 23000;
	// Pointer to TcpClent type object on managed heap
	TcpClient^ Client;
	// arrays of unsigned chars to send and receive data
	array<unsigned char>^ SendData;
	array<unsigned char>^ ReadData;
	// String command to ask for Channel 1 analogue voltage from the PLC
	// These command are available on Galil RIO47122 command reference manual
	// available online
	String^ AskScan = gcnew String("sRN LMDscandata");
	// String to store received data for display
	String^ ResponseData;

	// Creat TcpClient object and connect to it
	Client = gcnew TcpClient("192.168.1.200", PortNumber);
	// Configure connection
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;//ms
	Client->SendTimeout = 500;//ms
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	// unsigned char arrays of 16 bytes each are created on managed heap
	System::String^ Zid = gcnew System::String("5265207\n");
	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(2500);

	// Get the network streab object associated with clien so we 
	// can use it to read and write
	NetworkStream^ Stream = Client->GetStream();

	//Authentication
	SendData = System::Text::Encoding::ASCII->GetBytes(Zid);
	Stream->Write(SendData, 0, SendData->Length);
	//wait for server
	System::Threading::Thread::Sleep(10);
	Stream->Read(ReadData, 0, ReadData->Length);
	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
	//Validate Response
	Console::WriteLine(ResponseData);
	Console::ReadKey();

	// Convert string command to an array of unsigned char
	SendData = System::Text::Encoding::ASCII->GetBytes(AskScan);

	//Loop
	while (!(_kbhit() || PMData->Shutdown.Flags.Laser))
	{
		//Heartbeats and TS
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

		// Write command asking for data
		Stream->WriteByte(0x02);
		Stream->Write(SendData, 0, SendData->Length);
		Stream->WriteByte(0x03);
		// Wait for the server to prepare the data, 1 ms would be sufficient, but used 10 ms
		System::Threading::Thread::Sleep(10);
		// Read the incoming data
		Stream->Read(ReadData, 0, ReadData->Length);
		// Convert incoming data from an array of unsigned char bytes to an ASCII string
		ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
		// Print the received string on the screen
		Console::WriteLine(ResponseData);

		Thread::Sleep(25);
		if (PMData->Shutdown.Flags.Laser)
			break;
		if (_kbhit())
			break;

	}

	Stream->Close();
	Client->Close();

	return 0;
}