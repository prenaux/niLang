#include "stdafx.h"

#include "sqvm.h"
#include "sqtable.h"
#include "sqarray.h"
#include "sqfuncproto.h"
#include "sqclosure.h"
#include "sqobject.h"
#include "ScriptVM.h"
#include "ScriptTypes.h"
#include "ScriptObject.h"
#include <niLang/Utils/Path.h>
#include <niLang/Utils/MessageID.h>
#include <niLang/IRegex.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdio.h>
#include <niLang/Utils/TimerSleep.h>
#include <niLang/Utils/ModuleUtils.h>
#include "sq_hstring.h"

bool str2num(const SQChar *s,SQObjectPtr &res)
{
  if (*s == '#')
    ++s;
  const SQChar *end;
  const SQChar *t = s;
  if (ni::StrNICmp(t,_A("0x"),2) == 0) {
    const SQChar* sTemp;
    res = SQInt(ni::StrToUL(s,&sTemp,16));
  }
  else
  {
    if (ni::StrStr(s,_A(".")) || ni::StrStr(s,_A("e"))  || ni::StrStr(s,_A("E"))){
      SQFloat r = SQFloat(ni::StrToD(s,&end));
      if (s == end)
        return false;
      while (StrIsSpace((*end)))
        ++end;
      if (*end != _A('\0'))
        return false;
      res = r;
      return true;
    }

    while (*t != _A('\0')) {
      if (!StrIsNumberPart(*t++))
        return false;
    }

    res = SQInt(StrAToL(s));
  }
  return true;
}

static int base_getthisvm(HSQUIRRELVM v) {
  cScriptVM* pVM = (cScriptVM*)sq_getforeignptr(v);
  sqa_pushIUnknown(v,pVM);
  return 1;
}

static int base_ObjectToIScriptObject(HSQUIRRELVM v)
{
  cScriptVM* pVM = (cScriptVM*)sq_getforeignptr(v);
  Ptr<iScriptObject> ptrIScriptObject = pVM->CreateObject(2,0);
  if (!ptrIScriptObject.IsOK()) {
    return sq_throwerror(v,"base_ObjectToIScriptObject, Can't create IScriptObject.");
  }
  sqa_pushIUnknown(v,ptrIScriptObject);
  return 1;
}

static int base_IScriptObjectToObject(HSQUIRRELVM v)
{
  Ptr<iScriptObject> ptrIScriptObject;
  iUnknown* pIScriptObject;
  if (!SQ_SUCCEEDED(sqa_getIUnknown(v,2,&pIScriptObject,niGetInterfaceUUID(iScriptObject))))
    return sq_throwerror(v,_A("base_IScriptObjectToObject, the given instance doesn't implement iScriptObject."));

  ptrIScriptObject = (iScriptObject*)pIScriptObject;
  cScriptVM* pVM = (cScriptVM*)sq_getforeignptr(v);
  if (!pVM->PushObject(ptrIScriptObject)) {
    return sq_throwerror(v,_A("base_IScriptObjectToObject, can't push IScriptObject on the stack."));
  }
  return 1;
}

static int base_collectgarbage(HSQUIRRELVM v)
{
  cScriptVM* pVM = (cScriptVM*)sq_getforeignptr(v);
  sq_pushint(v, pVM->CollectGarbage());
  return 1;
}

static int base_GetLangDelegate(HSQUIRRELVM v)
{
  cScriptVM* pVM = (cScriptVM*)sq_getforeignptr(v);
  const SQChar *str=NULL;
  sq_getstring(v,-1,&str);
  pVM->mptrVM->Push(v->_ss->GetLangDelegate(v,str));
  return 1;
}

static int base_LockLangDelegates(HSQUIRRELVM v)
{
  v->_ss->LockLangDelegates();
  return 0;
}

static int base_getroottable(HSQUIRRELVM v)
{
  v->Push(v->_roottable);
  return 1;
}

static int base_setroottable(HSQUIRRELVM v)
{
  SQObjectPtr &o=stack_get(v,2);
  if(SQ_FAILED(sq_setroottable(v))) return SQ_ERROR;
  v->Push(o);
  return 1;
}

static int base_setraiseerrormode(HSQUIRRELVM v) {
  SQInt cur = v->_raiseErrorMode;
  SQInt value;
  sq_getint(v,-1,&value);
  v->_raiseErrorMode = value;
  sq_pushint(v,cur);
  return 1;
}
static int base_getraiseerrormode(HSQUIRRELVM v) {
  SQInt cur = v->_raiseErrorMode;
  sq_pushint(v,cur);
  return 1;
}

static int base_seterrorhandler(HSQUIRRELVM v)
{
  sq_seterrorhandler(v);
  return 0;
}

static int base_setdebughook(HSQUIRRELVM v)
{
  sq_setdebughook(v);
  return 0;
}

static int base_enabledebuginfos(HSQUIRRELVM v)
{
  SQObjectPtr &o=stack_get(v,2);
  sq_enabledebuginfos(v,(_sqtype(o) != OT_NULL)?1:0);
  return 0;
}

static int base_getbasestackinfos(HSQUIRRELVM v)
{
  SQInt level;
  SQStackInfos si;
  sq_getint(v, -1, &level);
  if (SQ_SUCCEEDED(sq_stackinfos(v, level, &si)))
  {
    const SQChar *fn = _A("unknown");
    const SQChar *src = _A("unknown");
    if(si.funcname)fn = si.funcname;
    if(si.source)src = si.source;
    sq_newtable(v);
    sq_pushstring(v, _HC(func));
    sq_pushstring(v, _H(fn));
    sq_createslot(v, -3);
    sq_pushstring(v, _HC(src));
    sq_pushstring(v, _H(src));
    sq_createslot(v, -3);
    sq_pushstring(v, _HC(line));
    sq_pushint(v, si.lineCol.x);
    sq_createslot(v, -3);
    sq_pushstring(v, _HC(col));
    sq_pushint(v, si.lineCol.y);
    sq_createslot(v, -3);
    return 1;
  }

  return 0;
}

static int base_getstackinfos(HSQUIRRELVM v)
{
  SQInt level;
  SQStackInfos si;
  int seq = 0;
  const SQChar *name = NULL;
  sq_getint(v, -1, &level);
  if (SQ_SUCCEEDED(sq_stackinfos(v, level, &si)))
  {
    const SQChar *fn = _A("unknown");
    const SQChar *src = _A("unknown");
    if(si.funcname)fn = si.funcname;
    if(si.source)src = si.source;
    sq_newtable(v);
    sq_pushstring(v, _HC(func));
    sq_pushstring(v, _H(fn));
    sq_createslot(v, -3);
    sq_pushstring(v, _HC(src));
    sq_pushstring(v, _H(src));
    sq_createslot(v, -3);
    sq_pushstring(v, _HC(line));
    sq_pushint(v, si.lineCol.x);
    sq_createslot(v, -3);
    sq_pushstring(v, _HC(col));
    sq_pushint(v, si.lineCol.y);
    sq_createslot(v, -3);
    sq_pushstring(v, _HC(locals));
    sq_newtable(v);
    seq=0;
    for (;;) {
      name = sq_getlocal(v, level, seq);
      if (!name) break;
      sq_pushstring(v, _H(name));
      sq_push(v, -2);
      sq_createslot(v, -4);
      sq_pop(v, 1);
      seq++;
    }
    sq_createslot(v, -3);
    return 1;
  }

  return 0;
}

static int base_assert(HSQUIRRELVM v)
{
  if(sq_gettype(v,-1)!=OT_NULL){
    return 0;
  }
  else return sq_throwerror(v,_A("assertion failed"));
}

static int base_debugbreak(HSQUIRRELVM v)
{
  niAssertUnreachable("Script DebugBreak");
  return 0;
}

static int get_slice_params(HSQUIRRELVM v,int &sidx,int &eidx,SQObjectPtr &o)
{
  int top = sq_gettop(v);
  sidx=0;
  eidx=0;
  o=stack_get(v,1);
  SQObjectPtr &start=stack_get(v,2);
  if(_sqtype(start)!=OT_NULL && sq_isnumeric(start)){
    sidx=toint(start);
  }
  if(top>2){
    SQObjectPtr &end=stack_get(v,3);
    if(sq_isnumeric(end)){
      eidx=toint(end);
    }
  }
  else {
    eidx = sq_getsize(v,1);
  }
  return 1;
}

static void _doPrint(bool bNewLine, const ni::achar* str) {
  static ThreadMutex _printMutex;
  AutoThreadLock lock(_printMutex);
  iFile* fpStdOut = ni::GetStdOut();
  niPanicAssert(niIsOK(fpStdOut));
  fpStdOut->WriteString(str);
  if (bNewLine) {
    fpStdOut->WriteString("\n");
  }
  fpStdOut->Flush();
}

static int base_print_(HSQUIRRELVM v, bool bNewLine)
{
  SQObjectPtr &o=stack_get(v,2);
  switch(_sqtype(o)){
    case OT_STRING: {
      _doPrint(bNewLine,_stringval(o));
      break;
    }
    case OT_INTEGER: {
      _doPrint(bNewLine,niFmt(_A("%d"),_int(o)));
      break;
    }
    case OT_FLOAT: {
      cString out;
      SQVM_CatFloatToString(out, _float(o));
      _doPrint(bNewLine,out.Chars());
      break;
    }
    default: {
      SQObjectPtr tname;
      v->TypeOf(o,tname);
      _doPrint(bNewLine,niFmt(_A("(%s)"),_stringval(tname)));
      break;
    }
  }
  return 0;
}

static int base_print(HSQUIRRELVM v) {
  return base_print_(v,false);
}
static int base_println(HSQUIRRELVM v) {
  return base_print_(v,true);
}

static int base_compilestring(HSQUIRRELVM v)
{
  int nargs=sq_gettop(v);
  const SQChar *src=NULL,*name=_A("unnamedbuffer");
  SQInt size;
  sq_getstring(v,2,&src);
  size=sq_getsize(v,2);
  if(nargs>2){
    sq_getstring(v,3,&name);
  }
  if(SQ_SUCCEEDED(sq_compilebuffer(v,src,size,name,0)))
    return 1;
  else
    return SQ_ERROR;
}

static int base_Array(HSQUIRRELVM v)
{
  SQInt reserveSize = 0;
  if (sq_gettop(v) > 1) {
    sq_getint(v,2,&reserveSize);
  }
  SQArray* a = SQArray::Create(0);
  if (reserveSize > 0) {
    a->Reserve(reserveSize);
  }
  v->Push(a);
  return 1;
}

static int base_Table(HSQUIRRELVM v)
{
  SQInt reserveSize = 0;
  if (sq_gettop(v) > 1) {
    sq_getint(v,2,&reserveSize);
  }
  SQTable* t = SQTable::Create();
  v->Push(t);
  return 1;
}

static int base_ultof(HSQUIRRELVM v)
{
  SQInt value;
  sq_getint(v,-1,&value);
  sq_pushf32(v,ni::ultof(value));
  return 1;
}

static int base_ftoul(HSQUIRRELVM v)
{
  tF32 value;
  sq_getf32(v,-1,&value);
  sq_pushint(v,ni::ftoul(value));
  return 1;
}

