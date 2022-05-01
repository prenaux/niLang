// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/Types.h"
#include "API/niLang/IHString.h"
#include "API/niLang/StringBase.h"
#include "API/niLang/Var.h"
#include "API/niLang/IFile.h"
#include "API/niLang/ILang.h"
#include "API/niLang/Utils/ObjectInterfaceCast.h"

namespace ni {

static VarData _VarNull = { 0, { NULL } };

niExportFunc(const Var*) VarGetNull() {
  return static_cast<const Var*>(&_VarNull);
}

///////////////////////////////////////////////
niExportFunc(void) VarDestruct(Var* apVar) {
  switch (niType(apVar->mType)) {
    case eType_IUnknown: {
      if (apVar->mpIUnknown) {
        apVar->mpIUnknown->Release();
      }
      apVar->mpIUnknown = NULL;
      break;
    }
    case eType_Matrixf: {
      niFree(apVar->mpPointer);
      apVar->mpPointer = NULL;
      break;
    }
    case eType_String: {
      apVar->mpString->~cString();
      niFree(apVar->mpPointer);
      apVar->mpPointer = NULL;
      break;
    }
  }
  apVar->mType = eType_Null;
}

///////////////////////////////////////////////
niExportFunc(tInt) VarCompare(niConst Var* a, niConst Var* b) {
  if (a->mType != b->mType)
    return (tInt)a->mType - (tInt)b->mType;

  const tType type = a->mType;
  if (type & eTypeFlags_Pointer)
    return (tIntPtr)a->mpPointer - (tIntPtr)b->mpPointer;

  switch (niType(type)) {
    case eType_Null:
      return 0; // null == null
    case eType_I8:
    case eType_U8:
    case eType_AChar:
      niCAssert(sizeof(achar) == sizeof(cchar));
      return (tInt)a->mI8 - (tInt)b->mI8;
    case eType_I16:
    case eType_U16:
      return (tInt)a->mI16 - (tInt)b->mI16;
    case eType_I32:
    case eType_U32:
    case eType_F32:
      return (tInt)a->mI32 - (tInt)b->mI32;
    case eType_Vec2f:
    case eType_Vec2i:
    case eType_I64:
    case eType_U64:
    case eType_F64:
      return memcmp(a->mData,b->mData,8);
    case eType_Vec3f:
    case eType_Vec3i:
      return memcmp(a->mData,b->mData,12);
    case eType_String:
    case eType_Matrixf:
      return (tIntPtr)a->mpPointer - (tIntPtr)b->mpPointer;
    default:
    case eType_Vec4f:
    case eType_Vec4i:
    case eType_UUID:
      return memcmp(a->mData,b->mData,16);
  }
}

///////////////////////////////////////////////
niExportFunc(tBool) VarIsString(const Var& aVar)
{
  switch (niType(aVar.mType)) {
    case eType_IUnknown:
      if (niFlagIs(aVar.mType,eTypeFlags_Pointer)) {
        tHStringPtr hspStr = ni::QueryInterface<iHString>(aVar.mpIUnknown);
        return hspStr.IsOK();
      }
      break;
    case eType_String:
      return eTrue;
    case eType_AChar:
      return niFlagIs(aVar.mType,eTypeFlags_Pointer);
    default:
      break;
  }
  return eFalse;
}

///////////////////////////////////////////////
niExportFunc(void) VarToString(cString& str, const Var& aVar)
{
  switch (aVar.mType) {
    case (eType_IUnknown|eTypeFlags_Pointer): {
      tHStringPtr hspStr = ni::QueryInterface<iHString>(aVar.mpIUnknown);
      if (hspStr.IsOK()) {
        str = niHStr(hspStr);
      }
      break;
    }
    case eType_String:
      str = aVar.GetString().Chars();
      break;
    case eType_ASZ:
      str = aVar.mcpAChar;
      break;
    default:
      break;
  }
}

///////////////////////////////////////////////
niExportFunc(tBool) VarConvertToInt(Var& aVar, tI64 aV, tType aDestType)
{
  tType dstType = niType(aDestType);
  switch (dstType) {
    case eType_I8: aVar.SetI8((tI8)aV); break;
    case eType_U8: aVar.SetU8((tU8)(tI8&)aV); break;
    case eType_I16: aVar.SetI16((tI16)aV);  break;
    case eType_U16: aVar.SetU16((tU16)aV);  break;
    case eType_I32: aVar.SetI32((tI32)aV);  break;
    case eType_U32: aVar.SetU32((tU32)aV);  break;
    case eType_I64: aVar.SetI64((tI64)aV);  break;
    case eType_U64: aVar.SetU64((tU64)aV);  break;
    case eType_F32: aVar.SetF32((tF32)aV);  break;
    case eType_F64: aVar.SetF64((tF64)aV);  break;
    default:
      return eFalse;
  }
  return eTrue;
}

///////////////////////////////////////////////
niExportFunc(tBool) VarConvertToFloat(Var& aVar, tF64 aV, tType aDestType)
{
  tType dstType = niType(aDestType);
  switch (dstType) {
    case eType_F32: aVar.SetF32((tF32)aV);  break;
    case eType_F64: aVar.SetF64((tF64)aV);  break;
    default:
      return eFalse;
  }
  return eTrue;
}

///////////////////////////////////////////////
static tBool _VarParseStringToType(const cString& str, Var& aVar, tType aDestType)
{
  if (VarIsStringType(aDestType)) {
    aVar = str;
    return eTrue;
  }
  else {
    const tType dstType = niType(aDestType);
    if (dstType == eType_U32) {
      tU32 n;
      if (str.contains("Vec3") || str.contains("RGB")) {
        sVec4f col = sVec4f::Zero();
        str.NumberArray(col.ptr(),3,eTrue,"(");
        col.w = 1.0f;
        n = ULColorBuild(col);
      }
      else if (str.contains("Vec4") || str.contains("RGBA")) {
        sVec4f col = sVec4f::Zero();
        str.NumberArray(col.ptr(),4,eTrue,"(");
        n = ULColorBuild(col);
      }
      else {
        n = str.Long();
      }
      aVar.SetU32(n);
    }
    else if (VarIsIntType(dstType)) {
      VarConvertToInt(aVar,str.LongLong(),dstType);
    }
    else if (VarIsFloatType(dstType)) {
      VarConvertToFloat(aVar,str.Double(),dstType);
    }
    else if (dstType == eType_Vec2i) {
      sVec2i v = sVec2i::Zero();
      str.NumberArray(v.ptr(),2,eTrue,"(");
      aVar.SetVec2i(v);
    }
    else if (dstType == eType_Vec2f) {
      sVec2f v = sVec2f::Zero();
      str.NumberArray(v.ptr(),2,eTrue,"(");
      aVar.SetVec2f(v);
    }
    else if (dstType == eType_Vec3i) {
      sVec3i v = sVec3i::Zero();
      str.NumberArray(v.ptr(),3,eTrue,"(");
      aVar.SetVec3i(v);
    }
    else if (dstType == eType_Vec3f) {
      sVec3f v = sVec3f::Zero();
      str.NumberArray(v.ptr(),3,eTrue,"(");
      aVar.SetVec3f(v);
    }
    else if (dstType == eType_Vec4i) {
      sVec4i v = sVec4i::Zero();
      str.NumberArray(v.ptr(),4,eTrue,"(");
      aVar.SetVec4i(v);
    }
    else if (dstType == eType_Vec4f) {
      sVec4f v = sVec4f::Zero();
      str.NumberArray(v.ptr(),4,eTrue,"(");
      aVar.SetVec4f(v);
    }
    else if (dstType == eType_Matrixf) {
      sMatrixf v = sMatrixf::Identity();
      str.NumberArray(v.ptr(),16,eTrue,"(");
      aVar.SetMatrixf(v);
    }
    else if (dstType == eType_UUID) {
      tUUID v = str.UUID();
      aVar.SetUUID(v);
    }
    else {
      goto _conversion_failed;
    }
    return eTrue;
  }

_conversion_failed:
  //  niError(niFmt(_A("Can't convert from type '%s' to type '%s'."),
  //                     GetTypeString(baseSrcType),
  //                     GetTypeString(aDestType)));
  aVar.SetNull();
  return eFalse;
}

///////////////////////////////////////////////
niExportFunc(tBool) VarConvertType(Var& aVar, tType aDestType)
{
  if (niType(aVar.GetType()) == niType(aDestType)) {
    return eTrue;
  }
  if (niType(aDestType) == eType_Variant) {
    aVar.SetVar(aVar);
    return eTrue;
  }

  tType srcType = niType(aVar.GetType());
  tType dstType = niType(aDestType);
  switch (srcType)
  {
    case eType_Null:
      {
        switch (dstType) {
          case eType_Matrixf: {
            // Set to identity
            aVar.SetMatrixf(sMatrixf::Identity());
            break;
          }
          case eType_AChar: {
            aVar = AZEROSTR;
            break;
          }
          case eType_String: {
            aVar = cString();
            break;
          }
          default: {
            aVar.Set(aDestType,NULL,0);
            break;
          }
        }
        return eTrue;
      }
    case eType_IUnknown:
      {
        iHString* hsp = niGetIUnknownHString(aVar.mpIUnknown);
        if (hsp) {
          return _VarParseStringToType(cString(niHStr(hsp)),aVar,aDestType);
        }
        // fallthrough
      }
    case eType_I8:
    case eType_U8:
    case eType_I16:
    case eType_U16:
    case eType_I32:
    case eType_U32:
    case eType_I64:
    case eType_U64:
      {
        typedef tI64  tIntType;
        tBool bDstUnsigned = eFalse;
        tIntType v = 0;
        switch (dstType) {
          case eType_U8:
          case eType_U16:
          case eType_U32:
          case eType_U64:
            bDstUnsigned = eTrue;
            break;
        }
        switch (srcType) {
          case eType_I8:
            v = (tIntType)aVar.GetI8();
            if (bDstUnsigned) v = (tU8)v;
            break;
          case eType_U8:
            v = (tIntType)aVar.GetU8();
            break;
          case eType_I16:
            v = (tIntType)aVar.GetI16();
            if (bDstUnsigned) v = (tU16)v;
            break;
          case eType_U16:
            v = (tIntType)aVar.GetU16();
            break;
          case eType_I32:
            v = (tIntType)aVar.GetI32();
            if (bDstUnsigned) v = (tU32)v;
            break;
          case eType_U32:
            v = (tIntType)aVar.GetU32();
            break;
          case eType_I64:
            v = (tIntType)aVar.GetI64();
            if (bDstUnsigned) v = (tU64)v;
            break;
          case eType_U64:
            v = (tIntType)aVar.GetU64();
            break;
          case eType_IUnknown:
            v = (tIntType)aVar.GetIUnknownPointer();
            break;
        }
        switch (dstType) {
          case eType_I8: aVar.SetI8((tI8)v);  break;
          case eType_U8: aVar.SetU8((tU8)v);  break;
          case eType_I16: aVar.SetI16((tI16)v); break;
          case eType_U16: aVar.SetU16((tU16)v); break;
          case eType_I32: aVar.SetI32((tI32)v); break;
          case eType_U32: aVar.SetU32((tU32)v); break;
          case eType_I64: aVar.SetI64((tI64)v); break;
          case eType_U64: aVar.SetU64((tU64)v); break;
          case eType_F32: aVar.SetF32((tF32)v); break;
          case eType_F64: aVar.SetF64((tF64)v); break;
          case eType_Vec2f: aVar.SetVec2f(Vec2((tF32)v)); break;
          case eType_Vec2i: aVar.SetVec2i(Vec2((tI32)v)); break;
          case eType_Vec3f: aVar.SetVec3f(Vec3((tF32)v)); break;
          case eType_Vec3i: aVar.SetVec3i(Vec3((tI32)v)); break;
          case eType_Vec4f: aVar.SetVec4f(Vec4((tF32)v)); break;
          case eType_Vec4i: aVar.SetVec4i(Vec4((tI32)v)); break;
          case eType_Matrixf:  aVar.SetMatrixf(Matrix((tF32)v)); break;
          case eType_AChar:
          case eType_String: {
            cString str;
            str.Format(_A("%d"),v);
            aVar = str;
            break;
          }
          default:
            goto _conversion_failed;
        }
        return eTrue;
      }
    case eType_F32:
    case eType_F64:
      {
        typedef tF64  tFloatType;
        tFloatType v = 0;
        switch (srcType) {
          case eType_F32: v = (tFloatType)aVar.GetF32(); break;
          case eType_F64: v = (tFloatType)aVar.GetF64(); break;
        }
        switch (dstType) {
          case eType_I8: aVar.SetI8((tI8)v);  break;
          case eType_U8: aVar.SetU8((tU8)v);  break;
          case eType_I16: aVar.SetI16((tI16)v); break;
          case eType_U16: aVar.SetU16((tU16)v); break;
          case eType_I32: aVar.SetI32((tI32)v); break;
          case eType_U32: aVar.SetU32((tU32)v); break;
          case eType_I64: aVar.SetI64((tI64)v); break;
          case eType_U64: aVar.SetU64((tU64)v); break;
          case eType_F32: aVar.SetF32((tF32)v); break;
          case eType_F64: aVar.SetF64((tF64)v); break;
          case eType_Vec2f: aVar.SetVec2f(Vec2((tF32)v)); break;
          case eType_Vec2i: aVar.SetVec2i(Vec2((tI32)v)); break;
          case eType_Vec3f: aVar.SetVec3f(Vec3((tF32)v)); break;
          case eType_Vec3i: aVar.SetVec3i(Vec3((tI32)v)); break;
          case eType_Vec4f: aVar.SetVec4f(Vec4((tF32)v)); break;
          case eType_Vec4i: aVar.SetVec4i(Vec4((tI32)v)); break;
          case eType_Matrixf:  aVar.SetMatrixf(Matrix((tF32)v)); break;
          case eType_AChar:
          case eType_String: {
            cString str;
            str.Format(_A("%g"),v);
            aVar = str;
            break;
          }
          default:
            goto _conversion_failed;
        }
        return eTrue;
      }
#define CONVERT_VECTOR_TYPE(TYPEDEF,TYPE,X,Y,Z,W)                       \
      case eType_##TYPE:                                                \
        {                                                               \
          TYPEDEF v = aVar.Get##TYPE();                                 \
          if (dstType == eType_Vec2i) {                              \
            aVar.SetVec2i(Vec2((tI32)X,(tI32)Y));                 \
          }                                                             \
          else if (dstType == eType_Vec2f) {                         \
            aVar.SetVec2f(Vec2((tF32)X,(tF32)Y));                 \
          }                                                             \
          else if (dstType == eType_Vec3i) {                         \
            aVar.SetVec3i(Vec3((tI32)X,(tI32)Y,(tI32)Z));         \
          }                                                             \
          else if (dstType == eType_Vec3f) {                         \
            aVar.SetVec3f(Vec3((tF32)X,(tF32)Y,(tF32)Z));         \
          }                                                             \
          else if (dstType == eType_Vec4i) {                         \
            aVar.SetVec4i(Vec4((tI32)X,(tI32)Y,(tI32)Z,(tI32)W)); \
          }                                                             \
          else if (dstType == eType_Vec4f) {                         \
            aVar.SetVec4f(Vec4((tF32)X,(tF32)Y,(tF32)Z,(tF32)W)); \
          }                                                             \
          else if (dstType == eType_Matrixf) {                          \
            aVar.SetMatrixf(Matrix((tF32)X,(tF32)Y,(tF32)Z,(tF32)W));   \
          }                                                             \
          else if (dstType == eType_U32) {                              \
            sVec4f r = Vec4((tF32)X,(tF32)Y,(tF32)Z,(tF32)W);     \
            aVar.SetU32(ULColorBuild(r));                               \
          }                                                             \
          else if (VarIsStringType(dstType)) {                          \
            aVar = cString(niUnsafeCast(TYPEDEF&,v));                   \
          }                                                             \
          else if (VarIsIntType(dstType)) {                         \
            VarConvertToInt(aVar,(tI64)v.x,dstType);                \
          }                                                             \
          else if (VarIsFloatType(dstType)) {                           \
            VarConvertToFloat(aVar,(tF64)v.x,dstType);                  \
          }                                                             \
          else {                                                        \
            goto _conversion_failed;                                    \
          }                                                             \
          return eTrue;                                                 \
        }

      CONVERT_VECTOR_TYPE(sVec2f,Vec2f,v.x,v.y,0,0)
          CONVERT_VECTOR_TYPE(sVec2i,Vec2i,v.x,v.y,0,0)
          CONVERT_VECTOR_TYPE(sVec3f,Vec3f,v.x,v.y,v.z,0)
          CONVERT_VECTOR_TYPE(sVec3i,Vec3i,v.x,v.y,v.z,0)
          CONVERT_VECTOR_TYPE(sVec4f,Vec4f,v.x,v.y,v.z,v.w)
          CONVERT_VECTOR_TYPE(sVec4i,Vec4i,v.x,v.y,v.z,v.w)

    case eType_Matrixf:
          {
            if (VarIsStringType(dstType)) {
              aVar = cString(niUnsafeCast(sMatrixf&,aVar.GetMatrixf()));
            }
            else {
              goto _conversion_failed;
            }
            return eTrue;
          }

    case eType_UUID: {
      if (VarIsStringType(dstType)) {
        aVar = cString(aVar.GetUUID());
      }
      else {
        goto _conversion_failed;
      }
      return eTrue;
    }

    case eType_AChar: {
      return _VarParseStringToType(cString(aVar.GetACharConstPointer()), aVar, aDestType);
    }
    case eType_String: {
      return _VarParseStringToType(aVar.GetString(), aVar, aDestType);
    }
  }

_conversion_failed:
  //  niError(niFmt(_A("Can't convert from type '%s' to type '%s'."),
  //                     GetTypeString(baseSrcType),
  //                     GetTypeString(aDestType)));
  aVar.SetNull();
  return eFalse;
}

///////////////////////////////////////////////
niExportFuncCPP(Ptr<iHString>) VarGetHString(const Var& aVar)
{
  switch (niType(aVar.mType)) {
    case eType_IUnknown:
      if (niFlagIs(aVar.mType,eTypeFlags_Pointer)) {
        return niGetIUnknownHString(aVar.mpIUnknown);
      }
      break;
    case eType_String: {
      niAssert(!niFlagTest(aVar.mType,eTypeFlags_Pointer));
      const cString& str = *aVar.mpString;
      return _H(str.Chars());
    }
    case eType_AChar:
      if (niFlagIs(aVar.mType,eTypeFlags_Pointer)) {
        return _H(aVar.mcpAChar);
      }
      break;
    default:
      break;
  }
  return NULL;
}

///////////////////////////////////////////////
niExportFunc(const achar*) VarGetCharsOrNull(const Var& aVar)
{
  switch (niType(aVar.mType)) {
    case eType_IUnknown:
      if (niFlagIs(aVar.mType,eTypeFlags_Pointer)) {
        return niHStr(niGetIUnknownHString(aVar.mpIUnknown));
      }
      break;
    case eType_String: {
      niAssert(!niFlagTest(aVar.mType,eTypeFlags_Pointer));
      const cString& str = *aVar.mpString;
      return str.Chars();
    }
    case eType_AChar:
      if (niFlagIs(aVar.mType,eTypeFlags_Pointer)) {
        return aVar.mcpAChar;
      }
      break;
    default:
      break;
  }
  return NULL;
}

///////////////////////////////////////////////
niExportFunc(tBool) VarSerializeWrite(iFile* apFile, const Var& aVar) {
  if (niType(aVar.mType) == eType_IUnknown) {
    return eFalse;
  }
  else if (VarIsString(aVar)) {
    apFile->WriteLE32(eType_String);
    apFile->WriteBitsString(ni::VarGetString(aVar).Chars());
  }
  else {
    apFile->WriteLE32(niType(aVar.mType));
    switch (niType(aVar.mType)) {
      case eType_Null: /* nothing */ break;
      case eType_I8:  apFile->Write8(aVar.GetI8()); break;
      case eType_U8:  apFile->Write8(aVar.GetU8()); break;
      case eType_I16: apFile->WriteLE16(aVar.GetI16()); break;
      case eType_U16: apFile->WriteLE16(aVar.GetU16()); break;
      case eType_I32: apFile->WriteLE32(aVar.GetI32()); break;
      case eType_U32: apFile->WriteLE32(aVar.GetU32()); break;
      case eType_I64: apFile->WriteLE64(aVar.GetI64()); break;
      case eType_U64: apFile->WriteLE64(aVar.GetU64()); break;
      case eType_F32: apFile->WriteF32(aVar.GetF32()); break;
      case eType_F64: apFile->WriteF64(aVar.GetF64()); break;
      case eType_Vec2i: apFile->WriteLE32Array((tU32*)aVar.GetVec2i().ptr(),2); break;
      case eType_Vec3i: apFile->WriteLE32Array((tU32*)aVar.GetVec3i().ptr(),3); break;
      case eType_Vec4i: apFile->WriteLE32Array((tU32*)aVar.GetVec4i().ptr(),4); break;
      case eType_Vec2f: apFile->WriteF32Array(aVar.GetVec2f().ptr(),2); break;
      case eType_Vec3f: apFile->WriteF32Array(aVar.GetVec3f().ptr(),3); break;
      case eType_Vec4f: apFile->WriteF32Array(aVar.GetVec4f().ptr(),4); break;
      case eType_Matrixf:  apFile->WriteF32Array(aVar.GetMatrixf().ptr(),16); break;
      default: break;
    }
  }
  return eTrue;
}

///////////////////////////////////////////////
niExportFunc(tBool) VarSerializeRead(iFile* apFile, Var& aVar) {
  tType type = apFile->ReadLE32();
  if (niType(type) == eType_IUnknown) {
    return eFalse;
  }
  else {
    switch (niType(type)) {
      case eType_Null: aVar.SetNull(); break;
      case eType_I8:  aVar = (tI8)apFile->Read8();  break;
      case eType_U8:  aVar = (tU8)apFile->Read8();  break;
      case eType_I16: aVar = (tI16)apFile->ReadLE16();  break;
      case eType_U16: aVar = (tU16)apFile->ReadLE16();  break;
      case eType_I32: aVar = (tI32)apFile->ReadLE32();  break;
      case eType_U32: aVar = (tU32)apFile->ReadLE32();  break;
      case eType_I64: aVar = (tI64)apFile->ReadLE64();  break;
      case eType_U64: aVar = (tU64)apFile->ReadLE64();  break;
      case eType_F32: aVar = apFile->ReadF32(); break;
      case eType_F64: aVar = apFile->ReadF64(); break;
      case eType_Vec2i: aVar = sVec2i::Zero(); apFile->ReadLE32Array((tU32*)aVar.mV2L,2); break;
      case eType_Vec3i: aVar = sVec3i::Zero(); apFile->ReadLE32Array((tU32*)aVar.mV3L,3); break;
      case eType_Vec4i: aVar = sVec4i::Zero(); apFile->ReadLE32Array((tU32*)aVar.mV4L,4); break;
      case eType_Vec2f: aVar = sVec2f::Zero(); apFile->ReadF32Array(aVar.mV2F,2); break;
      case eType_Vec3f: aVar = sVec3f::Zero(); apFile->ReadF32Array(aVar.mV3F,3); break;
      case eType_Vec4f: aVar = sVec4f::Zero(); apFile->ReadF32Array(aVar.mV4F,4); break;
      case eType_Matrixf:
        {
          sMatrixf mtx;
          apFile->ReadF32Array(mtx.ptr(),16);
          aVar = mtx;
          break;
        }
      case eType_String:
        aVar = apFile->ReadBitsString();
        break;
      default:
        return eFalse;
    }
  }
  return eTrue;
}

///////////////////////////////////////////////
niExportFunc(tBool) VarSerializeWriteBitsPacked(iFile* apFile, const Var& aVar) {
  // IsNull?
  if (niType(aVar.mType) == eType_Null) {
    apFile->WriteBit(eTrue);  // null
  }
  else {
    apFile->WriteBit(eFalse); // not null
    if (niType(aVar.mType) == eType_IUnknown) {
      return eFalse;
    }
    else if (VarIsString(aVar)) {
      apFile->WriteBits32(eType_String,knTypeRawNumBits);
      apFile->WriteBitsString(ni::VarGetString(aVar).Chars());
    }
    else {
      apFile->WriteBits32(niType(aVar.mType),knTypeRawNumBits);
      switch (niType(aVar.mType)) {
        case eType_Null: /* nothing */ break;
        case eType_I8:  apFile->WriteBitsPackedI8(aVar.GetI8());  break;
        case eType_U8:  apFile->WriteBitsPackedU8(aVar.GetU8());  break;
        case eType_I16: apFile->WriteBitsPackedI16(aVar.GetI16());  break;
        case eType_U16: apFile->WriteBitsPackedU16(aVar.GetU16());  break;
        case eType_I32: apFile->WriteBitsPackedI32(aVar.GetI32());  break;
        case eType_U32: apFile->WriteBitsPackedU32(aVar.GetU32());  break;
        case eType_I64: apFile->WriteBitsPackedI64(aVar.GetI64());  break;
        case eType_U64: apFile->WriteBitsPackedU64(aVar.GetU64());  break;
        case eType_F32: apFile->WriteF32(aVar.GetF32()); break;
        case eType_F64: apFile->WriteF64(aVar.GetF64()); break;
        case eType_Vec2i: apFile->WriteLE32Array((tU32*)aVar.GetVec2i().ptr(),2); break;
        case eType_Vec3i: apFile->WriteLE32Array((tU32*)aVar.GetVec3i().ptr(),3); break;
        case eType_Vec4i: apFile->WriteLE32Array((tU32*)aVar.GetVec4i().ptr(),4); break;
        case eType_Vec2f: apFile->WriteF32Array(aVar.GetVec2f().ptr(),2); break;
        case eType_Vec3f: apFile->WriteF32Array(aVar.GetVec3f().ptr(),3); break;
        case eType_Vec4f: apFile->WriteF32Array(aVar.GetVec4f().ptr(),4); break;
        case eType_Matrixf:  apFile->WriteF32Array(aVar.GetMatrixf().ptr(),16); break;
        default:
          return eFalse;
      }
    }
  }
  return eTrue;
}

///////////////////////////////////////////////
niExportFunc(tBool) VarSerializeReadBitsPacked(iFile* apFile, Var& aVar) {
  // IsNull ?
  if (apFile->ReadBit()) {
    aVar.SetNull();
  }
  else {
    tType type = apFile->ReadBitsU32(knTypeRawNumBits);
    if (niType(type) == eType_IUnknown) {
      return eFalse;
    }
    else {
      switch (niType(type)) {
        case eType_Null: aVar.SetNull(); break;
        case eType_I8:  aVar = (tI8)apFile->ReadBitsPackedI8(); break;
        case eType_U8:  aVar = (tU8)apFile->ReadBitsPackedU8(); break;
        case eType_I16: aVar = (tI16)apFile->ReadBitsPackedI16(); break;
        case eType_U16: aVar = (tU16)apFile->ReadBitsPackedU16(); break;
        case eType_I32: aVar = (tI32)apFile->ReadBitsPackedI32(); break;
        case eType_U32: aVar = (tU32)apFile->ReadBitsPackedU32(); break;
        case eType_I64: aVar = (tI64)apFile->ReadBitsPackedI64(); break;
        case eType_U64: aVar = (tU64)apFile->ReadBitsPackedU64(); break;
        case eType_F32: aVar = apFile->ReadF32(); break;
        case eType_F64: aVar = apFile->ReadF64(); break;
        case eType_Vec2i: aVar = sVec2i::Zero(); apFile->ReadLE32Array((tU32*)aVar.mV2L,2); break;
        case eType_Vec3i: aVar = sVec3i::Zero(); apFile->ReadLE32Array((tU32*)aVar.mV3L,3); break;
        case eType_Vec4i: aVar = sVec4i::Zero(); apFile->ReadLE32Array((tU32*)aVar.mV4L,4); break;
        case eType_Vec2f: aVar = sVec2f::Zero(); apFile->ReadF32Array(aVar.mV2F,2); break;
        case eType_Vec3f: aVar = sVec3f::Zero(); apFile->ReadF32Array(aVar.mV3F,3); break;
        case eType_Vec4f: aVar = sVec4f::Zero(); apFile->ReadF32Array(aVar.mV4F,4); break;
        case eType_Matrixf:
          {
            sMatrixf mtx;
            apFile->ReadF32Array(mtx.ptr(),16);
            aVar = mtx;
            break;
          }
        case eType_String:
          aVar = apFile->ReadBitsString();
          break;
        default:
          return eFalse;
      }
    }
  }
  return eTrue;
}

///////////////////////////////////////////////
niExportFunc(tBool) VarSerializeWriteRaw(iFile* apFile, const Var& aVar) {
  if (niType(aVar.mType) == eType_IUnknown) {
    return eFalse;
  }
  else if (VarIsString(aVar)) {
    apFile->WriteLE32(eType_String);
    apFile->WriteStringZ(ni::VarGetString(aVar).Chars());
  }
  else {
    apFile->WriteLE32(niType(aVar.mType));
    switch (niType(aVar.mType)) {
      case eType_Null: /* nothing */ break;
      case eType_I8:  apFile->Write8(aVar.GetI8()); break;
      case eType_U8:  apFile->Write8(aVar.GetU8()); break;
      case eType_I16: apFile->WriteLE16(aVar.GetI16()); break;
      case eType_U16: apFile->WriteLE16(aVar.GetU16()); break;
      case eType_I32: apFile->WriteLE32(aVar.GetI32()); break;
      case eType_U32: apFile->WriteLE32(aVar.GetU32()); break;
      case eType_I64: apFile->WriteLE64(aVar.GetI64()); break;
      case eType_U64: apFile->WriteLE64(aVar.GetU64()); break;
      case eType_F32: apFile->WriteF32(aVar.GetF32()); break;
      case eType_F64: apFile->WriteF64(aVar.GetF64()); break;
      case eType_Vec2i: apFile->WriteLE32Array((tU32*)aVar.GetVec2i().ptr(),2); break;
      case eType_Vec3i: apFile->WriteLE32Array((tU32*)aVar.GetVec3i().ptr(),3); break;
      case eType_Vec4i: apFile->WriteLE32Array((tU32*)aVar.GetVec4i().ptr(),4); break;
      case eType_Vec2f: apFile->WriteF32Array(aVar.GetVec2f().ptr(),2); break;
      case eType_Vec3f: apFile->WriteF32Array(aVar.GetVec3f().ptr(),3); break;
      case eType_Vec4f: apFile->WriteF32Array(aVar.GetVec4f().ptr(),4); break;
      case eType_Matrixf:  apFile->WriteF32Array(aVar.GetMatrixf().ptr(),16); break;
      default:
        return eFalse;
    }
  }
  return eTrue;
}

///////////////////////////////////////////////
niExportFunc(tBool) VarSerializeReadRaw(iFile* apFile, Var& aVar) {
  tType type = apFile->ReadLE32();
  if (niType(type) == eType_IUnknown) {
    return eFalse;
  }
  else {
    switch (niType(type)) {
      case eType_Null: aVar.SetNull(); break;
      case eType_I8:  aVar = (tI8)apFile->Read8();  break;
      case eType_U8:  aVar = (tU8)apFile->Read8();  break;
      case eType_I16: aVar = (tI16)apFile->ReadLE16();  break;
      case eType_U16: aVar = (tU16)apFile->ReadLE16();  break;
      case eType_I32: aVar = (tI32)apFile->ReadLE32();  break;
      case eType_U32: aVar = (tU32)apFile->ReadLE32();  break;
      case eType_I64: aVar = (tI64)apFile->ReadLE64();  break;
      case eType_U64: aVar = (tU64)apFile->ReadLE64();  break;
      case eType_F32: aVar = apFile->ReadF32(); break;
      case eType_F64: aVar = apFile->ReadF64(); break;
      case eType_Vec2i: aVar = sVec2i::Zero(); apFile->ReadLE32Array((tU32*)aVar.mV2L,2); break;
      case eType_Vec3i: aVar = sVec3i::Zero(); apFile->ReadLE32Array((tU32*)aVar.mV3L,3); break;
      case eType_Vec4i: aVar = sVec4i::Zero(); apFile->ReadLE32Array((tU32*)aVar.mV4L,4); break;
      case eType_Vec2f: aVar = sVec2f::Zero(); apFile->ReadF32Array(aVar.mV2F,2); break;
      case eType_Vec3f: aVar = sVec3f::Zero(); apFile->ReadF32Array(aVar.mV3F,3); break;
      case eType_Vec4f: aVar = sVec4f::Zero(); apFile->ReadF32Array(aVar.mV4F,4); break;
      case eType_Matrixf:
        {
          sMatrixf mtx;
          apFile->ReadF32Array(mtx.ptr(),16);
          aVar = mtx;
          break;
        }
      case eType_String:
        aVar = apFile->ReadString();
        break;
      default:
        return eFalse;
    }
  }
  return eTrue;
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
}
