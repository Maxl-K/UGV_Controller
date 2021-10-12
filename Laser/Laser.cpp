//Compile in a C++ CLR empty project
#using <System.dll>
#include <Windows.h>
#include <conio.h>
#include <SMObject.h>
#include <smstructs.h>
//#define MAX_WAIT_CYCLES 10
//^^ for home computer
#define MAX_WAIT_CYCLES 100

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;
using namespace System::Diagnostics;
using namespace System::Threading;

int main()
{
	Console::WriteLine("Process Awake");

	SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SMObject TStamps(TEXT("TStamps"), sizeof(TimeStamps));
	SMObject LaserObj(_TEXT("Laserobj"), sizeof(SM_Laser));

	PMObj.SMAccess();
	if (PMObj.SMAccessError) {
		Console::WriteLine("Shared memory access failed for PMObj");
	}
	TStamps.SMAccess();
	if (PMObj.SMAccessError) {
		Console::WriteLine("Shared memory access failed for TStamps");
	}
	LaserObj.SMAccess();
	if (LaserObj.SMAccessError) {
		Console::WriteLine("Shared memory access failed for LaserObj");
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
	TSData->LaserTimeStamp = NULL;

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
	//array<String^>^ Value = gcnew array<String^>(1082*2);
	array<String^>^ Value;
	array<double>^ Range;
	array<double>^ X;
	array<double>^ Y;
	double Angle, Resolution;
	int PointCloudSize;

	// Get the network streab object associated with clien so we 
	// can use it to read and write
	NetworkStream^ Stream = Client->GetStream();

	PMData->Heartbeat.Flags.Laser = 1;

	//Authentication
	SendData = System::Text::Encoding::ASCII->GetBytes(Zid);
	Stream->Write(SendData, 0, SendData->Length);
	//wait for server
	System::Threading::Thread::Sleep(10);
	Stream->Read(ReadData, 0, ReadData->Length);
	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
	//Validate Response
	if (ResponseData != "OK") {
		//Error?
		Console::WriteLine("Authentication failed. Response Value: " + ResponseData);
	}

	// Convert string command to an array of unsigned char
	SendData = System::Text::Encoding::ASCII->GetBytes(AskScan);

	//Loop
	while (!(_kbhit() || PMData->Shutdown.Flags.Laser))
	{
		//Heartbeats and TS
		QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
		TSData->LaserTimeStamp = (double)Counter / (double)Frequency * 1000; // ms
		Console::WriteLine("Laser time stamp    : {0,12:F3}", TSData->LaserTimeStamp);

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
		//Console::WriteLine(ResponseData);
		Value = ResponseData->Split();

		if (Value->Length < 26) {
			Console::WriteLine("Not enough data. Length recieved: " + Value->Length);
			continue;
		}
		if (Value[1] != "LMDscandata") {
			Console::WriteLine("Unexpected header. Header recieved: " + Value[1]);
			continue;
		}
		//angle = System::Text::Encoding::ASCII->GetString(*(unsigned char)Value[23]);
		Angle = System::Convert::ToInt32(Value[23], 16);
		Resolution = System::Convert::ToInt32(Value[24], 16) / 10000;
		PointCloudSize = System::Convert::ToInt32(Value[25], 16);

		Range = gcnew array<double>(PointCloudSize);
		X = gcnew array<double>(PointCloudSize);
		Y = gcnew array<double>(PointCloudSize);

		if (Value->Length < 26 + PointCloudSize) {
			continue;
		}

		std::cout << "Starting angle: " << Angle << std::endl;
		std::cout << "Resolution: " << Resolution << std::endl;
		std::cout << "Number of points: " << PointCloudSize << std::endl;
		
		LSData->PointCloudSize = PointCloudSize;
		for (int i = 0; i < PointCloudSize; i++) {
			Range[i] = System::Convert::ToInt32(Value[26 + i], 16);
			X[i] = (Range[i] * Math::Sin(i * Resolution * Math::PI / 180.0)) / 1000.0;
			Y[i] = (-Range[i] * Math::Cos(i * Resolution * Math::PI / 180.0)) / 1000.0;
			Console::WriteLine("Point " + i + " X: " + X[i] + "  Y: " + Y[i]);
			LSData->x[i] = X[i];
			LSData->y[i] = Y[i];
		}

		Thread::Sleep(25);
		if (PMData->Shutdown.Flags.Laser)
			break;
		if (_kbhit())
			break;

	}

	Stream->Close();
	Client->Close();

	//Console::WriteLine("Press any key to quit");
	//Console::ReadKey();

	return 0;
}