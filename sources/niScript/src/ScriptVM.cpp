// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"

#include <niLang/IConcurrent.h>
#include <niLang/Utils/VMCallCImpl.h>
#include "API/niScript/IScriptObject.h"
#include "ScriptVM.h"
#include "ScriptObject.h"
#include "ScriptTypes.h"
#include "ScriptAutomation.h"
#include "sqvm.h"
#include "sqtable.h"
#include "sqfuncproto.h"
#include "sqclosure.h"
#include "ScriptVM_Concurrent.h"
#include <niLang/Utils/ConcurrentImpl.h>
#include "sq_hstring.h"

#ifdef niWindows
#include <niLang/Platforms/Win32/Win32_UTF.h>
#endif

#pragma niTodo("Exceptions are always reported as error aswell as warning, never report them twice.")
#pragma niTodo("Add in the doc that the cloned method of a userdata should return the cloned object.")

//////////////////////////////////////////////////////////////////////////////////////////////
// Squirrel static functions interface.

static tU32 _fccClosure = SQ_CLOSURESTREAM_HEAD;

// static tI32 _nOpened = 0;

// #define niAssertStackIndex(idx) niAssertMsg(sqCheckStackIndex(v,idx), niFmt(_A("Stack index '%d' is out of range."),idx));

// Return eTrue if the index is valid, else eFalse.
// static tBool sqCheckStackIndex(HSQUIRRELVM v, int idx)
// {
//  if (idx == 0) return eFalse;
//  return ni::Abs(idx) <= sq_gettop(v);
// }

void sqGetIndexDump(cString& strOut, HSQUIRRELVM v, int idx, int size)
{
  HSQOBJECT obj;
  sq_resetobject(&obj);
  sq_getstackobj(v,idx,&obj);
  eScriptType type = sqa_getscripttype(v,idx);
  if (size > 0)
    strOut << niFmt(_A("[%2d][-%2d]"),idx,((size-idx)+1));
  strOut << niFmt(_A("(0x%p) "),_intptr(obj)) << sq_typeof(v,idx) << _A(" : ");
  switch (type)
  {
    case eScriptType_Int:   strOut << niFmt(_A("%d"),_int(obj)); break;
    case eScriptType_Float:     strOut << niFmt(_A("%.5f"),_float(obj)); break;
    case eScriptType_String:    strOut << niFmt(_A("'%s'"),_stringval(obj)); break;
    case eScriptType_Vec2: {
      sScriptTypeVec2f* pV = (sScriptTypeVec2f*)_userdata(obj);
      strOut << niFmt(_A("%s"),pV->_val);
      break;
    }
    case eScriptType_Vec3: {
      sScriptTypeVec3f* pV = (sScriptTypeVec3f*)_userdata(obj);
      strOut << niFmt(_A("%s"),pV->_val);
      break;
    }
    case eScriptType_Vec4: {
      sScriptTypeVec4f* pV = (sScriptTypeVec4f*)_userdata(obj);
      strOut << niFmt(_A("%s"),pV->_val);
      break;
    }
    case eScriptType_Matrix: {
      sScriptTypeMatrixf* pV = (sScriptTypeMatrixf*)_userdata(obj);
      strOut << niFmt(_A("%s"),pV->_val);
      break;
    }
    case eScriptType_IUnknown: {
      iUnknown* p = _iunknown(obj);
      strOut << niFmt(_A("%p %s"),p,p);
      strOut << _A("[");
      iunknown_gettype_concat(strOut,v,p);
      strOut << _A("]");
      break;
    }
    case eScriptType_InterfaceDef: {
      sScriptTypeInterfaceDef* pV = (sScriptTypeInterfaceDef*)_userdata(obj);
      strOut << pV->GetTypeString();
      break;
    }
    case eScriptType_MethodDef: {
      sScriptTypeMethodDef* pV = (sScriptTypeMethodDef*)_userdata(obj);
      strOut << pV->GetTypeString();
      break;
    }
    case eScriptType_PropertyDef: {
      sScriptTypePropertyDef* pV = (sScriptTypePropertyDef*)_userdata(obj);
      strOut << pV->GetTypeString();
      break;
    }
    case eScriptType_ErrorCode: {
      sScriptTypeErrorCode* pV = (sScriptTypeErrorCode*)_userdata(obj);
      strOut << pV->GetTypeString();
      break;
    }
    case eScriptType_ResolvedType: {
      sScriptTypeResolvedType* pV = (sScriptTypeResolvedType*)_userdata(obj);
      strOut << pV->GetTypeString();
      break;
    }
    case eScriptType_EnumDef: {
      sScriptTypeEnumDef* pV = (sScriptTypeEnumDef*)_userdata(obj);
      strOut << pV->GetTypeString();
      break;
    }
    case eScriptType_Table:
      strOut << _A("[") << _table(obj)->GetDebugName() << _A("]");
      break;
    case eScriptType_IndexedProperty: {
      sScriptTypeIndexedProperty* pV = (sScriptTypeIndexedProperty*)_userdata(obj);
      strOut << niFmt(_A("[0x%08x] %s::%s"),
                      pV->pObject.ptr(),
                      pV->pProp->pInterfaceDef->maszName,
                      pV->pProp->GetName().Chars());
      break;
    }
    case eScriptType_Closure:
      {
        const achar* aszSource = _A("unknown");
        const achar* aszFuncName = _A("unknown");
        SQFunctionProto *func = _funcproto(obj);
        if (_sqtype(func->_name) == OT_STRING)
          aszFuncName = _stringval(func->_name);
        if (_sqtype(func->_sourcename) == OT_STRING)
          aszSource = _stringval(func->_sourcename);
        strOut << aszFuncName << _A("[") << aszSource << _A("]");
        break;
      }

    case eScriptType_NativeClosure:
      {
        const achar* aszSource = _A("NATIVE");
        const achar* aszFuncName = _A("unknown");
        if (HStringIsNotEmpty(_nativeclosure(obj)->_name)) {
          aszFuncName = niHStr(_nativeclosure(obj)->_name);
        }
        strOut << aszFuncName << _A("[") << aszSource << _A("]");
        break;
      }
    case eScriptType_Array:
    case eScriptType_UserData:
    case eScriptType_Invalid:
    case eScriptType_Null:
    default:
      strOut << _A("No printable value.");
      break;
  }
}

enum eSQFormatErrorFlags
{
  eSQFormatErrorFlags_Callstack = niBit(0),
  eSQFormatErrorFlags_Locals = niBit(1),
  eSQFormatErrorFlags_Short = niBit(2),
  eSQFormatErrorFlags_Stack = niBit(3),
};

