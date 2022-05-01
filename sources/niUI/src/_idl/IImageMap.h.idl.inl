#include "../API/niUI/IImageMap.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iImageMap **/
IDLC_BEGIN_INTF(ni,iImageMap)
/** ni -> iImageMap::SetMaxNumPages/1 **/
IDLC_METH_BEGIN(ni,iImageMap,SetMaxNumPages,1)
	IDLC_DECL_VAR(tU32,anMax)
	IDLC_BUF_TO_BASE(ni::eType_U32,anMax)
	IDLC_METH_CALL_VOID(ni,iImageMap,SetMaxNumPages,1,(anMax))
IDLC_METH_END(ni,iImageMap,SetMaxNumPages,1)

/** ni -> iImageMap::GetMaxNumPages/0 **/
IDLC_METH_BEGIN(ni,iImageMap,GetMaxNumPages,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImageMap,GetMaxNumPages,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iImageMap,GetMaxNumPages,0)

/** ni -> iImageMap::SetPageSize/1 **/
IDLC_METH_BEGIN(ni,iImageMap,SetPageSize,1)
	IDLC_DECL_VAR(tU32,anSize)
	IDLC_BUF_TO_BASE(ni::eType_U32,anSize)
	IDLC_METH_CALL_VOID(ni,iImageMap,SetPageSize,1,(anSize))
IDLC_METH_END(ni,iImageMap,SetPageSize,1)

/** ni -> iImageMap::GetPageSize/0 **/
IDLC_METH_BEGIN(ni,iImageMap,GetPageSize,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImageMap,GetPageSize,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iImageMap,GetPageSize,0)

/** ni -> iImageMap::SetPageMipMaps/1 **/
IDLC_METH_BEGIN(ni,iImageMap,SetPageMipMaps,1)
	IDLC_DECL_VAR(tU32,anNumMipMaps)
	IDLC_BUF_TO_BASE(ni::eType_U32,anNumMipMaps)
	IDLC_METH_CALL_VOID(ni,iImageMap,SetPageMipMaps,1,(anNumMipMaps))
IDLC_METH_END(ni,iImageMap,SetPageMipMaps,1)

/** ni -> iImageMap::GetPageMipMaps/0 **/
IDLC_METH_BEGIN(ni,iImageMap,GetPageMipMaps,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImageMap,GetPageMipMaps,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iImageMap,GetPageMipMaps,0)

/** ni -> iImageMap::SetComputeMipMapsPerPage/1 **/
IDLC_METH_BEGIN(ni,iImageMap,SetComputeMipMapsPerPage,1)
	IDLC_DECL_VAR(tBool,abComputeMipMapsPerPage)
	IDLC_BUF_TO_BASE(ni::eType_I8,abComputeMipMapsPerPage)
	IDLC_METH_CALL_VOID(ni,iImageMap,SetComputeMipMapsPerPage,1,(abComputeMipMapsPerPage))
IDLC_METH_END(ni,iImageMap,SetComputeMipMapsPerPage,1)

/** ni -> iImageMap::GetComputeMipMapsPerPage/0 **/
IDLC_METH_BEGIN(ni,iImageMap,GetComputeMipMapsPerPage,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImageMap,GetComputeMipMapsPerPage,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iImageMap,GetComputeMipMapsPerPage,0)

/** ni -> iImageMap::SetPageFormat/1 **/
IDLC_METH_BEGIN(ni,iImageMap,SetPageFormat,1)
	IDLC_DECL_VAR(iHString*,ahspFormat)
	IDLC_BUF_TO_INTF(iHString,ahspFormat)
	IDLC_METH_CALL_VOID(ni,iImageMap,SetPageFormat,1,(ahspFormat))
IDLC_METH_END(ni,iImageMap,SetPageFormat,1)

/** ni -> iImageMap::GetPageFormat/0 **/
IDLC_METH_BEGIN(ni,iImageMap,GetPageFormat,0)
	IDLC_DECL_RETVAR(iHString*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImageMap,GetPageFormat,0,())
	IDLC_RET_FROM_INTF(iHString,_Ret)
