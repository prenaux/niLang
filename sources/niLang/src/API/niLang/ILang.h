#pragma once
#ifndef __ILANG_H_1CD42183_7376_49A5_BEE6_0141CD400B64__
#define __ILANG_H_1CD42183_7376_49A5_BEE6_0141CD400B64__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "StringDef.h"
#include "IOSProcess.h"
#include "IFileSystem.h"
#include "ITime.h"
#include "IHString.h"
#include "IJson.h"
#include "IXml.h"
#include "IScriptingHost.h"
#include "IExpression.h"
#include "ISerializable.h"
#include "IOSWindow.h"
#include "Utils/CollectionImpl.h"
#include "Math/MathRect.h"

namespace ni {
struct iRunnable;
struct iProf;
struct iCallback;
struct iDataTable;
struct iDataTableReadStack;
struct iDataTableWriteStack;
struct iDeviceResourceManager;
struct iGameCtrl;

/** \addtogroup niLang
 * @{
 */

//! Create instance callback map.
typedef tStringInterfaceCMap<iCallback> tCreateInstanceCMap;

//! Global instance map.
typedef tStringIUnknownCMap tGlobalInstanceCMap;

//! Enum to string flags
enum eEnumToStringFlags {
  //! Dont use the specified enum as default enumeration (will fully qualify the enumeration value's name).
  eEnumToStringFlags_Full = niBit(0),
  //! Handle the enumeration as a set of flags.
  eEnumToStringFlags_Flags = niBit(1),
  //! Search all registered enumerations if the specified fully qualified enumeration can't be found.
  eEnumToStringFlags_GlobalSearch = niBit(2),
  //! \internal
  eEnumToStringFlags_ForceDWORD = 0xFFFFFFFF
};

//! \see ni::eEnumToStringFlags
typedef tU32 tEnumToStringFlags;

//! System messages id.
enum eSystemMessage
{
  //! The application exits.
  //! \param A: unused
  //! \param B: unused
  eSystemMessage_Exit = niMessageID('_','S','Y','S','x'),
  //! Sent to a child process to notify which is the actual parent window.
  //! \param A: parent window handle
  eSystemMessage_SetupParentWindow = niMessageID('_','S','Y','S','W'),
  //! Sent to a parent process to notify which is the actual child window created.
  //! \param A: parent window handle
  //! \remark Sent as reply of SetupParentWindow
  eSystemMessage_SetupClientWindow = niMessageID('_','S','Y','S','w'),
  //! Log message.
  //! \param A: log type
  //! \param B: unused
  eSystemMessage_Log = niMessageID('_','S','Y','S','L'),
  //! Open file.
  //! \param A: file path
  //! \param B: unused
  eSystemMessage_OpenFile = niMessageID('_','S','Y','S','O'),
  //! \internal
  eSystemMessage_ForceDWORD = 0xFFFFFFFF
};

//! Clipboard types
enum eClipboardType
{
  //! System clipboard.
  eClipboardType_System = 0,
  //! Memory clipboard 1.
  eClipboardType_Memory1 = 1,
  //! Memory clipboard 2.
  eClipboardType_Memory2 = 2,
  //! Memory clipboard 3.
  eClipboardType_Memory3 = 3,
  //! Memory clipboard 4.
  eClipboardType_Memory4 = 4,
  //! Memory clipboard 5.
  eClipboardType_Memory5 = 5,
  //! Memory clipboard 6.
  eClipboardType_Memory6 = 6,
  //! Memory clipboard 7.
  eClipboardType_Memory7 = 7,
  //! Memory clipboard 8.
  eClipboardType_Memory8 = 8,
  //! Memory clipboard 9.
  eClipboardType_Memory9 = 9,
  //! Memory clipboard 10.
  eClipboardType_Memory10 = 10,
  //! \internal
  eClipboardType_Last = 11,
  //! \internal
  eClipboardType_ForceDWORD = 0xFFFFFFFF
};

//////////////////////////////////////////////////////////////////////////////////////////////
struct iLang : public iUnknown
{
  niDeclareInterfaceUUID(iLang,0x8e2cebd1,0x783b,0x4b0b,0xb7,0xb3,0x62,0xa3,0x87,0xd1,0x90,0xb1)

  //########################################################################################
  //! \name Platform
  //########################################################################################
  //! @{

#if niMinFeatures(15)
  //! Get the platform's process manager.
  //! {Property}
  virtual iOSProcessManager* __stdcall GetProcessManager() const = 0;
#endif

  //! Forcefully exit the application.
  virtual void __stdcall Exit(tU32 aulErrorCode) = 0;
  //! Adds a runnable that will be called when the application exits through iLang::Exit().
  //! \return eTrue if OnExit is supported, else eFalse.
  virtual tBool __stdcall OnExit(iRunnable* apRunnable) = 0;
  //! Show an error message box then exit.
  virtual void __stdcall FatalError(const achar* aszMsg) = 0;

  //! Set an OS environment variable.
  //! {Property}
  virtual void __stdcall SetEnv(const achar* aaszEnv, const achar* aaszValue) const = 0;
  //! Get an OS environment variable.
  //! {Property}
  virtual cString __stdcall GetEnv(const achar* aaszEnv) const = 0;
  //! @}

