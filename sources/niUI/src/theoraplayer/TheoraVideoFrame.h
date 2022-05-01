/************************************************************************************
This source file is part of the Theora Video Playback Library
For latest info, see http://libtheoraplayer.googlecode.com
*************************************************************************************
Copyright (c) 2008-2014 Kresimir Spes (kspes@cateia.com)
This program is free software; you can redistribute it and/or modify it under
the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
*************************************************************************************/
#ifndef _TheoraVideoFrame_h
#define _TheoraVideoFrame_h

#include "../stdafx.h"
#include "TheoraVideoClip.h"

struct TheoraPixelTransform;
/**

*/
class TheoraVideoFrame
{
protected:
	TheoraVideoClip* mParent;
	unsigned char* mBuffer;
	unsigned long mFrameNumber;
  ni::Ptr<ni::iBitmap2D> mptrBitmap;

public:
	//! global time in seconds this frame should be displayed on
	float mTimeToDisplay;
	//! whether the frame is ready for display or not
	bool mReady;
	//! indicates the frame is being used by TheoraWorkerThread instance
	bool mInUse;
	//! used to keep track of linear time in looping videos
	int mIteration;

	int mBpp;

	TheoraVideoFrame(TheoraVideoClip* parent);
	virtual ~TheoraVideoFrame();

	//! internal function, do not use directly
	void _setFrameNumber(unsigned long number) { mFrameNumber = number; }
	//! returns the frame number of this frame in the theora stream
	unsigned long getFrameNumber() { return mFrameNumber; }

	void clear();

	int getWidth();
	int getStride();
	int getHeight();

	unsigned char* getBuffer();
  iBitmap2D* getBitmap(ni::iGraphics* apGraphics, const char* aPixelFormat);

	//! Called by TheoraVideoClip to decode a source buffer onto itself
	virtual void decode(struct TheoraPixelTransform* t);
};
#endif
