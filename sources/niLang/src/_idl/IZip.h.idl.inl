#include "../API/niLang/IZip.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** NAMESPACE : ni **/
/** interface : iZipArchWrite **/
IDLC_BEGIN_INTF(ni,iZipArchWrite)
/** ni -> iZipArchWrite::AddFileBlock/3 **/
IDLC_METH_BEGIN(ni,iZipArchWrite,AddFileBlock,3)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_VAR(iFile*,apFile)
	IDLC_BUF_TO_INTF(iFile,apFile)
	IDLC_DECL_VAR(tI64,aSize)
	IDLC_BUF_TO_BASE(ni::eType_I64,aSize)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iZipArchWrite,AddFileBlock,3,(aaszName,apFile,aSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iZipArchWrite,AddFileBlock,3)

/** ni -> iZipArchWrite::AddRawBlock/3 -> NO AUTOMATION **/
IDLC_END_INTF(ni,iZipArchWrite)

/** interface : iZip **/
IDLC_BEGIN_INTF(ni,iZip)
/** ni -> iZip::ZipUncompressBuffer/4 **/
IDLC_METH_BEGIN(ni,iZip,ZipUncompressBuffer,4)
	IDLC_DECL_VAR(tPtr,apDest)
	IDLC_BUF_TO_PTR(ni::eType_Ptr,apDest)
	IDLC_DECL_VAR(tU32,anDestSize)
	IDLC_BUF_TO_BASE(ni::eType_U32,anDestSize)
	IDLC_DECL_VAR(tPtr,apSrc)
	IDLC_BUF_TO_PTR(ni::eType_Ptr,apSrc)
	IDLC_DECL_VAR(tU32,anSrcSize)
	IDLC_BUF_TO_BASE(ni::eType_U32,anSrcSize)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iZip,ZipUncompressBuffer,4,(apDest,anDestSize,apSrc,anSrcSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iZip,ZipUncompressBuffer,4)

/** ni -> iZip::ZipUncompressBufferInFile/4 **/
IDLC_METH_BEGIN(ni,iZip,ZipUncompressBufferInFile,4)
	IDLC_DECL_VAR(iFile*,apDest)
	IDLC_BUF_TO_INTF(iFile,apDest)
	IDLC_DECL_VAR(tU32,anDestSize)
	IDLC_BUF_TO_BASE(ni::eType_U32,anDestSize)
	IDLC_DECL_VAR(tPtr,apSrc)
	IDLC_BUF_TO_PTR(ni::eType_Ptr,apSrc)
	IDLC_DECL_VAR(tU32,anSrcSize)
	IDLC_BUF_TO_BASE(ni::eType_U32,anSrcSize)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iZip,ZipUncompressBufferInFile,4,(apDest,anDestSize,apSrc,anSrcSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iZip,ZipUncompressBufferInFile,4)

/** ni -> iZip::ZipUncompressFile/4 **/
IDLC_METH_BEGIN(ni,iZip,ZipUncompressFile,4)
	IDLC_DECL_VAR(iFile*,apDest)
	IDLC_BUF_TO_INTF(iFile,apDest)
	IDLC_DECL_VAR(tU32,anDestSize)
	IDLC_BUF_TO_BASE(ni::eType_U32,anDestSize)
	IDLC_DECL_VAR(iFile*,apSrc)
	IDLC_BUF_TO_INTF(iFile,apSrc)
	IDLC_DECL_VAR(tU32,anSrcSize)
	IDLC_BUF_TO_BASE(ni::eType_U32,anSrcSize)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iZip,ZipUncompressFile,4,(apDest,anDestSize,apSrc,anSrcSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iZip,ZipUncompressFile,4)

