// clang-format off
//
// Autogenerated IDL definition for 'API/niUI/IJpegWriter.h'.
//
#include "../API/niUI/IJpegWriter.h"

#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif

IDLC_BEGIN_NAMESPACE()

/** NAMESPACE : ni **/
/** interface : iJpegWriter **/
IDLC_BEGIN_INTF(ni,iJpegWriter)
/** ni -> iJpegWriter::BeginWrite/7 **/
IDLC_METH_BEGIN(ni,iJpegWriter,BeginWrite,7)
	IDLC_DECL_VAR(iFile*,apDest)
	IDLC_BUF_TO_INTF(iFile,apDest)
	IDLC_DECL_VAR(tU32,anWidth)
	IDLC_BUF_TO_BASE(ni::eType_U32,anWidth)
	IDLC_DECL_VAR(tU32,anHeight)
	IDLC_BUF_TO_BASE(ni::eType_U32,anHeight)
	IDLC_DECL_VAR(eColorSpace,aInCS)
	IDLC_BUF_TO_ENUM(eColorSpace,aInCS)
	IDLC_DECL_VAR(tU32,anC)
	IDLC_BUF_TO_BASE(ni::eType_U32,anC)
	IDLC_DECL_VAR(tU32,anQuality)
	IDLC_BUF_TO_BASE(ni::eType_U32,anQuality)
	IDLC_DECL_VAR(tJpegWriteFlags,aFlags)
	IDLC_BUF_TO_ENUM(tJpegWriteFlags,aFlags)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iJpegWriter,BeginWrite,7,(apDest,anWidth,anHeight,aInCS,anC,anQuality,aFlags))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iJpegWriter,BeginWrite,7)

/** ni -> iJpegWriter::EndWrite/0 **/
IDLC_METH_BEGIN(ni,iJpegWriter,EndWrite,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iJpegWriter,EndWrite,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iJpegWriter,EndWrite,0)

/** ni -> iJpegWriter::WriteScanline/1 **/
IDLC_METH_BEGIN(ni,iJpegWriter,WriteScanline,1)
	IDLC_DECL_VAR(iFile*,apSrc)
	IDLC_BUF_TO_INTF(iFile,apSrc)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iJpegWriter,WriteScanline,1,(apSrc))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iJpegWriter,WriteScanline,1)

/** ni -> iJpegWriter::WriteBitmap/4 **/
IDLC_METH_BEGIN(ni,iJpegWriter,WriteBitmap,4)
	IDLC_DECL_VAR(iFile*,apDest)
	IDLC_BUF_TO_INTF(iFile,apDest)
	IDLC_DECL_VAR(iBitmap2D*,apBmp)
	IDLC_BUF_TO_INTF(iBitmap2D,apBmp)
	IDLC_DECL_VAR(tU32,anQuality)
	IDLC_BUF_TO_BASE(ni::eType_U32,anQuality)
	IDLC_DECL_VAR(tJpegWriteFlags,aFlags)
	IDLC_BUF_TO_ENUM(tJpegWriteFlags,aFlags)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iJpegWriter,WriteBitmap,4,(apDest,apBmp,anQuality,aFlags))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iJpegWriter,WriteBitmap,4)

IDLC_END_INTF(ni,iJpegWriter)

IDLC_END_NAMESPACE()
// EOF //
