#pragma once
#ifndef __IPROF_H_D1BFEF3E_5012_40AA_B9F3_E3470367995C__
#define __IPROF_H_D1BFEF3E_5012_40AA_B9F3_E3470367995C__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "Types.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */

typedef struct {
  const char * name;
  void * highlevel;
  char   initialized;
  char   visited;
  char   pad0,pad1;
} sProfilerZone;

typedef struct sProfilerZoneStack {
  tI64             t_self_start;
  tI64             total_self_ticks;
  tI64             total_hier_ticks;
  tU32             total_entry_count;
  struct sProfilerZoneStack* parent;
  sProfilerZone*   zone;
  tInt             recursion_depth;
  void*            highlevel;
} sProfilerZoneStack;

//! Profiler report modes
enum eProfilerReportMode
{
  eProfilerReportMode_SelfTime = 0,
  eProfilerReportMode_HierTime = 1,
  eProfilerReportMode_CallGraph = 2,
  eProfilerReportMode_Last = 3,
  //! \internal
  eProfilerReportMode_ForceDWORD = 0xFFFFFFFF
};

//! {DispatchWrapper}
struct iProfDraw : public iUnknown
{
  niDeclareInterfaceUUID(iProfDraw,0x831e4fec,0x92b9,0x434a,0x93,0x29,0x0f,0x5f,0x4a,0xfa,0x25,0x8f);

  virtual void __stdcall BeginDraw(tBool abTranslucent) = 0;
  virtual void __stdcall EndDraw() = 0;
  virtual void __stdcall DrawRect(tF32 x0, tF32 y0, tF32 x1, tF32 y1, tU32 anColor) = 0;
  virtual void __stdcall DrawLine(tF32 x0, tF32 y0, tF32 x1, tF32 y1, tU32 anColor) = 0;

  virtual tF32 __stdcall GetTextHeight() = 0;
  virtual tF32 __stdcall GetTextWidth(const achar* aText) = 0;

  virtual void __stdcall BeginText() = 0;
  virtual void __stdcall EndText() = 0;
  virtual void __stdcall Text(tF32 x, tF32 y, const achar* aText, tU32 anColor) = 0;
};

//! Profiler interface
struct iProf : public iUnknown
{
  niDeclareInterfaceUUID(iProf,0x1e2d88ee,0x7007,0x4eb3,0x87,0xd4,0x14,0x1e,0x69,0x82,0x88,0x6b);

  //! Set whether to record profiling infos or not during the next update.
  //! {Property}
  virtual void __stdcall SetRecord(tBool abRecord) = 0;
  //! Get whether to record profiling infos or not during the next update.
  //! {Property}
  virtual tBool __stdcall GetRecord() const = 0;

  //! Update the profiler, once per frame.
  virtual void __stdcall Update() = 0;

  //! {NoAutomation}
  virtual sProfilerZoneStack* __stdcall GetZoneStackDummy() const = 0;
  //! {NoAutomation}
  virtual sProfilerZoneStack** __stdcall GetZoneStack() const = 0;
  //! {NoAutomation}
  virtual void __stdcall SetZoneStack(sProfilerZoneStack* apZoneStack) = 0;
  //! {NoAutomation}
  virtual sProfilerZoneStack* __stdcall StackAppend(sProfilerZone* apZoneStack) = 0;
  //! {NoAutomation}
  virtual void __stdcall GetTimeStamp(tI64* apTimeStamp) const = 0;

  virtual void __stdcall SetReportMode(eProfilerReportMode aMode) = 0;
  virtual eProfilerReportMode __stdcall GetReportMode() = 0;
  virtual void __stdcall InputMoveCursor(tI32 anDelta) = 0;
  virtual void __stdcall InputSelect() = 0;
  virtual void __stdcall InputSelectParent() = 0;
  virtual void __stdcall InputMoveFrame(tI32 anDelta) = 0;
  virtual void __stdcall InputSetFrame(tI32 anFrame) = 0;
  virtual void __stdcall InputSetCursor(tI32 anLine) = 0;

  virtual cString __stdcall TextReport(tU32 cols, tU32 rows) = 0;

  virtual tBool __stdcall DrawTable(
      ni::iProfDraw* drawer,
      tF32 sx, tF32 sy,
      tF32 full_width, tF32 height,
      tI32 precision) = 0;

  virtual tBool __stdcall DrawGraph(
      ni::iProfDraw* drawer, tF32 sx, tF32 sy,
      tF32 x_spacing, tF32 y_spacing) = 0;
};

niExportFunc(ni::iProf*) GetProf();

/**@}*/
}
#endif // __IPROF_H_D1BFEF3E_5012_40AA_B9F3_E3470367995C__
