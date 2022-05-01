#include "stdafx.h"
#include <niCURL.h>
#include <niCURL_ModuleDef.h>
#include <niLang/Utils/CryptoUtils.h>
#include <niLang/Utils/ProcessUtils.h>
#include <niLang/Utils/ConcurrentImpl.h>
#include <niLang/Utils/DataTableUtils.h>

using namespace ni;

#define TRACE_REMOTE_COMPILE(MSG) //niDebugFmt(MSG)

static Ptr<iCURL> _GetDataTableCURL() {
  static tBool _isInitialized;
  static Ptr<iCURL> _curl;
  if (!_isInitialized) {
    _curl = (iCURL*)New_niCURL_CURL(niVarNull, niVarNull);
    _isInitialized = eTrue;
    if (!_curl.IsOK()) {
      niWarning("Can't initialize CURL for DataTable actions.");
    }
  }
  return _curl.ptr();
}

Ptr<iFile> __stdcall RemoteCompileShader(const achar* aServerUrl, const achar* aaszProfile, const achar* aaszName, const achar* aaszPPPath) {
  Ptr<iCURL> _curl = _GetDataTableCURL();
  if (!_curl.IsOK()) {
    niError("Can't get CURL.");
    return NULL;
  }

  TRACE_REMOTE_COMPILE(("... RemoteCompileShader: server: %s, profile: %s, name: %s, path: %s",
                 aServerUrl, aaszProfile, aaszName, aaszPPPath));

  Ptr<iDataTable> dt = ni::CreateDataTable("Result");
  cString url = aServerUrl;
  url += "/api/1/shader_compiler/compile";
  url += _ASTR("?profile=") + aaszProfile;
  url += _ASTR("&name=") + aaszName;
  Ptr<tStringCMap> fields = tStringCMap::Create();
  // astl::upsert(*fields, _ASTR("profile"), aaszProfile);
  // astl::upsert(*fields, _ASTR("name"), aaszName);
  astl::upsert(*fields, _ASTR("file"), niFmt("@%s",aaszPPPath));
  TRACE_REMOTE_COMPILE(("... RemoteCompileShader[%s]: Fetching URL", url, aaszName));
  Ptr<iFile> recvData = ni::CreateFileDynamicMemory(0,"");
  Ptr<iFile> recvHeader = ni::CreateFileDynamicMemory(0,"");
  tBool bCompleted = eFalse;
  Ptr<iRunnable> runnable = _curl->URLPostMultiPart(ni::MessageHandler([&] (tU32 anMsg, const Var& A, const Var& B) {
        switch (anMsg) {
          case eCURLMessage_Started: {
            TRACE_REMOTE_COMPILE(("... RemoteCompileShader[%s]: Started", url, aaszName));
            break;
          }
          case eCURLMessage_ReceivingHeader: {
            TRACE_REMOTE_COMPILE(("... RemoteCompileShader[%s]: Receiving Header", url, aaszName));
            break;
          }
          case eCURLMessage_ReceivingData: {
            TRACE_REMOTE_COMPILE(("... RemoteCompileShader[%s]: Receiving Data", url, aaszName));
            break;
          }
          case eCURLMessage_Progress: {
            TRACE_REMOTE_COMPILE(("... RemoteCompileShader[%s]: Progress %s", url, A));
            break;
          }
          case eCURLMessage_Completed: {
            const tI64 responseCode = A.GetIntValue();
            if (responseCode >= 200 && responseCode <= 299) {
              bCompleted = eTrue;
              recvData->SeekSet(0);
              const cString content = recvData->ReadString();
              JsonParseStringToDataTable(content,dt);
              TRACE_REMOTE_COMPILE(("... RemoteCompileShader[%s]: Completed: %s", url, content));
            }
            else {
              recvData->SeekSet(0);
              const cString content = recvData->ReadString();
              JsonParseStringToDataTable(content,dt);
              niError(niFmt("RemoteCompileShader[%s]: Invalid ResponseCode: '%s'. Error: %s", url, A,
                            dt->GetString("desc")));
            }
            break;
          }
          case eCURLMessage_ResponseCode: {
            TRACE_REMOTE_COMPILE(("... RemoteCompileShader[%s]: ResponseCode: %d", url, A));
            break;
          }
          case eCURLMessage_Failed: {
            niError(niFmt("RemoteCompileShader[%s]: Request Failed: '%s'.", url, A));
            break;
          }
          default: {
            niError(niFmt("RemoteCompileShader[%s]: Unexpected message '%s'.", url, MessageID_ToString(anMsg)));
            break;
          }
        }
      }),
    url.Chars(),
    recvData,
    recvHeader,
    fields);
  runnable->Run();

  if (!bCompleted) {
    niError("Remote shader compilation failed.");
    return NULL;
  }

  const cString compiled = dt->GetString("compiled");

  TRACE_REMOTE_COMPILE(("... RemoteCompileShader, compiled: %s", compiled));

  Ptr<iFile> fpOutputMem = ni::CreateFileDynamicMemory(0,"");
  fpOutputMem->WriteRawFromString(eRawToStringEncoding_Base64, compiled.Chars());
  fpOutputMem->SeekSet(0);
  return fpOutputMem;
}
