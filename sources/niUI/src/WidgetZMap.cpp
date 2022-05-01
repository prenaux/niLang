// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Widget.h"
#include "WidgetZMap.h"

cWidget* cWidgetZMap::GetTargetByPos(const sVec2f &pos,
                                     tBool (*apfnExcludeWidget)(const cWidget* w, tIntPtr apUserData),
                                     tIntPtr apUserData) const
{
  if (mmapZOrder.empty()) return NULL;
  for (tWidgetZMapCIt zit = mmapZOrder.begin(); zit != mmapZOrder.end(); ++zit) {
    for (tWidgetPtrDeqCRIt lit = zit->second.rbegin(); lit != zit->second.rend(); ++lit) {
      cWidget* ret = (*lit)->GetMessageTargetByPos(pos,apfnExcludeWidget,apUserData);
      if (ret)
        return ret;
    }
  }
  return NULL;
}
