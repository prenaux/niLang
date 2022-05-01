/************************************************************************************
This source file is part of the Theora Video Playback Library
For latest info, see http://libtheoraplayer.googlecode.com
*************************************************************************************
Copyright (c) 2008-2014 Kresimir Spes (kspes@cateia.com)
This program is free software; you can redistribute it and/or modify it under
the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
*************************************************************************************/
#if defined(__FFMPEG) && !defined(_VideoStream_FFmpeg_h)
#define _VideoStream_FFmpeg_h

#include "TheoraAudioPacketQueue.h"
#include "TheoraVideoClip.h"

struct AVFormatContext;
struct AVCodecContext;
struct AVCodec;
struct AVFrame;
struct AVIOContext;

class VideoStream_FFmpeg : public TheoraVideoClip, public TheoraAudioPacketQueue
{
protected:
  bool mLoaded;

  AVFormatContext* mFormatContext;
  AVCodecContext* mCodecContext;
  AVIOContext* mAvioContext;
  AVCodec* mCodec;
  AVFrame* mFrame;
  ni::cString mstrUrl;

  int mVideoStreamIndex;
  int mFrameNumber;

  void unload();
  void doSeek();
public:
  VideoStream_FFmpeg(ni::iFile* data_source,
                     TheoraOutputMode output_mode,
                     int nPrecachedFrames,
                     bool usePower2Stride);
  ~VideoStream_FFmpeg();

  bool _readData();
  bool decodeNextFrame();
  void _restart();
  bool load(ni::iFile* source);
  float decodeAudio();
  void decodedAudioCheck();
  astl::string getDecoderName() { return "FFmpeg"; }
};

#endif
