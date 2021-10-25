//#define MAX_WAIT_CYCLES 100
//^^ for home computer
#define MAX_WAIT_CYCLES 100
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <map>

#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#include <GLUT/glut.h>
	#include <unistd.h>
	#include <sys/time.h>
#elif defined(WIN32)
	#include <Windows.h>
	#include <tchar.h>
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glut.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glut.h>
	#include <unistd.h>
	#include <sys/time.h>
#endif


#include "Camera.hpp"
#include "Ground.hpp"
#include "KeyManager.hpp"

#include "Shape.hpp"
#include "Vehicle.hpp"
#include "MyVehicle.hpp"

#include "Messages.hpp"
#include "HUD.hpp"

#include <SMObject.h>
#include <smstructs.h>
#using <System.dll>
#include <Windows.h>
#include <conio.h>

void display();
void reshape(int width, int height);
void idle();

void keydown(unsigned char key, int x, int y);
void keyup(unsigned char key, int x, int y);
void special_keydown(int keycode, int x, int y);
void special_keyup(int keycode, int x, int y);

void mouse(int button, int state, int x, int y);
void dragged(int x, int y);
void motion(int x, int y);

using namespace std;
using namespace scos;

// Used to store the previous mouse location so we
//   can calculate relative mouse movement.
int prev_mouse_x = -1;
int prev_mouse_y = -1;

// vehicle control related variables
Vehicle * vehicle = NULL;
double speed = 0;
double steering = 0;

// Other Initialisations and SM
__int64 Frequency, Counter = 0;
int WaitCounter = 0;
SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
SMObject TStamps(TEXT("TStamps"), sizeof(TimeStamps));
SMObject VehicleObj(_TEXT("VehicleObj"), sizeof(SM_VehicleControl));
SM_VehicleControl* VehicleData = (SM_VehicleControl*)VehicleObj.pData;
TimeStamps* TSData = (TimeStamps*)TStamps.pData;
ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;

//int _tmain(int argc, _TCHAR* argv[]) {
int main(int argc, char ** argv) {

	Console::WriteLine("Process Awake");

	//SM Creation and seeking access
	//Process Management
	while (PMObj.SMAccess());
	while (TStamps.SMAccess());
	while (VehicleObj.SMAccess());

	bool error = FALSE;
	error = error || (PMObj.SMAccessError);
	error = error || (TStamps.SMAccessError);
	error = error || (VehicleObj.SMAccessError);
	if (error) {
		Console::WriteLine("Shared memory access failed.");
		return -1;
	}

	//Part of Windows.h 
	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);

	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;

	glutInit(&argc, (char**)(argv));
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("MTRN3500 - GL");

	Camera::get()->setWindowDimensions(WINDOW_WIDTH, WINDOW_HEIGHT);

	glEnable(GL_DEPTH_TEST);

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);

	glutKeyboardFunc(keydown);
	glutKeyboardUpFunc(keyup);
	glutSpecialFunc(special_keydown);
	glutSpecialUpFunc(special_keyup);

	glutMouseFunc(mouse);
	glutMotionFunc(dragged);
	glutPassiveMotionFunc(motion);

	// -------------------------------------------------------------------------
	// Please uncomment the following line of code and replace 'MyVehicle'
	//   with the name of the class you want to show as the current 
	//   custom vehicle.
	// -------------------------------------------------------------------------
	vehicle = new MyVehicle();


	glutMainLoop();

	if (vehicle != NULL) {
		delete vehicle;
	}

	return 0;
}


void display() {
	// -------------------------------------------------------------------------
	//  This method is the main draw routine. 
	// -------------------------------------------------------------------------

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if(Camera::get()->isPursuitMode() && vehicle != NULL) {
		double x = vehicle->getX(), y = vehicle->getY(), z = vehicle->getZ();
		double dx = cos(vehicle->getRotation() * 3.141592765 / 180.0);
		double dy = sin(vehicle->getRotation() * 3.141592765 / 180.0);
		Camera::get()->setDestPos(x + (-3 * dx), y + 7, z + (-3 * dy));
		Camera::get()->setDestDir(dx, -1, dy);
	}
	Camera::get()->updateLocation();
	Camera::get()->setLookAt();

	Ground::draw();
	
	// draw my vehicle
	if (vehicle != NULL) {
		vehicle->draw();

	}

	SMObject LaserObj(_TEXT("Laserobj"), sizeof(SM_Laser));
	while (LaserObj.SMAccess());
	if (LaserObj.SMAccessError) {
		Console::WriteLine("Shared memory access failed for Laser");
	}
	SM_Laser* LData = (SM_Laser*)LaserObj.pData;
	vehicle->positionInGL();
	glTranslated(0.5, 0, 0);
	for (int i = 0; i < 361; i++) {
		double x = vehicle->getX(), y = vehicle->getY(), z = vehicle->getZ();
		glBegin(GL_LINES);
		glVertex3f(LData->x[i] / 1000, 0, -LData->y[i] / 1000);
		glVertex3f(LData->x[i] / 1000, 0.3, -LData->y[i] / 1000);
		glEnd();
	}

	// Display GPS Data?

	// draw HUD
	HUD::Draw();

	glutSwapBuffers();
};

