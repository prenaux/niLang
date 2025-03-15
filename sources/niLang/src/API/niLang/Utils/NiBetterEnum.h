#pragma once
#ifndef __BETTERENUM_H_20E95912_F00D_4A66_98FC_AE200B62FCB3__
#define __BETTERENUM_H_20E95912_F00D_4A66_98FC_AE200B62FCB3__

#include "NiBetterEnumImpl.h"

namespace ni {

/*!
\example

Example usage:

```cpp
#include <niLang/Utils/NiEnumXMacros.h>

NI_BETTER_ENUM(eBetterFileOpenMode, ni::tU32,
  //! Read open mode.
  Read = niBit(0),
  //! Write open mode.
  Write = niBit(1),
  //! Append open mode.
  Append = niBit(2)|Write,
  //! Optimized for random access.
  Random = niBit(3)
);

NI_DECL_BETTER_ENUM_DEF(eBetterFileOpenMode);
NI_IMPL_BETTER_ENUM_DEF(eBetterFileOpenMode);

static void _DoRegister() {
  NI_REGISTER_BETTER_ENUM_DEF(eBetterFileOpenMode);
}
```

*/


#define NI_BETTER_ENUM(NAME,TYPE,...) BETTER_ENUM(NAME,TYPE,__VA_ARGS__)

#define NI_DECL_BETTER_ENUM_DEF(ENUM) \
  niExportFunc(const ni::sEnumDef*) GetEnumDef_##ENUM();

#define NI_IMPL_BETTER_ENUM_DEF(ENUM) \
  niExportFunc(const ni::sEnumDef*) GetEnumDef_##ENUM() { \
    return ni::MakeEnumDef<ENUM>(); \
  }

#define NI_REGISTER_BETTER_ENUM_DEF(ENUM) \
  ni::GetLang()->RegisterEnumDef(GetEnumDef_##ENUM())

template <typename T>
concept ctBetterEnum = requires {
  { T::_size() } -> std::convertible_to<std::size_t>;
  { T::_values() }; // Returns _value_iterable, not const T*
  { T::_values()[0] } -> std::convertible_to<T>; // Can index into it
  { T::_names() }; // Returns _name_iterable
  { T::_name() } -> std::convertible_to<const char*>;
};

template <ctBetterEnum TENUM>
struct enum_traits {
  constexpr static size_t size = TENUM::_size();

  constexpr static TENUM get_value(size_t index) {
    return TENUM::_values()[index];
  }

  constexpr static const char* get_identifier(size_t index) {
    return TENUM::_names()[index];
  }

  constexpr static TENUM get_last_value() {
    return get_value(size - 1);
  }
};

template<ctBetterEnum E, size_t... I>
constexpr auto MakeEnumValueDefs(std::index_sequence<I...>) {
  static const ni::sEnumValueDef values[] = {
    { E::_names()[I], E::_values()[I] }...
  };
  return values;
}

template<ctBetterEnum E>
constexpr const ni::sEnumDef* MakeEnumDef() {
  constexpr auto size = E::_size();
  static const auto values = MakeEnumValueDefs<E>(std::make_index_sequence<size>());
  static const ni::sEnumDef def = {
    E::_name(),
    size,
    values
  };
  return &def;
}

}
#endif // __BETTERENUM_H_20E95912_F00D_4A66_98FC_AE200B62FCB3__