  //########################################################################################
  //! \name System
  //########################################################################################
  //! @{

  //! Get the system properties.
  //! {Property}
  virtual const tStringCMap* __stdcall GetProperties() const = 0;
  //! Check whether the specified system property exists.
  virtual tBool __stdcall HasProperty(const achar* aaszName) const = 0;
  //! Set the specified system property.
  //! {Property}
  virtual void __stdcall SetProperty(const achar* aaszName, const achar* aaszValue) = 0;
  //! Get the specified system property.
  //! {Property}
  virtual cString __stdcall GetProperty(const achar* aaszName) const = 0;

  //! Register a module definition.
  virtual tBool __stdcall RegisterModuleDef(const iModuleDef* apDef) = 0;
  //! Get the number of modules registered.
  //! {Property}
  virtual tU32 __stdcall GetNumModuleDefs() const = 0;
  //! Get the module at the specified index.
  //! {Property}
  virtual const iModuleDef* __stdcall GetModuleDef(tU32 anIndex) const = 0;
  //! Get the index of the module with the specified name.
  //! {Property}
  virtual tU32 __stdcall GetModuleDefIndex(const achar* aaszName) const = 0;
  //! Load a module definition from a DLL.
  virtual const iModuleDef* __stdcall LoadModuleDef(const achar* aName, const achar* aaszFile = NULL) = 0;

  //! Create instance map.
  //! {Property}
  virtual tCreateInstanceCMap* __stdcall GetCreateInstanceMap() const = 0;
  //! Create instance of the specified object type.
  virtual iUnknown* __stdcall CreateInstance(const achar* aOID, const Var& aVarA = niVarNull, const Var& aVarB = niVarNull) = 0;

  //! Get the global instance map.
  //! {Property}
  virtual tGlobalInstanceCMap* __stdcall GetGlobalInstanceMap() const = 0;
  //! Set a global instance.
  //! \remark All registered global instance will be included in the query interface
  //!     on iLang objects. By example if you register an object that implements
  //!     iResource you can do QueryInterface<iResource>(GetLang()) and it will return
  //!     the instance of the registered global instance.
  //! {Property}
  virtual tBool __stdcall SetGlobalInstance(const achar* aaszName, iUnknown* apInstance) = 0;
  //! Get a global instance.
  //! {Property}
  virtual iUnknown* __stdcall GetGlobalInstance(const achar* aaszName) const = 0;

  //! Register an enumeration definition.
  virtual tBool __stdcall RegisterEnumDef(const sEnumDef* apEnumDef) = 0;
  //! Get an enum definition from the specified name.
  virtual const sEnumDef* __stdcall GetEnumDef(const achar* aEID) = 0;

  //! Get the name of the interface with the specified UUID.
  //! {Property}
  virtual iHString* __stdcall GetInterfaceName(const tUUID& aUUID) const = 0;
  //! Get the UUID of the interface with the specified name.
  //! {Property}
  virtual const tUUID& __stdcall GetInterfaceUUID(iHString* ahspStr) const = 0;
  //! Get the interface definition from the specified UUID.
  virtual const sInterfaceDef* __stdcall GetInterfaceDefFromUUID(const tUUID& aUUID) const = 0;
  //! @}

  //########################################################################################
  //! \name HString & Localization
  //########################################################################################
  //! @{

  //! Set the default locale.
  //! {Property}
  virtual void __stdcall SetDefaultLocale(iHString* ahspLocale) = 0;
  //! Get the default locale.
  //! {Property}
  virtual iHString* __stdcall GetDefaultLocale() const = 0;

  //! Return the number of locales that have at least one localized string.
  //! {Property}
  virtual tU32 __stdcall GetNumLocales() const = 0;
  //! Return the name of the locale at the specified index.
  //! {Property}
  virtual iHString* __stdcall GetLocale(tU32 abIndex) const = 0;
  //! Remove all localized strings of the specified locale.
  virtual tBool __stdcall ClearLocalization(iHString* ahspLocale) = 0;

  //! Set the localization of the specified 'native' string.
  virtual tBool __stdcall SetLocalization(iHString* ahspLocale, iHString* ahspNative, iHString* ahspLocalized) = 0;
  //! Set the localization of the specified of all strings in the specified map.
  virtual tBool __stdcall SetLocalizationMap(iHString* ahspLocale, const tStringCMap* apLocalizationMap) = 0;
  //! Get all localized strings of the specified locale.
  virtual tU32 __stdcall GetLocalizationMap(iHString* ahspLocale, tStringCMap* apLocalizedMap) const = 0;

  //! Set whether the string table should mark missing locale translations.
  //! {Property}
  virtual void __stdcall SetMarkMissingLocalization(tBool abMarkMissing) = 0;
  //! Get whether the string table should mark missing locale translations.
  //! {Property}
  virtual tBool __stdcall GetMarkMissingLocalization() const = 0;
  //! Get all missing strings of the specified locale.
  //! \remark This retrieves all strings which should have been translated but are not.
  virtual tStringCVec* __stdcall GetMissingLocalization(iHString* locale) const = 0;
  //! @}

