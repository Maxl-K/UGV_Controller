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

protected:
	// YOUR CODE HERE (ADDITIONAL MEMBER VARIABLES THAT YOU MAY WANT TO ADD)
	int WaitCounter = 0;
	ProcessManagement* PMData = nullptr;
};