#pragma once
#ifndef __UTILS_EXCEPTION_H_6891C177_C46B_F647_9041_9002ADFB506C__
#define __UTILS_EXCEPTION_H_6891C177_C46B_F647_9041_9002ADFB506C__

#include "../ILang.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */

struct __ni_module_export sPanicException : public std::exception {
  sPanicException(const iHString* aKind, const char* aMsg) noexcept;

  // std::exception
  virtual const char* what() const noexcept override;

  const tHStringPtr _kind;
  const cString _msg;
};

// So that you can do _HC(panic) & niThrowPanic(panic,MSG)
niExportFunc(ni::iHString*) GetHString_panic();

niExportFuncCPP(void) ni_throw_panic(const char* file, int line, const char* func,sPanicException&& exc);

#if defined niNoExceptions
#define niCatchPanic(EXC) if(const sPanicException EXC = sPanicException{nullptr,""}; 0)
#else
#define niCatchPanic(EXC) catch (const sPanicException& EXC)
#endif

#define niThrowPanic(KIND) ni_throw_panic(__FILE__,__LINE__,__FUNCTION__,sPanicException{_HC(KIND),""})

#define niThrowPanicMsg(KIND,MSG) ni_throw_panic(__FILE__,__LINE__,__FUNCTION__,sPanicException{_HC(KIND),MSG})

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __UTILS_EXCEPTION_H_6891C177_C46B_F647_9041_9002ADFB506C__
