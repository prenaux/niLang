/************************************************************************************
This source file is part of the Theora Video Playback Library
For latest info, see http://libtheoraplayer.googlecode.com
*************************************************************************************
Copyright (c) 2008-2014 Kresimir Spes (kspes@cateia.com)
This program is free software; you can redistribute it and/or modify it under
the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
*************************************************************************************/
#include <niLang/Types.h>

#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <pthread.h>
#endif

#include "TheoraAsync.h"
#include "TheoraUtil.h"
#include "TheoraVideoManager.h"

#ifdef _WINRT
#include <wrl.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// Mutex
///////////////////////////////////////////////////////////////////////////////////////////////////

TheoraMutex::TheoraMutex()
{
#ifdef _WIN32
#ifndef _WINRT // WinXP does not have CreateTheoraMutexEx()
	mHandle = CreateMutex(0, 0, 0);
#else
	mHandle = CreateMutexEx(NULL, NULL, 0, SYNCHRONIZE);
#endif
#else
	mHandle = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init((pthread_mutex_t*)mHandle, 0);
#endif
}

TheoraMutex::~TheoraMutex()
{
#ifdef _WIN32
	CloseHandle(mHandle);
#else
	pthread_mutex_destroy((pthread_mutex_t*)mHandle);
	free((pthread_mutex_t*)mHandle);
	mHandle = NULL;
#endif
}

void TheoraMutex::lock()
{
#ifdef _WIN32
	WaitForSingleObjectEx(mHandle, INFINITE, FALSE);
#else
	pthread_mutex_lock((pthread_mutex_t*)mHandle);
#endif
}

void TheoraMutex::unlock()
{
#ifdef _WIN32
	ReleaseMutex(mHandle);
#else
	pthread_mutex_unlock((pthread_mutex_t*)mHandle);
#endif
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
