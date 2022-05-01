#ifndef __UNITSNAP_16822972_H__
#define __UNITSNAP_16822972_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Math/MathFloat.h>

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

inline tF32 UnitSnapf(tF32 v) {
  return (tF32)ni::FloatToIntNearest(v);
}
inline tI32 UnitSnapi(tF32 v) {
  return ni::FloatToIntNearest(v);
}
inline sVec2f UnitSnapf(const sVec2f& v) {
  sVec2f r;
  r.x = UnitSnapf(v.x);
  r.y = UnitSnapf(v.y);
  return r;
}
inline sVec2i UnitSnapi(const sVec2f& v) {
  sVec2i r;
  r.x = UnitSnapi(v.x);
  r.y = UnitSnapi(v.y);
  return r;
}
inline sVec3f UnitSnapf(const sVec3f& v) {
  sVec3f r;
  r.x = UnitSnapf(v.x);
  r.y = UnitSnapf(v.y);
  r.z = UnitSnapf(v.z);
  return r;
}
inline sVec3i UnitSnapi(const sVec3f& v) {
  sVec3i r;
  r.x = UnitSnapi(v.x);
  r.y = UnitSnapi(v.y);
  r.z = UnitSnapi(v.z);
  return r;
}
inline sVec4f UnitSnapf(const sVec4f& v) {
  sVec4f r;
  r.x = UnitSnapf(v.x);
  r.y = UnitSnapf(v.y);
  r.z = UnitSnapf(v.z);
  r.w = UnitSnapf(v.w);
  return r;
}
inline sVec4i UnitSnapi(const sVec4f& v) {
  sVec4i r;
  r.x = UnitSnapi(v.x);
  r.y = UnitSnapi(v.y);
  r.z = UnitSnapi(v.z);
  r.w = UnitSnapi(v.w);
  return r;
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __UNITSNAP_16822972_H__