/** ni -> iZip::ZipUncompressFileInBuffer/4 **/
IDLC_METH_BEGIN(ni,iZip,ZipUncompressFileInBuffer,4)
	IDLC_DECL_VAR(tPtr,apDest)
	IDLC_BUF_TO_PTR(ni::eType_Ptr,apDest)
	IDLC_DECL_VAR(tU32,anDestSize)
	IDLC_BUF_TO_BASE(ni::eType_U32,anDestSize)
	IDLC_DECL_VAR(iFile*,apSrc)
	IDLC_BUF_TO_INTF(iFile,apSrc)
	IDLC_DECL_VAR(tU32,anSrcSize)
	IDLC_BUF_TO_BASE(ni::eType_U32,anSrcSize)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iZip,ZipUncompressFileInBuffer,4,(apDest,anDestSize,apSrc,anSrcSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iZip,ZipUncompressFileInBuffer,4)

/** ni -> iZip::ZipInflateFileInBuffer/3 **/
IDLC_METH_BEGIN(ni,iZip,ZipInflateFileInBuffer,3)
	IDLC_DECL_VAR(tPtr,apDest)
	IDLC_BUF_TO_PTR(ni::eType_Ptr,apDest)
	IDLC_DECL_VAR(tU32,anDestSize)
	IDLC_BUF_TO_BASE(ni::eType_U32,anDestSize)
	IDLC_DECL_VAR(iFile*,apSrc)
	IDLC_BUF_TO_INTF(iFile,apSrc)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iZip,ZipInflateFileInBuffer,3,(apDest,anDestSize,apSrc))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iZip,ZipInflateFileInBuffer,3)

/** ni -> iZip::ZipCompressBuffer/4 **/
IDLC_METH_BEGIN(ni,iZip,ZipCompressBuffer,4)
	IDLC_DECL_VAR(tPtr,apDest)
	IDLC_BUF_TO_PTR(ni::eType_Ptr,apDest)
	IDLC_DECL_VAR(tPtr,apSrc)
	IDLC_BUF_TO_PTR(ni::eType_Ptr,apSrc)
	IDLC_DECL_VAR(tU32,anSrcSize)
	IDLC_BUF_TO_BASE(ni::eType_U32,anSrcSize)
	IDLC_DECL_VAR(tU32,anLevel)
	IDLC_BUF_TO_BASE(ni::eType_U32,anLevel)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iZip,ZipCompressBuffer,4,(apDest,apSrc,anSrcSize,anLevel))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iZip,ZipCompressBuffer,4)

/** ni -> iZip::ZipCompressBufferInFile/4 **/
IDLC_METH_BEGIN(ni,iZip,ZipCompressBufferInFile,4)
	IDLC_DECL_VAR(iFile*,apDest)
	IDLC_BUF_TO_INTF(iFile,apDest)
	IDLC_DECL_VAR(tPtr,apSrc)
	IDLC_BUF_TO_PTR(ni::eType_Ptr,apSrc)
	IDLC_DECL_VAR(tU32,anSrcSize)
	IDLC_BUF_TO_BASE(ni::eType_U32,anSrcSize)
	IDLC_DECL_VAR(tU32,anLevel)
	IDLC_BUF_TO_BASE(ni::eType_U32,anLevel)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iZip,ZipCompressBufferInFile,4,(apDest,apSrc,anSrcSize,anLevel))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iZip,ZipCompressBufferInFile,4)

/** ni -> iZip::ZipCompressFile/4 **/
IDLC_METH_BEGIN(ni,iZip,ZipCompressFile,4)
	IDLC_DECL_VAR(iFile*,apDest)
	IDLC_BUF_TO_INTF(iFile,apDest)
	IDLC_DECL_VAR(iFile*,apSrc)
	IDLC_BUF_TO_INTF(iFile,apSrc)
	IDLC_DECL_VAR(tU32,anSrcSize)
	IDLC_BUF_TO_BASE(ni::eType_U32,anSrcSize)
	IDLC_DECL_VAR(tU32,anLevel)
	IDLC_BUF_TO_BASE(ni::eType_U32,anLevel)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iZip,ZipCompressFile,4,(apDest,apSrc,anSrcSize,anLevel))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iZip,ZipCompressFile,4)

