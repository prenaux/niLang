// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "API/niLang/Types.h"
#include "API/niLang/Utils/UnknownImpl.h"
#include "API/niLang/ILang.h"
#include "API/niLang/IProf.h"
#include "API/niLang/Utils/Sync.h"
#include "API/niLang/STL/bimap.h"
#include "DataTableWriteStack.h"
#include "DataTableReadStack.h"
#include "DataTablePath.h"

namespace ni {

//////////////////////////////////////////////////////////////////////////////////////////////
class cLang : public cIUnknownImpl<iLang,eIUnknownImplFlags_NoRefCount|eIUnknownImplFlags_NoMemoryAllocator>
{
  niBeginClass(cLang);

 public:
  static iUnknown* __stdcall _CreateInstance(const Var& aVarA, const Var& aVarB);

  cLang();
  ~cLang();
  void _Construct();

  tBool __stdcall IsOK() const;

  void __stdcall Invalidate();

  //// iLang //////////////////////////////////
#if niMinFeatures(15)
  iOSProcessManager* __stdcall GetProcessManager() const;
#endif
  tBool __stdcall OnExit(iRunnable* apRunnable);
  void __stdcall Exit(tU32 aulErrorCode);
  void __stdcall _PlatformExit(tU32 aulErrorCode);
  void __stdcall FatalError(const achar* aszMsg);
  void __stdcall SetEnv(const achar* aaszEnv, const achar* aaszValue) const;
  cString __stdcall GetEnv(const achar* aaszEnv) const;

  void __stdcall SetDefaultLocale(iHString* ahspLocale);
  iHString* __stdcall GetDefaultLocale() const;
  tU32 __stdcall GetNumLocales() const;
  iHString* __stdcall GetLocale(tU32 abIndex) const;
  tBool __stdcall ClearLocalization(iHString* ahspLocale);
  tBool __stdcall SetLocalization(iHString* ahspLocale, iHString* ahspNative, iHString* ahspLocalized);
  tBool __stdcall SetLocalizationMap(iHString* ahspLocale, const tStringCMap* apLocalizationMap);
  tU32 __stdcall GetLocalizationMap(iHString* ahspLocale, tStringCMap* apLocalizedMap) const;
  void __stdcall SetMarkMissingLocalization(tBool abMarkMissing);
  tBool __stdcall GetMarkMissingLocalization() const;
  tStringCVec* __stdcall GetMissingLocalization(iHString* locale) const;

  tUUID __stdcall CreateLocalUUID();
  tUUID __stdcall CreateGlobalUUID();

  const iTime* __stdcall GetCurrentTime() const;
  tF64  __stdcall TimerInSeconds() const;

  void __stdcall ResetFrameTime();
  tBool __stdcall UpdateFrameTime(const tF64 afElapsedTime);
  tF64 __stdcall GetTotalFrameTime() const;
  tF64 __stdcall GetFrameTime() const;
  tU32 __stdcall GetFrameNumber() const;
  tF32 __stdcall GetFrameRate() const;
  tU32 __stdcall GetAverageFrameRate() const;

  void __stdcall SetLogFilter(tU32 exclude);
  tU32 __stdcall GetLogFilter() const;
  void __stdcall Log(tLogFlags type, const achar* file, const achar* func, tU32 line, const achar* msg);

  iFileSystem* __stdcall GetRootFS() const;
  iFileSystem* __stdcall CreateFileSystemDir(const achar* aaszDir, tFileSystemRightsFlags aRights) const;
  iFileSystem* __stdcall CreateFileSystemHashed(const iFileSystem* apBaseFS) const;

  iFile* __stdcall CreateFile(iFileBase* apBase);
  iFileBase* __stdcall CreateFileBaseWriteDummy();
  iFile* __stdcall CreateFileWriteDummy();
  iFileBase* __stdcall CreateFileBaseMemory(tPtr pMem, tSize nSize, tBool bFree, const achar* aszPath);
  iFile* __stdcall CreateFileMemory(tPtr pMem, tSize nSize, tBool bFree, const achar* aszPath);
  iFileBase* __stdcall CreateFileBaseMemoryAlloc(tSize nSize, const achar* aszPath);
  iFile* __stdcall CreateFileMemoryAlloc(tSize nSize, const achar* aszPath);
  iFileBase* __stdcall CreateFileBaseDynamicMemory(tSize anSize, const achar* aszPath);
  iFile* __stdcall CreateFileDynamicMemory(tSize anSize, const achar* aszPath);
  iFileBase* __stdcall CreateFileBaseWindow(iFileBase* apBase, tI64 anBase, tI64 anSize, const achar* aaszPath, tBool abAutoSeekSet);
  iFile* __stdcall CreateFileWindow(iFileBase* apBase, tI64 anBase, tI64 anSize, const achar* aaszPath, tBool abAutoSeekSet);