static int base_FourCC(HSQUIRRELVM v)
{
  SQInt a,b,c,d;
  if (!SQ_SUCCEEDED(sq_getint(v,-4,&a))) return sq_throwerror(v,_A("Invalid parameter A."));
  if (!SQ_SUCCEEDED(sq_getint(v,-3,&b))) return sq_throwerror(v,_A("Invalid parameter B."));
  if (!SQ_SUCCEEDED(sq_getint(v,-2,&c))) return sq_throwerror(v,_A("Invalid parameter C."));
  if (!SQ_SUCCEEDED(sq_getint(v,-1,&d))) return sq_throwerror(v,_A("Invalid parameter D."));
  sq_pushint(v,niFourCC(a,b,c,d));
  return 1;
}

static int base_MessageID_FromString(HSQUIRRELVM v)
{
  const SQChar *src=NULL;
  if (!SQ_SUCCEEDED(sq_getstring(v,-1,&src)))
    return sq_throwerror(v,_A("Invalid string parameter 0."));
  sq_pushint(v,ni::MessageID_FromString(src));
  return 1;
}

static int base_MessageID(HSQUIRRELVM v)
{
  SQInt nByte;
  const SQChar *src=NULL;
  if (!SQ_SUCCEEDED(sq_getstring(v,-2,&src)))
    return sq_throwerror(v,_A("param[0]: Not a string."));
  if (ni::StrLen(src) < 4)
    return sq_throwerror(v,_A("param[0]: Invalid string length."));
  if (!SQ_SUCCEEDED(sq_getint(v,-1,&nByte)))
    return sq_throwerror(v,_A("param[1]: Not an integer."));

  sq_pushint(v,
                 niMessageID(
                     (tU8)src[0],
                     (tU8)src[1],
                     (tU8)src[2],
                     (tU8)src[3],
                     nByte));
  return 1;
}

static int base_MessageID_GetCharA(HSQUIRRELVM v)
{
  SQInt msgID;
  if (!SQ_SUCCEEDED(sq_getint(v,-1,&msgID)))
    return sq_throwerror(v,_A("Invalid integer parameter 0."));
  sq_pushint(v,niMessageID_GetCharA(msgID));
  return 1;
}
static int base_MessageID_GetCharB(HSQUIRRELVM v)
{
  SQInt msgID;
  if (!SQ_SUCCEEDED(sq_getint(v,-1,&msgID)))
    return sq_throwerror(v,_A("Invalid integer parameter 0."));
  sq_pushint(v,niMessageID_GetCharB(msgID));
  return 1;
}
static int base_MessageID_GetCharC(HSQUIRRELVM v)
{
  SQInt msgID;
  if (!SQ_SUCCEEDED(sq_getint(v,-1,&msgID)))
    return sq_throwerror(v,_A("Invalid integer parameter 0."));
  sq_pushint(v,niMessageID_GetCharC(msgID));
  return 1;
}
static int base_MessageID_GetCharD(HSQUIRRELVM v)
{
  SQInt msgID;
  if (!SQ_SUCCEEDED(sq_getint(v,-1,&msgID)))
    return sq_throwerror(v,_A("Invalid integer parameter 0."));
  sq_pushint(v,niMessageID_GetCharD(msgID));
  return 1;
}
static int base_MessageID_GetBYTE(HSQUIRRELVM v)
{
  SQInt msgID;
  if (!SQ_SUCCEEDED(sq_getint(v,-1,&msgID)))
    return sq_throwerror(v,_A("Invalid integer parameter 0."));
  sq_pushint(v,niMessageID_GetBYTE(msgID));
  return 1;
}
static int base_MessageID_MaskA(HSQUIRRELVM v)
{
  SQInt msgID;
  if (!SQ_SUCCEEDED(sq_getint(v,-1,&msgID)))
    return sq_throwerror(v,_A("Invalid integer parameter 0."));
  sq_pushint(v,niMessageID_MaskA(msgID));
  return 1;
}
static int base_MessageID_MaskAB(HSQUIRRELVM v)
{
  SQInt msgID;
  if (!SQ_SUCCEEDED(sq_getint(v,-1,&msgID)))
    return sq_throwerror(v,_A("Invalid integer parameter 0."));
  sq_pushint(v,niMessageID_MaskAB(msgID));
  return 1;
}
static int base_MessageID_MaskABC(HSQUIRRELVM v)
{
  SQInt msgID;
  if (!SQ_SUCCEEDED(sq_getint(v,-1,&msgID)))
    return sq_throwerror(v,_A("Invalid integer parameter 0."));
  sq_pushint(v,niMessageID_MaskABC(msgID));
  return 1;
}
static int base_MessageID_MaskABCD(HSQUIRRELVM v)
{
  SQInt msgID;
  if (!SQ_SUCCEEDED(sq_getint(v,-1,&msgID)))
    return sq_throwerror(v,_A("Invalid integer parameter 0."));
  sq_pushint(v,niMessageID_MaskABCD(msgID));
  return 1;
}

static int base_MessageID_ToString(HSQUIRRELVM v)
{
  SQInt msgID;
  if (!SQ_SUCCEEDED(sq_getint(v,-1,&msgID)))
    return sq_throwerror(v,_A("Invalid integer parameter 0."));
  cString str = ni::MessageID_ToString(msgID);
  sq_pushstring(v,_H(str.Chars()));
  return 1;
}

static int base_clockSecs(HSQUIRRELVM v)
{
  sq_pushf64(v,ni::TimerInSeconds());
  return 1;
}

static int base_sleepMs(HSQUIRRELVM v)
{
  SQInt ms;
  if (!SQ_SUCCEEDED(sq_getint(v,-1,&ms)))
    return sq_throwerror(v,_A("Invalid integer parameter 0."));
  ni::SleepMs((tU32)ms);
  return 0;
}

static int base_sleepSecsCoarse(HSQUIRRELVM v)
{
  ni::tF64 secs;
  if (!SQ_SUCCEEDED(sq_getf64(v,-1,&secs)))
    return sq_throwerror(v,_A("Invalid float64 parameter 0."));
  ni::SleepSecsCoarse(secs);
  return 0;
}

static int base_sleepSecsSpin(HSQUIRRELVM v)
{
  ni::tF64 secs;
  if (!SQ_SUCCEEDED(sq_getf64(v,-1,&secs)))
    return sq_throwerror(v,_A("Invalid float64 parameter 0."));
  ni::SleepSecsSpin(secs);
  return 0;
}

static int base_sleepSecs(HSQUIRRELVM v)
{
  ni::tF64 secs;
  if (!SQ_SUCCEEDED(sq_getf64(v,-1,&secs)))
    return sq_throwerror(v,_A("Invalid float64 parameter 0."));
  ni::SleepSecs(secs);
  return 0;
}

static int base_EnumToString(HSQUIRRELVM v)
{
  const tU32 top = sq_gettop(v);

  SQInt nEnumToStringFlags = 0;
  if (top >= 4) {
    sq_getint(v,4,&nEnumToStringFlags);
  }

  const sEnumDef* enumDef = NULL;
  if (top >= 3) {
    sScriptTypeEnumDef* pV = sqa_getud<sScriptTypeEnumDef>(v,3);
    if (pV) {
      enumDef = pV->pEnumDef;
    }
  }

  SQInt nValue = 0;
  if (!SQ_SUCCEEDED(sq_getint(v,2,&nValue)))
    return sq_throwerror(v,_A("base_EnumToString: Can't get value."));

  const cString str = ni::GetLang()->EnumToString(nValue, enumDef, nEnumToStringFlags);
  sq_pushstring(v,_H(str));
  return 1;
}

static int base_StringToEnum(HSQUIRRELVM v)
{
  const tU32 top = sq_gettop(v);

  SQInt nStringToEnumFlags = 0;
  if (top >= 4) {
    sq_getint(v,4,&nStringToEnumFlags);
  }

  const sEnumDef* enumDef = NULL;
  if (top >= 3) {
    sScriptTypeEnumDef* pV = sqa_getud<sScriptTypeEnumDef>(v,3);
    if (pV) {
      enumDef = pV->pEnumDef;
    }
  }

  const SQChar* str = NULL;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&str)))
    return sq_throwerror(v,_A("base_StringToEnum: Can't get value."));

  const tU32 r = ni::GetLang()->StringToEnum(str, enumDef, nStringToEnumFlags);
  sq_pushint(v,r);
  return 1;
}

static int base_CreateCollectionVector(HSQUIRRELVM v)
{
  SQInt valueType;
  if (!SQ_SUCCEEDED(sq_getint(v,2,&valueType)))
    return sq_throwerror(v,_A("Invalid integer parameter for ValueType."));
  Ptr<iMutableCollection> coll = ni::CreateCollectionVector(valueType);
  if (!coll.IsOK())
    return sq_throwerror(v,_A("Invalid Vector collection value type."));
  sqa_pushIUnknown(v,coll);
  return 1;
}

static int base_CreateCollectionMap(HSQUIRRELVM v)
{
  SQInt keyType, valueType;
  if (!SQ_SUCCEEDED(sq_getint(v,2,&keyType)))
    return sq_throwerror(v,_A("Invalid integer parameter for KeyType."));
  if (!SQ_SUCCEEDED(sq_getint(v,3,&valueType)))
    return sq_throwerror(v,_A("Invalid integer parameter for ValueType."));
  Ptr<iMutableCollection> coll = ni::CreateCollectionMap(keyType,valueType);
  if (!coll.IsOK())
    return sq_throwerror(v,_A("Invalid Map collection key/value types."));
  sqa_pushIUnknown(v,coll);
  return 1;
}

static int base_GatherLastLogs(HSQUIRRELVM v) {
  SQInt count;
  if (!SQ_SUCCEEDED(sq_getint(v,2,&count)))
    return sq_throwerror(v,_A("Invalid integer parameter for count."));
  Ptr<tStringCVec> lastLogs = ni::tStringCVec::Create();
  ni_get_last_logs(lastLogs,count);
  sqa_pushIUnknown(v,lastLogs);
  return 1;
}

static int base_GetModuleFileName(HSQUIRRELVM v)
{
  const SQChar *src=NULL;
  if (!SQ_SUCCEEDED(sq_getstring(v,-1,&src)))
    return sq_throwerror(v,_A("Invalid string parameter 0."));
  ni::cString moduleFilename = ni::GetModuleFileName(src);
  sq_pushstring(v,_H(moduleFilename));
  return 1;
}

// ::SerializeReadObject(iFile) -> object
static int sqRead(ni::tPtr apFile, ni::tPtr apSrc, int size) {
  return reinterpret_cast<iFile*>(apFile)->ReadRaw(apSrc, size);
}
static int base_SerializeReadObject(HSQUIRRELVM v) {
  SQObjectPtr o;
  iFile* pIFile;
  if (!SQ_SUCCEEDED(sqa_getIUnknown(v,2,(iUnknown**)&pIFile,niGetInterfaceUUID(iFile))) ||
      !niIsOK(pIFile))
  {
    return sq_throwerror(v,"base_SerializeReadObject, invalid source iFile.");
  }
  if (!pIFile->GetCanRead()) {
    return sq_throwerror(v,"base_SerializeWriteObject, can't read from the source iFile.");
  }
  if (!ReadObject(v,(ni::tPtr)pIFile,sqRead,o)) {
    return -1;
  }
  v->Push(o);
  return 1;
}

