#pragma once
#ifndef __ISINKLIST_H_00B467F2_68C8_40DC_B374_A2D9E4A9AD3E__
#define __ISINKLIST_H_00B467F2_68C8_40DC_B374_A2D9E4A9AD3E__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "Types.h"
#include "ICollection.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */

//! Sink list interface.
struct iSinkList : public iCollection
{
  niDeclareInterfaceUUID(iSinkList,0xeaace572,0x345e,0x44f6,0xb0,0xeb,0xd9,0xb7,0x99,0x52,0x37,0xc4);
  //! Get the UUID of the sink.
  //! {Property}
  virtual const tUUID& __stdcall GetSinkUUID() const = 0;
  //! Remove all the sinks.
  virtual void __stdcall Clear() = 0;
  //! Check whether the specified sink has already been added.
  virtual tBool __stdcall HasSink(iUnknown* apSink) = 0;
  //! Add a sink to the sink list.
  virtual tBool __stdcall AddSink(iUnknown* apSink) = 0;
  //! Remove a sink from the sink list.
  virtual tBool __stdcall RemoveSink(iUnknown* apSink) = 0;
  //! Add a sink to the front of the sink list.
  virtual tBool __stdcall AddFrontSink(iUnknown* apSink) = 0;
  //! Mute the sink.
  //! {Property}
  virtual void __stdcall SetMute(tBool abMute) = 0;
  //! Get the muted status of the sink.
  //! {Property}
  virtual tBool __stdcall GetMute() const = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __ISINKLIST_H_00B467F2_68C8_40DC_B374_A2D9E4A9AD3E__
