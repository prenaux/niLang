// clang-format off
#ifndef __MODULEDEF_niLang_H__
#define __MODULEDEF_niLang_H__
/*
 * Autogenerated module definition for ni::niLang.
 */

#include <niLang/ObjModel.h>

namespace ni {
/** \addtogroup niLang
 * @{
 */

static const char *const kVersion_niLang = "1,0,0";
niExportFunc(const ni::iModuleDef*) GetModuleDef_niLang();
niExportFunc(ni::iUnknown*) New_niLang_Lang(const ni::Var&, const ni::Var&);
niExportFunc(ni::iUnknown*) New_niLang_CryptoRand(const ni::Var&, const ni::Var&);
niExportFunc(ni::iUnknown*) New_niLang_CryptoHash(const ni::Var&, const ni::Var&);
niExportFunc(ni::iUnknown*) New_niLang_Crypto(const ni::Var&, const ni::Var&);
niExportFunc(ni::iUnknown*) New_niLang_Math(const ni::Var&, const ni::Var&);
niExportFunc(ni::iUnknown*) New_niLang_Concurrent(const ni::Var&, const ni::Var&);
niExportFunc(ni::iUnknown*) New_niLang_FilePatternRegex(const ni::Var&, const ni::Var&);
niExportFunc(ni::iUnknown*) New_niLang_PCRE(const ni::Var&, const ni::Var&);
niExportFunc(ni::iUnknown*) New_niLang_DataTable(const ni::Var&, const ni::Var&);
niExportFunc(ni::iUnknown*) New_niLang_Console(const ni::Var&, const ni::Var&);
niExportFunc(ni::iUnknown*) New_niLang_OSWindowGeneric(const ni::Var&, const ni::Var&);
niExportFunc(ni::iUnknown*) New_niLang_Zip(const ni::Var&, const ni::Var&);
niExportFunc(ni::iUnknown*) New_niLang_URLFileHandlerManifestFileSystem(const ni::Var&, const ni::Var&);
niExportFunc(ni::iUnknown*) New_niLang_URLFileHandlerURL(const ni::Var&, const ni::Var&);
niExportFunc(ni::iUnknown*) New_niLang_URLFileHandlerFileSystem(const ni::Var&, const ni::Var&);
niExportFunc(ni::iUnknown*) New_niLang_URLFileHandlerZip(const ni::Var&, const ni::Var&);
niExportFunc(ni::iUnknown*) New_niLang_URLFileHandlerPrefixed(const ni::Var&, const ni::Var&);

niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iIterator();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iCollection();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iMutableCollection();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iRunnable();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iCallback();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iRunnableQueue();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iMessageHandler();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iMessageDesc();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iMessageQueue();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iFuture();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iFutureValue();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iExecutor();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iConcurrent();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iCommandSink();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iConsoleSink();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iConsole();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iCryptoRand();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iCrypto();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iCryptoHash();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iDataTableSink();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iDataTable();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iDataTableReadStack();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iDataTableWriteStack();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iDeviceResource();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iDeviceResourceManager();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iExpressionVariable();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iExpression();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iExpressionURLResolver();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iExpressionContext();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iFileEnumSink();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iFileBase();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iFile();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iFileMemory();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iURLFileHandler();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iBufferEncoder();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iBufferDecoder();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iFileSystemEnumerator();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iFileSystem();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iGameCtrl();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iHString();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iHStringCharIt();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iJsonParserSink();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iJsonWriterSink();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iJsonWriter();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iLang();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iMath();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iOSProcess();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iOSProcessEnumSink();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iOSProcessManager();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iOSGraphicsAPI();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iOSWindow();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iOSWindowGeneric();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iProfDraw();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iProf();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iRegex();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iPCRE();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iScriptingHost();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iSerializable();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iSinkList();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iRemoteAddress();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iRemoteAddressIPv4();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iSocket();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iStringTokenizer();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iTime();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iToString();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iXmlParserSink();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iZipArchWrite();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iZip();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iObjectTypeDef();
niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_iModuleDef();

niExportFunc(const ni::sEnumDef*) GetEnumDef_Unnamed();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eDataTablePropertyType();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eDataTableCopyFlags();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eExpressionVariableType();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eExpressionVariableFlags();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eTextEncodingFormat();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eFileFlags();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eFileSystemRightsFlags();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eGameCtrlAxis();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eGameCtrlButton();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eJsonType();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eEnumToStringFlags();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eSystemMessage();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eClipboardType();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eClassify();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eOSProcessSpawnFlags();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eOSProcessFile();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eKey();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eKeyMod();
niExportFunc(const ni::sEnumDef*) GetEnumDef_ePointerButton();
niExportFunc(const ni::sEnumDef*) GetEnumDef_ePointerAxis();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eGestureState();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eOSWindowMessage();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eOSWindowStyleFlags();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eOSWindowCreateFlags();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eOSWindowShowFlags();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eOSWindowZOrder();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eOSCursor();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eOSMonitorFlags();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eOSMessageBoxFlags();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eOSMessageBoxReturn();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eOSWindowSwitchReason();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eProfilerReportMode();
niExportFunc(const ni::sEnumDef*) GetEnumDef_ePCREOptionsFlags();
niExportFunc(const ni::sEnumDef*) GetEnumDef_ePCREError();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eSerializeFlags();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eSerializeMode();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eRemoteAddressType();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eRemoteAddressIPv4Reserved();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eSocketProtocol();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eSocketWaitFlags();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eSocketErrno();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eStringTokenizerCharType();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eTimeZone();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eRawToStringEncoding();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eXmlParserNodeType();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eStrFindFlags();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eUCPCategory();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eUCPCharType();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eUCPScript();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eTypeFlags();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eType();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eLogFlags();
niExportFunc(const ni::sEnumDef*) GetEnumDef_ePlaneType();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eQuatSlerp();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eRectCorners();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eRectEdges();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eRectFrameFlags();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eListInterfacesFlags();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eFileAttrFlags();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eFileTime();
niExportFunc(const ni::sEnumDef*) GetEnumDef_eFileOpenMode();


/**@}*/
} // end of namespace ni

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif /// __MODULEDEF_niLang_H__