// ::SerializeWriteObject(iFile, any) -> written bytes
static int sqWrite(ni::tPtr apFile, ni::tPtr apSrc, int size) {
  return reinterpret_cast<iFile*>(apFile)->WriteRaw(apSrc, size);
}
static int base_SerializeWriteObject(HSQUIRRELVM v) {
  iFile* pIFile;
  if (!SQ_SUCCEEDED(sqa_getIUnknown(v,2,(iUnknown**)&pIFile,niGetInterfaceUUID(iFile))) ||
      !niIsOK(pIFile))
  {
    return sq_throwerror(v,"base_SerializeWriteObject, invalid destination iFile.");
  }
  if (!pIFile->GetCanWrite()) {
    return sq_throwerror(v,"base_SerializeWriteObject, can't write in the destination iFile.");
  }
  tSize startPos = pIFile->Tell();
  const SQObjectPtr& o = stack_get(v,3);
  if (!WriteObject(v,(ni::tPtr)pIFile,sqWrite,o)) {
    return -1;
  }
  v->Push(SQInt(pIFile->Tell()-startPos));
  return 1;
}

static int default_to_intptr(HSQUIRRELVM v) {
  SQObjectPtr& o = stack_get(v,1);
  sq_pushint(v,o._var.mIntPtr);
  return 1;
}

static int default_shallow_clone(HSQUIRRELVM v) {
  SQVM* vm = v;
  SQObjectPtr &o=stack_get(v,1);
  SQObjectPtr target;
  vm->Clone(o,target,NULL);
  vm->Push(target);
  return 1;
}

static int default_deep_clone(HSQUIRRELVM v) {
  SQVM* vm = v;
  tSQDeepCloneGuardSet guardSet;
  SQObjectPtr &o=stack_get(v,1);
  SQObjectPtr target;
  if (!vm->Clone(o,target,&guardSet))
    return -1; // throw the last error
  vm->Push(target);
  return 1;
}

static int default_delegate_len(HSQUIRRELVM v)
{
  v->Push(SQInt(sq_getsize(v,1)));
  return 1;
}

static int default_delegate_empty(HSQUIRRELVM v)
{
  v->Push(SQInt(sq_getsize(v,1) == 0));
  return 1;
}

static int default_delegate_notempty(HSQUIRRELVM v)
{
  v->Push(SQInt(sq_getsize(v,1) != 0));
  return 1;
}

static int default_delegate_tonumber(HSQUIRRELVM v)
{
  SQObjectPtr &o=stack_get(v,1);
  switch(_sqtype(o)){
    case OT_STRING:{
      SQObjectPtr res;
      if (str2num(_stringval(o),res)) {
        v->Push(res);
      }
      else {
        v->Push(_null_);
      }
      break;
    }
    case OT_INTEGER:
    case OT_FLOAT:
      v->Push(o);
      break;
    default:
      v->Push(_null_);
      break;
  }
  return 1;
}

static int default_delegate_tofloat(HSQUIRRELVM v)
{
  SQObjectPtr &o=stack_get(v,1);
  switch(_sqtype(o)){
    case OT_STRING:{
      SQObjectPtr res;
      if (str2num(_stringval(o),res)) {
        v->Push(SQObjectPtr(tofloat(res)));
      }
      else {
        v->Push(_null_);
      }
      break;
    }
    case OT_INTEGER:case OT_FLOAT:
      v->Push(SQObjectPtr(tofloat(o)));
      break;
    default:
      v->Push(_null_);
      break;
  }
  return 1;
}

static int default_delegate_toint(HSQUIRRELVM v)
{
  SQObjectPtr &o=stack_get(v,1);
  switch(_sqtype(o)){
    case OT_STRING:
      {
        SQObjectPtr res;
        const achar* str = _stringval(o);
        tU32 ch = StrGetNext(str);
        if (!ch) {
          v->Push(SQObjectPtr(eFalse));
          break;
        }
        else if (!StrIsNumberPart(ch)) {
          if (ni::StrICmp(str,_A("true")) == 0) {
            v->Push(SQObjectPtr(eTrue));
            break;
          }
          else if (ni::StrICmp(str,_A("false")) == 0) {
            v->Push(SQObjectPtr(eFalse));
            break;
          }
        }
        else if (str2num(str,res)) {
          v->Push(SQObjectPtr((SQInt)toint(res)));
          break;
        }
        return sq_throwerror(v,niFmt(_A("toint, string '%s' can't be converted to a number."),_stringval(o)));
      }
    case OT_INTEGER:case OT_FLOAT:
      v->Push(SQObjectPtr((SQInt)toint(o)));
      break;
    default:
      v->Push(_null_);
      break;
  }
  return 1;
}

static int default_delegate_tostring(HSQUIRRELVM v)
{
  SQObjectPtr &o=stack_get(v,1);
  switch(_sqtype(o)){
    case OT_STRING:
      v->Push(o);
      break;
    case OT_INTEGER:
      v->Push(_H(niFmt(_A("%d"),_int(o))));
      break;
    case OT_FLOAT: {
      cString out;
      SQVM_CatFloatToString(out,_float(o));
      v->Push(_H(out));
      break;
    }
    default:
      v->Push(_null_);
      break;
  }
  return 1;
}

static int number_delegate_tochar(HSQUIRRELVM v)
{
  achar buffer[5] = {0,0,0,0,0};
  niCAssert(sizeof(buffer) == 5);
  SQObject &o = stack_get(v,1);
  const tU32 c = toint(o);
  StrSetChar(buffer,c);
  v->Push(_H(buffer));
  return 1;
}


/////////////////////////////////////////////////////////////////
//TABLE DEFAULT DELEGATE

static int table_rawdelete(HSQUIRRELVM v)
{
  //return SQ_SUCCEEDED(sq_getdelegate(v,-1))?1:SQ_ERROR;
  if(SQ_FAILED(sq_rawdeleteslot(v,1,1)))
    return SQ_ERROR;
  return 1;
}


static int table_rawset(HSQUIRRELVM v)
{
  return sq_rawset(v,-3);
}


static int table_rawget(HSQUIRRELVM v)
{
  return SQ_SUCCEEDED(sq_rawget(v,-2))?1:SQ_ERROR;
}

static int table_getkey(HSQUIRRELVM v)
{
  SQObject& table = stack_get(v,1);
  SQObject& val = stack_get(v,2);
  SQObjectPtr o;
  if (!_table(table)->GetKey(val,o))
    sq_pushnull(v);
  else
    sq_pushobject(v,o);
  return 1;
}

static int table_clear(HSQUIRRELVM v)
{
  SQObject& table = stack_get(v,-1);
  _table(table)->Clear();
  return 1;
}

static int table_invalidate(HSQUIRRELVM v)
{
  SQObject& table = stack_get(v,-1);
  _table(table)->Invalidate();
  return 1;
}

static SQInt table_setdelegate(HSQUIRRELVM v)
{
  if(SQ_FAILED(sq_setdelegate(v,-2)))
    return SQ_ERROR;
  sq_push(v,-1); // -1 because sq_setdelegate pops 1
  return 1;
}

static SQInt table_getdelegate(HSQUIRRELVM v)
{
  return SQ_SUCCEEDED(sq_getdelegate(v,-1))?1:0;
}

static SQInt table_getparent(HSQUIRRELVM v)
{
  return SQ_SUCCEEDED(sq_getparent(v,-1))?1:0;
}

static SQInt table_hasdelegate(HSQUIRRELVM v)
{
  SQObject& t = stack_get(v,1);
  SQObject& del = stack_get(v,2);
  tBool r = eFalse;
  SQTable* ct = _table(t)->GetDelegate();
  while (ct) {
    if (ct == _table(del)) {
      r = eTrue;
      break;
    }
    ct = ct->GetDelegate();
  }
  sq_pushint(v,r);
  return 1;
}

static int table_GetThisTable(HSQUIRRELVM v)
{
  SQObject& table = stack_get(v,-1);
  sq_pushobject(v,table);
  return 1;
}

static int table_SetCanCallMetaMethod(HSQUIRRELVM v)
{
  SQObject &o = stack_get(v, 1);
  SQObject &canCall = stack_get(v, 2);
  _table(o)->SetCanCallMetaMethod(toint(canCall));
  sq_pushnull(v);
  return 1;
}

static int table_GetCanCallMetaMethod(HSQUIRRELVM v)
{
  SQObject &o = stack_get(v, 1);
  sq_pushint(v,_table(o)->CanCallMetaMethod());
  return 1;
}

//ARRAY DEFAULT DELEGATE///////////////////////////////////////

static int array_capacity(HSQUIRRELVM v)
{
  v->Push(_array(stack_get(v,1))->Capacity());
  return 1;
}

static int array_append(HSQUIRRELVM v)
{
  return sq_arrayappend(v,-2);
}

static int array_extend(HSQUIRRELVM v)
{
  _array(stack_get(v,1))->Extend(_array(stack_get(v,2)));
  return 0;
}

static int array_reverse(HSQUIRRELVM v)
{
  return sq_arrayreverse(v,-1);
}

static int array_pop(HSQUIRRELVM v)
{
  return SQ_SUCCEEDED(sq_arraypop(v,1,1))?1:SQ_ERROR;
}

static int array_top(HSQUIRRELVM v)
{
  SQObject &o=stack_get(v,1);
  if(_array(o)->Size()>0){
    v->Push(_array(o)->Top());
    return 1;
  }
  else return sq_throwerror(v,_A("top() on a empty array"));
}

static int array_clear(HSQUIRRELVM v)
{
  SQObject &o=stack_get(v,1);
  _array(o)->Clear();
  return 0;
}

static int array_insert(HSQUIRRELVM v)
{
  SQObject &o=stack_get(v,1);
  SQObject &idx=stack_get(v,2);
  SQObject &val=stack_get(v,3);
  if (!sq_isnumeric(idx)) return sq_throwerror(v, _A("array wrong index type"));

  int intIdx = toint(idx);
  if (intIdx < 0 || intIdx > _array(o)->Size())
    return sq_throwerror(v, niFmt("array index '%d' out of range [0 ; %d]'.", intIdx, _array(o)->Size()));

  _array(o)->Insert(idx,val);
  return 0;
}

static int array_remove(HSQUIRRELVM v)
{
  SQObject &o = stack_get(v, 1);
  SQObject &idx = stack_get(v, 2);
  if(!sq_isnumeric(idx)) return sq_throwerror(v, _A("array wrong index type"));
  SQObjectPtr val;
  if(_array(o)->Get(toint(idx), val)) {
    _array(o)->Remove(toint(idx));
    v->Push(val);
    return 1;
  }
  return sq_throwerror(v, _A("idx out of range"));
}

static int array_resize(HSQUIRRELVM v)
{
  SQObject &o = stack_get(v, 1);
  SQObject &nsize = stack_get(v, 2);
  SQObjectPtr fill;
  if(sq_isnumeric(nsize)) {
    if(sq_gettop(v) > 2)
      fill = stack_get(v, 3);
    _array(o)->Resize(toint(nsize),fill);
    return 0;
  }
  return sq_throwerror(v, _A("size must be a number"));
}

static int array_reserve(HSQUIRRELVM v)
{
  SQObject &o = stack_get(v, 1);
  SQObject &nsize = stack_get(v, 2);
  if(sq_isnumeric(nsize)) {
    _array(o)->Reserve(toint(nsize));
    return 0;
  }
  return sq_throwerror(v, _A("size must be a number"));
}

