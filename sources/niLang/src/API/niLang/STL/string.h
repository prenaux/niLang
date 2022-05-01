#pragma once
#ifndef __STRING_H_3D75768D_9830_404B_9A9F_C41957BFF980__
#define __STRING_H_3D75768D_9830_404B_9A9F_C41957BFF980__

#include "../StringBase.h"
#include "../StringDef.h"

namespace astl {

typedef ni::cString string;

template <typename T>
inline ni::cString to_string(const T& v) {
  ni::cString o;
  o << v;
  return o;
}

}
#endif // __STRING_H_3D75768D_9830_404B_9A9F_C41957BFF980__
