#include "../API/niLang/IFile.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iFileEnumSink **/
IDLC_BEGIN_INTF(ni,iFileEnumSink)
/** ni -> iFileEnumSink::OnFound/3 **/
IDLC_METH_BEGIN(ni,iFileEnumSink,OnFound,3)
	IDLC_DECL_VAR(achar*,aszFile)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aszFile)
	IDLC_DECL_VAR(tU32,aFileAttrs)
	IDLC_BUF_TO_BASE(ni::eType_U32,aFileAttrs)
	IDLC_DECL_VAR(tI64,anFileSize)
	IDLC_BUF_TO_BASE(ni::eType_I64,anFileSize)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileEnumSink,OnFound,3,(aszFile,aFileAttrs,anFileSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFileEnumSink,OnFound,3)

IDLC_END_INTF(ni,iFileEnumSink)

/** interface : iFileBase **/
IDLC_BEGIN_INTF(ni,iFileBase)
/** ni -> iFileBase::GetFileFlags/0 **/
IDLC_METH_BEGIN(ni,iFileBase,GetFileFlags,0)
	IDLC_DECL_RETVAR(tFileFlags,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileBase,GetFileFlags,0,())
	IDLC_RET_FROM_ENUM(tFileFlags,_Ret)
IDLC_METH_END(ni,iFileBase,GetFileFlags,0)

/** ni -> iFileBase::GetSourcePath/0 **/
IDLC_METH_BEGIN(ni,iFileBase,GetSourcePath,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileBase,GetSourcePath,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iFileBase,GetSourcePath,0)

/** ni -> iFileBase::Seek/1 **/
IDLC_METH_BEGIN(ni,iFileBase,Seek,1)
	IDLC_DECL_VAR(tI64,offset)
	IDLC_BUF_TO_BASE(ni::eType_I64,offset)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileBase,Seek,1,(offset))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFileBase,Seek,1)

/** ni -> iFileBase::SeekSet/1 **/
IDLC_METH_BEGIN(ni,iFileBase,SeekSet,1)
	IDLC_DECL_VAR(tI64,offset)
	IDLC_BUF_TO_BASE(ni::eType_I64,offset)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileBase,SeekSet,1,(offset))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFileBase,SeekSet,1)

/** ni -> iFileBase::ReadRaw/2 **/
IDLC_METH_BEGIN(ni,iFileBase,ReadRaw,2)
	IDLC_DECL_VAR(void*,apOut)
	IDLC_BUF_TO_PTR(ni::eType_Ptr,apOut)
	IDLC_DECL_VAR(tSize,anSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,anSize)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileBase,ReadRaw,2,(apOut,anSize))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iFileBase,ReadRaw,2)

/** ni -> iFileBase::WriteRaw/2 **/
IDLC_METH_BEGIN(ni,iFileBase,WriteRaw,2)
	IDLC_DECL_VAR(void*,apIn)
	IDLC_BUF_TO_PTR(ni::eTypeFlags_Constant|ni::eType_Ptr,apIn)
	IDLC_DECL_VAR(tSize,anSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,anSize)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileBase,WriteRaw,2,(apIn,anSize))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iFileBase,WriteRaw,2)

/** ni -> iFileBase::Tell/0 **/
IDLC_METH_BEGIN(ni,iFileBase,Tell,0)
	IDLC_DECL_RETVAR(tI64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileBase,Tell,0,())
	IDLC_RET_FROM_BASE(ni::eType_I64,_Ret)
IDLC_METH_END(ni,iFileBase,Tell,0)

/** ni -> iFileBase::GetSize/0 **/
IDLC_METH_BEGIN(ni,iFileBase,GetSize,0)
	IDLC_DECL_RETVAR(tI64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileBase,GetSize,0,())
	IDLC_RET_FROM_BASE(ni::eType_I64,_Ret)
IDLC_METH_END(ni,iFileBase,GetSize,0)

/** ni -> iFileBase::SeekEnd/1 **/
IDLC_METH_BEGIN(ni,iFileBase,SeekEnd,1)
	IDLC_DECL_VAR(tI64,offset)
	IDLC_BUF_TO_BASE(ni::eType_I64,offset)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileBase,SeekEnd,1,(offset))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFileBase,SeekEnd,1)

/** ni -> iFileBase::Flush/0 **/
IDLC_METH_BEGIN(ni,iFileBase,Flush,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileBase,Flush,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFileBase,Flush,0)

/** ni -> iFileBase::GetTime/2 **/
IDLC_METH_BEGIN(ni,iFileBase,GetTime,2)
	IDLC_DECL_VAR(eFileTime,aFileTime)
	IDLC_BUF_TO_ENUM(eFileTime,aFileTime)
	IDLC_DECL_VAR(iTime*,apTime)
	IDLC_BUF_TO_INTF(iTime,apTime)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileBase,GetTime,2,(aFileTime,apTime))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFileBase,GetTime,2)

/** ni -> iFileBase::SetTime/2 **/
IDLC_METH_BEGIN(ni,iFileBase,SetTime,2)
	IDLC_DECL_VAR(eFileTime,aFileTime)
	IDLC_BUF_TO_ENUM(eFileTime,aFileTime)
	IDLC_DECL_VAR(iTime*,apTime)
	IDLC_BUF_TO_INTF(iTime,apTime)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileBase,SetTime,2,(aFileTime,apTime))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFileBase,SetTime,2)

/** ni -> iFileBase::Resize/1 **/
IDLC_METH_BEGIN(ni,iFileBase,Resize,1)
	IDLC_DECL_VAR(tI64,newSize)
	IDLC_BUF_TO_BASE(ni::eType_I64,newSize)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileBase,Resize,1,(newSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFileBase,Resize,1)

IDLC_END_INTF(ni,iFileBase)

/** interface : iFile **/
IDLC_BEGIN_INTF(ni,iFile)
/** ni -> iFile::GetFileBase/0 **/
IDLC_METH_BEGIN(ni,iFile,GetFileBase,0)
	IDLC_DECL_RETVAR(iFileBase*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,GetFileBase,0,())
	IDLC_RET_FROM_INTF(iFileBase,_Ret)
IDLC_METH_END(ni,iFile,GetFileBase,0)

/** ni -> iFile::GetSourcePath/0 **/
IDLC_METH_BEGIN(ni,iFile,GetSourcePath,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,GetSourcePath,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iFile,GetSourcePath,0)

/** ni -> iFile::Seek/1 **/
IDLC_METH_BEGIN(ni,iFile,Seek,1)
	IDLC_DECL_VAR(tI64,offset)
	IDLC_BUF_TO_BASE(ni::eType_I64,offset)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,Seek,1,(offset))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,Seek,1)

/** ni -> iFile::SeekSet/1 **/
IDLC_METH_BEGIN(ni,iFile,SeekSet,1)
	IDLC_DECL_VAR(tI64,offset)
	IDLC_BUF_TO_BASE(ni::eType_I64,offset)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,SeekSet,1,(offset))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,SeekSet,1)

