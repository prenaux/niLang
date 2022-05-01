#ifndef __SCRIPTVM_CONCURRENT_H_2CC08E3F_5B55_46D8_AC9C_FCD312E5F3CA__
#define __SCRIPTVM_CONCURRENT_H_2CC08E3F_5B55_46D8_AC9C_FCD312E5F3CA__

namespace ni {
struct iConcurrent;
}
tBool concurrent_vm_startup(HSQUIRRELVM rootVM, iConcurrent* apConcurrent);
void concurrent_vm_register(HSQUIRRELVM v);
void concurrent_vm_shutdown();
tBool concurrent_vm_is_started();
HSQUIRRELVM concurrent_vm_mainvm();
HSQUIRRELVM concurrent_vm_currentvm();

#endif // __SCRIPTVM_CONCURRENT_H_2CC08E3F_5B55_46D8_AC9C_FCD312E5F3CA__
