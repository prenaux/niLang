#include "../API/niUI/IBitmapCube.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iBitmapCube **/
IDLC_BEGIN_INTF(ni,iBitmapCube)
/** ni -> iBitmapCube::GetFace/1 **/
IDLC_METH_BEGIN(ni,iBitmapCube,GetFace,1)
	IDLC_DECL_VAR(eBitmapCubeFace,Face)
	IDLC_BUF_TO_ENUM(eBitmapCubeFace,Face)
	IDLC_DECL_RETVAR(iBitmap2D*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iBitmapCube,GetFace,1,(Face))
	IDLC_RET_FROM_INTF(iBitmap2D,_Ret)
IDLC_METH_END(ni,iBitmapCube,GetFace,1)

/** ni -> iBitmapCube::CreateResized/1 **/
IDLC_METH_BEGIN(ni,iBitmapCube,CreateResized,1)
	IDLC_DECL_VAR(tI32,nW)
	IDLC_BUF_TO_BASE(ni::eType_I32,nW)
	IDLC_DECL_RETVAR(iBitmapCube*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iBitmapCube,CreateResized,1,(nW))
	IDLC_RET_FROM_INTF(iBitmapCube,_Ret)
IDLC_METH_END(ni,iBitmapCube,CreateResized,1)

IDLC_END_INTF(ni,iBitmapCube)

IDLC_END_NAMESPACE()
// EOF //
