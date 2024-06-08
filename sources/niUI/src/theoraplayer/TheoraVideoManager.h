/************************************************************************************
This source file is part of the Theora Video Playback Library
For latest info, see http://libtheoraplayer.googlecode.com
*************************************************************************************
Copyright (c) 2008-2014 Kresimir Spes (kspes@cateia.com)
This program is free software; you can redistribute it and/or modify it under
the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
*************************************************************************************/

#ifndef _TheoraVideoManager_h
#define _TheoraVideoManager_h

#include "TheoraVideoClip.h"

TheoraVideoClip* Theora_createVideoClip(ni::iFile* data_source,
                                        TheoraOutputMode output_mode,
                                        int numPrecachedOverride,
                                        bool usePower2Stride,
                                        bool decodeFirstFirstFrame = true);
void Theora_destroyVideoClip(TheoraVideoClip* clip);

#endif
