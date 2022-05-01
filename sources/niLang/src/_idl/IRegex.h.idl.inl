#include "../API/niLang/IRegex.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iRegex **/
IDLC_BEGIN_INTF(ni,iRegex)
/** ni -> iRegex::GetImplType/0 **/
IDLC_METH_BEGIN(ni,iRegex,GetImplType,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iRegex,GetImplType,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iRegex,GetImplType,0)

/** ni -> iRegex::DoesMatch/1 **/
IDLC_METH_BEGIN(ni,iRegex,DoesMatch,1)
	IDLC_DECL_VAR(achar*,aaszText)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszText)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iRegex,DoesMatch,1,(aaszText))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iRegex,DoesMatch,1)

IDLC_END_INTF(ni,iRegex)

/** interface : iPCRE **/
/** iPCRE -> MIN FEATURES '15' **/
#if niMinFeatures(15)
IDLC_BEGIN_INTF(ni,iPCRE)
/** ni -> iPCRE::Compile/2 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iPCRE::Compile/2 **/
IDLC_METH_BEGIN(ni,iPCRE,Compile,2)
	IDLC_DECL_VAR(achar*,aaszRegEx)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszRegEx)
	IDLC_DECL_VAR(achar*,aaszOpt)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszOpt)
	IDLC_DECL_RETVAR(ePCREError,_Ret)
	IDLC_METH_CALL(_Ret,ni,iPCRE,Compile,2,(aaszRegEx,aaszOpt))
	IDLC_RET_FROM_ENUM(ePCREError,_Ret)
IDLC_METH_END(ni,iPCRE,Compile,2)
#endif // niMinFeatures(15)

/** ni -> iPCRE::Compile2/2 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iPCRE::Compile2/2 **/
IDLC_METH_BEGIN(ni,iPCRE,Compile2,2)
	IDLC_DECL_VAR(achar*,aaszRegEx)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszRegEx)
	IDLC_DECL_VAR(tPCREOptionsFlags,aOpt)
	IDLC_BUF_TO_ENUM(tPCREOptionsFlags,aOpt)
	IDLC_DECL_RETVAR(ePCREError,_Ret)
	IDLC_METH_CALL(_Ret,ni,iPCRE,Compile2,2,(aaszRegEx,aOpt))
	IDLC_RET_FROM_ENUM(ePCREError,_Ret)
IDLC_METH_END(ni,iPCRE,Compile2,2)
#endif // niMinFeatures(15)

/** ni -> iPCRE::GetIsCompiled/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iPCRE::GetIsCompiled/0 **/
IDLC_METH_BEGIN(ni,iPCRE,GetIsCompiled,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iPCRE,GetIsCompiled,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iPCRE,GetIsCompiled,0)
#endif // niMinFeatures(15)

/** ni -> iPCRE::GetLastCompileError/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iPCRE::GetLastCompileError/0 **/
IDLC_METH_BEGIN(ni,iPCRE,GetLastCompileError,0)
	IDLC_DECL_RETVAR(ePCREError,_Ret)
	IDLC_METH_CALL(_Ret,ni,iPCRE,GetLastCompileError,0,())
	IDLC_RET_FROM_ENUM(ePCREError,_Ret)
IDLC_METH_END(ni,iPCRE,GetLastCompileError,0)
#endif // niMinFeatures(15)

/** ni -> iPCRE::GetLastCompileErrorDesc/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iPCRE::GetLastCompileErrorDesc/0 **/
IDLC_METH_BEGIN(ni,iPCRE,GetLastCompileErrorDesc,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iPCRE,GetLastCompileErrorDesc,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iPCRE,GetLastCompileErrorDesc,0)
#endif // niMinFeatures(15)

/** ni -> iPCRE::GetLastCompileErrorOffset/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iPCRE::GetLastCompileErrorOffset/0 **/
IDLC_METH_BEGIN(ni,iPCRE,GetLastCompileErrorOffset,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iPCRE,GetLastCompileErrorOffset,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iPCRE,GetLastCompileErrorOffset,0)
#endif // niMinFeatures(15)

/** ni -> iPCRE::GetOptions/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iPCRE::GetOptions/0 **/
IDLC_METH_BEGIN(ni,iPCRE,GetOptions,0)
	IDLC_DECL_RETVAR(tPCREOptionsFlags,_Ret)
	IDLC_METH_CALL(_Ret,ni,iPCRE,GetOptions,0,())
	IDLC_RET_FROM_ENUM(tPCREOptionsFlags,_Ret)
IDLC_METH_END(ni,iPCRE,GetOptions,0)
#endif // niMinFeatures(15)

/** ni -> iPCRE::Reset/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iPCRE::Reset/0 **/
IDLC_METH_BEGIN(ni,iPCRE,Reset,0)
	IDLC_METH_CALL_VOID(ni,iPCRE,Reset,0,())
IDLC_METH_END(ni,iPCRE,Reset,0)
#endif // niMinFeatures(15)

/** ni -> iPCRE::GetNumMarkers/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iPCRE::GetNumMarkers/0 **/
IDLC_METH_BEGIN(ni,iPCRE,GetNumMarkers,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iPCRE,GetNumMarkers,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iPCRE,GetNumMarkers,0)
#endif // niMinFeatures(15)

