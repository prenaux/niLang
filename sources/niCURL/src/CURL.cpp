// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"

#include "curl/curl.h"
#include <niLang/Utils/ConcurrentImpl.h>
#include <niLang/Utils/Trace.h>

#ifdef niWindows
#include <niLang/Platforms/Win32/Win32_Redef.h>
#endif
#ifdef niMSVC
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"wldap32.lib")
#pragma comment(lib,"Advapi32.lib")
#endif

niDeclareModuleTrace_(niCURL,Verbose);
#define SHOULD_TRACE_CURL() niModuleShouldTrace_(niCURL,Verbose)

niDeclareModuleTrace_(niCURL,TraceGetMultiPart);
#define TRACE_GET_MULTIPART(FMT) niModuleTrace_(niCURL,TraceGetMultiPart,FMT)

niDeclareModuleTrace_(niCURL,TraceGetString);
#define TRACE_GET_STRING(FMT) niModuleTrace_(niCURL,TraceGetString,FMT)

// This flattens all properties, it doesnt handle objects or arrays...
struct _JsonParseToDataTableSink : cIUnknownImpl<iJsonParserSink,eIUnknownImplFlags_Local> {
  cString _keyName;
  iDataTable* _dt;
  _JsonParseToDataTableSink(iDataTable* apDT) : _dt(apDT) {}

  //! Called when a parsing error occured.
  virtual void __stdcall OnJsonParserSink_Error(const achar* aaszReason, tU32 anLine, tU32 anCol)
  {
    niWarning(niFmt("JsonParseToDataTableSink error:%d:%d: %s",
                    anLine, anCol, aaszReason));
  }

  //! Called when a value is parsed
  virtual void __stdcall OnJsonParserSink_Value(eJsonType aType, const achar* aValue) {
    switch (aType) {
      case eJsonType_Name: {
        _keyName = aValue;
        break;
      }
      case eJsonType_String: {
        _dt->SetString(_keyName.Chars(), aValue);
        break;
      }
      case eJsonType_Number: {
        if (StrChr(aValue,'.') || StrChr(aValue,'e')) {
          _dt->SetFloat(_keyName.Chars(), StrAToF(aValue));
        }
        else {
          _dt->SetInt(_keyName.Chars(), StrAToL(aValue));
        }
        break;
      }
      case eJsonType_True: {
        _dt->SetBool(_keyName.Chars(), eTrue);
        break;
      }
      case eJsonType_False: {
        _dt->SetBool(_keyName.Chars(), eFalse);
        break;
      }
      case eJsonType_Null: {
        _dt->SetVar(_keyName.Chars(), niVarNull);
        break;
      }
    }
  }
};

static ni::tBool __stdcall _JsonParseFileToDataTable(ni::iFile* apFile, ni::iDataTable* apDT) {
  niCheckIsOK(apFile, ni::eFalse);
  niCheckIsOK(apDT, ni::eFalse);
  _JsonParseToDataTableSink sinkParser(apDT);
  return JsonParseFile(apFile,&sinkParser);
}

static ni::tBool __stdcall _JsonParseStringToDataTable(const cString& aString, ni::iDataTable* apDT) {
  niCheckIsOK(apDT, ni::eFalse);
  _JsonParseToDataTableSink sinkParser(apDT);
  return JsonParseString(aString,&sinkParser);
}

static int _curlTrace(CURL *handle, curl_infotype type,
                      char *data, size_t size,
                      void *userp)
{
  niUnused(handle);
  switch(type) {
    case CURLINFO_TEXT:
      niDebugFmt(("CURL: (%db) %s",size,cString(data,data+size)));
    default: /* in case a new one is introduced to shock us */
      return 0;
    case CURLINFO_HEADER_OUT:
      niDebugFmt(("CURL: => Send header: (%db) %s",size,cString(data,data+size)));
      break;
    case CURLINFO_DATA_OUT:
      niDebugFmt(("CURL: => Send data: %db",size));
      break;
    case CURLINFO_SSL_DATA_OUT:
      niDebugFmt(("CURL: => Send SSL data: %db",size));
      break;
    case CURLINFO_HEADER_IN:
      niDebugFmt(("CURL: <= Recv header: (%db) %s",size,cString(data,data+size)));
      break;
    case CURLINFO_DATA_IN:
      niDebugFmt(("CURL: <= Recv data: %db",size));
      break;
    case CURLINFO_SSL_DATA_IN:
      niDebugFmt(("CURL: <= Recv SSL data: %db",size));
      break;
  }
  return 0;
}

