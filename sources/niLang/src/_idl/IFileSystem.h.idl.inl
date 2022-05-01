#include "../API/niLang/IFileSystem.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iFileSystemEnumerator **/
IDLC_BEGIN_INTF(ni,iFileSystemEnumerator)
/** ni -> iFileSystemEnumerator::GetFileSystem/0 **/
IDLC_METH_BEGIN(ni,iFileSystemEnumerator,GetFileSystem,0)
	IDLC_DECL_RETVAR(iFileSystem*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileSystemEnumerator,GetFileSystem,0,())
	IDLC_RET_FROM_INTF(iFileSystem,_Ret)
IDLC_METH_END(ni,iFileSystemEnumerator,GetFileSystem,0)

/** ni -> iFileSystemEnumerator::FindFirst/1 **/
IDLC_METH_BEGIN(ni,iFileSystemEnumerator,FindFirst,1)
	IDLC_DECL_VAR(achar*,aaszFilePattern)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszFilePattern)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileSystemEnumerator,FindFirst,1,(aaszFilePattern))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFileSystemEnumerator,FindFirst,1)

/** ni -> iFileSystemEnumerator::FindNext/0 **/
IDLC_METH_BEGIN(ni,iFileSystemEnumerator,FindNext,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileSystemEnumerator,FindNext,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFileSystemEnumerator,FindNext,0)

/** ni -> iFileSystemEnumerator::GetFileName/0 **/
IDLC_METH_BEGIN(ni,iFileSystemEnumerator,GetFileName,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileSystemEnumerator,GetFileName,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iFileSystemEnumerator,GetFileName,0)

/** ni -> iFileSystemEnumerator::GetFileSize/0 **/
IDLC_METH_BEGIN(ni,iFileSystemEnumerator,GetFileSize,0)
	IDLC_DECL_RETVAR(tI64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileSystemEnumerator,GetFileSize,0,())
	IDLC_RET_FROM_BASE(ni::eType_I64,_Ret)
IDLC_METH_END(ni,iFileSystemEnumerator,GetFileSize,0)

/** ni -> iFileSystemEnumerator::GetFileAttributes/0 **/
IDLC_METH_BEGIN(ni,iFileSystemEnumerator,GetFileAttributes,0)
	IDLC_DECL_RETVAR(tFileAttrFlags,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileSystemEnumerator,GetFileAttributes,0,())
	IDLC_RET_FROM_ENUM(tFileAttrFlags,_Ret)
IDLC_METH_END(ni,iFileSystemEnumerator,GetFileAttributes,0)

/** ni -> iFileSystemEnumerator::GetFileTime/0 **/
IDLC_METH_BEGIN(ni,iFileSystemEnumerator,GetFileTime,0)
	IDLC_DECL_RETVAR(iTime*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileSystemEnumerator,GetFileTime,0,())
	IDLC_RET_FROM_INTF(iTime,_Ret)
IDLC_METH_END(ni,iFileSystemEnumerator,GetFileTime,0)

IDLC_END_INTF(ni,iFileSystemEnumerator)

/** interface : iFileSystem **/
IDLC_BEGIN_INTF(ni,iFileSystem)
/** ni -> iFileSystem::GetRightsFlags/0 **/
IDLC_METH_BEGIN(ni,iFileSystem,GetRightsFlags,0)
	IDLC_DECL_RETVAR(tFileSystemRightsFlags,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileSystem,GetRightsFlags,0,())
	IDLC_RET_FROM_ENUM(tFileSystemRightsFlags,_Ret)
IDLC_METH_END(ni,iFileSystem,GetRightsFlags,0)

/** ni -> iFileSystem::GetBaseContainer/0 **/
IDLC_METH_BEGIN(ni,iFileSystem,GetBaseContainer,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileSystem,GetBaseContainer,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iFileSystem,GetBaseContainer,0)

/** ni -> iFileSystem::FileMakeDir/1 **/
IDLC_METH_BEGIN(ni,iFileSystem,FileMakeDir,1)
	IDLC_DECL_VAR(achar*,aszDir)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aszDir)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileSystem,FileMakeDir,1,(aszDir))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFileSystem,FileMakeDir,1)

/** ni -> iFileSystem::FileDeleteDir/1 **/
IDLC_METH_BEGIN(ni,iFileSystem,FileDeleteDir,1)
	IDLC_DECL_VAR(achar*,aszDir)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aszDir)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileSystem,FileDeleteDir,1,(aszDir))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFileSystem,FileDeleteDir,1)

