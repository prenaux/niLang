#pragma once
#ifndef __UTILS_EXCEPTION_H_6891C177_C46B_F647_9041_9002ADFB506C__
#define __UTILS_EXCEPTION_H_6891C177_C46B_F647_9041_9002ADFB506C__

#include "../Types.h"

namespace ni {

struct iHString;
template<class U> struct Nonnull;

/** \addtogroup niLang
 * @{
 */

#define _HSymGetName(X) GetHString_##X
#define _HSymExport(X) niExportFunc(const ni::iHString*) _HSymGetName(X)()
#define _HSymImpl_(VARNAME,STRING)                        \
  _HSymExport(VARNAME) {                                  \
    static ni::tHStringPtr _hstr_##VARNAME = _H(STRING);  \
    return _hstr_##VARNAME.raw_ptr();                     \
  }
#define _HSymImpl(X) _HSymImpl_(X,#X)
#define _HSymGet(X) _HSymGetName(X)()
#define _HSymGetNS(NS,X) NS::_HSymGet(X)

_HSymExport(panic);

// \remark This is not a iunknown as it should go around by value.
struct iException {
  // \remark Always return a valid object
  virtual const iHString* __stdcall GetKind() const noexcept = 0;
  // \remark Always return a valid string, but can be empty / AZEROSTR
  virtual const char* __stdcall GetMsg() const noexcept = 0;
};

struct __ni_module_export sPanicException final : public std::exception, public iException {
  sPanicException(const sPanicException& aRight) noexcept = delete;
  sPanicException(sPanicException&& aRight) noexcept;
  sPanicException(const iHString* aKind, const char* aMsg) noexcept;
  virtual ~sPanicException();

  // std::exception
  virtual const char* what() const noexcept override final;

  // ni::iException
  virtual const iHString* __stdcall GetKind() const noexcept override final;
  virtual const char* __stdcall GetMsg() const noexcept override final;

private:
  const iHString* _kind;
  const char* _msg;
};

niExportFuncCPP(void) ni_throw_panic(const char* file, int line, const char* func,sPanicException&& exc);

#define niThrowPanic(KIND) ni_throw_panic(__FILE__,__LINE__,__FUNCTION__,ni::sPanicException{_HSymGet(KIND),""})
#define niThrowPanicMsg(KIND,MSG) ni_throw_panic(__FILE__,__LINE__,__FUNCTION__,ni::sPanicException{_HSymGet(KIND),MSG})

#if defined niNoExceptions
#define niCatchPanic(EXC) if(const ni::sPanicException EXC = ni::sPanicException{nullptr,""}; 0)
#else
#define niCatchPanic(EXC) catch (const ni::sPanicException& EXC)
#endif

#define niCheckThrowPanic(exp)                                          \
  niAssume(exp);                                                        \
  if (!(exp)) {                                                         \
    ni_throw_panic(__FILE__,__LINE__,__FUNCTION__,ni::sPanicException{_HSymGetNS(ni,panic),#exp}); \
  }

#define niCheckThrowPanicMsg(exp,msg)                                   \
  niAssume(exp);                                                        \
  if (!(exp)) {                                                         \
    ni_throw_panic(__FILE__,__LINE__,__FUNCTION__,ni::sPanicException{_HSymGetNS(ni,panic),msg}); \
  }

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __UTILS_EXCEPTION_H_6891C177_C46B_F647_9041_9002ADFB506C__