struct CURLRunnable : public cIUnknownImpl<iRunnable>
{
  CURL* _curl;
  CURL* _multiHandle;
  curl_httppost* _formPost;
  curl_slist* _headerList;
  Ptr<iFutureValue> _future;
  Ptr<iFile> _recvHeader;
  Ptr<iFile> _recvData;
  Ptr<iFile> _postData;
  Ptr<iMessageHandler> _msgHandler;
  tU32    _state;
  long    _responseCode;
  sVec4i  _lastProgress;
  tSize   _recvDataSize, _recvHeaderSize;

  CURLRunnable(CURL* aCURL, CURLM* aMultiHandle) : _curl(aCURL), _multiHandle(aMultiHandle) {
    _formPost = NULL;
    _headerList = NULL;
    _lastProgress = sVec4i::Zero();
    _state = 0;
    _responseCode = 0;
    _future = ni::GetConcurrent()->CreateFutureValue();
  }

  ~CURLRunnable() {
    Invalidate();
  }

  void __stdcall Invalidate() {
    if (_multiHandle) {
      curl_multi_cleanup(_multiHandle);
      _multiHandle = NULL;
    }
    if (_curl) {
      curl_easy_cleanup(_curl);
      _lastProgress = sVec4i::Zero();
      _future = NULL;
      _recvHeader = NULL;
      _recvData = NULL;
      _postData = NULL;
      _msgHandler = NULL;
      _curl = NULL;
    }
    if (_formPost) {
      curl_formfree(_formPost);
      _formPost = NULL;
    }
    if (_headerList) {
      curl_slist_free_all(_headerList);
      _headerList = NULL;
    }
  }

  Var __stdcall _RunSingle() {
    niCheck(_curl, eFalse);

    tBool bRet = eTrue;
    CURLcode res = curl_easy_perform(_curl);

    if (bRet && _recvDataSize <= 0 && _recvHeaderSize <= 0) {
      _UpdateState(eCURLMessage_Failed, "CURL single perform error, no header received.");
      bRet = eFalse;
    }
    if (bRet && _recvDataSize <= 0) {
      _UpdateState(eCURLMessage_Failed, "CURL single perform error, no data received.");
      bRet = eFalse;
    }

    if (bRet && (res != CURLE_OK && res != CURLE_ABORTED_BY_CALLBACK)) {
      cString errMsg = "CURL perform error: ";
      if (res <= CURLE_SSL_ISSUER_ERROR) {
        errMsg += curl_easy_strerror(res);
      }
      else {
        errMsg += "Unknown";
      }
      _UpdateState(eCURLMessage_Failed, errMsg);
      bRet = eFalse;
    }

    if (bRet) {
      _UpdateState(eCURLMessage_Completed, _responseCode);
    }

    _future->SetValue(bRet);
    Invalidate();
    return bRet;
  }

  Var __stdcall _RunMulti() {
    niCheck(_curl, eFalse);

    tBool bRet = eTrue;
    int still_running;
    curl_multi_perform(_multiHandle, &still_running);

    do {
      struct timeval timeout;
      int rc; /* select() return code */
      CURLMcode mc; /* curl_multi_fdset() return code */

      fd_set fdread;
      fd_set fdwrite;
      fd_set fdexcep;
      int maxfd = -1;

      long curl_timeo = -1;

      FD_ZERO(&fdread);
      FD_ZERO(&fdwrite);
      FD_ZERO(&fdexcep);

      /* set a suitable timeout to play around with */
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;

      curl_multi_timeout(_multiHandle, &curl_timeo);
      if(curl_timeo >= 0) {
        timeout.tv_sec = curl_timeo / 1000;
        if(timeout.tv_sec > 1)
          timeout.tv_sec = 1;
        else
          timeout.tv_usec = (curl_timeo % 1000) * 1000;
      }

      /* get file descriptors from the transfers */
      mc = curl_multi_fdset(_multiHandle, &fdread, &fdwrite, &fdexcep, &maxfd);
      if(mc != CURLM_OK) {
        _UpdateState(eCURLMessage_Failed, niFmt("CURL multi perform error: %d.", (tI32)mc));
        bRet = eFalse;
        break;
      }

      /* On success the value of maxfd is guaranteed to be >= -1. We call
         select(maxfd + 1, ...); specially in case of (maxfd == -1) there are
         no fds ready yet so we call select(0, ...) --or Sleep() on Windows--
         to sleep 100ms, which is the minimum suggested value in the
         curl_multi_fdset() doc. */
      if(maxfd == -1) {
#ifdef _WIN32
        Sleep(100);
        rc = 0;
#else
        /* Portable sleep for platforms other than Windows. */
        struct timeval wait = { 0, 100 * 1000 }; /* 100ms */
        rc = select(0, NULL, NULL, NULL, &wait);
#endif
      }
      else {
        /* Note that on some platforms 'timeout' may be modified by select().
           If you need access to the original value save a copy beforehand. */
        rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);
      }

      switch(rc) {
        case -1:
          /* select error */
          break;
        case 0:
        default:
          /* timeout or readable/writable sockets */
          // niDebugFmt(("... CURL multi perform."));
          curl_multi_perform(_multiHandle, &still_running);
          // niDebugFmt(("... CURL multi perform, running %d.", still_running));
          break;
      }
    } while(still_running);

