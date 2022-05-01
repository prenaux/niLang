#include "windows.h"
#define ESCAPI_DEFINITIONS_ONLY
#include "escapi.h"

#define MAXDEVICES 16

extern struct SimpleCapParams gParams[];
extern int gDoCapture[];

extern HRESULT InitDevice(int device, int aPreferredMode);
extern void CleanupDevice(int device);
extern int CountCaptureDevices();
extern void GetCaptureDeviceName(int deviceno, char * namebuffer, int bufferlength);
extern void CheckForFail(int device);
extern int GetErrorCode(int device);
extern int GetErrorLine(int device);
extern float GetProperty(int device, int prop);
extern int GetPropertyAuto(int device, int prop);
extern int SetProperty(int device, int prop, float value, int autoval);

extern "C" void ESCAPI_init()
{
	CoInitialize(NULL);
}

extern "C" void ESCAPI_getCaptureDeviceName(unsigned int deviceno, char *namebuffer, int bufferlength)
{
	if (deviceno > MAXDEVICES)
		return;

	GetCaptureDeviceName(deviceno, namebuffer, bufferlength);
}

extern "C" int ESCAPI_countCaptureDevices()
{
	int c = CountCaptureDevices();
	return c;
}

extern "C" int ESCAPI_initCapture(unsigned int deviceno, int aPreferredMode, struct SimpleCapParams *aParams)
{
	if (deviceno > MAXDEVICES)
		return 0;
	gDoCapture[deviceno] = 0;
	gParams[deviceno] = *aParams;
	if (FAILED(InitDevice(deviceno,aPreferredMode))) return 0;
	return 1;
}

extern "C" void ESCAPI_deinitCapture(unsigned int deviceno)
{
	if (deviceno > MAXDEVICES)
		return;
	CleanupDevice(deviceno);
}

extern "C" void ESCAPI_doCapture(unsigned int deviceno)
{
	if (deviceno > MAXDEVICES)
		return;
	CheckForFail(deviceno);
	gDoCapture[deviceno] = -1;
}

extern "C" int ESCAPI_isCaptureDone(unsigned int deviceno)
{
	if (deviceno > MAXDEVICES)
		return 0;
	CheckForFail(deviceno);
	if (gDoCapture[deviceno] == 1)
		return 1;
	return 0;
}

extern "C" int ESCAPI_getCaptureErrorLine(unsigned int deviceno)
{
	if (deviceno > MAXDEVICES)
		return 0;
	return GetErrorLine(deviceno);
}

extern "C" int ESCAPI_getCaptureErrorCode(unsigned int deviceno)
{
	if (deviceno > MAXDEVICES)
		return 0;
	return GetErrorCode(deviceno);
}

extern "C" float ESCAPI_getCapturePropertyValue(unsigned int deviceno, int prop)
{
	if (deviceno > MAXDEVICES)
		return 0;
	return GetProperty(deviceno, prop);
}

extern "C" int ESCAPI_getCapturePropertyAuto(unsigned int deviceno, int prop)
{
	if (deviceno > MAXDEVICES)
		return 0;
	return GetPropertyAuto(deviceno, prop);
}

extern "C" int ESCAPI_setCaptureProperty(unsigned int deviceno, int prop, float value, int autoval)
{
	if (deviceno > MAXDEVICES)
		return 0;
	return SetProperty(deviceno, prop, value, autoval);
}
