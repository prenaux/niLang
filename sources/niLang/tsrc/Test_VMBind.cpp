#include "stdafx.h"
#include "../src/API/niLang/Utils/VMBind.h"

namespace ni { namespace vmbind {

DECLARE_VMBIND_PARAM_ENUM_TRAITS(ni::eFileOpenMode, eType_Enum, "eFileOpenMode");

}}

namespace _ {
using namespace ni;

niExportFunc(const ni::sInterfaceDef*) ManualInterfaceDef_iFileSystem() {

// Method: GetRightsFlags
static const ni::sMethodDef iFileSystem_GetRightsFlags = {
  "GetRightsFlags",
  ni::eTypeFlags_MethodGetter|0|ni::eType_Enum, NULL, "tFileSystemRightsFlags",
  0, NULL,
  NULL
};

// Method: GetBaseContainer
static const ni::sMethodDef iFileSystem_GetBaseContainer = {
  "GetBaseContainer",
  ni::eTypeFlags_MethodGetter|0|ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer, NULL, "const achar*",
  0, NULL,
  NULL
};

// Method: FileMakeDir
static const ni::sParameterDef iFileSystem_FileMakeDir_Parameters[1] = {
  { "aszDir", ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer, NULL, "const achar*" }
};
static const ni::sMethodDef iFileSystem_FileMakeDir = {
  "FileMakeDir",
  0|ni::eType_I8, NULL, "tBool",
  1, iFileSystem_FileMakeDir_Parameters,
  NULL
};

// Method: FileDeleteDir
static const ni::sParameterDef iFileSystem_FileDeleteDir_Parameters[1] = {
  { "aszDir", ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer, NULL, "const achar*" }
};
static const ni::sMethodDef iFileSystem_FileDeleteDir = {
  "FileDeleteDir",
  0|ni::eType_I8, NULL, "tBool",
  1, iFileSystem_FileDeleteDir_Parameters,
  NULL
};

// Method: FileCopy
static const ni::sParameterDef iFileSystem_FileCopy_Parameters[2] = {
  { "aszDest", ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer, NULL, "const achar*" },
  { "aszSrc", ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer, NULL, "const achar*" }
};
static const ni::sMethodDef iFileSystem_FileCopy = {
  "FileCopy",
  0|ni::eType_I8, NULL, "tBool",
  2, iFileSystem_FileCopy_Parameters,
  NULL
};

// Method: FileMove
static const ni::sParameterDef iFileSystem_FileMove_Parameters[2] = {
  { "aszDest", ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer, NULL, "const achar*" },
  { "aszSrc", ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer, NULL, "const achar*" }
};
static const ni::sMethodDef iFileSystem_FileMove = {
  "FileMove",
  0|ni::eType_I8, NULL, "tBool",
  2, iFileSystem_FileMove_Parameters,
  NULL
};

// Method: FileDelete
static const ni::sParameterDef iFileSystem_FileDelete_Parameters[1] = {
  { "aszFile", ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer, NULL, "const achar*" }
};
static const ni::sMethodDef iFileSystem_FileDelete = {
  "FileDelete",
  0|ni::eType_I8, NULL, "tBool",
  1, iFileSystem_FileDelete_Parameters,
  NULL
};

// Method: FileEnum
static const ni::sParameterDef iFileSystem_FileEnum_Parameters[3] = {
  { "aszFile", ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer, NULL, "const achar*" },
  { "aAttribs", ni::eType_U32, NULL, "tU32" },
  { "pSink", ni::eType_IUnknown|ni::eTypeFlags_Pointer, &niGetInterfaceUUID(iFileEnumSink), "iFileEnumSink*" }
};
static const ni::sMethodDef iFileSystem_FileEnum = {
  "FileEnum",
  0|ni::eType_U32, NULL, "tU32",
  3, iFileSystem_FileEnum_Parameters,
  NULL
};

// Method: FileExists
static const ni::sParameterDef iFileSystem_FileExists_Parameters[2] = {
  { "aszFile", ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer, NULL, "const achar*" },
  { "aAttribs", ni::eType_U32, NULL, "tU32" }
};
static const ni::sMethodDef iFileSystem_FileExists = {
  "FileExists",
  0|ni::eType_U32, NULL, "tU32",
  2, iFileSystem_FileExists_Parameters,
  NULL
};

// Method: FileSize
static const ni::sParameterDef iFileSystem_FileSize_Parameters[1] = {
  { "aszFile", ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer, NULL, "const achar*" }
};
static const ni::sMethodDef iFileSystem_FileSize = {
  "FileSize",
  0|ni::eType_I64, NULL, "tI64",
  1, iFileSystem_FileSize_Parameters,
  NULL
};

// Method: FileOpen
static const ni::sParameterDef iFileSystem_FileOpen_Parameters[2] = {
  { "aszFile", ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer, NULL, "const achar*" },
  { "aMode", ni::eType_Enum, NULL, "eFileOpenMode" }
};
static const ni::sMethodDef iFileSystem_FileOpen = {
  "FileOpen",
  0|ni::eType_IUnknown|ni::eTypeFlags_Pointer, &niGetInterfaceUUID(iFile), "iFile*",
  2, iFileSystem_FileOpen_Parameters,
  NULL
};

// Method: FileBaseOpen
static const ni::sParameterDef iFileSystem_FileBaseOpen_Parameters[2] = {
  { "aszFile", ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer, NULL, "const achar*" },
  { "aMode", ni::eType_Enum, NULL, "eFileOpenMode" }
};
static const ni::sMethodDef iFileSystem_FileBaseOpen = {
  "FileBaseOpen",
  0|ni::eType_IUnknown|ni::eTypeFlags_Pointer, &niGetInterfaceUUID(iFileBase), "iFileBase*",
  2, iFileSystem_FileBaseOpen_Parameters,
  NULL
};

// Method: GetAbsolutePath
static const ni::sParameterDef iFileSystem_GetAbsolutePath_Parameters[1] = {
  { "aaszFile", ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer, NULL, "const achar*" }
};
static const ni::sMethodDef iFileSystem_GetAbsolutePath = {
  "GetAbsolutePath",
  0|ni::eType_String, NULL, "cString",
  1, iFileSystem_GetAbsolutePath_Parameters,
  NULL
};

// Method: CreateEnumerator
static const ni::sMethodDef iFileSystem_CreateEnumerator = {
  "CreateEnumerator",
  0|ni::eType_IUnknown|ni::eTypeFlags_Pointer, &niGetInterfaceUUID(iFileSystemEnumerator), "iFileSystemEnumerator*",
  0, NULL,
  NULL
};

static const ni::sMethodDef* Methods_iFileSystem[] = {
	&iFileSystem_GetRightsFlags,
	&iFileSystem_GetBaseContainer,
	&iFileSystem_FileMakeDir,
	&iFileSystem_FileDeleteDir,
	&iFileSystem_FileCopy,
	&iFileSystem_FileMove,
	&iFileSystem_FileDelete,
	&iFileSystem_FileEnum,
	&iFileSystem_FileExists,
	&iFileSystem_FileSize,
	&iFileSystem_FileOpen,
	&iFileSystem_FileBaseOpen,
	&iFileSystem_GetAbsolutePath,
	&iFileSystem_CreateEnumerator,

};

//// Interface description //////////////////////////////////
static const ni::tUUID* Parents_iFileSystem[] = {
  &niGetInterfaceUUID(iUnknown),
};

static const ni::sInterfaceDef InterfaceDef_iFileSystem = {
  niGetInterfaceID(iFileSystem),
  &niGetInterfaceUUID(iFileSystem),
  1,
  Parents_iFileSystem,
  niCountOf(Methods_iFileSystem),Methods_iFileSystem,
  NULL
};

	return &InterfaceDef_iFileSystem;
}

niExportFunc(const ni::sInterfaceDef*) TestInterfaceDef_iFileSystem() {
  static const sInterfaceDef& InterfaceDef_iFileSystem =
      vmbind::interface_def<iFileSystem>()
      .parent<ni::iUnknown>()
      .method<&iFileSystem::GetRightsFlags>(
        "GetRightsFlags",eTypeFlags_MethodGetter)
      .method<&iFileSystem::GetBaseContainer>(
        "GetBaseContainer",eTypeFlags_MethodGetter)
      .method<&iFileSystem::FileMakeDir>("FileMakeDir")
      .method<&iFileSystem::FileDeleteDir>("FileDeleteDir")
      .method<&iFileSystem::FileCopy>("FileCopy")
      .method<&iFileSystem::FileMove>("FileMove")
      .method<&iFileSystem::FileDelete>("FileDelete")
      .method<&iFileSystem::FileEnum>("FileEnum")
      .method<&iFileSystem::FileExists>("FileExists")
      .method<&iFileSystem::FileSize>("FileSize")
      .method<&iFileSystem::FileOpen>("FileOpen")
      .method<&iFileSystem::FileBaseOpen>("FileBaseOpen")
      .method<&iFileSystem::GetAbsolutePath>("GetAbsolutePath")
      .method<&iFileSystem::CreateEnumerator>("CreateEnumerator")
      .build();

  return &InterfaceDef_iFileSystem;
}

struct FVMBind {
};

const tUUID& _GetUUID(const tUUID* aUUID) {
  return aUUID ? *aUUID : kuuidZero;
}

static inline tBool StrContains(const achar* aStr, const achar* aSearch) {
  return StrZContains(aStr,0,aSearch,0,0);
}
static inline tBool StrContainsI(const achar* aStr, const achar* aSearch) {
  return StrZContainsI(aStr,0,aSearch,0,0);
}

static inline tBool MatchTypeName(const achar* aM, const achar* aT) {
  if (StrIEq(aM,aT))
    return eTrue;
  if (StrContains(aM,aT))
    return eTrue;
  if (StrEq(aM,"tBool") && StrEq(aT,"tI8"))
    return eTrue;
  if (StrEndsWith(aM,"Flags") && StrEq(aT,"tU32"))
    return eTrue;
  return eFalse;
}

TEST_FIXTURE(FVMBind,Base) {
  niLet midef = ManualInterfaceDef_iFileSystem();
  niLet tidef = TestInterfaceDef_iFileSystem();
  niDebugFmt(("... tidef->maszName: %s", tidef->maszName));
  niDebugFmt(("... tidef->mUUID: %s", _GetUUID(tidef->mUUID)));
  niDebugFmt(("... tidef->mnNumBases: %s", tidef->mnNumBases));
  niDebugFmt(("... tidef->mnNumMethods: %s", tidef->mnNumMethods));
  CHECK_EQUAL(midef->maszName,tidef->maszName);
  CHECK_EQUAL(midef->mnNumBases,tidef->mnNumBases);
  CHECK_EQUAL(midef->mnNumMethods,tidef->mnNumMethods);
  niLoop(i,ni::Min(midef->mnNumBases,tidef->mnNumBases)) {
    niDebugFmt(("... tidef->mpBases[%d]: %s", i, *tidef->mpBases[i]));
    CHECK_EQUAL(*midef->mpBases[i],*tidef->mpBases[i]);
  }
  niLoop(i,ni::Min(midef->mnNumMethods,tidef->mnNumMethods)) {
    niLet mmeth = midef->mpMethods[i];
    niLet tmeth = tidef->mpMethods[i];
    niDebugFmt(("... tidef->mpMethods[%d]: %s", i, tmeth->maszName));
    niDebugFmt(("... tmeth->mReturnType: %d:%s, %s",
                tmeth->mReturnType,
                ni::GetTypeString(tmeth->mReturnType),
                tmeth->mReturnTypeName));
    niDebugFmt(("... mmeth->mReturnType: %d:%s, %s",
                mmeth->mReturnType,
                ni::GetTypeString(mmeth->mReturnType),
                mmeth->mReturnTypeName));
    CHECK_EQUAL(mmeth->maszName,tmeth->maszName);
    CHECK_EQUAL(mmeth->mReturnType,tmeth->mReturnType);
    CHECK_EQUAL(_GetUUID(mmeth->mReturnTypeUUID),
                _GetUUID(tmeth->mReturnTypeUUID));
    //CHECK_EQUAL(mmeth->mReturnTypeName,tmeth->mReturnTypeName);
    CHECK(MatchTypeName(mmeth->mReturnTypeName,tmeth->mReturnTypeName));
    CHECK_EQUAL(mmeth->mnNumParameters,tmeth->mnNumParameters);
  }
}

}