  //########################################################################################
  //! \name UUID
  //########################################################################################
  //! @{

  //! Create a local UUID.
  //! \remark A local UUID is garanteed to be unique only on the local computer.
  //! \remark Generating a local UUID can be considered to be faster in general.
  virtual tUUID __stdcall CreateLocalUUID() = 0;
  //! Create a global UUID.
  //! \remark A gloabl UUID is garanteed to be unique among all computers.
  //! \remark Generating a global UUID should be considered as being a slow operation.
  virtual tUUID __stdcall CreateGlobalUUID() = 0;
  //! @}

  //########################################################################################
  //! \name Log
  //########################################################################################
  //! @{

  //! Set the log filter.
  //! {Property}
  virtual void __stdcall SetLogFilter(tU32 exclude) = 0;
  //! Get the log filter.
  //! {Property}
  virtual tU32 __stdcall GetLogFilter() const = 0;
  //! Log a message.
  virtual void __stdcall Log(tLogFlags type, const achar* msg, const achar* file, tU32 line, const achar* func) = 0;
  //! @}

  //########################################################################################
  //! \name File system
  //########################################################################################
  //! @{

  //! Get the root file system.
  //! {Property}
  virtual iFileSystem* __stdcall GetRootFS() const = 0;
  //! Create a directory file system.
  virtual iFileSystem* __stdcall CreateFileSystemDir(const achar* aaszDir, tFileSystemRightsFlags aRights) const = 0;
  //! Creates a file system that store the file using its hashed file name.
  //! \remark This allows to use anything as filename and use the underlying file system more as a key/value store.
  virtual iFileSystem* __stdcall CreateFileSystemHashed(const iFileSystem* apFS) const = 0;
  //! @}

  //########################################################################################
  //! \name Files.
  //########################################################################################
  //! @{

  //! Create a file from the specified file base.
  virtual iFile* __stdcall CreateFile(iFileBase* apBase) = 0;
  //! Create a dummy file base.
  virtual iFileBase* __stdcall CreateFileBaseWriteDummy() = 0;
  //! Create a dummy file.
  virtual iFile* __stdcall CreateFileWriteDummy() = 0;
  //! Create a memory file base.
  virtual iFileBase* __stdcall CreateFileBaseMemory(tPtr apMem, tSize anSize, tBool abFree, const achar* aszPath = NULL) = 0;
  //! Create a memory file.
  virtual iFile* __stdcall CreateFileMemory(tPtr apMem, tSize anSize, tBool abFree, const achar* aszPath = NULL) = 0;
  //! Create a memory file base.
  virtual iFileBase* __stdcall CreateFileBaseMemoryAlloc(tSize anSize, const achar* aszPath = NULL) = 0;
  //! Create a memory file.
  virtual iFile* __stdcall CreateFileMemoryAlloc(tSize anSize, const achar* aszPath = NULL) = 0;
  //! Create a dynamic memory file base.
  virtual iFileBase* __stdcall CreateFileBaseDynamicMemory(tSize anSize = 0, const achar* aszPath = NULL) = 0;
  //! Create a dynamic memory file.
  virtual iFile* __stdcall CreateFileDynamicMemory(tSize anSize = 0, const achar* aszPath = NULL) = 0;
  //! Create a file base window.
  //! \remark If AutoSeekSet is true SeekSet will be called before each read/write operations
  //!         which allows to have multiple file windows on the same file and read from them
  //!         concurently. However that method is usually quite slow, a better approach is to
  //!         open a new shared file handle of the base file with and create the file window
  //!         with the offset and auto seek set disabled.
  virtual iFileBase* __stdcall CreateFileBaseWindow(iFileBase* apBase, tI64 anBase, tI64 anSize, const achar* aaszPath, tBool abAutoSeekSet) = 0;
  //! Create a file window.
  virtual iFile* __stdcall CreateFileWindow(iFileBase* apBase, tI64 anBase, tI64 anSize, const achar* aaszPath, tBool abAutoSeekSet) = 0;
  //! @}

  //########################################################################################
  //! \name File and buffer encoder/decoder
  //########################################################################################
  //! @{

  //! Create a file base buffer encoder.
  virtual iFileBase* __stdcall CreateFileBaseBufferEncoder(iFileBase* apBase, iBufferEncoder* apEnc) = 0;
  //! Create a file buffer encoder.
  virtual iFile* __stdcall CreateFileBufferEncoder(iFileBase* apBase, iBufferEncoder* apEnc) = 0;
  //! Create a file base buffer decoder.
  //! \param apBase
  //! \param apDec
  //! \param aDecodedSize specifies the size of the file when
  //!        decoded. Pass zero to autodetect. Should be specified
  //!        whenever possible, without it the whole file is decoded
  //!        to determin the its size.
  virtual iFileBase* __stdcall CreateFileBaseBufferDecoder(iFileBase* apBase, iBufferDecoder* apDec, tSize aDecodedSize) = 0;
  //! Create a file buffer decoder.
  virtual iFile* __stdcall CreateFileBufferDecoder(iFileBase* apBase, iBufferDecoder* apDec, tSize aDecodedSize) = 0;
  //! Encodes a file using the specified buffer encoder.
  virtual tSize __stdcall FileBufferEncode(iBufferEncoder* apEnc, iFile* apSrc, tSize anSrcSize, iFile* apDest) = 0;
  //! Decodes a file using the specified buffer decoder.
  virtual tSize __stdcall FileBufferDecode(iBufferDecoder* apDec, iFile* apSrc, tSize anSrcSize, iFile* apDest, tSize anDestSize) = 0;
  //! @}