template <typename T, typename FUN_CMP, typename FUN_INVALID_CMP>
bool quicksort(HSQUIRRELVM v, T& array, int hi, int lo, FUN_CMP& cmp, FUN_INVALID_CMP& invalidCmp)
{
  int ret = 0;
  while (hi > lo) {
    int i = lo;
    int j = hi;
    do {
      for (;;) {
        if (!cmp(v,array[i],array[lo],ret))
          return false;
        if (!((ret < 0) && (i < j)))
          break;
        ++i;
      }

      for (;;) {
        if (j <= 0) {
          invalidCmp(v);
          return false;
        }
        if (!cmp(v,array[--j],array[lo],ret))
          return false;
        if (!(ret > 0))
          break;
      }

      if (i < j) {
        ni::Swap(array[i],array[j]);
      }
    } while (i < j);

    ni::Swap(array[lo],array[j]);
    if ((j-lo) > (hi-(j+1))) {
      if (!quicksort(v,array,j-1,lo,cmp,invalidCmp))
        return false;
      lo = j+1;
    }
    else {
      if (!quicksort(v,array,hi,j+1,cmp,invalidCmp))
        return false;
      hi = j-1;
    }
  }
  return true;
}

static int array_sort(HSQUIRRELVM v)
{
  SQObjectPtr &o = stack_get(v,1);
  SQObject &funcobj = stack_get(v,2);
  if(_array(o)->Size() > 1) {
    struct _Local {
      static void funErr(HSQUIRRELVM v) {
        v->Raise_MsgError("compare func is miss-behaved");
      };
      static bool funCmp(HSQUIRRELVM v, const SQObjectPtr& a, const SQObjectPtr& b, int& ret) {
        ret = 0;
        int top = sq_gettop(v);
        sq_push(v, 2);
        sq_pushroottable(v);
        v->Push(a);
        v->Push(b);
        if (SQ_FAILED(sq_call(v, 3, 1))) {
          v->Raise_MsgError("compare func failed");
          return false;
        }
        sq_getint(v, -1, &ret);
        sq_settop(v, top);
        return true;
      };
      static bool objCmp(HSQUIRRELVM v, const SQObjectPtr& a, const SQObjectPtr& b, int& ret) {
        if (!v->ObjCmp(a,b,ret))
          return false;
        return true;
      };
    };
    if (_sqtype(funcobj) == OT_CLOSURE || _sqtype(funcobj) == OT_NATIVECLOSURE) {
      if (!quicksort(v,_array(o)->_values, (int)_array(o)->_values.size(),0,
                     _Local::funCmp, _Local::funErr))
        return SQ_ERROR;
    }
    else {
      if (!quicksort(v,_array(o)->_values, (int)_array(o)->_values.size(), 0,
                     _Local::objCmp, _Local::funErr))
        return SQ_ERROR;
    }
  }
  v->Push(o);
  return 1;
}

static int array_slice(HSQUIRRELVM v)
{
  int sidx,eidx;
  SQObjectPtr o;
  if(get_slice_params(v,sidx,eidx,o)==-1)return -1;
  if(sidx<0)sidx=_array(o)->Size()+sidx;
  if(eidx<0)eidx=_array(o)->Size()+eidx;
  if(eidx <= sidx)return sq_throwerror(v,_A("wrong indexes"));
  SQArray *arr=SQArray::Create(eidx-sidx);
  SQObjectPtr t;
  int count=0;
  for(int i=sidx;i<eidx;i++){
    _array(o)->Get(i,t);
    arr->Set(count++,t);
  }
  v->Push(arr);
  return 1;

}

//STRING DEFAULT DELEGATE//////////////////////////

static int string_size(HSQUIRRELVM v)
{
  v->Push(SQInt(sq_getsize(v,1)*sizeof(SQChar)));
  return 1;
}

static int string_slice(HSQUIRRELVM v)
{
  int sidx,eidx;
  SQObjectPtr o;
  if(SQ_FAILED(get_slice_params(v,sidx,eidx,o)))return -1;
  if(sidx<0)sidx=_stringlen(o)+sidx;
  if(eidx<0)eidx=_stringlen(o)+eidx;
  if (eidx<sidx) {
    v->Push(v->GetEmptyString());
  }
  else {
    const achar* p = &_stringval(o)[sidx];
    const int slen = _stringlen(o);
    if (sidx >= slen) {
      v->Push(v->GetEmptyString());
    }
    else {
      int count = eidx-sidx;
      if ((sidx+count) >= slen)
        count = slen - sidx;
      v->Push(_H(cString(p,count)));
    }
  }
  return 1;
}

///////////////////////////////////////////////
static int string_split(HSQUIRRELVM v)
{
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("string_split, Invalid parameter."));

  astl::vector<cString> vToks;
  StringSplit(cString(str),param,&vToks);

  sq_newarray(v,0);
  for (tU32 i = 0; i < vToks.size(); ++i) {
    sq_pushstring(v,_H(vToks[i]));
    sq_arrayappend(v,-2);
  }

  return 1;
}
static int string_split_sep(HSQUIRRELVM v)
{
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("string_split, Invalid parameter."));

  astl::vector<cString> vToks;
  StringSplitSep(cString(str),param,&vToks);

  sq_newarray(v,0);
  for (tU32 i = 0; i < vToks.size(); ++i) {
    sq_pushstring(v,_H(vToks[i]));
    sq_arrayappend(v,-2);
  }

  return 1;
}

static int string_split_sep_quoted(HSQUIRRELVM v)
{
  const SQChar *str,*delim;
  SQInt quote;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&delim))) return sq_throwerror(v,_A("string_split_sep_quoted, Invalid delim parameter."));
  if (!SQ_SUCCEEDED(sq_getint(v,3,&quote))) return sq_throwerror(v,_A("string_split_sep_quoted, Invalid quote parameter."));

  astl::vector<cString> vToks;
  StringSplitSepQuoted(cString(str),delim,quote,&vToks);

  sq_newarray(v,0);
  for (tU32 i = 0; i < vToks.size(); ++i) {
    sq_pushstring(v,_H(vToks[i]));
    sq_arrayappend(v,-2);
  }

  return 1;
}

static int string_split_csv_fields(HSQUIRRELVM v)
{
  const SQChar *str,*delim;
  SQInt quote;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&delim))) return sq_throwerror(v,_A("string_split_sep_quoted, Invalid delim parameter."));
  if (!SQ_SUCCEEDED(sq_getint(v,3,&quote))) return sq_throwerror(v,_A("string_split_sep_quoted, Invalid quote parameter."));

  astl::vector<cString> vFields;
  StringSplitCsvFields(cString(str),delim,quote,&vFields);

  sq_newarray(v,0);
  for (tU32 i = 0; i < vFields.size(); ++i) {
    sq_pushstring(v,_H(vFields[i]));
    sq_arrayappend(v,-2);
  }

  return 1;
}

///////////////////////////////////////////////
static int string_find(HSQUIRRELVM v) {
  int top,start_idx=0;
  const SQChar *str,*substr;
  if (((top=sq_gettop(v))>1) && SQ_SUCCEEDED(sq_getstring(v,1,&str)) && SQ_SUCCEEDED(sq_getstring(v,2,&substr)))
  {
    if (top>2)
      sq_getint(v,3,&start_idx);
    tI32 ret = StrZFind(str,0,substr,0,start_idx);
    if (ret != -1) {
      sq_pushint(v,ret);
      return 1;
    }
    return 0;
  }
  return sq_throwerror(v,_A("invalid param"));
}

static int string_rfind(HSQUIRRELVM v) {
  int top,start_idx=0x7fffffff;
  const SQChar *str,*substr;
  if (((top=sq_gettop(v))>1) && SQ_SUCCEEDED(sq_getstring(v,1,&str)) && SQ_SUCCEEDED(sq_getstring(v,2,&substr)))
  {
    if (top>2)
      sq_getint(v,3,&start_idx);
    tI32 ret = StrZRFind(str,0,substr,0,start_idx);
    if (ret != -1) {
      sq_pushint(v,ret);
      return 1;
    }
    return 0;
  }
  return sq_throwerror(v,_A("invalid param"));
}
static int string_ifind(HSQUIRRELVM v) {
  int top,start_idx=0;
  const SQChar *str,*substr;
  if (((top=sq_gettop(v))>1) && SQ_SUCCEEDED(sq_getstring(v,1,&str)) && SQ_SUCCEEDED(sq_getstring(v,2,&substr)))
  {
    if (top>2)
      sq_getint(v,3,&start_idx);
    tI32 ret = StrZFindI(str,0,substr,0,start_idx);
    if (ret != -1) {
      sq_pushint(v,ret);
      return 1;
    }
    return 0;
  }
  return sq_throwerror(v,_A("invalid param"));
}
static int string_irfind(HSQUIRRELVM v) {
  int top,start_idx=0x7fffffff;
  const SQChar *str,*substr;
  if (((top=sq_gettop(v))>1) && SQ_SUCCEEDED(sq_getstring(v,1,&str)) && SQ_SUCCEEDED(sq_getstring(v,2,&substr)))
  {
    if (top>2)
      sq_getint(v,3,&start_idx);
    tI32 ret = StrZRFindI(str,0,substr,0,start_idx);
    if (ret != -1) {
      sq_pushint(v,ret);
      return 1;
    }
    return 0;
  }
  return sq_throwerror(v,_A("invalid param"));
}

static int string_contains(HSQUIRRELVM v)
{
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("string_contains, Invalid parameter."));
  sq_pushint(v,StrZFind(str,0,param,0) != -1);
  return 1;
}

static int string_icontains(HSQUIRRELVM v)
{
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("string_contains, Invalid parameter."));
  sq_pushint(v,StrZFindI(str,0,param,0) != -1);
  return 1;
}

#define MAX_FORMAT_LEN   20
#define MAX_WFORMAT_LEN   3
#define ADDITIONAL_FORMAT_SPACE (100*sizeof(SQChar))

static int validate_format(HSQUIRRELVM v, SQChar *fmt, const SQChar *src, int n,int &width)
{
  SQChar swidth[MAX_WFORMAT_LEN];
  int wc = 0;
  int start = n;
  fmt[0] = '%';
  while (StrChr(_A("-+ #0"), src[n]))
    n++;

  while (StrIsDigit(src[n])) {
    swidth[wc] = src[n];
    n++;
    wc++;
    if(wc>=MAX_WFORMAT_LEN)
      return sq_throwerror(v,_A("width format too long"));
  }
  swidth[wc] = '\0';
  if(wc > 0) {
    width = (int)StrAToL(swidth);
  }
  else
    width = 0;
  if (src[n] == '.') {
    n++;

    wc = 0;
    while (StrIsDigit(src[n])) {
      swidth[wc] = src[n];
      n++;
      wc++;
      if(wc>=MAX_WFORMAT_LEN)
        return sq_throwerror(v,_A("precision format too long"));
    }
    swidth[wc] = '\0';
    if(wc > 0) {
      width += (int)StrAToL(swidth);
    }
  }
  if (n-start > MAX_FORMAT_LEN )
    return sq_throwerror(v,_A("format too long"));
  memcpy(&fmt[1],&src[start],((n-start)+1)*sizeof(SQChar));
  fmt[(n-start)+2] = '\0';
  return n;
}

