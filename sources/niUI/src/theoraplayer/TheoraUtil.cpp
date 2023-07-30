/************************************************************************************
This source file is part of the Theora Video Playback Library
For latest info, see http://libtheoraplayer.googlecode.com
*************************************************************************************
Copyright (c) 2008-2014 Kresimir Spes (kspes@cateia.com)
This program is free software; you can redistribute it and/or modify it under
the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
*************************************************************************************/
#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#include <pthread.h>
#endif

#include "TheoraUtil.h"

#ifdef _WIN32
#include <windows.h>
#pragma warning( disable: 4996 ) // MSVC++
#endif

astl::string str(int i)
{
	char s[32];
	sprintf(s, "%d", i);
	return astl::string(s);
}

astl::string strf(float i)
{
	char s[32];
	sprintf(s, "%.3f", i);
	return astl::string(s);
}

void _psleep(int miliseconds)
{
#ifdef _WIN32
	Sleep(miliseconds);
#else
	usleep(miliseconds * 1000);
#endif
}


int _nextPow2(int x)
{
	int y;
	for (y = 1; y < x; y *= 2);
	return y;
}