/** ni -> iFile::ReadRaw/2 **/
IDLC_METH_BEGIN(ni,iFile,ReadRaw,2)
	IDLC_DECL_VAR(void*,pOut)
	IDLC_BUF_TO_PTR(ni::eType_Ptr,pOut)
	IDLC_DECL_VAR(tSize,nSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,nSize)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadRaw,2,(pOut,nSize))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iFile,ReadRaw,2)

/** ni -> iFile::WriteRaw/2 **/
IDLC_METH_BEGIN(ni,iFile,WriteRaw,2)
	IDLC_DECL_VAR(void*,pIn)
	IDLC_BUF_TO_PTR(ni::eTypeFlags_Constant|ni::eType_Ptr,pIn)
	IDLC_DECL_VAR(tSize,nSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,nSize)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,WriteRaw,2,(pIn,nSize))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iFile,WriteRaw,2)

/** ni -> iFile::Tell/0 **/
IDLC_METH_BEGIN(ni,iFile,Tell,0)
	IDLC_DECL_RETVAR(tI64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,Tell,0,())
	IDLC_RET_FROM_BASE(ni::eType_I64,_Ret)
IDLC_METH_END(ni,iFile,Tell,0)

/** ni -> iFile::GetSize/0 **/
IDLC_METH_BEGIN(ni,iFile,GetSize,0)
	IDLC_DECL_RETVAR(tI64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,GetSize,0,())
	IDLC_RET_FROM_BASE(ni::eType_I64,_Ret)
IDLC_METH_END(ni,iFile,GetSize,0)

/** ni -> iFile::SeekEnd/1 **/
IDLC_METH_BEGIN(ni,iFile,SeekEnd,1)
	IDLC_DECL_VAR(tI64,offset)
	IDLC_BUF_TO_BASE(ni::eType_I64,offset)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,SeekEnd,1,(offset))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,SeekEnd,1)

/** ni -> iFile::Flush/0 **/
IDLC_METH_BEGIN(ni,iFile,Flush,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,Flush,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,Flush,0)

/** ni -> iFile::GetTime/2 **/
IDLC_METH_BEGIN(ni,iFile,GetTime,2)
	IDLC_DECL_VAR(eFileTime,aFileTime)
	IDLC_BUF_TO_ENUM(eFileTime,aFileTime)
	IDLC_DECL_VAR(iTime*,apTime)
	IDLC_BUF_TO_INTF(iTime,apTime)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,GetTime,2,(aFileTime,apTime))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,GetTime,2)

/** ni -> iFile::SetTime/2 **/
IDLC_METH_BEGIN(ni,iFile,SetTime,2)
	IDLC_DECL_VAR(eFileTime,aFileTime)
	IDLC_BUF_TO_ENUM(eFileTime,aFileTime)
	IDLC_DECL_VAR(iTime*,apTime)
	IDLC_BUF_TO_INTF(iTime,apTime)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,SetTime,2,(aFileTime,apTime))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,SetTime,2)

/** ni -> iFile::Resize/1 **/
IDLC_METH_BEGIN(ni,iFile,Resize,1)
	IDLC_DECL_VAR(tI64,newSize)
	IDLC_BUF_TO_BASE(ni::eType_I64,newSize)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,Resize,1,(newSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,Resize,1)

/** ni -> iFile::GetBase/0 **/
IDLC_METH_BEGIN(ni,iFile,GetBase,0)
	IDLC_DECL_RETVAR(tPtr,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,GetBase,0,())
	IDLC_RET_FROM_PTR(ni::eType_Ptr,_Ret)
IDLC_METH_END(ni,iFile,GetBase,0)

/** ni -> iFile::GetHere/0 **/
IDLC_METH_BEGIN(ni,iFile,GetHere,0)
	IDLC_DECL_RETVAR(tPtr,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,GetHere,0,())
	IDLC_RET_FROM_PTR(ni::eType_Ptr,_Ret)
IDLC_METH_END(ni,iFile,GetHere,0)

/** ni -> iFile::GetStop/0 **/
IDLC_METH_BEGIN(ni,iFile,GetStop,0)
	IDLC_DECL_RETVAR(tPtr,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,GetStop,0,())
	IDLC_RET_FROM_PTR(ni::eType_Ptr,_Ret)
IDLC_METH_END(ni,iFile,GetStop,0)

/** ni -> iFile::SetMemPtr/4 **/
IDLC_METH_BEGIN(ni,iFile,SetMemPtr,4)
	IDLC_DECL_VAR(tPtr,apMem)
	IDLC_BUF_TO_PTR(ni::eType_Ptr,apMem)
	IDLC_DECL_VAR(tSize,anSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,anSize)
	IDLC_DECL_VAR(tBool,abFree)
	IDLC_BUF_TO_BASE(ni::eType_I8,abFree)
	IDLC_DECL_VAR(tBool,abKeepHere)
	IDLC_BUF_TO_BASE(ni::eType_I8,abKeepHere)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,SetMemPtr,4,(apMem,anSize,abFree,abKeepHere))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,SetMemPtr,4)

/** ni -> iFile::Reset/0 **/
IDLC_METH_BEGIN(ni,iFile,Reset,0)
	IDLC_METH_CALL_VOID(ni,iFile,Reset,0,())
IDLC_METH_END(ni,iFile,Reset,0)

/** ni -> iFile::WriteFile/2 **/
IDLC_METH_BEGIN(ni,iFile,WriteFile,2)
	IDLC_DECL_VAR(iFileBase*,apFile)
	IDLC_BUF_TO_INTF(iFileBase,apFile)
	IDLC_DECL_VAR(tI64,anSize)
	IDLC_BUF_TO_BASE(ni::eType_I64,anSize)
	IDLC_DECL_RETVAR(tI64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,WriteFile,2,(apFile,anSize))
	IDLC_RET_FROM_BASE(ni::eType_I64,_Ret)
IDLC_METH_END(ni,iFile,WriteFile,2)

/** ni -> iFile::GetFileFlags/0 **/
IDLC_METH_BEGIN(ni,iFile,GetFileFlags,0)
	IDLC_DECL_RETVAR(tFileFlags,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,GetFileFlags,0,())
	IDLC_RET_FROM_ENUM(tFileFlags,_Ret)
IDLC_METH_END(ni,iFile,GetFileFlags,0)

/** ni -> iFile::GetCanRead/0 **/
IDLC_METH_BEGIN(ni,iFile,GetCanRead,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,GetCanRead,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,GetCanRead,0)

/** ni -> iFile::GetCanWrite/0 **/
IDLC_METH_BEGIN(ni,iFile,GetCanWrite,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,GetCanWrite,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,GetCanWrite,0)

/** ni -> iFile::GetCanSeek/0 **/
IDLC_METH_BEGIN(ni,iFile,GetCanSeek,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,GetCanSeek,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,GetCanSeek,0)

/** ni -> iFile::GetPartialRead/0 **/
IDLC_METH_BEGIN(ni,iFile,GetPartialRead,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,GetPartialRead,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,GetPartialRead,0)

