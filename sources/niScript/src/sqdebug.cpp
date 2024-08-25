#include "stdafx.h"

#include <stdarg.h>
#include "sqvm.h"
#include "sqfuncproto.h"
#include "sqclosure.h"
#include "ScriptVM.h"

cString sq_getcallinfo_string(HSQUIRRELVM v, int level)
{
  cScriptVM* pVM = reinterpret_cast<cScriptVM*>(sq_getforeignptr(v));
  int cssize = v->_callsstack.size();
  if (cssize > level)
  {
    cString strFunc = _A("unknown"), strSource = _A("NOSOURCE");
    sVec2i lineCol {0,0};
    SQCallInfo &ci = v->_callsstack[cssize-level-1];
    switch (_sqtype(ci._closurePtr))
    {
      case OT_CLOSURE:
        {
          SQFunctionProto *func = _funcproto(_closure(ci._closurePtr)->_function);
          if (_sqtype(func->_name) == OT_STRING) {
            strFunc = _stringval(func->_name);
          }
          if (_sqtype(func->_sourcename) == OT_STRING) {
            strSource = _stringval(func->_sourcename);
          }
          lineCol = func->GetLineCol(ci._ip);
          break;
        }
      case OT_NATIVECLOSURE:
        {
          strSource = _A("NATIVE");
          strFunc = _A("unknown");
          if (HStringIsNotEmpty(_nativeclosure(ci._closurePtr)->_name))
            strFunc = niHStr(_nativeclosure(ci._closurePtr)->_name);
          lineCol = {-1,-1};
          break;
        }
      default:
        return AZEROSTR;
    }
    return niFmt(_A("%s(%d:%d): FUNCTION [%s]"), strSource.Chars(),
                 lineCol.x + pVM->GetErrorLineOffset(), lineCol.y,
                 strFunc.Chars());
  }
  return AZEROSTR;
}


niExportFunc(SQRESULT) sq_stackinfos(HSQUIRRELVM v, int level, SQStackInfos *si)
{
  int cssize = v->_callsstack.size();
  if (cssize > level)
  {
    memset(si, 0, sizeof(SQStackInfos));
    SQCallInfo &ci = v->_callsstack[cssize-level-1];
    switch (_sqtype(ci._closurePtr))
    {
      case OT_CLOSURE:
        {
          SQFunctionProto *func = _funcproto(_closure(ci._closurePtr)->_function);
          if (_sqtype(func->_name) == OT_STRING)
            si->funcname = _stringval(func->_name);
          if (_sqtype(func->_sourcename) == OT_STRING)
            si->source = _stringval(func->_sourcename);
          si->lineCol = func->GetLineCol(ci._ip);
          break;
        }
      case OT_NATIVECLOSURE:
        {
          si->source = _A("NATIVE");
          si->funcname = _A("unknown");
          if (HStringIsNotEmpty(_nativeclosure(ci._closurePtr)->_name))
            si->funcname = niHStr(_nativeclosure(ci._closurePtr)->_name);
          si->lineCol = {-1,-1};
          break;
        }
    }
    return SQ_OK;
  }
  return SQ_ERROR;
}