/** ni -> iZip::UnzOpen/1 -> NO AUTOMATION **/
/** ni -> iZip::UnzClose/1 -> NO AUTOMATION **/
/** ni -> iZip::UnzGetGlobalInfo/2 -> NO AUTOMATION **/
/** ni -> iZip::UnzGetGlobalComment/2 -> NO AUTOMATION **/
/** ni -> iZip::UnzGoToFirstFile/1 -> NO AUTOMATION **/
/** ni -> iZip::UnzGoToNextFile/1 -> NO AUTOMATION **/
/** ni -> iZip::UnzLocateFile/3 -> NO AUTOMATION **/
/** ni -> iZip::UnzLocateFileInc/3 -> NO AUTOMATION **/
/** ni -> iZip::UnzGetCurrentFileInfo/5 -> NO AUTOMATION **/
/** ni -> iZip::UnzOpenCurrentFile/2 -> NO AUTOMATION **/
/** ni -> iZip::UnzCloseCurrentFile/1 -> NO AUTOMATION **/
/** ni -> iZip::UnzReadCurrentFile/3 -> NO AUTOMATION **/
/** ni -> iZip::Unztell/1 -> NO AUTOMATION **/
/** ni -> iZip::Unzeof/1 -> NO AUTOMATION **/
/** ni -> iZip::UnzGetLocalExtrafield/3 -> NO AUTOMATION **/
/** ni -> iZip::ZlibVersion/0 -> NO AUTOMATION **/
/** ni -> iZip::Deflate/2 -> NO AUTOMATION **/
/** ni -> iZip::DeflateEnd/1 -> NO AUTOMATION **/
/** ni -> iZip::Inflate/2 -> NO AUTOMATION **/
/** ni -> iZip::InflateEnd/1 -> NO AUTOMATION **/
/** ni -> iZip::DeflateSetDictionary/3 -> NO AUTOMATION **/
/** ni -> iZip::DeflateCopy/2 -> NO AUTOMATION **/
/** ni -> iZip::DeflateReset/1 -> NO AUTOMATION **/
/** ni -> iZip::DeflateParams/3 -> NO AUTOMATION **/
/** ni -> iZip::DeflateTune/5 -> NO AUTOMATION **/
/** ni -> iZip::DeflateBound/2 -> NO AUTOMATION **/
/** ni -> iZip::DeflatePrime/3 -> NO AUTOMATION **/
/** ni -> iZip::DeflateSetHeader/2 -> NO AUTOMATION **/
/** ni -> iZip::InflateSetDictionary/3 -> NO AUTOMATION **/
/** ni -> iZip::InflateSync/1 -> NO AUTOMATION **/
/** ni -> iZip::InflateCopy/2 -> NO AUTOMATION **/
/** ni -> iZip::InflateReset/1 -> NO AUTOMATION **/
/** ni -> iZip::InflatePrime/3 -> NO AUTOMATION **/
/** ni -> iZip::InflateGetHeader/2 -> NO AUTOMATION **/
/** ni -> iZip::InflateBack/5 -> NO AUTOMATION **/
/** ni -> iZip::InflateBackEnd/1 -> NO AUTOMATION **/
/** ni -> iZip::ZlibCompileFlags/0 -> NO AUTOMATION **/
/** ni -> iZip::Compress/4 -> NO AUTOMATION **/
/** ni -> iZip::Compress2/5 -> NO AUTOMATION **/
/** ni -> iZip::CompressBound/1 -> NO AUTOMATION **/
/** ni -> iZip::Uncompress/4 -> NO AUTOMATION **/
/** ni -> iZip::Adler32/3 -> NO AUTOMATION **/
/** ni -> iZip::Adler32Combine/3 -> NO AUTOMATION **/
/** ni -> iZip::Crc32/3 -> NO AUTOMATION **/
/** ni -> iZip::Crc32Combine/3 -> NO AUTOMATION **/
/** ni -> iZip::DeflateInit/2 -> NO AUTOMATION **/
/** ni -> iZip::InflateInit/1 -> NO AUTOMATION **/
/** ni -> iZip::DeflateInit2/6 -> NO AUTOMATION **/
/** ni -> iZip::InflateInit2/2 -> NO AUTOMATION **/
/** ni -> iZip::InflateBackInit/3 -> NO AUTOMATION **/
/** ni -> iZip::ZError/1 -> NO AUTOMATION **/
/** ni -> iZip::InflateSyncPoint/1 -> NO AUTOMATION **/
/** ni -> iZip::GetCRCTable/0 -> NO AUTOMATION **/
/** ni -> iZip::CreateZipBufferEncoder/1 **/
IDLC_METH_BEGIN(ni,iZip,CreateZipBufferEncoder,1)
	IDLC_DECL_VAR(tU32,anCLevel)
	IDLC_BUF_TO_BASE(ni::eType_U32,anCLevel)
	IDLC_DECL_RETVAR(iBufferEncoder*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iZip,CreateZipBufferEncoder,1,(anCLevel))
	IDLC_RET_FROM_INTF(iBufferEncoder,_Ret)
