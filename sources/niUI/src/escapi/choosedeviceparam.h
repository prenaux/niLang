#ifndef __CHOOSEDEVICEPARAM_H_F1285117_79AC_485D_AB1B_3A00776F2BE5__
#define __CHOOSEDEVICEPARAM_H_F1285117_79AC_485D_AB1B_3A00776F2BE5__

struct ChooseDeviceParam
{
	IMFActivate **mDevices;    // Array of IMFActivate pointers.
	UINT32      mCount;          // Number of elements in the array.
	UINT32      mSelection;      // Selected device, by array index.

	~ChooseDeviceParam()
	{
		unsigned int i;
		for (i = 0; i < mCount; i++)
		{
			if (mDevices[i])
				mDevices[i]->Release();
		}
		CoTaskMemFree(mDevices);
	}
};

#endif // __CHOOSEDEVICEPARAM_H_F1285117_79AC_485D_AB1B_3A00776F2BE5__
