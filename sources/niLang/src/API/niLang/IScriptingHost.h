#pragma once
#ifndef __ISCRIPTINGHOST_H_845D00E1_56C6_4040_A3E3_48937BDE336F__
#define __ISCRIPTINGHOST_H_845D00E1_56C6_4040_A3E3_48937BDE336F__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niLang
 * @{
 */

//! Scripting host interface.
//! \remark The context is an indicator of a 'group/class' of object/implementation.
//! {DispatchWrapper}
struct iScriptingHost : public iUnknown
{
  niDeclareInterfaceUUID(iScriptingHost,0x6f7e1cfc,0x118b,0x452c,0x8c,0xb2,0x0a,0x04,0xde,0xd6,0x48,0xf2);

#if niMinFeatures(15)
  //! Evaluate the specified string.
  virtual tBool __stdcall EvalString(iHString* ahspContext, const ni::achar* aaszCode) = 0;
  //! Check whether the scripting host can evaluate the specified code resource.
  virtual tBool __stdcall CanEvalImpl(iHString* ahspContext, iHString* ahspCodeResource) = 0;
  //! Evaluate the specified implementation from the specified 'code resource'.
  //! \remark The code resource can be a file name, class name, or whathever is relevant as
  //!         an implementation identifier depending on the type of the scripting host.
  virtual iUnknown* __stdcall EvalImpl(iHString* ahspContext, iHString* ahspCodeResource, const tUUID& aIID) = 0;
  //! Service the scripting host.
  virtual void __stdcall Service(tBool abForceGC) = 0;
#endif
};


/**@}*/
}
#endif // __ISCRIPTINGHOST_H_845D00E1_56C6_4040_A3E3_48937BDE336F__