  //########################################################################################
  //! \name OS Commands
  //########################################################################################
  //! @{

  //! Run the specified OS command.
  //! \remark This is equivalent to typing a command on the OS's command line/console.
  virtual tI32 __stdcall RunCommand(const achar* aaszCmd) = 0;
  //! Start the specified file/path.
  //! \remark This opens a file/path with the default application set to open the file/path's type.
  virtual tI32 __stdcall StartPath(const achar* aaszFile) = 0;
  //! @}

  //########################################################################################
  //! \name Time
  //########################################################################################
  //! @{

  //! Get the current time.
  //! {Property}
  virtual const iTime* __stdcall GetCurrentTime() const = 0;

  //! Get the current value of the lowest level timer available in seconds.
  virtual tF64 __stdcall TimerInSeconds() const = 0;

  //! Reset the frame time.
  virtual void __stdcall ResetFrameTime() = 0;
  //! Update the frame.
  virtual tBool __stdcall UpdateFrameTime(const tF64 afElapsedTime) = 0;
  //! Get the total time since the last ResetFrameTime.
  //! {Property}
  virtual tF64 __stdcall GetTotalFrameTime() const = 0;
  //! Get the time taken by the last frame.
  //! {Property}
  virtual tF64 __stdcall GetFrameTime() const = 0;
  //! Get the current frame number.
  //! {Property}
  virtual tU32 __stdcall GetFrameNumber() const = 0;
  //! Get the frame rate.
  //! {Property}
  virtual tF32 __stdcall GetFrameRate() const = 0;
  //! Get the average frame rate per seconds.
  //! {Property}
  //! \remark This value is estimated every second, it shouldn't be used for accurate measurment.
  virtual tU32 __stdcall GetAverageFrameRate() const = 0;
  //! @}

  //########################################################################################
  //! \name Json
  //########################################################################################
  //! @{

#if niMinFeatures(20)
  //! Parse Json contained in the specified file.
  virtual ni::tBool __stdcall JsonParseFile(ni::iFile* apFile, ni::iJsonParserSink* apSink) = 0;
  //! Parse Json contained in the specified string.
  virtual ni::tBool __stdcall JsonParseString(const ni::cString& aString, ni::iJsonParserSink* apSink) = 0;
  //! Create a Json writer to write to the specified sink.
  virtual ni::iJsonWriter* __stdcall CreateJsonSinkWriter(ni::iJsonWriterSink* apSink, ni::tBool abPrettyPrint) = 0;
  //! Create a Json writer to write to the specified file.
  virtual ni::iJsonWriter* __stdcall CreateJsonFileWriter(ni::iFile* apFile, ni::tBool abPrettyPrint) = 0;
#endif
  //! @}

  //########################################################################################
  //! \name Xml
  //########################################################################################
  //! @{

#if niMinFeatures(20)
  //! Parse Xml contained in the specified file.
  virtual ni::tBool __stdcall XmlParseFile(ni::iFile* apFile, ni::iXmlParserSink* apSink) = 0;
  //! Parse Xml contained in the specified string.
  virtual ni::tBool __stdcall XmlParseString(const ni::cString& aString, ni::iXmlParserSink* apSink) = 0;
#endif
  //! @}

  //########################################################################################
  //! \name Expressions
  //########################################################################################
  //! @{

  //! Get the root expression context.
  //! {Property}
  virtual iExpressionContext* __stdcall GetExpressionContext() const = 0;
  //! Create a new expression context.
  virtual iExpressionContext* __stdcall CreateExpressionContext() = 0;
  //! Evaluate the specified expression.
  //! \remark If the evaluated expression fails it returns an empty string
  //!         variable named "EVALERR". This means that Eval() always
  //!         returns a valid variable object and so their is no need to
  //!         check for a NULL pointer.
  virtual Ptr<iExpressionVariable> __stdcall Eval(const achar* aaszExpr) = 0;
  //! Convert an enum value to an expression string.
  virtual cString __stdcall EnumToString(tU32 anValue, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags) = 0;
  //! Convert an expression string to an enum value.
  virtual tU32 __stdcall StringToEnum(const achar* aExpr, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags) = 0;
  //! @}

