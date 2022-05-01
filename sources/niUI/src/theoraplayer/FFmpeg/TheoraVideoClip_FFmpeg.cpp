/************************************************************************************
This source file is part of the Theora Video Playback Library
For latest info, see http://libtheoraplayer.googlecode.com
*************************************************************************************
Copyright (c) 2008-2014 Kresimir Spes (kspes@cateia.com)
This program is free software; you can redistribute it and/or modify it under
the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
*************************************************************************************/
#ifdef __FFMPEG
#include "TheoraAudioInterface.h"
#include "TheoraTimer.h"
#include "TheoraUtil.h"
#include "TheoraFrameQueue.h"
#include "TheoraVideoFrame.h"
#include "TheoraVideoManager.h"
#include "TheoraVideoClip_FFmpeg.h"
#include "TheoraPixelTransform.h"

#define READ_BUFFER_SIZE 4096
#define FFMPEG_LOG_LEVEL AV_LOG_WARNING

#ifdef __cplusplus
#define __STDC_CONSTANT_MACROS
#ifdef _STDINT_H
#undef _STDINT_H
#endif
# include <stdint.h>
#endif

// #define _FFMPEG_DEBUG

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avassert.h>
}

#include <niLang/IDataTable.h>

static bool ffmpegInitialised = 0;

static int readFunction(void* data, uint8_t* buf, int buf_size)
{
#ifdef _FFMPEG_DEBUG
	niWarning(_ASTR("reading ") + str(buf_size) + " bytes");
#endif

	ni::iFile* src = (ni::iFile*) data;
	return src->ReadRaw(buf, buf_size);
}

static int64_t seekFunction(void* data, int64_t offset, int whence)
{
#ifdef _FFMPEG_DEBUG
	niWarning(_ASTR("seeking: offset = ") + str((long) offset) + ", whence = " + str(whence));
#endif

	ni::iFile* src = (ni::iFile*) data;
	if (whence == AVSEEK_SIZE)
		return src->GetSize();
	else if (whence == SEEK_SET)
		src->SeekSet((long) offset);
	else if (whence == SEEK_END) {
		src->SeekSet(src->GetSize() - (long) offset);
  }
	return src->Tell();
}

static void avlog_theoraplayer(void* p, int level, const char* fmt, va_list vargs)
{
	static char logstr[2048];
  if (level < FFMPEG_LOG_LEVEL) {
    vsnprintf(logstr, sizeof(logstr), fmt, vargs);
    niWarning(_ASTR("ffmpeg: ") + astl::string(logstr));
  }
}


astl::string text;

static void _log(const char* s)
{
	text += s;
//	niWarning(text);
//	text = "";
}

static void _log(const char c)
{
	char s[2] = {c, 0};
	_log(s);
}

static const AVCodec *next_codec_for_id(enum AVCodecID id, const AVCodec *prev, int encoder)
{
	while ((prev = av_codec_next((AVCodec*)prev))) {
		if (prev->id == id &&
			(encoder ? av_codec_is_encoder(prev) : av_codec_is_decoder(prev)))
			return prev;
	}
	return NULL;
}

static int compare_codec_desc(const void *a, const void *b)
{
	const AVCodecDescriptor **da = (const AVCodecDescriptor **) a;
	const AVCodecDescriptor **db = (const AVCodecDescriptor **) b;

	return (*da)->type != (*db)->type ? (*da)->type - (*db)->type :
	strcmp((*da)->name, (*db)->name);
}

static unsigned int get_codecs_sorted(const AVCodecDescriptor ***rcodecs)
{
	const AVCodecDescriptor *desc = NULL;
	const AVCodecDescriptor **codecs;
	unsigned nb_codecs = 0, i = 0;

	while ((desc = avcodec_descriptor_next(desc)))
		++nb_codecs;
	if (!(codecs = (const AVCodecDescriptor**) av_calloc(nb_codecs, sizeof(*codecs)))) {
		av_log(NULL, AV_LOG_ERROR, "Out of memory\n");
		exit(1);
	}
	desc = NULL;
	while ((desc = avcodec_descriptor_next(desc)))
		codecs[i++] = desc;
	av_assert0(i == nb_codecs);
	qsort(codecs, nb_codecs, sizeof(*codecs), compare_codec_desc);
	*rcodecs = codecs;
	return nb_codecs;
}

static char get_media_type_char(enum AVMediaType type)
{
	switch (type) {
		case AVMEDIA_TYPE_VIDEO:    return 'V';
		case AVMEDIA_TYPE_AUDIO:    return 'A';
		case AVMEDIA_TYPE_DATA:     return 'D';
		case AVMEDIA_TYPE_SUBTITLE: return 'S';
		case AVMEDIA_TYPE_ATTACHMENT:return 'T';
		default:                    return '?';
	}
}

static void print_codecs_for_id(enum AVCodecID id, int encoder)
{
	const AVCodec *codec = NULL;

	_log(encoder ? "encoders" : "decoders");

	while ((codec = next_codec_for_id(id, codec, encoder)))
		_log(codec->name);

	_log(")");
}