/** ni -> iFileSystem::FileCopy/2 **/
IDLC_METH_BEGIN(ni,iFileSystem,FileCopy,2)
	IDLC_DECL_VAR(achar*,aszDest)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aszDest)
	IDLC_DECL_VAR(achar*,aszSrc)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aszSrc)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileSystem,FileCopy,2,(aszDest,aszSrc))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFileSystem,FileCopy,2)

/** ni -> iFileSystem::FileMove/2 **/
IDLC_METH_BEGIN(ni,iFileSystem,FileMove,2)
	IDLC_DECL_VAR(achar*,aszDest)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aszDest)
	IDLC_DECL_VAR(achar*,aszSrc)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aszSrc)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileSystem,FileMove,2,(aszDest,aszSrc))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFileSystem,FileMove,2)

/** ni -> iFileSystem::FileDelete/1 **/
IDLC_METH_BEGIN(ni,iFileSystem,FileDelete,1)
	IDLC_DECL_VAR(achar*,aszFile)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aszFile)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileSystem,FileDelete,1,(aszFile))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFileSystem,FileDelete,1)

/** ni -> iFileSystem::FileEnum/3 **/
IDLC_METH_BEGIN(ni,iFileSystem,FileEnum,3)
	IDLC_DECL_VAR(achar*,aszFile)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aszFile)
	IDLC_DECL_VAR(tU32,aAttribs)
	IDLC_BUF_TO_BASE(ni::eType_U32,aAttribs)
	IDLC_DECL_VAR(iFileEnumSink*,pSink)
	IDLC_BUF_TO_INTF(iFileEnumSink,pSink)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileSystem,FileEnum,3,(aszFile,aAttribs,pSink))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iFileSystem,FileEnum,3)

/** ni -> iFileSystem::FileExists/2 **/
IDLC_METH_BEGIN(ni,iFileSystem,FileExists,2)
	IDLC_DECL_VAR(achar*,aszFile)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aszFile)
	IDLC_DECL_VAR(tU32,aAttribs)
	IDLC_BUF_TO_BASE(ni::eType_U32,aAttribs)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileSystem,FileExists,2,(aszFile,aAttribs))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iFileSystem,FileExists,2)

/** ni -> iFileSystem::FileSize/1 **/
IDLC_METH_BEGIN(ni,iFileSystem,FileSize,1)
	IDLC_DECL_VAR(achar*,aszFile)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aszFile)
	IDLC_DECL_RETVAR(tI64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileSystem,FileSize,1,(aszFile))
	IDLC_RET_FROM_BASE(ni::eType_I64,_Ret)
IDLC_METH_END(ni,iFileSystem,FileSize,1)

/** ni -> iFileSystem::FileOpen/2 **/
IDLC_METH_BEGIN(ni,iFileSystem,FileOpen,2)
	IDLC_DECL_VAR(achar*,aszFile)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aszFile)
	IDLC_DECL_VAR(eFileOpenMode,aMode)
	IDLC_BUF_TO_ENUM(eFileOpenMode,aMode)
	IDLC_DECL_RETVAR(iFile*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileSystem,FileOpen,2,(aszFile,aMode))
	IDLC_RET_FROM_INTF(iFile,_Ret)
IDLC_METH_END(ni,iFileSystem,FileOpen,2)

/** ni -> iFileSystem::FileBaseOpen/2 **/
IDLC_METH_BEGIN(ni,iFileSystem,FileBaseOpen,2)
	IDLC_DECL_VAR(achar*,aszFile)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aszFile)
	IDLC_DECL_VAR(eFileOpenMode,aMode)
	IDLC_BUF_TO_ENUM(eFileOpenMode,aMode)
	IDLC_DECL_RETVAR(iFileBase*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileSystem,FileBaseOpen,2,(aszFile,aMode))
	IDLC_RET_FROM_INTF(iFileBase,_Ret)
IDLC_METH_END(ni,iFileSystem,FileBaseOpen,2)

/** ni -> iFileSystem::GetAbsolutePath/1 **/
IDLC_METH_BEGIN(ni,iFileSystem,GetAbsolutePath,1)
	IDLC_DECL_VAR(achar*,aaszFile)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszFile)
	IDLC_DECL_RETVAR(cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileSystem,GetAbsolutePath,1,(aaszFile))
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iFileSystem,GetAbsolutePath,1)

/** ni -> iFileSystem::CreateEnumerator/0 **/
IDLC_METH_BEGIN(ni,iFileSystem,CreateEnumerator,0)
	IDLC_DECL_RETVAR(iFileSystemEnumerator*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileSystem,CreateEnumerator,0,())
	IDLC_RET_FROM_INTF(iFileSystemEnumerator,_Ret)
IDLC_METH_END(ni,iFileSystem,CreateEnumerator,0)

IDLC_END_INTF(ni,iFileSystem)

IDLC_END_NAMESPACE()
// EOF //
