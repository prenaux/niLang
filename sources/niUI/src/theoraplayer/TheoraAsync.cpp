/************************************************************************************
This source file is part of the Theora Video Playback Library
For latest info, see http://libtheoraplayer.googlecode.com
*************************************************************************************
Copyright (c) 2008-2014 Kresimir Spes (kspes@cateia.com)
This program is free software; you can redistribute it and/or modify it under
the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
*************************************************************************************/
#include <niLang/Types.h>

#include "TheoraAsync.h"
#include "TheoraUtil.h"
#include "TheoraVideoManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Mutex
///////////////////////////////////////////////////////////////////////////////////////////////////

TheoraMutex::TheoraMutex() {
}

TheoraMutex::~TheoraMutex() {
}

void TheoraMutex::lock() {
  mMutex.ThreadLock();
}

void TheoraMutex::unlock() {
  mMutex.ThreadUnlock();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// ScopeLock
///////////////////////////////////////////////////////////////////////////////////////////////////

TheoraMutex::ScopeLock::ScopeLock(TheoraMutex* mutex, bool logUnhandledUnlocks) : mMutex(NULL)
{
	mLogUnhandledUnlocks = logUnhandledUnlocks;
	acquire(mutex);
}

TheoraMutex::ScopeLock::~ScopeLock()
{
	if (release() && mLogUnhandledUnlocks)
	{
		th_writelog("A mutex has been scope-unlocked automatically!");
	}
}

bool TheoraMutex::ScopeLock::acquire(TheoraMutex* mutex)
{
	if (mMutex == NULL && mutex != NULL)
	{
		mMutex = mutex;
		mMutex->lock();
		return true;
	}
	return false;
}

bool TheoraMutex::ScopeLock::release()
{
	if (mMutex != NULL)
	{
		mMutex->unlock();
		mMutex = NULL;
		return true;
	}
	return false;
}