    if (bRet && _recvHeaderSize <= 0) {
      _UpdateState(eCURLMessage_Failed, "CURL multi perform error, no header received.");
      bRet = eFalse;
    }

    if (bRet && _recvDataSize <= 0) {
      _UpdateState(eCURLMessage_Failed, "CURL multi perform error, no data received.");
      bRet = eFalse;
    }

    // bRet is set to false if there was an error
    if (bRet) {
      _UpdateState(eCURLMessage_Completed, _responseCode);
    }

    _future->SetValue(bRet);
    Invalidate();
    return bRet;
  }

  virtual Var __stdcall Run() {
    _recvDataSize = _recvHeaderSize = 0;

    if (_multiHandle) {
      return _RunMulti();
    }
    else {
      return _RunSingle();
    }
  }

  void _UpdateState(tU32 aNewState, const Var& avarA = niVarNull) {
    _UpdateResponseCode();
    if (_state == 0) {
      _state = eCURLMessage_Started;
      // niDebugFmt(("... _UpdateState Started: %d, %s", aNewState, avarA));
      if (_msgHandler.IsOK()) {
        ni::GetConcurrent()->SendMessage(_msgHandler, _state, niVarNull, _future.ptr());
      }
    }
    else if (_state != aNewState) {
      _state = aNewState;
      // niDebugFmt(("... _UpdateState: %d, %p, %s", aNewState, (tIntPtr)_msgHandler.ptr(), avarA));
      if (_msgHandler.IsOK()) {
        ni::GetConcurrent()->SendMessage(_msgHandler, _state, avarA, _future.ptr());
      }
    }
  }

  void _UpdateResponseCode() {
    long newResponseCode = 0;
    curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &newResponseCode);
    if (_responseCode != newResponseCode) {
      _responseCode = newResponseCode;
      if (_msgHandler.IsOK()) {
        ni::GetConcurrent()->SendMessage(_msgHandler, eCURLMessage_ResponseCode, _responseCode, _future.ptr());
      }
    }
  }

  size_t __cdecl _ReadPostData(void* buffer, size_t size, size_t nmemb) {
    _UpdateResponseCode();
    _UpdateState(eCURLMessage_Sending);
    if (_postData.IsOK()) {
      return _postData->ReadRaw(buffer,size*nmemb);
    }
    else {
      return 0;
    }
  }
  static size_t __cdecl _ReadPostDataCallback(void* buffer, size_t size, size_t nmemb, void* userp) {
    CURLRunnable* _this = (CURLRunnable*)userp;
    return _this->_ReadPostData(buffer,size,nmemb);
  }

  size_t __cdecl _WriteRecvData(void* buffer, size_t size, size_t nmemb) {
    _UpdateResponseCode();
    _UpdateState(eCURLMessage_ReceivingData,_recvData.ptr());
    _recvDataSize += size*nmemb;
    if (_recvData.IsOK()) {
      return _recvData->WriteRaw(buffer,size*nmemb);
    }
    else {
      return 0;
    }
  }
  static size_t __cdecl _WriteRecvDataCallback(void* buffer, size_t size, size_t nmemb, void* userp) {
    CURLRunnable* _this = (CURLRunnable*)userp;
    return _this->_WriteRecvData(buffer,size,nmemb);
  }

  size_t __cdecl _WriteRecvHeader(void* buffer, size_t size, size_t nmemb) {
    _UpdateResponseCode();
    _UpdateState(eCURLMessage_ReceivingHeader,_recvHeader);
    _recvHeaderSize += size*nmemb;
    if (_recvHeader.IsOK()) {
      return _recvHeader->WriteRaw(buffer,size*nmemb);
    }
    else {
      return 0;
    }
  }
  static size_t __cdecl _WriteRecvHeaderCallback(void* buffer, size_t size, size_t nmemb, void* userp) {
    CURLRunnable* _this = (CURLRunnable*)userp;
    return _this->_WriteRecvHeader(buffer,size,nmemb);
  }

  int _Progress(double dltotal, double dlnow, double ultotal, double ulnow)
  {
    _UpdateResponseCode();
    if (_state == 0) {
      _UpdateState(eCURLMessage_Started);
    }

    const sVec4i progress = Vec4<tI32>((tI32)dlnow,(tI32)dltotal,
                                       (tI32)ulnow,(tI32)ultotal);
    // CURL triggers the progress callback before any data is retrieved, with
    // zero as value for all parameters,
    if (progress != _lastProgress) {
      if (_msgHandler.IsOK()) {
        ni::GetConcurrent()->SendMessage(
            _msgHandler,eCURLMessage_Progress,
            progress,_future.ptr());
      }
      _lastProgress = progress;
    }

    if (_future->GetIsCancelled()) {
      return -1;
    }
    else {
      return 0;
    }
  };
  static size_t __cdecl _ProgressCallback(void* clientp,
                                          double dltotal, double dlnow,
                                          double ultotal, double ulnow)
  {
    CURLRunnable* _this = (CURLRunnable*)clientp;
    return _this->_Progress(dltotal,dlnow,ultotal,ulnow);
  }
};