IDLC_METH_END(ni,iImageMap,GetPageFormat,0)

/** ni -> iImageMap::GetNumPages/0 **/
IDLC_METH_BEGIN(ni,iImageMap,GetNumPages,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImageMap,GetNumPages,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iImageMap,GetNumPages,0)

/** ni -> iImageMap::GetPage/1 **/
IDLC_METH_BEGIN(ni,iImageMap,GetPage,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(iTexture*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImageMap,GetPage,1,(anIndex))
	IDLC_RET_FROM_INTF(iTexture,_Ret)
IDLC_METH_END(ni,iImageMap,GetPage,1)

/** ni -> iImageMap::Clear/0 **/
IDLC_METH_BEGIN(ni,iImageMap,Clear,0)
	IDLC_METH_CALL_VOID(ni,iImageMap,Clear,0,())
IDLC_METH_END(ni,iImageMap,Clear,0)

/** ni -> iImageMap::SetDefaultImageBlendMode/1 **/
IDLC_METH_BEGIN(ni,iImageMap,SetDefaultImageBlendMode,1)
	IDLC_DECL_VAR(eBlendMode,aMode)
	IDLC_BUF_TO_ENUM(eBlendMode,aMode)
	IDLC_METH_CALL_VOID(ni,iImageMap,SetDefaultImageBlendMode,1,(aMode))
IDLC_METH_END(ni,iImageMap,SetDefaultImageBlendMode,1)

/** ni -> iImageMap::GetDefaultImageBlendMode/0 **/
IDLC_METH_BEGIN(ni,iImageMap,GetDefaultImageBlendMode,0)
	IDLC_DECL_RETVAR(eBlendMode,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImageMap,GetDefaultImageBlendMode,0,())
	IDLC_RET_FROM_ENUM(eBlendMode,_Ret)
IDLC_METH_END(ni,iImageMap,GetDefaultImageBlendMode,0)

/** ni -> iImageMap::SetDefaultImageFilter/1 **/
IDLC_METH_BEGIN(ni,iImageMap,SetDefaultImageFilter,1)
	IDLC_DECL_VAR(tBool,abFiltering)
	IDLC_BUF_TO_BASE(ni::eType_I8,abFiltering)
	IDLC_METH_CALL_VOID(ni,iImageMap,SetDefaultImageFilter,1,(abFiltering))
IDLC_METH_END(ni,iImageMap,SetDefaultImageFilter,1)

/** ni -> iImageMap::GetDefaultImageFilter/0 **/
IDLC_METH_BEGIN(ni,iImageMap,GetDefaultImageFilter,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImageMap,GetDefaultImageFilter,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iImageMap,GetDefaultImageFilter,0)

/** ni -> iImageMap::GetNumImages/0 **/
IDLC_METH_BEGIN(ni,iImageMap,GetNumImages,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImageMap,GetNumImages,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iImageMap,GetNumImages,0)

/** ni -> iImageMap::GetImage/1 **/
IDLC_METH_BEGIN(ni,iImageMap,GetImage,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(iOverlay*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImageMap,GetImage,1,(anIndex))
	IDLC_RET_FROM_INTF(iOverlay,_Ret)
IDLC_METH_END(ni,iImageMap,GetImage,1)

/** ni -> iImageMap::GetImageIndex/1 **/
IDLC_METH_BEGIN(ni,iImageMap,GetImageIndex,1)
	IDLC_DECL_VAR(iOverlay*,apImage)
	IDLC_BUF_TO_INTF(iOverlay,apImage)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImageMap,GetImageIndex,1,(apImage))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iImageMap,GetImageIndex,1)

/** ni -> iImageMap::GetImageFromName/1 **/
IDLC_METH_BEGIN(ni,iImageMap,GetImageFromName,1)
	IDLC_DECL_VAR(iHString*,ahspName)
	IDLC_BUF_TO_INTF(iHString,ahspName)
	IDLC_DECL_RETVAR(iOverlay*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImageMap,GetImageFromName,1,(ahspName))
	IDLC_RET_FROM_INTF(iOverlay,_Ret)
