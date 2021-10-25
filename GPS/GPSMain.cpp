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

	while (!(_kbhit() || gps.getShutdownFlag()))
	{
		gps.setHeartbeat(MAX_WAIT_CYCLES);
		// GPS CODE HERE >>>

		int PortNumber = 24000;
		TcpClient^ Client;
		array<unsigned char>^ SendData;
		array<unsigned char>^ ReadData;

		Client = gcnew TcpClient("192.168.1.200", PortNumber);
		// Configure connection
		Client->NoDelay = true;
		Client->ReceiveTimeout = 500;//ms
		Client->SendTimeout = 500;//ms
		Client->ReceiveBufferSize = 1024;
		Client->SendBufferSize = 1024;

		SendData = gcnew array<unsigned char>(16);
		ReadData = gcnew array<unsigned char>(2500);
		System::String^ ResponseData;

		NetworkStream^ Stream = Client->GetStream();
		SM_GPS GPSDATASTRUCT;
		unsigned char* BytePtr;
		unsigned long data_length;

		while (!gps.getShutdownFlag()) {
			gps.setHeartbeat(MAX_WAIT_CYCLES);
			System::Threading::Thread::Sleep(20);
			if (Stream->DataAvailable) {

				data_length = Stream->Read(ReadData, 0, ReadData->Length);

				BytePtr = (unsigned char*)(&GPSDATASTRUCT);
				for (int i = 0; i < sizeof(SM_GPS); i++) {
					BytePtr[i] = ReadData[i];
				}
				std::cout << std::hex << "length: " << data_length << std::endl;
				std::cout << std::hex << "Expected CRC " << GPSDATASTRUCT.Checksum << std::endl;
				if (GPSDATASTRUCT.Checksum == CalculateBlockCRC32(data_length - 4, BytePtr) && *(BytePtr) == 0xAA) {
					std::cout << "Northing: " << GPSDATASTRUCT.Northing << std::endl;
					std::cout << "Easting: " << GPSDATASTRUCT.Easting << std::endl;
					std::cout << "Height: " << GPSDATASTRUCT.Height << std::endl;
					std::cout << "Calculated CRC: " << GPSDATASTRUCT.Checksum << std::endl;
					Console::WriteLine();
				}
				else {
					Console::WriteLine("Invalid Data");
				}

			}
		}

		// GPS CODE HERE ^^^
		Thread::Sleep(25);
	}
	return 0;
}