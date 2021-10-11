#define MAX_WAIT_CYCLES 100
#include <zmq.hpp>
#include <Windows.h>

#include "SMStructs.h"
#include "SMFcn.h"
#include "SMObject.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <turbojpeg.h>

void display();
void idle();

GLuint tex;

//ZMQ settings
zmq::context_t context(1);
zmq::socket_t subscriber(context, ZMQ_SUB);

// Other Initialisations and SM
__int64 Frequency, Counter = 0;
int WaitCounter = 0;
SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
SMObject TStamps(TEXT("TStamps"), sizeof(TimeStamps));
TimeStamps* TSData = (TimeStamps*)TStamps.pData;
ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;

int main(int argc, char** argv)
{
	//SM Creation and seeking access
	//Process Management
	PMObj.SMAccess();
	TStamps.SMAccess();

	//Part of Windows.h 
	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);

	//Define window size
	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;

	//GL Window setup
	glutInit(&argc, (char**)(argv));
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("MTRN3500 - Camera");

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glGenTextures(1, &tex);

	//Socket to talk to server
	subscriber.connect("tcp://192.168.1.200:26000");
	subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

	glutMainLoop();

	return 1;
}


void display()
{
	//Set camera as gl texture
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);

	//Map Camera to window
	glBegin(GL_QUADS);
	glTexCoord2f(0, 1); glVertex2f(-1, -1);
	glTexCoord2f(1, 1); glVertex2f(1, -1);
	glTexCoord2f(1, 0); glVertex2f(1, 1);
	glTexCoord2f(0, 0); glVertex2f(-1, 1);
	glEnd();
	glutSwapBuffers();
}
void idle()
{

	//Heartbeats and Timestamps

	TimeStamps* TSData = (TimeStamps*)TStamps.pData;
	ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;

	QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
	TSData->CameraTimestamp = (double)Counter / (double)Frequency * 1000; // ms
	Console::WriteLine("Camera time stamp    : {0,12:F3}", TSData->CameraTimestamp);

	if (PMData->Heartbeat.Flags.Camera == 0) {
		PMData->Heartbeat.Flags.Camera = 1;
		//std::cout << "Camera: " << (int)(PMData->Heartbeat.Flags.Camera) << std::endl;
		WaitCounter = 0;
	}
	else {
		if (WaitCounter++ > MAX_WAIT_CYCLES)
		{
			std::cout << "PM Shutdown Requested: " << (int)(PMData->Heartbeat.Status) << std::endl;
			PMData->Shutdown.Status = 0xFF;
		}
	}
	if (PMData->Shutdown.Flags.Camera == 1)
	{
		//Console::ReadKey();
		exit(0);
	}

	//End Hearbeats and Timestamps

	//receive from zmq
	zmq::message_t update;
	if (subscriber.recv(&update, ZMQ_NOBLOCK))
	{
		//Receive camera data
		long unsigned int _jpegSize = update.size();
		std::cout << "received " << _jpegSize << " bytes of data\n";
		unsigned char* _compressedImage = static_cast<unsigned char*>(update.data());
		int jpegSubsamp = 0, width = 0, height = 0;

		//JPEG Decompression
		tjhandle _jpegDecompressor = tjInitDecompress();
		tjDecompressHeader2(_jpegDecompressor, _compressedImage, _jpegSize, &width, &height, &jpegSubsamp);
		unsigned char* buffer = new unsigned char[width * height * 3]; //!< will contain the decompressed image
		printf("Dimensions:  %d   %d\n", height, width);
		tjDecompress2(_jpegDecompressor, _compressedImage, _jpegSize, buffer, width, 0/*pitch*/, height, TJPF_RGB, TJFLAG_FASTDCT);
		tjDestroy(_jpegDecompressor);

		//load texture
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, buffer);
		delete[] buffer;
	}

	display();
}

