#pragma once
#ifndef __ICURL_H_585EE2CB_EA5B_4D4E_9943_891B1A03A152__
#define __ICURL_H_585EE2CB_EA5B_4D4E_9943_891B1A03A152__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Utils/MessageID.h>

namespace ni {

struct iCURL;

/** \addtogroup niCURL
 * @{
 */

//! CURL message.
enum eCURLMessage
{
  //! The request started.
  //! \param A: unused
  //! \param B: iFuture, which can be used to cancel the request
  eCURLMessage_Started = niMessageID('C','U','R','L','t'),
  //! Received a response code from the server.
  //! \param A: the last received HTTP or FTP code.
  //! \param B: iFuture, which can be used to cancel the request
  eCURLMessage_ResponseCode = niMessageID('C','U','R','L','o'),
  //! Started receiving the header.
  //! \param A: unused
  //! \param B: iFuture, which can be used to cancel the request
  eCURLMessage_ReceivingHeader = niMessageID('C','U','R','L','h'),
  //! Started receiving the data.
  //! \param A: unused
  //! \param B: iFuture, which can be used to cancel the request
  eCURLMessage_ReceivingData = niMessageID('C','U','R','L','r'),
  //! Received a part.
  //! \param A: iFile, buffer file which contains the data of the part.
  //! \param B: iFuture, which can be used to cancel the request
  //! \remark Sent by URLGetMultiPart, it includes the part's header which has a content type which looks like 'Content-Type: multipart/x-mixed-replace;boundary=ipcamera'.
  eCURLMessage_ReceivedPart = niMessageID('C','U','R','L','m'),
  //! Started sending data.
  //! \param A: unused
  //! \param B: iFuture, which can be used to cancel the request
  eCURLMessage_Sending = niMessageID('C','U','R','L','s'),
  //! The request has successfully completed.
  //! \param A: Response code (200, 404, ...)
  //! \param B: iFuture, which can be used to cancel the request
  eCURLMessage_Completed = niMessageID('C','U','R','L','C'),
  //! The request failed.
  //! \param A: error message string
  //! \param B: iFuture, which can be used to cancel the request
  eCURLMessage_Failed = niMessageID('C','U','R','L','F'),
  //! Progress message
  //! \param A: Vec4i(dlNow,dlTotal,upNow,upTotal)
  //! \param B: iFuture, which can be used to cancel the request
  eCURLMessage_Progress = niMessageID('C','U','R','L','p'),
  //! \internal
  eCURLMessage_ForceDWORD = 0xFFFFFFFF
};

//! HTTP authentication modes
//! \remark This uses the same value as CURLOPT_HTTPAUTH options in libcurl for convenience.
enum eCURLHttpAuth
{
  eCURLHttpAuth_None         = 0,
  eCURLHttpAuth_Basic        = niBit(0),
  eCURLHttpAuth_Digest       = niBit(1),
  eCURLHttpAuth_Gssnegotiate = niBit(2),
  eCURLHttpAuth_Ntlm         = niBit(3),
  eCURLHttpAuth_DigestIE     = niBit(4),
  eCURLHttpAuth_NtlmWB       = niBit(5),
  eCURLHttpAuth_Only         = niBit(31),
  eCURLHttpAuth_Any          = ~eCURLHttpAuth_DigestIE,
  eCURLHttpAuth_AnySafe      = ~(eCURLHttpAuth_Basic|eCURLHttpAuth_DigestIE),
  //! \internal
  eCURLHttpAuth_ForceDWORD = 0xFFFFFFFF
};

//! CURL interface.
struct iCURL : public iUnknown
{
  niDeclareInterfaceUUID(iCURL,0x5266eb8e,0xa915,0x42f8,0x99,0x38,0xc8,0xa1,0x87,0x0f,0x4e,0x02);

  //! {Property}
  virtual cString __stdcall GetVersion() const = 0;
  //! {Property}
  virtual cString __stdcall GetProtocols() const = 0;

