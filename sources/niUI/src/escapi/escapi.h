/* Extremely Simple Capture API */
struct SimpleCapParams
{
	/* Target buffer.
	 * Must be at least mWidth * mHeight * sizeof(int) of size!
	 */
	int * mTargetBuf;
	/* Buffer width */
	int mWidth;
	/* Buffer height */
	int mHeight;
};

enum CAPTURE_PROPETIES
{
	CAPTURE_BRIGHTNESS,
	CAPTURE_CONTRAST,
	CAPTURE_HUE,
	CAPTURE_SATURATION,
	CAPTURE_SHARPNESS,
	CAPTURE_GAMMA,
	CAPTURE_COLORENABLE,
	CAPTURE_WHITEBALANCE,
	CAPTURE_BACKLIGHTCOMPENSATION,
	CAPTURE_GAIN,
	CAPTURE_PAN,
	CAPTURE_TILT,
	CAPTURE_ROLL,
	CAPTURE_ZOOM,
	CAPTURE_EXPOSURE,
	CAPTURE_IRIS,
	CAPTURE_FOCUS,
	CAPTURE_PROP_MAX
};

/* should be called once to intialize ESCAPI */
extern "C" void ESCAPI_init();

/* return the number of capture devices found */
extern "C" int ESCAPI_countCaptureDevices();

/* initCapture tries to open the video capture device.
 *
 * aPreferredMode can be used to specify the actual resolution/mode of the webcam, if -1 is specified the modes will be enumerated and the resolution best matching what is specified in SimpleCapParams will be used.
 *
 * Returns 0 on failure, 1 on success.
 * Note: Capture parameter values must not change while capture device
 *       is in use (i.e. between initCapture and deinitCapture).
 *       Do *not* free the target buffer, or change its pointer!
 */
extern "C" int ESCAPI_initCapture(unsigned int deviceno, int aPreferredMode, struct SimpleCapParams *aParams);

/* deinitCapture closes the video capture device. */
extern "C" void ESCAPI_deinitCapture(unsigned int deviceno);

/* doCapture requests video frame to be captured. */
extern "C" void ESCAPI_doCapture(unsigned int deviceno);

/* isCaptureDone returns 1 when the requested frame has been captured.*/
extern "C" int ESCAPI_isCaptureDone(unsigned int deviceno);

/* Get the user-friendly name of a capture device. */
extern "C" void ESCAPI_getCaptureDeviceName(unsigned int deviceno, char *namebuffer, int bufferlength);

/*
	On properties -
	- Not all cameras support properties at all.
	- Not all properties can be set to auto.
	- Not all cameras support all properties.
	- Messing around with camera properties may lead to weird results, so YMMV.
*/

/* Gets value (0..1) of a camera property (see CAPTURE_PROPERTIES, above) */
extern "C" float ESCAPI_getCapturePropertyValue(unsigned int deviceno, int prop);
/* Gets whether the property is set to automatic (see CAPTURE_PROPERTIES, above) */
extern "C" int ESCAPI_getCapturePropertyAuto(unsigned int deviceno, int prop);
/* Set camera property to a value (0..1) and whether it should be set to auto. */
extern "C" int ESCAPI_setCaptureProperty(unsigned int deviceno, int prop, float value, int autoval);

/*
	All error situations in ESCAPI are considered catastrophic. If such should
	occur, the following functions can be used to check which line reported the
	error, and what the HRESULT of the error was. These may help figure out
	what the problem is.
*/

/* Return line number of error, or 0 if no catastrophic error has occurred. */
extern "C" int ESCAPI_getCaptureErrorLine(unsigned int deviceno);
/* Return HRESULT of the catastrophic error, or 0 if none. */
extern "C" int ESCAPI_getCaptureErrorCode(unsigned int deviceno);
