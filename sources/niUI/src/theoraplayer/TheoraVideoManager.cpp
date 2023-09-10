/************************************************************************************
This source file is part of the Theora Video Playback Library
For latest info, see http://libtheoraplayer.googlecode.com
*************************************************************************************
Copyright (c) 2008-2014 Kresimir Spes (kspes@cateia.com)
This program is free software; you can redistribute it and/or modify it under
the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
*************************************************************************************/
#include <niLang/Types.h>
#include "TheoraVideoManager.h"
#include "TheoraVideoClip.h"
#include "TheoraFrameQueue.h"
#include "TheoraAudioInterface.h"
#include "TheoraUtil.h"
#ifdef __THEORA
	#include <theora/codec.h>
	#include <vorbis/codec.h>
	#include "Theora/TheoraVideoClip_Theora.h"
#endif
#ifdef __AVFOUNDATION
	#include "AVFoundation/TheoraVideoClip_AVFoundation.h"
#endif
#ifdef __FFMPEG
	#include "FFmpeg/TheoraVideoClip_FFmpeg.h"
	#include "FFmpeg/VideoStream_FFmpeg.h"
#endif
#ifdef _ANDROID //libtheoraplayer addition for cpu feature detection
	#include "cpu-features.h"
#endif

// declaring function prototype here so I don't have to put it in a header file
// it only needs to be used by this plugin and called once
extern "C" {
	void initYUVConversionModule();
}

static const int _knDefaultNumPrecachedFrames = 2;

#ifdef _DECODING_BENCHMARK
void benchmark(TheoraVideoClip* clip)
{
	int nPrecached = 256;
	int n = nPrecached;
	char msg[1024];
	clock_t t = clock();
	while (n > 0)
	{
		clip->waitForCache(1.0f, 1000000);
		n -= 32;
		clip->getFrameQueue()->clear();
	}
	float diff = ((float) (clock() - t) * 1000.0f) / CLOCKS_PER_SEC;
	sprintf(msg, "BENCHMARK: %s: Decoding %d frames took %.1fms (%.2fms average per frame)\n",clip->getName().cStr(), nPrecached, diff, diff / nPrecached);
	TheoraVideoManager::getSingleton().logMessage(msg);
	clip->seek(0);
}
#endif

TheoraVideoClip* Theora_createVideoClip(ni::iFile* data_source,
                                        TheoraOutputMode output_mode,
                                        int numPrecachedOverride,
                                        bool usePower2Stride,
                                        bool decodeFirstFirstFrame)
{
	TheoraVideoClip* clip = NULL;
	int nPrecached = numPrecachedOverride ? numPrecachedOverride : _knDefaultNumPrecachedFrames;
	niLog(Info,astl::string("Creating video from data source: ") + data_source->GetSourcePath() + " [" + str(nPrecached) + " precached frames].");

#ifdef __AVFOUNDATION
  // if the user has his own data source, it's going to be a problem for AVAssetReader since it only supports reading from files...
	if (!clip && ni::StrEndsWith(data_source->GetSourcePath(),".mp4")) {
		clip = new TheoraVideoClip_AVFoundation(data_source, output_mode, nPrecached, usePower2Stride);
	}
#endif
#ifdef __THEORA
	if (!clip &&
      (ni::StrEndsWith(data_source->GetSourcePath(),".ogv") ||
       ni::StrEndsWith(data_source->GetSourcePath(),".ogw")))
  {
		clip = new TheoraVideoClip_Theora(data_source, output_mode, nPrecached, usePower2Stride);
  }
#endif
#ifdef __FFMPEG
	if (!clip &&
      (ni::StrEndsWith(data_source->GetSourcePath(),".mp4") ||
       ni::StrEndsWith(data_source->GetSourcePath(),".mkv") ||
       ni::StrEndsWith(data_source->GetSourcePath(),".wmv")))
  {
		clip = new TheoraVideoClip_FFmpeg(data_source, output_mode, nPrecached, usePower2Stride);
  }

  if (!clip && ni::StrEndsWith(data_source->GetSourcePath(),".ipcam")) {
    clip = new VideoStream_FFmpeg(data_source, output_mode, nPrecached, usePower2Stride);
  }
#endif

  if (decodeFirstFirstFrame) {
    if (clip != NULL)
    {
      if (!clip->load(data_source)) {
        niError("Couldn't load the video clip.");
        delete clip;
        return NULL;
      }
      clip->decodeNextFrame(); // ensure the first frame is always preloaded and have the main thread do it to prevent potential thread starvation
    }
    else
    {
      niError(niFmt("Failed creating video clip: '%s'.", data_source->GetSourcePath()));
      return NULL;
    }
  }

#ifdef _DECODING_BENCHMARK
	benchmark(clip);
#endif
	return clip;
}

void Theora_destroyVideoClip(TheoraVideoClip* clip)
{
	if (clip) {
		th_writelog(_ASTR("Destroying video clip: ") + clip->getName());
		delete clip;
	}
}
