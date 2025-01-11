#pragma once
#ifndef __DLLLOADER_H_B8DB559B_A075_EF11_AA74_1BE41DF5A1EB__
#define __DLLLOADER_H_B8DB559B_A075_EF11_AA74_1BE41DF5A1EB__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"
#include "../StringDef.h"
#include "../STL/scope_guard.h"
#include "ThreadImpl.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

/**
@brief Example of using DLLLoader to load dynamic libraries.

@section Example

In the symbols header file (`ni_dll_sym_glx.h`):
- Declare the functions you want to load using the `NI_DLL_PROC` macro. This allows defining the entry points in a single place.

@code
// ni_dll_sym_glx.h
NI_DLL_PROC(GLXContext, __cdecl, glXCreateContext, (Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct));
NI_DLL_PROC(Bool, __cdecl, glXMakeCurrent, (Display *dpy, GLXDrawable drawable, GLXContext ctx));
NI_DLL_PROC(void, __cdecl, glXDestroyContext, (Display *dpy, GLXContext ctx));
NI_DLL_PROC(XVisualInfo*, __cdecl, glXChooseVisual, (Display *dpy, int screen, int *attribList));
NI_DLL_PROC(void, __cdecl, glXSwapBuffers, (Display *dpy, GLXDrawable drawable));
@endcode

In the implementation file (`example_glx.cpp`):
- Redefine the `NI_DLL_PROC` macro using `NI_DLL_PROC_DECL` to declare the function pointers and include the symbols header (`ni_dll_sym_glx.h`).
- Use `NI_DLL_BEGIN_LOADER` and `NI_DLL_END_LOADER` to define the loading process.
- Redefine the `NI_DLL_PROC` macro again using `NI_DLL_PROC_LOAD` to load each function.
- Ensure `ni_dll_load_glx()` is called before using any loaded function, as it will load the library if it hasn't been loaded already.
- Use `dll_FUNCNAME` as function name to call the function loaded from the dll.

@code
////////////////////////////////////////////////////////////////////////////
// ni_dll_load_glx
#define NI_DLL_PROC(RET, CALLCONV, NAME, PARAMS) NI_DLL_PROC_DECL(RET, CALLCONV, NAME, PARAMS)
#include "ni_dll_sym_glx.h"
#undef NI_DLL_PROC

NI_DLL_BEGIN_LOADER(glx, "libGL.so");
#define NI_DLL_PROC(RET, CALLCONV, NAME, PARAMS) NI_DLL_PROC_LOAD(RET, CALLCONV, NAME, PARAMS)
#include "ni_dll_sym_glx.h"
#undef NI_DLL_PROC
NI_DLL_END_LOADER(glx);
@endcode

To use the dynamically loaded functions:
- Call `ni_dll_load_glx()` before using any of the `dll_` functions.

@code
tBool InitGLX() {
  // Load the GLX library if not already loaded
  niCheck(ni_dll_load_glx(), eFalse);

  // Use the GLX library
  XVisualInfo* xvi = dll_glXChooseVisual(display, 0, nullptr);
  if (!xvi) {
    niError("glXChooseVisual failed.");
    return eFalse;
  }

  // ... more code ...
  return eTrue;
}
@endcode

*/

typedef void* (*tpfnDLLGetProcAddress)(const char* aName);

struct sDLLLoader {
  const cString _dllName;
  const cString _fileName;
  ni::ThreadMutex _loadMutex;
  tIntPtr _dllHandle = 0;
  astl::vector<cString> _loadErrors;
  tBool _isLoaded = eFalse;
  struct {
    tU32 _dll = 0;
    tU32 _custom = 0;
    tU32 _static = 0;
  } _numLoaded;

  sDLLLoader(const achar* aName, const achar* aFileName)
      : _dllName(aName)
      , _fileName(aFileName) {
  }

  ~sDLLLoader() {
    _FreeHandle();
  }

  void _AddError(const achar* aError) {
    _loadErrors.push_back(aError);
  }

  tBool _LoadHandle() {
#ifndef niNoDLL
    niPanicAssert(_dllHandle == 0);
    if (_fileName.empty()) {
      niLog(Info, niFmt("DLLLoader: %s: Using static methods.", _dllName, _fileName));
      return eTrue;
    }
    _dllHandle = ni_dll_load(_fileName.Chars());
    if (!_dllHandle) {
      _AddError(niFmt("DLLLoader: %s: ni_dll_load of '%s' failed.", _dllName, _fileName));
      return eFalse;
    }
    else {
      niLog(Info, niFmt("DLLLoader: %s: Loaded dll '%s'.", _dllName, _fileName));
    }
    return eTrue;
#else
    return eTrue;
#endif
  }