/** ni -> iFile::GetPartialWrite/0 **/
IDLC_METH_BEGIN(ni,iFile,GetPartialWrite,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,GetPartialWrite,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,GetPartialWrite,0)

/** ni -> iFile::GetIsEncoder/0 **/
IDLC_METH_BEGIN(ni,iFile,GetIsEncoder,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,GetIsEncoder,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,GetIsEncoder,0)

/** ni -> iFile::GetIsDummy/0 **/
IDLC_METH_BEGIN(ni,iFile,GetIsDummy,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,GetIsDummy,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,GetIsDummy,0)

/** ni -> iFile::GetIsMemory/0 **/
IDLC_METH_BEGIN(ni,iFile,GetIsMemory,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,GetIsMemory,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,GetIsMemory,0)

/** ni -> iFile::GetIsStream/0 **/
IDLC_METH_BEGIN(ni,iFile,GetIsStream,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,GetIsStream,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,GetIsStream,0)

/** ni -> iFile::ReadF32/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadF32,0)
	IDLC_DECL_RETVAR(tF32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadF32,0,())
	IDLC_RET_FROM_BASE(ni::eType_F32,_Ret)
IDLC_METH_END(ni,iFile,ReadF32,0)

/** ni -> iFile::ReadF64/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadF64,0)
	IDLC_DECL_RETVAR(tF64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadF64,0,())
	IDLC_RET_FROM_BASE(ni::eType_F64,_Ret)
IDLC_METH_END(ni,iFile,ReadF64,0)

/** ni -> iFile::Read8/0 **/
IDLC_METH_BEGIN(ni,iFile,Read8,0)
	IDLC_DECL_RETVAR(tU8,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,Read8,0,())
	IDLC_RET_FROM_BASE(ni::eType_U8,_Ret)
IDLC_METH_END(ni,iFile,Read8,0)

/** ni -> iFile::ReadLE16/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadLE16,0)
	IDLC_DECL_RETVAR(tU16,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadLE16,0,())
	IDLC_RET_FROM_BASE(ni::eType_U16,_Ret)
IDLC_METH_END(ni,iFile,ReadLE16,0)

/** ni -> iFile::ReadBE16/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadBE16,0)
	IDLC_DECL_RETVAR(tU16,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBE16,0,())
	IDLC_RET_FROM_BASE(ni::eType_U16,_Ret)
IDLC_METH_END(ni,iFile,ReadBE16,0)

/** ni -> iFile::ReadLE32/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadLE32,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadLE32,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iFile,ReadLE32,0)

/** ni -> iFile::ReadBE32/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadBE32,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBE32,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iFile,ReadBE32,0)

/** ni -> iFile::ReadLE64/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadLE64,0)
	IDLC_DECL_RETVAR(tU64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadLE64,0,())
	IDLC_RET_FROM_BASE(ni::eType_U64,_Ret)
IDLC_METH_END(ni,iFile,ReadLE64,0)

/** ni -> iFile::ReadBE64/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadBE64,0)
	IDLC_DECL_RETVAR(tU64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBE64,0,())
	IDLC_RET_FROM_BASE(ni::eType_U64,_Ret)
IDLC_METH_END(ni,iFile,ReadBE64,0)

/** ni -> iFile::ReadF32Array/2 -> NO AUTOMATION **/
/** ni -> iFile::ReadF64Array/2 -> NO AUTOMATION **/
/** ni -> iFile::ReadLE16Array/2 -> NO AUTOMATION **/
/** ni -> iFile::ReadBE16Array/2 -> NO AUTOMATION **/
/** ni -> iFile::ReadLE32Array/2 -> NO AUTOMATION **/
/** ni -> iFile::ReadBE32Array/2 -> NO AUTOMATION **/
/** ni -> iFile::ReadLE64Array/2 -> NO AUTOMATION **/
/** ni -> iFile::ReadBE64Array/2 -> NO AUTOMATION **/
/** ni -> iFile::WriteF32/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteF32,1)
	IDLC_DECL_VAR(tF32,v)
	IDLC_BUF_TO_BASE(ni::eType_F32,v)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,WriteF32,1,(v))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iFile,WriteF32,1)

/** ni -> iFile::WriteF64/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteF64,1)
	IDLC_DECL_VAR(tF64,v)
	IDLC_BUF_TO_BASE(ni::eType_F64,v)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,WriteF64,1,(v))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iFile,WriteF64,1)

/** ni -> iFile::Write8/1 **/
IDLC_METH_BEGIN(ni,iFile,Write8,1)
	IDLC_DECL_VAR(tU8,v)
	IDLC_BUF_TO_BASE(ni::eType_U8,v)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,Write8,1,(v))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iFile,Write8,1)

/** ni -> iFile::WriteLE16/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteLE16,1)
	IDLC_DECL_VAR(tU16,v)
	IDLC_BUF_TO_BASE(ni::eType_U16,v)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,WriteLE16,1,(v))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iFile,WriteLE16,1)

/** ni -> iFile::WriteBE16/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteBE16,1)
	IDLC_DECL_VAR(tU16,v)
	IDLC_BUF_TO_BASE(ni::eType_U16,v)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,WriteBE16,1,(v))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iFile,WriteBE16,1)

/** ni -> iFile::WriteLE32/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteLE32,1)
	IDLC_DECL_VAR(tU32,v)
	IDLC_BUF_TO_BASE(ni::eType_U32,v)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,WriteLE32,1,(v))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iFile,WriteLE32,1)

/** ni -> iFile::WriteBE32/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteBE32,1)
	IDLC_DECL_VAR(tU32,v)
	IDLC_BUF_TO_BASE(ni::eType_U32,v)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,WriteBE32,1,(v))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iFile,WriteBE32,1)

/** ni -> iFile::WriteLE64/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteLE64,1)
	IDLC_DECL_VAR(tU64,v)
	IDLC_BUF_TO_BASE(ni::eType_U64,v)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,WriteLE64,1,(v))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iFile,WriteLE64,1)

/** ni -> iFile::WriteBE64/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteBE64,1)
	IDLC_DECL_VAR(tU64,v)
	IDLC_BUF_TO_BASE(ni::eType_U64,v)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,WriteBE64,1,(v))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iFile,WriteBE64,1)

/** ni -> iFile::WriteF32Array/2 -> NO AUTOMATION **/
/** ni -> iFile::WriteF64Array/2 -> NO AUTOMATION **/
/** ni -> iFile::WriteLE16Array/2 -> NO AUTOMATION **/
/** ni -> iFile::WriteBE16Array/2 -> NO AUTOMATION **/
/** ni -> iFile::WriteLE32Array/2 -> NO AUTOMATION **/
/** ni -> iFile::WriteBE32Array/2 -> NO AUTOMATION **/
/** ni -> iFile::WriteLE64Array/2 -> NO AUTOMATION **/
/** ni -> iFile::WriteBE64Array/2 -> NO AUTOMATION **/
/** ni -> iFile::GetTextEncodingFormat/0 **/
IDLC_METH_BEGIN(ni,iFile,GetTextEncodingFormat,0)
	IDLC_DECL_RETVAR(eTextEncodingFormat,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,GetTextEncodingFormat,0,())
	IDLC_RET_FROM_ENUM(eTextEncodingFormat,_Ret)