  //########################################################################################
  //! \name Scripting hosts
  //########################################################################################
  //! @{

#if niMinFeatures(15)
  //! Add a scripting host.
  virtual tBool __stdcall AddScriptingHost(iHString* ahspName, iScriptingHost* apHost) = 0;
  //! Remove a scripting host.
  virtual tBool __stdcall RemoveScriptingHost(iHString* ahspName) = 0;
  //! Get the number of scripting hosts.
  //! {Property}
  virtual tU32 __stdcall GetNumScriptingHosts() const = 0;
  //! Get the name of the scripting host at the specified index.
  //! {Property}
  virtual iHString* __stdcall GetScriptingHostName(tU32 anIndex) const = 0;
  //! Get the scripting host at the specified index.
  //! {Property}
  virtual iScriptingHost* __stdcall GetScriptingHost(tU32 anIndex) = 0;
  //! Get the scripting host from the specified name.
  //! {Property}
  virtual iScriptingHost* __stdcall GetScriptingHostFromName(iHString* ahspName) = 0;
  //! Get the index of the specified scripting host.
  //! {Property}
  virtual tU32 __stdcall GetScriptingHostIndex(iScriptingHost* apHost) const = 0;

  //! Service all scripting hosts.
  virtual void __stdcall ServiceAllScriptingHosts(tBool abForceGC) = 0;

  //! Get the first scripting host that can evaluate the code resources.
  virtual iScriptingHost* __stdcall FindScriptingHost(iHString* ahspContext, iHString* ahspCodeResource) = 0;
#endif
  //! @}

  //########################################################################################
  //! \name Profiler
  //########################################################################################
  //! @{

  //! Get the profiler instance.
  //! {Property}
  virtual iProf* __stdcall GetProf() const = 0;
  //! @}

  //########################################################################################
  //! \name URL File
  //########################################################################################
  //! @{

  //! Get the protocol that will be used to access the specified URL.
  //! \remark If no protocol is specified returns the "file" protocol if an
  //!         absolute file path is detected (the path starts with a drive
  //!         letter or / or \) else return the "default" protocol.
  virtual cString __stdcall URLGetProtocol(const achar* aURL) = 0;
  //! Get the URL file handler that will be used to access the specified URL.
  //! \remark If no protocol is specified the "default" protocol is assumed.
  virtual iURLFileHandler* __stdcall URLGetHandler(const achar* aURL) = 0;
  //! Open a synchronous stream to the specified URL.
  //! \remark The operation is blocking, if necessary the whole file is
  //!         downloaded before the function returns.
  //! \remark The operation is handed to a URLFileHandler which is retrived according to
  //!         the protocol specified in the URL. For example for http://foo.com/stuff.jpg
  //!         the GlobalInstance "URLFileHandler.http" is retrieved as file handler.
  //! \remark The RootFS is set as the default and file protocol handlers.
  virtual iFile* __stdcall URLOpen(const achar* aURL) = 0;
  //! Validates the specified URL and if possible checks whether the resource
  //! the URL points to actually exists.
  //! \remark \see ni::iLang::URLOpen for precision about URL handling.
  virtual tBool __stdcall URLExists(const achar* aURL) = 0;
  //! Find the path/url matching the specified resource, basepath and with the extensions derived from the specified loader prefix.
  //! \remark Example: found = URLFindFilePath("foo.bar", "data/", "BitmapLoader.")
  virtual cString __stdcall URLFindFilePath(const achar* aszRes, const achar* aszBasePath, const achar* aLoaderPrefix) = 0;
  //! @}

  //########################################################################################
  //! \name HString & Localization
  //########################################################################################
  //! @{

  //! Create a new HString.
  virtual Ptr<iHString> __stdcall CreateHString(const cString& aStr) = 0;
  //! Load a localization table.
  //! \remark The table's format is \<Localization>\<String native="Hello" fr="Bonjour" cn="nihao" />\</Localization>
  virtual tBool __stdcall LoadLocalization(iDataTable* apDT) = 0;
  //! @}

  //########################################################################################
  //! \name Serialization.
  //########################################################################################
  //! @{

  //! Check if the file contains an object of the specified type.
  virtual tBool __stdcall IsSerializedObject(iFile* apFile, const achar* aID, tI64* apObjSize) = 0;
  //! Serialize an object.
  virtual iUnknown* __stdcall SerializeObject(iFile* apFile, iUnknown* apObject, eSerializeMode aMode, tI64* apObjSize) = 0;
  //! Read serialization header.
  //! {NoAutomation}
  virtual tI64 __stdcall ReadSerializationHeader(iFile* apFile, cString* apID, tI64* apSize) = 0;
  //! Write serialization header.
  //! {NoAutomation}
  virtual tI64 __stdcall WriteSerializationHeader(iFile* apFile, const achar* apID, tI64 anSize) = 0;
  //! @}

  //########################################################################################
  //! \name Device resource manager
  //########################################################################################
  //! @{

  //! Create a new device resource manager.
  virtual iDeviceResourceManager* __stdcall CreateDeviceResourceManager(const achar* aszType) = 0;
  //! @}

  //########################################################################################
  //! \name Messages
  //########################################################################################
  //! @{

  //! Get the system message targets.
  //! {Property}
  virtual tMessageHandlerSinkLst* __stdcall GetSystemMessageHandlers() const = 0;
  //! @}

