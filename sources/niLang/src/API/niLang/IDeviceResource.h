#ifndef __IDEVICERESOURCE_8481598_H__
#define __IDEVICERESOURCE_8481598_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niLang
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
//! Device resource interface.
struct iDeviceResource : public iUnknown {
  niDeclareInterfaceUUID(iDeviceResource,0xbd49fea5,0xd9e4,0x4798,0xa3,0xfe,0x1e,0xf3,0x90,0x98,0x0c,0xe7);

  //! Get the resource's name.
  //! \return NULL if the resource is not named.
  //! {Property}
  virtual iHString* __stdcall GetDeviceResourceName() const = 0;
  //! Called when the resource is going to be used by the device.
  //! \param  apDevice is an implementation/device specific object passed to bind to support
  //!         things such as virtual device resources.
  //! \remark Should return a device-compatible resource to be used.
  //! \remark This allows resources to be virtual, proxy 'resources' can be created
  //!         to mimic the device resource and return a valid device-compatible resource
  //!         only when Bind is called.
  //! \remark Object that make use of this method documents it in the interface declaration with
  //!         the remark "Bindable"
  virtual iDeviceResource* __stdcall Bind(iUnknown* apDevice) = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//! Device resource manager interface.
//! \remark When the inteface is released it invalidates all the device resources contained.
struct iDeviceResourceManager : public iUnknown {
  niDeclareInterfaceUUID(iDeviceResourceManager,0x96aa6bb9,0x898d,0x4d68,0xad,0x3f,0x5a,0x39,0x0b,0xdc,0x62,0x05);

  //! Resource type.
  //! {Property}
  virtual iHString* __stdcall GetType() const = 0;
  //! Clear the resource manager and invalidate all resources.
  virtual void __stdcall Clear() = 0;
  //! Get the number of resources in the manager.
  //! {Property}
  virtual tU32 __stdcall GetSize() const = 0;
  //! Get a resource in the manager.
  //! {Property}
  virtual iDeviceResource* __stdcall GetFromName(iHString* ahspName) const = 0;
  //! Get the resource at the specified index.
  //! {Property}
  virtual iDeviceResource* __stdcall GetFromIndex(tU32 anIndex) const = 0;
  //! Register a resource in the manager.
  //! \return A unique index that will not change until the resource is
  //!         removed from the device resource manager. If the resource cant
  //!         be registered eInvalidHandle is returned instead.
  virtual tU32 __stdcall Register(iDeviceResource* apRes) = 0;
  //! Unregister a resource in the manager.
  virtual tBool __stdcall Unregister(iDeviceResource* apRes) = 0;
  //! Get the index of the first resource that has the specified name. eInvalidHandle if it cant be found.
  virtual tU32 __stdcall GetIndexFromName(iHString* ahspName) const = 0;
  //! Get the index of the specified resource. eInvalidHandle if it cant be found.
  virtual tU32 __stdcall GetIndexFromResource(
    iDeviceResource* apResource) const = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
} // namespace ni
#endif // __IDEVICERESOURCE_8481598_H__
