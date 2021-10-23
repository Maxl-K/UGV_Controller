#include "Vehicle.h"
#include <Windows.h>

int Vehicle::connect(String^ hostName, int portNumber)
{
	// YOUR CODE HERE
	return 1;
}
int Vehicle::setupSharedMemory()
{
	ProcessManagementData = new SMObject(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	ProcessManagementData->SMCreate();
	while (ProcessManagementData->SMAccess());
	if (ProcessManagementData->SMAccessError) {
		Console::WriteLine("Shared memory access failed for Vehicle");
	}
	PMData = (ProcessManagement*)ProcessManagementData->pData;
	return ERROR;
}
int Vehicle::getData()
{
	// YOUR CODE HERE
	return 1;
}
int Vehicle::checkData()
{
	// YOUR CODE HERE
	return 1;
}
int Vehicle::sendDataToSharedMemory()
{
	// YOUR CODE HERE
	return 1;
}
bool Vehicle::getShutdownFlag()
{
	bool shutdown = FALSE;
	try {
		if (PMData->Shutdown.Flags.Vehicle == 1) {
			shutdown = TRUE;
		}
	}
	catch (Exception^) {
		Console::WriteLine("Failed");
	}
	return shutdown;
}
int Vehicle::setHeartbeat(int maxWaitCycles)
{
	if (PMData->Heartbeat.Flags.Vehicle == 0) {
		PMData->Heartbeat.Flags.Vehicle = 1;
		WaitCounter = 0;
	};
	if (PMData->Heartbeat.Flags.Vehicle == 1) {

		if (WaitCounter++ > maxWaitCycles)
		{
			PMData->Shutdown.Status = 0xFF;
		}
	}
	return 1;
}

Vehicle::~Vehicle()
{
	// YOUR CODE HERE
}