#pragma once
#ifndef __IFETCH_H_6CDBF307_BCF5_8944_9DFB_F5A3FF7BCC11__
#define __IFETCH_H_6CDBF307_BCF5_8944_9DFB_F5A3FF7BCC11__

namespace ni {

struct iFetchRequest;

//! HTTP method used to execute the fetch request.
enum eFetchMethod {
  eFetchMethod_Get = 0,
  eFetchMethod_Post = 1,
  //! \internal
  eFetchMethod_ForceDWORD = 0xFFFFFFFF
};

//! State of the fetch request.
//! \remark See https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/readyState
enum eFetchReadyState {
  //! Request not sent yet
  eFetchReadyState_Unsent = 0,
  //! Fetch has been sent.
  eFetchReadyState_Opened = 1,
  //! Headers and status are available.
  eFetchReadyState_HeadersReceived = 2,
  //! Download in progress.
  eFetchReadyState_Loading = 3,
  //! Download finished.
  eFetchReadyState_Done = 4,
  //! \internal
  eFetchReadyState_ForceDWORD = 0xFFFFFFFF
};

struct iFetchSink : public iUnknown {
  niDeclareInterfaceUUID(iFetchSink,0x10833ab2,0xd5e7,0x3d49,0x86,0xbb,0x57,0x6a,0x9d,0x6a,0x7e,0xde);

  virtual void __stdcall OnFetchSink_Success(iFetchRequest* apFetch) = 0;
  virtual void __stdcall OnFetchSink_Error(iFetchRequest* apFetch) = 0;
  virtual void __stdcall OnFetchSink_Progress(iFetchRequest* apFetch) = 0;
  virtual void __stdcall OnFetchSink_ReadyStateChange(iFetchRequest* apFetch) = 0;
};

struct iFetchRequest : public iUnknown {
  niDeclareInterfaceUUID(iFetchRequest,0xeb50cb02,0x4181,0x2a46,0xa8,0x39,0x6c,0x9b,0x10,0xcf,0xc2,0xd9);

  //! Get the fetch method.
  //! {Property}
  virtual eFetchMethod __stdcall GetMethod() const = 0;

  //! Get the ready state of the request.
  //! {Property}
  virtual eFetchReadyState __stdcall GetReadyState() const = 0;

  //! Get the status code of the request.
  //! {Property}
  virtual tU32 __stdcall GetStatus() const = 0;

  //! The headers received.
  //! {Property}
  virtual iFile* __stdcall GetReceivedHeaders() const = 0;

  //! The data received.
  //! {Property}
  virtual iFile* __stdcall GetReceivedData() const = 0;

  //! Whether the request had failed.
  //! {Property}
  virtual tBool __stdcall GetHasFailed() const = 0;
};

}
#endif // __IFETCH_H_6CDBF307_BCF5_8944_9DFB_F5A3FF7BCC11__
