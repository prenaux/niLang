#pragma once
#ifndef __OSXMETAL_H_5DE6C4B9_12AD_4544_8F88_59BA1F7CCFCE__
#define __OSXMETAL_H_5DE6C4B9_12AD_4544_8F88_59BA1F7CCFCE__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../../IOSWindow.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_OSX
 * @{
 */

struct iOSXMetalAPI : public iOSGraphicsAPI {
  niDeclareInterfaceUUID(iOSXMetalAPI,0x55e47e7e,0x5979,0x0545,0x98,0x0c,0x65,0xee,0xac,0xb7,0x22,0x5c);
  virtual void* __stdcall GetDevice() const = 0;
  virtual void* __stdcall GetCommandQueue() const = 0;
  virtual void* __stdcall GetMTKView() const = 0;
  virtual void* __stdcall NewRenderCommandEncoder(const double4& aClearColor, tF32 aClearDepth, tU32 aClearStencil) = 0;
  virtual tBool __stdcall PresentAndCommit(iRunnable* apOnCompleted) = 0;
};

niExportFunc(tBool) osxMetalSetDefaultDevice();
niExportFunc(tBool) osxMetalSetDevice(void* apDevice);
niExportFunc(void*) osxMetalGetDevice();

niExportFunc(iOSXMetalAPI*) osxMetalCreateAPIForMTKView(void* apDevice, void* apMTKView);

#ifdef niOSX
niExportFunc(iOSXMetalAPI*) osxMetalCreateAPIForWindow(void* apDevice, iOSWindow* apWindow);
#endif

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
} // namespace ni {
#endif // __OSXMETAL_H_5DE6C4B9_12AD_4544_8F88_59BA1F7CCFCE__