  iFileBase* __stdcall CreateFileBaseBufferEncoder(iFileBase* apBase, iBufferEncoder* apEnc) niImpl;
  iFile* __stdcall CreateFileBufferEncoder(iFileBase* apBase, iBufferEncoder* apEnc) niImpl;
  iFileBase* __stdcall CreateFileBaseBufferDecoder(iFileBase* apBase, iBufferDecoder* apDec, tSize aDecodedSize) niImpl;
  iFile* __stdcall CreateFileBufferDecoder(iFileBase* apBase, iBufferDecoder* apDec, tSize aDecodedSize) niImpl;
  tSize __stdcall FileBufferEncode(iBufferEncoder* apEnc, iFile* apSrc, tSize anSrcSize, iFile* apDest) niImpl;
  tSize __stdcall FileBufferDecode(iBufferDecoder* apDec, iFile* apSrc, tSize anSrcSize, iFile* apDest, tSize anDestSize) niImpl;

  tI32 __stdcall RunCommand(const achar* aaszCmd);
  tI32 __stdcall StartPath(const achar* aaszFile);

#if niMinFeatures(20)
  tBool __stdcall JsonParseFile(iFile* apFile, iJsonParserSink* apSink);
  tBool __stdcall JsonParseString(const cString& aString, iJsonParserSink* apSink);
  iJsonWriter* __stdcall CreateJsonSinkWriter(iJsonWriterSink* apSink, tBool abPrettyPrint);
  iJsonWriter* __stdcall CreateJsonFileWriter(iFile* apFile, tBool abPrettyPrint);
#endif

#if niMinFeatures(20)
  ni::tBool __stdcall XmlParseFile(ni::iFile* apFile, ni::iXmlParserSink* apSink);
  ni::tBool __stdcall XmlParseString(const ni::cString& aString, ni::iXmlParserSink* apSink);
#endif

#if niMinFeatures(15)
  tBool __stdcall AddScriptingHost(iHString* ahspName, iScriptingHost* apHost);
  tBool __stdcall RemoveScriptingHost(iHString* ahspName);
  tU32 __stdcall GetNumScriptingHosts() const;
  iHString* __stdcall GetScriptingHostName(tU32 anIndex) const;
  iScriptingHost* __stdcall GetScriptingHost(tU32 anIndex);
  iScriptingHost* __stdcall GetScriptingHostFromName(iHString* ahspName);
  tU32 __stdcall GetScriptingHostIndexFromName(iHString* ahspName);
  tU32 __stdcall GetScriptingHostIndex(iScriptingHost* apHost) const;
  void __stdcall ServiceAllScriptingHosts(tBool abForceGC);
  iScriptingHost* __stdcall FindScriptingHost(iHString* ahspContext, iHString* ahspCodeResource);
#endif

  iExpressionContext* __stdcall CreateExpressionContext() niImpl;
  iExpressionContext* __stdcall GetExpressionContext() const niImpl;
  Ptr<iExpressionVariable> __stdcall Eval(const achar* aaszExpr) niImpl;
  cString __stdcall EnumToString(tU32 anValue, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags) niImpl;
  tU32 __stdcall StringToEnum(const achar* aExpr, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags) niImpl;

  iProf* __stdcall GetProf() const;

  cString __stdcall URLGetProtocol(const achar* aURL) niImpl;
  iURLFileHandler* __stdcall URLGetHandler(const achar* aURL) niImpl;
  iFile* __stdcall URLOpen(const achar* aURL) niImpl;
  tBool __stdcall URLExists(const achar* aURL) niImpl;
  cString __stdcall URLFindFilePath(const achar* aszRes, const achar* aszBasePath, const achar* aLoaderPrefix) niImpl;