static int string_format(HSQUIRRELVM v)
{
  const SQChar *format;
  cString strDest;
  SQChar fmt[MAX_FORMAT_LEN];
  sq_getstring(v,2,&format);
  int n = 0, nparam = 3, w;
  while(format[n] != '\0') {
    if(format[n] != '%') {
      strDest.appendChar(format[n]);
      n++;
    }
    else if(format[n+1] == '%') {
      strDest.appendChar('%');
      n += 2;
    }
    else {
      n++;
      if( nparam > sq_gettop(v) )
        return sq_throwerror(v,_A("not enough paramters for the given format string"));
      n = validate_format(v,fmt,format,n,w);
      if(n < 0) return -1;
      int valtype = 0;
      const SQChar *ts = "";
      SQInt ti = 0;
      SQFloat tf = 0;
      switch(format[n]) {
        case 's':
          switch (sq_gettype(v,nparam)) {
            case OT_NULL:
              ts = _A("{NULL}");
              break;
            case OT_STRING:
              sq_getstring(v,nparam,&ts);
              break;
            default:
              return sq_throwerror(v,_A("string expected for the specified format"));
          }
          valtype = 's';
          break;
        case 'i': case 'd': case 'c':case 'o':  case 'u':  case 'x':  case 'X':
          if(SQ_FAILED(sq_getint(v,nparam,&ti)))
            return sq_throwerror(v,_A("int expected for the specified format"));
          valtype = 'i';
          break;
        case 'f': case 'g': case 'G': case 'e':  case 'E':
          if(SQ_FAILED(sq_getf64(v,nparam,&tf)))
            return sq_throwerror(v,_A("float expected for the specified format"));
          valtype = 'f';
          break;
        default:
          return sq_throwerror(v,_A("invalid format"));
      }
      n++;
      switch(valtype) {
        case 's': strDest += niFmt(fmt,ts); break;
        case 'i': strDest += niFmt(fmt,ti); break;
        case 'f': strDest += niFmt(fmt,tf); break;
      };
      nparam ++;
    }
  }
  sq_pushstring(v,_H(strDest));
  return 1;
}

static int string_compare(HSQUIRRELVM v)
{
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("string_compare, Invalid parameter."));
  sq_pushint(v,ni::StrCmp(str,param));
  return 1;
}

static int string_icompare(HSQUIRRELVM v)
{
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("string_icompare, Invalid parameter."));
  sq_pushint(v,ni::StrICmp(str,param));
  return 1;
}

static int string_fpatmatch(HSQUIRRELVM v)
{
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("string_compare, Invalid parameter."));
  sq_pushint(v,afilepattern_match(param,str));
  return 1;
}

static int string_eq(HSQUIRRELVM v)
{
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("string_eq, Invalid parameter."));
  sq_pushint(v,ni::StrCmp(str,param) == 0);
  return 1;
}

static int string_ieq(HSQUIRRELVM v)
{
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("string_ieq, Invalid parameter."));
  sq_pushint(v,ni::StrICmp(str,param) == 0);
  return 1;
}

static int string_setfile(HSQUIRRELVM v)
{
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("Invalid file parameter."));
  cPath path(str);
  path.SetFile(param);
  sq_pushstring(v,_H(path.GetPath()));
  return 1;
}

static int string_getfile(HSQUIRRELVM v)
{
  const SQChar *str;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  sq_pushstring(v,_H(cPath(str).GetFile()));
  return 1;
}

static int string_getpath(HSQUIRRELVM v)
{
  const SQChar *str;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  sq_pushstring(v,_H(cPath(str).GetPath()));
  return 1;
}

static int string_setfilenoext(HSQUIRRELVM v)
{
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("Invalid file parameter."));
  cPath pathCur(str);
  cString curExt = pathCur.GetExtension();
  pathCur.SetFile(cPath(param).GetFile().Chars());
  pathCur.SetExtension(curExt.Chars());
  sq_pushstring(v,_H(pathCur.GetPath()));
  return 1;
}

static int string_getfilenoext(HSQUIRRELVM v)
{
  const SQChar *str;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  cPath path(str);
  path.SetExtension(NULL);
  sq_pushstring(v,_H(path.GetFile()));
  return 1;
}

static int string_setext(HSQUIRRELVM v)
{
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("Invalid extension parameter."));
  cPath path(str);
  path.SetExtension(param);
  sq_pushstring(v,_H(path.GetPath()));
  return 1;
}

static int string_getext(HSQUIRRELVM v)
{
  const SQChar *str;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  sq_pushstring(v,_H(cPath(str).GetExtension()));
  return 1;
}

static int string_setdir(HSQUIRRELVM v)
{
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("Invalid directory parameter."));
  cPath path(str);
  path.SetDirectory(param);
  sq_pushstring(v,_H(path.GetPath()));
  return 1;
}

static int string_getdir(HSQUIRRELVM v)
{
  const SQChar *str;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  sq_pushstring(v,_H(cPath(str).GetDirectory()));
  return 1;
}

static int string_setprotocol(HSQUIRRELVM v)
{
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("Invalid extension parameter."));
  cPath path(str);
  path.SetProtocol(param);
  sq_pushstring(v,_H(path.GetPath()));
  return 1;
}

static int string_getprotocol(HSQUIRRELVM v)
{
  const SQChar *str;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  sq_pushstring(v,_H(cPath(str).GetProtocol()));
  return 1;
}

static int string_adddirfront(HSQUIRRELVM v)
{
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("Invalid directory parameter."));
  cPath path(str);
  path.AddDirectoryFront(param);
  sq_pushstring(v,_H(path.GetPath()));
  return 1;
}

static int string_adddirback(HSQUIRRELVM v)
{
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("Invalid directory parameter."));
  cPath path(str);
  path.AddDirectoryBack(param);
  sq_pushstring(v,_H(path.GetPath()));
  return 1;
}

static int string_removedirback(HSQUIRRELVM v)
{
  const SQChar *str;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  cPath path(str);
  path.RemoveDirectoryBack();
  sq_pushstring(v,_H(path.GetPath()));
  return 1;
}

static int string_startswith(HSQUIRRELVM v)
{
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("string_startswith, Invalid parameter."));
  sq_pushint(v,cString(str).StartsWith(param));
  return 1;
}

static int string_endswith(HSQUIRRELVM v)
{
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("string_endswith, Invalid parameter."));
  sq_pushint(v,cString(str).EndsWith(param));
  return 1;
}

static int string_startswithi(HSQUIRRELVM v)
{
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("string_startswithi, Invalid parameter."));
  sq_pushint(v,cString(str).StartsWithI(param));
  return 1;
}

static int string_endswithi(HSQUIRRELVM v)
{
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("string_endswithi, Invalid parameter."));
  sq_pushint(v,cString(str).EndsWithI(param));
  return 1;
}

static int string_trimrightex(HSQUIRRELVM v)
{
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("string_trimrightex, Invalid parameter."));
  sq_pushstring(v,_H(cString(str).TrimRightEx(param)));
  return 1;
}

static int string_trimright(HSQUIRRELVM v)
{
  const SQChar *str;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  sq_pushstring(v,_H(cString(str).TrimRight()));
  return 1;
}

static int string_trimleftex(HSQUIRRELVM v)
{
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("string_trimleftex, Invalid parameter."));
  sq_pushstring(v,_H(cString(str).TrimLeftEx(param)));
  return 1;
}

static int string_trimleft(HSQUIRRELVM v)
{
  const SQChar *str;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  sq_pushstring(v,_H(cString(str).TrimLeft()));
  return 1;
}

static int string_trimex(HSQUIRRELVM v)
{
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("string_trimex, Invalid parameter."));
  sq_pushstring(v,_H(cString(str).TrimEx(param)));
  return 1;
}

static int string_trim(HSQUIRRELVM v)
{
  const SQChar *str;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  sq_pushstring(v,_H(cString(str).Trim()));
  return 1;
}

static int string_normalize(HSQUIRRELVM v)
{
  const SQChar *str;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  sq_pushstring(v,_H(cString(str).Normalize()));
  return 1;
}

static int string_before(HSQUIRRELVM v) {
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("string_before, Invalid parameter."));
  sq_pushstring(v,_H(cString(str).Before(param).Chars()));
  return 1;
}
static int string_rbefore(HSQUIRRELVM v) {
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("string_rbefore, Invalid parameter."));
  sq_pushstring(v,_H(cString(str).RBefore(param)));
  return 1;
}
static int string_after(HSQUIRRELVM v) {
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("string_after, Invalid parameter."));
  sq_pushstring(v,_H(cString(str).After(param)));
  return 1;
}
static int string_rafter(HSQUIRRELVM v) {
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("string_rafter, Invalid parameter."));
  sq_pushstring(v,_H(cString(str).RAfter(param)));
  return 1;
}

static int string_beforei(HSQUIRRELVM v) {
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("string_beforei, Invalid parameter."));
  sq_pushstring(v,_H(cString(str).BeforeI(param)));
  return 1;
}
static int string_rbeforei(HSQUIRRELVM v) {
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("string_rbeforei, Invalid parameter."));
  sq_pushstring(v,_H(cString(str).RBeforeI(param)));
  return 1;
}
static int string_afteri(HSQUIRRELVM v) {
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("string_afteri, Invalid parameter."));
  sq_pushstring(v,_H(cString(str).AfterI(param)));
  return 1;
}
static int string_rafteri(HSQUIRRELVM v) {
  const SQChar *str,*param;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getstring(v,2,&param))) return sq_throwerror(v,_A("string_rafteri, Invalid parameter."));
  sq_pushstring(v,_H(cString(str).RAfterI(param)));
  return 1;
}

static int string_left(HSQUIRRELVM v) {
  const SQChar *str;
  SQInt count;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getint(v,2,&count)))
    return sq_throwerror(v,_A("string_left, Invalid count parameter."));
  sq_pushstring(v,_H(cString(str).Left(count)));
  return 1;
}
static int string_right(HSQUIRRELVM v) {
  const SQChar *str;
  SQInt count;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getint(v,2,&count)))
    return sq_throwerror(v,_A("string_right, Invalid count parameter."));
  sq_pushstring(v,_H(cString(str).Right(count)));
  return 1;
}
static int string_mid(HSQUIRRELVM v) {
  const SQChar *str;
  SQInt first;
  SQInt count;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str))) return -1;
  if (!SQ_SUCCEEDED(sq_getint(v,2,&first)))
    return sq_throwerror(v,_A("string_mid, Invalid first parameter."));
  if (!SQ_SUCCEEDED(sq_getint(v,3,&count)))
    return sq_throwerror(v,_A("string_mid, Invalid count parameter."));
  sq_pushstring(v,_H(cString(str).Mid(first,count)));
  return 1;
}

static int string_tolower(HSQUIRRELVM v) {
  SQObject& s=stack_get(v,1);
  cString str(_stringval(s));
  str.ToLower();
  v->Push(_H(str));
  return 1;
}
static int string_toupper(HSQUIRRELVM v) {
  SQObject& s=stack_get(v,1);
  cString str(_stringval(s));
  str.ToUpper();
  v->Push(_H(str));
  return 1;
}

static int string_topropertyname(HSQUIRRELVM v) {
  SQObject& s=stack_get(v,1);
  cString str; StringToPropertyName(str,_stringval(s));
  v->Push(_H(str));
  return 1;
}
static int string_topropertymethodname(HSQUIRRELVM v) {
  SQObject& s=stack_get(v,1);
  cString str; StringToPropertyMethodName(str,_stringval(s));
  v->Push(_H(str));
  return 1;
}

static int string_topropertyname2(HSQUIRRELVM v) {
  SQObject& s=stack_get(v,1);
  cString str; StringToPropertyName2(str,_stringval(s));
  v->Push(_H(str));
  return 1;
}
static int string_topropertymethodname2(HSQUIRRELVM v) {
  SQObject& s=stack_get(v,1);
  cString str; StringToPropertyMethodName2(str,_stringval(s));
  v->Push(_H(str));
  return 1;
}

