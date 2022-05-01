#ifndef __JNIHEADERS_H_D7F98C9F_E92A_46CE_8F91_9BEDEC775FC5__
#define __JNIHEADERS_H_D7F98C9F_E92A_46CE_8F91_9BEDEC775FC5__

#include "../Types.h"

/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_JNI
 * @{
 */

#ifdef niJNI

#include "../Var.h"

#include <jni.h>
#undef JNIEXPORT

#if defined niGCC || defined niCLang
#define JNIEXPORT extern "C" __attribute__ ((visibility("default")))
#elif defined niMSVC
#define JNIEXPORT extern "C" __declspec(dllexport)
#else
#error "Unknown platform for JNI build."
#endif

#endif // #ifdef niJNI

/**@}*/
/**@}*/
#endif // __JNIHEADERS_H_D7F98C9F_E92A_46CE_8F91_9BEDEC775FC5__
