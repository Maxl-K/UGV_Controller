#pragma once
#include <UGV_module.h>
#include <smstructs.h>
#include <Windows.h>

ref class Vehicle : public UGV_module
{

public:
	int connect(String^ hostName, int portNumber) override;
	int setupSharedMemory() override;
	int getData() override;
	int checkData() override;
	int sendDataToSharedMemory() override;
	bool getShutdownFlag() override;
	int setHeartbeat(int maxWaitCycles) override;
	~Vehicle();
	int PortNumber = 25000;
	System::String^ Ip = gcnew System::String("192.168.1.200");

protected:
	// YOUR CODE HERE (ADDITIONAL MEMBER VARIABLES THAT YOU MAY WANT TO ADD)
	int WaitCounter = 0;
	ProcessManagement* PMData = nullptr;
	SM_VehicleControl* VehicleData = nullptr;
	array<unsigned char>^ SendData;
	System::String^ Message;
	int Vehicle_flag = 1;
};