#pragma once
#include <UGV_module.h>
#include <smstructs.h>
#include <Windows.h>

ref class Laser : public UGV_module
{

public:
	int connect(String^ hostName, int portNumber) override;
	int setupSharedMemory() override;
	int getData() override;
	int checkData() override;
	int sendDataToSharedMemory() override;
	bool getShutdownFlag() override;
	int setHeartbeat(int maxWaitCycles) override;
	~Laser();
	int PortNumber = 23000;
	System::String^ Ip = gcnew System::String("192.168.1.200");

protected:
	// YOUR CODE HERE (ADDITIONAL MEMBER VARIABLES THAT YOU MAY WANT TO ADD)
	int WaitCounter = 0;
	ProcessManagement* PMData = nullptr;
	SM_Laser* LaserData = nullptr;
	TcpClient^ Client = nullptr;
	String^ AskScan = gcnew String("sRN LMDscandata");
	String^ ResponseData;
	array<unsigned char>^ SendData;
	array<unsigned char>^ ReadData;
	NetworkStream^ Stream = nullptr;

	array<String^>^ Value = nullptr;
};