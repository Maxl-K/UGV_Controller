#pragma once
#include <UGV_module.h>
#include <smstructs.h>
//#include <Windows.h>

#define CRC32_POLYNOMIAL 0xEDB88320L

unsigned long CRC32Value(int i);
unsigned long CalculateBlockCRC32(unsigned long ulCount, unsigned char* ucBuffer);

ref class GPS : public UGV_module
{

public:
	int connect(String^ hostName, int portNumber) override;
	int setupSharedMemory() override;
	int getData() override;
	int checkData() override;
	int sendDataToSharedMemory() override;
	bool getShutdownFlag() override;
	int setHeartbeat(int maxWaitCycles) override;
	~GPS();
	int PortNumber = 24000;
	System::String^ Ip = gcnew System::String("192.168.1.200");

protected:
	// YOUR CODE HERE (ADDITIONAL MEMBER VARIABLES THAT YOU MAY WANT TO ADD)
	int WaitCounter = 0;
	ProcessManagement* PMData = nullptr;
	SM_GPS* GPSData = nullptr;
	SM_GPS* GPSDATAPtr = nullptr;
	unsigned long data_length;
	unsigned char* BytePtr;
};