  //// iOSPlatform //////////////////////////////////
  tU32 __stdcall GetNumMonitors() const;
  tU32 __stdcall GetMonitorIndex(tIntPtr aHandle) const;
  tIntPtr __stdcall GetMonitorHandle(tU32 anIndex) const;
  const achar* __stdcall GetMonitorName(tU32 anIndex) const;
  sRecti __stdcall GetMonitorRect(tU32 anIndex) const;
  tOSMonitorFlags __stdcall GetMonitorFlags(tU32 anIndex) const;
  iOSWindow* __stdcall CreateWindow(iOSWindow* apParent, const achar* aaszTitle, const sRecti& aRect, tOSWindowCreateFlags aCreate, tOSWindowStyleFlags aStyle);
  iOSWindow* __stdcall CreateWindowEx(tIntPtr aOSWindowHandle, tOSWindowCreateFlags aCreate);
  eOSMessageBoxReturn __stdcall MessageBox(iOSWindow* apParent, const achar* aaszTitle, const achar* aaszText, tOSMessageBoxFlags aFlags);
  cString __stdcall OpenFileDialog(iOSWindow* aParent, const achar* aTitle, const achar* aFilter, const achar* aInitDir) niImpl;
  cString __stdcall SaveFileDialog(iOSWindow* aParent, const achar* aTitle, const achar* aFilter, const achar* aInitDir) niImpl;
  cString __stdcall PickDirectoryDialog(iOSWindow* aParent, const achar* aTitle, const achar* aInitDir) niImpl;
  tU32 __stdcall GetNumGameCtrls() const;
  iGameCtrl* __stdcall GetGameCtrl(tU32 aulIdx) const;

  tMessageHandlerSinkLst* __stdcall GetSystemMessageHandlers() const;

  tBool __stdcall IsSerializedObject(iFile* apFile, const achar* aID, tI64* apObjSize);
  iUnknown* __stdcall SerializeObject(iFile* apFile, iUnknown* apObject, eSerializeMode aMode, tI64* apSize);
  tI64 __stdcall ReadSerializationHeader(iFile* apFile, cString* apID, tI64* apObjSize);
  tI64 __stdcall WriteSerializationHeader(iFile* apFile, const achar* apID, tI64 anSize);

  iDeviceResourceManager* __stdcall CreateDeviceResourceManager(const achar* aszType);

  iDataTable* __stdcall CreateDataTable(const achar* aaszName);
  tBool __stdcall SerializeDataTable(const achar* aaszSerName, eSerializeMode aMode, iDataTable* apTable, iFile* apFile);
  iDataTableWriteStack* __stdcall CreateDataTableWriteStack(iDataTable* apDT);
  iDataTableWriteStack* __stdcall CreateDataTableWriteStackFromName(const achar* aaszName);
  iDataTableReadStack* __stdcall CreateDataTableReadStack(iDataTable* apDT);
  cString __stdcall GetAbsoluteDataTablePath(iDataTable* apDT, tU32 anPropIndex);

  tBool __stdcall SetClipboard(eClipboardType aType, iDataTable* apDT);
  iDataTable* __stdcall GetClipboard(eClipboardType aType) const;

  Ptr<iHString> __stdcall CreateHString(const cString& astrKey);
  tBool __stdcall LoadLocalization(iDataTable* apDT);
  //// iLang //////////////////////////////////

 public:
  // FrameTime
  tF64 mfLastElapsedTime;
  tF64 mfTotalFrameTime;
  tF64 mfFrameTime;
  tU32 mnFrameNumber;
  tF32 mfAvgCounter;
  tU32 mnAverageFPSCounter;
  tU32 mnAverageFPS;

  // HString
  void __stdcall _InvalidateHStringTable();

  typedef astl::hash_map<cString,iHString*>      tStringTableHMap;
  ni::ThreadMutex mmutexHStringTable;
  niSync tStringTableHMap mmapHStrings;
  void _UnregisterHString(iHString* apHString);

  typedef astl::hstring_hash_map<Ptr<iHString> > tLocalizationMap;
  struct sLocalizationTable {
    sLocalizationTable() {
      missing = tStringCVec::Create();
    }
    Ptr<tStringCVec> missing;
    tLocalizationMap map;
  };
  typedef astl::hstring_hash_map<sLocalizationTable*> tLocalesHMap;
  ni::ThreadMutex mmutexLocales;
  niSync tHStringPtr mhspDefaultLocale;
  niSync tLocalesHMap mmapLocales;
  tBool mbMarkMissingLocalization;

