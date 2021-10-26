#include "Laser.h"
#include <Windows.h>

int Laser::connect(String^ hostName, int portNumber)
{
	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(2500);
	// Creat TcpClient object and connect to it
	try {
		Console::WriteLine("Attempting to connect to LIDAR scanner.");
		Client = gcnew TcpClient(hostName, portNumber);
		Console::WriteLine("Connection to scanner successful.");
	}
	catch (Exception^) {
		Console::WriteLine("Connection to scanner unsuccessful. Connection timeout.");
		return ERROR;
	}
	// Configure connection
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;//ms
	Client->SendTimeout = 500;//ms
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	Stream = Client->GetStream();

	//Authentication
	System::String^ Zid = gcnew System::String("5265207\n");
	SendData = System::Text::Encoding::ASCII->GetBytes(Zid);
	Stream->Write(SendData, 0, SendData->Length);
	//wait for server
	System::Threading::Thread::Sleep(10);
	Stream->Read(ReadData, 0, ReadData->Length);

	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
	//Validate Response
	if (ResponseData != "OK") {
		//Error?
		Console::WriteLine("Authentication failed. Response Value: " + ResponseData);
		//return ERR_INVALID_DATA;
	}
	return SUCCESS;
}
int Laser::setupSharedMemory()
{
	ProcessManagementData = new SMObject(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	while (ProcessManagementData->SMAccess());
	if (ProcessManagementData->SMAccessError) {
		Console::WriteLine("Shared memory access failed for Vehicle");
		return ERROR;
	}
	PMData = (ProcessManagement*)ProcessManagementData->pData;

	ProcessManagementData = new SMObject(_TEXT("Laserobj"), sizeof(SM_Laser));
	while (ProcessManagementData->SMAccess());
	if (ProcessManagementData->SMAccessError) {
		Console::WriteLine("Shared memory access failed for Vehicle");
		return ERROR;
	}
	LaserData = (SM_Laser*)ProcessManagementData->pData;
	Console::WriteLine("Shared memory created successfully.");
	return SUCCESS;
}
int Laser::getData()
{
	// Convert string command to an array of unsigned char
	SendData = System::Text::Encoding::ASCII->GetBytes(AskScan);
	// Write command asking for data
	Stream->WriteByte(0x02);
	Stream->Write(SendData, 0, SendData->Length);
	Stream->WriteByte(0x03);
	// Wait for the server to prepare the data, 1 ms would be sufficient, but used 10 ms
	System::Threading::Thread::Sleep(10);
	// Read the incoming data
	Stream->Read(ReadData, 0, ReadData->Length);
	// Convert incoming data from an array of unsigned char bytes to an ASCII string
	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
	// Print the received string on the screen
	//Console::WriteLine(ResponseData);
	Value = ResponseData->Split();
	return SUCCESS;
}
int Laser::checkData()
{
	if (Value->Length <= 26 + 361) {
		Console::WriteLine("Not enough data. Length recieved: " + Value->Length);
		return ERR_INVALID_DATA;
	}
	if (Value[1] != "LMDscandata") {
		Console::WriteLine("Unexpected header. Header recieved: " + Value[1]);
		return ERR_INVALID_DATA;
	}
	if (System::Convert::ToInt32(Value[25]) == 361) {
		Console::WriteLine("Unexpected data length.");
		return ERR_INVALID_DATA;
	}
	return SUCCESS;
}
int Laser::sendDataToSharedMemory()
{
	array<double>^ Range;
	array<double>^ X;
	array<double>^ Y;
	double Angle, Resolution;
	int PointCloudSize;

	Angle = System::Convert::ToInt32(Value[23], 16.0);
	Resolution = System::Convert::ToInt32(Value[24], 16) / 10000.0;
	PointCloudSize = System::Convert::ToInt32(Value[25], 16);

	Range = gcnew array<double>(PointCloudSize);
	X = gcnew array<double>(PointCloudSize);
	Y = gcnew array<double>(PointCloudSize);

	if (Value->Length < 26 + PointCloudSize) {
		return ERR_NO_DATA;
	}

	std::cout << "Starting angle: " << Angle << std::endl;
	std::cout << "Resolution: " << Resolution << std::endl;
	std::cout << "Number of points: " << PointCloudSize << std::endl;

	LaserData->PointCloudSize = PointCloudSize;
	for (int i = 0; i < PointCloudSize; i++) {
		Range[i] = System::Convert::ToInt32(Value[26 + i], 16);
		X[i] = (Range[i] * Math::Sin(i * Resolution * Math::PI / 180.0));
		Y[i] = (-Range[i] * Math::Cos(i * Resolution * Math::PI / 180.0));
		Console::WriteLine("Point " + i + " X: " + X[i] + "  Y: " + Y[i]);
		LaserData->x[i] = X[i];
		LaserData->y[i] = Y[i];
	}

	return SUCCESS;
}
bool Laser::getShutdownFlag()
{
	bool shutdown = FALSE;
	try {
		if (PMData->Shutdown.Flags.Laser == 1) {
			shutdown = TRUE;
		}
	}
	catch (Exception^) {
		Console::WriteLine("Failed");
	}
	return shutdown;
}
int Laser::setHeartbeat(int maxWaitCycles)
{
	if (PMData->Heartbeat.Flags.Laser == 0) {
		PMData->Heartbeat.Flags.Laser = 1;
		WaitCounter = 0;
	};
	if (PMData->Heartbeat.Flags.Laser == 1) {

		if (WaitCounter++ > maxWaitCycles)
		{
			PMData->Shutdown.Status = 0xFF;
		}
	}
	return SUCCESS;
}

Laser::~Laser()
{
	// YOUR CODE HERE
	Stream->Close();
	Client->Close();
}