  //########################################################################################
  //! \name Data tables
  //########################################################################################
  //! @{

  //! Create a data table.
  virtual iDataTable* __stdcall CreateDataTable(const achar* aaszName) = 0;
  //! Serialize the specified data table.
  virtual tBool __stdcall SerializeDataTable(const achar* aaszType, eSerializeMode aMode, iDataTable* apTable, iFile* apFile) = 0;
  //! Create a data table write stack.
  virtual iDataTableWriteStack* __stdcall CreateDataTableWriteStack(iDataTable* apDT) = 0;
  //! Create a data table write stack.
  virtual iDataTableWriteStack* __stdcall CreateDataTableWriteStackFromName(const achar* aaszName) = 0;
  //! Create a data table read stack.
  virtual iDataTableReadStack* __stdcall CreateDataTableReadStack(iDataTable* apDT) = 0;
  //! Build an absolute path to the specified datatable and property.
  //! \remark If anPropIndex == eInvalidHandle the path points to the datatable and not the property
  virtual cString __stdcall GetAbsoluteDataTablePath(iDataTable* apDT, tU32 anPropIndex) = 0;
  //! @}

  //########################################################################################
  //! \name Clipboard
  //########################################################################################
  //! @{

  //! Set the data of the specified clipboard.
  //! {Property}
  //! \remark The datatable should have a at least a type member, type can be :<br>
  //!     <ul>
  //!     <li>text : Has a 'text' property of type string containing the text</li>
  //!     <li>bitmap : Has a 'bitmap' property of type iUnknown containing the bitmap</li>
  //!     </ul>
  virtual tBool __stdcall SetClipboard(eClipboardType aType, iDataTable* apDT) = 0;
  //! Get the data of the specified clipboard.
  //! {Property}
  virtual iDataTable* __stdcall GetClipboard(eClipboardType aType) const = 0;
  //! @}

  //########################################################################################
  //! \name OS Windows
  //########################################################################################
  //! @{

  //! Get the number of monitors connected to the comptuer.
  //! {Property}
  virtual tU32 __stdcall GetNumMonitors() const = 0;
  //! Get the index of the monitor with the specified OS handle.
  //! {Property}
  virtual tU32 __stdcall GetMonitorIndex(tIntPtr aHandle) const = 0;
  //! Get the OS handle to the monitor at the specified index.
  //! {Property}
  virtual tIntPtr __stdcall GetMonitorHandle(tU32 anIndex) const = 0;
  //! Get the name of the monitor at the specified index.
  //! {Property}
  virtual const achar* __stdcall GetMonitorName(tU32 anIndex) const = 0;
  //! Get the rectangle of the monitor at the specified index.
  //! {Property}
  virtual sRecti __stdcall GetMonitorRect(tU32 anIndex) const = 0;
  //! Get the flags of the monitor at the specified index.
  //! {Property}
  virtual tOSMonitorFlags __stdcall GetMonitorFlags(tU32 anIndex) const = 0;

  //! Create a new OS window.
  virtual iOSWindow* __stdcall CreateWindow(iOSWindow* apParent, const achar* aaszTitle, const sRecti& aRect, tOSWindowCreateFlags aCreate, tOSWindowStyleFlags aStyle) = 0;
  //! Create a an OS window from a generic OS window handle.
  virtual iOSWindow* __stdcall CreateWindowEx(tIntPtr aOSWindowHandle, tOSWindowCreateFlags aCreate) = 0;

  //! Displays a native modal message box.
  virtual eOSMessageBoxReturn __stdcall MessageBox(iOSWindow* apParent, const achar* aaszTitle, const achar* aaszText, tOSMessageBoxFlags aFlags) = 0;
  //! Displays a native file open dialog box.
  //! \param aParent is the parent window.
  //! \param aTitle title of the dialog. "Open" by default.
  //! \param aFilter the file extension filters. Example "pdf;txt;*", '*' means that all extensions are accepted.
  //! \param aInitDir the directory to start from.
  //! \return An empty string if the user canceled, else the file path selected.
  virtual cString __stdcall OpenFileDialog(iOSWindow* aParent, const achar* aTitle, const achar* aFilter, const achar* aInitDir) = 0;
  //! Displays a native file save dialog box.
  //! \param aParent is the parent window.
  //! \param aTitle title of the dialog. "Open" by default.
  //! \param aFilter the file extension filters. Example "pdf;txt;*", '*' means that all extensions are accepted.
  //! \param aInitDir the directory to start from.
  //! \return An empty string if the user canceled, else the file path selected.
  virtual cString __stdcall SaveFileDialog(iOSWindow* aParent, const achar* aTitle, const achar* aFilter, const achar* aInitDir) = 0;
  //! Displays a native directory picker dialog box.
  //! \param aParent is the parent window.
  //! \param aTitle title of the dialog. "Open" by default.
  //! \param aInitDir the directory to start from.
  //! \return An empty string if the user canceled, else the file path selected.
  virtual cString __stdcall PickDirectoryDialog(iOSWindow* aParent, const achar* aTitle, const achar* aInitDir) = 0;
  //! @}