IDLC_METH_END(ni,iFile,GetTextEncodingFormat,0)

/** ni -> iFile::SetTextEncodingFormat/1 **/
IDLC_METH_BEGIN(ni,iFile,SetTextEncodingFormat,1)
	IDLC_DECL_VAR(eTextEncodingFormat,aFormat)
	IDLC_BUF_TO_ENUM(eTextEncodingFormat,aFormat)
	IDLC_METH_CALL_VOID(ni,iFile,SetTextEncodingFormat,1,(aFormat))
IDLC_METH_END(ni,iFile,SetTextEncodingFormat,1)

/** ni -> iFile::BeginTextFileRead/1 **/
IDLC_METH_BEGIN(ni,iFile,BeginTextFileRead,1)
	IDLC_DECL_VAR(tBool,abSeekSetZero)
	IDLC_BUF_TO_BASE(ni::eType_I8,abSeekSetZero)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,BeginTextFileRead,1,(abSeekSetZero))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,BeginTextFileRead,1)

/** ni -> iFile::BeginTextFileWrite/2 **/
IDLC_METH_BEGIN(ni,iFile,BeginTextFileWrite,2)
	IDLC_DECL_VAR(eTextEncodingFormat,aFormat)
	IDLC_BUF_TO_ENUM(eTextEncodingFormat,aFormat)
	IDLC_DECL_VAR(tBool,abSeekSetZero)
	IDLC_BUF_TO_BASE(ni::eType_I8,abSeekSetZero)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,BeginTextFileWrite,2,(aFormat,abSeekSetZero))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,BeginTextFileWrite,2)

/** ni -> iFile::ReadString/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadString,0)
	IDLC_DECL_RETVAR(cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadString,0,())
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iFile,ReadString,0)

/** ni -> iFile::ReadStringLine/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadStringLine,0)
	IDLC_DECL_RETVAR(cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadStringLine,0,())
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iFile,ReadStringLine,0)

/** ni -> iFile::WriteString/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteString,1)
	IDLC_DECL_VAR(achar*,aaszString)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszString)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,WriteString,1,(aaszString))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iFile,WriteString,1)

/** ni -> iFile::WriteStringZ/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteStringZ,1)
	IDLC_DECL_VAR(achar*,aaszString)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszString)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,WriteStringZ,1,(aaszString))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iFile,WriteStringZ,1)

/** ni -> iFile::ReadChar/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadChar,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadChar,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iFile,ReadChar,0)

/** ni -> iFile::WriteChar/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteChar,1)
	IDLC_DECL_VAR(tU32,anChar)
	IDLC_BUF_TO_BASE(ni::eType_U32,anChar)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,WriteChar,1,(anChar))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iFile,WriteChar,1)

/** ni -> iFile::ReadStringEx/1 **/
IDLC_METH_BEGIN(ni,iFile,ReadStringEx,1)
	IDLC_DECL_VAR(eTextEncodingFormat,aFmt)
	IDLC_BUF_TO_ENUM(eTextEncodingFormat,aFmt)
	IDLC_DECL_RETVAR(cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadStringEx,1,(aFmt))
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iFile,ReadStringEx,1)

/** ni -> iFile::ReadStringLineEx/1 **/
IDLC_METH_BEGIN(ni,iFile,ReadStringLineEx,1)
	IDLC_DECL_VAR(eTextEncodingFormat,aFmt)
	IDLC_BUF_TO_ENUM(eTextEncodingFormat,aFmt)
	IDLC_DECL_RETVAR(cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadStringLineEx,1,(aFmt))
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iFile,ReadStringLineEx,1)

/** ni -> iFile::WriteStringEx/2 **/
IDLC_METH_BEGIN(ni,iFile,WriteStringEx,2)
	IDLC_DECL_VAR(eTextEncodingFormat,aFmt)
	IDLC_BUF_TO_ENUM(eTextEncodingFormat,aFmt)
	IDLC_DECL_VAR(achar*,aaszString)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszString)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,WriteStringEx,2,(aFmt,aaszString))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iFile,WriteStringEx,2)

/** ni -> iFile::WriteStringZEx/2 **/
IDLC_METH_BEGIN(ni,iFile,WriteStringZEx,2)
	IDLC_DECL_VAR(eTextEncodingFormat,aFmt)
	IDLC_BUF_TO_ENUM(eTextEncodingFormat,aFmt)
	IDLC_DECL_VAR(achar*,aaszString)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszString)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,WriteStringZEx,2,(aFmt,aaszString))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iFile,WriteStringZEx,2)

/** ni -> iFile::ReadCharEx/1 **/
IDLC_METH_BEGIN(ni,iFile,ReadCharEx,1)
	IDLC_DECL_VAR(eTextEncodingFormat,aFmt)
	IDLC_BUF_TO_ENUM(eTextEncodingFormat,aFmt)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadCharEx,1,(aFmt))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iFile,ReadCharEx,1)

/** ni -> iFile::WriteCharEx/2 **/
IDLC_METH_BEGIN(ni,iFile,WriteCharEx,2)
	IDLC_DECL_VAR(eTextEncodingFormat,aFmt)
	IDLC_BUF_TO_ENUM(eTextEncodingFormat,aFmt)
	IDLC_DECL_VAR(tU32,anChar)
	IDLC_BUF_TO_BASE(ni::eType_U32,anChar)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,WriteCharEx,2,(aFmt,anChar))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iFile,WriteCharEx,2)

/** ni -> iFile::BeginWriteBits/0 **/
IDLC_METH_BEGIN(ni,iFile,BeginWriteBits,0)
	IDLC_METH_CALL_VOID(ni,iFile,BeginWriteBits,0,())
IDLC_METH_END(ni,iFile,BeginWriteBits,0)

/** ni -> iFile::EndWriteBits/0 **/
IDLC_METH_BEGIN(ni,iFile,EndWriteBits,0)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,EndWriteBits,0,())
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iFile,EndWriteBits,0)

/** ni -> iFile::IsWriteBitsBegan/0 **/
IDLC_METH_BEGIN(ni,iFile,IsWriteBitsBegan,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,IsWriteBitsBegan,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,IsWriteBitsBegan,0)

/** ni -> iFile::WriteBit/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteBit,1)
	IDLC_DECL_VAR(tBool,abBit)
	IDLC_BUF_TO_BASE(ni::eType_I8,abBit)
	IDLC_METH_CALL_VOID(ni,iFile,WriteBit,1,(abBit))
IDLC_METH_END(ni,iFile,WriteBit,1)

/** ni -> iFile::WriteBits/3 **/
IDLC_METH_BEGIN(ni,iFile,WriteBits,3)
	IDLC_DECL_VAR(tPtr,apData)
	IDLC_BUF_TO_PTR(ni::eType_Ptr,apData)
	IDLC_DECL_VAR(tU32,anBits)
	IDLC_BUF_TO_BASE(ni::eType_U32,anBits)
	IDLC_DECL_VAR(tBool,abInvertBytesOrder)
	IDLC_BUF_TO_BASE(ni::eType_I8,abInvertBytesOrder)
	IDLC_METH_CALL_VOID(ni,iFile,WriteBits,3,(apData,anBits,abInvertBytesOrder))
