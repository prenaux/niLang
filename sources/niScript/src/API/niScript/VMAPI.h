#ifndef __VMAPI_H_D91B3BAB_01A1_4C38_B23F_FE82D00FA955__
#define __VMAPI_H_D91B3BAB_01A1_4C38_B23F_FE82D00FA955__

#include <niLang/Types.h>

#if niMinFeatures(15)
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_VMAPI
 * @{
 */

#include <niLang/Var.h>
#include <niLang/ObjModel.h>

namespace ni {
struct iScriptObject;
}

typedef ni::tF64  SQFloat;
typedef ni::tInt  SQInt;
typedef ni::tInt  SQRESULT;
typedef ni::tUInt SQUnsignedInteger;
typedef ni::tBool SQBool;
typedef ni::tI32  SQInt32;
typedef ni::tPtr  SQLargestMember;

#define ZERO_LARGESTMEMBER(M) M = 0

struct SQVM;
struct SQTable;
struct SQArray;
struct SQClosure;
struct SQNativeClosure;
struct SQUserData;
struct SQFunctionProto;
struct SQCollectable;
struct SQUserData;
struct SQSharedState;

typedef ni::achar SQChar;

#define SQUIRREL_EOB 0
#define SQ_BYTECODE_STREAM_TAG  (('A'<<8)|'Q')

#define SQOBJECT_COLLECTABLE  ni::eTypeFlags_Collectable
#define SQOBJECT_NUMERIC    ni::eTypeFlags_Numeric
typedef unsigned int SQObjectType;

enum eRawType {
  _RT_NULL          = ni::eType_Null,
  _RT_INTEGER       = ni::eType_I32,
  _RT_FLOAT         = ni::eType_F64,
  _RT_STRING        = ni::eType_IUnknownPtr|ni::eTypeFlags_STRING,
  _RT_TABLE         = ni::eType_IUnknownPtr|ni::eTypeFlags_TABLE,
  _RT_ARRAY         = ni::eType_IUnknownPtr|ni::eTypeFlags_ARRAY,
  _RT_USERDATA      = ni::eType_IUnknownPtr|ni::eTypeFlags_USERDATA,
  _RT_CLOSURE       = ni::eType_IUnknownPtr|ni::eTypeFlags_CLOSURE,
  _RT_NATIVECLOSURE = ni::eType_IUnknownPtr|ni::eTypeFlags_NATIVECLOSURE,
  _RT_FUNCPROTO     = ni::eType_IUnknownPtr|ni::eTypeFlags_FUNCPROTO,
  _RT_IUNKNOWN      = ni::eType_IUnknownPtr,
};

enum eObjectType {
  OT_NULL          = _RT_NULL,
  OT_INTEGER       = (_RT_INTEGER|SQOBJECT_NUMERIC),
  OT_FLOAT         = (_RT_FLOAT|SQOBJECT_NUMERIC),
  OT_STRING        = (_RT_STRING),
  OT_TABLE         = (_RT_TABLE|SQOBJECT_COLLECTABLE),
  OT_ARRAY         = (_RT_ARRAY|SQOBJECT_COLLECTABLE),
  OT_USERDATA      = (_RT_USERDATA|SQOBJECT_COLLECTABLE),
  OT_CLOSURE       = (_RT_CLOSURE|SQOBJECT_COLLECTABLE),
  OT_NATIVECLOSURE = (_RT_NATIVECLOSURE|SQOBJECT_COLLECTABLE),
  OT_FUNCPROTO     = (_RT_FUNCPROTO|SQOBJECT_COLLECTABLE),
  OT_IUNKNOWN      = (_RT_IUNKNOWN),
};

#define ISREFCOUNTED(t) ((t)&SQOBJECT_COLLECTABLE)
#define _RAW_TYPE(t) ((t)&(~(SQOBJECT_COLLECTABLE|SQOBJECT_NUMERIC)))

struct SQObject {
  ni::VarData _var;
};

typedef struct tagSQStackInfos{
  const SQChar* funcname;
  const SQChar* source;
  int line;
}SQStackInfos;

typedef struct SQVM* HSQUIRRELVM;
typedef SQObject HSQOBJECT;
typedef int (*SQFUNCTION)(HSQUIRRELVM);
typedef int (*SQUSERDATARELEASE)(ni::tPtr,int size);
typedef void (*SQCOMPILERERROR)(HSQUIRRELVM,const SQChar* /*desc*/,const SQChar * /*source*/,int /*line*/,int /*column*/);
typedef int (*SQWRITEFUNC)(ni::tPtr,ni::tPtr,int);
typedef int (*SQREADFUNC)(ni::tPtr,ni::tPtr,int);

typedef SQInt (*SQLEXREADFUNC)(ni::tPtr);

typedef struct tagSQRegFunction{
  const SQChar *name;
  SQFUNCTION f;
  int nparamscheck;
  const SQChar *typemask;
}SQRegFunction;

niExportFunc(int) sq_registerfuncs(HSQUIRRELVM v, const SQRegFunction* funcs);

/*vm*/
niExportFunc(SQSharedState*) sq_getss();
niExportFunc(void) sq_seterrorhandler(HSQUIRRELVM v);
niExportFunc(void) sq_setforeignptr(HSQUIRRELVM v,ni::tPtr p);
niExportFunc(ni::tPtr) sq_getforeignptr(HSQUIRRELVM v);

/*compiler*/
niExportFunc(SQRESULT) sq_compile(HSQUIRRELVM v,SQLEXREADFUNC read,ni::tPtr p,const SQChar *sourcename,int raiseerror);
niExportFunc(SQRESULT) sq_compilebuffer(HSQUIRRELVM v,const SQChar *s,int size,const SQChar *sourcename,int raiseerror);
niExportFunc(void) sq_enabledebuginfos(HSQUIRRELVM v, int debuginfo);
niExportFunc(bool) sq_aredebuginfosenabled(HSQUIRRELVM v);
niExportFunc(void) sq_setcompilererrorhandler(HSQUIRRELVM v,SQCOMPILERERROR f);

/*stack operations*/
niExportFunc(void) sq_push(HSQUIRRELVM v,int idx);
niExportFunc(void) sq_pop(HSQUIRRELVM v,int nelemstopop);
niExportFunc(void) sq_remove(HSQUIRRELVM v,int idx);
niExportFunc(int) sq_gettop(HSQUIRRELVM v);
niExportFunc(void) sq_settop(HSQUIRRELVM v,int newtop);
niExportFunc(void) sq_reservestack(HSQUIRRELVM v,int nsize);
niExportFunc(int) sq_cmp(HSQUIRRELVM v);
niExportFunc(void) sq_move(HSQUIRRELVM dest,HSQUIRRELVM src,int idx);

/*object creation handling*/
niExportFunc(void) sq_pushud(HSQUIRRELVM v,SQUserData* ud);
niExportFunc(void) sq_newtable(HSQUIRRELVM v);
niExportFunc(void) sq_newarray(HSQUIRRELVM v,int size);
niExportFunc(void) sq_newclosure(HSQUIRRELVM v,SQFUNCTION func,unsigned int nfreevars);
niExportFunc(SQRESULT) sq_setparamscheck(HSQUIRRELVM v,int nparamscheck,const SQChar *typemask);
niExportFunc(void) sq_pushstring(HSQUIRRELVM v,const SQChar *s,int len);
niExportFunc(void) sq_pushhstring(HSQUIRRELVM v,ni::iHString *s);
niExportFunc(void) sq_pushf32(HSQUIRRELVM v,ni::tF32 f);
niExportFunc(void) sq_pushf64(HSQUIRRELVM v,ni::tF64 f);
niExportFunc(void) sq_pushint(HSQUIRRELVM v,SQInt n);
niExportFunc(void) sq_pushnull(HSQUIRRELVM v);
niExportFunc(SQObjectType) sq_gettype(HSQUIRRELVM v,int idx);
niExportFunc(SQInt) sq_getsize(HSQUIRRELVM v,int idx);
niExportFunc(SQRESULT) sq_getstring(HSQUIRRELVM v,int idx,const SQChar **c);
niExportFunc(SQRESULT) sq_gethstring(HSQUIRRELVM v,int idx,ni::iHString **c);
niExportFunc(SQRESULT) sq_getint(HSQUIRRELVM v,int idx,SQInt *i);
niExportFunc(SQRESULT) sq_getf32(HSQUIRRELVM v,int idx,ni::tF32* f);
niExportFunc(SQRESULT) sq_getf64(HSQUIRRELVM v,int idx,ni::tF64* f);
niExportFunc(SQRESULT) sq_getuserdata(HSQUIRRELVM v,int idx,SQUserData** p);
niExportFunc(SQRESULT) sq_getclosureinfo(HSQUIRRELVM v,int idx,ni::tU32* nparams,ni::tU32* nfreevars);
niExportFunc(SQRESULT) sq_setnativeclosurename(HSQUIRRELVM v,int idx,const SQChar *name);

/*object manipulation*/
niExportFunc(void) sq_pushroottable(HSQUIRRELVM v);
niExportFunc(SQRESULT) sq_setroottable(HSQUIRRELVM v);
niExportFunc(SQRESULT) sq_createslot(HSQUIRRELVM v,int idx);
niExportFunc(SQRESULT) sq_deleteslot(HSQUIRRELVM v,int idx,int pushval);
niExportFunc(SQRESULT) sq_set(HSQUIRRELVM v,int idx);
niExportFunc(SQRESULT) sq_get(HSQUIRRELVM v,int idx);
niExportFunc(SQRESULT) sq_rawget(HSQUIRRELVM v,int idx);
niExportFunc(SQRESULT) sq_rawset(HSQUIRRELVM v,int idx);
niExportFunc(SQRESULT) sq_rawdeleteslot(HSQUIRRELVM v,int idx,int pushval);
niExportFunc(SQRESULT) sq_arrayappend(HSQUIRRELVM v,int idx);
niExportFunc(SQRESULT) sq_arraypop(HSQUIRRELVM v,int idx,int pushval);
niExportFunc(SQRESULT) sq_arrayresize(HSQUIRRELVM v,int idx,int newsize);
niExportFunc(SQRESULT) sq_arrayreverse(HSQUIRRELVM v,int idx);
niExportFunc(SQRESULT) sq_setdelegate(HSQUIRRELVM v,int idx);
niExportFunc(SQRESULT) sq_getdelegate(HSQUIRRELVM v,int idx);
niExportFunc(SQRESULT) sq_getparent(HSQUIRRELVM v,int idx);
niExportFunc(SQRESULT) sq_shallow_clone(HSQUIRRELVM v,int idx);
niExportFunc(SQRESULT) sq_deep_clone(HSQUIRRELVM v,int idx);
niExportFunc(SQRESULT) sq_setfreevariable(HSQUIRRELVM v,int idx,unsigned int nval);
niExportFunc(SQRESULT) sq_next(HSQUIRRELVM v,int idx);

/*calls*/
niExportFunc(SQRESULT) sq_call(HSQUIRRELVM v,int params,int retval);
niExportFunc(const SQChar*) sq_getlocal(HSQUIRRELVM v,unsigned int level,unsigned int idx);
niExportFunc(SQRESULT) sq_throwerror(HSQUIRRELVM v,const SQChar *err);
niExportFunc(void) sq_getlasterror(HSQUIRRELVM v);

/*raw object handling*/
niExportFunc(SQRESULT) sq_getstackobj(HSQUIRRELVM v,int idx,HSQOBJECT *po);
niExportFunc(void) sq_pushobject(HSQUIRRELVM v,HSQOBJECT obj);
niExportFunc(int) sq_addref(SQSharedState& aSS,HSQOBJECT *po);
niExportFunc(int) sq_release(SQSharedState& aSS,HSQOBJECT *po);
niExportFunc(void) sq_resetobject(HSQOBJECT *po);

/*serialization*/
niExportFunc(SQRESULT) sq_writeclosure(HSQUIRRELVM vm,SQWRITEFUNC writef,ni::tPtr up);
niExportFunc(SQRESULT) sq_readclosure(HSQUIRRELVM vm,SQREADFUNC readf,ni::tPtr up);

/*debug*/
niExportFunc(SQRESULT) sq_stackinfos(HSQUIRRELVM v,int level,SQStackInfos *si);
niExportFunc(void) sq_setdebughook(HSQUIRRELVM v);

/*UTILITY MACRO*/
#define sq_isnumeric(o) (sq_type(o)&SQOBJECT_NUMERIC)
#define sq_istable(o) (sq_type(o)==OT_TABLE)
#define sq_isarray(o) (sq_type(o)==OT_ARRAY)
#define sq_isfunction(o) (sq_type(o)==OT_FUNCPROTO)
#define sq_isclosure(o) (sq_type(o)==OT_CLOSURE)
#define sq_isnativeclosure(o) (sq_type(o)==OT_NATIVECLOSURE)
#define sq_isstring(o) (sq_type(o)==OT_STRING)
#define sq_isint(o) (sq_type(o)==OT_INTEGER)
#define sq_isfloat(o) (sq_type(o)==OT_FLOAT)
#define sq_isuserdata(o) (sq_type(o)==OT_USERDATA)
#define sq_isnull(o) (sq_type(o)==OT_NULL)
#define sq_isiunknown(o) (sq_type(o)==OT_IUNKNOWN)
#define sq_type(o) (o._var.mType)

#define SQ_OK (0)
#define SQ_ERROR (-1)

#define SQ_FAILED(res) (res<0)
#define SQ_SUCCEEDED(res) (res>=0)

// Script types
enum eScriptType
{
  eScriptType_Invalid = ni::eInvalidHandle,
  eScriptType_Null = OT_NULL,
  eScriptType_Int = OT_INTEGER,
  eScriptType_Float = OT_FLOAT,
  eScriptType_String = OT_STRING,
  eScriptType_Table = OT_TABLE,
  eScriptType_Array = OT_ARRAY,
  eScriptType_Closure = OT_CLOSURE,
  eScriptType_NativeClosure = OT_NATIVECLOSURE,
  eScriptType_UserData = OT_USERDATA,
  eScriptType_Vec2 = niFourCC('V','E','C','2'),
  eScriptType_Vec3 = niFourCC('V','E','C','3'),
  eScriptType_Vec4 = niFourCC('V','E','C','4'),
  eScriptType_Matrix = niFourCC('M','T','X',' '),
  eScriptType_IUnknown = OT_IUNKNOWN, // niFourCC('I','U','K','N'),
  eScriptType_MethodDef = niFourCC('M','D','E','F'),
  eScriptType_PropertyDef = niFourCC('P','D','E','F'),
  eScriptType_EnumDef = niFourCC('E','D','E','F'),
  eScriptType_Iterator = niFourCC('I','T','E','R'),
  eScriptType_IndexedProperty = niFourCC('I','D','X','P'),
  eScriptType_UUID = niFourCC('U','U','I','D'),
};

niExportFunc(eScriptType) sqa_getscripttype(HSQUIRRELVM v, int idx);

niExportFunc(int) sqa_registernewtypes(HSQUIRRELVM v);
niExportFunc(void) sqa_registerglobalfunction(HSQUIRRELVM v, const SQChar* aaszName, SQFUNCTION func);
niExportFunc(void) sqa_registerfunction(HSQUIRRELVM v, const SQChar* aaszName, SQFUNCTION func);
niExportFunc(void) sqa_setdebugname(HSQUIRRELVM v, int idx, const SQChar* aaszName);

niExportFunc(void*) sqa_getud(HSQUIRRELVM v, int idx, int aType);
template <typename T>
inline T* sqa_getud(HSQUIRRELVM v, int idx) {
  void* p = sqa_getud(v,idx,T::_GetType());
  return (T*)p;
}

niExportFunc(int) sqa_pushIUnknown(HSQUIRRELVM v, ni::iUnknown* apClass);
niExportFunc(int) sqa_getIUnknown(HSQUIRRELVM v, int idx, ni::iUnknown** appIUnknown, const ni::tUUID& aIID);

niExportFunc(int) sqa_pushvec2f(HSQUIRRELVM v, const ni::sVec2f& aV);
niExportFunc(int) sqa_getvec2f(HSQUIRRELVM v, int idx, ni::sVec2f* aV);
niExportFunc(int) sqa_pushvec2i(HSQUIRRELVM v, const ni::sVec2i& aV);
niExportFunc(int) sqa_getvec2i(HSQUIRRELVM v, int idx, ni::sVec2i* aV);

niExportFunc(int) sqa_pushvec3f(HSQUIRRELVM v, const ni::sVec3f& aV);
niExportFunc(int) sqa_getvec3f(HSQUIRRELVM v, int idx, ni::sVec3f* aV);
niExportFunc(int) sqa_pushvec3i(HSQUIRRELVM v, const ni::sVec3i& aV);
niExportFunc(int) sqa_getvec3i(HSQUIRRELVM v, int idx, ni::sVec3i* aV);

niExportFunc(int) sqa_pushvec4f(HSQUIRRELVM v, const ni::sVec4f& aV);
niExportFunc(int) sqa_getvec4f(HSQUIRRELVM v, int idx, ni::sVec4f* aV);
niExportFunc(int) sqa_pushvec4i(HSQUIRRELVM v, const ni::sVec4i& aV);
niExportFunc(int) sqa_getvec4i(HSQUIRRELVM v, int idx, ni::sVec4i* aV);

niExportFunc(int) sqa_pushmatrixf(HSQUIRRELVM v, const ni::sMatrixf& aV);
niExportFunc(int) sqa_getmatrixf(HSQUIRRELVM v, int idx, ni::sMatrixf* aV);

niExportFunc(int) sqa_pushvar(HSQUIRRELVM v, const ni::Var& aVar);
niExportFunc(int) sqa_getvar(HSQUIRRELVM v, int idx, ni::Var* aVar);
niExportFunc(int) sqa_getvar_astype(HSQUIRRELVM v, int idx, ni::Var* aVar, const ni::tType aType);

niExportFunc(int) sqa_pushUUID(HSQUIRRELVM v, const ni::tUUID& aV);
niExportFunc(int) sqa_getUUID(HSQUIRRELVM v, int idx, ni::tUUID* aV);

niExportFunc(int) sqa_pushMethodDef(HSQUIRRELVM v, const ni::sInterfaceDef* apInterfaceDef, const ni::sMethodDef* apMethDef);
niExportFunc(int) sqa_getMethodDef(HSQUIRRELVM v, int idx, const ni::sInterfaceDef** appInterfaceDef, const ni::sMethodDef** appMethDef);

niExportFunc(int) sqa_pushIUnknown(HSQUIRRELVM v, ni::iUnknown* apClass);
niExportFunc(int) sqa_getIUnknown(HSQUIRRELVM v, int idx, ni::iUnknown** appIUnknown, const ni::tUUID& aIID);

niExportFuncCPP(cString) sqa_gettypestring(int type);

#ifdef _DEBUG
#define niSqGuardSimple(v)    int __sq_top = sq_gettop(v);
#define niSqGuardSimple_(v,op)  int __sq_top = sq_gettop(v) op;
#define niSqUnGuardSimple(v)  niAssertMsg(sq_gettop(v) == __sq_top, niFmt(_A("VM stack corrupted, expected %d, have %d"),__sq_top,sq_gettop(v)));
#else
#define niSqGuardSimple(v)
#define niSqGuardSimple_(v,op)
#define niSqUnGuardSimple(v)
#endif

/**@}*/
/**@}*/
#endif // niMinFeatures
#endif // __VMAPI_H_D91B3BAB_01A1_4C38_B23F_FE82D00FA955__