  void _FreeHandle() {
#ifndef niNoDLL
    if (_dllHandle) {
      niLog(Info, niFmt("DLLLoader: %s: freed dll '%s'.", _dllName, _fileName));
      ni_dll_free(_dllHandle);
      _dllHandle = 0;
    }
#endif
  }

  tBool IsLoaded() const {
    return _isLoaded;
  }

  tBool HasLoadError() const {
    return !_loadErrors.empty();
  }

  tBool BeginLoad() {
#ifndef niNoDLL
    _loadErrors.clear();
    _loadMutex.ThreadLock();
    niZeroMember(_numLoaded);
    // once we begin loading we mark it as loaded, the mutex makes sure that
    // we dont have a race condition if this is called from multiple thread
    _isLoaded = eTrue;
    return _LoadHandle();
#else
    return ni::eTrue;
#endif
  }

  tBool EndLoad() {
#ifndef niNoDLL
    if (HasLoadError()) {
      for (auto& err : _loadErrors) {
        niError(err.Chars());
      }
      niLog(Info, niFmt(
              "DLLLoader: %s: Loading failed with %d errors. Loaded %d from dll, %d from custom, %d statically.",
              _dllName, _loadErrors.size(),
              _numLoaded._dll, _numLoaded._custom, _numLoaded._static));
      _FreeHandle();
    }
    else {
      niLog(Info, niFmt(
              "DLLLoader: %s: Loaded successfully. Loaded %d from dll, %d from custom, %d statically.",
              _dllName, _numLoaded._dll, _numLoaded._custom, _numLoaded._static));
    }
    _loadMutex.ThreadUnlock();
    return !HasLoadError();
#else
    _isLoaded = eTrue;
    return ni::eTrue;
#endif
  }

  void* LoadProc(const achar* aProcName, tBool abOptional = eFalse) {
#ifndef niNoDLL
    void* r = _dllHandle ? ni_dll_get_proc(_dllHandle, aProcName) : nullptr;
    if (!r) {
      if (!abOptional) {
        _AddError(niFmt("DLLLoader: %s: Can't load proc '%s' from handle '%p'.", _dllName, aProcName, (tIntPtr)_dllHandle));
      }
      return nullptr;
    }
    ++_numLoaded._dll;
    return r;
#else
    return NULL;
#endif
  }

  void* LoadProcCustom(const achar* aProcName, tpfnDLLGetProcAddress apfnLoadProc, tBool abOptional = eFalse, const achar* aaszExt = "") {
#ifndef niNoDLL
    void* r = apfnLoadProc(niFmt("%s%s",aProcName,aaszExt));
    if (!r) {
      if (!abOptional) {
        _AddError(niFmt("DLLLoader: %s: Can't load proc '%s' with custom GetProcAddress.", _dllName, aProcName));
      }
      return nullptr;
    }
    ++_numLoaded._custom;
    return r;
#else
    return NULL;
#endif
  }
};

#define NI_DLL_BEGIN_LOADER(NAME,FILENAME) static ni::tBool ni_dll_load_##NAME() { \
  static ni::sDLLLoader _dllLoader(#NAME,FILENAME);                     \
  if (_dllLoader.IsLoaded())                                            \
    return !_dllLoader.HasLoadError();                                  \
  niDefer {                                                             \
    _dllLoader.EndLoad();                                               \
  };                                                                    \
  niCheck(_dllLoader.BeginLoad(),ni::eFalse);                           \

#define NI_DLL_END_LOADER(NAME)   return !_dllLoader.HasLoadError(); }

#define NI_DLL_PROC_DECL(RET, CALLCONV, NAME, PARAMS) typedef RET (CALLCONV *tpfn_##NAME) PARAMS; static tpfn_##NAME dll_##NAME = nullptr
#define NI_DLL_PROC_LOAD(RET, CALLCONV, NAME, PARAMS) dll_##NAME = (tpfn_##NAME)_dllLoader.LoadProc(#NAME)

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
} // end of namespace ni
#endif // __DLLLOADER_H_B8DB559B_A075_EF11_AA74_1BE41DF5A1EB__