/** ni -> iPCRE::GetMarker/1 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iPCRE::GetMarker/1 **/
IDLC_METH_BEGIN(ni,iPCRE,GetMarker,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(sVec2i,_Ret)
	IDLC_METH_CALL(_Ret,ni,iPCRE,GetMarker,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_Vec2i,_Ret)
IDLC_METH_END(ni,iPCRE,GetMarker,1)
#endif // niMinFeatures(15)

/** ni -> iPCRE::GetString/1 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iPCRE::GetString/1 **/
IDLC_METH_BEGIN(ni,iPCRE,GetString,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iPCRE,GetString,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iPCRE,GetString,1)
#endif // niMinFeatures(15)

/** ni -> iPCRE::GetNumNamed/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iPCRE::GetNumNamed/0 **/
IDLC_METH_BEGIN(ni,iPCRE,GetNumNamed,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iPCRE,GetNumNamed,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iPCRE,GetNumNamed,0)
#endif // niMinFeatures(15)

/** ni -> iPCRE::GetNamedName/1 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iPCRE::GetNamedName/1 **/
IDLC_METH_BEGIN(ni,iPCRE,GetNamedName,1)
	IDLC_DECL_VAR(tU32,anIndex)
	IDLC_BUF_TO_BASE(ni::eType_U32,anIndex)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iPCRE,GetNamedName,1,(anIndex))
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iPCRE,GetNamedName,1)
#endif // niMinFeatures(15)

/** ni -> iPCRE::GetNamedMarker/1 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iPCRE::GetNamedMarker/1 **/
IDLC_METH_BEGIN(ni,iPCRE,GetNamedMarker,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(sVec2i,_Ret)
	IDLC_METH_CALL(_Ret,ni,iPCRE,GetNamedMarker,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_Vec2i,_Ret)
IDLC_METH_END(ni,iPCRE,GetNamedMarker,1)
#endif // niMinFeatures(15)

/** ni -> iPCRE::GetNamedString/1 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iPCRE::GetNamedString/1 **/
IDLC_METH_BEGIN(ni,iPCRE,GetNamedString,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iPCRE,GetNamedString,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iPCRE,GetNamedString,1)
#endif // niMinFeatures(15)

/** ni -> iPCRE::GetNamedIndex/1 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iPCRE::GetNamedIndex/1 **/
IDLC_METH_BEGIN(ni,iPCRE,GetNamedIndex,1)
	IDLC_DECL_VAR(achar*,aaszName)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszName)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iPCRE,GetNamedIndex,1,(aaszName))
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iPCRE,GetNamedIndex,1)
#endif // niMinFeatures(15)

/** ni -> iPCRE::MatchRaw/1 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iPCRE::MatchRaw/1 **/
IDLC_METH_BEGIN(ni,iPCRE,MatchRaw,1)
	IDLC_DECL_VAR(achar*,aaszString)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszString)
	IDLC_DECL_RETVAR(tI32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iPCRE,MatchRaw,1,(aaszString))
	IDLC_RET_FROM_BASE(ni::eType_I32,_Ret)
IDLC_METH_END(ni,iPCRE,MatchRaw,1)
#endif // niMinFeatures(15)

/** ni -> iPCRE::Match/2 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iPCRE::Match/2 **/
IDLC_METH_BEGIN(ni,iPCRE,Match,2)
	IDLC_DECL_VAR(iHString*,ahspString)
	IDLC_BUF_TO_INTF(iHString,ahspString)
	IDLC_DECL_VAR(tU32,anOffset)
	IDLC_BUF_TO_BASE(ni::eType_U32,anOffset)
	IDLC_DECL_RETVAR(tI32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iPCRE,Match,2,(ahspString,anOffset))
	IDLC_RET_FROM_BASE(ni::eType_I32,_Ret)
IDLC_METH_END(ni,iPCRE,Match,2)
#endif // niMinFeatures(15)

/** ni -> iPCRE::Split/2 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iPCRE::Split/2 **/
IDLC_METH_BEGIN(ni,iPCRE,Split,2)
	IDLC_DECL_VAR(achar*,aaszString)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszString)
	IDLC_DECL_VAR(tI32,anMaxFields)
	IDLC_BUF_TO_BASE(ni::eType_I32,anMaxFields)
	IDLC_DECL_RETVAR(tI32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iPCRE,Split,2,(aaszString,anMaxFields))
	IDLC_RET_FROM_BASE(ni::eType_I32,_Ret)
IDLC_METH_END(ni,iPCRE,Split,2)
#endif // niMinFeatures(15)

/** ni -> iPCRE::Sub/3 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iPCRE::Sub/3 **/
IDLC_METH_BEGIN(ni,iPCRE,Sub,3)
	IDLC_DECL_VAR(achar*,aaszString)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszString)
	IDLC_DECL_VAR(achar*,aaszReplacement)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszReplacement)
	IDLC_DECL_VAR(tBool,abDoDollarSub)
	IDLC_BUF_TO_BASE(ni::eType_I8,abDoDollarSub)
	IDLC_DECL_RETVAR(cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iPCRE,Sub,3,(aaszString,aaszReplacement,abDoDollarSub))
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iPCRE,Sub,3)
#endif // niMinFeatures(15)

IDLC_END_INTF(ni,iPCRE)

#endif // if niMinFeatures(15)
IDLC_END_NAMESPACE()
// EOF //
