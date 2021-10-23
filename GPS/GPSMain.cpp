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
	GPS gps;
	gps.setupSharedMemory();
	Console::WriteLine("Process Awake");
	gps.connect(gps.Ip, gps.PortNumber);

	int PortNumber = 24000;
	TcpClient^ Client;
	array<unsigned char>^ SendData;
	array<unsigned char>^ ReadData;

	Client = gcnew TcpClient("192.168.1.200", PortNumber);
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;//ms
	Client->SendTimeout = 500;//ms
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(2500);
	System::String^ ResponseData = nullptr;
	
	NetworkStream^ Stream = Client->GetStream();
	SM_GPS GPSDataStruct;
	unsigned char* BytePtr;
	unsigned long data_length;

	while (!(_kbhit() || gps.getShutdownFlag()))
	{
		gps.setHeartbeat(MAX_WAIT_CYCLES);

		if (Stream->DataAvailable) {
			data_length = Stream->Read(ReadData, 0, ReadData->Length);

			BytePtr = (unsigned char*)(&GPSDataStruct);
			for (int i = 0; i < sizeof(SM_GPS); i++) {
				BytePtr[i] = ReadData[i];
			}
			std::cout << std::hex << "Server recevied CRC " << GPSDataStruct.Checksum << std::endl;
			if (GPSDataStruct.Checksum == CalculateBlockCRC32(data_length - 4, BytePtr) && *(BytePtr) == 0xAA) {
				GPSSMPtr->Northing = GPSDataStruct.Northing;
				GPSSMPtr->Easting = GPSDataStruct.Easting;
				GPSSMPtr->Height = GPSDataStruct.Height;
				GPSSMPtr->CRC = GPSDataStruct.Checksum;
				std::cout << "Northing: " << GPSSMPtr->Northing << std::endl;
				std::cout << "Easting: " << GPSSMPtr->Easting << std::endl;
				std::cout << "Height: " << GPSSMPtr->Height << std::endl;
				Console::WriteLine();
			}
			else {
				Console::WriteLine("Invalid data recieved.");
			}

		Thread::Sleep(25);
	}

	return 0;
}