int show_codecs(void *optctx, const char *opt, const char *arg)
{
	const AVCodecDescriptor **codecs;
	unsigned i, nb_codecs = get_codecs_sorted(&codecs);

	char tmp[1024];
	niWarning("Codecs:\n"
			" D..... = Decoding supported\n"
			" .E.... = Encoding supported\n"
			" ..V... = Video codec\n"
			" ..A... = Audio codec\n"
			" ..S... = Subtitle codec\n"
			" ...I.. = Intra frame-only codec\n"
			" ....L. = Lossy compression\n"
			" .....S = Lossless compression\n"
			" -------\n");
	for (i = 0; i < nb_codecs; ++i) {
		const AVCodecDescriptor *desc = codecs[i];
		const AVCodec *codec = NULL;

		_log(" ");
		_log(avcodec_find_decoder(desc->id) ? "D" : ".");
		_log(avcodec_find_encoder(desc->id) ? "E" : ".");

		_log(get_media_type_char(desc->type));
		_log((desc->props & AV_CODEC_PROP_INTRA_ONLY) ? "I" : ".");
		_log((desc->props & AV_CODEC_PROP_LOSSY)      ? "L" : ".");
		_log((desc->props & AV_CODEC_PROP_LOSSLESS)   ? "S" : ".");


		sprintf(tmp, " %-20s %s", desc->name, desc->long_name ? desc->long_name : "");

		_log(tmp);
		/* print decoders/encoders when there's more than one or their
		 * names are different from codec name */
		while ((codec = next_codec_for_id(desc->id, codec, 0))) {
			if (strcmp(codec->name, desc->name)) {
				print_codecs_for_id(desc->id, 0);
				break;
			}
		}
		codec = NULL;
		while ((codec = next_codec_for_id(desc->id, codec, 1))) {
			if (strcmp(codec->name, desc->name)) {
				print_codecs_for_id(desc->id, 1);
				break;
			}
		}
		_log("\n");
	}
	av_free(codecs);

	av_log(0, 0, "%s", text.c_str());
	return 0;
}

TheoraVideoClip_FFmpeg::TheoraVideoClip_FFmpeg(ni::iFile* data_source,
														 TheoraOutputMode output_mode,
														 int nPrecachedFrames,
														 bool usePower2Stride):
								 						 TheoraVideoClip(data_source, output_mode, nPrecachedFrames, usePower2Stride),
														 TheoraAudioPacketQueue()
{
  mFormatContext = NULL;
  mCodecContext = NULL;
  mAvioContext = NULL;
  mCodec = NULL;
  mFrame = NULL;
  mInputBuffer = NULL;
  mFrameNumber = 0;
	mVideoStreamIndex = -1;
  mLoaded = false;
}

TheoraVideoClip_FFmpeg::~TheoraVideoClip_FFmpeg()
{
	unload();
}

