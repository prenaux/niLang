#ifndef __VARCAST_H_5E31ACE2_90D4_4A19_A233_0C5CAE277E84__
#define __VARCAST_H_5E31ACE2_90D4_4A19_A233_0C5CAE277E84__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Var.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

///////////////////////////////////////////////
struct VarCast : public ni::Var {
#define DO_VAR_CAST_(TYPE,TYPEB,TYPECAST)      \
  {                                             \
    if (this->Is##TYPE##Value()) {              \
      return TYPECAST this->Get##TYPE##Value(); \
    }                                           \
    else {                                      \
      ni::Var t = *this;                        \
      VarConvertType(t,eType_##TYPEB);          \
      return t.Get##TYPEB();                    \
    }                                           \
  }

#define DO_VAR_CAST(TYPE,TYPEB) DO_VAR_CAST_(TYPE,TYPEB,(t##TYPEB))

#if defined niTypeIntIsOtherType
  operator int () const { DO_VAR_CAST(Int,I32); }
  operator unsigned () const { DO_VAR_CAST(Int,U32); }
#endif

  operator ni::tI8 () const { DO_VAR_CAST(Int,I8); }
  operator ni::tU8 () const { DO_VAR_CAST(Int,U8); }
  operator ni::tI16 () const { DO_VAR_CAST(Int,I16); }
  operator ni::tU16 () const { DO_VAR_CAST(Int,U16); }
  operator ni::tI32 () const { DO_VAR_CAST(Int,I32); }
  operator ni::tU32 () const { DO_VAR_CAST(Int,U32); }
  operator ni::tI64 () const { DO_VAR_CAST(Int,I64); }
  operator ni::tU64 () const { DO_VAR_CAST(Int,U64); }
  operator ni::tF32 () const { DO_VAR_CAST(Float,F32); }
  operator ni::tF64 () const { DO_VAR_CAST(Float,F64); }
  operator ni::sVec2f () const { DO_VAR_CAST_(Vec2f,Vec2f,); }
  operator ni::sVec2i () const { DO_VAR_CAST_(Vec2i,Vec2i,); }
  operator ni::sVec3f () const { DO_VAR_CAST_(Vec3f,Vec3f,); }
  operator ni::sVec3i () const { DO_VAR_CAST_(Vec3i,Vec3i,); }
  operator ni::sVec4f () const { DO_VAR_CAST_(Vec4f,Vec4f,); }
  operator ni::sVec4i () const { DO_VAR_CAST_(Vec4i,Vec4i,); }
  operator ni::sMatrixf () const {
    if (this->IsMatrixf()) {
      return this->GetMatrixf();
    }
    return ni::sMatrixf::Identity();
  }
  operator ni::tHStringPtr () const {
    return ni::VarGetHString(*this);
  }

  template <typename T>
  operator T* () const {
    return ni::VarQueryInterface<T>(*this);
  }

  template <typename T>
  operator const T* () const {
    return ni::VarQueryInterface<T>(*this);
  }

#undef DO_VAR_CAST_
#undef DO_VAR_CAST
};

#define niVarCast(O) reinterpret_cast<const VarCast&>(O)
#define niVarCastTo(TYPE,O) ((TYPE)reinterpret_cast<const VarCast&>(O))

/**@}*/
/**@}*/
}
#endif // __VARCAST_H_5E31ACE2_90D4_4A19_A233_0C5CAE277E84__
