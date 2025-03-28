// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/ILang.h"
#include "API/niLang/IMath.h"
#include "API/niLang/Utils/UnknownImpl.h"
#define MATH_IMPL_INCLUDE_ONLY
#include "API/niLang/Math/MathImpl.h"

#define IMPL_TRANSFORM(TYPE, VEC, TRAN)             \
  niAssert(t##TYPE##CVec::IsSameType(VEC));         \
  niCheckSilent(t##TYPE##CVec::IsSameType(VEC), ;); \
  s##TYPE* pData = apVecs->_Data();                 \
  niLoop (i, pData->size()) {                       \
    ni::TRAN(*pData, *pData, aMatrix);              \
    ++pData;                                        \
  }

using namespace ni;

class cMath : public ImplLocal<iMath> {
  niBeginClass(cMath);

 public:
  //// iMath ////////////////////////////////////
#define MATH_IMPL_FORWARDS 1
#define MATH_IMPL_BEGIN
#define MATH_IMPL_END
#define MATH_IMPL_FN(T) [[nodiscard]] T __stdcall
#define MATH_IMPL_FN_VOID void __stdcall
#define MATH_IMPL_CONST_OVR const niImpl
#include "API/niLang/Math/MathImpl.h"
  //// iMath ////////////////////////////////////

 private:
  niEndClass(cMath);
};

namespace ni {

niExportFunc(ni::iMath*) GetMath()
{
  static cMath _math;
  return &_math;
}

niExportFunc(ni::iUnknown*) New_niLang_Math(const ni::Var&, const ni::Var&)
{
  return GetMath();
}

} // namespace ni