struct FileWritePart : public cIUnknownImpl<iFileBase,eIUnknownImplFlags_Default>
{
  FileWritePart(const char* aURL, const char* aExt, iMessageHandler* apHandler) {
    _numParts = 0;
    _msgHandler = apHandler;
    ResetPart(niFmt("$CURL-GET-PART$%s$%s",aURL,aExt));
  }

  void ResetPart(const char* aFileName) {
    _fpPart = ni::CreateFileDynamicMemory(0,aFileName);
  }

  tFileFlags __stdcall GetFileFlags() const { return eFileFlags_Write; }
  tBool  __stdcall Seek(tI64 offset) { return eTrue; }
  tBool  __stdcall SeekSet(tI64 offset) { return eTrue; }
  tBool __stdcall SeekEnd(tI64 offset) { return eTrue; }
  tSize  __stdcall ReadRaw(void* pOut, tSize nSize) { return 0; }
  const achar* __stdcall GetSourcePath() const { return NULL; }
  tBool __stdcall Flush()  { return eTrue; }
  tBool __stdcall GetTime(eFileTime aFileTime, iTime* apTime) const { niUnused(aFileTime);niUnused(apTime); return eFalse; }
  tBool __stdcall SetTime(eFileTime aFileTime, const iTime* apTime) { niUnused(aFileTime);niUnused(apTime); return eFalse; }
  tBool __stdcall Resize(tI64 newSize) { return eFalse; }

  tSize  __stdcall WriteRaw(const void* apIn, tSize anSize) {
    const char* p = (const char*)apIn;
    TRACE_GET_MULTIPART(("... URLGetMultiPart: RECEIVED DATA: %db", anSize, p));

    tU32 delimiterSize = 0;
    tI32 foundDelimiter = -1;

    // look for the delimiter
    if (!_partDelimiter.empty() && anSize > _partDelimiter.size()) {
      niLoop(i,anSize-_partDelimiter.size()) {
        if (*p == '-' && *(p+1) == '-') {
          if (ni::MemCmp((tPtr)p,(tPtr)_partDelimiter.Chars(),_partDelimiter.size()) == 0) {
            foundDelimiter = i;
            delimiterSize = _partDelimiter.size();
            break;
          }
          // The standard case is `boundary=abc` and `delimiter=--abc`, the
          // delimiter starts with `--`. But when the boundary starts with
          // `--` some encoder do not output `----` but only two `-` thus we
          // need to handle that case.
          if (ni::MemCmp((tPtr)p,(tPtr)_partDelimiter.Chars()+2,_partDelimiter.size()-2) == 0) {
            foundDelimiter = i;
            delimiterSize = _partDelimiter.size()-2;
            break;
          }
        }
        ++p;
      }
    }

    if (foundDelimiter >= 0) {
      TRACE_GET_MULTIPART(("... URLGetMultiPart: FOUND DELIMITER[%d]: %d.", _numParts, foundDelimiter));
      {
        TRACE_GET_MULTIPART(("... URLGetMultiPart: CURRENT PART ADDED %d BYTES.", foundDelimiter));
        _fpPart->WriteRaw(apIn,foundDelimiter);
      }
      {
        ReadPartFromFile();
      }
      const tU32 startSize = anSize-(p-(char*)apIn);
      TRACE_GET_MULTIPART(("... URLGetMultiPart: NEW PART %d BYTES.", foundDelimiter));
      _fpPart->WriteRaw((tPtr)p+delimiterSize,startSize-delimiterSize);
    }
    else {
      TRACE_GET_MULTIPART(("... URLGetMultiPart: CURRENT PART ADDED %d BYTES.", anSize));
      _fpPart->WriteRaw(apIn,anSize);
    }

    _size += anSize;
    return anSize;
  }
  tI64 __stdcall Tell() {
    return _size;
  }
  inline tI64 __stdcall GetSize() const {
    return _size;
  }