IDLC_METH_END(ni,iZip,CreateZipBufferEncoder,1)

/** ni -> iZip::CreateZipBufferDecoder/0 **/
IDLC_METH_BEGIN(ni,iZip,CreateZipBufferDecoder,0)
	IDLC_DECL_RETVAR(iBufferDecoder*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iZip,CreateZipBufferDecoder,0,())
	IDLC_RET_FROM_INTF(iBufferDecoder,_Ret)
IDLC_METH_END(ni,iZip,CreateZipBufferDecoder,0)

/** ni -> iZip::CreateFileZipBufferEncoder/2 **/
IDLC_METH_BEGIN(ni,iZip,CreateFileZipBufferEncoder,2)
	IDLC_DECL_VAR(iFileBase*,apBaseFile)
	IDLC_BUF_TO_INTF(iFileBase,apBaseFile)
	IDLC_DECL_VAR(tU32,anCLevel)
	IDLC_BUF_TO_BASE(ni::eType_U32,anCLevel)
	IDLC_DECL_RETVAR(iFile*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iZip,CreateFileZipBufferEncoder,2,(apBaseFile,anCLevel))
	IDLC_RET_FROM_INTF(iFile,_Ret)
IDLC_METH_END(ni,iZip,CreateFileZipBufferEncoder,2)

/** ni -> iZip::CreateFileZipBufferDecoder/2 **/
IDLC_METH_BEGIN(ni,iZip,CreateFileZipBufferDecoder,2)
	IDLC_DECL_VAR(iFileBase*,apBaseFile)
	IDLC_BUF_TO_INTF(iFileBase,apBaseFile)
	IDLC_DECL_VAR(tSize,aDecodedSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,aDecodedSize)
	IDLC_DECL_RETVAR(iFile*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iZip,CreateFileZipBufferDecoder,2,(apBaseFile,aDecodedSize))
	IDLC_RET_FROM_INTF(iFile,_Ret)
IDLC_METH_END(ni,iZip,CreateFileZipBufferDecoder,2)

/** ni -> iZip::CreateRawBufferEncoder/0 **/
IDLC_METH_BEGIN(ni,iZip,CreateRawBufferEncoder,0)
	IDLC_DECL_RETVAR(iBufferEncoder*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iZip,CreateRawBufferEncoder,0,())
	IDLC_RET_FROM_INTF(iBufferEncoder,_Ret)
IDLC_METH_END(ni,iZip,CreateRawBufferEncoder,0)

/** ni -> iZip::CreateRawBufferDecoder/0 **/
IDLC_METH_BEGIN(ni,iZip,CreateRawBufferDecoder,0)
	IDLC_DECL_RETVAR(iBufferDecoder*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iZip,CreateRawBufferDecoder,0,())
	IDLC_RET_FROM_INTF(iBufferDecoder,_Ret)
