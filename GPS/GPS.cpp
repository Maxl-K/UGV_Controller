#include "GPS.h"
#include <Windows.h>

int GPS::connect(String^ hostName, int portNumber)
{
	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(2500);

	try {
		Console::WriteLine("Attempting to connect to GPS.");
		Client = gcnew TcpClient(hostName, portNumber);
		Console::WriteLine("Connection to scanner successful.");
	}
	catch (Exception^) {
		Console::WriteLine("Connection to GPS unsuccessful. Connection timeout.");
		return ERROR;
	}

	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;//ms
	Client->SendTimeout = 500;//ms
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	Stream = Client->GetStream();
	return SUCCESS;
}
int GPS::setupSharedMemory()
{
	ProcessManagementData = new SMObject(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	while(ProcessManagementData->SMAccess());
	if (ProcessManagementData->SMAccessError) {
		Console::WriteLine("Shared memory access failed for PM");
		return ERROR;
	}
	PMData = (ProcessManagement*)ProcessManagementData->pData;
	ProcessManagementData = new SMObject(_TEXT("GPSObj"), sizeof(SM_GPS));
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
	unsigned long data_length_;
	if (Stream->DataAvailable) {
		data_length_ = Stream->Read(ReadData, 0, ReadData->Length);
		data_length = (unsigned long*)data_length_;

		BytePtr = (unsigned char*)(GPSDataStruct);
		for (int i = 0; i < sizeof(SM_GPS); i++) {
			BytePtr[i] = ReadData[i];
		}
		std::cout << std::hex << "GPS data received with CRC " << GPSDataStruct->Checksum << std::endl;
	}

	return SUCCESS;
}
int GPS::checkData()
{
	unsigned long Calculated_CRC = CalculateBlockCRC32(*data_length - 4, BytePtr);
	unsigned int Expected_CRC = GPSDataStruct->Checksum;
	if (Expected_CRC == Calculated_CRC && *(BytePtr) == 0xAA) {
		return SUCCESS;
	}
	else {
		Console::WriteLine("Invalid Data recieved. Header: ", *(BytePtr), " Expected: 0xAA", " Calculated CRC: ", Calculated_CRC, " Expected: ", Expected_CRC);
		return ERR_INVALID_DATA;
	}
	return SUCCESS;
}
int GPS::sendDataToSharedMemory()
{
	GPSData->Northing = GPSDataStruct->Northing;
	GPSData->Easting = GPSDataStruct->Easting;
	GPSData->Height = GPSDataStruct->Height;
	GPSData->Checksum = GPSDataStruct->Checksum;
	std::cout << "Northing: " << GPSData->Northing << std::endl;
	std::cout << "Easting: " << GPSData->Easting << std::endl;
	std::cout << "Height: " << GPSData->Height << std::endl;
	Console::WriteLine();
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