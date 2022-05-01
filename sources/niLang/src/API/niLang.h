#ifndef __NILANG_H_14DE26AB_EC18_4028_98ED_D24D1BB917D8__
#define __NILANG_H_14DE26AB_EC18_4028_98ED_D24D1BB917D8__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "niLang/Types.h"
#include "niLang/Utils/SmartPtr.h"
#include "niLang/Utils/QPtr.h"
#include "niLang/ICollection.h"
#include "niLang/IFile.h"
#include "niLang/IFileSystem.h"
#include "niLang/ITime.h"
#include "niLang/IRegex.h"
#include "niLang/IToString.h"
#include "niLang/StringLib.h"
#include "niLang/StringBase.h"
#include "niLang/StringDef.h"
#include "niLang/IHString.h"
#include "niLang/ObjModel.h"
#include "niLang/Utils/SinkList.h"

#include "niLang/Math/MathVec2.h"
#include "niLang/Math/MathVec3.h"
#include "niLang/Math/MathVec4.h"
#include "niLang/Math/MathRect.h"
#include "niLang/Math/MathPlane.h"
#include "niLang/Math/MathUtils.h"

#include "niLang/IOSProcess.h"

#include "niLang/IDeviceResource.h"
#include "niLang/ISerializable.h"
#include "niLang/IStringTokenizer.h"
#include "niLang/IDataTable.h"
#include "niLang/IGameCtrl.h"
#include "niLang/IOSProcess.h"
#include "niLang/IConsole.h"

#include "niLang/ILang.h"
#include "niLang/Var.h"

#include "niLang/Utils/UnknownImpl.h"
#include "niLang/Utils/CollectionImpl.h"

#include "niLang/Utils/Path.h"
#include "niLang/Utils/FourCC.h"
#include "niLang/Utils/UUID.h"

#include "niLang/Utils/ObjModelImpl.h"

#include "niLang/ICrypto.h"
#include "niLang/IConcurrent.h"

namespace ni {
niExportFunc(const ni::iModuleDef*) GetModuleDef_niLang();
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __NILANG_H_14DE26AB_EC18_4028_98ED_D24D1BB917D8__