static int string_encodexml(HSQUIRRELVM v) {
  SQObject& s=stack_get(v,1);
  cString str; StringEncodeXml(str,_stringval(s));
  v->Push(_H(str));
  return 1;
}

static int string_decodexml(HSQUIRRELVM v) {
  SQObject& s=stack_get(v,1);
  cString str; StringDecodeXml(str,_stringval(s));
  v->Push(_H(str));
  return 1;
}

static int string_encodeurl(HSQUIRRELVM v) {
  SQObject& s=stack_get(v,1);
  cString str; StringEncodeUrl(str,_stringval(s));
  v->Push(_H(str));
  return 1;
}

static int string_decodeurl(HSQUIRRELVM v) {
  SQObject& s=stack_get(v,1);
  cString str; StringDecodeUrl(str,_stringval(s));
  v->Push(_H(str));
  return 1;
}

static int string_getlocalized(HSQUIRRELVM v)
{
  iHString *hspLocale, *hspText;
  if (!SQ_SUCCEEDED(sq_gethstring(v,1,&hspText))) {
    return sq_throwerror(v,_A("string_getlocalized, can't get this string."));
  }
  if ((sq_gettop(v) > 1) && SQ_SUCCEEDED(sq_gethstring(v,2,&hspLocale))) {
    sq_pushstring(v,hspText->GetLocalizedEx(hspLocale));
  }
  else {
    sq_pushstring(v,hspText->GetLocalized());
  }
  return 1;
}

// string::CharIt(offset = 0, size = -1)
static int string_charIt(HSQUIRRELVM v) {
  iHString *hspText;
  if (!SQ_SUCCEEDED(sq_gethstring(v,1,&hspText))) {
    return sq_throwerror(v,_A("string_charIt, can't get this string."));
  }
  Ptr<iHStringCharIt> it;
  int offset = 0;
  int size = -1;
  const int top = sq_gettop(v);
  if (top > 1) {
    sq_getint(v,2,&offset);
  }
  if (top > 2) {
    sq_getint(v,3,&size);
    it = hspText->CreateRangeIt(offset,size);
  }
  else {
    it = hspText->CreateCharIt(offset);
  }
  if (!it.IsOK()) {
    return sq_throwerror(v,_A("string_charIt, invalid param"));
  }
  sqa_pushIUnknown(v,it);
  return 1;
}

// $SYMBOL[FOO=BAR,STUFF=BLI]
static int string_parsesymbol(HSQUIRRELVM v) {
  iHString *hspText;
  if (!SQ_SUCCEEDED(sq_gethstring(v,1,&hspText))) {
    return sq_throwerror(v,_A("string_parsesymbol, can't get this string."));
  }
  const char* b = hspText->GetChars();
  if (*b != '$' && *b != '#') {
    v->Push(hspText);
  }
  else {
    ++b; // skip the '$' or '#'
    const char* e = b;
    while (*e) {
      if (*e == '[' && *(e+1) != '[')
        break;
      ++e;
    }
    v->Push(_H(cString(b,e)));
  }
  return 1;
}

// $SYMBOL[META=DATA,FOO=BAR,STUFF=BLI]
static int string_parsemetadata(HSQUIRRELVM v) {
  iHString *hspText;
  if (!SQ_SUCCEEDED(sq_gethstring(v,1,&hspText))) {
    return sq_throwerror(v,_A("string_parsemetadata, can't get this string."));
  }
  SQTable* t = SQTable::Create();
  const char* p = hspText->GetChars();
  while (*p) {
    if (*p == '[' && *(p+1) != '[')
      break;
    ++p;
  }
  if (*p == '[') {
    ++p; // skip the '['
    while (*p && *p != ']') {

      // skip leading spaces
      while (*p == ' ' || *p == '\t') {
        ++p;
      }

      tHStringPtr key, val;

      {
        const char* beginKey = p;
        while (*p && *p != '=' && *p != ',' && *p != ']') {
          ++p;
        }
        const char* endKey = (*p == '=' || *p == ',' || *p == ']' || !*p) ? (p-1) : p;

        {
          // skip trailing spaces
          while (beginKey != endKey && (*endKey == ' ' || *endKey == '\t' || *endKey == ']')) {
            --endKey;
          }
          if (beginKey <= endKey) {
            key = _H(cString(beginKey,endKey+1));
          }
        }
      }

      if (*p == '=') {
        ++p; // skip the =

        const char* beginVal = p;
        while (*p && *p != ',' && *p != ']') {
          ++p;
        }
        const char* endVal = (*p == ',' || *p == ']' || !*p) ? (p-1) : p;
        if (*p == ',')
          ++p; // skip the comma

        if (niIsOK(key)) {
          // skip leading spaces
          while (beginVal != endVal && (*beginVal == ' ' || *beginVal == '\t')) {
            ++beginVal;
          }
          // skip trailing spaces
          while (beginVal != endVal && (*endVal == ' ' || *endVal == '\t'|| *endVal == ']')) {
            --endVal;
          }
          if (beginVal <= endVal) {
            val = _H(cString(beginVal,endVal+1));
          }
        }
      }
      else if (*p == ',') {
        // skip the , if its alone
        ++p;
      }

      if (!HStringIsEmpty(key)) {
        t->NewSlot(key, val.IsOK() ? val : _null_);
      }
    }
  }
  v->Push(t);
  return 1;
}


static int string_IsCIdentifier(HSQUIRRELVM v) {
  const SQChar *str;
  if (!SQ_SUCCEEDED(sq_getstring(v,1,&str)))
    return -1;

  int result = eTrue;
  const char* p = str;
  if (*p >= '0' && *p <= '9') {
    // can't start with a number...
    result = eFalse;
  }
  else {
    while (*p) {
      const int c = *p;
      if ((c >= '0' && c <= '9') ||
          (c >= 'a' && c <= 'z') ||
          (c >= 'A' && c <= 'Z') ||
          c == '_')
      {
        // valid indentifier character, go on
      }
      else {
        // invalid indentifier for C
        result = eFalse;
        break;
      }
      ++p;
    }
  }
  sq_pushint(v,result);
  return 1;
}

//CLOSURE DEFAULT DELEGATE//////////////////////////

// closure.acall(this table, ...arguments)
static int closure_call(HSQUIRRELVM v)
{
  return SQ_SUCCEEDED(sq_call(v,sq_gettop(v)-1,1))?1:SQ_ERROR;
}

// closure.acall([this table, arguments])
static int closure_acall(HSQUIRRELVM v)
{
  SQArray *aparams = _array(stack_get(v,2));
  int nparams = aparams->Size();
  v->Push(stack_get(v,1));
  for (int i = 0; i < nparams; ++i)
    v->Push(aparams->_values[i]);
  return SQ_SUCCEEDED(sq_call(v,nparams,1))?1:SQ_ERROR;
}

// closure.athiscall(this table, [arguments])
static int closure_athiscall(HSQUIRRELVM v)
{
  SQObjectPtr& athis = stack_get(v,2);
  SQObjectPtr& aparams = stack_get(v,3);
  v->Push(stack_get(v,1));
  v->Push(athis);
  int nparams = 0;
  if (sq_isnull(aparams)) {
    // nothing to do here
  }
  else if (sq_isarray(aparams)) {
    SQArray* params = _array(aparams);
    nparams += params->Size();
    for (int i = 0; i < nparams; ++i)
      v->Push(params->_values[i]);
  }
  else {
    return sq_throwerror(v,niFmt("closure_athiscall, unexpected params type '%s'.",
                                 sqa_gettypestring(sq_type(aparams))));
  }

  int r = SQ_SUCCEEDED(sq_call(v,nparams+1,1))?1:SQ_ERROR;
  return r;
}

static int closure_GetNumParams(HSQUIRRELVM v)
{
  SQClosure *c = _closure(stack_get(v,1));
  SQFunctionProto *proto = _funcproto(c->_function);
  sq_pushint(v,(tU32)proto->_parameters.size());
  return 1;
}

static int closure_GetParamName(HSQUIRRELVM v)
{
  SQClosure *c = _closure(stack_get(v,1));
  SQFunctionProto *proto = _funcproto(c->_function);
  SQInt index = _int(stack_get(v,2));
  if ((tSize)index >= proto->_parameters.size()) {
    return sq_throwerror(v,niFmt("invalid parameter index '%d'.",index));
  }
  sq_pushobject(v,proto->_parameters[index]._name);
  return 1;
}

static int closure_GetParamType(HSQUIRRELVM v)
{
  SQClosure *c = _closure(stack_get(v,1));
  SQFunctionProto *proto = _funcproto(c->_function);
  SQInt index = _int(stack_get(v,2));
  if ((tSize)index >= proto->_parameters.size()) {
    return sq_throwerror(v,niFmt("invalid parameter index '%d'.",index));
  }
  sq_pushobject(v,proto->_parameters[index]._type);
  return 1;
}

static int closure_GetReturnType(HSQUIRRELVM v)
{
  SQClosure *c = _closure(stack_get(v,1));
  SQFunctionProto *proto = _funcproto(c->_function);
  sq_pushobject(v,proto->_returntype);
  return 1;
}

static int closure_GetNumFreeVars(HSQUIRRELVM v)
{
  SQClosure *c = _closure(stack_get(v,1));
  sq_pushint(v,(tU32)c->_outervalues.size());
  return 1;
}

static int closure_GetFreeVar(HSQUIRRELVM v)
{
  SQClosure *c = _closure(stack_get(v,1));
  SQInt index = _int(stack_get(v,2));
  if ((tSize)index >= c->_outervalues.size()) {
    return sq_throwerror(v,niFmt("invalid free variable index '%d'.",index));
  }
  sq_pushobject(v,c->_outervalues[index]);
  return 1;
}

static int closure_SafeGetFreeVar(HSQUIRRELVM v)
{
  SQClosure *c = _closure(stack_get(v,1));
  SQInt index = _int(stack_get(v,2));
  if ((tSize)index >= c->_outervalues.size()) {
    sq_pushnull(v);
  }
  else {
    sq_pushobject(v,c->_outervalues[index]);
  }
  return 1;
}

static int closure_GetRoot(HSQUIRRELVM v) {
  SQClosure *c = _closure(stack_get(v,1));
  QPtr<SQTable> t = c->_root;
  sq_pushobject(v, t.IsOK() ? SQObjectPtr(t.ptr()) : _null_);
  return 1;
}

static int closure_SetRoot(HSQUIRRELVM v) {
  SQClosure *c = _closure(stack_get(v,1));
  SQTable *t = _table(stack_get(v,2));
  c->_root = t;
  sq_pushobject(v, t ? SQObjectPtr(t) : _null_);
  return 1;
}

