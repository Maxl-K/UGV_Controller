#include "GPS.h"
#include <Windows.h>

int GPS::connect(String^ hostName, int portNumber)
{
	Client = gcnew TcpClient(Ip, PortNumber);
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;//ms
	Client->SendTimeout = 500;//ms
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	ReadData = gcnew array<unsigned char>(2500);
	Stream = Client->GetStream();
	return SUCCESS;
}
int GPS::setupSharedMemory()
{
	ProcessManagementData = new SMObject(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	while (ProcessManagementData->SMAccess());
	if (ProcessManagementData->SMAccessError) {
		Console::WriteLine("Shared memory access failed for PM");
		return ERROR;
	}
	PMData = (ProcessManagement*)ProcessManagementData->pData;

	ProcessManagementData = new SMObject (_TEXT("GPSObj"), sizeof(SM_GPS));
	while (ProcessManagementData->SMAccess());
	if (ProcessManagementData->SMAccessError) {
		Console::WriteLine("Shared memory access failed for GPS");
		return ERROR;
	}
	GPSData = (SM_GPS*)ProcessManagementData->pData;
	Console::WriteLine("Shared memory created successfully.");
	return SUCCESS;
}
int GPS::getData()
{
	SM_GPS GPSDATASTRUCT;
	GPSDATAPtr = (&GPSDATASTRUCT);

	if (Stream->DataAvailable) {

		data_length = Stream->Read(ReadData, 0, ReadData->Length);
		//BytePtr = (unsigned char*)(&GPSDATASTRUCT);
		BytePtr = (unsigned char*)GPSData;
		// ^^^
		for (int i = 0; i < sizeof(SM_GPS); i++) {
			*(BytePtr++) = ReadData[i];
		}

		if (GPSData == nullptr) {
			Console::WriteLine("Nullptr");
		}
		if (BytePtr == nullptr) {
			Console::WriteLine("Nullptr");
		}
		//std::cout << "CRC: " << GPSData->Checksum << std::endl;
		//std::cout << "Northing: " << GPSData->Checksum << std::endl;
		//std::cout << "Northing: " << GPSData->Northing << std::endl;
		//std::cout << "Easting: " << GPSData->Easting << std::endl;

		//std::cout << std::hex << "length: " << data_length << std::endl;

		//std::cout << std::hex << "Expected CRC " << GPSData->Checksum << std::endl;

		/*
		if (GPSDATAPtr->Checksum == CalculateBlockCRC32(*data_length_ptr - 4, BytePtr) && *(BytePtr) == 0xAA) {
			std::cout << "Northing: " << GPSDATAPtr->Northing << std::endl;
			std::cout << "Easting: " << GPSDATAPtr->Easting << std::endl;
			std::cout << "Height: " << GPSDATAPtr->Height << std::endl;
			std::cout << "Calculated CRC: " << GPSDATAPtr->Checksum << std::endl;
			Console::WriteLine();
		}
		else {
			Console::WriteLine("Invalid Data");
		}
		*/
	}
	return SUCCESS;
}
int GPS::checkData()
{
	//if (GPSData->Checksum == CalculateBlockCRC32(data_length - 4, BytePtr) && *(BytePtr) == 0xAA) {
	std::cout << "Calculated CRC: " << CalculateBlockCRC32(108, (unsigned char*)GPSData) << std::endl;
	if (GPSData->Checksum != CalculateBlockCRC32(108, (unsigned char*)GPSData)) {
		Console::WriteLine("Invalid Data");
		return ERR_INVALID_DATA;
	}
	else if (*(unsigned char*)(GPSData) != 0xAA){
		Console::WriteLine("Invalid Data");
		return ERR_INVALID_DATA;
	}
	return SUCCESS;
}
int GPS::sendDataToSharedMemory()
{
	std::cout << "Northing: " << GPSData->Northing << std::endl;
	std::cout << "Easting: " << GPSData->Easting << std::endl;
	std::cout << "Height: " << GPSData->Height << std::endl;
	Console::WriteLine();

	//GPSData->Northing = GPSDATAPtr->Northing;
	//GPSData->Easting = GPSDATAPtr->Easting;
	//GPSData->Height = GPSDATAPtr->Height;
	//GPSData->Checksum = GPSDATAPtr->Checksum;
	return SUCCESS;
}
bool GPS::getShutdownFlag()
{
	bool shutdown = FALSE;
	try {
		if (PMData->Shutdown.Flags.GPS == 1) {
			shutdown = TRUE;
		}
	}
	catch (Exception^) {
		Console::WriteLine("Failed");
	}
	return shutdown;
}
int GPS::setHeartbeat(int maxWaitCycles)
{
	if (PMData->Heartbeat.Flags.GPS == 0) {
		PMData->Heartbeat.Flags.GPS = 1;
		WaitCounter = 0;
	};
	if (PMData->Heartbeat.Flags.GPS == 1) {

		if (WaitCounter++ > maxWaitCycles)
		{
			PMData->Shutdown.Status = 0xFF;
		}
	}
	return 1;
}

GPS::~GPS()
{
	Stream->Close();
	Client->Close();
}

unsigned long CRC32Value(int i)
{
	int j;
	unsigned long ulCRC;
	ulCRC = i;
	for (j = 8; j > 0; j--)
	{
		if (ulCRC & 1)
			ulCRC = (ulCRC >> 1) ^ CRC32_POLYNOMIAL;
		else
			ulCRC >>= 1;
	}
	return ulCRC;
}

unsigned long CalculateBlockCRC32(unsigned long ulCount, /* Number of bytes in the data block */
	unsigned char* ucBuffer) /* Data block */
{
	unsigned long ulTemp1;
	unsigned long ulTemp2;
	unsigned long ulCRC = 0;
	while (ulCount-- != 0)
	{
		ulTemp1 = (ulCRC >> 8) & 0x00FFFFFFL;
		ulTemp2 = CRC32Value(((int)ulCRC ^ *ucBuffer++) & 0xff);
		ulCRC = ulTemp1 ^ ulTemp2;
	}
	return(ulCRC);
}