IDLC_METH_END(ni,iZip,CreateRawBufferDecoder,0)

/** ni -> iZip::CreateFileRawBufferEncoder/1 **/
IDLC_METH_BEGIN(ni,iZip,CreateFileRawBufferEncoder,1)
	IDLC_DECL_VAR(iFileBase*,apBaseFile)
	IDLC_BUF_TO_INTF(iFileBase,apBaseFile)
	IDLC_DECL_RETVAR(iFile*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iZip,CreateFileRawBufferEncoder,1,(apBaseFile))
	IDLC_RET_FROM_INTF(iFile,_Ret)
IDLC_METH_END(ni,iZip,CreateFileRawBufferEncoder,1)

/** ni -> iZip::CreateFileRawBufferDecoder/2 **/
IDLC_METH_BEGIN(ni,iZip,CreateFileRawBufferDecoder,2)
	IDLC_DECL_VAR(iFileBase*,apBaseFile)
	IDLC_BUF_TO_INTF(iFileBase,apBaseFile)
	IDLC_DECL_VAR(tSize,aDecodedSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,aDecodedSize)
	IDLC_DECL_RETVAR(iFile*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iZip,CreateFileRawBufferDecoder,2,(apBaseFile,aDecodedSize))
	IDLC_RET_FROM_INTF(iFile,_Ret)
IDLC_METH_END(ni,iZip,CreateFileRawBufferDecoder,2)

/** ni -> iZip::CreateZipArchive/3 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iZip::CreateZipArchive/3 **/
IDLC_METH_BEGIN(ni,iZip,CreateZipArchive,3)
	IDLC_DECL_VAR(iFile*,apDest)
	IDLC_BUF_TO_INTF(iFile,apDest)
	IDLC_DECL_VAR(achar*,aaszPwd)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszPwd)
	IDLC_DECL_VAR(tStringCVec*,apStoredExtensions)
	IDLC_BUF_TO_INTF(tStringCVec,apStoredExtensions)
	IDLC_DECL_RETVAR(iZipArchWrite*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iZip,CreateZipArchive,3,(apDest,aaszPwd,apStoredExtensions))
	IDLC_RET_FROM_INTF(iZipArchWrite,_Ret)
IDLC_METH_END(ni,iZip,CreateZipArchive,3)
#endif // niMinFeatures(15)

/** ni -> iZip::GZipOpen/2 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iZip::GZipOpen/2 **/
IDLC_METH_BEGIN(ni,iZip,GZipOpen,2)
	IDLC_DECL_VAR(iFileBase*,apFile)
	IDLC_BUF_TO_INTF(iFileBase,apFile)
	IDLC_DECL_VAR(tU32,aulCompressionMode)
	IDLC_BUF_TO_BASE(ni::eType_U32,aulCompressionMode)
	IDLC_DECL_RETVAR(iFile*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iZip,GZipOpen,2,(apFile,aulCompressionMode))
	IDLC_RET_FROM_INTF(iFile,_Ret)
IDLC_METH_END(ni,iZip,GZipOpen,2)
#endif // niMinFeatures(15)

/** ni -> iZip::ZipOpen/2 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iZip::ZipOpen/2 **/
IDLC_METH_BEGIN(ni,iZip,ZipOpen,2)
	IDLC_DECL_VAR(iFileBase*,apFile)
	IDLC_BUF_TO_INTF(iFileBase,apFile)
	IDLC_DECL_VAR(tU32,aulCompressionMode)
	IDLC_BUF_TO_BASE(ni::eType_U32,aulCompressionMode)
	IDLC_DECL_RETVAR(iFile*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iZip,ZipOpen,2,(apFile,aulCompressionMode))
	IDLC_RET_FROM_INTF(iFile,_Ret)
IDLC_METH_END(ni,iZip,ZipOpen,2)
#endif // niMinFeatures(15)

IDLC_END_INTF(ni,iZip)

IDLC_END_NAMESPACE()
// EOF //
