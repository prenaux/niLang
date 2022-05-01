#pragma once
#ifndef __VIDEOUTILS_H_EB54059C_0741_4459_A8A4_43F42399B995__
#define __VIDEOUTILS_H_EB54059C_0741_4459_A8A4_43F42399B995__

// Set in the build script
// #define NI_VIDEO_USE_DSHOW
// #define NI_VIDEO_USE_FFMPEG
// #define NI_VIDEO_USE_OGG
// #define NI_VIDEO_USE_THEORAPLAYER
// #define NI_VIDEO_USE_WEBCAM
// #define NI_VIDEO_USE_IPCAM

#if defined NI_VIDEO_USE_THEORAPLAYER || defined NI_VIDEO_USE_WEBCAM || defined NI_VIDEO_USE_IPCAM
#define NI_VIDEO
#endif

tBool Video_FileNameIsVideo(const char* aaszFileName);

#endif // __VIDEOUTILS_H_EB54059C_0741_4459_A8A4_43F42399B995__