static inline tU32 _GetFormatErrorFlags(tU32 anRaiseErrorMode) {
  switch (anRaiseErrorMode) {
    default:
    case eScriptRaiseErrorMode_ShortCallstack:
      return eSQFormatErrorFlags_Callstack|eSQFormatErrorFlags_Short;
    case eScriptRaiseErrorMode_Callstack:
      return eSQFormatErrorFlags_Callstack;
    case eScriptRaiseErrorMode_Locals:
      return eSQFormatErrorFlags_Callstack|eSQFormatErrorFlags_Locals|eSQFormatErrorFlags_Stack;
    case eScriptRaiseErrorMode_ShortLocals:
      return eSQFormatErrorFlags_Callstack|eSQFormatErrorFlags_Locals|eSQFormatErrorFlags_Short;
  }
}

void sqFormatError(HSQUIRRELVM v, const achar* aszErr, cString& strErr, int level)
{
  strErr << _A("=== NI ERROR ===================\n");
  if (aszErr) strErr << aszErr;
  strErr << AEOL;
  sqGetCallstack(strErr,v,level);
  strErr << _A("================================\n");
}

cString& sqGetStackDump(cString& strOut, HSQUIRRELVM v)
{
  tI32 size = sq_gettop(v);
  strOut << _A("--- STACK ---\n");
  strOut << _A("Size: ") << size << AEOL;
  for (int i = 1; i <= size; ++i)
  {
    sqGetIndexDump(strOut,v,i,size);
    strOut << AEOL;
  }
  return strOut;
}

cString& sqGetCallstack(cString& astrOut, HSQUIRRELVM v, int aLevel)
{
  const tU32 nFormatFlags = _GetFormatErrorFlags(v->_raiseErrorMode);
  cScriptVM* pVM = reinterpret_cast<cScriptVM*>(sq_getforeignptr(v));
  SQStackInfos si;
  int level=aLevel; //1 is to skip this function that is level 0
  const SQChar *name=NULL;
  int seq=0;
  if (niFlagIs(nFormatFlags,eSQFormatErrorFlags_Callstack)) {
    bool addedCallstack = false;
    while (SQ_SUCCEEDED(sq_stackinfos(v,level,&si))) {
      if (!addedCallstack) {
        astrOut << _A("--- CALLSTACK ------------------\n");
        addedCallstack = true;
      }
      astrOut << niFmt("%s [%s", (si.funcname ? si.funcname : "unknown"), (si.source ? si.source : "unknown"));
      if (si.lineCol.x >= 0) {
        astrOut << niFmt(":%d:%d",si.lineCol.x+pVM->GetErrorLineOffset(),si.lineCol.y);
      }
      astrOut << "]\n";
      level++;
    }
  }
  if (niFlagIs(nFormatFlags,eSQFormatErrorFlags_Stack)) {
    sqGetStackDump(astrOut,v);
  }
  if (niFlagIs(nFormatFlags,eSQFormatErrorFlags_Locals)) {
    astrOut << _A("--- LOCALS ---------------------\n");
    int cstksize= (int) ((SQVM*)v)->_callsstack.size();
    for(level = 0; level <= cstksize; ++level) {
      seq=0;
      for (;;) {
        name = sq_getlocal(v,level,seq);
        if (!name)
          break;
        seq++;
        astrOut << _A("L") << (tU32)level;
        sqGetIndexDump(astrOut,v,-1,-1);
        astrOut << AEOL;
        sq_pop(v,1);
      }
    }
  }

  return astrOut;
}

#if 0
static int sqWriteGetSize(ni::tPtr apFile, ni::tPtr apSrc, int size)
{
  *((tSize*)apSrc) += size;
  return size;
}
#endif

static int sqWrite(ni::tPtr apFile, ni::tPtr apSrc, int size)
{
  return reinterpret_cast<iFile*>(apFile)->WriteRaw(apSrc, size);
}