  //########################################################################################
  //! \name OS Input
  //########################################################################################
  //! @{

  //! Get the number of game controllers available on the system.
  //! {Property}
  virtual tU32 __stdcall GetNumGameCtrls() const = 0;
  //! Get a game controller input device interface.
  //! \param  aulIdx is the index of the game controller that will be managed by the
  //!     created interface.
  //! \return NULL if the interface can't be get, else a pointer to a valid
  //!     iGameCtrl interface.
  //! {Property}
  virtual iGameCtrl* __stdcall GetGameCtrl(tU32 aulIdx) const = 0;
  //! @}
};

//! Get the lang instance
niExportFunc(ni::iLang*) GetLang();

//! Convert the specified enum to a its value string
#define niEnumToChars(ENUM,VALUE)                                       \
  ni::GetLang()->EnumToString((ENUM)VALUE,GetEnumDef_##ENUM(),0).Chars()

//! Convert the specified enum to its expanded value string (ENUM.VALUESTRING)
#define niFullEnumToChars(ENUM,VALUE)                                   \
  ni::GetLang()->EnumToString((ENUM)VALUE,GetEnumDef_##ENUM(),ni::eEnumToStringFlags_Full).Chars()

//! Convert the specified flags to its value string (FLAG1|...|FLAGN)
#define niFlagsToChars(ENUM,VALUE)                                      \
  ni::GetLang()->EnumToString((ENUM)VALUE,GetEnumDef_##ENUM(),ni::eEnumToStringFlags_Flags).Chars()

//! Convert the specified flags to its expanded value string (FLAGS.FLAG1|...|FLAGS.FLAGN)
#define niFullFlagsToChars(ENUM,VALUE)                                  \
  ni::GetLang()->EnumToString((ENUM)VALUE,GetEnumDef_##ENUM(),ni::eEnumToStringFlags_Flags|ni::eEnumToStringFlags_Full).Chars()

#define niDefConstHString_(VARNAME,STRING)                \
  __forceinline ni::iHString* GetHString_##VARNAME() {    \
    static ni::tHStringPtr _hstr_##VARNAME = _H(STRING);  \
    return _hstr_##VARNAME;                               \
  }

#define niDefConstHString(STR)  niDefConstHString_(STR,#STR)
#define niGetConstHString(STR)  GetHString_##STR()

#ifdef niNoUnsafePtr
#define _H(STR)           ni::GetLang()->CreateHString(STR).non_null()
#else
#define _H(STR)           ni::GetLang()->CreateHString(STR)
#endif
#define _HC(STR)          niGetConstHString(STR)
#define _HDecl_(NAME,STR) niDefConstHString_(NAME,STR)
#define _HDecl(STR)       niDefConstHString(STR)

//! Create an object instance using function linkage
#define niCreateInstance(MODULE,OBJECT,A,B) New_##MODULE##_##OBJECT(A,B)

#define niRegisterModuleDef(NAME)                         \
  const ni::iModuleDef* __stdcall GetModuleDef_##NAME();  \
  ni::GetLang()->RegisterModule(GetModuleDef_##NAME());

niExportFunc(ni::iUnknown*) New_niLang_Lang(const ni::Var&,const ni::Var&);

///////////////////////////////////////////////
static inline tBool __stdcall HasProperty(const achar* aaszName) {
  return GetLang()->HasProperty(aaszName);
}
static inline void __stdcall SetProperty(const achar* aaszName, const achar* aaszValue) {
  GetLang()->SetProperty(aaszName,aaszValue);
}
static inline cString __stdcall GetProperty(const achar* aaszName, const achar* aaszDefault = AZEROSTR) {
  if (!HasProperty(aaszName))
    return aaszDefault;
  return GetLang()->GetProperty(aaszName);
}

///////////////////////////////////////////////
static inline cString GetToolkitBinDir(const achar* aToolkitName, const achar* aSubDir) {
  cString r = ni::GetRootFS()->GetAbsolutePath(
    (ni::GetLang()->GetProperty("ni.dirs.bin") +
     "../../../" + aToolkitName + "/").Chars());
  if (niStringIsOK(aSubDir)) {
    r += aSubDir;
    r += "/";
  }
  return r;
}

///////////////////////////////////////////////
static inline cString GetToolkitDir(const achar* aToolkitName, const achar* aSubDir) {
  cString r = ni::GetRootFS()->GetAbsolutePath(
    (ni::GetLang()->GetProperty("ni.dirs.data") +
     "../../" + aToolkitName + "/").Chars());
  if (niStringIsOK(aSubDir)) {
    r += aSubDir;
    r += "/";
  }
  return r;
}

///////////////////////////////////////////////
static inline cString GetModuleDataDir(const achar* aToolkitName, const achar* aModuleName) {
  return GetToolkitDir(aToolkitName,"data") + aModuleName + "/";
}

///////////////////////////////////////////////
inline tBool HasCreateInstance(const achar* aaszName) {
  return ni::GetLang()->GetCreateInstanceMap()->find(aaszName) !=
      ni::GetLang()->GetCreateInstanceMap()->end();
}

///////////////////////////////////////////////
inline tBool RegisterCreateInstance(const achar* aaszName, iCallback* apCallback) {
  niAssert(niStringIsOK(aaszName));
  niAssert(niIsOK(apCallback));
  astl::upsert(
      *ni::GetLang()->GetCreateInstanceMap(),
      aaszName,
      apCallback);
  return eTrue;
}

///////////////////////////////////////////////
inline tBool UnregisterCreateInstance(const achar* aaszName, iCallback* apCallback) {
  Nonnull<tCreateInstanceCMap> mapCI(ni::GetLang()->GetCreateInstanceMap());
  tCreateInstanceCMap::iterator it = mapCI->find(aaszName);
  if (it == mapCI->end())
    return eFalse;
  if (apCallback && it->second.ptr() != apCallback)
    return eFalse;
  mapCI->erase(it);
  return eTrue;
}

///////////////////////////////////////////////
struct sPropertyString {
  sPropertyString(const achar* const aName, const achar* const aDefault)
      : _name(aName)
      , _default(aDefault)
      , _fetched(eFalse)
  {}

  const cString& get(tBool abForceGet = eFalse) {
    if (abForceGet || !_fetched) {
      _fetched = eTrue;
      if (ni::GetLang()->HasProperty(_name)) {
        _value = ni::GetLang()->GetProperty(_name);
      }
      else {
        _value = _default;
      }
    }
    return _value;
  }

private:
  const achar* const _name;
  const achar* const _default;
  tBool _fetched;
  cString _value;
};

///////////////////////////////////////////////
struct sPropertyBool {
  sPropertyBool(const achar* const aName, const tBool aDefault)
      : _name(aName)
      , _default(aDefault)
      , _fetched(eFalse)
  {}

  const tBool get(tBool abForceGet = eFalse) {
    if (abForceGet || !_fetched) {
      _fetched = eTrue;
      if (ni::GetLang()->HasProperty(_name)) {
        _value = ni::GetLang()->GetProperty(_name).Bool(_default);
      }
      else {
        _value = _default;
      }
    }
    return (tBool)_value;
  }

private:
  const achar* const _name;
  const tBool _default;
  tBool _fetched;
  tI64 _value;
};

///////////////////////////////////////////////
inline cString URLExistsComputeShortestRelativeFilePath(const achar* aszFileName) {
  cString strFilename;
  if (niStringIsOK(aszFileName)) {
    if (ni::GetLang()->URLExists(aszFileName)) {
      strFilename = aszFileName;
      {
        char* p = strFilename.data();
        while (*p) {
          if (*p == '\\')
            *p = '/';
          ++p;
        }
      }
      if (strFilename[0] == '/' || strFilename[1] == ':') {
        cString search = strFilename;
        // absolute path, find the shortest relative relative path
        while (search.contains("/")) {
          search = search.After("/");
          if (ni::GetLang()->URLExists(search.Chars())) {
            strFilename = search;
          }
        }
      }
    }
  }
  return strFilename;
}

///////////////////////////////////////////////
inline Var __stdcall ExpressionVariableToVar(iExpressionVariable* apVar) {
  if (!apVar)
    return niVarNull;

  switch (apVar->GetType()) {
    case  eExpressionVariableType_Float: {
      return apVar->GetFloat();
    }
    case  eExpressionVariableType_Vec2: {
      return apVar->GetVec2();
    }
    case  eExpressionVariableType_Vec3: {
      return apVar->GetVec3();
    }
    case  eExpressionVariableType_Vec4: {
      return apVar->GetVec4();
    }
    case  eExpressionVariableType_Matrix: {
      return apVar->GetMatrix();
    }
    case  eExpressionVariableType_String: {
      return apVar->GetString();
    }
    default: {
      return niVarNull;
    }
  }
}

///////////////////////////////////////////////
inline Var __stdcall EvalVar(iExpressionContext* apContext, const achar* aExpr) {
  niAssert(apContext != NULL);

  if (!niStringIsOK(aExpr))
    return niVarNull;

  if (aExpr[0] == '`') {
    return (aExpr+1);
  }

  Ptr<iExpressionVariable> ev = apContext->Eval(aExpr);
  if (ev.is_null())
    return niVarNull;
  else
    return ExpressionVariableToVar(ev.non_null());
}

///////////////////////////////////////////////
inline iFile* CreateBin2HFile(const unsigned char* data, const int size, const char* name) {
  cString fileName = name;
  int dotPos = fileName.rfind("_");
  if (dotPos > 0) {
    fileName.data()[dotPos] = '.';
  }
  return ni::GetLang()->CreateFileMemory((tPtr)data, size, eFalse, fileName.Chars());
}

#define niExternBin2H(NAME)                     \
  extern unsigned char NAME##_DATA[];           \
  extern int NAME##_DATA_SIZE;
#define niFileOpenBin2H(NAME) CreateBin2HFile(NAME##_DATA, NAME##_DATA_SIZE, "BIN2H_" #NAME)

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __ILANG_H_1CD42183_7376_49A5_BEE6_0141CD400B64__
