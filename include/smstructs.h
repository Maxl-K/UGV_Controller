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
};

struct SM_VehicleControl
{
	double Speed;
	double Steering;
};

struct SM_GPS
{
	double northing;
	double easting;
	double height;
};

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