IDLC_METH_END(ni,iFile,WriteBits,3)

/** ni -> iFile::WriteBits8/2 **/
IDLC_METH_BEGIN(ni,iFile,WriteBits8,2)
	IDLC_DECL_VAR(tU8,anData)
	IDLC_BUF_TO_BASE(ni::eType_U8,anData)
	IDLC_DECL_VAR(tU32,anBits)
	IDLC_BUF_TO_BASE(ni::eType_U32,anBits)
	IDLC_METH_CALL_VOID(ni,iFile,WriteBits8,2,(anData,anBits))
IDLC_METH_END(ni,iFile,WriteBits8,2)

/** ni -> iFile::WriteBits16/2 **/
IDLC_METH_BEGIN(ni,iFile,WriteBits16,2)
	IDLC_DECL_VAR(tU16,anData)
	IDLC_BUF_TO_BASE(ni::eType_U16,anData)
	IDLC_DECL_VAR(tU32,anBits)
	IDLC_BUF_TO_BASE(ni::eType_U32,anBits)
	IDLC_METH_CALL_VOID(ni,iFile,WriteBits16,2,(anData,anBits))
IDLC_METH_END(ni,iFile,WriteBits16,2)

/** ni -> iFile::WriteBits32/2 **/
IDLC_METH_BEGIN(ni,iFile,WriteBits32,2)
	IDLC_DECL_VAR(tU32,anData)
	IDLC_BUF_TO_BASE(ni::eType_U32,anData)
	IDLC_DECL_VAR(tU32,anBits)
	IDLC_BUF_TO_BASE(ni::eType_U32,anBits)
	IDLC_METH_CALL_VOID(ni,iFile,WriteBits32,2,(anData,anBits))
IDLC_METH_END(ni,iFile,WriteBits32,2)

/** ni -> iFile::WriteBits64/2 **/
IDLC_METH_BEGIN(ni,iFile,WriteBits64,2)
	IDLC_DECL_VAR(tU64,anData)
	IDLC_BUF_TO_BASE(ni::eType_U64,anData)
	IDLC_DECL_VAR(tU32,anBits)
	IDLC_BUF_TO_BASE(ni::eType_U32,anBits)
	IDLC_METH_CALL_VOID(ni,iFile,WriteBits64,2,(anData,anBits))
IDLC_METH_END(ni,iFile,WriteBits64,2)

/** ni -> iFile::BeginReadBits/0 **/
IDLC_METH_BEGIN(ni,iFile,BeginReadBits,0)
	IDLC_METH_CALL_VOID(ni,iFile,BeginReadBits,0,())
IDLC_METH_END(ni,iFile,BeginReadBits,0)

/** ni -> iFile::EndReadBits/0 **/
IDLC_METH_BEGIN(ni,iFile,EndReadBits,0)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,EndReadBits,0,())
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iFile,EndReadBits,0)

/** ni -> iFile::IsReadBitsBegan/0 **/
IDLC_METH_BEGIN(ni,iFile,IsReadBitsBegan,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,IsReadBitsBegan,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,IsReadBitsBegan,0)

/** ni -> iFile::ReadBit/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadBit,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBit,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,ReadBit,0)

/** ni -> iFile::ReadBits/3 **/
IDLC_METH_BEGIN(ni,iFile,ReadBits,3)
	IDLC_DECL_VAR(tPtr,apData)
	IDLC_BUF_TO_PTR(ni::eType_Ptr,apData)
	IDLC_DECL_VAR(tU32,anBits)
	IDLC_BUF_TO_BASE(ni::eType_U32,anBits)
	IDLC_DECL_VAR(tBool,abInvertBytesOrder)
	IDLC_BUF_TO_BASE(ni::eType_I8,abInvertBytesOrder)
	IDLC_METH_CALL_VOID(ni,iFile,ReadBits,3,(apData,anBits,abInvertBytesOrder))
IDLC_METH_END(ni,iFile,ReadBits,3)

/** ni -> iFile::ReadBitsU8/1 **/
IDLC_METH_BEGIN(ni,iFile,ReadBitsU8,1)
	IDLC_DECL_VAR(tU32,anBits)
	IDLC_BUF_TO_BASE(ni::eType_U32,anBits)
	IDLC_DECL_RETVAR(tU8,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBitsU8,1,(anBits))
	IDLC_RET_FROM_BASE(ni::eType_U8,_Ret)
IDLC_METH_END(ni,iFile,ReadBitsU8,1)

/** ni -> iFile::ReadBitsU16/1 **/
IDLC_METH_BEGIN(ni,iFile,ReadBitsU16,1)
	IDLC_DECL_VAR(tU32,anBits)
	IDLC_BUF_TO_BASE(ni::eType_U32,anBits)
	IDLC_DECL_RETVAR(tU16,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBitsU16,1,(anBits))
	IDLC_RET_FROM_BASE(ni::eType_U16,_Ret)
IDLC_METH_END(ni,iFile,ReadBitsU16,1)

/** ni -> iFile::ReadBitsU32/1 **/
IDLC_METH_BEGIN(ni,iFile,ReadBitsU32,1)
	IDLC_DECL_VAR(tU32,anBits)
	IDLC_BUF_TO_BASE(ni::eType_U32,anBits)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBitsU32,1,(anBits))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iFile,ReadBitsU32,1)

/** ni -> iFile::ReadBitsU64/1 **/
IDLC_METH_BEGIN(ni,iFile,ReadBitsU64,1)
	IDLC_DECL_VAR(tU32,anBits)
	IDLC_BUF_TO_BASE(ni::eType_U32,anBits)
	IDLC_DECL_RETVAR(tU64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBitsU64,1,(anBits))
	IDLC_RET_FROM_BASE(ni::eType_U64,_Ret)
IDLC_METH_END(ni,iFile,ReadBitsU64,1)

/** ni -> iFile::ReadBitsI8/1 **/
IDLC_METH_BEGIN(ni,iFile,ReadBitsI8,1)
	IDLC_DECL_VAR(tU32,anBits)
	IDLC_BUF_TO_BASE(ni::eType_U32,anBits)
	IDLC_DECL_RETVAR(tI8,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBitsI8,1,(anBits))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,ReadBitsI8,1)

/** ni -> iFile::ReadBitsI16/1 **/
IDLC_METH_BEGIN(ni,iFile,ReadBitsI16,1)
	IDLC_DECL_VAR(tU32,anBits)
	IDLC_BUF_TO_BASE(ni::eType_U32,anBits)
	IDLC_DECL_RETVAR(tI16,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBitsI16,1,(anBits))
	IDLC_RET_FROM_BASE(ni::eType_I16,_Ret)
IDLC_METH_END(ni,iFile,ReadBitsI16,1)