IDLC_METH_END(ni,iImageMap,GetImageFromName,1)

/** ni -> iImageMap::AddImage/2 **/
IDLC_METH_BEGIN(ni,iImageMap,AddImage,2)
	IDLC_DECL_VAR(iHString*,ahspName)
	IDLC_BUF_TO_INTF(iHString,ahspName)
	IDLC_DECL_VAR(iBitmap2D*,apBitmap)
	IDLC_BUF_TO_INTF(iBitmap2D,apBitmap)
	IDLC_DECL_RETVAR(iOverlay*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImageMap,AddImage,2,(ahspName,apBitmap))
	IDLC_RET_FROM_INTF(iOverlay,_Ret)
IDLC_METH_END(ni,iImageMap,AddImage,2)

/** ni -> iImageMap::AddImageFromResource/2 **/
IDLC_METH_BEGIN(ni,iImageMap,AddImageFromResource,2)
	IDLC_DECL_VAR(iHString*,ahspName)
	IDLC_BUF_TO_INTF(iHString,ahspName)
	IDLC_DECL_VAR(iHString*,ahspRes)
	IDLC_BUF_TO_INTF(iHString,ahspRes)
	IDLC_DECL_RETVAR(iOverlay*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImageMap,AddImageFromResource,2,(ahspName,ahspRes))
	IDLC_RET_FROM_INTF(iOverlay,_Ret)
IDLC_METH_END(ni,iImageMap,AddImageFromResource,2)

/** ni -> iImageMap::AddImageFromIconSet/5 **/
IDLC_METH_BEGIN(ni,iImageMap,AddImageFromIconSet,5)
	IDLC_DECL_VAR(iHString*,ahspName)
	IDLC_BUF_TO_INTF(iHString,ahspName)
	IDLC_DECL_VAR(iHString*,ahspFolder)
	IDLC_BUF_TO_INTF(iHString,ahspFolder)
	IDLC_DECL_VAR(iHString*,ahspRes)
	IDLC_BUF_TO_INTF(iHString,ahspRes)
	IDLC_DECL_VAR(tU32,anMaxRes)
	IDLC_BUF_TO_BASE(ni::eType_U32,anMaxRes)
	IDLC_DECL_VAR(tU32,anMinRes)
	IDLC_BUF_TO_BASE(ni::eType_U32,anMinRes)
	IDLC_DECL_RETVAR(iOverlay*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImageMap,AddImageFromIconSet,5,(ahspName,ahspFolder,ahspRes,anMaxRes,anMinRes))
	IDLC_RET_FROM_INTF(iOverlay,_Ret)
IDLC_METH_END(ni,iImageMap,AddImageFromIconSet,5)

/** ni -> iImageMap::RemoveImage/1 **/
IDLC_METH_BEGIN(ni,iImageMap,RemoveImage,1)
	IDLC_DECL_VAR(iOverlay*,apImage)
	IDLC_BUF_TO_INTF(iOverlay,apImage)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImageMap,RemoveImage,1,(apImage))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iImageMap,RemoveImage,1)

/** ni -> iImageMap::GetShouldSerialize/0 **/
IDLC_METH_BEGIN(ni,iImageMap,GetShouldSerialize,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImageMap,GetShouldSerialize,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iImageMap,GetShouldSerialize,0)

/** ni -> iImageMap::Serialize/2 **/
IDLC_METH_BEGIN(ni,iImageMap,Serialize,2)
	IDLC_DECL_VAR(ni::iFile*,apFile)
	IDLC_BUF_TO_INTF(ni::iFile,apFile)
	IDLC_DECL_VAR(tImageMapSerializeFlags,aFlags)
	IDLC_BUF_TO_ENUM(tImageMapSerializeFlags,aFlags)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iImageMap,Serialize,2,(apFile,aFlags))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iImageMap,Serialize,2)

IDLC_END_INTF(ni,iImageMap)

IDLC_END_NAMESPACE()
// EOF //
