#ifndef __IGRAPHICSD3D9_14215215_H__
#define __IGRAPHICSD3D9_14215215_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#ifdef GDRV_DX9
#include "API/niUI/Utils/GraphicsDriverDX9.h"

namespace ni {
/** \addtogroup D3D9
 * @{
 */

const achar* GetD3D9ErrorString(HRESULT hr);
#define D3D9ERROR() niWarning(niFmt(_A("D3D9 Error: %s."), GetD3D9ErrorString(hr)))

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif
#endif // __IGRAPHICSD3D9_14215215_H__