/** ni -> iFile::ReadBitsI32/1 **/
IDLC_METH_BEGIN(ni,iFile,ReadBitsI32,1)
	IDLC_DECL_VAR(tU32,anBits)
	IDLC_BUF_TO_BASE(ni::eType_U32,anBits)
	IDLC_DECL_RETVAR(tI32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBitsI32,1,(anBits))
	IDLC_RET_FROM_BASE(ni::eType_I32,_Ret)
IDLC_METH_END(ni,iFile,ReadBitsI32,1)

/** ni -> iFile::ReadBitsI64/1 **/
IDLC_METH_BEGIN(ni,iFile,ReadBitsI64,1)
	IDLC_DECL_VAR(tU32,anBits)
	IDLC_BUF_TO_BASE(ni::eType_U32,anBits)
	IDLC_DECL_RETVAR(tI64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBitsI64,1,(anBits))
	IDLC_RET_FROM_BASE(ni::eType_I64,_Ret)
IDLC_METH_END(ni,iFile,ReadBitsI64,1)

/** ni -> iFile::GetMinNumBitsU32/1 **/
IDLC_METH_BEGIN(ni,iFile,GetMinNumBitsU32,1)
	IDLC_DECL_VAR(tU32,anNumber)
	IDLC_BUF_TO_BASE(ni::eType_U32,anNumber)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,GetMinNumBitsU32,1,(anNumber))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iFile,GetMinNumBitsU32,1)

/** ni -> iFile::GetMinNumBitsI32/1 **/
IDLC_METH_BEGIN(ni,iFile,GetMinNumBitsI32,1)
	IDLC_DECL_VAR(tI32,anNumber)
	IDLC_BUF_TO_BASE(ni::eType_I32,anNumber)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,GetMinNumBitsI32,1,(anNumber))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iFile,GetMinNumBitsI32,1)

/** ni -> iFile::GetMinNumBitsU64/1 **/
IDLC_METH_BEGIN(ni,iFile,GetMinNumBitsU64,1)
	IDLC_DECL_VAR(tU64,anNumber)
	IDLC_BUF_TO_BASE(ni::eType_U64,anNumber)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,GetMinNumBitsU64,1,(anNumber))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iFile,GetMinNumBitsU64,1)

/** ni -> iFile::GetMinNumBitsI64/1 **/
IDLC_METH_BEGIN(ni,iFile,GetMinNumBitsI64,1)
	IDLC_DECL_VAR(tI64,anNumber)
	IDLC_BUF_TO_BASE(ni::eType_I64,anNumber)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,GetMinNumBitsI64,1,(anNumber))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iFile,GetMinNumBitsI64,1)

