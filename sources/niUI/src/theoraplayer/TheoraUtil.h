/************************************************************************************
This source file is part of the Theora Video Playback Library
For latest info, see http://libtheoraplayer.googlecode.com
*************************************************************************************
Copyright (c) 2008-2014 Kresimir Spes (kspes@cateia.com)
This program is free software; you can redistribute it and/or modify it under
the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
*************************************************************************************/
#ifndef _TheoraUtil_h
#define _TheoraUtil_h

#include <niLang/STL/string.h>
#include <niLang/STL/vector.h>

#define foreach(type,lst) for (astl::vector<type>::iterator it=lst.begin();it != lst.end(); ++it)
#define foreach_l(type,lst) for (astl::list<type>::iterator it=lst.begin();it != lst.end(); ++it)

#define th_writelog(x) niLog(Info,x)

astl::string str(int i);
astl::string strf(float i);
void _psleep(int milliseconds);
int _nextPow2(int x);

#endif