bool TheoraVideoClip_FFmpeg::load(ni::iFile* source)
{
	mVideoStreamIndex = -1;
	mFrameNumber = 0;
	AVDictionary* optionsDict = NULL;

	if (!ffmpegInitialised)
	{
#ifdef _FFMPEG_DEBUG
		niLog(Info,"Initializing ffmpeg");
		show_codecs(0, 0, 0);
#endif
		niLog(Info,_ASTR("avcodec version: ") + str(avcodec_version()));
		av_register_all();
    av_log_set_level(FFMPEG_LOG_LEVEL);
		av_log_set_callback(avlog_theoraplayer);
		ffmpegInitialised = 1;
	}

	mInputBuffer = (unsigned char*) av_malloc(READ_BUFFER_SIZE);

#ifdef _FFMPEG_DEBUG
	niWarning((mName + ": avio context created").Chars());
#endif

	mFormatContext = avformat_alloc_context();
#ifdef _FFMPEG_DEBUG
	niWarning(mName + ": avformat context created");
#endif

  ni::cString strFormatOpenInputUrl = "";

  if (ni::StrEndsWith(source->GetSourcePath(),".ipcam")) {
    ni::Ptr<ni::iDataTable> dt = ni::GetLang()->CreateDataTable("ipcam");
    if (!ni::GetLang()->SerializeDataTable("xml", ni::eSerializeMode_Read, dt, source)) {
      niWarning("ffmpeg: can't read ipcam datatable.");
      return false;
    }
    strFormatOpenInputUrl = dt->GetString("url");
    if (strFormatOpenInputUrl.empty()) {
      niWarning("ffmpeg: no url in ipcam datatable.");
      return false;
    }
    if (dt->GetBoolDefault("swap_rgb",ni::eFalse)) {
      this->setOutputMode(TH_BGRX);
    }
  }
  else {
    mAvioContext = avio_alloc_context(mInputBuffer, READ_BUFFER_SIZE, 0, source, &readFunction, NULL, &seekFunction);
    mFormatContext->pb = mAvioContext;
  }

	int err;
  char err_buffer[2048] = {0};
	if ((err = avformat_open_input(&mFormatContext, strFormatOpenInputUrl.c_str(), NULL, NULL)) != 0)
	{
		niWarning("ffmpeg: avformat input opening failed!");
		niWarning(_ASTR("ffmpeg: error_code: ") + str(err) + ": " + av_make_error_string(err_buffer, sizeof(err_buffer), err));
		return false;
	}

#ifdef _FFMPEG_DEBUG
	niWarning(mName + ": avformat input opened");
#endif

	// Retrieve stream information
	if (avformat_find_stream_info(mFormatContext, NULL) < 0) {
    niError("Couldn't find stream information");
		return false;
  }

#ifdef _FFMPEG_DEBUG
	niWarning(mName + ": got stream info");
#endif

	// Dump information about file onto standard error
	//	av_dump_format(mFormatContext, 0, "", 0);

	// Find the first video stream
	for (size_t i = 0; i < mFormatContext->nb_streams; ++i)
	{
		if(mFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			mVideoStreamIndex = (int)i;
			break;
		}
	}
	if (mVideoStreamIndex == -1) {
    niError("Didn't find a video stream.");
    return false;
  }

#ifdef _FFMPEG_DEBUG
	niWarning(mName + ": Found video stream at index " + str(mVideoStreamIndex));
#endif

	// Get a pointer to the codec context for the video stream
	mCodecContext = mFormatContext->streams[mVideoStreamIndex]->codec;

	// Find the decoder for the video stream
	mCodec = avcodec_find_decoder(mCodecContext->codec_id);
	if (mCodec == NULL)
	{
		niError("Codec not found.");
		return false;
	}
	// Open codec
	if(avcodec_open2(mCodecContext, mCodec, &optionsDict) < 0) {
    niError("Could not open codec.");
		return false;
  }

#ifdef _FFMPEG_DEBUG
	niWarning(mName + ": Codec opened");
#endif

	mFrame = av_frame_alloc();

#ifdef _FFMPEG_DEBUG
	niWarning(mName + ": Frame allocated");
#endif

	//AVRational rational = mCodecContext->time_base;

	mFPS = 25; //TODOOOOOO!!!

	mWidth = mStride = mCodecContext->width;
	mHeight = mCodecContext->height;
	mFrameDuration = 1.0f / mFPS;
	mDuration = mFormatContext->duration / AV_TIME_BASE;
  if (!strFormatOpenInputUrl.empty() || mDuration < 0) {
    mDuration = 0;
  }
	if (mFrameQueue == NULL) // todo - why is this set in the backend class? it should be set in the base class, check other backends as well
	{
		mFrameQueue = new TheoraFrameQueue(this);
		mFrameQueue->setSize(mNumPrecachedFrames);
	}

  return true;
}

void TheoraVideoClip_FFmpeg::unload()
{
	if (mInputBuffer)
	{
//		av_free(mInputBuffer);
		mInputBuffer = NULL;
	}
	if (mAvioContext)
	{
		av_free(mAvioContext);
		mAvioContext = NULL;
	}
	if (mFrame)
	{
		av_free(mFrame);
		mFrame = NULL;
	}
	if (mCodecContext)
	{
		avcodec_close(mCodecContext);
		mCodecContext = NULL;
	}
	if (mFormatContext)
	{
		avformat_close_input(&mFormatContext);
		mFormatContext = NULL;
	}
}

bool TheoraVideoClip_FFmpeg::_readData()
{
	return 1;
}

bool TheoraVideoClip_FFmpeg::decodeNextFrame()
{
	TheoraVideoFrame* frame = mFrameQueue->requestEmptyFrame();
	if (!frame) return 0;

	AVPacket packet;
	int frameFinished;

	while (av_read_frame(mFormatContext, &packet) >= 0)
	{
		if (packet.stream_index == mVideoStreamIndex)
		{
			avcodec_decode_video2(mCodecContext, mFrame, &frameFinished, &packet);

			if (frameFinished)
			{
				TheoraPixelTransform t;
				memset(&t, 0, sizeof(TheoraPixelTransform));

				t.y = mFrame->data[0]; t.yStride = mFrame->linesize[0];
				t.u = mFrame->data[1]; t.uStride = mFrame->linesize[1];
				t.v = mFrame->data[2]; t.vStride = mFrame->linesize[2];

				frame->decode(&t);
				frame->mTimeToDisplay = mFrameNumber / mFPS;
				frame->mIteration = mIteration;
				frame->_setFrameNumber(mFrameNumber++);

				av_free_packet(&packet);
				break;
			}
		}
		av_free_packet(&packet);
	}
	return 1;
}

void TheoraVideoClip_FFmpeg::decodedAudioCheck()
{
	if (!mAudioInterface || mTimer->isPaused()) return;

	TheoraMutex::ScopeLock lock(mAudioMutex);
	flushAudioPackets(mAudioInterface);
	lock.release();
}

float TheoraVideoClip_FFmpeg::decodeAudio()
{
	return -1;
}

void TheoraVideoClip_FFmpeg::doSeek()
{

}

void TheoraVideoClip_FFmpeg::_restart()
{

}

#endif