/** ni -> iFile::WriteBitsPackedU64/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteBitsPackedU64,1)
	IDLC_DECL_VAR(tU64,anNumber)
	IDLC_BUF_TO_BASE(ni::eType_U64,anNumber)
	IDLC_METH_CALL_VOID(ni,iFile,WriteBitsPackedU64,1,(anNumber))
IDLC_METH_END(ni,iFile,WriteBitsPackedU64,1)

/** ni -> iFile::WriteBitsPackedU48/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteBitsPackedU48,1)
	IDLC_DECL_VAR(tU64,anNumber)
	IDLC_BUF_TO_BASE(ni::eType_U64,anNumber)
	IDLC_METH_CALL_VOID(ni,iFile,WriteBitsPackedU48,1,(anNumber))
IDLC_METH_END(ni,iFile,WriteBitsPackedU48,1)

/** ni -> iFile::WriteBitsPackedU32/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteBitsPackedU32,1)
	IDLC_DECL_VAR(tU32,anNumber)
	IDLC_BUF_TO_BASE(ni::eType_U32,anNumber)
	IDLC_METH_CALL_VOID(ni,iFile,WriteBitsPackedU32,1,(anNumber))
IDLC_METH_END(ni,iFile,WriteBitsPackedU32,1)

/** ni -> iFile::WriteBitsPackedU24/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteBitsPackedU24,1)
	IDLC_DECL_VAR(tU32,anNumber)
	IDLC_BUF_TO_BASE(ni::eType_U32,anNumber)
	IDLC_METH_CALL_VOID(ni,iFile,WriteBitsPackedU24,1,(anNumber))
IDLC_METH_END(ni,iFile,WriteBitsPackedU24,1)

/** ni -> iFile::WriteBitsPackedU16/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteBitsPackedU16,1)
	IDLC_DECL_VAR(tU16,anNumber)
	IDLC_BUF_TO_BASE(ni::eType_U16,anNumber)
	IDLC_METH_CALL_VOID(ni,iFile,WriteBitsPackedU16,1,(anNumber))
IDLC_METH_END(ni,iFile,WriteBitsPackedU16,1)

/** ni -> iFile::WriteBitsPackedU8/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteBitsPackedU8,1)
	IDLC_DECL_VAR(tU8,anNumber)
	IDLC_BUF_TO_BASE(ni::eType_U8,anNumber)
	IDLC_METH_CALL_VOID(ni,iFile,WriteBitsPackedU8,1,(anNumber))
IDLC_METH_END(ni,iFile,WriteBitsPackedU8,1)

/** ni -> iFile::ReadBitsPackedU64/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadBitsPackedU64,0)
	IDLC_DECL_RETVAR(tU64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBitsPackedU64,0,())
	IDLC_RET_FROM_BASE(ni::eType_U64,_Ret)
IDLC_METH_END(ni,iFile,ReadBitsPackedU64,0)

/** ni -> iFile::ReadBitsPackedU48/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadBitsPackedU48,0)
	IDLC_DECL_RETVAR(tU64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBitsPackedU48,0,())
	IDLC_RET_FROM_BASE(ni::eType_U64,_Ret)
IDLC_METH_END(ni,iFile,ReadBitsPackedU48,0)

/** ni -> iFile::ReadBitsPackedU32/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadBitsPackedU32,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBitsPackedU32,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iFile,ReadBitsPackedU32,0)

/** ni -> iFile::ReadBitsPackedU24/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadBitsPackedU24,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBitsPackedU24,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iFile,ReadBitsPackedU24,0)

/** ni -> iFile::ReadBitsPackedU16/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadBitsPackedU16,0)
	IDLC_DECL_RETVAR(tU16,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBitsPackedU16,0,())
	IDLC_RET_FROM_BASE(ni::eType_U16,_Ret)
IDLC_METH_END(ni,iFile,ReadBitsPackedU16,0)

/** ni -> iFile::ReadBitsPackedU8/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadBitsPackedU8,0)
	IDLC_DECL_RETVAR(tU8,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBitsPackedU8,0,())
	IDLC_RET_FROM_BASE(ni::eType_U8,_Ret)
IDLC_METH_END(ni,iFile,ReadBitsPackedU8,0)

/** ni -> iFile::WriteBitsPackedI64/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteBitsPackedI64,1)
	IDLC_DECL_VAR(tI64,anNumber)
	IDLC_BUF_TO_BASE(ni::eType_I64,anNumber)
	IDLC_METH_CALL_VOID(ni,iFile,WriteBitsPackedI64,1,(anNumber))
IDLC_METH_END(ni,iFile,WriteBitsPackedI64,1)

/** ni -> iFile::WriteBitsPackedI48/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteBitsPackedI48,1)
	IDLC_DECL_VAR(tI64,anNumber)
	IDLC_BUF_TO_BASE(ni::eType_I64,anNumber)
	IDLC_METH_CALL_VOID(ni,iFile,WriteBitsPackedI48,1,(anNumber))
IDLC_METH_END(ni,iFile,WriteBitsPackedI48,1)

/** ni -> iFile::WriteBitsPackedI32/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteBitsPackedI32,1)
	IDLC_DECL_VAR(tI32,anNumber)
	IDLC_BUF_TO_BASE(ni::eType_I32,anNumber)
	IDLC_METH_CALL_VOID(ni,iFile,WriteBitsPackedI32,1,(anNumber))
IDLC_METH_END(ni,iFile,WriteBitsPackedI32,1)

/** ni -> iFile::WriteBitsPackedI24/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteBitsPackedI24,1)
	IDLC_DECL_VAR(tI32,anNumber)
	IDLC_BUF_TO_BASE(ni::eType_I32,anNumber)
	IDLC_METH_CALL_VOID(ni,iFile,WriteBitsPackedI24,1,(anNumber))
IDLC_METH_END(ni,iFile,WriteBitsPackedI24,1)

/** ni -> iFile::WriteBitsPackedI16/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteBitsPackedI16,1)
	IDLC_DECL_VAR(tI16,anNumber)
	IDLC_BUF_TO_BASE(ni::eType_I16,anNumber)
	IDLC_METH_CALL_VOID(ni,iFile,WriteBitsPackedI16,1,(anNumber))
IDLC_METH_END(ni,iFile,WriteBitsPackedI16,1)

/** ni -> iFile::WriteBitsPackedI8/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteBitsPackedI8,1)
	IDLC_DECL_VAR(tI8,anNumber)
	IDLC_BUF_TO_BASE(ni::eType_I8,anNumber)
	IDLC_METH_CALL_VOID(ni,iFile,WriteBitsPackedI8,1,(anNumber))
IDLC_METH_END(ni,iFile,WriteBitsPackedI8,1)

/** ni -> iFile::ReadBitsPackedI64/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadBitsPackedI64,0)
	IDLC_DECL_RETVAR(tI64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBitsPackedI64,0,())
	IDLC_RET_FROM_BASE(ni::eType_I64,_Ret)
IDLC_METH_END(ni,iFile,ReadBitsPackedI64,0)

/** ni -> iFile::ReadBitsPackedI48/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadBitsPackedI48,0)
	IDLC_DECL_RETVAR(tI64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBitsPackedI48,0,())
	IDLC_RET_FROM_BASE(ni::eType_I64,_Ret)
IDLC_METH_END(ni,iFile,ReadBitsPackedI48,0)

/** ni -> iFile::ReadBitsPackedI32/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadBitsPackedI32,0)
	IDLC_DECL_RETVAR(tI32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBitsPackedI32,0,())
	IDLC_RET_FROM_BASE(ni::eType_I32,_Ret)
IDLC_METH_END(ni,iFile,ReadBitsPackedI32,0)

/** ni -> iFile::ReadBitsPackedI24/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadBitsPackedI24,0)
	IDLC_DECL_RETVAR(tI32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBitsPackedI24,0,())
	IDLC_RET_FROM_BASE(ni::eType_I32,_Ret)
IDLC_METH_END(ni,iFile,ReadBitsPackedI24,0)

/** ni -> iFile::ReadBitsPackedI16/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadBitsPackedI16,0)
	IDLC_DECL_RETVAR(tI16,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBitsPackedI16,0,())
	IDLC_RET_FROM_BASE(ni::eType_I16,_Ret)
IDLC_METH_END(ni,iFile,ReadBitsPackedI16,0)

/** ni -> iFile::ReadBitsPackedI8/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadBitsPackedI8,0)
	IDLC_DECL_RETVAR(tI8,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBitsPackedI8,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,ReadBitsPackedI8,0)

/** ni -> iFile::ReadBitsString/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadBitsString,0)
	IDLC_DECL_RETVAR(cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBitsString,0,())
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iFile,ReadBitsString,0)

/** ni -> iFile::WriteBitsString/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteBitsString,1)
	IDLC_DECL_VAR(achar*,aaszIn)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszIn)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,WriteBitsString,1,(aaszIn))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iFile,WriteBitsString,1)

/** ni -> iFile::WriteVar/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteVar,1)
	IDLC_DECL_VAR(Var,aVar)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,aVar)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,WriteVar,1,(aVar))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,WriteVar,1)

/** ni -> iFile::ReadVar/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadVar,0)
	IDLC_DECL_RETVAR(Var,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadVar,0,())
	IDLC_RET_FROM_BASE(ni::eType_Variant,_Ret)
IDLC_METH_END(ni,iFile,ReadVar,0)

/** ni -> iFile::ReadVarEx/1 -> NO AUTOMATION **/
/** ni -> iFile::WriteBitsPackedVar/1 **/
IDLC_METH_BEGIN(ni,iFile,WriteBitsPackedVar,1)
	IDLC_DECL_VAR(Var,aVar)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer,aVar)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,WriteBitsPackedVar,1,(aVar))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFile,WriteBitsPackedVar,1)

/** ni -> iFile::ReadBitsPackedVar/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadBitsPackedVar,0)
	IDLC_DECL_RETVAR(Var,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadBitsPackedVar,0,())
	IDLC_RET_FROM_BASE(ni::eType_Variant,_Ret)
IDLC_METH_END(ni,iFile,ReadBitsPackedVar,0)

/** ni -> iFile::ReadBitsPackedVarEx/1 -> NO AUTOMATION **/
/** ni -> iFile::ReadRawToString/2 **/
IDLC_METH_BEGIN(ni,iFile,ReadRawToString,2)
	IDLC_DECL_VAR(eRawToStringEncoding,aFormat)
	IDLC_BUF_TO_ENUM(eRawToStringEncoding,aFormat)
	IDLC_DECL_VAR(tInt,anNumBytes)
	IDLC_BUF_TO_BASE(ni::eType_Int,anNumBytes)
	IDLC_DECL_RETVAR(cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadRawToString,2,(aFormat,anNumBytes))
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iFile,ReadRawToString,2)

/** ni -> iFile::WriteRawFromString/2 **/
IDLC_METH_BEGIN(ni,iFile,WriteRawFromString,2)
	IDLC_DECL_VAR(eRawToStringEncoding,aFormat)
	IDLC_BUF_TO_ENUM(eRawToStringEncoding,aFormat)
	IDLC_DECL_VAR(achar*,aaszString)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszString)
	IDLC_DECL_RETVAR(tInt,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,WriteRawFromString,2,(aFormat,aaszString))
	IDLC_RET_FROM_BASE(ni::eType_Int,_Ret)
IDLC_METH_END(ni,iFile,WriteRawFromString,2)