  sLocalizationTable* _GetLocalizationTable(iHString* locale, tBool abNew) const;
  iHString* __stdcall _GetLocalized(iHString* locale, iHString* native);

  // Interfaces
  typedef astl::map<cString,Ptr<iUnknown> > tGlobalInstanceMap;
  tGlobalInstanceMap  mmapGlobalInstances;

  // System Properties
  static void _InitDefaultSystemProperties(tStringCMap* prop);
  Ptr<tStringCMap> mptrSystemProperties;
  const tStringCMap* __stdcall GetProperties() const;
  tBool __stdcall HasProperty(const achar* aaszName) const;
  void __stdcall SetProperty(const achar* aaszName, const achar* aaszValue);
  cString __stdcall GetProperty(const achar* aaszName) const;

  // Scripting hosts
  __sync_mutex_(ScriptingHosts);
  struct sSH {
    tHStringPtr hspName;
    Ptr<iScriptingHost> ptrHost;
  };
  typedef astl::vector<sSH> tSHVec;
  tSHVec mvSH;

  // Exit
  astl::vector<Ptr<iRunnable> > mvOnExit;

  // Modules
  void _StartupModules();

  struct sModuleDef {
    sModuleDef() : mhDLL(NULL) {}
    tIntPtr mhDLL;
    Ptr<iModuleDef> mptrModuleDef;
  };
  astl::vector<sModuleDef> mvModuleDefs;

  tBool __stdcall _FinalizeRegisterModuleDef(const sModuleDef& aMod);

  tBool __stdcall RegisterModuleDef(const iModuleDef* apDef) niImpl;
  tU32 __stdcall GetNumModuleDefs() const niImpl;
  const iModuleDef* __stdcall GetModuleDef(tU32 anIndex) const niImpl;
  tU32 __stdcall GetModuleDefIndex(const achar* aaszName) const niImpl;
  const iModuleDef* __stdcall LoadModuleDef(const achar* aMID, const achar* aaszFile = NULL) niImpl;

  Nonnull<tCreateInstanceCMap> mmapCreateInstance;
  tCreateInstanceCMap* __stdcall GetCreateInstanceMap() const {
    return mmapCreateInstance;
  }
  iUnknown* __stdcall CreateInstance(
      const achar* aOID,
      const Var& aVarA = niVarNull, const Var& aVarB = niVarNull) niImpl;

  Nonnull<tGlobalInstanceCMap> mmapGlobalInstance;
  tGlobalInstanceCMap* __stdcall GetGlobalInstanceMap() const {
    return mmapGlobalInstance;
  }
  tBool __stdcall SetGlobalInstance(const achar* aaszName, iUnknown* apInstance) niImpl;
  iUnknown* __stdcall GetGlobalInstance(const achar* aaszName) const niImpl;

  typedef astl::map<cString,const sEnumDef*> tEnumDefMap;
  astl::map<cString,const sEnumDef*> mmapEnumDefs;
  tBool __stdcall RegisterEnumDef(const sEnumDef* apEnumDef);
  const sEnumDef* __stdcall GetEnumDef(const achar* aEID) niImpl;

  typedef astl::bimap<tUUID,tHStringPtr> tUUIDNameBMap;
  tUUIDNameBMap mbmapUUIDNames;
  typedef astl::map<tUUID,const sInterfaceDef*> tUUIDDefMap;
  tUUIDDefMap mmapUUIDDef;
  iHString* __stdcall GetInterfaceName(const tUUID& aUUID) const niImpl;
  const tUUID& __stdcall GetInterfaceUUID(iHString* ahspStr) const niImpl;
  const sInterfaceDef* __stdcall GetInterfaceDefFromUUID(const tUUID& aUUID) const niImpl;

  void _PlatformStartup();

  Ptr<tMessageHandlerSinkLst>  mptrSystemMessageHandlers;

  Ptr<iDataTable> mptrClipboard[eClipboardType_Last];

  Ptr<iExpressionContext> _ctx;

  niEndClass(cLang);
};

ni::iUnknown* iMessageHandler_CreateDispatchWrapper(ni::iDispatch* apDispatch);
cLang* GetLangImpl();

}
//////////////////////////////////////////////////////////////////////////////////////////////
#endif // __SYSTEM_H__