SQRegFunction SQSharedState::_table_default_delegate_funcz[]={
  {_A("ToIntPtr"),default_to_intptr,1, _A("t")},
  {_A("ShallowClone"),default_shallow_clone,1, _A("t")},
  {_A("DeepClone"),default_deep_clone,1, _A("t")},
  {_A("len"),default_delegate_len,1, _A("t")},
  {_A("Len"),default_delegate_len,1, _A("t")},
  {_A("empty"),default_delegate_empty,1, _A("t")},
  {_A("Empty"),default_delegate_empty,1, _A("t")},
  {_A("notempty"),default_delegate_notempty,1, _A("t")},
  {_A("NotEmpty"),default_delegate_notempty,1, _A("t")},
  {_A("clear"),table_clear,1, _A("t")},
  {_A("Clear"),table_clear,1, _A("t")},
  {_A("rawget"),table_rawget,2, _A("t")},
  {_A("RawGet"),table_rawget,2, _A("t")},
  {_A("rawset"),table_rawset,3, _A("t")},
  {_A("RawSet"),table_rawset,3, _A("t")},
  {_A("rawdelete"),table_rawdelete,2, _A("t")},
  {_A("RawDelete"),table_rawdelete,2, _A("t")},
  {_A("getkey"),table_getkey,2, _A("t")},
  {_A("GetKey"),table_getkey,2, _A("t")},
  {_A("setdelegate"),table_setdelegate,2, _A(".t|o")},
  {_A("SetDelegate"),table_setdelegate,2, _A(".t|o")},
  {_A("getdelegate"),table_getdelegate,1, _A(".")},
  {_A("GetDelegate"),table_getdelegate,1, _A(".")},
  {_A("getparent"),table_getparent,1, _A(".")},
  {_A("GetParent"),table_getparent,1, _A(".")},
  {_A("hasdelegate"),table_hasdelegate,2, _A("tt")},
  {_A("HasDelegate"),table_hasdelegate,2, _A("tt")},
  {_A("Invalidate"),table_invalidate,1, _A("t")},
  {_A("__GetThisTable"),table_GetThisTable,1,"t"},
  {_A("__SetCanCallMetaMethod"),table_SetCanCallMetaMethod,2,"tn"},
  {_A("__GetCanCallMetaMethod"),table_GetCanCallMetaMethod,1,"t"},
  {0,0}
};

SQRegFunction SQSharedState::_array_default_delegate_funcz[]={
  {_A("ToIntPtr"),default_to_intptr,1, _A("a")},
  {_A("ShallowClone"),default_shallow_clone,1, _A("a")},
  {_A("DeepClone"),default_deep_clone,1, _A("a")},
  {_A("len"),default_delegate_len,1, _A("a")},
  {_A("Len"),default_delegate_len,1, _A("a")},
  {_A("empty"),default_delegate_empty,1, _A("a")},
  {_A("Empty"),default_delegate_empty,1, _A("a")},
  {_A("notempty"),default_delegate_notempty,1, _A("a")},
  {_A("NotEmpty"),default_delegate_notempty,1, _A("a")},
  {_A("append"),array_append,2, _A("a")},
  {_A("Append"),array_append,2, _A("a")},
  {_A("extend"),array_extend,2, _A("aa")},
  {_A("Extend"),array_extend,2, _A("aa")},
  {_A("push"),array_append,2, _A("a")},
  {_A("Push"),array_append,2, _A("a")},
  {_A("pop"),array_pop,1, _A("a")},
  {_A("Pop"),array_pop,1, _A("a")},
  {_A("top"),array_top,1, _A("a")},
  {_A("Top"),array_top,1, _A("a")},
  {_A("clear"),array_clear,1, _A("a")},
  {_A("Clear"),array_clear,1, _A("a")},
  {_A("insert"),array_insert,3, _A("an")},
  {_A("Insert"),array_insert,3, _A("an")},
  {_A("remove"),array_remove,2, _A("an")},
  {_A("Remove"),array_remove,2, _A("an")},
  {_A("resize"),array_resize,-2, _A("an")},
  {_A("Resize"),array_resize,-2, _A("an")},
  {_A("reserve"),array_reserve,-2, _A("an")},
  {_A("Reserve"),array_reserve,-2, _A("an")},
  {_A("reverse"),array_reverse,1, _A("a")},
  {_A("Reverse"),array_reverse,1, _A("a")},
  {_A("sort"),array_sort,-1, _A("ac")},
  {_A("Sort"),array_sort,-1, _A("ac")},
  {_A("slice"),array_slice,-1, _A("ann")},
  {_A("Slice"),array_slice,-1, _A("ann")},
  {_A("capacity"),array_capacity,1, _A("a")},
  {_A("Capacity"),array_capacity,1, _A("a")},
  {0,0}
};

SQRegFunction SQSharedState::_string_default_delegate_funcz[]={
  {_A("ToIntPtr"),default_to_intptr,1, _A("s")},
  {_A("len"),default_delegate_len,1, _A("s")},
  {_A("Len"),default_delegate_len,1, _A("s")},
  {_A("empty"),default_delegate_empty,1, _A("s")},
  {_A("Empty"),default_delegate_empty,1, _A("s")},
  {_A("notempty"),default_delegate_notempty,1, _A("s")},
  {_A("NotEmpty"),default_delegate_notempty,1, _A("s")},
  {_A("size"),string_size,1, _A("s")},
  {_A("Size"),string_size,1, _A("s")},
  {_A("toint"),default_delegate_toint,1, _A("s")},
  {_A("ToInt"),default_delegate_toint,1, _A("s")},
  {_A("tofloat"),default_delegate_tofloat,1, _A("s")},
  {_A("ToFloat"),default_delegate_tofloat,1, _A("s")},
  {_A("tostring"),default_delegate_tostring,1, _A("s")},
  {_A("ToString"),default_delegate_tostring,1, _A("s")},
  {_A("tonumber"),default_delegate_tonumber,1, _A("s")},
  {_A("ToNumber"),default_delegate_tonumber,1, _A("s")},
  {_A("slice"),string_slice,-1, _A("snn")},
  {_A("Slice"),string_slice,-1, _A("snn")},
  {_A("split"),string_split,2, _A("ss")},
  {_A("Split"),string_split,2, _A("ss")},
  {_A("splitsep"),string_split_sep,2, _A("ss")},
  {_A("SplitSep"),string_split_sep,2, _A("ss")},
  {_A("splitsepquoted"),string_split_sep_quoted,3, _A("ssn")},
  {_A("SplitSepQuoted"),string_split_sep_quoted,3, _A("ssn")},
  {_A("splitcsvfields"),string_split_csv_fields,3, _A("ssn")},
  {_A("Splitcsvfields"),string_split_csv_fields,3, _A("ssn")},
  {_A("find"),string_find,-2, _A("ssn")},
  {_A("Find"),string_find,-2, _A("ssn")},
  {_A("rfind"),string_rfind,-2, _A("ssn")},
  {_A("RFind"),string_rfind,-2, _A("ssn")},
  {_A("ifind"),string_ifind,-2, _A("ssn")},
  {_A("IFind"),string_ifind,-2, _A("ssn")},
  {_A("irfind"),string_irfind,-2, _A("ssn")},
  {_A("IRFind"),string_irfind,-2, _A("ssn")},
  {_A("contains"),string_contains, 2, _A("ss")},
  {_A("Contains"),string_contains, 2, _A("ss")},
  {_A("icontains"),string_icontains, 2, _A("ss")},
  {_A("IContains"),string_icontains, 2, _A("ss")},
  {_A("tolower"),string_tolower,1, _A("s")},
  {_A("ToLower"),string_tolower,1, _A("s")},
  {_A("toupper"),string_toupper,1, _A("s")},
  {_A("ToUpper"),string_toupper,1, _A("s")},
  {_A("compare"),string_compare,-2, _A("ss")},
  {_A("Compare"),string_compare,-2, _A("ss")},
  {_A("icompare"),string_icompare,-2, _A("ss")},
  {_A("ICompare"),string_icompare,-2, _A("ss")},
  {_A("fpatmatch"),string_fpatmatch,-2, _A("ss")},
  {_A("FPatMatch"),string_fpatmatch,-2, _A("ss")},
  {_A("cmp"),string_compare,-2, _A("ss")},
  {_A("Cmp"),string_compare,-2, _A("ss")},
  {_A("icmp"),string_icompare,-2, _A("ss")},
  {_A("ICmp"),string_icompare,-2, _A("ss")},
  {_A("eq"),string_eq,-2, _A("ss")},
  {_A("Eq"),string_eq,-2, _A("ss")},
  {_A("ieq"),string_ieq,-2, _A("ss")},
  {_A("IEq"),string_ieq,-2, _A("ss")},
  {_A("getpath"),string_getpath,1, _A("s")},
  {_A("GetPath"),string_getpath,1, _A("s")},
  {_A("setfile"),string_setfile,2, _A("ss")},
  {_A("SetFile"),string_setfile,2, _A("ss")},
  {_A("getfile"),string_getfile,1, _A("s")},
  {_A("GetFile"),string_getfile,1, _A("s")},
  {_A("setfilenoext"),string_setfilenoext,2, _A("ss")},
  {_A("SetFileNoExt"),string_setfilenoext,2, _A("ss")},
  {_A("getfilenoext"),string_getfilenoext,1, _A("s")},
  {_A("GetFileNoExt"),string_getfilenoext,1, _A("s")},
  {_A("setext"),string_setext,2, _A("ss")},
  {_A("SetExt"),string_setext,2, _A("ss")},
  {_A("getext"),string_getext,1, _A("s")},
  {_A("GetExt"),string_getext,1, _A("s")},
  {_A("setprotocol"),string_setprotocol,2, _A("ss")},
  {_A("SetProtocol"),string_setprotocol,2, _A("ss")},
  {_A("getprotocol"),string_getprotocol,1, _A("s")},
  {_A("GetProtocol"),string_getprotocol,1, _A("s")},
  {_A("setdir"),string_setdir,2, _A("ss")},
  {_A("SetDir"),string_setdir,2, _A("ss")},
  {_A("adddirfront"),string_adddirfront,2,_A("ss")},
  {_A("AddDirFront"),string_adddirfront,2,_A("ss")},
  {_A("adddirback"),string_adddirback,2,_A("ss")},
  {_A("AddDirBack"),string_adddirback,2,_A("ss")},
  {_A("removedirback"),string_removedirback,1,_A("s")},
  {_A("RemoveDirBack"),string_removedirback,1,_A("s")},
  {_A("getdir"),string_getdir,1, _A("s")},
  {_A("GetDir"),string_getdir,1, _A("s")},
  {_A("startswith"),string_startswith,2, _A("ss")},
  {_A("StartsWith"),string_startswith,2, _A("ss")},
  {_A("endswith"),string_endswith,2, _A("ss")},
  {_A("EndsWith"),string_endswith,2, _A("ss")},
  {_A("startswithi"),string_startswithi,2, _A("ss")},
  {_A("StartsWithI"),string_startswithi,2, _A("ss")},
  {_A("endswithi"),string_endswithi,2, _A("ss")},
  {_A("EndsWithI"),string_endswithi,2, _A("ss")},
  {_A("trimleftex"),  string_trimleftex,  2, _A("ss")},
  {_A("TrimLeftEx"),  string_trimleftex,  2, _A("ss")},
  {_A("trimleft"),  string_trimleft,  1, _A("s")},
  {_A("TrimLeft"),  string_trimleft,  1, _A("s")},
  {_A("trimrightex"),string_trimrightex,  2, _A("ss")},
  {_A("TrimRightEx"),string_trimrightex,  2, _A("ss")},
  {_A("trimright"), string_trimright, 1, _A("s")},
  {_A("TrimRight"), string_trimright, 1, _A("s")},
  {_A("trimex"),    string_trimex,    2, _A("ss")},
  {_A("TrimEx"),    string_trimex,    2, _A("ss")},
  {_A("trim"),    string_trim,    1, _A("s")},
  {_A("Trim"),    string_trim,    1, _A("s")},
  {_A("normalize"), string_normalize, 1, _A("s")},
  {_A("Normalize"), string_normalize, 1, _A("s")},
  {_A("before"),    string_before,    2, _A("ss")},
  {_A("Before"),    string_before,    2, _A("ss")},
  {_A("rbefore"), string_rbefore,   2, _A("ss")},
  {_A("RBefore"), string_rbefore,   2, _A("ss")},
  {_A("after"),   string_after,   2, _A("ss")},
  {_A("After"),   string_after,   2, _A("ss")},
  {_A("rafter"),    string_rafter,    2, _A("ss")},
  {_A("RAfter"),    string_rafter,    2, _A("ss")},
  {_A("beforei"),   string_beforei,   2, _A("ss")},
  {_A("BeforeI"),   string_beforei,   2, _A("ss")},
  {_A("rbeforei"),  string_rbeforei,  2, _A("ss")},
  {_A("RBeforeI"),  string_rbeforei,  2, _A("ss")},
  {_A("afteri"),    string_afteri,    2, _A("ss")},
  {_A("AfterI"),    string_afteri,    2, _A("ss")},
  {_A("rafteri"),   string_rafteri,   2, _A("ss")},
  {_A("RAfterI"),   string_rafteri,   2, _A("ss")},
  {_A("left"),string_left, 2, _A("sn")},
  {_A("Left"),string_left, 2, _A("sn")},
  {_A("right"),string_right, 2, _A("sn")},
  {_A("Right"),string_right, 2, _A("sn")},
  {_A("mid"),string_mid, 3, _A("snn")},
  {_A("Mid"),string_mid, 3, _A("snn")},
  {_A("topropertyname"),string_topropertyname,1, _A("s")},
  {_A("ToPropertyName"),string_topropertyname,1, _A("s")},
  {_A("topropertymethodname"),string_topropertymethodname,1, _A("s")},
  {_A("ToPropertyMethodName"),string_topropertymethodname,1, _A("s")},
  {_A("topropertyname2"),string_topropertyname2,1, _A("s")},
  {_A("ToPropertyName2"),string_topropertyname2,1, _A("s")},
  {_A("topropertymethodname2"),string_topropertymethodname2,1, _A("s")},
  {_A("ToPropertyMethodName2"),string_topropertymethodname2,1, _A("s")},
  {_A("encodexml"),string_encodexml,1, _A("s")},
  {_A("EncodeXml"),string_encodexml,1, _A("s")},
  {_A("decodexml"),string_decodexml,1, _A("s")},
  {_A("DecodeXml"),string_decodexml,1, _A("s")},
  {_A("encodeurl"),string_encodeurl,1, _A("s")},
  {_A("EncodeUrl"),string_encodeurl,1, _A("s")},
  {_A("decodeurl"),string_decodeurl,1, _A("s")},
  {_A("DecodeUrl"),string_decodeurl,1, _A("s")},
  {_A("GetLocalized"),string_getlocalized,-1, _A("ss")},
  {_A("CharIt"),string_charIt,-1,_A("snn")},
  {_A("CreateCharIt"),string_charIt,-2,_A("snn")},
  {_A("parsesymbol"),string_parsesymbol,1,_A("s")},
  {_A("parsemetadata"),string_parsemetadata,1,_A("s")},
  {_A("IsCIdentifier"),string_IsCIdentifier,1,_A("s")},
  {0,0}
};