/** ni -> iFile::ReadQuotedLine/0 **/
IDLC_METH_BEGIN(ni,iFile,ReadQuotedLine,0)
	IDLC_DECL_RETVAR(ni::cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFile,ReadQuotedLine,0,())
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iFile,ReadQuotedLine,0)

IDLC_END_INTF(ni,iFile)

/** interface : iFileMemory **/
IDLC_BEGIN_INTF(ni,iFileMemory)
/** ni -> iFileMemory::GetBase/0 **/
IDLC_METH_BEGIN(ni,iFileMemory,GetBase,0)
	IDLC_DECL_RETVAR(tPtr,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileMemory,GetBase,0,())
	IDLC_RET_FROM_PTR(ni::eType_Ptr,_Ret)
IDLC_METH_END(ni,iFileMemory,GetBase,0)

/** ni -> iFileMemory::GetHere/0 **/
IDLC_METH_BEGIN(ni,iFileMemory,GetHere,0)
	IDLC_DECL_RETVAR(tPtr,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileMemory,GetHere,0,())
	IDLC_RET_FROM_PTR(ni::eType_Ptr,_Ret)
IDLC_METH_END(ni,iFileMemory,GetHere,0)

/** ni -> iFileMemory::GetStop/0 **/
IDLC_METH_BEGIN(ni,iFileMemory,GetStop,0)
	IDLC_DECL_RETVAR(tPtr,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileMemory,GetStop,0,())
	IDLC_RET_FROM_PTR(ni::eType_Ptr,_Ret)
IDLC_METH_END(ni,iFileMemory,GetStop,0)

/** ni -> iFileMemory::SetMemPtr/4 **/
IDLC_METH_BEGIN(ni,iFileMemory,SetMemPtr,4)
	IDLC_DECL_VAR(tPtr,apMem)
	IDLC_BUF_TO_PTR(ni::eType_Ptr,apMem)
	IDLC_DECL_VAR(tSize,anSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,anSize)
	IDLC_DECL_VAR(tBool,abFree)
	IDLC_BUF_TO_BASE(ni::eType_I8,abFree)
	IDLC_DECL_VAR(tBool,abKeepHere)
	IDLC_BUF_TO_BASE(ni::eType_I8,abKeepHere)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iFileMemory,SetMemPtr,4,(apMem,anSize,abFree,abKeepHere))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iFileMemory,SetMemPtr,4)

/** ni -> iFileMemory::Reset/0 **/
IDLC_METH_BEGIN(ni,iFileMemory,Reset,0)
	IDLC_METH_CALL_VOID(ni,iFileMemory,Reset,0,())
IDLC_METH_END(ni,iFileMemory,Reset,0)

IDLC_END_INTF(ni,iFileMemory)

/** interface : iURLFileHandler **/
IDLC_BEGIN_INTF(ni,iURLFileHandler)
/** ni -> iURLFileHandler::URLOpen/1 **/
IDLC_METH_BEGIN(ni,iURLFileHandler,URLOpen,1)
	IDLC_DECL_VAR(achar*,aURL)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aURL)
	IDLC_DECL_RETVAR(iFile*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iURLFileHandler,URLOpen,1,(aURL))
	IDLC_RET_FROM_INTF(iFile,_Ret)
IDLC_METH_END(ni,iURLFileHandler,URLOpen,1)

/** ni -> iURLFileHandler::URLExists/1 **/
IDLC_METH_BEGIN(ni,iURLFileHandler,URLExists,1)
	IDLC_DECL_VAR(achar*,aURL)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aURL)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iURLFileHandler,URLExists,1,(aURL))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iURLFileHandler,URLExists,1)

IDLC_END_INTF(ni,iURLFileHandler)

/** interface : iBufferEncoder **/
IDLC_BEGIN_INTF(ni,iBufferEncoder)
/** ni -> iBufferEncoder::EncodeMarker/0 **/
IDLC_METH_BEGIN(ni,iBufferEncoder,EncodeMarker,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iBufferEncoder,EncodeMarker,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iBufferEncoder,EncodeMarker,0)

/** ni -> iBufferEncoder::EncodeMaxDestSize/1 **/
IDLC_METH_BEGIN(ni,iBufferEncoder,EncodeMaxDestSize,1)
	IDLC_DECL_VAR(tU32,anSrcSize)
	IDLC_BUF_TO_BASE(ni::eType_U32,anSrcSize)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iBufferEncoder,EncodeMaxDestSize,1,(anSrcSize))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iBufferEncoder,EncodeMaxDestSize,1)

/** ni -> iBufferEncoder::EncodeBuffer/4 **/
IDLC_METH_BEGIN(ni,iBufferEncoder,EncodeBuffer,4)
	IDLC_DECL_VAR(tPtr,apDest)
	IDLC_BUF_TO_PTR(ni::eType_Ptr,apDest)
	IDLC_DECL_VAR(tSize,anDestSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,anDestSize)
	IDLC_DECL_VAR(tPtr,apSrc)
	IDLC_BUF_TO_PTR(ni::eType_Ptr,apSrc)
	IDLC_DECL_VAR(tSize,anSrcSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,anSrcSize)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iBufferEncoder,EncodeBuffer,4,(apDest,anDestSize,apSrc,anSrcSize))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iBufferEncoder,EncodeBuffer,4)

IDLC_END_INTF(ni,iBufferEncoder)

/** interface : iBufferDecoder **/
IDLC_BEGIN_INTF(ni,iBufferDecoder)
/** ni -> iBufferDecoder::DecodeMarker/0 **/
IDLC_METH_BEGIN(ni,iBufferDecoder,DecodeMarker,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iBufferDecoder,DecodeMarker,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iBufferDecoder,DecodeMarker,0)

/** ni -> iBufferDecoder::DecodeMaxDestSize/1 **/
IDLC_METH_BEGIN(ni,iBufferDecoder,DecodeMaxDestSize,1)
	IDLC_DECL_VAR(tU32,anSrcSize)
	IDLC_BUF_TO_BASE(ni::eType_U32,anSrcSize)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iBufferDecoder,DecodeMaxDestSize,1,(anSrcSize))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iBufferDecoder,DecodeMaxDestSize,1)

/** ni -> iBufferDecoder::DecodeBuffer/4 **/
IDLC_METH_BEGIN(ni,iBufferDecoder,DecodeBuffer,4)
	IDLC_DECL_VAR(tPtr,apDest)
	IDLC_BUF_TO_PTR(ni::eType_Ptr,apDest)
	IDLC_DECL_VAR(tSize,anDestSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,anDestSize)
	IDLC_DECL_VAR(tPtr,apSrc)
	IDLC_BUF_TO_PTR(ni::eType_Ptr,apSrc)
	IDLC_DECL_VAR(tSize,anSrcSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,anSrcSize)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iBufferDecoder,DecodeBuffer,4,(apDest,anDestSize,apSrc,anSrcSize))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iBufferDecoder,DecodeBuffer,4)

IDLC_END_INTF(ni,iBufferDecoder)

IDLC_END_NAMESPACE()
// EOF //