void reshape(int width, int height) {

	Camera::get()->setWindowDimensions(width, height);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
};

double getTime()
{
#if defined(WIN32)
	LARGE_INTEGER freqli;
	LARGE_INTEGER li;
	if(QueryPerformanceCounter(&li) && QueryPerformanceFrequency(&freqli)) {
		return double(li.QuadPart) / double(freqli.QuadPart);
	}
	else {
		static ULONGLONG start = GetTickCount64();
		return (GetTickCount64() - start) / 1000.0;
	}
#else
	struct timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec + (t.tv_usec / 1000000.0);
#endif
}

void idle() {

	//Heartbeats and Timestamps

	TimeStamps* TSData = (TimeStamps*)TStamps.pData;
	ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;

	QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
	TSData->DisplayTimestamp = (double)Counter / (double)Frequency * 1000; // ms
	Console::WriteLine("Display time stamp    : {0,12:F3}", TSData->DisplayTimestamp);

	if (PMData->Heartbeat.Flags.Display == 0) {
		PMData->Heartbeat.Flags.Display = 1;
		//std::cout << "Display: " << (int)(PMData->Heartbeat.Flags.Display) << std::endl;
		WaitCounter = 0;
	}
	else  {
		if (WaitCounter++ > MAX_WAIT_CYCLES)
		{
			//std::cout << "PM Shutdown Requested: " << (int)(PMData->Heartbeat.Status) << std::endl;
			PMData->Shutdown.Status = 0xFF;
		}
	}
	if (PMData->Shutdown.Flags.Display == 1)
	{
		//Console::ReadKey();
		exit(0);
	}

	//End Hearbeats and Timestamps

	if (KeyManager::get()->isAsciiKeyPressed('a')) {
		Camera::get()->strafeLeft();
	}

	if (KeyManager::get()->isAsciiKeyPressed('c')) {
		Camera::get()->strafeDown();
	}

	if (KeyManager::get()->isAsciiKeyPressed('d')) {
		Camera::get()->strafeRight();
	}

	if (KeyManager::get()->isAsciiKeyPressed('s')) {
		Camera::get()->moveBackward();
	}

	if (KeyManager::get()->isAsciiKeyPressed('w')) {
		Camera::get()->moveForward();
	}

	if (KeyManager::get()->isAsciiKeyPressed(' ')) {
		Camera::get()->strafeUp();
	}

	speed = 0;
	steering = 0;

	if (KeyManager::get()->isSpecialKeyPressed(GLUT_KEY_LEFT)) {
		steering = Vehicle::MAX_LEFT_STEERING_DEGS * -1;   
	}

	if (KeyManager::get()->isSpecialKeyPressed(GLUT_KEY_RIGHT)) {
		steering = Vehicle::MAX_RIGHT_STEERING_DEGS * -1;
	}

	if (KeyManager::get()->isSpecialKeyPressed(GLUT_KEY_UP)) {
		speed = Vehicle::MAX_FORWARD_SPEED_MPS;
	}

	if (KeyManager::get()->isSpecialKeyPressed(GLUT_KEY_DOWN)) {
		speed = Vehicle::MAX_BACKWARD_SPEED_MPS;
	}

	SM_VehicleControl* VehicleData = (SM_VehicleControl*)VehicleObj.pData;

	const float sleep_time_between_frames_in_seconds = 0.025;

	static double previousTime = getTime();
	const double currTime = getTime();
	const double elapsedTime = currTime - previousTime;
	previousTime = currTime;

	// do a simulation step
	if (vehicle != NULL) {
		vehicle->update(speed, steering, elapsedTime);
		VehicleData->Speed = speed;
		VehicleData->Steering = steering;
	}

	display();

#ifdef _WIN32 
	Sleep(sleep_time_between_frames_in_seconds * 1000);
#else
	usleep(sleep_time_between_frames_in_seconds * 1e6);
#endif
};

void keydown(unsigned char key, int x, int y) {

	// keys that will be held down for extended periods of time will be handled
	//   in the idle function
	KeyManager::get()->asciiKeyPressed(key);

	// keys that react ocne when pressed rather than need to be held down
	//   can be handles normally, like this...
	switch (key) {
	case 27: // ESC key
		exit(0);
		break;      
	case '0':
		Camera::get()->jumpToOrigin();
		break;
	case 'p':
		Camera::get()->togglePursuitMode();
		break;
	}

};

void keyup(unsigned char key, int x, int y) {
	KeyManager::get()->asciiKeyReleased(key);
};

void special_keydown(int keycode, int x, int y) {

	KeyManager::get()->specialKeyPressed(keycode);

};

void special_keyup(int keycode, int x, int y) {  
	KeyManager::get()->specialKeyReleased(keycode);  
};

void mouse(int button, int state, int x, int y) {

};

void dragged(int x, int y) {

	if (prev_mouse_x >= 0) {

		int dx = x - prev_mouse_x;
		int dy = y - prev_mouse_y;

		Camera::get()->mouseRotateCamera(dx, dy);
	}

	prev_mouse_x = x;
	prev_mouse_y = y;
};

void motion(int x, int y) {

	prev_mouse_x = x;
	prev_mouse_y = y;
};