  void ReadPartFromFile() {
    if (_fpPart->GetSize() <= 0) {
      return;
    }
    ni::GetConcurrent()->SendMessage(
        _msgHandler,eCURLMessage_ReceivedPart,
        _fpPart.ptr(),_future.ptr());
    ++_numParts;
    ResetPart(_fpPart->GetSourcePath());
  }

  tBool ReadPartDelimiter(const cString& aHTTPHeader) {
    ReadPartFromFile();
    _partDelimiter = "--";
    _partDelimiter << aHTTPHeader.AfterI("Content-Type:").Before("\r\n").AfterI("boundary").After("=").Trim();
    if (_partDelimiter.empty()) {
      return eFalse;
    }
    return eTrue;
  }

  QPtr<iFuture> _future;
  Ptr<iFile> _fpPart;
  Ptr<iMessageHandler> _msgHandler;
  cString _partDelimiter;
  tI64 _size;
  tU32 _numParts;
};

class cCURL : public cIUnknownImpl<iCURL>
{
  niBeginClass(cCURL);
 public:
  tU32 mnRequestTimeoutInSecs;
  tU32 mnConnectionTimeoutInSecs;
  tHStringPtr mhspUserAgent;
  tHStringPtr mhspUserName;
  tHStringPtr mhspUserPass;
  eCURLHttpAuth mHttpAuth;

  cCURL() {
    mnConnectionTimeoutInSecs = 10;
    mnRequestTimeoutInSecs = 60;
    mhspUserAgent = _H("niCURL");
    mHttpAuth = eCURLHttpAuth_None;
  }

  virtual cString __stdcall GetVersion() const {
    return _ASTR(curl_version());
  }

  virtual cString __stdcall GetProtocols() const {
    cString o;
    curl_version_info_data* curlInfo = curl_version_info(CURLVERSION_NOW);
    for (int i = 0; curlInfo->protocols[i]; ++i) {
      if (i != 0) {
        o += ",";
      }
      o += curlInfo->protocols[i];
    }
    return o;
  }


  virtual void __stdcall SetConnectionTimeoutInSecs(tU32 anInSecs) {
    mnConnectionTimeoutInSecs = anInSecs;
  }
  virtual tU32 __stdcall GetConnectionTimeoutInSecs() const {
    return mnConnectionTimeoutInSecs;
  }

  virtual void __stdcall SetRequestTimeoutInSecs(tU32 anInSecs) {
    mnRequestTimeoutInSecs = anInSecs;
  }
  virtual tU32 __stdcall GetRequestTimeoutInSecs() const {
    return mnRequestTimeoutInSecs;
  }

  virtual void __stdcall SetUserAgent(const achar* aaszUserAgent) niImpl {
    mhspUserAgent = _H(aaszUserAgent);
  }
  virtual const achar* __stdcall GetUserAgent() const {
    return niHStr(mhspUserAgent);
  }

  virtual void __stdcall SetUserName(const achar* aaszUserName) {
    mhspUserName = _H(aaszUserName);
  }

  virtual const achar* __stdcall GetUserName() const {
    return niHStr(mhspUserName);
  }

  virtual void __stdcall SetUserPass(const achar* aaszUserPass) {
    mhspUserPass = _H(aaszUserPass);
  }

  virtual const achar* __stdcall GetUserPass() const {
    return niHStr(mhspUserPass);
  }

  virtual void __stdcall SetHttpAuth(eCURLHttpAuth aHttpAuth) {
    mHttpAuth = aHttpAuth;
  }

  virtual const eCURLHttpAuth __stdcall GetHttpAuth() const {
    return mHttpAuth;
  }

  void _CURLSetBase(
      CURLRunnable* runnable, CURL* curl,
      iMessageHandler* apMessageHandler,
      const achar* aURL, iFile* apRecvData, iFile* apRecvHeader)
  {
    runnable->_msgHandler = apMessageHandler;


    if (SHOULD_TRACE_CURL()) {
      curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    }

    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, _curlTrace);

