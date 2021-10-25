#pragma once

#ifndef SMSTRUCTS_H
#define SMSTRUCTS_H

#using <System.dll>
#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <iostream>
#include <conio.h>

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;


#define STANDARD_LASER_LENGTH 361

struct SM_Laser
{
	double x[STANDARD_LASER_LENGTH];
	double y[STANDARD_LASER_LENGTH];
	int PointCloudSize;
};

struct SM_VehicleControl
{
	double Speed;
	double Steering;
};

#pragma pack(1)
struct SM_GPS
{
	unsigned int Header;
	unsigned char garbage[40];
	double Northing;
	double Easting;
	double Height;
	unsigned char garbage2[40];
	unsigned int Checksum;
};
#pragma pack()

struct UnitFlags
{
	unsigned char	Laser : 1,				//NONCRITICAL
					Display : 1,			//NONCRITICAL
					Vehicle : 1,			//NONCRITICAL
					GPS : 1,				//NONCRITICAL
					Camera : 1,				//NONCRITICAL
					PM : 1,					//CRITICAL
					Garbage : 2;
};

union ExecFlags
{
	UnitFlags Flags;
	unsigned short Status;
};

struct ProcessManagement
{
	ExecFlags Heartbeat;
	ExecFlags Shutdown;
	long int LifeCounter;
	bool ready;
};

struct TimeStamps
{
	double GPSTimeStamp;
	double IMUTimeStamp;
	double LaserTimeStamp;
	double VehicleTimeStamp;
	double PMTimeStamp;
	double DisplayTimestamp;
	double CameraTimestamp;
};

#define NONCRITICALMASK 0xff	//0 011 0000
#define CRITICALMASK 0x0		//0 100 1111
#endif
