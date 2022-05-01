// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"
#include "Graphics.h"
#include "VideoUtils.h"

///////////////////////////////////////////////
tBool Video_FileNameIsVideo(const char* aaszFileName) {
  return
      StrEndsWith(aaszFileName, ".avi") ||
      StrEndsWith(aaszFileName, ".mp4") ||
      StrEndsWith(aaszFileName, ".mkv") ||
      StrEndsWith(aaszFileName, ".ogv") ||
      StrEndsWith(aaszFileName, ".ogw") ||
      StrEndsWith(aaszFileName, ".mov") ||
      StrEndsWith(aaszFileName, ".wmv") ||
      StrEndsWith(aaszFileName, ".webcam") ||
      StrEndsWith(aaszFileName, ".ipcam");
}

#ifdef NI_VIDEO

#if defined NI_VIDEO_USE_THEORAPLAYER
#pragma message("niVideo: Using THEORAPLAYER")
iVideoDecoder* CreateVideoDecoder_TheoraPlayer(iGraphics* apGraphics, iHString* ahspName, iFile* apFile, tVideoDecoderFlags aFlags);
tBool Video_FileNameIsVideoTheoraPlayer(const char* aaszFileName) {
  return StrEndsWith(aaszFileName, ".ogv") ||
      StrEndsWith(aaszFileName, ".ogw") ||
      StrEndsWith(aaszFileName, ".mp4") ||
      StrEndsWith(aaszFileName, ".mkv") ||
      StrEndsWith(aaszFileName, ".wmv");
}
#endif

#if defined NI_VIDEO_USE_WEBCAM
#pragma message("niVideo: Using WEBCAM")
iVideoDecoder* CreateVideoDecoder_Webcam(iGraphics* apGraphics, iHString* ahspName, iFile* apFile, tVideoDecoderFlags aFlags);
tBool Video_FileNameIsVideoWebcam(const char* aaszFileName) {
  return StrEndsWith(aaszFileName, ".webcam");
}
#endif

#if defined NI_VIDEO_USE_IPCAM
#pragma message("niVideo: Using IPCAM")
iVideoDecoder* CreateVideoDecoder_Ipcam(iGraphics* apGraphics, iHString* ahspName, iFile* apFile, tVideoDecoderFlags aFlags);
tBool Video_FileNameIsVideoIpcam(const char* aaszFileName) {
  return StrEndsWith(aaszFileName, ".ipcam");
}
#endif

#if defined NI_VIDEO_USE_FFMPEG
#pragma message("niVideo: Using FFMPEG")
iVideoDecoder* CreateVideoDecoder_FFmpeg(iGraphics* apGraphics, iHString* ahspName, iFile* apFile, tVideoDecoderFlags aFlags);
#endif

///////////////////////////////////////////////
tBool __stdcall cGraphics::IsVideoFile(iFile* apFile) const {
  if (!niIsOK(apFile)) {
    return eFalse;
  }
  return Video_FileNameIsVideo(apFile->GetSourcePath());
}

///////////////////////////////////////////////
iVideoDecoder* __stdcall cGraphics::CreateVideoDecoder(iHString* ahspName, iFile* apFile, tVideoDecoderFlags aFlags) {
  niCheckIsOK(apFile,NULL);

#if defined NI_VIDEO_USE_WEBCAM
  if (Video_FileNameIsVideoWebcam(apFile->GetSourcePath())) {
    return CreateVideoDecoder_Webcam(this, ahspName, apFile, aFlags);
  }
#endif

#if defined NI_VIDEO_USE_IPCAM
  if (Video_FileNameIsVideoIpcam(apFile->GetSourcePath())) {
#ifndef NI_VIDEO_USE_FFMPEG
    return CreateVideoDecoder_Ipcam(this, ahspName, apFile, aFlags);
#else
    return CreateVideoDecoder_FFmpeg(this, ahspName, apFile, aFlags);
#endif
  }
#endif

#if defined NI_VIDEO_USE_THEORAPLAYER
  if (Video_FileNameIsVideoTheoraPlayer(apFile->GetSourcePath())) {
    return CreateVideoDecoder_TheoraPlayer(this, ahspName, apFile, aFlags);
  }
#endif

  return NULL;
}

#else // #ifdef NI_VIDEO
#pragma niNote("niVideo: No video decoder")

///////////////////////////////////////////////
tBool __stdcall cGraphics::IsVideoFile(iFile* apFile) const {
  return eFalse;
}

///////////////////////////////////////////////
iVideoDecoder* __stdcall cGraphics::CreateVideoDecoder(iHString* ahspName, iFile* apFile, tVideoDecoderFlags aFlags) {
  niError(niFmt("Can't open video '%s', no video decoder.", ahspName));
  return NULL;
}

#endif
