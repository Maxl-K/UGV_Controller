#include "Vehicle.h"
#include <Windows.h>

int Vehicle::connect(String^ hostName, int portNumber)
{
	Client = gcnew TcpClient("192.168.1.200", PortNumber);
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;//ms
	Client->SendTimeout = 500;//ms
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(2500);

	Stream = Client->GetStream();
	System::Threading::Thread::Sleep(10);

	System::String^ Zid = gcnew System::String("5265207\n");
	array<unsigned char>^ SendZid = gcnew array<unsigned char>(16);
	SendZid = System::Text::Encoding::ASCII->GetBytes(Zid);

	Stream->Write(SendZid, 0, SendZid->Length);

	return SUCCESS;
}
int Vehicle::setupSharedMemory()
{
	ProcessManagementData = new SMObject(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	while (ProcessManagementData->SMAccess());
	if (ProcessManagementData->SMAccessError) {
		Console::WriteLine("Shared memory access failed for Vehicle");
		return ERROR;
	}
	PMData = (ProcessManagement*)ProcessManagementData->pData;

	ProcessManagementData = new SMObject (_TEXT("VehicleObj"), sizeof(SM_VehicleControl));
	while (ProcessManagementData->SMAccess());
	if (ProcessManagementData->SMAccessError) {
		Console::WriteLine("Shared memory access failed for Vehicle");
		return ERROR;
	}
	VehicleData = (SM_VehicleControl*)ProcessManagementData->pData;
	return SUCCESS;
}
int Vehicle::getData()
{
	double speed = 0.0;
	speed = VehicleData->Speed;
	double steering = 0.0;
	steering = VehicleData->Steering;
	//std::cout << "Speed: " << speed << " Steering: " << steering << std::endl;

	Vehicle_flag = !Vehicle_flag;

	Message = "# " + steering.ToString("F3") + " " + speed.ToString("F3") + " " + Vehicle_flag.ToString("D1");
	Message = Message + " #";
	Console::WriteLine(Message);
	//Console::WriteLine();
	SendData = Encoding::ASCII->GetBytes(Message);
	Stream->Write(SendData, 0, SendData->Length);

	return SUCCESS;
}
int Vehicle::checkData()
{
	if (VehicleData == NULL) {
		return ERR_NO_DATA;
	}
	return SUCCESS;
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