  //! \remark Default is 10 secs.
  //! {Property}
  virtual void __stdcall SetConnectionTimeoutInSecs(tU32 anInSecs) = 0;
  //! {Property}
  virtual tU32 __stdcall GetConnectionTimeoutInSecs() const = 0;

  //! \remark Default is 60 secs.
  //! {Property}
  virtual void __stdcall SetRequestTimeoutInSecs(tU32 anInSecs) = 0;
  //! {Property}
  virtual tU32 __stdcall GetRequestTimeoutInSecs() const = 0;

  //! {Property}
  virtual void __stdcall SetUserAgent(const achar* aaszUserAgent) = 0;
  //! {Property}
  virtual const achar* __stdcall GetUserAgent() const = 0;

  //! {Property}
  virtual void __stdcall SetUserName(const achar* aaszUserName) = 0;
  //! {Property}
  virtual const achar* __stdcall GetUserName() const = 0;

  //! {Property}
  virtual void __stdcall SetUserPass(const achar* aaszUserPass) = 0;
  //! {Property}
  virtual const achar* __stdcall GetUserPass() const = 0;

  //! {Property}
  virtual void __stdcall SetHttpAuth(eCURLHttpAuth aHttpAuth) = 0;
  //! {Property}
  virtual const eCURLHttpAuth __stdcall GetHttpAuth() const = 0;

  virtual Ptr<iRunnable> __stdcall URLGet(
      iMessageHandler* apMessageHandler,
      const achar* aURL, iFile* apRecvData, iFile* apRecvHeader, const tStringCVec* apHeaders = NULL) = 0;

  virtual Ptr<iRunnable> __stdcall URLPostFile(
      iMessageHandler* apMessageHandler,
      const achar* aURL, iFile* apRecvData, iFile* apRecvHeader,
      iFile* apPostData,
      const tStringCVec* apHeaders = NULL, const achar* aContentType = NULL) = 0;

  virtual Ptr<iRunnable> __stdcall URLPostFields(
      iMessageHandler* apMessageHandler,
      const achar* aURL, iFile* apRecvData, iFile* apRecvHeader,
      const achar* aPostFields,
      const tStringCVec* apHeaders = NULL, const achar* aContentType = NULL) = 0;

  //! {NoAutomation}
  virtual Ptr<iRunnable> __stdcall URLPostRaw(
      iMessageHandler* apMessageHandler,
      const achar* aURL, iFile* apRecvData, iFile* apRecvHeader,
      tPtr aPostData, tSize anPostDataSize,
      const tStringCVec* apHeaders = NULL, const achar* aContentType = NULL) = 0;

  //! Post with Content-Type: multipart/form-data. Fields are similar to the
  //! command line version of CURL, @path will upload the file located at the
  //! specified location.
  virtual Ptr<iRunnable> __stdcall URLPostMultiPart(
      iMessageHandler* apMessageHandler,
      const achar* aURL, iFile* apRecvData, iFile* apRecvHeader,
      tStringCMap* apPostFields) = 0;

  virtual Ptr<iRunnable> __stdcall URLGetMultiPart(
      iMessageHandler* apMessageHandler, const achar* aURL, const achar* aPartExt) = 0;

  //! Does a simple get on a URL and return the result as a string.
  virtual cString __stdcall URLGetString(const achar* aURL) = 0;
  //! Does a simple get on a URL and return the result as a datatable.
  //! \remark For this to work the endpoint is assumed to return a json response.
  virtual tI32 __stdcall URLGetDataTable(const achar* aURL, iDataTable* apResult) = 0;
};

niExportFunc(ni::iUnknown*) New_niCURL_CURL(const ni::Var&,const ni::Var&);

/**@}*/
}
#endif // __ICURL_H_585EE2CB_EA5B_4D4E_9943_891B1A03A152__