    curl_easy_setopt(curl, CURLOPT_USERAGENT, niHStr(mhspUserAgent));

    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, mnConnectionTimeoutInSecs); // timeout for the connection in secs
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, mnRequestTimeoutInSecs); // timeout for the whole query in secs
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    curl_easy_setopt(curl, CURLOPT_URL, aURL);

    runnable->_recvData = apRecvData;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CURLRunnable::_WriteRecvDataCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, runnable);

    runnable->_recvHeader = apRecvHeader;
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, CURLRunnable::_WriteRecvHeaderCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, runnable);

    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION,CURLRunnable::_ProgressCallback);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, runnable);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);

    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, mHttpAuth);

    if (mHttpAuth != eCURLHttpAuth_None) {
      curl_easy_setopt(curl, CURLOPT_USERNAME, niHStr(mhspUserName));
      curl_easy_setopt(curl, CURLOPT_PASSWORD, niHStr(mhspUserPass));
    }
  }

  void _CURLSetPostData(
      CURLRunnable* runnable, CURL* curl,
      iFile* apPostData)
  {
    curl_easy_setopt(curl,CURLOPT_POSTFIELDSIZE,apPostData?apPostData->GetSize():0);
    runnable->_postData = apPostData;
    if (runnable->_postData.IsOK()) {
      curl_easy_setopt(curl,CURLOPT_READFUNCTION, CURLRunnable::_ReadPostDataCallback);
      curl_easy_setopt(curl,CURLOPT_READDATA, runnable);
    }
  }

  void _CURLSetHeaders(
      CURLRunnable* runnable, CURL* curl,
      const tStringCVec* apHeaders, const achar* aContentType)
  {
    niUnused(runnable);
    struct curl_slist* curlHeaders = NULL;
    if (niStringIsOK(aContentType)) {
      cString contentFull = "Content-Type: ";
      contentFull += aContentType;
      curlHeaders = curl_slist_append(curlHeaders, contentFull.Chars());
    }
    if (apHeaders != NULL && !apHeaders->empty()) {
      niLoop(i,apHeaders->size()) {
        curlHeaders = curl_slist_append(curlHeaders, apHeaders->at(i).Chars());
      }
    }
    if (curlHeaders != NULL) {
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curlHeaders);
    }
  }

  virtual Ptr<iRunnable> __stdcall URLGet(
      iMessageHandler* apMessageHandler,
      const achar* aURL, iFile* apRecvData = NULL, iFile* apRecvHeader = NULL, const tStringCVec* apHeaders = NULL)
  {
    niCheckIsOK(apRecvData,NULL);
    niCheckIsOK(apRecvHeader,NULL);

    CURL* curl = curl_easy_init();
    if (!curl) {
      niError("Can't create CURL handle.");
      return NULL;
    }

    Ptr<CURLRunnable> runnable = niNew CURLRunnable(curl,NULL);
    _CURLSetBase(runnable, curl, apMessageHandler, aURL, apRecvData, apRecvHeader);
    _CURLSetHeaders(runnable, curl, apHeaders, NULL);

    return runnable.ptr();
  }

  virtual Ptr<iRunnable> __stdcall URLPostFile(
      iMessageHandler* apMessageHandler,
      const achar* aURL, iFile* apRecvData, iFile* apRecvHeader,
      iFile* apPostData, const tStringCVec* apHeaders,
      const achar* aContentType)
  {
    niCheckIsOK(apRecvData, NULL);
    niCheckIsOK(apRecvHeader, NULL);

    CURL* curl = curl_easy_init();
    if (!curl) {
      niError("Can't create CURL handle.");
      return NULL;
    }

    Ptr<CURLRunnable> runnable = niNew CURLRunnable(curl,NULL);
    _CURLSetBase(runnable, curl, apMessageHandler, aURL, apRecvData, apRecvHeader);
    _CURLSetHeaders(runnable, curl, apHeaders, aContentType);
    _CURLSetPostData(runnable, curl, apPostData);

    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    return runnable.ptr();
  }

  virtual Ptr<iRunnable> __stdcall URLPostFields(
      iMessageHandler* apMessageHandler,
      const achar* aURL, iFile* apRecvData, iFile* apRecvHeader,
      const achar* aPostFields, const tStringCVec* apHeaders, const achar* aContentType)
  {
    Ptr<iFile> ptrPostData;
    if (niStringIsOK(aPostFields)) {
      ptrPostData = ni::CreateFileDynamicMemory(0,NULL);
      niCheckIsOK(ptrPostData,NULL);
      ptrPostData->WriteString(aPostFields);
      ptrPostData->SeekSet(0);
    }
    return URLPostFile(apMessageHandler, aURL, apRecvData, apRecvHeader, ptrPostData, apHeaders, aContentType);
  }

  //! {NoAutomation}
  virtual Ptr<iRunnable> __stdcall URLPostRaw(
      iMessageHandler* apMessageHandler,
      const achar* aURL, iFile* apRecvData, iFile* apRecvHeader,
      tPtr aPostData, tSize anPostDataSize,
      const tStringCVec* apHeaders, const achar* aContentType)
  {
    Ptr<iFile> ptrPostData = ni::CreateFileDynamicMemory(0,NULL);
    niCheckIsOK(ptrPostData,NULL);
    ptrPostData->WriteRaw(aPostData, anPostDataSize);
    ptrPostData->SeekSet(0);
    return URLPostFile(apMessageHandler, aURL, apRecvData, apRecvHeader, ptrPostData, apHeaders, aContentType);
  }

  //! Post with Content-Type: multipart/form-data.
  virtual Ptr<iRunnable> __stdcall URLPostMultiPart(
      iMessageHandler* apMessageHandler,
      const achar* aURL, iFile* apRecvData, iFile* apRecvHeader,
      tStringCMap* apPostFields)
  {
    niCheckIsOK(apRecvData, NULL);
    niCheckIsOK(apRecvHeader, NULL);

    if (!apPostFields || apPostFields->empty()) {
      niError("No post fields specified.");
      return NULL;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
      niError("Can't create CURL handle.");
      return NULL;
    }

    CURL* multi_handle = curl_multi_init();
    if (!multi_handle) {
      niError("Can't create CURL multi handle.");
      return NULL;
    }

    Ptr<CURLRunnable> runnable = niNew CURLRunnable(curl,multi_handle);
    _CURLSetBase(runnable, curl, apMessageHandler, aURL, apRecvData, apRecvHeader);

    struct curl_httppost* lastptr=NULL;
    for (tStringCMap::const_iterator it = apPostFields->begin(); it != apPostFields->end(); ++it) {
      const achar* name = it->first.Chars();
      const achar* val = it->second.Chars();
      if (*val == '@') {
        ++val; // skip the @
        /* Fill in the file upload field. This makes libcurl load data from
           the given file name when curl_easy_perform() is called. */
        curl_formadd(&runnable->_formPost,
                     &lastptr,
                     CURLFORM_COPYNAME, name,
                     CURLFORM_FILE, val,
                     CURLFORM_END);
      }
      else {
        /* Fill in the submit field too, even if this is rarely needed */
        curl_formadd(&runnable->_formPost,
                     &lastptr,
                     CURLFORM_COPYNAME, name,
                     CURLFORM_COPYCONTENTS, val,
                     CURLFORM_END);
      }
    }

    /* initialize custom header list (stating that Expect: 100-continue is not
       wanted) */
    static const char buf[] = "Expect:";
    runnable->_headerList = curl_slist_append(runnable->_headerList, buf);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, runnable->_headerList);

    curl_easy_setopt(curl, CURLOPT_HTTPPOST, runnable->_formPost);

    curl_multi_add_handle(multi_handle, curl);

    return runnable.ptr();
  }

  Ptr<iRunnable> __stdcall URLGetMultiPart(
      iMessageHandler* apMessageHandler, const achar* aURL, const achar* aPartExt)
  {
    niCheckIsOK(apMessageHandler,NULL);
    niCheck(niStringIsOK(aURL),NULL);

    Ptr<FileWritePart> writePart = niNew FileWritePart(aURL,aPartExt,apMessageHandler);
    Ptr<iFile> recvData = ni::CreateFile(writePart);
    Ptr<iFile> recvHeader = ni::CreateFileDynamicMemory(0,"");
    Ptr<iRunnable> runnable = this->URLGet(
        ni::MessageHandler([=] (tU32 anMsg, const Var& A, const Var& B) {
            writePart->_future = B;
            switch (anMsg) {
              case eCURLMessage_Started: {
                // niDebugFmt(("... Started"));
                break;
              }
              case eCURLMessage_ReceivingHeader: {
                recvHeader->Reset();
                // niDebugFmt(("... Receiving Header"));
                break;
              }
              case eCURLMessage_ReceivingData: {
                recvHeader->SeekSet(0);
                const cString header = recvHeader->ReadString();
                writePart->ReadPartDelimiter(header);
                // niDebugFmt(("... Received Header:---HTTP HEADER---\n%s------\nDelimiter: '%s'", header, writePart->_partDelimiter));
                // niDebugFmt(("... Receiving Data"));
                break;
              }
              case eCURLMessage_Progress: {
                // niDebugFmt(("... Progress %s", A));
                break;
              }
              case eCURLMessage_Completed: {
                // niDebugFmt(("... Completed"));
                break;
              }
              case eCURLMessage_ResponseCode: {
                // niDebugFmt(("... ResponseCode: %d", A));
                break;
              }
              case eCURLMessage_Failed: {
                // niDebugFmt(("... Failed: %s", A));
                break;
              }
              default: {
                niWarning(niFmt("Unexpected message '%s'.", MessageID_ToString(anMsg)));
                break;
              }
            }
            ni::GetConcurrent()->SendMessage(
                writePart->_msgHandler,anMsg,A,B);
          },
          // eInvalidHandle as threadId indicates that we want the message
          // handle to always be called from the caller thread. This implies
          // that the message handler is thread safe.
          eInvalidHandle),
        aURL,
        recvData,
        recvHeader);
    return runnable;
  }

  cString __stdcall URLGetString(const achar* aURL) {
    Ptr<iFile> recvData = ni::CreateFileDynamicMemory(0,"");
    Ptr<iFile> recvHeader = ni::CreateFileDynamicMemory(0,"");
    cString responseString;
    tI32 responseCode = -1;
    Ptr<iRunnable> runnable = this->URLGet(
      ni::MessageHandler([&] (tU32 anMsg, const Var& A, const Var& B) {
        switch (anMsg) {
          case eCURLMessage_Started: {
            TRACE_GET_STRING(("... URLGetString[%s]: Started", aURL));
            break;
          }
          case eCURLMessage_ReceivingHeader: {
            TRACE_GET_STRING(("... URLGetString[%s]: Receiving Header", aURL));
            break;
          }
          case eCURLMessage_ReceivingData: {
            TRACE_GET_STRING(("... URLGetString[%s]: Receiving Data", aURL));
            break;
          }
          case eCURLMessage_Progress: {
            TRACE_GET_STRING(("... URLGetString[%s]: Progress %s", aURL, A));
            break;
          }
          case eCURLMessage_Completed: {
            responseCode = A.GetIntValue();
            recvData->SeekSet(0);
            responseString = recvData->ReadString();
            if (niModuleShouldTrace_(niCURL,TraceGetString)) {
              TRACE_GET_STRING(("... URLGetString[%s]: Response: %d, Completed: %s",
                              aURL, responseCode, responseString));
            }
            break;
          }
          case eCURLMessage_ResponseCode: {
            TRACE_GET_STRING(("... URLGetString[%s]: ResponseCode: %d", aURL, A));
            break;
          }
          case eCURLMessage_Failed: {
            niError(niFmt("URLGetString[%s]: Request Failed: '%s'.", aURL, A));
            break;
          }
          default: {
            niError(niFmt("URLGetString[%s]: Unexpected message '%s'.", aURL, MessageID_ToString(anMsg)));
            break;
          }
        }
      }),
      aURL,
      recvData,
      recvHeader);
    if (!niIsOK(runnable)) {
      niError(niFmt("Can't create URLGet runnable"));
      return AZEROSTR;
    }

    runnable->Run();
    return responseString;
  }

  tI32 __stdcall URLGetDataTable(const achar* aURL, iDataTable* apDT) {
    Ptr<iFile> recvData = ni::CreateFileDynamicMemory(0,"");
    Ptr<iFile> recvHeader = ni::CreateFileDynamicMemory(0,"");
    tI32 responseCode = -1;
    Ptr<iRunnable> runnable = this->URLGet(
      ni::MessageHandler([&] (tU32 anMsg, const Var& A, const Var& B) {
        switch (anMsg) {
          case eCURLMessage_Started: {
            TRACE_GET_STRING(("... URLGetDataTable[%s]: Started", aURL));
            break;
          }
          case eCURLMessage_ReceivingHeader: {
            TRACE_GET_STRING(("... URLGetDataTable[%s]: Receiving Header", aURL));
            break;
          }
          case eCURLMessage_ReceivingData: {
            TRACE_GET_STRING(("... URLGetDataTable[%s]: Receiving Data", aURL));
            break;
          }
          case eCURLMessage_Progress: {
            TRACE_GET_STRING(("... URLGetDataTable[%s]: Progress %s", aURL, A));
            break;
          }
          case eCURLMessage_Completed: {
            responseCode = A.GetIntValue();
            if (apDT) {
              // for now we assume it's a json output
              recvData->SeekSet(0);
              _JsonParseFileToDataTable(recvData,apDT);
            }
            if (niModuleShouldTrace_(niCURL,TraceGetString)) {
              recvData->SeekSet(0);
              const cString content = recvData->ReadString();
              TRACE_GET_STRING(("... URLGetDataTable[%s]: Response: %d, Completed: %s",
                              aURL, responseCode, content));
            }
            break;
          }
          case eCURLMessage_ResponseCode: {
            TRACE_GET_STRING(("... URLGetDataTable[%s]: ResponseCode: %d", aURL, A));
            break;
          }
          case eCURLMessage_Failed: {
            niError(niFmt("URLGetDataTable[%s]: Request Failed: '%s'.", aURL, A));
            break;
          }
          default: {
            niError(niFmt("URLGetDataTable[%s]: Unexpected message '%s'.", aURL, MessageID_ToString(anMsg)));
            break;
          }
        }
      }),
      aURL,
      recvData,
      recvHeader);
    if (!niIsOK(runnable)) {
      niError(niFmt("Can't create URLGet runnable"));
      return -1;
    }

    runnable->Run();
    return responseCode;
  }

  niEndClass(cCURL);
};

niExportFunc(iUnknown*) New_niCURL_CURL(const Var&, const Var&) {
  return niNew cCURL();
}