static int sqRead(ni::tPtr apFile, ni::tPtr apSrc, int size)
{
  return reinterpret_cast<iFile*>(apFile)->ReadRaw(apSrc, size);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Default print functions

///////////////////////////////////////////////
static void __stdcall _builtin_vmprint(const achar* aaszMsg) {
  if (!niStringIsOK(aaszMsg)) return;
  niPrint(aaszMsg);
}
IDLC_STATIC_METH_BEGIN(ni,vmprint,1) {
  IDLC_DECL_VAR(ni::achar*,anValue);
  IDLC_BUF_TO_BASE(ni::eType_ASZ,anValue);
  IDLC_STATIC_METH_CALL_VOID((void),(void),_builtin_vmprint,1,(anValue));
} IDLC_STATIC_METH_END(ni,vmprint,1);
const ni::sMethodDef kFuncDecl_vmprint = {
  "vmprint",eType_Null,NULL,"void",1,NULL,VMCall_vmprint
};

///////////////////////////////////////////////
static void __stdcall _builtin_vmprintln(const achar* aaszMsg) {
  niPrintln(aaszMsg);
}
IDLC_STATIC_METH_BEGIN(ni,vmprintln,1) {
  IDLC_DECL_VAR(ni::achar*,anValue);
  IDLC_BUF_TO_BASE(ni::eType_ASZ,anValue);
  IDLC_STATIC_METH_CALL_VOID((void),(void),_builtin_vmprintln,1,(anValue));
} IDLC_STATIC_METH_END(ni,vmprintln,1);
const ni::sMethodDef kFuncDecl_vmprintln = {
  "vmprintln",eType_Null,NULL,"void",1,NULL,VMCall_vmprintln
};

///////////////////////////////////////////////
static void __stdcall _builtin_vmprintdebug(const achar* aaszMsg) {
  if (!niStringIsOK(aaszMsg))
    return;
  niLog_(ni::eLogFlags_Debug|ni::eLogFlags_NoNewLine,aaszMsg);
}
IDLC_STATIC_METH_BEGIN(ni,vmprintdebug,1) {
  IDLC_DECL_VAR(ni::achar*,anValue);
  IDLC_BUF_TO_BASE(ni::eType_ASZ,anValue);
  IDLC_STATIC_METH_CALL_VOID((void),(void),_builtin_vmprintdebug,1,(anValue));
} IDLC_STATIC_METH_END(ni,vmprintdebug,1);
const ni::sMethodDef kFuncDecl_vmprintdebug = {
  "vmprintdebug",eType_Null,NULL,"void",1,NULL,VMCall_vmprintdebug
};

///////////////////////////////////////////////
static void __stdcall _builtin_vmprintdebugln(const achar* aaszMsg) {
  if (!niStringIsOK(aaszMsg))
    return;
  niLog_(ni::eLogFlags_Debug,aaszMsg);
}
IDLC_STATIC_METH_BEGIN(ni,vmprintdebugln,1) {
  IDLC_DECL_VAR(ni::achar*,anValue);
  IDLC_BUF_TO_BASE(ni::eType_ASZ,anValue);
  IDLC_STATIC_METH_CALL_VOID((void),(void),_builtin_vmprintdebugln,1,(anValue));
} IDLC_STATIC_METH_END(ni,vmprintdebugln,1);
const ni::sMethodDef kFuncDecl_vmprintdebugln = {
  "vmprintdebugln",eType_Null,NULL,"void",1,NULL,VMCall_vmprintdebugln
};

//////////////////////////////////////////////////////////////////////////////////////////////
// Maintains a list of VM, needed to do garbage collection
struct VMList : public ImplRC<iUnknown>
{
#if 0
  VMList() {
    niDebugFmt(("... VMList::VMList"));
  }
  ~VMList() {
    niDebugFmt(("... VMList::~VMList"));
  }
#endif

  void Register(cScriptVM* apVM) {
    __sync_lock();
#ifndef NO_GARBAGE_COLLECTOR
    niAssert(!astl::contains(_vmList,apVM));
    astl::push_back(_vmList, apVM);
#endif
  }

  int GC() {
    __sync_lock();
#ifndef NO_GARBAGE_COLLECTOR
    AutoThreadLock chainLock(_gc()->_gc_chain_mutex);
    if (_vmTimeGC) {
      _vmTimeStart = ni::TimerInSeconds();
      if (_gc()->_gc_chain_lastgc_sync == _gc()->_gc_chain_sync) {
#ifdef _DEBUG
        niDebugFmt(("GC, %d VM(s), skipped, no change since last gc.",
                    _vmList.size()));
#endif
        return 0;
      }
#ifdef _DEBUG
      niDebugFmt(("GC, %d VM(s), start GC.",_vmList.size()));
#endif
    }
    else {
      if (_gc()->_gc_chain_lastgc_sync == _gc()->_gc_chain_sync) {
        return 0;
      }
    }

    astl::set<Ptr<SQSharedState>> sss;
    SQCollectable* tchain = NULL;
    for (tVMList::iterator it = _vmList.begin(); it != _vmList.end(); ) {
      QPtr<cScriptVM> vm = *it;
      if (!vm.IsOK()) {
        it = _vmList.erase(it);
      }
      else {
        SQVM* sqvm = vm->mptrVM;
        if (sqvm) {
          sqvm->Mark(&tchain);
          niAssert(sqvm->_ss.IsOK());
          sss.insert(sqvm->_ss);
        }
        ++it;
      }
    }
    // mark the shared states
    for (astl::set<Ptr<SQSharedState>>::iterator it = sss.begin();
         it != sss.end(); ++it) {
      (*it)->Mark(&tchain);
    }
    int n = _gc()->CollectGarbage(&tchain);

    _gc()->_gc_chain_lastgc_sync = _gc()->_gc_chain_sync;
    if (_vmTimeGC) {
#ifdef _DEBUG
      double time = ni::TimerInSeconds() - _vmTimeStart;
      niDebugFmt(("GC, %d VM(s), collected %d garbage(s) in %gs.",
                  _vmList.size(),
                  n,time));
#endif
    }
    return n;
#else
    return 0;
#endif
  }

private:
  __sync_mutex();

#ifndef NO_GARBAGE_COLLECTOR
  tBool _vmTimeGC = eTrue;
  double _vmTimeStart = 0;
#endif

#ifndef NO_GARBAGE_COLLECTOR
  typedef astl::list<WeakPtr<cScriptVM> > tVMList;
  tVMList _vmList;
#endif
};
static VMList* _GetVMList() {
  static Ptr<VMList> _vmList = niNew VMList();
  return _vmList;
}

static Ptr<tInterfaceCVec<iFileSystem> > _ptrImportFileSystems;

///////////////////////////////////////////////
struct URLFileHandler_Script : public ImplRC<iURLFileHandler> {
  virtual iFile* __stdcall URLOpen(const achar* aURL) {
    // niDebugFmt(("... SCRIPT: URLOPEN: %s", aURL));

    niAssert(_ptrImportFileSystems.IsOK());
    const cString path = StringURLGetPath(aURL);
    niLoopr(ri,_ptrImportFileSystems->size()) {
      Ptr<iFileSystem> fs = _ptrImportFileSystems->at(ri);
      if (fs.IsOK()) {
        Ptr<iFile> fp = fs->FileOpen(path.Chars(),eFileOpenMode_Read);
        if (niIsOK(fp)) {
          return fp.GetRawAndSetNull();
        }
      }
    }
    return NULL;
  }

  virtual tBool __stdcall URLExists(const achar* aURL) {
    // niDebugFmt(("... SCRIPT: URLEXISTS: %s", aURL));

    niAssert(_ptrImportFileSystems.IsOK());
    const cString path = StringURLGetPath(aURL);
    niLoopr(ri,_ptrImportFileSystems->size()) {
      Ptr<iFileSystem> fs = _ptrImportFileSystems->at(ri);
      if (fs.IsOK()) {
        if (fs->FileExists(path.Chars(),eFileAttrFlags_AllFiles))
          return eTrue;
      }
    }
    return eFalse;
  }
};

niExportFunc(iUnknown*) New_niScript_URLFileHandlerScript(const Var& avarA, const Var& avarB) {
  return niNew URLFileHandler_Script();
}

static void __stdcall _ImportInitialize() {
  if (niIsOK(_ptrImportFileSystems))
    return;

  _ptrImportFileSystems = tInterfaceCVec<iFileSystem>::Create();
  // This is to avoid Release() when the app exists, the object will 'leak'
  // but the OS takes care of reclaiming the memory. Without this the app
  // might crash on exit because the memory allocator might have been
  // invalidated before the destructor is called.
  _ptrImportFileSystems->AddRef();

  tGlobalInstanceCMap* imap = ni::GetLang()->GetGlobalInstanceMap();
  const tBool hasScriptHandler = (imap->find("URLFileHandler.script") != imap->end());
  if (!hasScriptHandler) {
    Ptr<URLFileHandler_Script> scriptURLFileHandler = niNew URLFileHandler_Script();
    astl::upsert(*imap,_ASTR("URLFileHandler.script"),scriptURLFileHandler);

    tBool bPlayerMode = ni::GetLang()->GetProperty("niResources.PlayerMode").Bool();
    if (!bPlayerMode) {
      ni::Ptr<iFileSystem> rootFS = ni::GetLang()->GetRootFS();
      if (niIsOK(rootFS)) {
        _ptrImportFileSystems->Add(rootFS.ptr());
        cString scriptsDirPath = ni::GetLang()->GetProperty("ni.dirs.scripts");
        if (ni::GetLang()->GetRootFS()->FileExists(
                scriptsDirPath.Chars(),ni::eFileAttrFlags_AllDirectories))
        {
          scriptsDirPath = rootFS->GetAbsolutePath(scriptsDirPath.Chars());
          Ptr<iFileSystem> scriptsDirFS = ni::GetLang()->CreateFileSystemDir(
              scriptsDirPath.Chars(),ni::eFileSystemRightsFlags_ReadOnly);
          _ptrImportFileSystems->Add(scriptsDirFS.ptr());
        }
      }
      else {
        niWarning("No root file system to add as default import file system.");
      }
    }
  }
}

///////////////////////////////////////////////
cScriptVM::cScriptVM(cScriptVM* apParentVM)
{
  _InitializeSQGCAndGlobals();
  ZeroMembers();
  mbDebug = eFalse;

  if (apParentVM) {
    mptrParentVM = apParentVM;
    mptrScriptAutomation = apParentVM->mptrScriptAutomation;

    mptrVM = niNew SQVM(apParentVM->mptrVM->_ss);
    if (!mptrVM->Init(false)) {
      niError(_A("Can't initialize VM."));
      return;
    }

    niAssert(mptrVM->GetNumRefs() == 1);
    sq_setforeignptr(mptrVM, (tPtr)this);

#define CONCURRENT_ROOT_TABLE_NAME "__concurrent_child_vm_root"

    // New root table
    Ptr<SQTable> ptrRootTable = SQTable::Create();
#if 0
    ptrRootTable->SetDelegate(_table(mptrParentVM->mptrVM->_roottable));
#else
    SQObjectPtr concurrentRoot;
    const SQObjectPtr concurrentRootTableName = _H(CONCURRENT_ROOT_TABLE_NAME);
    if (_table(apParentVM->mptrVM->_roottable)->Get(concurrentRootTableName, concurrentRoot)) {
      if (!sq_istable(concurrentRoot)) {
        niWarning(CONCURRENT_ROOT_TABLE_NAME " is not a table.");
      }
      else {
        ptrRootTable->SetDelegate(_table(concurrentRoot));
      }
    }
    else {
      niWarning("No " CONCURRENT_ROOT_TABLE_NAME " table found.");
    }
#endif
    mptrVM->_roottable = ptrRootTable.ptr();

    // Copy other handlers...
    mptrVM->_errorhandler = apParentVM->mptrVM->_errorhandler;
    mptrVM->_debughook = apParentVM->mptrVM->_debughook;

    sq_pushroottable(mptrVM);
    sqa_setdebugname(mptrVM,-1, _A("roottable"));

#ifdef _DEBUG
    niDebugFmt(("ScriptVM[%p], child VM initialized.",(tIntPtr)this));
#endif
  }
  else {
    Ptr<SQSharedState> ptrSharedStates = niNew SQSharedState();
    mptrVM = niNew SQVM(ptrSharedStates);
    if (!mptrVM->Init(true)) {
      niError(_A("Can't initialize VM."));
      return;
    }

    _ImportInitialize();

    niAssert(mptrVM->GetNumRefs() == 1);
    sq_setforeignptr(mptrVM, (tPtr)this);

    sq_pushroottable(mptrVM);
    sqa_setdebugname(mptrVM,-1, _A("roottable"));

    mptrScriptAutomation = niNew cScriptAutomation();
    if (!niIsOK(mptrScriptAutomation)) {
      niError(_A("Can't create script automation utility class."));
      return;
    }

    RegisterFunction(&kFuncDecl_vmprintdebug,_A("vmprintdebug"));
    RegisterFunction(&kFuncDecl_vmprintdebugln,_A("vmprintdebugln"));
    RegisterFunction(&kFuncDecl_vmprint,_A("vmprint"));
    RegisterFunction(&kFuncDecl_vmprintln,_A("vmprintln"));

#ifdef _DEBUG
    niDebugFmt(("ScriptVM[%p], root VM initialized.",(tIntPtr)this));
#endif
  }

  _GetVMList()->Register(this);
  //sq_pop(mpVM,1); // pop the root table
  //-- dont pop it because functions like RegisterFunction wants to use the root table
  //   as default destination table
}

///////////////////////////////////////////////
cScriptVM::~cScriptVM()
{
  Invalidate();
}

///////////////////////////////////////////////
//! Invalidate.
void __stdcall cScriptVM::Invalidate()
{
  if (!mptrVM.IsOK())
    return;

#ifdef SCRIPTOBJECT_INVALIDATEPOLICY
  //astl::clear_invalidate(mlstScriptObjects);
  //  int numObj = mlstScriptObjects.size();
  tScriptObjectPLst lst = mlstScriptObjects;
  for (tScriptObjectPLstIt itO = lst.begin(); itO != lst.end(); ++itO)
  {
    cScriptObject* pO = static_cast<cScriptObject*>(*itO);
    pO->Invalidate();
  }
#endif

  mptrScriptAutomation = NULL;
  if (mptrVM.IsOK()) {
    if (concurrent_vm_is_started() && concurrent_vm_mainvm() == mptrVM.ptr()) {
#ifdef _DEBUG
      niDebugFmt(("ScriptVM[%p], root VM, concurrent shutdown.\n", (tIntPtr)this));
#endif
      concurrent_vm_shutdown();
      if (concurrent_vm_is_started()) {
        niWarning("ConcurrentVM shutdown failed.");
      }
    }

    Ptr<SQVM> v = mptrVM;
    mptrVM = NULL;
#ifdef niDebug
    if (mptrParentVM.IsOK()) {
      niDebugFmt(("ScriptVM[%p], child VM invalidated, %d refs.\n", (tIntPtr)this, v->GetNumRefs()));
    }
    else {
      niDebugFmt(("ScriptVM[%p], root VM invalidated, %d refs.\n", (tIntPtr)this, v->GetNumRefs()));
    }
#endif
    niAssert(v->GetNumRefs() == 1);
    v = NULL;
  }
}

///////////////////////////////////////////////
//! Zeros all the class members.
void cScriptVM::ZeroMembers()
{
  mbDebug = eFalse;
  mptrVM = NULL;
  mnLineOffset = 0;
}

///////////////////////////////////////////////
//! Sanity check.
tBool __stdcall cScriptVM::IsOK() const
{
  niClassIsOK(cScriptVM);
  return mptrVM.IsOK();
}

///////////////////////////////////////////////
iScriptObject* __stdcall cScriptVM::Compile(iFile* apFile, const achar* aaszName)
{
  if (!niIsOK(apFile)) {
    niError(_A("Invalid iFile."));
    return NULL;
  }

  iScriptObject* pObject;

  if (apFile->ReadLE32() == _fccClosure)
  {
    apFile->SeekSet(apFile->Tell()-4);
    pObject = ReadClosure(apFile);
    if (!niIsOK(pObject))
    {
      niError(_A("Can't read the binary closure."));
      return NULL;
    }
  }
  else
  {
    apFile->SeekSet(apFile->Tell()-4);
    tHStringNN hspSourceName = _H(
      niStringIsOK(aaszName)
      ? aaszName
      : ni::GetRootFS()->GetAbsolutePath(apFile->GetSourcePath()).Chars());
    if (HStringIsEmpty(hspSourceName)) {
      niError(_A("No name specified nor file source path."));
      return NULL;
    }

    if (!apFile->BeginTextFileRead()) {
      niError("Can't begin text file read.");
      return NULL;
    }

    cString sourceCode = apFile->ReadString();
    if (SQ_FAILED(sq_compilestring(mptrVM, hspSourceName, sourceCode.Chars()))) {
      niError(_A("Compilation failed."));
      return NULL;
    }

    pObject = CreateObject(-1,1);
    if (!niIsOK(pObject)) {
      niError(_A("Can't get compiled object."));
      return NULL;
    }

    if (pObject->GetType() != eScriptObjectType_Function) {
      niError(_A("Compiled object is not a function."));
      return NULL;
    }
  }

  return pObject;
}

///////////////////////////////////////////////
//! Compile the given file.
iScriptObject* __stdcall cScriptVM::CompileString(const achar* aaszCode, const achar* aaszName)
{
  if (!niStringIsOK(aaszCode)) {
    niError(_A("Empty Code."));
    return NULL;
  }

  tHStringNN hspSourceName = _H(
    niStringIsOK(aaszName)
    ? aaszName
    : "[VM::CompileString]");

  Ptr<iScriptObject> obj;
  if (SQ_FAILED(sq_compilestring(
        mptrVM,
        hspSourceName,
        aaszCode)))
  {
    niError(_A("Compilation failed."));
    return NULL;
  }

  obj = CreateObject(-1,1);
  if (!obj.IsOK()) {
    niError(_A("Can't get compiled object."));
    return NULL;
  }

  return obj.GetRawAndSetNull();
}

///////////////////////////////////////////////
iUnknown* __stdcall cScriptVM::CompileGetIUnknownObject(iScriptObject* apThisTable, iFile* apFile, const achar* aaszObjectName, const tUUID& aIID)
{
  if (!niIsOK(apFile)) {
    niError(_A("Invalid file."));
    return NULL;
  }

  if (niIsOK(apThisTable))
  {
    if (apThisTable->GetType() != eScriptObjectType_Table)
    {
      niError(_A("The this object is not a table."));
      return NULL;
    }
  }
  else
  {
    apThisTable = NULL;
  }

  // Compile
  Ptr<iScriptObject> ptrCode = Compile(apFile);
  if (!niIsOK(ptrCode)) {
    niError(_A("Can't compile the given file."));
    return NULL;
  }

  // Call the compiled code
  PushObject(ptrCode);
  if (apThisTable)  PushObject(apThisTable);
  else        PushRootTable();
  if (!Call(1,eFalse)) {
    niError(_A("Can't call the object."));
    return NULL;
  }
  Pop(1);

  // Get the return value
  if (apThisTable)  PushObject(apThisTable);
  else        PushRootTable();
  Ptr<iScriptObject> ptrObj = CreateObjectGet(aaszObjectName,eScriptObjectType_Last,1);
  if (!niIsOK(ptrObj))
  {
    Pop(1); // pop the table
    niError(niFmt(_A("Can't get the specified object '%s'."),aaszObjectName));
    return NULL;
  }
  Pop(1);

  // Get the iUnknown in the returned object
  iUnknown* pI = ptrObj->GetIUnknownEx(aIID);
  if (!niIsOK(pI)) {
    niSafeRelease(pI);
    niError(niFmt(_A("Can't get the interface '%s' on object '%s'."),
                  niHStr(ni::GetLang()->GetInterfaceName(aIID)),aaszObjectName));
    return NULL;
  }

  return pI;
}

///////////////////////////////////////////////
//! Set the line error reporting offset.
void __stdcall cScriptVM::SetErrorLineOffset(tI32 anOffset)
{
  mnLineOffset = anOffset;
}

///////////////////////////////////////////////
//! Get the line error reporting offset.
tI32 __stdcall cScriptVM::GetErrorLineOffset() const
{
  return mnLineOffset;
}

///////////////////////////////////////////////
eScriptRaiseErrorMode __stdcall cScriptVM::SetRaiseErrorMode(eScriptRaiseErrorMode aMode) {
  eScriptRaiseErrorMode cur = eScriptRaiseErrorMode_ShortCallstack;
  if (mptrVM.IsOK()) {
    cur = (eScriptRaiseErrorMode)mptrVM->_raiseErrorMode;
    mptrVM->_raiseErrorMode = aMode;
  }
  return cur;
}
eScriptRaiseErrorMode __stdcall cScriptVM::GetRaiseErrorMode() const {
  eScriptRaiseErrorMode cur = eScriptRaiseErrorMode_ShortCallstack;
  if (mptrVM.IsOK()) {
    cur = (eScriptRaiseErrorMode)mptrVM->_raiseErrorMode;
  }
  return cur;
}

///////////////////////////////////////////////
void __stdcall cScriptVM::SetLogRaiseError(tLogFlags aLogFlags) {
  if (mptrVM.IsOK()) {
    mptrVM->_logRaiseError = aLogFlags;
  }
}
tLogFlags __stdcall cScriptVM::GetLogRaiseError() const {
  if (mptrVM.IsOK()) {
    return mptrVM->_logRaiseError;
  }
  else {
    return 0;
  }
}

///////////////////////////////////////////////
tBool __stdcall cScriptVM::Run(iScriptObject* apThis, const achar* aaszCommand)
{
  niSqGuard(mpVM);

  Ptr<iScriptObject> obj = CompileString(aaszCommand,"[VM::Run]");
  if (!obj.IsOK()) {
    niError("Can't compile command.");
    return ni::eFalse;
  }
  // push closure
  this->PushObject(obj);
  // push 'this'
  if (apThis) this->PushObject(apThis);
  else sq_pushroottable(mptrVM);
  // call closure
  tBool bRet = SQ_SUCCEEDED(sq_call(mptrVM,1,0));
  // pop the closure
  sq_pop(mptrVM,1);
  niSqUnGuard(mpVM);
  return bRet;
}

///////////////////////////////////////////////
//! Collect garbage.
tI32 __stdcall cScriptVM::CollectGarbage()
{
  niGuardObject(this);
  return _GetVMList()->GC();
}

///////////////////////////////////////////////
//! Register a function.
tBool __stdcall cScriptVM::RegisterFunction(const sMethodDef* apFunction, const achar* aaszName)
{
  sq_pushstring(mptrVM, _H(aaszName?aaszName:apFunction->maszName));
  sqa_pushMethodDef(mptrVM,NULL,apFunction);
  sq_createslot(mptrVM,-3);
  return eTrue;
}

///////////////////////////////////////////////
//! Push the root table on the stack.
tBool __stdcall cScriptVM::PushRootTable()
{
  sq_pushroottable(mptrVM);
  return eTrue;
}

///////////////////////////////////////////////
//! Push a string on the stack.
tBool __stdcall cScriptVM::PushString(const iHString* ahspValue)
{
  sq_pushstring(mptrVM, ahspValue);
  return eTrue;
}

///////////////////////////////////////////////
//! Push an integer on the stack.
tBool __stdcall cScriptVM::PushInteger(tI32 anValue)
{
  sq_pushint(mptrVM, anValue);
  return eTrue;
}

///////////////////////////////////////////////
//! Push a float on the stack.
tBool __stdcall cScriptVM::PushFloat(tF64 afValue)
{
  sq_pushf64(mptrVM,afValue);
  return eTrue;
}

///////////////////////////////////////////////
//! Push an interface on the stack.
tBool __stdcall cScriptVM::PushInterface(iUnknown* apInterface)
{
  return SQ_SUCCEEDED(sqa_pushIUnknown(mptrVM,apInterface));
}

///////////////////////////////////////////////
//! Push a variant on the stack.
tBool __stdcall cScriptVM::PushVariant(const Var& aVariant)
{
  return SQ_SUCCEEDED(sqa_pushvar(mptrVM,aVariant));
}

///////////////////////////////////////////////
//! Push a script object on the stack.
tBool __stdcall cScriptVM::PushObject(iScriptObject* apObject)
{
  if (!niIsOK(apObject)) return eFalse;
  return static_cast<cScriptObject*>(apObject)->Push();
}

///////////////////////////////////////////////
//! Pop n object of the stack.
tBool __stdcall cScriptVM::Pop(tU32 anNumPop)
{
  sq_pop(mptrVM,anNumPop);
  return eTrue;
}

///////////////////////////////////////////////
//! Get the stack size.
tI32 __stdcall cScriptVM::GetStackSize() const
{
  return sq_gettop(mptrVM);
}

///////////////////////////////////////////////
//! Set the stack size.
tBool __stdcall cScriptVM::SetStackSize(tU32 anSize)
{
  sq_settop(mptrVM,anSize);
  return eTrue;
}

///////////////////////////////////////////////
//! Create a new slot.
tBool __stdcall cScriptVM::CreateSlot(tI32 idx)
{
  return SQ_SUCCEEDED(sq_createslot(mptrVM,idx));
}

///////////////////////////////////////////////
//! Create a new table.
//! \param  apDelegate is an optional delegate table.
//! \param  abSetAsActive, if eTrue the new table will be set as active.
//! \return NULL if the table's creation failed.
iScriptObject* __stdcall cScriptVM::CreateTable(iScriptObject* apDelegate, tI32 anNumPop)
{
  sq_newtable(mptrVM);
  iScriptObject* pTable = niNew cScriptObject(this,-1,0);
  if (!niIsOK(pTable))
  {
    niError(_A("Can't create table object."));
    return NULL;
  }

  if (niIsOK(apDelegate))
  {
    static_cast<cScriptObject*>(apDelegate)->Push();
    if (SQ_FAILED(sq_setdelegate(mptrVM,-2)))
    {
      sq_pop(mptrVM,1); // pop the new table
      niError(_A("Can't set the delegate table."));
      return NULL;
    }
  }

  if (anNumPop) sq_pop(mptrVM,anNumPop);  // pop the new table

  return pTable;
}

///////////////////////////////////////////////
tInterfaceCVec<iFileSystem>* __stdcall cScriptVM::GetImportFileSystems() const {
  return _ptrImportFileSystems;
}

///////////////////////////////////////////////
iFile* __stdcall cScriptVM::ImportFileOpen(const achar* aaszFile) {
  return ni::GetLang()->URLOpen(
      (StrFindProtocol(aaszFile) < 0)
      ? (niFmt("script://%s",aaszFile)) // use default 'script' protocol
      : (aaszFile) // use the protocol specified in the URL
  );
}

///////////////////////////////////////////////
//! Call the system import function.
Ptr<iScriptObject> __stdcall cScriptVM::_DoImport(tBool abNew, iUnknown* apPathOrFile)
{
  tU32 numPop = 0;
  niLet funName = abNew?_HC(NewImport):_HC(Import);

  // Get the closure
  sq_pushroottable(mptrVM);
  ++numPop;
  sq_pushstring(mptrVM,funName);
  if (!SQ_SUCCEEDED(sq_get(mptrVM,-2))) {
    niError(niFmt("Can't get the '%s' closure.", funName));
    Pop(numPop);
    return nullptr;
  }

  // push 'this'
  sq_pushroottable(mptrVM);
  ++numPop; // we'll have to pop this if the next call fails
  // push the first argument
  if (SQ_FAILED(sqa_pushIUnknown(mptrVM,apPathOrFile))) {
    niError(niFmt("Can't push apPathOrFile."));
    Pop(numPop);
    return nullptr;
  }
  --numPop; // we dont want to pop this...

  // call the import function
  if (SQ_FAILED(sq_call(mptrVM,2,1))) {
    niError(niFmt("Call to '%s' failed.", funName));
    Pop(numPop);
    return nullptr;
  }
  // Get the return value and pop it.
  Ptr<iScriptObject> ret = CreateObject(-1,1);

  // Pop whatever is left over
  Pop(numPop);
  return ret;
}
Ptr<iScriptObject> __stdcall cScriptVM::Import(iUnknown* apPathOrFile) {
  return this->_DoImport(ni::eFalse,apPathOrFile);
}
Ptr<iScriptObject> __stdcall cScriptVM::NewImport(iUnknown* apPathOrFile) {
  return this->_DoImport(ni::eTrue,apPathOrFile);
}

///////////////////////////////////////////////
//! Create a new script object.
iScriptObject* __stdcall cScriptVM::CreateObject(tI32 anIndex, tI32 anNumPop)
{
  return niNew cScriptObject(this,anIndex,anNumPop,eTrue);
}

///////////////////////////////////////////////
//! Get an object that of the active table.
//! \param  aaszKey is the name of the object to get.
//! \param  aRequiredType is the type that the object should have. If eScriptObjectType_Last this is ignored.
//! \return NULL if the object can't be found or that the type doesn't match aRequiredType.
iScriptObject* __stdcall cScriptVM::CreateObjectGet(const achar* aaszKey, eScriptObjectType aRequiredType, tI32 anNumPop, tBool abTry)
{
  sq_pushstring(mptrVM,_H(aaszKey));
  if (SQ_FAILED(sq_get(mptrVM,-2))) {
    if (!abTry) {
      niError(niFmt(_A("Can't get the object '%s'."), aaszKey));
    }
    return NULL;
  }

  cScriptObject* pObject = niNew cScriptObject(this,-1,0);
  if (!niIsOK(pObject))
  {
    sq_pop(mptrVM,1); // pop the object
    niError(niFmt(_A("Can't create script object '%s'."), aaszKey));
    return NULL;
  }
  if (anNumPop) sq_pop(mptrVM,anNumPop);  // pop the object

  if (aRequiredType != eScriptObjectType_Last && pObject->GetType() != aRequiredType)
  {
    if (!abTry) {
      niError(niFmt(
        "Object '%s' don't have the required type, %d expected, %d found.",
        aaszKey,
        aRequiredType,
        pObject->GetType()));
    }
    return NULL;
  }

  return pObject;
}

///////////////////////////////////////////////
//! Call a function.
tBool __stdcall cScriptVM::Call(tU32 anNumParams, tBool abPushRet)
{
  return SQ_SUCCEEDED(sq_call(mptrVM, anNumParams, abPushRet?1:0));
}

///////////////////////////////////////////////
//! Write a closure
tBool __stdcall cScriptVM::WriteClosure(iFile* apFile, iScriptObject* apObject)
{
  if (!niIsOK(apObject) || apObject->GetType() != eScriptObjectType_Function) {
    niError(_A("Invalid script object, must be a function."));
    return eFalse;
  }

  if (!niIsOK(apFile)) {
    niError(_A("Invalid file."));
    return eFalse;
  }

  apFile->WriteLE32(_fccClosure);
  PushObject(apObject);
  if (!SQ_SUCCEEDED(sq_writeclosure(mptrVM,sqWrite,(ni::tPtr)apFile)))
  {
    Pop(1);
    niError(_A("Can't write the closure."));
    return eFalse;
  }
  Pop(1);
  return eTrue;
}

///////////////////////////////////////////////
//! Read a closure
iScriptObject* __stdcall cScriptVM::ReadClosure(iFile* apFile)
{
  if (!niIsOK(apFile))
  {
    niError(_A("Invalid file."));
    return NULL;
  }

  Ptr<iFile> fp = apFile;

  if (apFile->ReadLE32() != _fccClosure) {
    niError(_A("Not a valid closure."));
    return NULL;
  }

  if (!SQ_SUCCEEDED(sq_readclosure(mptrVM,sqRead,(tPtr)apFile))) {
    niError(_A("Can't read the closure."));
    return NULL;
  }

  return CreateObject(-1,1);
}

///////////////////////////////////////////////
tBool __stdcall cScriptVM::ScriptCall(iScriptObject* apThis, const achar* aaszFunc, const Var* apParams, tU32 anParams, Var* apRet)
{
  niCheck(niIsStringOK(aaszFunc),eFalse);

  const tU32 nStartStackSize = this->GetStackSize();

  tBool bPushRoot = eFalse;
  astl::vector<cString> vTables;
  const achar* p = aaszFunc;
  // skip white spaces
  while (*p && *p == ' ') { ++p; }
  if (*p == ':' && *(p+1) == ':') {
    bPushRoot = eTrue;
    p += 2;
  }

  StringSplit(cString(p),_A("."),&vTables);

  if (vTables.empty()) {
    niError(niFmt(_A("No function name defined '%s'."),aaszFunc));
    return eFalse;
  }

  Ptr<iScriptObject> o;
  tU32 nNumPop = 0;
  if (bPushRoot) {
    this->PushRootTable();
    ++nNumPop;
  }
  else if (apThis) {
    this->PushObject(apThis);
    ++nNumPop;
  }
  niLoop(i,vTables.size()) {
    o = this->CreateObjectGet(vTables[i].Chars(),
                              i == (vTables.size()-1) ?
                              eScriptObjectType_Function :
                              eScriptObjectType_Table,0);
    if (!o.IsOK()) {
      niError(niFmt(_A("Can't get object key '%s' trying to call '%s'."),vTables[i].Chars(),aaszFunc));
      break;
    }
    ++nNumPop;
  }

  tBool bRet = eFalse;
  if (niIsOK(o)) {
    // this table of the call
    if (apThis) {
      this->PushObject(apThis);
    }
    else {
      this->PushRootTable();
    }
    // push the parameters
    tU32 numParams = 0;
    tBool bFailPushParams = eFalse;
    niLoop(i,o->GetNumCallParameters()-1) {
      if (i < anParams) {
        if (!this->PushVariant(apParams[i])) {
          niError(niFmt(_A("Calling '%s', can't push parameter '%d'."),aaszFunc,i));
          nNumPop += i+1; // parameters already push plus the this table
          break;
        }
      }
      else {
        sq_pushnull(mptrVM);
      }
      ++numParams;
    }
    if (!bFailPushParams) {
      if (!this->Call(numParams+1,apRet!=NULL)) {
        niError(niFmt(_A("Call to '%s' failed."),aaszFunc));
      }
      else {
        bRet = eTrue;
        if (apRet != NULL) {
          o = this->CreateObject(-1,1);
          if (!o.IsOK()) {
            niError(niFmt(_A("Calling '%s', can't get the return value."),aaszFunc));
            ++nNumPop;
          }
          else {
            switch (o->GetType()) {
              case eScriptObjectType_Int:
                *apRet = o->GetInt();
                break;
              case eScriptObjectType_Float:
                *apRet = o->GetFloat();
                break;
              case eScriptObjectType_String:
                *apRet = o->GetString();
                break;
              case eScriptObjectType_IUnknown:
                *apRet = o->GetIUnknown();
                break;
              case eScriptObjectType_Vec2:
                *apRet = o->GetVec2();
                break;
              case eScriptObjectType_Vec3:
                *apRet = o->GetVec3();
                break;
              case eScriptObjectType_Vec4:
                *apRet = o->GetVec4();
                break;
              case eScriptObjectType_Matrix:
                *apRet = o->GetMatrix();
                break;
              default:
                *apRet = niVarNull;
                break;
            }
          }
        }
      }
    }
  }

  if (nNumPop) {
    this->Pop(nNumPop);
  }

  const tU32 nEndStackSize = this->GetStackSize();
  niAssert(nEndStackSize == nStartStackSize);
  if (nEndStackSize != nStartStackSize) {
    niError(niFmt(_A("Calling '%s', corrupted stack, should be %d, is %d."),aaszFunc,nStartStackSize,nEndStackSize));
  }
  return bRet;
}

///////////////////////////////////////////////
Ptr<iScriptObject> __stdcall cScriptVM::ScriptVar(iScriptObject* apThis, const achar* aaszVar, tBool abTry)
{
  niCheck(niIsStringOK(aaszVar),NULL);
  const tU32 nStartStackSize = this->GetStackSize();

  tBool bPushRoot = eFalse;
  astl::vector<cString> vTables;
  const achar* p = aaszVar;
  // skip white spaces
  while (*p && *p == ' ') { ++p; }
  if (*p == ':' && *(p+1) == ':') {
    bPushRoot = eTrue;
    p += 2;
  }

  StringSplit(cString(p),_A("."),&vTables);
  if (vTables.empty()) {
    niError(niFmt(_A("No variable name defined '%s'."),aaszVar));
    return nullptr;
  }

  Ptr<iScriptObject> o;
  tU32 nNumPop = 0;
  if (bPushRoot) {
    this->PushRootTable();
    ++nNumPop;
  }
  else if (apThis) {
    this->PushObject(apThis);
    ++nNumPop;
  }

  niLoop(i,vTables.size()) {
    o = this->CreateObjectGet(vTables[i].Chars(),eScriptObjectType_Last,0,abTry);
    if (!o.IsOK()) {
      if (!abTry) {
        niError(niFmt(_A("Can't get object key '%s' trying to get '%s'."),vTables[i].Chars(),aaszVar));
      }
      break;
    }
    ++nNumPop;
  }

  if (nNumPop) {
    this->Pop(nNumPop);
  }

  const tU32 nEndStackSize = this->GetStackSize();
  if (nEndStackSize != nStartStackSize) {
    niError(niFmt(_A("Getting '%s', corrupted stack, should be %d, is %d."),aaszVar,nStartStackSize,nEndStackSize));
  }
  niAssert(nEndStackSize == nStartStackSize);
  return o.GetRawAndSetNull();
}

#ifdef SCRIPTOBJECT_INVALIDATEPOLICY
///////////////////////////////////////////////
void cScriptVM::RegisterScriptObject(cScriptObject* pObj)
{
  mlstScriptObjects.push_back(pObj);
}
///////////////////////////////////////////////
void cScriptVM::UnregisterScriptObject(cScriptObject* pObj)
{
  astl::find_erase(mlstScriptObjects,pObj);
}
#endif

///////////////////////////////////////////////
tBool __stdcall cScriptVM::EvalString(iHString* ahspContext, const ni::achar* aaszCode) {
  return Run(NULL,aaszCode);
}

///////////////////////////////////////////////
tBool __stdcall cScriptVM::CanEvalImpl(iHString* ahspContext, iHString* ahspCodeResource) {
  return StrEndsWithI(niHStr(ahspCodeResource),"ni");
}

///////////////////////////////////////////////
iUnknown* __stdcall cScriptVM::EvalImpl(iHString* ahspContext, iHString* ahspCodeResource, const tUUID& aIID) {
  Ptr<iFile> fp = this->ImportFileOpen(niHStr(ahspCodeResource));
  niCheck(fp.IsOK(),NULL);

  ni::Ptr<iUnknown> ptrImpl = NULL;

  {
    //
    // All of this *MUST* be in a local scope so that all the smart
    // pointers release their references *before* ptrImpl.GetRawAndSetNull() is called;
    // if not the smart pointers will release the table (i.e. the object
    // we're returing) after GetRawAndSetNull() which will look like
    // there's isn't any reference an so return garbage (a dangling
    // pointer to the table that's just been destroyed).
    //
    ni::Var ret;
    ni::Var params[3];
    params[0] = ahspContext;
    params[1] = ahspCodeResource;
    params[2] = aIID;
    if (!this->ScriptCall(NULL,_A("::interop.evalImpl"),params,3,&ret)) {
      niError("::interop.evalImpl failed.");
      return NULL;
    }
    if (ret.IsNull() || !ret.IsIUnknownPointer()) {
      niError("::interop.evalImpl didn't return a valid iUnknown object.");
      return NULL;
    }

    ptrImpl = ret.mpIUnknown->QueryInterface(aIID);
    if (!ptrImpl.IsOK()) {
      niError(niFmt("Can't query interface '%s'.",aIID));
      return NULL;
    }
  }

  return ptrImpl.GetRawAndSetNull();
}

///////////////////////////////////////////////
void __stdcall cScriptVM::Service(tBool abForceGC) {
  if (abForceGC)
    this->CollectGarbage();
}

///////////////////////////////////////////////
niExportFunc(ni::iUnknown*) New_niScript_ScriptVM(const ni::Var& avarA, const ni::Var& avarB) {
  if (avarA.IsIUnknownPointer()) {
    Ptr<iConcurrent> ptrConcurrent = ni::VarQueryInterface<iConcurrent>(avarA);
    if (ptrConcurrent.IsOK()) {
      if (concurrent_vm_is_started()) {
        niError("A ConcurrentVM is already started.");
        return NULL;
      }
      if (ptrConcurrent->GetMainThreadID() != ptrConcurrent->GetCurrentThreadID()) {
        niError("The ConcurrentVM must be created from the main thread.");
        return NULL;
      }

      Ptr<cScriptVM> ptrNewVM = niNew cScriptVM(NULL);
      if (!concurrent_vm_startup((HSQUIRRELVM)ptrNewVM->GetHandle(),ptrConcurrent)) {
        niError("Can't startup ConcurrentVM.");
        return NULL;
      }
      HSQUIRRELVM newvm = (HSQUIRRELVM)ptrNewVM->GetHandle();
      RegisterSQRegFunctions(
        as_nn(_table(newvm->_roottable)),
        SQSharedState::_concurrent_funcs);

#ifdef niDebug
      niDebugFmt(("ScriptVM[%p], concurrent VM initialized.",(tIntPtr)ptrNewVM.ptr()));
#endif
      return ptrNewVM.GetRawAndSetNull();
    }
    else {
      Ptr<iScriptVM> ptrParentVM = ni::VarQueryInterface<iScriptVM>(avarA);
      if (!ptrParentVM.IsOK()) {
        niError("Invalid parent VM.");
        return NULL;
      }
      if (ptrParentVM->GetParentVM()) {
        niError("Parent VM can't be a child VM.");
        return NULL;
      }
      return niNew cScriptVM(down_cast<cScriptVM*>(ptrParentVM.ptr()));
    }
  }
  else {
    return niNew cScriptVM(NULL);
  }
}