SQRegFunction SQSharedState::_number_default_delegate_funcz[]={
  {_A("toint"),default_delegate_toint,1, _A("n")},
  {_A("ToInt"),default_delegate_toint,1, _A("n")},
  {_A("tofloat"),default_delegate_tofloat,1, _A("n")},
  {_A("ToFloat"),default_delegate_tofloat,1, _A("n")},
  {_A("tonumber"),default_delegate_tonumber,1, _A("n")},
  {_A("ToNumber"),default_delegate_tonumber,1, _A("n")},
  {_A("tostring"),default_delegate_tostring,1, _A("n")},
  {_A("ToString"),default_delegate_tostring,1, _A("n")},
  {_A("tochar"),number_delegate_tochar,1, _A("n")},
  {_A("ToChar"),number_delegate_tochar,1, _A("n")},
  {0,0}
};

SQRegFunction SQSharedState::_closure_default_delegate_funcz[]={
  {_A("ToIntPtr"),default_to_intptr,1, _A("c")},
  {_A("call"),closure_call,-1, _A("c")},
  {_A("Call"),closure_call,-1, _A("c")},
  {_A("acall"),closure_acall,2, _A("ca")},
  {_A("ACall"),closure_acall,2, _A("ca")},
  {_A("athiscall"),closure_athiscall,3, _A("ct.")},
  {_A("GetNumParams"),closure_GetNumParams,1, _A("c")},
  {_A("GetParamName"),closure_GetParamName,2, _A("cn")},
  {_A("GetParamType"),closure_GetParamType,2, _A("cn")},
  {_A("GetReturnType"),closure_GetReturnType,1, _A("c")},
  {_A("GetNumFreeVars"),closure_GetNumFreeVars,1, _A("c")},
  {_A("GetFreeVar"),closure_GetFreeVar,2, _A("cn")},
  {_A("SafeGetFreeVar"),closure_SafeGetFreeVar,2, _A("cn")},
  {_A("GetRoot"),closure_GetRoot,1, _A("c")},
  {_A("SetRoot"),closure_SetRoot,2, _A("ct")},
  {0,0}
};

SQRegFunction SQSharedState::_base_funcs[] = {
  // constructors
  {_A("Array"),base_Array,-1, _A(".n")},
  {_A("Table"),base_Table,-1, _A(".n")},
  //generic
  {_A("GetThisVM"),base_getthisvm,1, _A("t")},
  {_A("ObjectToIScriptObject"),base_ObjectToIScriptObject,2, _A("t.")},
  {_A("IScriptObjectToObject"),base_IScriptObjectToObject,2, _A("t.")},
  {_A("LockLangDelegates"), base_LockLangDelegates, 1, _A(".")},
  {_A("GetLangDelegate"),base_GetLangDelegate, 2, _A(".s")},
  {_A("seterrorhandler"),base_seterrorhandler,2, _A("tc")},
  {_A("SetErrorHandler"),base_seterrorhandler,2, _A("tc")},
  {_A("setraiseerrormode"),base_setraiseerrormode,2, _A("tn")},
  {_A("SetRaiseErrorMode"),base_setraiseerrormode,2, _A("tn")},
  {_A("getraiseerrormode"),base_getraiseerrormode,1, _A("t")},
  {_A("GetRaiseErrorMode"),base_getraiseerrormode,1, _A("t")},
  {_A("setdebughook"),base_setdebughook,2, _A("tc")},
  {_A("SetDebugHook"),base_setdebughook,2, _A("tc")},
  {_A("enabledebuginfos"),base_enabledebuginfos,2, _A("t.")},
  {_A("EnableDebugInfos"),base_enabledebuginfos,2, _A("t.")},
  {_A("getstackinfos"),base_getstackinfos,2, _A("tn")},
  {_A("GetStackInfos"),base_getstackinfos,2, _A("tn")},
  {_A("getbasestackinfos"),base_getbasestackinfos,2, _A("tn")},
  {_A("GetBaseStackInfos"),base_getbasestackinfos,2, _A("tn")},
  // getrootable is the current context's roottable, modules have a different roottable
  {_A("getroottable"),base_getroottable,1, _A("t")},
  // getvmroottable always returns the root table of the current vm
  {_A("getvmroottable"),base_getroottable,1, _A("t")},
  {_A("debugbreak"),base_debugbreak,1, _A("t")},
  {_A("DebugBreak"),base_debugbreak,1, _A("t")},
  {_A("assert"),base_assert,2, _A("t")},
  {_A("Assert"),base_assert,2, _A("t")},
  {_A("compilestring"),base_compilestring,-2, _A(".ss")},
  {_A("CompileString"),base_compilestring,-2, _A(".ss")},
  {_A("collectgarbage"),base_collectgarbage,1, _A("t")},
  {_A("CollectGarbage"),base_collectgarbage,1, _A("t")},
  {_A("ultof"), base_ultof, 2, _A("tn")},
  {_A("ULToF"), base_ultof, 2, _A("tn")},
  {_A("ftoul"), base_ftoul, 2, _A("tn")},
  {_A("FToUL"), base_ftoul, 2, _A("tn")},
  {_A("FourCC"), base_FourCC, 5, _A("tnnnn")},
  {_A("clock"), base_clockSecs, -1, _A("t")},
  {_A("Clock"), base_clockSecs, -1, _A("t")},
  {_A("sleepms"), base_sleepMs, 2, _A("tn")},
  {_A("SleepMs"), base_sleepMs, 2, _A("tn")},
  {_A("sleepsecs"), base_sleepSecs, 2, _A("tn")},
  {_A("SleepSecs"), base_sleepSecs, 2, _A("tn")},
  {_A("sleepsecscoarse"), base_sleepSecsCoarse, 2, _A("tn")},
  {_A("SleepSecsCoarse"), base_sleepSecsCoarse, 2, _A("tn")},
  {_A("sleepsecsspin"), base_sleepSecsSpin, 2, _A("tn")},
  {_A("SleepSecsSpin"), base_sleepSecsSpin, 2, _A("tn")},
  {_A("EnumToString"), base_EnumToString, -2, _A(".n")},
  {_A("StringToEnum"), base_StringToEnum, -2, _A(".s")},
  {_A("format"),string_format,-2,_A(".s")},
  {_A("Format"),string_format,-2,_A(".s")},
  {_A("MessageID"), base_MessageID, 3, _A("tsn")},
  {_A("MessageID_ToString"), base_MessageID_ToString, 2, _A("tn")},
  {_A("MessageID_FromString"), base_MessageID_FromString, 2, _A("ts")},
  {_A("MessageID_GetCharA"), base_MessageID_GetCharA, 2, _A("tn")},
  {_A("MessageID_GetCharB"), base_MessageID_GetCharB, 2, _A("tn")},
  {_A("MessageID_GetCharC"), base_MessageID_GetCharC, 2, _A("tn")},
  {_A("MessageID_GetCharD"), base_MessageID_GetCharD, 2, _A("tn")},
  {_A("MessageID_GetBYTE"), base_MessageID_GetBYTE, 2, _A("tn")},
  {_A("MessageID_MaskA"), base_MessageID_MaskA, 2, _A("tn")},
  {_A("MessageID_MaskAB"), base_MessageID_MaskAB, 2, _A("tn")},
  {_A("MessageID_MaskABC"), base_MessageID_MaskABC, 2, _A("tn")},
  {_A("MessageID_MaskABCD"), base_MessageID_MaskABCD, 2, _A("tn")},
  {_A("CreateCollectionVector"), base_CreateCollectionVector, 2, _A("tn")},
  {_A("CreateCollectionMap"), base_CreateCollectionMap, 3, _A("tnn")},
  {_A("GetModuleFileName"), base_GetModuleFileName, 2, _A("ts")},
  {_A("GatherLastLogs"), base_GatherLastLogs, 2, _A("tn")},
  {_A("SerializeReadObject"), base_SerializeReadObject, 2, _A("t.")},
  {_A("SerializeWriteObject"), base_SerializeWriteObject, 3, _A("t..")},
  {0,0}
};

niExportFunc(int) sq_registerfuncs(HSQUIRRELVM v, const SQRegFunction* regs) {
  int i = 0;
  while (regs[i].name != NULL) {
    sq_pushstring(v,_H(regs[i].name));
    sq_newclosure(v,regs[i].f,0);
    sq_setnativeclosurename(v,-1,regs[i].name);
    sq_setparamscheck(v,regs[i].nparamscheck,NULL);
    sq_createslot(v,-3);
    ++i;
  }
  return i;
}
