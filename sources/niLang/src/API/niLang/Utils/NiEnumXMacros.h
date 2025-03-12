#pragma once
#ifndef __NIENUMXMACROS_H_45FEE666_D8C9_4400_A63B_A3E8C97C13B2__
#define __NIENUMXMACROS_H_45FEE666_D8C9_4400_A63B_A3E8C97C13B2__

namespace ni {

/*!
\example

Example usage:

```cpp
#define FOREACH_eXFileOpenMode(N,DO)             \
  DO(N, Read, niBit(0), )                        \
  DO(N, Write, niBit(1), )                       \
  DO(N, Append, niBit(2)|eXFileOpenMode_Write, ) \
  DO(N, Random, niBit(3), )

NI_XENUM(eXFileOpenMode);

NI_DECL_XENUM_DEF(eXFileOpenMode);
NI_IMPL_XENUM_DEF(eXFileOpenMode);

static void _DoRegister() {
  NI_REGISTER_XENUM_DEF(eXFileOpenMode);
}
```

*/

#define NI_XENUM_ENTRY(ENUMNAME, NAME, VALUE, ATTR) ENUMNAME##_##NAME ATTR = VALUE,
#define NI_XENUM(ENUMNAME) \
  enum ENUMNAME {                             \
    FOREACH_##ENUMNAME(ENUMNAME,NI_XENUM_ENTRY) \
  };

#define NI_DECL_XENUM_DEF(ENUM) \
  niExportFunc(const ni::sEnumDef*) GetEnumDef_##ENUM();

#define NI_REGISTER_XENUM_DEF(ENUM) \
  ni::GetLang()->RegisterEnumDef(GetEnumDef_##ENUM())

#define NI_XENUM_VALUE_DEF(ENUMNAME, name, value, attr) { #name, ENUMNAME##_##name },
#define NI_XENUM_DECLARE_VALUE_DEFS(ENUMNAME)     \
  static const ni::sEnumValueDef Enum_##ENUMNAME##_Values[] = { \
    FOREACH_##ENUMNAME(ENUMNAME, NI_XENUM_VALUE_DEF)                  \
  };

#define NI_IMPL_XENUM_DEF(ENUMNAME)                                 \
  niExportFunc(const ni::sEnumDef*) GetEnumDef_##ENUMNAME() {       \
    NI_XENUM_DECLARE_VALUE_DEFS(ENUMNAME);      \
    static const ni::sEnumDef Enum_##ENUMNAME = {                   \
      #ENUMNAME,                                                    \
      niCountOf(Enum_##ENUMNAME##_Values), Enum_##ENUMNAME##_Values \
    };                                                              \
    return &Enum_##ENUMNAME;                                        \
  }

}
#endif // __NIENUMXMACROS_H_45FEE666_D8C9_4400_A63B_A3E8C97C13B2__
