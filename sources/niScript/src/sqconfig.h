#ifndef __SQCONFIG_H_D9768A43_C462_48A5_901A_44C03ABDBC90__
#define __SQCONFIG_H_D9768A43_C462_48A5_901A_44C03ABDBC90__

//
// NO_GARBAGE_COLLECTOR is disabled everywhere because until now it doesn't
// really add any benefit to not have it, performances (on a real application)
// are the same, and memory usage is only marginally lower without it.
//
// Later however, it would be worth investigating whether removing it entierly
// could be a good idea. Using .invalidate()/clear() actually forcing everything
// contained to be invalidated as-well could be some kind of solution... (However keep
// in mind in that case that you probably don't want all content in tables to
// be invalidated, such as C++ objects, or even other tables, so it needs to be
// thouroughly evaluated)
//
// #define NO_GARBAGE_COLLECTOR

#include <niLang/STL/set.h>
#include <niLang/Utils/UnknownImpl.h>
#include <niLang/StringDef.h>
#include <niLang/ILang.h>
#include "API/niScript/VMAPI.h"
#include "API/niScript/IScriptObject.h"
#include "API/niScript/IScriptVM.h"
using namespace ni;

extern tBool _bDoDebugDump;
#if defined niDebug && !defined niEmbedded
#define _DEBUG_DUMP
#endif

#endif // __SQCONFIG_H_D9768A43_C462_48A5_901A_44C03ABDBC90__
