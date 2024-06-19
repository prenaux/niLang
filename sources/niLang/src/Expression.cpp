// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "API/niLang/Types.h"
#include "API/niLang/ILang.h"
#include "API/niLang/IExpression.h"
#include "API/niLang/IDataTable.h"
#include "API/niLang/ICrypto.h"
#include "API/niLang/Utils/UnknownImpl.h"
#include "API/niLang/Utils/ConcurrentImpl.h"
#include "API/niLang/Math/MathRect.h"
#include "API/niLang/Math/MathMatrix.h"
#include "API/niLang/Math/MathVec2.h"
#include "API/niLang/Math/MathVec3.h"
#include "API/niLang/Math/MathVec4.h"
#include "API/niLang/Math/MathQuat.h"
#include "API/niLang_ModuleDef.h"
#include "niLang/Utils/Path.h"
#include <niLang/IRegex.h>

using namespace ni;

// #define USE_MODULO_OP
// #define DEFAULT_ENUM_IS_FULLY_QUALIFIED

typedef tF64  tScalarFloat;
typedef tF32  tVectorFloat;

#define EXPRESSION_TRACE niWarning

#define OP_GET_FRAME_TIME ni::GetLang()->GetFrameTime()

//////////////////////////////////////////////////////////////////////////////////////////////
// Operation syntax :
//  operator :
//   operand0 OPERATOR operand1
//  unary operator :
//     UNARYOPERATOR operand0
//  function :
//     function(op0, ..., opN)
//////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations.
struct ExprVar;
struct Op;
class Evaluator;

//////////////////////////////////////////////////////////////////////////////////////////////
//! Mathematic variables.

inline tBool __stdcall _FromVar(iExpressionVariable* apExprVar, const Var& aVar) {
  niAssert(niIsOK(apExprVar));
  Var value = aVar;
  switch (apExprVar->GetType()) {
    case eExpressionVariableType_Float:
      VarConvertType(value,eType_F64);
      apExprVar->SetFloat(value.GetFloatValue());
      break;
    case eExpressionVariableType_Vec2:
      VarConvertType(value,eType_Vec2f);
      apExprVar->SetVec2(value.GetVec2fValue());
      break;
    case eExpressionVariableType_Vec3:
      VarConvertType(value,eType_Vec3f);
      apExprVar->SetVec3(value.GetVec3fValue());
      break;
    case eExpressionVariableType_Vec4:
      VarConvertType(value,eType_Vec4f);
      apExprVar->SetVec4(value.GetVec4fValue());
      break;
    case eExpressionVariableType_Matrix:
      VarConvertType(value,eType_Matrixf);
      apExprVar->SetMatrix(value.GetMatrixf());
      break;
    case eExpressionVariableType_String:
      VarConvertType(value,eType_String);
      apExprVar->SetString(value.GetString());
      break;
    case eExpressionVariableType_IUnknown:
      apExprVar->SetIUnknown(value.GetIUnknownPointer());
      break;
    default:
      return eFalse;
  }
  return eTrue;
}

inline Var _ToVar(const iExpressionVariable* apExprVar) {
  niAssert(niIsOK(apExprVar));
  switch (apExprVar->GetType()) {
    case eExpressionVariableType_Float:
      return apExprVar->GetFloat();
    case eExpressionVariableType_Vec2:
      return apExprVar->GetVec2();
    case eExpressionVariableType_Vec3:
      return apExprVar->GetVec3();
    case eExpressionVariableType_Vec4:
      return apExprVar->GetVec4();
    case eExpressionVariableType_Matrix:
      return apExprVar->GetMatrix();
    case eExpressionVariableType_String:
      return apExprVar->GetString();
    case eExpressionVariableType_IUnknown:
      return apExprVar->GetIUnknown();
  }
  return niVarNull;
}

///////////////////////////////////////////////
struct ExprVar : public ImplRC<iExpressionVariable>
{
 protected:
  ExprVar(iHString* ahspName, eExpressionVariableType aType) : hspName(ahspName), Type(aType) {}
 public:
  tHStringPtr       hspName;
  eExpressionVariableType  Type;
  tExpressionVariableFlags Flags;

  inline void __stdcall SetName(iHString* ahspString) { hspName = ahspString; }
  inline iHString* __stdcall GetName() const { return hspName; }
  tBool __stdcall Copy(const iExpressionVariable* apVar);
  iExpressionVariable* __stdcall Clone() const;
  eExpressionVariableType __stdcall GetType() const;
  tExpressionVariableFlags __stdcall GetFlags() const;

  tBool __stdcall SetVar(const Var& aVar) {
    return _FromVar(this, aVar);
  }

  Var __stdcall ToVar() const {
    return _ToVar(this);
  }
};

///////////////////////////////////////////////
struct ExprVarFromRunnable : public ExprVar
{
  ExprVarFromRunnable(iHString* ahspName, eExpressionVariableType aType, iRunnable* apRunnable)
      : ExprVar(ahspName,aType)
      , mRunnable(apRunnable)
  {
  }

  Ptr<iRunnable> mRunnable;

  void _UpdateValue(Var& aValue, eExpressionVariableType aType) const {
    aValue = mRunnable->Run();
    // niDebugFmt(("... _UpdateValue: %s (%s)", aValue, ni::GetTypeString(aValue.mType)));
    switch (aType) {
      case eExpressionVariableType_Float:
        VarConvertType(aValue,eType_F64);
        break;


#define VAR_APPLY_SCALAR_OR_ELSE(VALUE, SCALARAPPLY, ELSEAPPLY)   \
        {                                                         \
          tScalarFloat v;                                         \
          switch (niType(VALUE.mType)) {                          \
            case eType_I8: v = (tScalarFloat)VALUE.mI8; break;    \
            case eType_U8: v = (tScalarFloat)VALUE.mU8; break;    \
            case eType_I16: v = (tScalarFloat)VALUE.mI16; break;  \
            case eType_U16: v = (tScalarFloat)VALUE.mU16; break;  \
            case eType_I32: v = (tScalarFloat)VALUE.mI32; break;  \
            case eType_U32: v = (tScalarFloat)VALUE.mU32; break;  \
            case eType_I64: v = (tScalarFloat)VALUE.mI64; break;  \
            case eType_U64: v = (tScalarFloat)VALUE.mU64; break;  \
            case eType_F32: v = (tScalarFloat)VALUE.mF32; break;  \
            case eType_F64: v = (tScalarFloat)VALUE.mF64; break;  \
            default: ELSEAPPLY; return;                           \
          }                                                       \
          SCALARAPPLY;                                            \
        }

      case eExpressionVariableType_Vec2:
        VAR_APPLY_SCALAR_OR_ELSE(
            aValue,
            aValue.SetVec2f(Vec2f(v,v)),
            VarConvertType(aValue,eType_Vec2f));
        break;
      case eExpressionVariableType_Vec3:
        VAR_APPLY_SCALAR_OR_ELSE(
            aValue,
            aValue.SetVec3f(Vec3f(v,v,v)),
            VarConvertType(aValue,eType_Vec3f));
        break;
      case eExpressionVariableType_Vec4:
        VAR_APPLY_SCALAR_OR_ELSE(
            aValue,
            aValue.SetVec4f(Vec4f(v,v,v,v)),
            VarConvertType(aValue,eType_Vec4f));
        break;
      case eExpressionVariableType_Matrix:
        VAR_APPLY_SCALAR_OR_ELSE(
            aValue,
            aValue.SetMatrixf(Matrix<tF32>((tF32)v,(tF32)v,(tF32)v,(tF32)v,
                                           (tF32)v,(tF32)v,(tF32)v,(tF32)v,
                                           (tF32)v,(tF32)v,(tF32)v,(tF32)v,
                                           (tF32)v,(tF32)v,(tF32)v,(tF32)v)),
            VarConvertType(aValue,eType_Matrixf));
        break;

#undef VAR_APPLY_SCALAR_OR_ELSE

      case eExpressionVariableType_String:
        VarConvertType(aValue,eType_String);
        break;
      case eExpressionVariableType_IUnknown:
        // we can't convert other types to IUnknown, this will lead to undefined behavior
        if (!aValue.IsIUnknownPointer()) {
          aValue = niVarNull;
        }
        break;

      default:
        niAssertUnreachable("Unreachable.");
        break;
    }
  }

  virtual tScalarFloat __stdcall GetFloat() const niImpl {
    Var v;
    _UpdateValue(v,eExpressionVariableType_Float);
    // niDebugFmt(("... GetFloat: %s (%s)", v, ni::GetTypeString(v.mType)));
    return v.GetFloatValue();
  }
  virtual sVec2<tVectorFloat> __stdcall GetVec2() const niImpl {
    Var v;
    _UpdateValue(v,eExpressionVariableType_Vec2);
    return v.GetVec2fValue();
  }
  virtual sVec3<tVectorFloat> __stdcall GetVec3() const niImpl {
    Var v;
    _UpdateValue(v,eExpressionVariableType_Vec3);
    return v.GetVec3fValue();
  }
  virtual sVec4<tVectorFloat> __stdcall GetVec4() const niImpl {
    Var v;
    _UpdateValue(v,eExpressionVariableType_Vec4);
    return v.GetVec4fValue();
  }
  virtual sMatrix<tVectorFloat> __stdcall GetMatrix() const niImpl {
    Var v;
    _UpdateValue(v,eExpressionVariableType_Matrix);
    return v.GetMatrixf();
  }
  virtual cString __stdcall GetString() const niImpl {
    Var v;
    _UpdateValue(v,eExpressionVariableType_String);
    return v.GetString();
  }

  virtual iUnknown* __stdcall GetIUnknown() const niImpl {
    Var v;
    _UpdateValue(v,eExpressionVariableType_IUnknown);
    return v.GetIUnknownPointer();
  }

  virtual void __stdcall SetFloat(tScalarFloat aV) {}
  virtual void __stdcall SetVec2(const sVec2<tVectorFloat>& aV) {}
  virtual void __stdcall SetVec3(const sVec3<tVectorFloat>& aV) {}
  virtual void __stdcall SetVec4(const sVec4<tVectorFloat>& aV) {}
  virtual void __stdcall SetMatrix(const sMatrix<tVectorFloat>& aV) {}
  virtual void __stdcall SetString(const cString& aString) {}
  virtual void __stdcall SetIUnknown(const iUnknown* aV) {}
};

///////////////////////////////////////////////
struct ExprVarFloat : public ExprVar
{
  ExprVarFloat(iHString* ahspName) : ExprVar(ahspName,eExpressionVariableType_Float) { Value = 0; }
  tScalarFloat Value;

  virtual tScalarFloat __stdcall GetFloat() const { return Value; }
  virtual sVec2<tVectorFloat> __stdcall GetVec2() const { return Vec2<tVectorFloat>((tVectorFloat)Value,(tVectorFloat)Value); }
  virtual sVec3<tVectorFloat> __stdcall GetVec3() const { return Vec3<tVectorFloat>((tVectorFloat)Value,(tVectorFloat)Value,(tVectorFloat)Value); }
  virtual sVec4<tVectorFloat> __stdcall GetVec4() const { return Vec4<tVectorFloat>((tVectorFloat)Value,(tVectorFloat)Value,(tVectorFloat)Value,(tVectorFloat)Value); }
  virtual sMatrix<tVectorFloat> __stdcall GetMatrix() const {
    return Matrix<tVectorFloat>(
        (tVectorFloat)Value,(tVectorFloat)Value,(tVectorFloat)Value,(tVectorFloat)Value,
        (tVectorFloat)Value,(tVectorFloat)Value,(tVectorFloat)Value,(tVectorFloat)Value,
        (tVectorFloat)Value,(tVectorFloat)Value,(tVectorFloat)Value,(tVectorFloat)Value,
        (tVectorFloat)Value,(tVectorFloat)Value,(tVectorFloat)Value,(tVectorFloat)Value);
  }

  virtual void __stdcall SetFloat(tScalarFloat aV) { Value = aV; }
  virtual void __stdcall SetVec2(const sVec2<tVectorFloat>& aV) { Value = aV.x; }
  virtual void __stdcall SetVec3(const sVec3<tVectorFloat>& aV) { Value = aV.x; }
  virtual void __stdcall SetVec4(const sVec4<tVectorFloat>& aV) { Value = aV.x; }
  virtual void __stdcall SetMatrix(const sMatrix<tVectorFloat>& aV) { Value = aV._11; }

  virtual cString __stdcall GetString() const {
    return niFmt("%g",Value);
  }
  virtual void __stdcall SetString(const cString& aString) {
    Value = aString.Double();
  }
  virtual iUnknown* __stdcall GetIUnknown() const {
    return NULL;
  }
  virtual void __stdcall SetIUnknown(const iUnknown* aV) {}
};

///////////////////////////////////////////////
struct ExprVarVec2 :  public ExprVar
{
  ExprVarVec2(iHString* ahspName) : ExprVar(ahspName,eExpressionVariableType_Vec2) {}
  sVec2<tVectorFloat> Value;

  virtual tScalarFloat __stdcall GetFloat() const { return Value.x; }
  virtual sVec2<tVectorFloat> __stdcall GetVec2() const { return Value; }
  virtual sVec3<tVectorFloat> __stdcall GetVec3() const { return Vec3<tVectorFloat>(Value.x,Value.y,tVectorFloat(0)); }
  virtual sVec4<tVectorFloat> __stdcall GetVec4() const { return Vec4<tVectorFloat>(Value.x,Value.y,tVectorFloat(0),tVectorFloat(0)); }
  virtual sMatrix<tVectorFloat> __stdcall GetMatrix() const {
    return Matrix<tVectorFloat>(Value.x,Value.y,tVectorFloat(0),tVectorFloat(0),
                                tVectorFloat(0),tVectorFloat(0),tVectorFloat(0),tVectorFloat(0),
                                tVectorFloat(0),tVectorFloat(0),tVectorFloat(0),tVectorFloat(0),
                                tVectorFloat(0),tVectorFloat(0),tVectorFloat(0),tVectorFloat(0));
  }

  virtual void __stdcall SetFloat(tScalarFloat aV) { Value.x = Value.y = (tVectorFloat)aV; }
  virtual void __stdcall SetVec2(const sVec2<tVectorFloat>& aV) { Value.x = aV.x; Value.y = aV.y; }
  virtual void __stdcall SetVec3(const sVec3<tVectorFloat>& aV) { Value.x = aV.x; Value.y = aV.y; }
  virtual void __stdcall SetVec4(const sVec4<tVectorFloat>& aV) { Value.x = aV.x; Value.y = aV.y; }
  virtual void __stdcall SetMatrix(const sMatrix<tVectorFloat>& aV) { Value.x = aV._11;  Value.y = aV._12; }

  virtual cString __stdcall GetString() const {
    return niFmt("%s",Value);
  }
  virtual void __stdcall SetString(const cString& aString) {
    Value = aString.Vec2<tVectorFloat>();
  }

  virtual iUnknown* __stdcall GetIUnknown() const {
    return NULL;
  }
  virtual void __stdcall SetIUnknown(const iUnknown* aV) {}
};

///////////////////////////////////////////////
struct ExprVarVec3 :  public ExprVar
{
  ExprVarVec3(iHString* ahspName) : ExprVar(ahspName,eExpressionVariableType_Vec3) {}
  sVec3<tVectorFloat> Value;

  virtual tScalarFloat     __stdcall GetFloat() const { return Value.x; }
  virtual sVec2<tVectorFloat> __stdcall GetVec2() const { return Vec2<tVectorFloat>(Value.x,Value.y); }
  virtual sVec3<tVectorFloat> __stdcall GetVec3() const { return Value; }
  virtual sVec4<tVectorFloat> __stdcall GetVec4() const { return Vec4<tVectorFloat>(Value.x,Value.y,Value.z,tVectorFloat(0)); }
  virtual sMatrix<tVectorFloat> __stdcall GetMatrix() const {
    return Matrix<tVectorFloat>(Value.x,Value.y,Value.z,tVectorFloat(0),
                                tVectorFloat(0),tVectorFloat(0),tVectorFloat(0),tVectorFloat(0),
                                tVectorFloat(0),tVectorFloat(0),tVectorFloat(0),tVectorFloat(0),
                                tVectorFloat(0),tVectorFloat(0),tVectorFloat(0),tVectorFloat(0));
  }

  virtual void __stdcall SetFloat(tScalarFloat aV) { Value.x = Value.y = Value.z = (tVectorFloat)aV; }
  virtual void __stdcall SetVec2(const sVec2<tVectorFloat>& aV) { Value.x = aV.x; Value.y = aV.y; }
  virtual void __stdcall SetVec3(const sVec3<tVectorFloat>& aV) { Value.x = aV.x; Value.y = aV.y; Value.z = aV.z; }
  virtual void __stdcall SetVec4(const sVec4<tVectorFloat>& aV) { Value.x = aV.x; Value.y = aV.y; Value.z = aV.z; }
  virtual void __stdcall SetMatrix(const sMatrix<tVectorFloat>& aV) { Value.x = aV._11;  Value.y = aV._12; Value.z = aV._13; }

  virtual cString __stdcall GetString() const {
    return niFmt("%s",Value);
  }
  virtual void __stdcall SetString(const cString& aString) {
    Value = aString.Vec3<tVectorFloat>();
  }

  virtual iUnknown* __stdcall GetIUnknown() const {
    return NULL;
  }
  virtual void __stdcall SetIUnknown(const iUnknown* aV) {}
};

///////////////////////////////////////////////
struct ExprVarVec4 :  public ExprVar
{
  ExprVarVec4(iHString* ahspName) : ExprVar(ahspName,eExpressionVariableType_Vec4) {}
  sVec4<tVectorFloat> Value;

  virtual tScalarFloat     __stdcall GetFloat() const { return Value.x; }
  virtual sVec2<tVectorFloat> __stdcall GetVec2() const { return Vec2<tVectorFloat>(Value.x,Value.y); }
  virtual sVec3<tVectorFloat> __stdcall GetVec3() const { return Vec3<tVectorFloat>(Value.x,Value.y,Value.z); }
  virtual sVec4<tVectorFloat> __stdcall GetVec4() const { return Value; }
  virtual sMatrix<tVectorFloat> __stdcall GetMatrix() const {
    return Matrix<tVectorFloat>(Value.x,Value.y,Value.z,Value.w,
                                tVectorFloat(0),tVectorFloat(0),tVectorFloat(0),tVectorFloat(0),
                                tVectorFloat(0),tVectorFloat(0),tVectorFloat(0),tVectorFloat(0),
                                tVectorFloat(0),tVectorFloat(0),tVectorFloat(0),tVectorFloat(0));
  }

  virtual void __stdcall SetFloat(tScalarFloat aV) { Value.x = Value.y = Value.z = Value.w = (tVectorFloat)aV; }
  virtual void __stdcall SetVec2(const sVec2<tVectorFloat>& aV) { Value.x = aV.x; Value.y = aV.y; }
  virtual void __stdcall SetVec3(const sVec3<tVectorFloat>& aV) { Value.x = aV.x; Value.y = aV.y; Value.z = aV.z; }
  virtual void __stdcall SetVec4(const sVec4<tVectorFloat>& aV) { Value.x = aV.x; Value.y = aV.y; Value.z = aV.z; Value.w = aV.w; }
  virtual void __stdcall SetMatrix(const sMatrix<tVectorFloat>& aV) { Value.x = aV._11;  Value.y = aV._12; Value.z = aV._13; Value.w = aV._14; }

  virtual cString __stdcall GetString() const {
    return niFmt("%s",Value);
  }
  virtual void __stdcall SetString(const cString& aString) {
    Value = aString.Vec4<tVectorFloat>();
  }

  virtual iUnknown* __stdcall GetIUnknown() const {
    return NULL;
  }
  virtual void __stdcall SetIUnknown(const iUnknown* aV) {}
};

///////////////////////////////////////////////
struct ExprVarMatrix :  public ExprVar
{
  ExprVarMatrix(iHString* ahspName) : ExprVar(ahspName,eExpressionVariableType_Matrix) {}
  sMatrix<tVectorFloat> Value;

  virtual tScalarFloat     __stdcall GetFloat() const { return Value._11; }
  virtual sVec2<tVectorFloat> __stdcall GetVec2() const { return Vec2<tVectorFloat>(Value._11,Value._12); }
  virtual sVec3<tVectorFloat> __stdcall GetVec3() const { return Vec3<tVectorFloat>(Value._11,Value._12,Value._13); }
  virtual sVec4<tVectorFloat> __stdcall GetVec4() const { return Vec4<tVectorFloat>(Value._11,Value._12,Value._13,Value._14); }
  virtual sMatrix<tVectorFloat> __stdcall GetMatrix() const { return Value; }

  virtual void __stdcall SetFloat(tScalarFloat aV) { tVectorFloat v = (tVectorFloat)aV; Value.Set(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v); }
  virtual void __stdcall SetVec2(const sVec2<tVectorFloat>& aV) { Value._11 = aV.x; Value._12 = aV.y; }
  virtual void __stdcall SetVec3(const sVec3<tVectorFloat>& aV) { Value._11 = aV.x; Value._12 = aV.y; Value._13 = aV.z; }
  virtual void __stdcall SetVec4(const sVec4<tVectorFloat>& aV) { Value._11 = aV.x; Value._12 = aV.y; Value._13 = aV.z; Value._14 = aV.w; }
  virtual void __stdcall SetMatrix(const sMatrix<tVectorFloat>& aV)   { Value = aV; }

  virtual cString __stdcall GetString() const {
    return niFmt("%s",Value);
  }
  virtual void __stdcall SetString(const cString& aString) {
    Value = aString.Matrix<tVectorFloat>();
  }

  virtual iUnknown* __stdcall GetIUnknown() const {
    return NULL;
  }
  virtual void __stdcall SetIUnknown(const iUnknown* aV) {}
};

///////////////////////////////////////////////
struct ExprVarString :  public ExprVar
{
  ExprVarString(iHString* ahspName) :
      ExprVar(ahspName,eExpressionVariableType_String) {}
  cString Value;

  virtual tScalarFloat     __stdcall GetFloat() const { return Value.Double(); }
  virtual sVec2<tVectorFloat> __stdcall GetVec2() const { return Value.Vec2<tVectorFloat>(); }
  virtual sVec3<tVectorFloat> __stdcall GetVec3() const { return Value.Vec3<tVectorFloat>(); }
  virtual sVec4<tVectorFloat> __stdcall GetVec4() const { return Value.Vec4<tVectorFloat>(); }
  virtual sMatrix<tVectorFloat> __stdcall GetMatrix() const { return Value.Matrix<tVectorFloat>(); }

  virtual void __stdcall SetFloat(tScalarFloat aV) { Value = niFmt("%g",aV); }
  virtual void __stdcall SetVec2(const sVec2<tVectorFloat>& aV) { Value = niFmt("%s",aV); }
  virtual void __stdcall SetVec3(const sVec3<tVectorFloat>& aV) { Value = niFmt("%s",aV); }
  virtual void __stdcall SetVec4(const sVec4<tVectorFloat>& aV) { Value = niFmt("%s",aV); }
  virtual void __stdcall SetMatrix(const sMatrix<tVectorFloat>& aV)   { Value = niFmt("%s",aV); }


  virtual cString __stdcall GetString() const {
    return Value;
  }
  virtual void __stdcall SetString(const cString& aString) {
    Value = aString;
  }

  virtual iUnknown* __stdcall GetIUnknown() const {
    return NULL;
  }

  virtual void __stdcall SetIUnknown(const iUnknown* aV) {}
};

///////////////////////////////////////////////
struct ExprVarIUnknown :  public ExprVar
{
  ExprVarIUnknown(iHString* ahspName) :
    ExprVar(ahspName,eExpressionVariableType_IUnknown) {}
  Ptr<iUnknown> Value;

  virtual tScalarFloat        __stdcall GetFloat() const { return reinterpret_cast<tUIntPtr>(Value.ptr()); }
  virtual sVec2<tVectorFloat> __stdcall GetVec2() const { return Vec2<tVectorFloat>(GetFloat(),tVectorFloat(0)); }
  virtual sVec3<tVectorFloat> __stdcall GetVec3() const { return Vec3<tVectorFloat>(GetFloat(),tVectorFloat(0),tVectorFloat(0)); }
  virtual sVec4<tVectorFloat> __stdcall GetVec4() const { return Vec4<tVectorFloat>(GetFloat(),tVectorFloat(0),tVectorFloat(0),tVectorFloat(0)); }
  virtual sMatrix<tVectorFloat> __stdcall GetMatrix() const {
    return Matrix<tVectorFloat>(GetFloat(),tVectorFloat(0),tVectorFloat(0),tVectorFloat(0),
                                tVectorFloat(0),tVectorFloat(0),tVectorFloat(0),tVectorFloat(0),
                                tVectorFloat(0),tVectorFloat(0),tVectorFloat(0),tVectorFloat(0),
                                tVectorFloat(0),tVectorFloat(0),tVectorFloat(0),tVectorFloat(0)); }

  virtual void __stdcall SetFloat(tScalarFloat aV) {}
  virtual void __stdcall SetVec2(const sVec2<tVectorFloat>& aV) {}
  virtual void __stdcall SetVec3(const sVec3<tVectorFloat>& aV) {}
  virtual void __stdcall SetVec4(const sVec4<tVectorFloat>& aV) {}
  virtual void __stdcall SetMatrix(const sMatrix<tVectorFloat>& aV) {}

  virtual cString __stdcall GetString() const {
    return Value.IsOK() ? niFmt("%s",Value) : AZEROSTR;
  }

  virtual void __stdcall SetString(const cString& aString) {}

  virtual iUnknown* __stdcall GetIUnknown() const {
    return Value;
  }
  virtual void __stdcall SetIUnknown(const iUnknown* aV) {
    Value = aV;
  }
};


static ExprVar* _CreateVariable(const achar* aaszName, eExpressionVariableType aType, tExpressionVariableFlags aFlags = eExpressionVariableFlags_Default)
{
  tHStringPtr hspName;
  if (niStringIsOK(aaszName)) {
    hspName = _H(aaszName);
  }
  ExprVar* pVar = NULL;
  switch (aType) {
    case eExpressionVariableType_Float:
      pVar = niNew ExprVarFloat(hspName);
      break;
    case eExpressionVariableType_Vec2:
      pVar = niNew ExprVarVec2(hspName);
      break;
    case eExpressionVariableType_Vec3:
      pVar = niNew ExprVarVec3(hspName);
      break;
    case eExpressionVariableType_Vec4:
      pVar = niNew ExprVarVec4(hspName);
      break;
    case eExpressionVariableType_Matrix:
      pVar = niNew ExprVarMatrix(hspName);
      break;
    case eExpressionVariableType_String:
      pVar = niNew ExprVarString(hspName);
      break;
    case eExpressionVariableType_IUnknown:
      pVar = niNew ExprVarIUnknown(hspName);
      break;
    default:
      niAssertUnreachable("Unreachable code.");
      return NULL;
  }

  ((ExprVar*)(pVar))->Flags = aFlags;
  return pVar;
}

//////////////////////////////////////////////////////////////////////////////////////////////
enum eMathExprTokenType
{
  //! Unkown token, probably a variable.
  eMathExprTokenType_Unknown = 0,
  //! Real number token.
  eMathExprTokenType_Float = 1,
  //! Operator token.
  eMathExprTokenType_Operator = 2,
  //! UnaryOperator token.
  eMathExprTokenType_UnaryOperator = 3,
  //! Function token.
  eMathExprTokenType_Function = 4,
  //! Left paranthesis token.
  eMathExprTokenType_OpenGroup = 5,
  //! Right paranthesis token.
  eMathExprTokenType_CloseGroup = 6,
  //! Separator token.
  eMathExprTokenType_Separator = 7,
  //! String token.
  eMathExprTokenType_String = 8,
  //! URL token.
  eMathExprTokenType_URL = 9,
  //! \internal
  eMathExprTokenType_Last = 10,
  //! \internal
  eMathExprTokenType_ForceDWORD = 0xFFFFFFFF
};

struct sMathExprToken
{
  eMathExprTokenType Type = eMathExprTokenType_Last;
  cString            strToken = AZEROSTR;
  tF64               fFloat = 0; // Real number data.
  tI32               nGroupDepth = 0;  // Group depth
};
typedef astl::vector<sMathExprToken>  tMathExprTokenVec;

//////////////////////////////////////////////////////////////////////////////////////////////
//! Math operation type.
enum eMathOperationType
{
  //! Operator type. op0 OPERATOR op1.
  eMathOperationType_Operator = 0,
  //! UnaryOperator type. UNARYOPERATOR op0.
  eMathOperationType_UnaryOperator = 1,
  //! Function type. FUNCTION(op0,...,opN).
  eMathOperationType_Function = 2,
  //! Group type. ( operations ).
  eMathOperationType_Group = 3,
  //! \internal
  eMathOperationType_Last = 4,
  //! \internal
  eMathOperationType_ForceDWORD = 0xFFFFFFFF
};

//! Math operation
struct Op : public ImplRC<ni::iUnknown>
{
  struct sOperand
  {
    Ptr<ExprVar> ptrVariable;
    Ptr<Op>      ptrOperation;

    tBool IsOK() const {
      return (ptrVariable.IsOK()) || (ptrOperation.IsOK());
    }

    tBool Eval(iExpressionContext* apContext) {
      return (!ptrOperation.IsOK()) ? eTrue : (ptrOperation->Eval(apContext) != NULL);
    }

    tBool IsConstant() const {
      if (!ptrOperation.IsOK()) { return niFlagIs(ptrVariable->GetFlags(),eExpressionVariableFlags_Constant); }
      else                      { return ptrOperation->IsConstant(); }
    }

    iExpressionVariable* GetVariable() const {
      if (!ptrOperation.IsOK()) { return ptrVariable; }
      else                      { return ptrOperation->GetEvalResult(); }
    }

    tBool ToBool() const {
      iExpressionVariable* pVar = this->GetVariable();
      niAssert(pVar != NULL);
      if (!pVar) {
        return eFalse;
      }
      if (pVar->GetType() == eExpressionVariableType_String) {
        return pVar->GetString().Bool(eFalse);
      }
      else {
        return pVar->GetFloat() != 0;
      }
    }

    const achar* GetName() const {
      return ptrOperation.IsOK() ? ptrOperation->GetName() : NULL;
    }
  };

  typedef astl::vector<sOperand>  tOperandVec;
  typedef tOperandVec::iterator   tOperandVecIt;
  typedef tOperandVec::const_iterator tOperandVecCIt;

 public:
  Op(eMathOperationType aType, const achar* aszName, tU32 aulNumOperands);

  //! Create a new empty operation of the same type.
  virtual Op* Create() const = 0;

  //! Eval the operation value.
  iExpressionVariable* __stdcall Eval(iExpressionContext* apContext);
  //! Get the result value variable of this operation.
  iExpressionVariable* __stdcall GetEvalResult() const;

  //! Get the operation type.
  eMathOperationType GetType() const;
  //! Return the name of the operation.
  const achar* GetName() const;

  //! Return whether the operation is lazy.
  tBool IsLazy() const;
  //! Return eTrue if the result of this operation will always be the same.
  tBool IsConstant() const;
  //! Return eTrue if the operands are constant.
  tBool AreOperandsConstant() const;
  //! Return eTrue if this operation accepts a variable number of operand.
  tBool IsVarNumOperands() const;
  //! Get the number of operands.
  tU32 GetNumOperands() const;
  //! Set the specified operand variable.
  //! \return eTrue if the operand variable has been set correctly, eFalse else.
  //! \remark the operand variables can only be set before the first evaluate.
  tBool SetOperandVariable(tU32 aulIdx, iExpressionVariable* apVar);
  //! Set the specified operand operation.
  //! \return eTrue if the operand operation has been set correctly, eFalse else.
  //! \remark the operand operation can only be set before the first evaluate.
  tBool SetOperandOperation(tU32 aulIdx, Ptr<Op> apOperation);
  //! Get the specified operand.
  iExpressionVariable* GetOperand(tU32 aulIdx) const;

  //! Get the operand common type.
  virtual eExpressionVariableType GetCommonOperandsType() const;

  //! Setup the evaluation.
  //! \remark If this method is called you can rely on the fact that all operands are valid.
  //! \remark mptrReturn must be valid after the call to this function, otherwise Evaluate() will fail.
  virtual tBool SetupEvaluation(iExpressionContext* apContext) = 0;
  //! Do the evaluation without checking.
  virtual tBool DoEvaluate(iExpressionContext* apContext) = 0;

  cString                mstrName;
  Ptr<iExpressionVariable> mptrResult;
  tOperandVec            mvOperands;
  eMathOperationType     mType;
  tBool                  mbEvaluated;
  tBool                  mbConstant;
  tBool                  mbVarArgs;
  tBool                  mbLazy;
};

//////////////////////////////////////////////////////////////////////////////////////////////
struct Expression : public ImplRC<ni::iExpression> {
  Ptr<iExpressionContext> _context;
  Ptr<Op> _root;

  Expression(iExpressionContext* apContext, Op* apRootOp) {
    niAssert(niIsOK(apContext));
    niAssert(niIsOK(apRootOp));
    _context = apContext;
    _root = apRootOp;
  }

  virtual Ptr<iExpressionVariable> __stdcall Eval() {
    return _root->Eval(_context);
  }
  virtual iExpressionVariable* __stdcall GetEvalResult() const {
    return _root->GetEvalResult();
  }
  virtual iExpressionContext* __stdcall GetContext() const {
    return _context;
  }
};

//////////////////////////////////////////////////////////////////////////////////////////////
class Evaluator : public ImplRC<ni::iExpressionContext>
{
 public:
  typedef astl::map<cString,Ptr<Op>>  tOperationMap;
  typedef tOperationMap::iterator       tOperationMapIt;
  typedef tOperationMap::const_iterator tOperationMapCIt;

 public:
  Evaluator(Evaluator* apParent);
  ~Evaluator();

  tBool _RegisterReservedVariables();

  //! Sanity check.
  tBool __stdcall IsOK() const;

  iExpressionContext* __stdcall GetParentContext() const {
    return mptrParent;
  }

  iExpressionContext* __stdcall CreateContext() {
    return niNew Evaluator(this);
  }

  //! Add an operation.
  tBool AddOperation(Op* apOperation);
  //! Add an enumeration definition.
  tBool __stdcall AddEnumDef(const sEnumDef* apEnumDef);

  iExpressionVariable* __stdcall CreateVariable(const achar* aaszName, eExpressionVariableType aType, tExpressionVariableFlags aFlags = eExpressionVariableFlags_Default);
  iExpressionVariable* __stdcall CreateVariableFromExpr(const achar* aaszName, const achar* aaszExpr, tExpressionVariableFlags aFlags = eExpressionVariableFlags_Default);
  iExpressionVariable* __stdcall CreateVariableFromRunnable(const achar* aaszName, eExpressionVariableType aType, iRunnable* apRunnable, tExpressionVariableFlags aFlags = eExpressionVariableFlags_Default);
  iExpressionVariable* __stdcall CreateVariableFromVar(const achar* aaszName, const Var& aInitialValue, tExpressionVariableFlags aFlags = eExpressionVariableFlags_Default);

  //! Tokenize the given string and output the result in the given token vector.
  tBool TokenizeExpr(const achar* aaszExpr, tMathExprTokenVec& aOut);
  //! Parse the given string and output the result in the given operation list.
  Ptr<Op> ParseExpr(const achar* aaszExpr);
  //! Parse the tokens.
  Ptr<Op> _ProcessToken(Op* apCurrentOp, tU32& anCurrentOperand, tMathExprTokenVec& avToks, tMathExprTokenVec::iterator& aitTok);

  //! Get the unknown symbols in the specified expression.
  tBool __stdcall GetUnknownSymbols(const achar* aaszExpr, tStringCVec* apList);

  Op* __stdcall GetOperator(const achar* aaszName) const;
  Op* __stdcall GetUnaryOperator(const achar* aaszName) const;
  Op* __stdcall GetFunction(const achar* aaszName) const;
  const sEnumDef* __stdcall GetEnumDef(const achar* aaszName) const;

  const achar* __stdcall GetEnumName(tU32 anIndex) const;

  void __stdcall SetDefaultEnumDef(const sEnumDef* apEnumDef);
  const sEnumDef* __stdcall GetDefaultEnumDef() const;

  void __stdcall SetGlobalEnumSearch(tBool abEnabled);
  tBool __stdcall GetGlobalEnumSearch() const;

  tBool __stdcall RegisterURLResolver(const achar* aaszProtocol, iExpressionURLResolver* apResolver) {
    niCheckIsOK(apResolver,eFalse);
    astl::upsert(mmapURLResolvers,aaszProtocol,apResolver);
    return eTrue;
  }
  tBool __stdcall UnregisterURLResolver(const achar* aaszProtocol) {
    return !!astl::map_find_erase(mmapURLResolvers,aaszProtocol);
  }
  iExpressionURLResolver* __stdcall FindURLResolver(const achar* aaszProtocol) const {
    tURLResolverMap::const_iterator it = mmapURLResolvers.find(aaszProtocol);
    if (it == mmapURLResolvers.end()) {
      if (mptrParent.IsOK())
        return mptrParent->FindURLResolver(aaszProtocol);
      else
        return NULL;
    }
    return it->second;
  }

 private:
  //! Create an operation.
  Ptr<Op> _CreateOperation(const achar* aaszName, eMathExprTokenType aOpType);

  Ptr<Evaluator> mptrParent;
  astl::vector<Ptr<iExpressionVariable> > mvVariables;
  tOperationMap mmapOperators;
  tOperationMap mmapUnaryOperators;
  tOperationMap mmapFunctions;
  tBool     mbOK;

  typedef astl::map<cString,const sEnumDef*>  tEnumMap;
  mutable tEnumMap mmapEnums;
  const sEnumDef*  mpDefaultEnum;
  tBool mbGlobalEnumSearch;

  typedef astl::map<cString,Ptr<iExpressionURLResolver> > tURLResolverMap;
  tURLResolverMap mmapURLResolvers;

 public:

  ///////////////////////////////////////////////
  virtual tBool __stdcall AddVariable(iExpressionVariable* apVar) {
    if (!niIsOK(apVar)) {
      niError("Invalid variable.");
      return eFalse;
    }
    tHStringPtr name = apVar->GetName();
    if (HStringIsEmpty(name)) {
      niError("Empty variable name.");
      return eFalse;
    }
    if (_FindLocalVariable(name) != NULL) {
      niError(niFmt("Variable '%s' already exists.", name));
      return eFalse;
    }
    this->mvVariables.push_back(apVar);
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall RemoveVariable(iExpressionVariable* apVariable)
  {
    if (!niIsOK(apVariable)) return eFalse;
    if (apVariable->GetFlags()&eExpressionVariableFlags_Reserved) return eFalse;
    return astl::find_erase(mvVariables,apVariable);
  }

  ///////////////////////////////////////////////
  iExpressionVariable* __stdcall _FindLocalVariable(iHString* ahspName) const
  {
    niLoop(i,mvVariables.size()) {
      iExpressionVariable* v = mvVariables[i];
      if (v && v->GetName() == ahspName) {
        return v;
      }
    }
    return NULL;
  }
  iExpressionVariable* __stdcall FindVariable(iHString* ahspName) const
  {
    iExpressionVariable* v = _FindLocalVariable(ahspName);
    if (v) {
      return v;
    }
    if (mptrParent.IsOK()) {
      return mptrParent->FindVariable(ahspName);
    }
    return NULL;
  }

  ///////////////////////////////////////////////
  Ptr<Op> __stdcall _CreateExpression(const achar *aaszExpr) {
    Ptr<Op> ptrOp = ParseExpr(aaszExpr);
    if (!niIsOK(ptrOp)) {
      niError(niFmt(_A("Can't parse expression '%s'."), aaszExpr));
      return NULL;
    }
    if (!ptrOp->Eval(this)) {
      niError(niFmt(_A("Can't evaluate expression '%s'."), aaszExpr));
      return NULL;
    }
    return ptrOp;
  }

  ///////////////////////////////////////////////
  ni::iExpression * __stdcall CreateExpression(const achar *aaszExpr)
  {
    Ptr<Op> ptrOp = _CreateExpression(aaszExpr);
    if (!ptrOp.IsOK()) return NULL;
    return niNew Expression(this,ptrOp);
  }

  ///////////////////////////////////////////////
  Ptr<iExpressionVariable> __stdcall Eval(const achar* aaszExpr)
  {
    Ptr<iExpressionVariable> ptrResult;
    {
      // local block so that ptrExp is destroyed before ptrResult is returned
      // so that it doesnt get released when the function returns
      Ptr<Op> ptrExp = _CreateExpression(aaszExpr);
      if (!ptrExp.IsOK()) {
        return NULL;
      }
      ptrResult = ptrExp->GetEvalResult();
    }
    return ptrResult;
  }

  ///////////////////////////////////////////////
  cString __stdcall GetEnumValueString(tU32 anValue) const
  {
    if (mpDefaultEnum) {
      niLoop(k,mpDefaultEnum->mnNumValues) {
        const sEnumValueDef& ev = mpDefaultEnum->mpValues[k];
        if (ev.mnValue == anValue) {
#ifdef DEFAULT_ENUM_IS_FULLY_QUALIFIED
          return
              cString(mpDefaultEnum->mEID)
              << _A(".") << ev.maszName;
#else
          return ev.maszName;
#endif
        }
      }
    }
    niLoopit(tEnumMap::const_iterator,it,mmapEnums) {
      const sEnumDef* pDef = it->second;
      if (!pDef || pDef == mpDefaultEnum)
        continue;
      niLoop(k,pDef->mnNumValues) {
        const sEnumValueDef& ev = pDef->mpValues[k];
        if (ev.mnValue == anValue) {
          return cString(pDef->maszName) << _A(".") << ev.maszName;
        }
      }
    }
    return AZEROSTR;
  }

  ///////////////////////////////////////////////
  cString __stdcall GetEnumFlagsString(tU32 anValue) const
  {
    cString strRet;
    if (mpDefaultEnum) {
      niLoopr(k,mpDefaultEnum->mnNumValues) {
        const sEnumValueDef& ev = mpDefaultEnum->mpValues[k];
        const tU32 v = ev.mnValue;
        if (v != 0 && niFlagTest(anValue,v)) {
          if (strRet.IsNotEmpty()) strRet << _A("|");
#ifdef DEFAULT_ENUM_IS_FULLY_QUALIFIED
          strRet << mpDefaultEnum->mEID << _A(".");
#endif
          strRet << ev.maszName;
          niFlagOff(anValue,v);
        }
      }
    }
    niLooprit(auto,it,mmapEnums) {
      const sEnumDef* pDef = it->second;
      if (!pDef || pDef == mpDefaultEnum)
        continue;
      niLoop(k,pDef->mnNumValues) {
        const sEnumValueDef& ev = pDef->mpValues[k];
        const tU32 v = ev.mnValue;
        if (v != 0 && niFlagTest(anValue,v)) {
          if (strRet.IsNotEmpty()) strRet << _A("|");
          strRet << pDef->maszName << _A(".") << ev.maszName;
          niFlagOff(anValue,v);
        }
      }
    }
    if (anValue) {
      if (strRet.IsNotEmpty()) strRet << _A("|");
      strRet << niFmt(_A("0x%08x"),anValue);
    }
    return strRet;
  }

};

enum eMathEqTokenizerCharType {
  eMathEqTokenizerCharType_Normal = 0,
  eMathEqTokenizerCharType_Skip = 1,
  eMathEqTokenizerCharType_Splitter = 2,
  eMathEqTokenizerCharType_SplitterAndToken = 3,
  eMathEqTokenizerCharType_SplitterAndToken2 = 4,
};

static inline bool _IsStringDelimiterChar(const tU32 c) {
  return
      c == '"' ||
      c == '\'' ||
      c == '`';
}

/** Mathematic expression tokenizer, the separator is the space character,
  * separator and tokens are ',', '(', ')', '[', ']', '+', '-', '*', '/', '%',
  * '!', '|', and '^', strings between two quote are one token, '\' char is
  * ignored, '\\' is a '\', and '\"' is '"'.
  */
class cMathEqTokenizer
{
 public:
  cMathEqTokenizer(const achar* aaszExpr) {
    mnPrevChar = eFalse;
    mbPrevBackslash = eFalse;
    mStringStarted = 0;
    mURLStarted = 0;
    mnCurrentToken = 0;
    StringTokenize(aaszExpr);
  }
  ~cMathEqTokenizer() {
  }

  const cString* GetNextToken() {
    if (mnCurrentToken >= mvTokens.size()) {
      return NULL;
    }
    return &mvTokens[mnCurrentToken++];
  }

 private:
  inline tSize StringTokenize(const achar* aaszExpr)
  {
    cString current; current.reserve(64);
    tU32 c = 0, c2 = 0;
    tSize numToks = 0;
    const achar* pIterator = aaszExpr;

    auto handleCurrent = [&] () {
      if (current.IsNotEmpty()) {
        mvTokens.push_back(current);
        // niDebugFmt(("... TOK[%d]: CURRENT: '%s'", mvTokens.size()-1, mvTokens.back()));
        current.Clear(64);
        ++numToks;
      }
    };

    auto handleSplitterAndToken = [&] () {
      handleCurrent();
      cString& r = astl::push_back(mvTokens);
      r.appendChar(c);
      // niDebugFmt(("... TOK[%d]: split: '%s'", mvTokens.size()-1, mvTokens.back()));
    };

    auto handleSplitterAndToken2 = [&] () {
      StrGetNextX(&pIterator); // skip c2
      handleCurrent();
      cString& r = astl::push_back(mvTokens);
      r.appendChar(c);
      r.appendChar(c2);
      // niDebugFmt(("... TOK[%d]: split2: '%s'", mvTokens.size()-1, mvTokens.back()));
    };

    do {
      c = StrGetNextX(&pIterator);
      if (!c) break;
      c2 = StrGetNext(pIterator);
      switch (this->GetCharType(c,c2)) {
        case eMathEqTokenizerCharType_Normal: {
          current.appendChar(c);
          break;
        }
        case eMathEqTokenizerCharType_Skip: {
          break;
        }
        case eMathEqTokenizerCharType_Splitter: {
          handleCurrent();
          break;
        }
        case eMathEqTokenizerCharType_SplitterAndToken: {
          handleSplitterAndToken();
          break;
        }
        case eMathEqTokenizerCharType_SplitterAndToken2: {
          handleSplitterAndToken2();
          break;
        }
        default: break;
      }
    } while (1);
    handleCurrent();
    return numToks;
  }

  eMathEqTokenizerCharType __stdcall GetCharType(const tU32 c, const tU32 c2)
  {
    eMathEqTokenizerCharType type = eMathEqTokenizerCharType_Normal;
    if (mStringStarted == '`') {
      // the backquote makes the remainder of the expression a string
    }
    else if (mStringStarted) {
      if (!mbPrevBackslash && _IsStringDelimiterChar(c)) {
        mStringStarted = 0;
      }
    }
    else if (mURLStarted) {
      if (c == ')' ||
          c == ',' ||
          c == ']') {
        type = eMathEqTokenizerCharType_SplitterAndToken;
        mURLStarted = eFalse;
      }
      else if (StrIsSpace(c)) {
        type = eMathEqTokenizerCharType_Splitter;
        mURLStarted = eFalse;
      }
    }
    else if (c == '\\') {
      if (mbPrevBackslash) {
        mbPrevBackslash = eFalse;
      }
      else {
        type = eMathEqTokenizerCharType_Skip;
        mbPrevBackslash = eTrue;
      }
    }
    else
    {
      if (c == '/' && mnPrevChar == ':') {
        mURLStarted = eTrue;
      }
      else if (_IsStringDelimiterChar(c)) {
        mStringStarted = c;
      }
      else if (!mStringStarted)
      {
        if (StrIsSpace(c)) {
          type = eMathEqTokenizerCharType_Splitter;
        }
        else {
          switch (c) {
            case '-':
            case '+': {
              if (mnPrevChar != 'e' && mnPrevChar != 'E') {
                type = eMathEqTokenizerCharType_SplitterAndToken;
              }
              break;
            }
            case '|':
            case '&':
            case '=': {
              type = (c2 == c) ? eMathEqTokenizerCharType_SplitterAndToken2 : eMathEqTokenizerCharType_SplitterAndToken;
              break;
            }
            case '!':
            case '>':
            case '<': {
              type = (c2 == '=')  ? eMathEqTokenizerCharType_SplitterAndToken2 : eMathEqTokenizerCharType_SplitterAndToken;
              break;
            }
            case ',':
            case '(': case ')': case '[': case ']':
            case '*': case '/':
            case '%': case '^': case '~':
            case '?': case '.': {
              type = eMathEqTokenizerCharType_SplitterAndToken;
              break;
            }
          }
        }
      }
      mbPrevBackslash = eFalse;
    }
    mnPrevChar = (type == eMathEqTokenizerCharType_SplitterAndToken2) ? c2 : c;
    return type;
  }

 private:
  tU32    mnPrevChar;
  tBool   mbPrevBackslash;
  tU32    mStringStarted;
  tU32    mURLStarted;
  tU32  mnCurrentToken;
  astl::vector<cString> mvTokens;
};

template <typename T, typename U>
inline void _FloatAssign(T& d, U x) {
  d = (T)x;
}

enum eNumberType
{
  eNumberType_ErrorNotANumber = -5,
  eNumberType_ErrorScientificExponentExpected = -4,
  eNumberType_ErrorScientificInvalid = -3,
  eNumberType_ErrorScientificNoDotInExponent = -2,
  eNumberType_ErrorHexadecimalOverEight = -1,
  eNumberType_Error = 0,
  eNumberType_Int = 1,
  eNumberType_Float = 2,
  eNumberType_Hexadecimal = 3,
  eNumberType_Scientific = 4,
};

static eNumberType GetNumberType(const achar* aaszNumber, cString* apstrToParse)
{
  const achar* it = aaszNumber;
  cString strTmp;
  if (!apstrToParse)
    apstrToParse = &strTmp;

#define CUR_CHAR    curChar
#define APPEND_CHAR(X)  apstrToParse->appendChar(X)
#define NEXT()      curChar = StrGetNextX(&it)
#define LEN()     apstrToParse->Len()

  eNumberType type = eNumberType_Int;
  tU32 curChar = 0;
  NEXT();
  const tU32 firstchar = CUR_CHAR;
  NEXT();
  if (firstchar == _A('0') && StrToUpper(CUR_CHAR) == _A('X'))
  {
    NEXT();
    type = eNumberType_Hexadecimal;
    while (ni::StrIsXDigit(CUR_CHAR)) {
      APPEND_CHAR(CUR_CHAR);
      NEXT();
    }
    if (LEN() > 8) {
      return eNumberType_ErrorHexadecimalOverEight;
    }
  }
  else
  {
    if (!StrIsNumberPart(firstchar))
      return eNumberType_ErrorNotANumber;

    APPEND_CHAR(firstchar);
    if (firstchar == _A('.'))
      type = eNumberType_Float;
    while (CUR_CHAR == _A('.') || ni::StrIsDigit(CUR_CHAR) ||
           CUR_CHAR == _A('e') || CUR_CHAR == _A('E'))
    {
      if (CUR_CHAR == _A('.'))
      {
        if (type == eNumberType_Scientific)
          return eNumberType_ErrorScientificNoDotInExponent;
        type = eNumberType_Float;
      }

      if (CUR_CHAR == _A('e') || CUR_CHAR == _A('E'))
      {
        if (type != eNumberType_Float && type != eNumberType_Int)
          return eNumberType_ErrorScientificInvalid;
        type = eNumberType_Scientific;
        APPEND_CHAR('e');
        NEXT();
        if (CUR_CHAR == '+' || CUR_CHAR == '-')
        {
          APPEND_CHAR(CUR_CHAR);
          NEXT();
        }
        if (!ni::StrIsDigit(CUR_CHAR))
          return eNumberType_ErrorScientificExponentExpected;
      }
      APPEND_CHAR(CUR_CHAR);
      NEXT();
    }
  }

  return type;
}

static eNumberType ReadNumberEx(eNumberType type, const achar* aaszNum, Var& aOut)
{
  const achar* sTemp;
  switch (type) {
    case eNumberType_Scientific:
    case eNumberType_Float:
      aOut = ni::StrToD(&aaszNum[0],&sTemp);
      return eNumberType_Float;
    case eNumberType_Int:
      aOut = ni::StrToL(&aaszNum[0],&sTemp,10);
      return eNumberType_Int;
    case eNumberType_Hexadecimal:
      aOut = ni::StrToUL(&aaszNum[0],&sTemp,16);
      return eNumberType_Int;
  }
  return type;
}

static eNumberType ReadNumber(const achar* aaszNumber, Var& aOut)
{
  cString strNum;
  eNumberType type = GetNumberType(aaszNumber,&strNum);
  return ReadNumberEx(type,strNum.Chars(),aOut);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Macros
#define BeginOpO(NAME,STR)                                              \
  class Op##NAME : public Op {                  \
public:                                                                 \
Op##NAME() : Op(eMathOperationType_Operator, STR, 2) {} \
Op* Create() const { return niNew Op##NAME(); }

#define BeginOpUO(NAME,STR)                                             \
  class Op##NAME : public Op {                  \
public:                                                                 \
Op##NAME() : Op(eMathOperationType_UnaryOperator, STR, 1) {} \
Op* Create() const { return niNew Op##NAME(); }

#define BeginOpF_(NAME,NUMPARAMS,INIT)                                  \
  class Op##NAME : public Op {                                          \
public:                                                                 \
Op##NAME() : Op(eMathOperationType_Function, _A(#NAME), NUMPARAMS) { INIT; } \
Op* Create() const { return niNew Op##NAME(); }

#define BeginOpF(NAME,NUMPARAMS) BeginOpF_(NAME,NUMPARAMS,;)
#define BeginOpVF(NAME,NUMPARAMS) BeginOpF_(NAME,NUMPARAMS,mbConstant = eFalse;)

#define BeginOpLF(NAME,NUMPARAMS)                         \
  class Op##NAME : public Op {                            \
public:                                                   \
Op##NAME()                                                \
: Op(eMathOperationType_Function, _A(#NAME), NUMPARAMS) { \
  mbLazy = eTrue;                                         \
}                                                         \
Op* Create() const {                                      \
  return niNew Op##NAME();                                \
}

#define BeginOpWF(NAME,NUMPARAMS)                                       \
  class Op##NAME : public Op                    \
  {                                                                     \
  tBool mbConstantOperands;                                             \
  tScalarFloat mfVal;                                                   \
  tScalarFloat mfBase;                                                  \
  tScalarFloat mfTime;                                                  \
  union {                                                               \
    tScalarFloat mfAmplitude;                                           \
    tScalarFloat mfInc;                                                 \
  };                                                                    \
  union {                                                               \
    tScalarFloat mfPhase;                                               \
    tScalarFloat mfMax;                                                 \
    tScalarFloat mnMax;                                                 \
  };                                                                    \
  union {                                                               \
    tScalarFloat mfFreq;                                                \
    tScalarFloat mfTimeForIncrement;                                    \
  };                                                                    \
public:                                                                 \
Op##NAME() :                                                \
Op(eMathOperationType_Function, _A(#NAME), NUMPARAMS)       \
{ mfVal = 0; mfBase = 0; mfTime = 0; mfInc = 0; mnMax = 0; mfFreq = 0; mbConstantOperands = eFalse; mbConstant = eFalse; } \
Op* Create() const { return niNew Op##NAME(); }

#define EndOp() };

#define AddOp(NAME) AddOperation(niNew Op##NAME())

//////////////////////////////////////////////////////////////////////////////////////////////
//! Math variable implementation.

///////////////////////////////////////////////
tBool ExprVar::Copy(const iExpressionVariable* apVar)
{
  switch (Type)
  {
    case eExpressionVariableType_Float:
      {
        static_cast<ExprVarFloat*>(this)->Value = apVar->GetFloat();
        break;
      }

    case eExpressionVariableType_Vec2:
      {
        static_cast<ExprVarVec2*>(this)->Value = apVar->GetVec2();
        break;
      }

    case eExpressionVariableType_Vec3:
      {
        static_cast<ExprVarVec3*>(this)->Value = apVar->GetVec3();
        break;
      }

    case eExpressionVariableType_Vec4:
      {
        static_cast<ExprVarVec4*>(this)->Value = apVar->GetVec4();
        break;
      }

    case eExpressionVariableType_Matrix:
      {
        static_cast<ExprVarMatrix*>(this)->Value = apVar->GetMatrix();
        break;
      }

    case eExpressionVariableType_String:
      {
        static_cast<ExprVarString*>(this)->Value = apVar->GetString();
        break;
      }
    case eExpressionVariableType_IUnknown:
      {
        static_cast<ExprVarIUnknown*>(this)->Value = apVar->GetIUnknown();
        break;
      }

    default:
      niAssertUnreachable("Unreachable.");
      break;
  }
  return eTrue;
}

///////////////////////////////////////////////
//! Clone this variable.
iExpressionVariable* __stdcall  ExprVar::Clone() const
{
  iExpressionVariable* pNew = NULL;
  switch (Type)
  {
    case eExpressionVariableType_Float:
      {
        pNew = niNew ExprVarFloat(hspName);
        static_cast<ExprVarFloat*>(pNew)->Value = this->GetFloat();
        break;
      }

    case eExpressionVariableType_Vec2:
      {
        pNew = niNew ExprVarVec2(hspName);
        static_cast<ExprVarVec2*>(pNew)->Value = this->GetVec2();
        break;
      }

    case eExpressionVariableType_Vec3:
      {
        pNew = niNew ExprVarVec3(hspName);
        static_cast<ExprVarVec3*>(pNew)->Value = this->GetVec3();
        break;
      }

    case eExpressionVariableType_Vec4:
      {
        pNew = niNew ExprVarVec4(hspName);
        static_cast<ExprVarVec4*>(pNew)->Value = this->GetVec4();
        break;
      }

    case eExpressionVariableType_Matrix:
      {
        pNew = niNew ExprVarMatrix(hspName);
        static_cast<ExprVarMatrix*>(pNew)->Value = this->GetMatrix();
        break;
      }

    case eExpressionVariableType_String:
      {
        pNew = niNew ExprVarString(hspName);
        static_cast<ExprVarString*>(pNew)->Value = this->GetString();
        break;
      }
    case eExpressionVariableType_IUnknown:
      {
        pNew = niNew ExprVarIUnknown(hspName);
        static_cast<ExprVarIUnknown*>(pNew)->Value = this->GetIUnknown();
        break;
      }
    default:
      niAssertUnreachable("Unreachable.");
      break;
  }
  return pNew;
}

///////////////////////////////////////////////
//! Get the variable's type.
eExpressionVariableType __stdcall  ExprVar::GetType() const
{
  return Type;
}

///////////////////////////////////////////////
//! Get the variable's flags.
tExpressionVariableFlags __stdcall  ExprVar::GetFlags() const
{
  return Flags;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//! Math operation implementation.

///////////////////////////////////////////////
Op::Op(eMathOperationType aType, const achar* aszName, tU32 aulNumOperands)
{
  mstrName = aszName;
  mstrName.ToLower();
  mType = aType;
  mbEvaluated = eFalse;
  mbConstant = eTrue;
  mbVarArgs = (aulNumOperands == eInvalidHandle);
  mbLazy = eFalse;
  if (aulNumOperands != eInvalidHandle) {
    mvOperands.resize(aulNumOperands);
  }
}

///////////////////////////////////////////////
//! Eval the operation value.
iExpressionVariable* Op::Eval(iExpressionContext* apContext)
{
  if (!apContext) {
    EXPRESSION_TRACE(niFmt(_A("Operation '%s' no context passed to Eval."),GetName()));
    return NULL;
  }

  if (!mbEvaluated) {
    for (tOperandVecIt it = mvOperands.begin(); it != mvOperands.end(); ++it) {
      if (!it->IsOK()) {
        EXPRESSION_TRACE(niFmt(_A("Operand %d not initialized."), astl::iterator_index(mvOperands,it)));
        return NULL;
      }
    }
  }

  if (mptrResult.IsOK() && niFlagIs(mptrResult->GetFlags(),eExpressionVariableFlags_Constant)) {
    return mptrResult;
  }
  else if (mbLazy) {
    if (!SetupEvaluation(apContext)) {
      EXPRESSION_TRACE(niFmt(_A("Operation '%s' evaluation setup failed."),GetName()));
      return NULL;
    }
  }
  else {
    for (tOperandVecIt it = mvOperands.begin(); it != mvOperands.end(); ++it) {
      if (!it->Eval(apContext)) {
        EXPRESSION_TRACE(niFmt(_A("Operation '%s', can't evaluate operand %d (%s)."),
                               GetName(),
                               astl::iterator_index(mvOperands,it),
                               it->GetName()));
        return NULL;
      }
    }

    if (!mbEvaluated) {
      // Merge constant operations into a constant variable
      for (tOperandVecIt it = mvOperands.begin(); it != mvOperands.end(); ++it) {
        if (!it->IsConstant() || !it->ptrOperation.IsOK())
          continue;
        it->ptrVariable = (ExprVar*)it->ptrOperation->GetEvalResult();
        it->ptrVariable->Flags |= eExpressionVariableFlags_Constant;
        it->ptrOperation = NULL;
      }

      if (!SetupEvaluation(apContext)) {
        EXPRESSION_TRACE(niFmt(_A("Operation '%s' evaluation setup failed."),GetName()));
        return NULL;
      }
      if (!niIsOK(mptrResult)) {
        EXPRESSION_TRACE(niFmt(_A("Operation '%s', Result variable not initialized."),GetName()));
        return NULL;
      }
    }
  }

  mbEvaluated = eTrue;
  if (!DoEvaluate(apContext))
    return NULL;

  return mptrResult;
}

///////////////////////////////////////////////
//! Return the name of the operation.
const achar* Op::GetName() const
{
  return mstrName.Chars();
}

///////////////////////////////////////////////
//! Get the operation type.
eMathOperationType Op::GetType() const
{
  return mType;
}

///////////////////////////////////////////////
tBool Op::IsLazy() const
{
  return mbLazy;
}

///////////////////////////////////////////////
//! Return eTrue if the result of this operation will always be the same.
//! \remark The value returned will be accurate only after the first evaluate.
tBool Op::IsConstant() const
{
  return mbConstant && AreOperandsConstant();
}

///////////////////////////////////////////////
//! Return eTrue if the operands are constant.
//! \remark The value returned will be accurate only after the first evaluate.
tBool Op::AreOperandsConstant() const
{
  tBool bOpsConstants = eTrue;
  for (tOperandVecCIt it = mvOperands.begin(); it != mvOperands.end(); ++it)
  {
    bOpsConstants = bOpsConstants && it->IsConstant();
  }
  return bOpsConstants;
}

///////////////////////////////////////////////
//! Get the return value variable of this operation.
iExpressionVariable* Op::GetEvalResult() const
{
  return mptrResult;
}

///////////////////////////////////////////////
tBool Op::IsVarNumOperands() const {
  return mbVarArgs;
}

///////////////////////////////////////////////
//! Get the number of operand variables.
tU32 Op::GetNumOperands() const
{
  return (tU32)mvOperands.size();
}

///////////////////////////////////////////////
tBool Op::SetOperandVariable(tU32 aulIdx, iExpressionVariable* apVar)
{
  if (mbEvaluated)
    return eFalse;

  if (mbVarArgs && aulIdx == mvOperands.size()) {
    // append an operand if last value and vararg operation
    mvOperands.push_back(sOperand());
  }

  if (aulIdx >= mvOperands.size())
    return eFalse;

  mvOperands[aulIdx].ptrVariable = (ExprVar*)apVar;
  mvOperands[aulIdx].ptrOperation = NULL;
  return niIsOK(apVar);
}

///////////////////////////////////////////////
tBool Op::SetOperandOperation(tU32 aulIdx, Ptr<Op> aptrOperation)
{
  if (mbEvaluated)
    return eFalse;

  if (mbVarArgs && aulIdx == mvOperands.size()) {
    // append an operand if last value and vararg operation
    mvOperands.push_back(sOperand());
  }

  if (aulIdx >= mvOperands.size()) return eFalse;
  mvOperands[aulIdx].ptrVariable = NULL;
  mvOperands[aulIdx].ptrOperation = aptrOperation;
  return niIsOK(aptrOperation);
}

///////////////////////////////////////////////
iExpressionVariable* Op::GetOperand(tU32 aulIdx) const
{
  if (aulIdx >= mvOperands.size()) return NULL;
  return mvOperands[aulIdx].GetVariable();
}

///////////////////////////////////////////////
eExpressionVariableType Op::GetCommonOperandsType() const
{
  eExpressionVariableType commonType = eExpressionVariableType_Float;
  for (tOperandVecCIt it = mvOperands.begin(); it != mvOperands.end(); ++it)
  {
    if (it->GetVariable()->GetType() > commonType)
      commonType = it->GetVariable()->GetType();
  }
  return commonType;
}

//////////////////////////////////////////////////////////////////////////////////////////////
#define DEST (pDest[i])
#define OP0 (pSrc0[i])
#define OP1 (pSrc1[i])
#define OP2 (pSrc2[i])
#define OP3 (pSrc3[i])

#define DoWave(YCOMP)                                         \
  tScalarFloat fX, fY;                                        \
  mfTime = (tScalarFloat)fmod(mfTime+OP_GET_FRAME_TIME,1000); \
  fX = (mfPhase+mfTime)*mfFreq;                               \
  fX -= (tScalarFloat)floor(fX);                              \
  YCOMP;                                                      \
  mfVal = (fY*mfAmplitude)+mfBase;

#define DoSwitch(DOOP)                                        \
  tU32 i = 0;                                                 \
  switch (mptrResult->GetType())                              \
  {                                                           \
    case eExpressionVariableType_Float:                       \
      {                                                       \
        typedef tScalarFloat TDEST;                           \
        tScalarFloat* pDest;                                  \
        tScalarFloat fDest = (tScalarFloat)0; pDest = &fDest; \
        DOOP;                                                 \
        mptrResult->SetFloat(fDest);                          \
        break;                                                \
      }                                                       \
    case eExpressionVariableType_Vec2:                     \
      {                                                       \
        typedef tVectorFloat TDEST;                           \
        tVectorFloat* pDest;                                  \
        sVec2<tVectorFloat> vDest; pDest = vDest.ptr();    \
        for ( ; i < 2; ++i) { DOOP; }                         \
        mptrResult->SetVec2(vDest);                        \
        break;                                                \
      }                                                       \
    case eExpressionVariableType_Vec3:                     \
      {                                                       \
        typedef tVectorFloat TDEST;                           \
        tVectorFloat* pDest;                                  \
        sVec3<tVectorFloat> vDest; pDest = vDest.ptr();    \
        for ( ; i < 3; ++i) { DOOP; }                         \
        mptrResult->SetVec3(vDest);                        \
        break;                                                \
      }                                                       \
    case eExpressionVariableType_Vec4:                     \
      {                                                       \
        typedef tVectorFloat TDEST;                           \
        tVectorFloat* pDest;                                  \
        sVec4<tVectorFloat> vDest; pDest = vDest.ptr();    \
        for ( ; i < 4; ++i) { DOOP; }                         \
        mptrResult->SetVec4(vDest);                        \
        break;                                                \
      }                                                       \
    case eExpressionVariableType_Matrix:                      \
      {                                                       \
        typedef tVectorFloat TDEST;                           \
        tVectorFloat* pDest;                                  \
        sMatrix<tVectorFloat> mtxDest; pDest = mtxDest.ptr(); \
        for ( ; i < 16; ++i) { DOOP; }                        \
        mptrResult->SetMatrix(mtxDest);                       \
        break;                                                \
      }                                                       \
    default:                                                  \
      return eFalse;                                          \
  }

#define DoSwitch1(DOOP)                                                 \
  tU32 i = 0;                                                           \
  switch (mptrResult->GetType())                                        \
  {                                                                     \
    case eExpressionVariableType_Float:                                 \
      {                                                                 \
        const tScalarFloat* pSrc0;                                      \
        typedef tScalarFloat TDEST;                                     \
        tScalarFloat* pDest;                                            \
        tScalarFloat fDest = (tScalarFloat)0; pDest = &fDest;           \
        tScalarFloat fSrc0 = mvOperands[0].GetVariable()->GetFloat(); pSrc0 = &fSrc0; \
        DOOP;                                                           \
        mptrResult->SetFloat(fDest);                                    \
        break;                                                          \
      }                                                                 \
    case eExpressionVariableType_Vec2:                               \
      {                                                                 \
        const tVectorFloat* pSrc0;                                      \
        typedef tVectorFloat TDEST;                                     \
        tVectorFloat* pDest;                                            \
        sVec2<tVectorFloat> vDest; pDest = vDest.ptr();              \
        sVec2<tVectorFloat> vSrc0 = mvOperands[0].GetVariable()->GetVec2(); pSrc0 = vSrc0.ptr(); \
        for ( ; i < 2; ++i) { DOOP; }                                   \
        mptrResult->SetVec2(vDest);                                  \
        break;                                                          \
      }                                                                 \
    case eExpressionVariableType_Vec3:                               \
      {                                                                 \
        const tVectorFloat* pSrc0;                                      \
        typedef tVectorFloat TDEST;                                     \
        tVectorFloat* pDest;                                            \
        sVec3<tVectorFloat> vDest; pDest = vDest.ptr();              \
        sVec3<tVectorFloat> vSrc0 = mvOperands[0].GetVariable()->GetVec3(); pSrc0 = vSrc0.ptr(); \
        for ( ; i < 3; ++i) { DOOP; }                                   \
        mptrResult->SetVec3(vDest);                                  \
        break;                                                          \
      }                                                                 \
    case eExpressionVariableType_Vec4:                               \
      {                                                                 \
        const tVectorFloat* pSrc0;                                      \
        typedef tVectorFloat TDEST;                                     \
        tVectorFloat* pDest;                                            \
        sVec4<tVectorFloat> vDest; pDest = vDest.ptr();              \
        sVec4<tVectorFloat> vSrc0 = mvOperands[0].GetVariable()->GetVec4(); pSrc0 = vSrc0.ptr(); \
        for ( ; i < 4; ++i) { DOOP; }                                   \
        mptrResult->SetVec4(vDest);                                  \
        break;                                                          \
      }                                                                 \
    case eExpressionVariableType_Matrix:                                \
      {                                                                 \
        const tVectorFloat* pSrc0;                                      \
        typedef tVectorFloat TDEST;                                     \
        tVectorFloat* pDest;                                            \
        sMatrix<tVectorFloat> mtxDest; pDest = mtxDest.ptr();           \
        sMatrix<tVectorFloat> mtxSrc0 = mvOperands[0].GetVariable()->GetMatrix(); pSrc0 = mtxSrc0.ptr(); \
        for ( ; i < 16; ++i) { DOOP; }                                  \
        mptrResult->SetMatrix(mtxDest);                                 \
        break;                                                          \
      }                                                                 \
    default:                                                            \
      return eFalse;                                                    \
  }

#define DoSwitch2(DOOP)                                                 \
  tU32 i = 0;                                                           \
  switch (mptrResult->GetType())                                        \
  {                                                                     \
    case eExpressionVariableType_Float:                                 \
      {                                                                 \
        typedef tScalarFloat TDEST;                                     \
        tScalarFloat* pDest;                                            \
        const tScalarFloat* pSrc0;                                      \
        const tScalarFloat* pSrc1;                                      \
        tScalarFloat fDest = (tScalarFloat)0; pDest = &fDest;           \
        tScalarFloat fSrc0 = mvOperands[0].GetVariable()->GetFloat(); pSrc0 = &fSrc0; \
        tScalarFloat fSrc1 = mvOperands[1].GetVariable()->GetFloat(); pSrc1 = &fSrc1; \
        { DOOP; }                                                       \
        mptrResult->SetFloat(fDest);                                    \
        break;                                                          \
      }                                                                 \
    case eExpressionVariableType_Vec2:                               \
      {                                                                 \
        typedef tVectorFloat TDEST;                                     \
        tVectorFloat* pDest;                                            \
        const tVectorFloat* pSrc0;                                      \
        const tVectorFloat* pSrc1;                                      \
        sVec2<tVectorFloat> vDest; pDest = vDest.ptr();              \
        sVec2<tVectorFloat> vSrc0 = mvOperands[0].GetVariable()->GetVec2(); pSrc0 = vSrc0.ptr(); \
        sVec2<tVectorFloat> vSrc1 = mvOperands[1].GetVariable()->GetVec2(); pSrc1 = vSrc1.ptr(); \
        for ( ; i < 2; ++i) { DOOP; }                                   \
        mptrResult->SetVec2(vDest);                                  \
        break;                                                          \
      }                                                                 \
    case eExpressionVariableType_Vec3:                               \
      {                                                                 \
        typedef tVectorFloat TDEST;                                     \
        tVectorFloat* pDest;                                            \
        const tVectorFloat* pSrc0;                                      \
        const tVectorFloat* pSrc1;                                      \
        sVec3<tVectorFloat> vDest; pDest = vDest.ptr();              \
        sVec3<tVectorFloat> vSrc0 = mvOperands[0].GetVariable()->GetVec3(); pSrc0 = vSrc0.ptr(); \
        sVec3<tVectorFloat> vSrc1 = mvOperands[1].GetVariable()->GetVec3(); pSrc1 = vSrc1.ptr(); \
        for ( ; i < 3; ++i) { DOOP; }                                   \
        mptrResult->SetVec3(vDest);                                  \
        break;                                                          \
      }                                                                 \
    case eExpressionVariableType_Vec4:                               \
      {                                                                 \
        typedef tVectorFloat TDEST;                                     \
        tVectorFloat* pDest;                                            \
        const tVectorFloat* pSrc0;                                      \
        const tVectorFloat* pSrc1;                                      \
        sVec4<tVectorFloat> vDest; pDest = vDest.ptr();              \
        sVec4<tVectorFloat> vSrc0 = mvOperands[0].GetVariable()->GetVec4(); pSrc0 = vSrc0.ptr(); \
        sVec4<tVectorFloat> vSrc1 = mvOperands[1].GetVariable()->GetVec4(); pSrc1 = vSrc1.ptr(); \
        for ( ; i < 4; ++i) { DOOP; }                                   \
        mptrResult->SetVec4(vDest);                                  \
        break;                                                          \
      }                                                                 \
    case eExpressionVariableType_Matrix:                                \
      {                                                                 \
        typedef tVectorFloat TDEST;                                     \
        tVectorFloat* pDest;                                            \
        const tVectorFloat* pSrc0;                                      \
        const tVectorFloat* pSrc1;                                      \
        sMatrix<tVectorFloat> mtxDest; pDest = mtxDest.ptr();           \
        sMatrix<tVectorFloat> mtxSrc0 = mvOperands[0].GetVariable()->GetMatrix(); pSrc0 = mtxSrc0.ptr(); \
        sMatrix<tVectorFloat> mtxSrc1 = mvOperands[1].GetVariable()->GetMatrix(); pSrc1 = mtxSrc1.ptr(); \
        for ( ; i < 16; ++i) { DOOP; }                                  \
        mptrResult->SetMatrix(mtxDest);                                 \
        break;                                                          \
      }                                                                 \
    default:                                                            \
      return eFalse;                                                    \
  }

#define DoSwitch3(DOOP)                                                 \
  tU32 i = 0;                                                           \
  switch (mptrResult->GetType())                                        \
  {                                                                     \
    case eExpressionVariableType_Float:                                 \
      {                                                                 \
        typedef tScalarFloat TDEST;                                     \
        tScalarFloat* pDest;                                            \
        const tScalarFloat* pSrc0;                                      \
        const tScalarFloat* pSrc1;                                      \
        const tScalarFloat* pSrc2;                                      \
        tScalarFloat fDest = (tScalarFloat)0; pDest = &fDest;           \
        tScalarFloat fSrc0 = mvOperands[0].GetVariable()->GetFloat(); pSrc0 = &fSrc0; \
        tScalarFloat fSrc1 = mvOperands[1].GetVariable()->GetFloat(); pSrc1 = &fSrc1; \
        tScalarFloat fSrc2 = mvOperands[2].GetVariable()->GetFloat(); pSrc2 = &fSrc2; \
        DOOP;                                                           \
        mptrResult->SetFloat(fDest);                                    \
        break;                                                          \
      }                                                                 \
    case eExpressionVariableType_Vec2:                               \
      {                                                                 \
        typedef tVectorFloat TDEST;                                     \
        tVectorFloat* pDest;                                            \
        const tVectorFloat* pSrc0;                                      \
        const tVectorFloat* pSrc1;                                      \
        const tVectorFloat* pSrc2;                                      \
        sVec2<tVectorFloat> vDest; pDest = vDest.ptr();              \
        sVec2<tVectorFloat> vSrc0 = mvOperands[0].GetVariable()->GetVec2(); pSrc0 = vSrc0.ptr(); \
        sVec2<tVectorFloat> vSrc1 = mvOperands[1].GetVariable()->GetVec2(); pSrc1 = vSrc1.ptr(); \
        sVec2<tVectorFloat> vSrc2 = mvOperands[2].GetVariable()->GetVec2(); pSrc2 = vSrc2.ptr(); \
        for ( ; i < 2; ++i) { DOOP; }                                   \
        mptrResult->SetVec2(vDest);                                  \
        break;                                                          \
      }                                                                 \
    case eExpressionVariableType_Vec3:                               \
      {                                                                 \
        typedef tVectorFloat TDEST;                                     \
        tVectorFloat* pDest;                                            \
        const tVectorFloat* pSrc0;                                      \
        const tVectorFloat* pSrc1;                                      \
        const tVectorFloat* pSrc2;                                      \
        sVec3<tVectorFloat> vDest; pDest = vDest.ptr();              \
        sVec3<tVectorFloat> vSrc0 = mvOperands[0].GetVariable()->GetVec3(); pSrc0 = vSrc0.ptr(); \
        sVec3<tVectorFloat> vSrc1 = mvOperands[1].GetVariable()->GetVec3(); pSrc1 = vSrc1.ptr(); \
        sVec3<tVectorFloat> vSrc2 = mvOperands[2].GetVariable()->GetVec3(); pSrc2 = vSrc2.ptr(); \
        for ( ; i < 3; ++i) { DOOP; }                                   \
        mptrResult->SetVec3(vDest);                                  \
        break;                                                          \
      }                                                                 \
    case eExpressionVariableType_Vec4:                               \
      {                                                                 \
        typedef tVectorFloat TDEST;                                     \
        tVectorFloat* pDest;                                            \
        const tVectorFloat* pSrc0;                                      \
        const tVectorFloat* pSrc1;                                      \
        const tVectorFloat* pSrc2;                                      \
        sVec4<tVectorFloat> vDest; pDest = vDest.ptr();              \
        sVec4<tVectorFloat> vSrc0 = mvOperands[0].GetVariable()->GetVec4(); pSrc0 = vSrc0.ptr(); \
        sVec4<tVectorFloat> vSrc1 = mvOperands[1].GetVariable()->GetVec4(); pSrc1 = vSrc1.ptr(); \
        sVec4<tVectorFloat> vSrc2 = mvOperands[2].GetVariable()->GetVec4(); pSrc2 = vSrc2.ptr(); \
        for ( ; i < 4; ++i) { DOOP; }                                   \
        mptrResult->SetVec4(vDest);                                  \
        break;                                                          \
      }                                                                 \
    case eExpressionVariableType_Matrix:                                \
      {                                                                 \
        typedef tVectorFloat TDEST;                                     \
        tVectorFloat* pDest;                                            \
        const tVectorFloat* pSrc0;                                      \
        const tVectorFloat* pSrc1;                                      \
        const tVectorFloat* pSrc2;                                      \
        sMatrix<tVectorFloat> mtxDest; pDest = mtxDest.ptr();           \
        sMatrix<tVectorFloat> mtxSrc0 = mvOperands[0].GetVariable()->GetMatrix(); pSrc0 = mtxSrc0.ptr(); \
        sMatrix<tVectorFloat> mtxSrc1 = mvOperands[1].GetVariable()->GetMatrix(); pSrc1 = mtxSrc1.ptr(); \
        sMatrix<tVectorFloat> mtxSrc2 = mvOperands[2].GetVariable()->GetMatrix(); pSrc2 = mtxSrc2.ptr(); \
        for ( ; i < 16; ++i) { DOOP; }                                  \
        mptrResult->SetMatrix(mtxDest);                                 \
        break;                                                          \
      }                                                                 \
    default:                                                            \
      return eFalse;                                                    \
  }

// Group operation
class OpGroup : public Op
{
 public:
  OpGroup() :
      Op(eMathOperationType_Group, _A("#GROUP#"), 1)
  {
  }

  Op* Create() const { return niNew OpGroup(); }

  tBool SetupEvaluation(iExpressionContext*) {
    mptrResult = mvOperands[0].GetVariable();
    return eTrue;
  }

  tBool DoEvaluate(iExpressionContext*) {
    mptrResult = mvOperands[0].GetVariable();
    return eTrue;
  }
};

// URL operation
class OpURL : public Op
{
 public:
  Ptr<iExpressionURLResolver> mptrResolver;
  cString mstrURL;
  tBool mbShouldEval;

  OpURL(iExpressionURLResolver* apResolver, const cString& aEncodedURL) :
      Op(eMathOperationType_Function, _A("#URL#"), 0)
  {
    niAssert(apResolver != NULL);
    mbConstant = eFalse; // the result can change potentially at each eval
    mptrResolver = apResolver;
    StringDecodeUrl(mstrURL, aEncodedURL);
    mbShouldEval = eTrue;
  }

  Op* Create() const { return niNew OpURL(mptrResolver,mstrURL); }

  tBool SetupEvaluation(iExpressionContext* ctx) {
    if(!mptrResolver.IsOK() || mstrURL.empty())
      return eFalse;
    mptrResult = ctx->CreateVariableFromVar(NULL, mptrResolver->ResolveURL(mstrURL.Chars()));
    // Don't eval in the first DoEvaluate since we already have the initial value...
    mbShouldEval = eFalse;
    return eTrue;
  }

  tBool DoEvaluate(iExpressionContext* ctx) {
    if (mbShouldEval) {
      mptrResult = ctx->CreateVariableFromVar(NULL, mptrResolver->ResolveURL(mstrURL.Chars()));
    }
    // We always re-eval the value at the next DoEvaluate(iExpressionContext*)
    mbShouldEval = eTrue;
    return mptrResult.IsOK();
  }
};

// + operator
BeginOpO(OperatorAdd,_A("+"))
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  if (mptrResult->GetType() == eExpressionVariableType_String) {
    cString out;
    out << mvOperands[0].GetVariable()->GetString();
    out << mvOperands[1].GetVariable()->GetString();
    mptrResult->SetString(out);
  }
  else {
    DoSwitch2(DEST=OP0+OP1);
  }
  return eTrue;
}
EndOp()

// + unary operator
BeginOpUO(OperatorPlus,_A("+"))
tBool SetupEvaluation(iExpressionContext*)
{
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  mptrResult = mvOperands[0].GetVariable();
  return eTrue;
}
EndOp()

// - operator
BeginOpO(OperatorSub,_A("-"))
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch2(DEST=OP0-OP1);
  return eTrue;
}
EndOp()

// - unary operator
BeginOpUO(OperatorMinus,_A("-"))
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch1(DEST=-OP0);
  return eTrue;
}
EndOp()

// * operator
BeginOpO(OperatorMul,_A("*"))
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch2(DEST=OP0*OP1);
  return eTrue;
}
EndOp()

// / operator
BeginOpO(OperatorDiv,_A("/"))
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch2(DEST=OP0/OP1);
  return eTrue;
}
EndOp()

// % operator
#ifdef USE_MODULO_OP
BeginOpO(OperatorMod,_A("%"))
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch2(DEST=ni::FMod(OP0,OP1));
  return eTrue;
}
EndOp()
#endif

// ~ unary operator
BeginOpUO(OperatorBitNegate,_A("~"))
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch1(DEST=(TDEST)~(int)OP0);
  return eTrue;
}
EndOp()

// | operator
BeginOpO(OperatorBitOr,_A("|"))
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch2(DEST=(TDEST)((int)OP0|(int)OP1));
  return eTrue;
}
EndOp()

// ^ operator
BeginOpO(OperatorBitXor,_A("^"))
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch2(DEST=(TDEST)((int)OP0^(int)OP1));
  return eTrue;
}
EndOp()

// & operator
BeginOpO(OperatorBitAnd,_A("&"))
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch2(DEST=(TDEST)((int)OP0&(int)OP1));
  return eTrue;
}
EndOp()

// logical && operator
BeginOpO(OperatorLogicalAnd,_A("&&"))
tBool SetupEvaluation(iExpressionContext*) {
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*) {
  mptrResult->SetFloat(mvOperands[0].ToBool() && mvOperands[1].ToBool());
  return eTrue;
}
EndOp()

// logical || operator
BeginOpO(OperatorLogicalOr,_A("||"))
tBool SetupEvaluation(iExpressionContext*) {
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*) {
  mptrResult->SetFloat(mvOperands[0].ToBool() || mvOperands[1].ToBool());
  return eTrue;
}
EndOp()

// logical ! operator
BeginOpUO(OperatorLogicalNot,_A("!"))
tBool SetupEvaluation(iExpressionContext*) {
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*) {
  mptrResult->SetFloat(!mvOperands[0].ToBool());
  return eTrue;
}
EndOp()

// logical !! operator (TO_BOOL)
BeginOpUO(OperatorLogicalToBool,_A("!!"))
tBool SetupEvaluation(iExpressionContext*) {
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*) {
  mptrResult->SetFloat(mvOperands[0].ToBool());
  return eTrue;
}
EndOp()

// cmp == operator
BeginOpO(OperatorCmpEq,_A("=="))
tBool SetupEvaluation(iExpressionContext*) {
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*) {
  const eExpressionVariableType op0Type = mvOperands[0].GetVariable()->GetType();
  const eExpressionVariableType op1Type = mvOperands[1].GetVariable()->GetType();
  if (op0Type != op1Type) {
    mptrResult->SetFloat(0);
  }
  else {
    if (op0Type == eExpressionVariableType_String) {
      mptrResult->SetFloat(mvOperands[0].GetVariable()->GetString() ==
                           mvOperands[1].GetVariable()->GetString());
    }
    else {
      DoSwitch2(DEST=(TDEST)(OP0 == OP1));
    }
  }
  return eTrue;
}
EndOp()

// cmp != operator
BeginOpO(OperatorCmpNotEq,_A("!="))
tBool SetupEvaluation(iExpressionContext*) {
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*) {
  const eExpressionVariableType op0Type = mvOperands[0].GetVariable()->GetType();
  const eExpressionVariableType op1Type = mvOperands[1].GetVariable()->GetType();
  if (op0Type != op1Type) {
    mptrResult->SetFloat(1);
  }
  else {
    if (op0Type == eExpressionVariableType_String) {
      mptrResult->SetFloat(mvOperands[0].GetVariable()->GetString() !=
                           mvOperands[1].GetVariable()->GetString());
    }
    else {
      DoSwitch2(DEST=(TDEST)(OP0 != OP1));
    }
  }
  return eTrue;
}
EndOp()

// cmp < operator
BeginOpO(OperatorCmpLt,_A("<"))
tBool SetupEvaluation(iExpressionContext*) {
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*) {
  const eExpressionVariableType op0Type = mvOperands[0].GetVariable()->GetType();
  const eExpressionVariableType op1Type = mvOperands[1].GetVariable()->GetType();
  if (op0Type != op1Type) {
    mptrResult->SetFloat(op0Type < op1Type);
  }
  else {
    if (op0Type == eExpressionVariableType_String) {
      mptrResult->SetFloat(mvOperands[0].GetVariable()->GetString() <
                           mvOperands[1].GetVariable()->GetString());
    }
    else {
      DoSwitch2(DEST=(TDEST)(OP0 < OP1));
    }
  }
  return eTrue;
}
EndOp()

// cmp <= operator
BeginOpO(OperatorCmpLte,_A("<="))
tBool SetupEvaluation(iExpressionContext*) {
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*) {
  const eExpressionVariableType op0Type = mvOperands[0].GetVariable()->GetType();
  const eExpressionVariableType op1Type = mvOperands[1].GetVariable()->GetType();
  if (op0Type != op1Type) {
    mptrResult->SetFloat(op0Type <= op1Type);
  }
  else {
    if (op0Type == eExpressionVariableType_String) {
      mptrResult->SetFloat(mvOperands[0].GetVariable()->GetString() <=
                           mvOperands[1].GetVariable()->GetString());
    }
    else {
      DoSwitch2(DEST=(TDEST)(OP0 <= OP1));
    }
  }
  return eTrue;
}
EndOp()

// cmp > operator
BeginOpO(OperatorCmpGt,_A(">"))
tBool SetupEvaluation(iExpressionContext*) {
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*) {
  const eExpressionVariableType op0Type = mvOperands[0].GetVariable()->GetType();
  const eExpressionVariableType op1Type = mvOperands[1].GetVariable()->GetType();
  if (op0Type != op1Type) {
    mptrResult->SetFloat(op0Type > op1Type);
  }
  else {
    if (op0Type == eExpressionVariableType_String) {
      mptrResult->SetFloat(mvOperands[0].GetVariable()->GetString() >
                           mvOperands[1].GetVariable()->GetString());
    }
    else {
      DoSwitch2(DEST=(TDEST)(OP0 > OP1));
    }
  }
  return eTrue;
}
EndOp()

// cmp >= operator
BeginOpO(OperatorCmpGte,_A(">="))
tBool SetupEvaluation(iExpressionContext*) {
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*) {
  const eExpressionVariableType op0Type = mvOperands[0].GetVariable()->GetType();
  const eExpressionVariableType op1Type = mvOperands[1].GetVariable()->GetType();
  if (op0Type != op1Type) {
    mptrResult->SetFloat(op0Type >= op1Type);
  }
  else {
    if (op0Type == eExpressionVariableType_String) {
      mptrResult->SetFloat(mvOperands[0].GetVariable()->GetString() >=
                           mvOperands[1].GetVariable()->GetString());
    }
    else {
      DoSwitch2(DEST=(TDEST)(OP0 >= OP1));
    }
  }
  return eTrue;
}
EndOp()

//! A 2D vector.
BeginOpF(Vec2,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec2);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  sVec2<tVectorFloat> dest;
  dest.x = (tVectorFloat)mvOperands[0].GetVariable()->GetFloat();
  dest.y = (tVectorFloat)mvOperands[1].GetVariable()->GetFloat();
  mptrResult->SetVec2(dest);
  return eTrue;
}
EndOp()

//! A 3D vector.
BeginOpF(Vec3,3)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec3);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  sVec3<tVectorFloat> dest;
  dest.x = (tVectorFloat)mvOperands[0].GetVariable()->GetFloat();
  dest.y = (tVectorFloat)mvOperands[1].GetVariable()->GetFloat();
  dest.z = (tVectorFloat)mvOperands[2].GetVariable()->GetFloat();
  mptrResult->SetVec3(dest);
  return eTrue;
}
EndOp()

//! A 3D vector.
BeginOpF(RGB,3)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec3,eExpressionVariableFlags_Color);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  sVec3<tVectorFloat> dest;
  dest.x = (tVectorFloat)mvOperands[0].GetVariable()->GetFloat();
  dest.y = (tVectorFloat)mvOperands[1].GetVariable()->GetFloat();
  dest.z = (tVectorFloat)mvOperands[2].GetVariable()->GetFloat();
  mptrResult->SetVec3(dest);
  return eTrue;
}
EndOp()

//! A 4D vector.
BeginOpF(Vec4,4)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  sVec4<tVectorFloat> dest;
  dest.x = (tVectorFloat)mvOperands[0].GetVariable()->GetFloat();
  dest.y = (tVectorFloat)mvOperands[1].GetVariable()->GetFloat();
  dest.z = (tVectorFloat)mvOperands[2].GetVariable()->GetFloat();
  dest.w = (tVectorFloat)mvOperands[3].GetVariable()->GetFloat();
  mptrResult->SetVec4(dest);
  return eTrue;
}
EndOp()

//! A 4D color.
BeginOpF(RGBA,4)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4,eExpressionVariableFlags_Color);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  sVec4<tVectorFloat> dest;
  dest.x = (tVectorFloat)mvOperands[0].GetVariable()->GetFloat();
  dest.y = (tVectorFloat)mvOperands[1].GetVariable()->GetFloat();
  dest.z = (tVectorFloat)mvOperands[2].GetVariable()->GetFloat();
  dest.w = (tVectorFloat)mvOperands[3].GetVariable()->GetFloat();
  mptrResult->SetVec4(dest);
  return eTrue;
}
EndOp()

//! A quaternion vector.
BeginOpF(Quat,4)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4,eExpressionVariableFlags_Quat);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  sQuat<tVectorFloat> dest;
  dest.x = (tVectorFloat)mvOperands[0].GetVariable()->GetFloat();
  dest.y = (tVectorFloat)mvOperands[1].GetVariable()->GetFloat();
  dest.z = (tVectorFloat)mvOperands[2].GetVariable()->GetFloat();
  dest.w = (tVectorFloat)mvOperands[3].GetVariable()->GetFloat();
  mptrResult->SetVec4(dest);
  return eTrue;
}
EndOp()

//! A rectangle.
BeginOpF(Rect,4)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  sRect<tVectorFloat> dest(
      (tVectorFloat)mvOperands[0].GetVariable()->GetFloat(),
      (tVectorFloat)mvOperands[1].GetVariable()->GetFloat(),
      (tVectorFloat)mvOperands[2].GetVariable()->GetFloat(),
      (tVectorFloat)mvOperands[3].GetVariable()->GetFloat());
  mptrResult->SetVec4(dest);
  return eTrue;
}
EndOp()

//! A matrix.
BeginOpF(Matrix,16)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Matrix);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  sMatrix<tVectorFloat> dest;
  dest._11 = (tVectorFloat)mvOperands[ 0].GetVariable()->GetFloat();
  dest._12 = (tVectorFloat)mvOperands[ 1].GetVariable()->GetFloat();
  dest._13 = (tVectorFloat)mvOperands[ 2].GetVariable()->GetFloat();
  dest._14 = (tVectorFloat)mvOperands[ 3].GetVariable()->GetFloat();
  dest._21 = (tVectorFloat)mvOperands[ 4].GetVariable()->GetFloat();
  dest._22 = (tVectorFloat)mvOperands[ 5].GetVariable()->GetFloat();
  dest._23 = (tVectorFloat)mvOperands[ 6].GetVariable()->GetFloat();
  dest._24 = (tVectorFloat)mvOperands[ 7].GetVariable()->GetFloat();
  dest._31 = (tVectorFloat)mvOperands[ 8].GetVariable()->GetFloat();
  dest._32 = (tVectorFloat)mvOperands[ 9].GetVariable()->GetFloat();
  dest._33 = (tVectorFloat)mvOperands[10].GetVariable()->GetFloat();
  dest._34 = (tVectorFloat)mvOperands[11].GetVariable()->GetFloat();
  dest._41 = (tVectorFloat)mvOperands[12].GetVariable()->GetFloat();
  dest._42 = (tVectorFloat)mvOperands[13].GetVariable()->GetFloat();
  dest._43 = (tVectorFloat)mvOperands[14].GetVariable()->GetFloat();
  dest._44 = (tVectorFloat)mvOperands[15].GetVariable()->GetFloat();
  mptrResult->SetMatrix(dest);
  return eTrue;
}
EndOp()

//! Cast to real number.
BeginOpF(ToReal,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  mptrResult->Copy(mvOperands[0].GetVariable());
  return eTrue;
}
EndOp()

//! Cast to real number, synonym of ToReal
BeginOpF(ToFloat,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  mptrResult->Copy(mvOperands[0].GetVariable());
  return eTrue;
}
EndOp()

//! Cast to a bool, synonym of !!
BeginOpF(ToBool,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  mptrResult->SetFloat(mvOperands[0].ToBool());
  return eTrue;
}
EndOp()

//! Cast to 2d vector.
BeginOpF(ToVec2,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec2);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  mptrResult->Copy(mvOperands[0].GetVariable());
  return eTrue;
}
EndOp()

//! Cast to 3d vector.
BeginOpF(ToVec3,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec3);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  mptrResult->Copy(mvOperands[0].GetVariable());
  return eTrue;
}
EndOp()

//! Cast to 3d color.
BeginOpF(ToRGB,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec3,eExpressionVariableFlags_Color);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  mptrResult->Copy(mvOperands[0].GetVariable());
  return eTrue;
}
EndOp()

//! Cast to 4d vector.
BeginOpF(ToVec4,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  mptrResult->Copy(mvOperands[0].GetVariable());
  return eTrue;
}
EndOp()

//! Cast to 4d color.
BeginOpF(ToRGBA,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4,eExpressionVariableFlags_Color);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  mptrResult->Copy(mvOperands[0].GetVariable());
  return eTrue;
}
EndOp()

//! Cast to quaternion.
BeginOpF(ToQuat,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4,eExpressionVariableFlags_Quat);
 return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  mptrResult->Copy(mvOperands[0].GetVariable());
  return eTrue;
}
EndOp()

//! Cast to matrix.
BeginOpF(ToMatrix,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Matrix);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  mptrResult->Copy(mvOperands[0].GetVariable());
  return eTrue;
}
EndOp()

//! Cast to string.
BeginOpF(ToString,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_String);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  mptrResult->Copy(mvOperands[0].GetVariable());
  return eTrue;
}
EndOp()

//! Return a string which contains the typename.
BeginOpF(GetTypeString,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_String);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  cString typeString = niEnumToChars(eExpressionVariableType,mvOperands[0].GetVariable()->GetType());
  mptrResult->SetString(typeString);
  return eTrue;
}
EndOp()

//! Check type
BeginOpF(IsFloat,1)
tBool SetupEvaluation(iExpressionContext*) {
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*) {
  mptrResult->SetFloat(mvOperands[0].GetVariable()->GetType() == eExpressionVariableType_Float);
  return eTrue;
}
EndOp()
BeginOpF(IsVec2,1)
tBool SetupEvaluation(iExpressionContext*) {
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*) {
  mptrResult->SetFloat(mvOperands[0].GetVariable()->GetType() == eExpressionVariableType_Vec2);
  return eTrue;
}
EndOp()
BeginOpF(IsVec3,1)
tBool SetupEvaluation(iExpressionContext*) {
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*) {
  mptrResult->SetFloat(mvOperands[0].GetVariable()->GetType() == eExpressionVariableType_Vec3);
  return eTrue;
}
EndOp()
BeginOpF(IsVec4,1)
tBool SetupEvaluation(iExpressionContext*) {
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*) {
  mptrResult->SetFloat(mvOperands[0].GetVariable()->GetType() == eExpressionVariableType_Vec4);
  return eTrue;
}
EndOp()
BeginOpF(IsMatrix,1)
tBool SetupEvaluation(iExpressionContext*) {
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*) {
  mptrResult->SetFloat(mvOperands[0].GetVariable()->GetType() == eExpressionVariableType_Matrix);
  return eTrue;
}
EndOp()
BeginOpF(IsString,1)
tBool SetupEvaluation(iExpressionContext*) {
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*) {
  mptrResult->SetFloat(mvOperands[0].GetVariable()->GetType() == eExpressionVariableType_String);
  return eTrue;
}
EndOp()

// GetX(vec)
BeginOpF(GetX,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  iExpressionVariable* pVar = mvOperands[0].GetVariable();
  switch (pVar->GetType())
  {
    case eExpressionVariableType_Float: mptrResult->SetFloat(static_cast<const ExprVarFloat*>(pVar)->Value);  break;
    case eExpressionVariableType_Vec2: mptrResult->SetFloat(static_cast<const ExprVarVec2*>(pVar)->Value.x);  break;
    case eExpressionVariableType_Vec3: mptrResult->SetFloat(static_cast<const ExprVarVec3*>(pVar)->Value.x);  break;
    case eExpressionVariableType_Vec4: mptrResult->SetFloat(static_cast<const ExprVarVec4*>(pVar)->Value.x);  break;
    case eExpressionVariableType_Matrix:  mptrResult->SetFloat(static_cast<const ExprVarMatrix*>(pVar)->Value._11); break;
  }
  return eTrue;
}
EndOp()

// GetY(vec)
BeginOpF(GetY,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  iExpressionVariable* pVar = mvOperands[0].GetVariable();
  switch (pVar->GetType())
  {
    case eExpressionVariableType_Float: mptrResult->SetFloat(static_cast<const ExprVarFloat*>(pVar)->Value);  break;
    case eExpressionVariableType_Vec2: mptrResult->SetFloat(static_cast<const ExprVarVec2*>(pVar)->Value.y);  break;
    case eExpressionVariableType_Vec3: mptrResult->SetFloat(static_cast<const ExprVarVec3*>(pVar)->Value.y);  break;
    case eExpressionVariableType_Vec4: mptrResult->SetFloat(static_cast<const ExprVarVec4*>(pVar)->Value.y);  break;
    case eExpressionVariableType_Matrix:  mptrResult->SetFloat(static_cast<const ExprVarMatrix*>(pVar)->Value._12); break;
  }
  return eTrue;
}
EndOp()

// GetZ(vec)
BeginOpF(GetZ,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  iExpressionVariable* pVar = mvOperands[0].GetVariable();
  switch (pVar->GetType())
  {
    case eExpressionVariableType_Float: mptrResult->SetFloat(static_cast<const ExprVarFloat*>(pVar)->Value);  break;
    case eExpressionVariableType_Vec2: mptrResult->SetFloat(0);  break;
    case eExpressionVariableType_Vec3: mptrResult->SetFloat(static_cast<const ExprVarVec3*>(pVar)->Value.z);  break;
    case eExpressionVariableType_Vec4: mptrResult->SetFloat(static_cast<const ExprVarVec4*>(pVar)->Value.z);  break;
    case eExpressionVariableType_Matrix:  mptrResult->SetFloat(static_cast<const ExprVarMatrix*>(pVar)->Value._13);break;
  }
  return eTrue;
}
EndOp()

// GetW(vec)
BeginOpF(GetW,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  iExpressionVariable* pVar = mvOperands[0].GetVariable();
  switch (pVar->GetType())
  {
    case eExpressionVariableType_Float: mptrResult->SetFloat(static_cast<const ExprVarFloat*>(pVar)->Value);  break;
    case eExpressionVariableType_Vec2:
    case eExpressionVariableType_Vec3:
      mptrResult->SetFloat(0);
      break;
    case eExpressionVariableType_Vec4: mptrResult->SetFloat(static_cast<const ExprVarVec4*>(pVar)->Value.w);  break;
    case eExpressionVariableType_Matrix:  mptrResult->SetFloat(static_cast<const ExprVarMatrix*>(pVar)->Value._14); break;
  }
  return eTrue;
}
EndOp()

// GetWidth(vec4)
BeginOpF(GetWidth,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  if (mvOperands[0].GetVariable()->GetType() != eExpressionVariableType_Vec4) {
    EXPRESSION_TRACE(_A("Operand 0 is not a vector4."));
    return eFalse;
  }
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  iExpressionVariable* pVar = mvOperands[0].GetVariable();
  niAssert(pVar->GetType() == eExpressionVariableType_Vec4);
  mptrResult->SetFloat(static_cast<const ExprVarVec4*>(pVar)->Value.z-
                       static_cast<const ExprVarVec4*>(pVar)->Value.x);
  return eTrue;
}
EndOp()

// GetHeight(vec4)
BeginOpF(GetHeight,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  if (mvOperands[0].GetVariable()->GetType() != eExpressionVariableType_Vec4) {
    EXPRESSION_TRACE(_A("Operand 0 is not a vector4."));
    return eFalse;
  }
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  iExpressionVariable* pVar = mvOperands[0].GetVariable();
  niAssert(pVar->GetType() == eExpressionVariableType_Vec4);
  mptrResult->SetFloat(static_cast<const ExprVarVec4*>(pVar)->Value.w-
                       static_cast<const ExprVarVec4*>(pVar)->Value.y);
  return eTrue;
}
EndOp()

// rotate function
BeginOpF(Rotate,2)
tBool SetupEvaluation(iExpressionContext*)
{
  if (mvOperands[1].GetVariable()->GetType() != eExpressionVariableType_Matrix)
  {
    EXPRESSION_TRACE(_A("Operand 1 is not a matrix."));
    return eFalse;
  }
  eExpressionVariableType type = mvOperands[0].GetVariable()->GetType();
  if (type != eExpressionVariableType_Vec2 &&
      type != eExpressionVariableType_Vec3 &&
      type != eExpressionVariableType_Matrix)
  {
    type = eExpressionVariableType_Vec3;
  }
  mptrResult = _CreateVariable(NULL,type);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  switch (mptrResult->GetType())
  {
    case eExpressionVariableType_Vec2: {
      VecTransformNormal(static_cast<ExprVarVec2*>(mptrResult.ptr())->Value,
                         static_cast<ExprVarVec2*>(mvOperands[0].GetVariable())->Value,
                         static_cast<ExprVarMatrix*>(mvOperands[1].GetVariable())->Value);
      break;
    }
    case eExpressionVariableType_Vec3: {
      VecTransformNormal(static_cast<ExprVarVec3*>(mptrResult.ptr())->Value,
                         mvOperands[0].GetVariable()->GetVec3(),
                         static_cast<ExprVarMatrix*>(mvOperands[1].GetVariable())->Value);
      break;
    }
    default: {
      EXPRESSION_TRACE(_A("Operand 1 is not a vec2 or vec3."));
      return eFalse;
    }
  }
  return eTrue;
}
EndOp()

// mod function (% modulo)
BeginOpF(Mod,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch2(DEST=ni::FMod(OP0,OP1));
  return eTrue;
}
EndOp()

// sin function
BeginOpF(Sin,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch1(DEST=ni::Sin(OP0));
  return eTrue;
}
EndOp()

// asin function
BeginOpF(ASin,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch1(DEST=ni::ASin(OP0));
  return eTrue;
}
EndOp()

// cos function
BeginOpF(Cos,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch1(DEST=ni::Cos(OP0));
  return eTrue;
}
EndOp()

// acos function
BeginOpF(ACos,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch1(DEST=ni::ACos(OP0));
  return eTrue;
}
EndOp()

// tan function
BeginOpF(Tan,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch1(DEST=ni::Tan(OP0));
  return eTrue;
}
EndOp()

// atan function
BeginOpF(ATan,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch1(DEST=ni::ATan(OP0));
  return eTrue;
}
EndOp()

// rad function
BeginOpF(Rad,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch1(DEST=ni::Rad(OP0));
  return eTrue;
}
EndOp()

// deg function
BeginOpF(Deg,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch1(DEST=ni::Deg(OP0));
  return eTrue;
}
EndOp()

// floor function
BeginOpF(Floor,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch1(DEST=ni::Floor(OP0));
  return eTrue;
}
EndOp()

// ceil function
BeginOpF(Ceil,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch1(DEST=ni::Ceil(OP0));
  return eTrue;
}
EndOp()

// abs function
BeginOpF(Abs,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch1(DEST=ni::Abs(OP0));
  return eTrue;
}
EndOp()

// sign function
BeginOpF(Sign,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch1(DEST=ni::Sign(OP0));
  return eTrue;
}
EndOp()

// Pow function
BeginOpF(Pow,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch2(DEST=ni::Pow(OP0,OP1));
  return eTrue;
}
EndOp()

// LShift function
BeginOpF(LShift,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch2(_FloatAssign(DEST,(int)OP0<<(int)OP1));
  return eTrue;
}
EndOp()

// RShift function
BeginOpF(RShift,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch2(_FloatAssign(DEST,(int)OP0>>(int)OP1));
  return eTrue;
}
EndOp()

// Mul function
BeginOpF(Mul,2)
tBool SetupEvaluation(iExpressionContext*)
{
  if (mvOperands[1].GetVariable()->GetType() != eExpressionVariableType_Matrix) {
    EXPRESSION_TRACE(_A("Second operand is not a matrix."));
    return eFalse;
  }
  mptrResult = _CreateVariable(NULL,mvOperands[0].GetVariable()->GetType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  switch (mptrResult->GetType())
  {
    case eExpressionVariableType_Float:
      {
        static_cast<ExprVarFloat*>(mptrResult.ptr())->Value =
            static_cast<ExprVarFloat*>(mvOperands[0].GetVariable())->Value *
            static_cast<ExprVarMatrix*>(mvOperands[1].GetVariable())->Value._11;
        break;
      }
    case eExpressionVariableType_Vec2:
      {
        VecTransformCoord(static_cast<ExprVarVec2*>(mptrResult.ptr())->Value,
                          static_cast<ExprVarVec2*>(mvOperands[0].GetVariable())->Value,
                          static_cast<ExprVarMatrix*>(mvOperands[1].GetVariable())->Value);
        break;
      }
    case eExpressionVariableType_Vec3:
      {
        VecTransformCoord(static_cast<ExprVarVec3*>(mptrResult.ptr())->Value,
                          static_cast<ExprVarVec3*>(mvOperands[0].GetVariable())->Value,
                          static_cast<ExprVarMatrix*>(mvOperands[1].GetVariable())->Value);
        break;
      }
    case eExpressionVariableType_Vec4:
      {
        VecTransform(static_cast<ExprVarVec4*>(mptrResult.ptr())->Value,
                     static_cast<ExprVarVec4*>(mvOperands[0].GetVariable())->Value,
                     static_cast<ExprVarMatrix*>(mvOperands[1].GetVariable())->Value);
        break;
      }
    case eExpressionVariableType_Matrix:
      {
        MatrixMultiply(static_cast<ExprVarMatrix*>(mptrResult.ptr())->Value,
                       static_cast<ExprVarMatrix*>(mvOperands[0].GetVariable())->Value,
                       static_cast<ExprVarMatrix*>(mvOperands[1].GetVariable())->Value);
        break;
      }
  }
  return eTrue;
}
EndOp()


// lerp function
BeginOpF(Lerp,3)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch3(DEST=ni::Lerp(OP0,OP1,OP2));
  return eTrue;
}
EndOp()

// clamp function
BeginOpF(Clamp,3)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch3(DEST=ni::Clamp(OP0,OP1,OP2));
  return eTrue;
}
EndOp()

// Sat function
BeginOpF(Sat,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch1(DEST=ni::ClampZeroOne(OP0));
  return eTrue;
}
EndOp()

// sqrt function
BeginOpF(Sqrt,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch1(DEST=ni::Sqrt(OP0));
  return eTrue;
}
EndOp()

// dot2 function
BeginOpF(Dot2,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  static_cast<ExprVarFloat*>(mptrResult.ptr())->Value =
      VecDot(mvOperands[0].GetVariable()->GetVec2(), mvOperands[1].GetVariable()->GetVec2());
  return eTrue;
}
EndOp()

// dot3 function
BeginOpF(Dot3,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  static_cast<ExprVarFloat*>(mptrResult.ptr())->Value =
      VecDot(mvOperands[0].GetVariable()->GetVec3(), mvOperands[1].GetVariable()->GetVec3());
  return eTrue;
}
EndOp()

// dot4 function
BeginOpF(Dot4,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  static_cast<ExprVarFloat*>(mptrResult.ptr())->Value =
      VecDot(mvOperands[0].GetVariable()->GetVec4(), mvOperands[1].GetVariable()->GetVec4());
  return eTrue;
}
EndOp()

// Len2 function
BeginOpF(Len2,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  static_cast<ExprVarFloat*>(mptrResult.ptr())->Value = VecLength(mvOperands[0].GetVariable()->GetVec2());
  return eTrue;
}
EndOp()

// Len3 function
BeginOpF(Len3,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  static_cast<ExprVarFloat*>(mptrResult.ptr())->Value = VecLength(mvOperands[0].GetVariable()->GetVec3());
  return eTrue;
}
EndOp()

// Len4 function
BeginOpF(Len4,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  static_cast<ExprVarFloat*>(mptrResult.ptr())->Value = VecLength(mvOperands[0].GetVariable()->GetVec4());
  return eTrue;
}
EndOp()

// cross3 function
BeginOpF(Cross3,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec3);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  VecCross(static_cast<ExprVarVec3*>(mptrResult.ptr())->Value,
           mvOperands[0].GetVariable()->GetVec3(), mvOperands[1].GetVariable()->GetVec3());
  return eTrue;
}
EndOp()

// cross4 function
BeginOpF(Cross4,3)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  VecCross(static_cast<ExprVarVec4*>(mptrResult.ptr())->Value,
           mvOperands[0].GetVariable()->GetVec4(),
           mvOperands[1].GetVariable()->GetVec4(),
           mvOperands[2].GetVariable()->GetVec4());
  return eTrue;
}
EndOp()

// normalize2(vec2) function
BeginOpF(Normalize2,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec2);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  VecNormalize(static_cast<ExprVarVec2*>(mptrResult.ptr())->Value,
               mvOperands[0].GetVariable()->GetVec2());
  return eTrue;
}
EndOp()

// normalize3(vec3) function
BeginOpF(Normalize3,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec3);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  VecNormalize(static_cast<ExprVarVec3*>(mptrResult.ptr())->Value,
               mvOperands[0].GetVariable()->GetVec3());
  return eTrue;
}
EndOp()

// normalize4(vec4) function
BeginOpF(Normalize4,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  VecNormalize(static_cast<ExprVarVec4*>(mptrResult.ptr())->Value,
               mvOperands[0].GetVariable()->GetVec4());
  return eTrue;
}
EndOp()

// min function
BeginOpF(Min,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch2(DEST=ni::Min(OP0,OP1));
  return eTrue;
}
EndOp()

// max function
BeginOpF(Max,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch2(DEST=ni::Max(OP0,OP1));
  return eTrue;
}
EndOp()

// Rand function
BeginOpF(Rand,0)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch(DEST=RandFloat());
  return eTrue;
}
EndOp()

// Rand2 function
BeginOpF(Rand2,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch2(_FloatAssign(DEST,RandFloatRange(OP0,OP1)));
  return eTrue;
}
EndOp()

// Bit function
BeginOpF(Bit,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  DoSwitch1(_FloatAssign(DEST,niBit((int)OP0)));
  return eTrue;
}
EndOp()

// MatrixRotationX function
BeginOpF(MatrixRotationX,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Matrix);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  MatrixRotationX(static_cast<ExprVarMatrix*>(mptrResult.ptr())->Value,
                  (tVectorFloat)mvOperands[0].GetVariable()->GetFloat());
  return eTrue;
}
EndOp()

// MatrixRotationY function
BeginOpF(MatrixRotationY,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Matrix);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  MatrixRotationY(static_cast<ExprVarMatrix*>(mptrResult.ptr())->Value,
                  (tVectorFloat)mvOperands[0].GetVariable()->GetFloat());
  return eTrue;
}
EndOp()

// MatrixRotationZ function
BeginOpF(MatrixRotationZ,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Matrix);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  MatrixRotationZ(static_cast<ExprVarMatrix*>(mptrResult.ptr())->Value,
                  (tVectorFloat)mvOperands[0].GetVariable()->GetFloat());
  return eTrue;
}
EndOp()

// MatrixRotationAxis function
BeginOpF(MatrixRotationAxis,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Matrix);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  MatrixRotationAxis(static_cast<ExprVarMatrix*>(mptrResult.ptr())->Value,
                     mvOperands[0].GetVariable()->GetVec3(),
                     (tVectorFloat)mvOperands[1].GetVariable()->GetFloat());
  return eTrue;
}
EndOp()

// MatrixRotationQuat function
BeginOpF(MatrixRotationQuat,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Matrix);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  MatrixRotationQuat(static_cast<ExprVarMatrix*>(mptrResult.ptr())->Value,
                     (const sQuat<tVectorFloat>&)mvOperands[0].GetVariable()->GetVec4());
  return eTrue;
}
EndOp()

// MatrixRotationYPR function
BeginOpF(MatrixRotationYPR,3)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Matrix);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  MatrixRotationYawPitchRoll(static_cast<ExprVarMatrix*>(mptrResult.ptr())->Value,
                             (tVectorFloat)mvOperands[0].GetVariable()->GetFloat(),
                             (tVectorFloat)mvOperands[1].GetVariable()->GetFloat(),
                             (tVectorFloat)mvOperands[2].GetVariable()->GetFloat());
  return eTrue;
}
EndOp()

// MatrixTranslation function
BeginOpF(MatrixTranslation,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Matrix);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  MatrixTranslation(static_cast<ExprVarMatrix*>(mptrResult.ptr())->Value,
                    mvOperands[0].GetVariable()->GetVec3());
  return eTrue;
}
EndOp()

// MatrixScale function
BeginOpF(MatrixScale,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Matrix);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  MatrixScaling(static_cast<ExprVarMatrix*>(mptrResult.ptr())->Value,
                mvOperands[0].GetVariable()->GetVec3());
  return eTrue;
}
EndOp()

// MatrixIdentity function
BeginOpF(MatrixIdentity,0)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Matrix);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  MatrixIdentity(static_cast<ExprVarMatrix*>(mptrResult.ptr())->Value);
  return eTrue;
}
EndOp()

// MatrixInverse function
BeginOpF(MatrixInverse,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Matrix);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  MatrixInverse(static_cast<ExprVarMatrix*>(mptrResult.ptr())->Value,
                mvOperands[0].GetVariable()->GetMatrix());
  return eTrue;
}
EndOp()

// QuatIdentity()
BeginOpF(QuatIdentity,0)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4,eExpressionVariableFlags_Quat);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  QuatIdentity((sQuatf&)static_cast<ExprVarVec4*>(mptrResult.ptr())->Value);
  return eTrue;
}
EndOp()

// QuatBaryCentric(Q1,Q2,Q3,f,g)
BeginOpF(QuatBaryCentric,5)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4,eExpressionVariableFlags_Quat);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  QuatBaryCentric((sQuat<tVectorFloat>&)static_cast<ExprVarVec4*>(mptrResult.ptr())->Value,
                  (const sQuatf&)mvOperands[0].GetVariable()->GetVec4(),
                  (const sQuatf&)mvOperands[1].GetVariable()->GetVec4(),
                  (const sQuatf&)mvOperands[2].GetVariable()->GetVec4(),
                  (tVectorFloat)mvOperands[3].GetVariable()->GetFloat(),
                  (tVectorFloat)mvOperands[4].GetVariable()->GetFloat());
  return eTrue;
}
EndOp()

// QuatConjugate(Q)
BeginOpF(QuatConjugate,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4,eExpressionVariableFlags_Quat);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  QuatConjugate((sQuatf&)static_cast<ExprVarVec4*>(mptrResult.ptr())->Value,
                (const sQuatf&)mvOperands[0].GetVariable()->GetVec4());
  return eTrue;
}
EndOp()

// QuatExp(Q)
BeginOpF(QuatExp,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4,eExpressionVariableFlags_Quat);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  QuatExp((sQuatf&)static_cast<ExprVarVec4*>(mptrResult.ptr())->Value,
          (const sQuatf&)mvOperands[0].GetVariable()->GetVec4());
  return eTrue;
}
EndOp()

// QuatInverse(Q)
BeginOpF(QuatInverse,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4,eExpressionVariableFlags_Quat);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  QuatInverse((sQuatf&)static_cast<ExprVarVec4*>(mptrResult.ptr())->Value,
              (const sQuatf&)mvOperands[0].GetVariable()->GetVec4());
  return eTrue;
}
EndOp()

// QuatLn(Q)
BeginOpF(QuatLn,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4,eExpressionVariableFlags_Quat);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  QuatLn((sQuatf&)static_cast<ExprVarVec4*>(mptrResult.ptr())->Value,
         (const sQuatf&)mvOperands[0].GetVariable()->GetVec4());
  return eTrue;
}
EndOp()

// QuatMul(Q1,Q2)
BeginOpF(QuatMul,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4,eExpressionVariableFlags_Quat);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  QuatMultiply((sQuatf&)static_cast<ExprVarVec4*>(mptrResult.ptr())->Value,
               (const sQuatf&)mvOperands[0].GetVariable()->GetVec4(),
               (const sQuatf&)mvOperands[1].GetVariable()->GetVec4());
  return eTrue;
}
EndOp()

// QuatRotationAxis(V,angle)
BeginOpF(QuatRotationAxis,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4,eExpressionVariableFlags_Quat);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  QuatRotationAxis((sQuat<tVectorFloat>&)static_cast<ExprVarVec4*>(mptrResult.ptr())->Value,
                   mvOperands[0].GetVariable()->GetVec3(),
                   (tVectorFloat)mvOperands[1].GetVariable()->GetFloat());
  return eTrue;
}
EndOp()

// QuatRotationX(angle)
BeginOpF(QuatRotationX,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4,eExpressionVariableFlags_Quat);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  QuatRotationX((sQuatf&)static_cast<ExprVarVec4*>(mptrResult.ptr())->Value,
                (tVectorFloat)mvOperands[0].GetVariable()->GetFloat());
  return eTrue;
}
EndOp()

// QuatRotationY(angle)
BeginOpF(QuatRotationY,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4,eExpressionVariableFlags_Quat);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  QuatRotationY((sQuatf&)static_cast<ExprVarVec4*>(mptrResult.ptr())->Value,
                (tVectorFloat)mvOperands[0].GetVariable()->GetFloat());
  return eTrue;
}
EndOp()

// QuatRotationZ(angle)
BeginOpF(QuatRotationZ,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4,eExpressionVariableFlags_Quat);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  QuatRotationZ((sQuatf&)static_cast<ExprVarVec4*>(mptrResult.ptr())->Value,
                (tVectorFloat)mvOperands[0].GetVariable()->GetFloat());
  return eTrue;
}
EndOp()

// QuatRotationXYZ(angle)
BeginOpF(QuatRotationXYZ,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4,eExpressionVariableFlags_Quat);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  sVec3f angle = mvOperands[0].GetVariable()->GetVec3();
  QuatRotationXYZ((sQuatf&)static_cast<ExprVarVec4*>(mptrResult.ptr())->Value,
                  angle.x,angle.y,angle.z);
  return eTrue;
}
EndOp()

// QuatRotationMatrix(M)
BeginOpF(QuatRotationMatrix,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4,eExpressionVariableFlags_Quat);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  QuatRotationMatrix((sQuatf&)static_cast<ExprVarVec4*>(mptrResult.ptr())->Value,
                     mvOperands[0].GetVariable()->GetMatrix());
  return eTrue;
}
EndOp()

// QuatRotationYPR(yaw,pitch,roll)
BeginOpF(QuatRotationYPR,3)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4,eExpressionVariableFlags_Quat);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  QuatRotationYawPitchRoll((sQuatf&)static_cast<ExprVarVec4*>(mptrResult.ptr())->Value,
                           (tVectorFloat)mvOperands[0].GetVariable()->GetFloat(),
                           (tVectorFloat)mvOperands[1].GetVariable()->GetFloat(),
                           (tVectorFloat)mvOperands[2].GetVariable()->GetFloat());
  return eTrue;
}
EndOp()

// QuatRotationVector(vFrom,vTo)
BeginOpF(QuatRotationVector,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4,eExpressionVariableFlags_Quat);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  QuatRotationVector((sQuatf&)static_cast<ExprVarVec4*>(mptrResult.ptr())->Value,
                     mvOperands[0].GetVariable()->GetVec3(),
                     mvOperands[1].GetVariable()->GetVec3());
  return eTrue;
}
EndOp()

// QuatSlerp(Q1,Q2,t)
BeginOpF(QuatSlerp,3)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4,eExpressionVariableFlags_Quat);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  QuatSlerp((sQuat<tVectorFloat>&)static_cast<ExprVarVec4*>(mptrResult.ptr())->Value,
            (const sQuatf&)mvOperands[0].GetVariable()->GetVec4(),
            (const sQuatf&)mvOperands[1].GetVariable()->GetVec4(),
            (tVectorFloat)mvOperands[2].GetVariable()->GetFloat(),
            eQuatSlerp_Short);
  return eTrue;
}
EndOp()

// QuatSquad(Q1,Q2,Q3,Q4,t)
BeginOpF(QuatSquad,5)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4,eExpressionVariableFlags_Quat);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  QuatSquad((sQuat<tVectorFloat>&)static_cast<ExprVarVec4*>(mptrResult.ptr())->Value,
            (const sQuatf&)mvOperands[0].GetVariable()->GetVec4(),
            (const sQuatf&)mvOperands[1].GetVariable()->GetVec4(),
            (const sQuatf&)mvOperands[2].GetVariable()->GetVec4(),
            (const sQuatf&)mvOperands[3].GetVariable()->GetVec4(),
            (tVectorFloat)mvOperands[4].GetVariable()->GetFloat());
  return eTrue;
}
EndOp()

// QuatToAxisAngle(Q)
BeginOpF(QuatToAxisAngle,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec4,eExpressionVariableFlags_Quat);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  sVec3f axis;
  tF32 angle;
  QuatToAxisAngle((const sQuatf&)mvOperands[0].GetVariable()->GetVec4(),axis,angle);
  sVec4f& q = static_cast<ExprVarVec4*>(mptrResult.ptr())->Value;
  q.x = axis.x;
  q.y = axis.y;
  q.z = axis.z;
  q.w = angle;
  return eTrue;
}
EndOp()

// QuatToEuler(Q)
BeginOpF(QuatToEuler,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Vec3,eExpressionVariableFlags_Quat);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  QuatToEuler((const sQuatf&)mvOperands[0].GetVariable()->GetVec4(),
              static_cast<ExprVarVec3*>(mptrResult.ptr())->Value);
  return eTrue;
}
EndOp()

// Inc(BaseValue, Inc per Second) function
BeginOpWF(Inc,2)
void SetupOperands()
{
  if (mbConstantOperands) return;
  mfBase = mvOperands[0].GetVariable()->GetFloat();
  mfInc = mvOperands[1].GetVariable()->GetFloat();
}
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  SetupOperands();
  mbConstantOperands = AreOperandsConstant();
  mfVal = mfBase;
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  SetupOperands();
  mfVal += mfInc*OP_GET_FRAME_TIME;
  DoSwitch(_FloatAssign(DEST,mfVal));
  return eTrue;
}
EndOp()

// IncWrap(BaseValue, Inc per Second, Max Value - attained; > test used)
BeginOpWF(IncWrap,3)
void SetupOperands()
{
  if (mbConstantOperands) return;
  mfBase = mvOperands[0].GetVariable()->GetFloat();
  mfInc = mvOperands[1].GetVariable()->GetFloat();
  mfMax = mvOperands[2].GetVariable()->GetFloat();
}
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  SetupOperands();
  mbConstantOperands = AreOperandsConstant();
  mfVal = mfBase;
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  SetupOperands();
  mfVal += mfInc*OP_GET_FRAME_TIME;
  if (mfVal > mfMax)
  {
    while (1)
    {
      mfVal -= mfMax;
      if (mfVal < mfBase)
      {
        mfVal += mfBase;
        break;
      }
    }
  }

  DoSwitch(_FloatAssign(DEST,mfVal));
  return eTrue;
}
EndOp()

// Dec(BaseValue, Dec per Second)
BeginOpWF(Dec,2)
void SetupOperands()
{
  if (mbConstantOperands) return;
  mfBase = mvOperands[0].GetVariable()->GetFloat();
  mfInc = mvOperands[1].GetVariable()->GetFloat();
}
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  SetupOperands();
  mbConstantOperands = AreOperandsConstant();
  mfVal = mfBase;
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  SetupOperands();
  mfVal -= mfInc*OP_GET_FRAME_TIME;
  DoSwitch(_FloatAssign(DEST,mfVal));
  return eTrue;
}
EndOp()

// DecWrap(BaseValue, Dec per Second, Min Value - attained; < test used)
BeginOpWF(DecWrap,3)
void SetupOperands()
{
  if (mbConstantOperands) return;
  mfBase = mvOperands[0].GetVariable()->GetFloat();
  mfInc = mvOperands[1].GetVariable()->GetFloat();
  mfMax = mvOperands[2].GetVariable()->GetFloat();
}
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  SetupOperands();
  mbConstantOperands = AreOperandsConstant();
  mfVal = mfBase;
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  SetupOperands();

  mfVal -= mfInc*OP_GET_FRAME_TIME;
  if (mfVal < mfMax)
  {
    while (1)
    {
      mfVal += mfMax;
      if (mfVal > mfBase)
      {
        mfVal -= mfBase;
        break;
      }
    }
  }

  DoSwitch(_FloatAssign(DEST,mfVal));
  return eTrue;
}
EndOp()

// Step(BaseValue, Increment, Time for increment)
BeginOpWF(Step,3)
void SetupOperands()
{
  if (mbConstantOperands) return;
  mfBase = mvOperands[0].GetVariable()->GetFloat();
  mfInc = mvOperands[1].GetVariable()->GetFloat();
  mfTimeForIncrement = mvOperands[2].GetVariable()->GetFloat();
}
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  SetupOperands();
  mbConstantOperands = AreOperandsConstant();
  mfVal = mfBase;
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  SetupOperands();

  mfTime += OP_GET_FRAME_TIME;
  if (mfTime >= mfTimeForIncrement)
  {
    mfVal = (mfVal-mfBase)+mfInc+mfBase;
    mfTime = 0.0f;
  }

  DoSwitch(_FloatAssign(DEST,mfVal));
  return eTrue;
}
EndOp()

// StepWrap(BaseValue, Increment, Time for increment,
//           Max Value - > test used; if 0 value never wrap)
BeginOpWF(StepWrap,4)
void SetupOperands()
{
  if (mbConstantOperands) return;
  mfBase = mvOperands[0].GetVariable()->GetFloat();
  mfInc = mvOperands[1].GetVariable()->GetFloat();
  mfTimeForIncrement = mvOperands[2].GetVariable()->GetFloat();
  mfMax = mvOperands[3].GetVariable()->GetFloat();
}
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  SetupOperands();
  mbConstantOperands = AreOperandsConstant();
  mfVal = mfBase;
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  SetupOperands();

  mfTime += OP_GET_FRAME_TIME;
  if (mfTime >= mfTimeForIncrement)
  {
    mfVal = (mfVal-mfBase)+mfInc+mfBase;
    mfTime = 0.0f;
    if (mnMax && mfVal > mfMax)
    {
      mfVal = mfBase;
    }
  }

  DoSwitch(_FloatAssign(DEST,mfVal));
  return eTrue;
}
EndOp()

// StepBack(BaseValue, Decrement, Time for decrement)
BeginOpWF(StepBack,3)
void SetupOperands()
{
  if (mbConstantOperands) return;
  mfBase = mvOperands[0].GetVariable()->GetFloat();
  mfInc = mvOperands[1].GetVariable()->GetFloat();
  mfTimeForIncrement = mvOperands[2].GetVariable()->GetFloat();
}
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  SetupOperands();
  mbConstantOperands = AreOperandsConstant();
  mfVal = mfBase;
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  SetupOperands();

  mfTime += OP_GET_FRAME_TIME;
  if (mfTime >= mfTimeForIncrement)
  {
    mfVal = (mfVal-mfBase)-mfInc+mfBase;
    mfTime = 0.0f;
  }

  DoSwitch(_FloatAssign(DEST,mfVal));
  return eTrue;
}
EndOp()

// StepBackWrap(BaseValue, Decrement, Time for decrement,
//              Min Value - < test used; if 0 value never wrap)
BeginOpWF(StepBackWrap,4)
void SetupOperands()
{
  if (mbConstantOperands) return;
  mfBase = mvOperands[0].GetVariable()->GetFloat();
  mfInc = mvOperands[1].GetVariable()->GetFloat();
  mfTimeForIncrement = mvOperands[2].GetVariable()->GetFloat();
  mfMax = mvOperands[3].GetVariable()->GetFloat();
}
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  SetupOperands();
  mbConstantOperands = AreOperandsConstant();
  mfVal = mfBase;
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  SetupOperands();

  mfTime += OP_GET_FRAME_TIME;
  if (mfTime >= mfTimeForIncrement)
  {
    mfVal = (mfVal-mfBase)-mfInc+mfBase;
    mfTime = 0.0f;
    if (mfVal < mfMax)
    {
      mfVal = mfBase;
    }
  }

  DoSwitch(_FloatAssign(DEST,mfVal));
  return eTrue;
}
EndOp()

// SinWave(Base,Amplitude,Phase,Frequence)
BeginOpWF(SinWave,4)
void SetupOperands()
{
  if (mbConstantOperands) return;
  mfBase = mvOperands[0].GetVariable()->GetFloat();
  mfAmplitude = mvOperands[1].GetVariable()->GetFloat();
  mfPhase = mvOperands[2].GetVariable()->GetFloat();
  mfFreq = mvOperands[3].GetVariable()->GetFloat();
}

tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  SetupOperands();
  mbConstantOperands = AreOperandsConstant();
  mfVal = mfBase;
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  SetupOperands();
  DoWave(fY = ni::Sin(fX*ni2Pi));
  DoSwitch(_FloatAssign(DEST,mfVal));
  return eTrue;
}
EndOp()

// CosWave(Base,Amplitude,Phase,Frequence)
BeginOpWF(CosWave,4)
void SetupOperands()
{
  if (mbConstantOperands) return;
  mfBase = mvOperands[0].GetVariable()->GetFloat();
  mfAmplitude = mvOperands[1].GetVariable()->GetFloat();
  mfPhase = mvOperands[2].GetVariable()->GetFloat();
  mfFreq = mvOperands[3].GetVariable()->GetFloat();
}
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());

  SetupOperands();
  mbConstantOperands = AreOperandsConstant();

  mfVal = mfBase;
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  SetupOperands();
  DoWave(fY = ni::Cos(fX*ni2Pi));
  DoSwitch(_FloatAssign(DEST,mfVal));
  return eTrue;
}
EndOp()

// TriangleWave(Base,Amplitude,Phase,Frequence)
BeginOpWF(TriangleWave,4)
void SetupOperands()
{
  if (mbConstantOperands) return;
  mfBase = mvOperands[0].GetVariable()->GetFloat();
  mfAmplitude = mvOperands[1].GetVariable()->GetFloat();
  mfPhase = mvOperands[2].GetVariable()->GetFloat();
  mfFreq = mvOperands[3].GetVariable()->GetFloat();
}
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  SetupOperands();
  mbConstantOperands = AreOperandsConstant();
  mfVal = mfBase;
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  SetupOperands();
  DoWave(fY = (fX < .5)?(4.0*fX-1.0):(-4.0*fX+3.0));
  DoSwitch(_FloatAssign(DEST,mfVal));
  return eTrue;
}
EndOp()

// SquareWave(Base,Amplitude,Phase,Frequence)
BeginOpWF(SquareWave,4)
void SetupOperands()
{
  if (mbConstantOperands) return;
  mfBase = mvOperands[0].GetVariable()->GetFloat();
  mfAmplitude = mvOperands[1].GetVariable()->GetFloat();
  mfPhase = mvOperands[2].GetVariable()->GetFloat();
  mfFreq = mvOperands[3].GetVariable()->GetFloat();
}
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  SetupOperands();
  mbConstantOperands = AreOperandsConstant();
  mfVal = mfBase;
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  SetupOperands();
  DoWave(fY = (fX < .5)?(1.0):(-1.0));
  DoSwitch(_FloatAssign(DEST,mfVal));
  return eTrue;
}
EndOp()

// SawWave(Base,Amplitude,Phase,Frequence)
BeginOpWF(SawWave,4)
void SetupOperands()
{
  if (mbConstantOperands) return;
  mfBase = mvOperands[0].GetVariable()->GetFloat();
  mfAmplitude = mvOperands[1].GetVariable()->GetFloat();
  mfPhase = mvOperands[2].GetVariable()->GetFloat();
  mfFreq = mvOperands[3].GetVariable()->GetFloat();
}
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  SetupOperands();
  mbConstantOperands = AreOperandsConstant();
  mfVal = mfBase;
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  SetupOperands();
  DoWave(fY = fX);
  DoSwitch(_FloatAssign(DEST,mfVal));
  return eTrue;
}
EndOp()

// InvSawWave(Base,Amplitude,Phase,Frequence)
BeginOpWF(InvSawWave,4)
void SetupOperands()
{
  if (mbConstantOperands) return;
  mfBase = mvOperands[0].GetVariable()->GetFloat();
  mfAmplitude = mvOperands[1].GetVariable()->GetFloat();
  mfPhase = mvOperands[2].GetVariable()->GetFloat();
  mfFreq = mvOperands[3].GetVariable()->GetFloat();
}
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,GetCommonOperandsType());
  SetupOperands();
  mbConstantOperands = AreOperandsConstant();
  mfVal = mfBase;
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  SetupOperands();
  DoWave(fY = (1.0)-fX);
  DoSwitch(_FloatAssign(DEST,mfVal));
  return eTrue;
}
EndOp()

//! Format a string.
BeginOpF(Format,eInvalidHandle)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_String);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  switch (mvOperands.size()) {
    case 0:
      // empty string
      break;
    case 1:
      // the string itself
      mptrResult->SetString(mvOperands[0].GetVariable()->GetString());
      break;
    default: {
      cString o;
      const tU32 numFormatParams = (tU32)mvOperands.size()-1;
      astl::vector<Var> args;
      args.resize(numFormatParams);
      astl::vector<const Var*> argsPtr;
      argsPtr.resize(numFormatParams);
      niLoop(i,numFormatParams) {
        args[i] = _ToVar(mvOperands[i+1].GetVariable());
        argsPtr[i] = &args[i];
      }
      StringCatFormatEx(
          o,NULL,mvOperands[0].GetVariable()->GetString().Chars(),
          argsPtr.data(),(tU32)argsPtr.size());
      mptrResult->SetString(o);
    }
  }
  return eTrue;
}
EndOp()

//! FormatCurrentTime(TIMEFORMAT: string) the current time.
BeginOpVF(FormatCurrentTime,eInvalidHandle)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_String);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  cString currentTime;
  switch (mvOperands.size()) {
  case 0:
    currentTime = ni::GetLang()->GetCurrentTime()->Format(NULL);
    break;
  default:
    currentTime = ni::GetLang()->GetCurrentTime()->Format(
      mvOperands[0].GetVariable()->GetString().Chars());
    break;
  }
  // niDebugFmt((".... Eval CurrentTime: %d, %s", mbConstant, currentTime));
  mptrResult->SetString(currentTime);
  return eTrue;
}
EndOp()

//! FormatTime(TIMEFORMAT: string) the current time.
BeginOpVF(FormatTimeSeconds,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_String);
  return eTrue;
}

tBool DoEvaluate(iExpressionContext*)
{
  Ptr<iTime> t = ni::GetLang()->GetCurrentTime()->Clone();
  t->SetUnixTimeSecs(mvOperands[0].GetVariable()->GetFloat());
  mptrResult->SetString(t->Format(mvOperands[1].GetVariable()->GetString().Chars()));
  return eTrue;
}
EndOp()

//! StrLen(aText) -> Int
BeginOpF(StrLen,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  mptrResult->SetFloat(
      mvOperands[0].GetVariable()->GetString().size());
  return eTrue;
}
EndOp()

//! StrSlice(aText, aStartIndex, aEndIndex: optional) -> String
BeginOpF(StrSlice,eInvalidHandle)
tBool SetupEvaluation(iExpressionContext*)
{
  if (mvOperands.size() < 2) {
    EXPRESSION_TRACE("StrSlice(aText, aStartIndex, aEndIndex: optional) -> String: Not enough parameters.");
    return eFalse;
  }
  else if (mvOperands.size() > 3) {
    EXPRESSION_TRACE("StrSlice(aText, aStartIndex, aEndIndex: optional) -> String: Too many parameters.");
    return eFalse;
  }
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_String);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  cString text = mvOperands[0].GetVariable()->GetString();
  const tInt slen = text.size();
  tInt sidx = (tInt)mvOperands[1].GetVariable()->GetFloat();
  tInt eidx = (mvOperands.size() >= 3) ? (tInt)mvOperands[2].GetVariable()->GetFloat() : slen;
  mptrResult->SetString(text.slice(sidx,eidx));
  return eTrue;
}
EndOp()

//! StrMid(aText, aStartIndex, aCount) -> String
BeginOpF(StrMid,3)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_String);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  cString text = mvOperands[0].GetVariable()->GetString();
  tU32 sidx = (tU32)mvOperands[1].GetVariable()->GetFloat();
  tU32 count = (tU32)mvOperands[2].GetVariable()->GetFloat();
  mptrResult->SetString(text.Mid(sidx,count));
  return eTrue;
}
EndOp()

//! StrLeft(aText, aCount) -> String
BeginOpF(StrLeft,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_String);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  cString text = mvOperands[0].GetVariable()->GetString();
  tU32 count = (tU32)mvOperands[1].GetVariable()->GetFloat();
  mptrResult->SetString(text.Left(count));
  return eTrue;
}
EndOp()

//! StrRight(aText, aCount) -> String
BeginOpF(StrRight,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_String);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  cString text = mvOperands[0].GetVariable()->GetString();
  tU32 count = (tU32)mvOperands[1].GetVariable()->GetFloat();
  mptrResult->SetString(text.Right(count));
  return eTrue;
}
EndOp()

//! StrBefore(aText, aSub) -> String
BeginOpF(StrBefore,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_String);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  cString text = mvOperands[0].GetVariable()->GetString();
  cString sub = mvOperands[1].GetVariable()->GetString();
  mptrResult->SetString(text.Before(sub));
  return eTrue;
}
EndOp()

//! StrRBefore(aText, aSub) -> String
BeginOpF(StrRBefore,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_String);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  cString text = mvOperands[0].GetVariable()->GetString();
  cString sub = mvOperands[1].GetVariable()->GetString();
  mptrResult->SetString(text.RBefore(sub));
  return eTrue;
}
EndOp()

//! StrAfter(aText, aSub) -> String
BeginOpF(StrAfter,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_String);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  cString text = mvOperands[0].GetVariable()->GetString();
  cString sub = mvOperands[1].GetVariable()->GetString();
  mptrResult->SetString(text.After(sub));
  return eTrue;
}
EndOp()

//! StrRAfter(aText, aSub) -> String
BeginOpF(StrRAfter,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_String);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  cString text = mvOperands[0].GetVariable()->GetString();
  cString sub = mvOperands[1].GetVariable()->GetString();
  mptrResult->SetString(text.RAfter(sub));
  return eTrue;
}
EndOp()

//! StrToLower(aText) -> String
BeginOpF(StrToLower,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_String);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  cString text = mvOperands[0].GetVariable()->GetString();
  text.ToLower();
  mptrResult->SetString(text);
  return eTrue;
}
EndOp()


//! StrToUpper(aText) -> String
BeginOpF(StrToUpper,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_String);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  cString text = mvOperands[0].GetVariable()->GetString();
  text.ToUpper();
  mptrResult->SetString(text);
  return eTrue;
}
EndOp()

//! StrTrim(aText) -> String
BeginOpF(StrTrim,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_String);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  cString text = mvOperands[0].GetVariable()->GetString();
  text.Trim();
  mptrResult->SetString(text);
  return eTrue;
}
EndOp()

//! StrNormalize(aText) -> String
BeginOpF(StrNormalize,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_String);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  cString text = mvOperands[0].GetVariable()->GetString();
  text.Normalize();
  mptrResult->SetString(text);
  return eTrue;
}
EndOp()

//! StrFind(aText, aSub) -> Int
BeginOpF(StrFind,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  cString text = mvOperands[0].GetVariable()->GetString();
  cString tofind = mvOperands[1].GetVariable()->GetString();
  mptrResult->SetFloat(text.find(tofind));
  return eTrue;
}
EndOp()

//! StrRFind(aText, aSub) -> Int
BeginOpF(StrRFind,eInvalidHandle)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_Float);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  cString text = mvOperands[0].GetVariable()->GetString();
  cString tofind = mvOperands[1].GetVariable()->GetString();
  mptrResult->SetFloat(text.rfind(tofind));
  return eTrue;
}
EndOp()

//! DigestHex(aText, aType) -> String
BeginOpF(DigestHex,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_String);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  cString text = mvOperands[0].GetVariable()->GetString();
  cString type = mvOperands[1].GetVariable()->GetString();
  mptrResult->SetString(
      ni::GetCrypto()->Digest(text.Chars(), type.Chars(), eRawToStringEncoding_Hex));
  return eTrue;
}
EndOp()

//! KDFGenSaltBlowfish(aRounds) -> String
BeginOpF(KDFGenSaltBlowfish,1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_String);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  tInt rounds = (tInt)mvOperands[0].GetVariable()->GetFloat();
  mptrResult->SetString(
      ni::GetCrypto()->KDFGenSaltBlowfish(NULL,rounds));
  return eTrue;
}
EndOp()

//! KDFCrypt(aKey, aSalt) -> String
BeginOpF(KDFCrypt,2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_String);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  cString key = mvOperands[0].GetVariable()->GetString();
  cString salt = mvOperands[1].GetVariable()->GetString();
  mptrResult->SetString(
      ni::GetCrypto()->KDFCrypt(key.Chars(), salt.Chars()));
  return eTrue;
}
EndOp()

// Get function
BeginOpLF(Get,1)
tBool SetupEvaluation(iExpressionContext* apContext)
{
  if (mvOperands[0].GetVariable()->GetType() != eExpressionVariableType_String) {
    EXPRESSION_TRACE("Get(): first operand is not a string.");
    return eFalse;
  }
  tHStringPtr key = _H(mvOperands[0].GetVariable()->GetString());
  mptrResult = apContext->FindVariable(key);
  if (!mptrResult.IsOK()) {
    EXPRESSION_TRACE(niFmt("Get(): can't find variable '%s'.", key));
    return eFalse;
  }
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  return eTrue;
}
EndOp()

// Global function
BeginOpLF(Global,eInvalidHandle)
tBool SetupEvaluation(iExpressionContext* apContext)
{
  const tSize numOperands = mvOperands.size();
  if (numOperands < 3) {
    EXPRESSION_TRACE(_A("Global(): must have at least three operands."));
    return eFalse;
  }
  if ((numOperands % 2) != 1) {
    EXPRESSION_TRACE("Global(): must have a last operand which is the returned valued.");
    return eFalse;
  }

  for (tU32 i = 0; i < numOperands - 1; i += 2) {
    Op::sOperand& opKey = mvOperands[i];
    if (!opKey.Eval(apContext)) {
      EXPRESSION_TRACE(niFmt("Global(): operation, can't evaluate key operand '%s' (%d).",
                             opKey.GetName(), i));
      return eFalse;
    }
    if (opKey.GetVariable()->GetType() != eExpressionVariableType_String) {
      EXPRESSION_TRACE(niFmt("Global(): operation, key operand '%s' (%d) is not a string.",
                             opKey.GetName(), i));
      return eFalse;
    }
    tHStringPtr key = _H(opKey.GetVariable()->GetString());
    if (HStringIsEmpty(key)) {
      EXPRESSION_TRACE(niFmt("Global(): variable name resolves to an empty string '%s' (%d).",
                             key, i+1));
      return eFalse;
    }

    Op::sOperand& opVal = mvOperands[i+1];
    if (!opVal.Eval(apContext)) {
      EXPRESSION_TRACE(niFmt("Global(): operation, can't evaluate value operand '%s' (%d).",
                             key, i+1));
      return eFalse;
    }

    Ptr<iExpressionVariable> val = opVal.GetVariable()->Clone();
    val->SetName(key);
    if (!apContext->AddVariable(val)) {
      EXPRESSION_TRACE(niFmt("Global(): operation, can't add variable '%s' (%d).",
                             key, i));
      return eFalse;
    }
  }

  mptrResult = mvOperands.back().GetVariable();
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  return eTrue;
}
EndOp()


// Set function
BeginOpLF(Set,eInvalidHandle)
Ptr<iExpressionContext> mptrLocalContext;
astl::vector<Ptr<iExpressionVariable> > mvLocalVariables;

tBool SetupEvaluation(iExpressionContext* apContext)
{
  if (!mptrLocalContext.IsOK() || (mptrLocalContext->GetParentContext() != apContext)) {
    // niDebugFmt(("... Creating local Set() context: was %p, parent was %p, new parent is %p", (tIntPtr)mptrLocalContext.ptr(), (tIntPtr)(mptrLocalContext.IsOK() ? mptrLocalContext->GetParentContext() : 0), (tIntPtr)apContext));
    mptrLocalContext = apContext->CreateContext();
    if (!mptrLocalContext.IsOK()) {
      EXPRESSION_TRACE(_A("Set(): can't create the local context."));
      return eFalse;
    }
    mvLocalVariables.clear();
  }

  const tSize numOperands = mvOperands.size();
  if (numOperands < 3) {
    EXPRESSION_TRACE(_A("Set(): must have at least three operands."));
    return eFalse;
  }
  if ((numOperands % 2) != 1) {
    EXPRESSION_TRACE("Set(): must have a last operand which is the returned valued.");
    return eFalse;
  }

  tI32 i = 0;
  if (!mvLocalVariables.empty()) {
    // niDebugFmt(("... Partial eval, refetching data of variables"));
    const tSize numLocals = mvLocalVariables.size();
    niLoop(li,numLocals) {
      iExpressionVariable* val = mvLocalVariables[li];
      const tU32 opValIndex = (li*2)+1;
      Op::sOperand& opVal = mvOperands[opValIndex];
      if (!opVal.Eval(mptrLocalContext)) {
        EXPRESSION_TRACE(niFmt("Set(): operation, can't evaluate value operand %d (%s).",
                                opValIndex, val->GetName()));
        return eFalse;
      }
      val->Copy(opVal.GetVariable());
    }
    i = numLocals * 2;
  }
  else {
    const tU32 numDeclsTimes2 = (numOperands - (numOperands%2));
    mvLocalVariables.resize(numDeclsTimes2/2);
    tU32 i = 0;
    for ( ; i < numDeclsTimes2; i += 2) {
      Op::sOperand& opKey = mvOperands[i];
      if (!opKey.Eval(mptrLocalContext)) {
        EXPRESSION_TRACE(niFmt("Set(): operation, can't evaluate key operand '%s' (%d).",
                               opKey.GetName(), i));
        return eFalse;
      }
      if (opKey.GetVariable()->GetType() != eExpressionVariableType_String) {
        EXPRESSION_TRACE(niFmt("Set(): operation, key operand '%s' (%d) is not a string.",
                               opKey.GetName(), i));
        return eFalse;
      }
      tHStringPtr key = _H(opKey.GetVariable()->GetString());
      if (HStringIsEmpty(key)) {
        EXPRESSION_TRACE(niFmt("Set(): variable name resolves to an empty string '%s' (%d).",
                               key, i+1));
        return eFalse;
      }

      Op::sOperand& opVal = mvOperands[i+1];
      if (!opVal.Eval(mptrLocalContext)) {
        EXPRESSION_TRACE(niFmt("Set(): operation, can't evaluate value operand '%s' (%d).",
                               key, i+1));
        return eFalse;
      }

      Ptr<iExpressionVariable> val = opVal.GetVariable()->Clone();
      val->SetName(key);
      if (!mptrLocalContext->AddVariable(val)) {
        EXPRESSION_TRACE(niFmt("Set(): operation, can't add variable '%s' (%d).",
                               key, i));
        return eFalse;
      }
      mvLocalVariables[i/2] = val;
    }
  }

  for ( ; i < numOperands; ++i) {
    Op::sOperand& op = mvOperands[i];
    if (!op.Eval(mptrLocalContext)) {
      EXPRESSION_TRACE(niFmt("Set(): operation, can't evaluate last operand %d.", i));
      return eFalse;
    }
  }

  mptrResult = mvOperands.back().GetVariable();
  // niDebugFmt(("... Set result is constant: %s", niFlagIs(mptrResult->GetFlags(),eExpressionVariableFlags_Constant)));
  return eTrue;
}
tBool DoEvaluate(iExpressionContext*)
{
  return eTrue;
}
EndOp()

// If function
BeginOpLF(If,eInvalidHandle)
tBool SetupEvaluation(iExpressionContext* apContext)
{
  const tSize numOperands = mvOperands.size();
  if (numOperands < 3) {
    EXPRESSION_TRACE(_A("If(): must have at least three operands."));
    return eFalse;
  }
  if ((numOperands % 2) != 1) {
    EXPRESSION_TRACE("If(): must have a last operand which is the value returned if not test pass.");
    return eFalse;
  }
  return eTrue;
}
tBool DoEvaluate(iExpressionContext* apContext)
{
  const tSize numOperands = mvOperands.size();

  Op::sOperand* opVal = &mvOperands.back();
  const tSize numTestsTimes2 = numOperands-1;
  tSize i = 0;
  for ( ; i < numTestsTimes2; i += 2) {
    Op::sOperand& opTest = mvOperands[i];
    if (!opTest.Eval(apContext)) {
      EXPRESSION_TRACE(niFmt("If(): operation, can't evaluate test operand '%s' (%d).",
                             opTest.GetName(), i));
      return eFalse;
    }
    if (opTest.ToBool()) {
      opVal = &mvOperands[i+1];
      break;
    }
  }

  if (!opVal->Eval(apContext)) {
    EXPRESSION_TRACE(niFmt("If(): operation, can't evaluate value operand %d.", i));
    return eFalse;
  }
  if (!mptrResult.IsOK()) {
    mptrResult = opVal->GetVariable()->Clone();
  }
  else {
    mptrResult->Copy(opVal->GetVariable());
  }
  return eTrue;
}
EndOp()

// Eval function
BeginOpF(Eval,1)
tBool SetupEvaluation(iExpressionContext* apContext)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_String);
  return eTrue;
}
tBool DoEvaluate(iExpressionContext* apContext)
{
  auto v = mvOperands[0];
  v.Eval(apContext);
  auto newV = v.GetVariable();
  if (newV && newV->GetType() == ni::eExpressionVariableType_String) {
    mptrResult = apContext->Eval(v.GetVariable()->GetString().Chars());
  }
  return eTrue;
}
EndOp()


inline eExpressionVariableType _VarTypeToExpressionType(eDataTablePropertyType aType) {
  switch (aType) {
    case eDataTablePropertyType_String:
      return eExpressionVariableType_String;
    case eDataTablePropertyType_Bool:
    case eDataTablePropertyType_Int32:
    case eDataTablePropertyType_Int64:
    case eDataTablePropertyType_Float32:
    case eDataTablePropertyType_Float64:
      return eExpressionVariableType_Float;
    case eDataTablePropertyType_Vec2:
      return eExpressionVariableType_Vec2;
    case eDataTablePropertyType_Vec3:
      return eExpressionVariableType_Vec3;
    case eDataTablePropertyType_Vec4:
      return eExpressionVariableType_Vec4;
    case eDataTablePropertyType_Matrix:
      return eExpressionVariableType_Matrix;
    case eDataTablePropertyType_IUnknown:
      return ni::eExpressionVariableType_IUnknown;
    default:
    case eDataTablePropertyType_Unknown:
      niAssert("Invalid type");
      return eExpressionVariableType_Float;
  };
}

// DataTable function
BeginOpF(DTGet,2)
tBool SetupEvaluation(iExpressionContext* apContext)
{
  mptrResult = _CreateVariable(NULL,ni::eExpressionVariableType_Float);
  return eTrue;
}

tBool DoEvaluate(iExpressionContext* apContext)
{
  QPtr<iDataTable> dt = mvOperands[0].GetVariable()->GetIUnknown();
  if (!dt.IsOK()) {
    EXPRESSION_TRACE("DTGet(): operation, input is not a valid datatable.");
    return eFalse;
  }

  auto var = mvOperands[1].GetVariable();
  if (var->GetType() == ni::eExpressionVariableType_Float) {
    Ptr<ExprVar> ret = _CreateVariable(NULL, _VarTypeToExpressionType(dt->GetPropertyTypeFromIndex(var->GetFloat())));
    ret->SetVar(dt->GetVarFromIndex(var->GetFloat()));
    mptrResult = ret;
  }
  else if (var->GetType() == ni::eExpressionVariableType_String) {
    cString path = var->GetString();
    if (path.contains("@")) {
      const Var& v = dt->GetVarFromPath(var->GetString().Chars(), niVarNull);
      mptrResult = apContext->CreateVariableFromVar(NULL, v);
    }
    else {
      Ptr<ExprVar> ret = _CreateVariable(NULL, _VarTypeToExpressionType(dt->GetPropertyType(var->GetString().Chars())));
      ret->SetVar(dt->GetVar(var->GetString().Chars()));
      mptrResult = ret;
    }
  }
  else {
    return eFalse;
  }
  return eTrue;
}
EndOp()

// DataTable function
BeginOpF(DTGetChild, eInvalidHandle)
tBool SetupEvaluation(iExpressionContext* apContext)
{
  mptrResult = _CreateVariable(NULL,ni::eExpressionVariableType_IUnknown);
  return eTrue;
}

tBool DoEvaluate(iExpressionContext* apContext)
{
  QPtr<iDataTable> dt = mvOperands[0].GetVariable()->GetIUnknown();
  if (!dt.IsOK()) {
    return eTrue;
  }

  Ptr<iDataTable> d = dt.ptr();
  for (tU32 i = 1; i < mvOperands.size(); ++i) {
    Ptr<iExpressionVariable> var = mvOperands[i].GetVariable();
    if (var->GetType() == ni::eExpressionVariableType_Float) {
      d = d->GetChildFromIndex(var->GetFloat());
    }
    else if (var->GetType() == ni::eExpressionVariableType_String) {
      d = d->GetChild(var->GetString().Chars());
    }
    else {
      return eTrue;
    }
  }

  mptrResult->SetIUnknown(d);
  return eTrue;
}
EndOp()

// DataTable function
BeginOpF(DTFindChild,3)
tBool SetupEvaluation(iExpressionContext* apContext)
{
  mptrResult = _CreateVariable(NULL,ni::eExpressionVariableType_IUnknown);
  return eTrue;
}

tBool DoEvaluate(iExpressionContext* apContext)
{
  QPtr<iDataTable> dt = mvOperands[0].GetVariable()->GetIUnknown();
  if (!dt.IsOK()) {
    EXPRESSION_TRACE("DTSearch(): operation, input 0 is not a valid datatable.");
    return eFalse;
  }

  auto var1 = mvOperands[1].GetVariable()->GetString();
  auto var2 = mvOperands[2].GetVariable()->GetString();
  niLoop(i, dt->GetNumChildren()) {
    Ptr<iDataTable> c = dt->GetChildFromIndex(i);
    cString val = c->GetString(var1.Chars());
    if (val.Eq(var2)) {
      mptrResult->SetIUnknown(c);
      break;
    }
  }
  return eTrue;
}
EndOp()

// DataTable function
BeginOpF(DTCopyMatch,3)
tBool SetupEvaluation(iExpressionContext* apContext)
{
  mptrResult = _CreateVariable(NULL,ni::eExpressionVariableType_IUnknown);
  return eTrue;
}

tBool DoEvaluate(iExpressionContext* apContext)
{
  QPtr<iDataTable> dt = mvOperands[0].GetVariable()->GetIUnknown();
  if (!dt.IsOK()) {
    EXPRESSION_TRACE("DTSearch(): operation, input 0 is not a valid datatable.");
    return eFalse;
  }

  cString var1 = mvOperands[1].GetVariable()->GetString();
  if (var1.IsEmpty()) {
    mptrResult->SetIUnknown(dt->Clone());
    return eTrue;
  }

  Ptr<iDataTable> ret = dt->CloneEx(eDataTableCopyFlags_Default);
  cString var2 = mvOperands[2].GetVariable()->GetString();
  Ptr<iRegex> regex = ni::CreateFilePatternRegex(var2.Chars());
  niLoop(i, dt->GetNumChildren()) {
    Ptr<iDataTable> c = dt->GetChildFromIndex(i);
    cString val = c->GetString(var1.Chars());
    if (regex.IsOK() && regex->DoesMatch(val.Chars())) {
      ret->AddChild(c->Clone());
    }
  }

  mptrResult->SetIUnknown(ret);
  return eTrue;
}
EndOp()


// DataTable function
BeginOpF(DTGetChildIndex,2)
tBool SetupEvaluation(iExpressionContext* apContext)
{
  mptrResult = _CreateVariable(NULL,ni::eExpressionVariableType_Float);
  return eTrue;
}

tBool DoEvaluate(iExpressionContext* apContext)
{
  QPtr<iDataTable> dt = mvOperands[0].GetVariable()->GetIUnknown();
  if (!dt.IsOK()) {
    EXPRESSION_TRACE("DTGet(): operation, input is not a valid datatable.");
    return eFalse;
  }

  auto var = mvOperands[1].GetVariable()->GetString();
  mptrResult->SetFloat(dt->GetChildIndex(var.Chars()));
  return eTrue;
}
EndOp()

// DataTable function
BeginOpF(DTGetNumChildren,1)
tBool SetupEvaluation(iExpressionContext* apContext)
{
  mptrResult = _CreateVariable(NULL,ni::eExpressionVariableType_Float);
  return eTrue;
}

tBool DoEvaluate(iExpressionContext* apContext)
{
  QPtr<iDataTable> dt = mvOperands[0].GetVariable()->GetIUnknown();
  if (!dt.IsOK()) {
    EXPRESSION_TRACE("DTGet(): operation, input is not a valid datatable.");
    return eFalse;
  }
  mptrResult->SetFloat(dt->GetNumChildren());
  return eTrue;
}
EndOp()

// DataTable function
BeginOpF(DTGetName,1)
tBool SetupEvaluation(iExpressionContext* apContext)
{
  mptrResult = _CreateVariable(NULL,ni::eExpressionVariableType_String);
  return eTrue;
}

tBool DoEvaluate(iExpressionContext* apContext)
{
  QPtr<iDataTable> dt = mvOperands[0].GetVariable()->GetIUnknown();
  if (!dt.IsOK()) {
    EXPRESSION_TRACE("DTGet(): operation, input is not a valid datatable.");
    return eFalse;
  }
  mptrResult->SetString(dt->GetName());
  return eTrue;
}
EndOp()

// DataTable function
BeginOpF(DTGetIndex,1)
tBool SetupEvaluation(iExpressionContext* apContext)
{
  mptrResult = _CreateVariable(NULL,ni::eExpressionVariableType_Float);
  return eTrue;
}

tBool DoEvaluate(iExpressionContext* apContext)
{
  QPtr<iDataTable> dt = mvOperands[0].GetVariable()->GetIUnknown();
  if (!dt.IsOK()) {
    EXPRESSION_TRACE("DTGet(): operation, input is not a valid datatable.");
    return eFalse;
  }
  mptrResult->SetFloat(dt->GetIndex());
  return eTrue;
}
EndOp()

// DT Json function
BeginOpF(DTArray,eInvalidHandle)
tBool SetupEvaluation(iExpressionContext* apContext)
{
  mptrResult = _CreateVariable(NULL,ni::eExpressionVariableType_IUnknown);
  return eTrue;
}

tBool DoEvaluate(iExpressionContext* apContext)
{
  if (mvOperands.size() > 0) {
    Ptr<iDataTable> dt = ni::CreateDataTable();
    dt->SetBool("__isArray", true);
    Ptr<iDataTableWriteStack> stack = ni::CreateDataTableWriteStack(dt);
    for (auto const& v: mvOperands) {
      Ptr<iExpressionVariable> val = v.GetVariable();
      if (val.IsOK()) {
        switch (val->GetType()) {
          case eExpressionVariableType_Float: {
            stack->PushNew("jnum");
            stack->SetFloat("v", val->GetFloat());
            stack->Pop();
            break;
          }
          case eExpressionVariableType_Vec2:
          case eExpressionVariableType_Vec3:
          case eExpressionVariableType_Vec4:
          case eExpressionVariableType_Matrix:
          case eExpressionVariableType_String: {
            stack->PushNew("jstr");
            stack->SetString("v", val->GetString().Chars());
            stack->Pop();
            break;
          }
          case eExpressionVariableType_IUnknown:
            QPtr<iDataTable> data = val->GetIUnknown();
            if (data.IsOK()) {
              stack->PushAppend(data);
              stack->Pop();
            }
            else {
              stack->PushNew("jnull");
              stack->Pop();
            }
            break;
        }
      }
    }

    mptrResult->SetIUnknown(dt);
  }
  return eTrue;
}
EndOp()

BeginOpF(DTObject,eInvalidHandle)
tBool SetupEvaluation(iExpressionContext* apContext)
{
  if (mvOperands.size() % 2 != 0) {
    EXPRESSION_TRACE("Object(K,V...) -> String: Number of parameters should be even.");
    return eFalse;
  }
  mptrResult = _CreateVariable(NULL,ni::eExpressionVariableType_IUnknown);
  return eTrue;
}

tBool DoEvaluate(iExpressionContext* apContext)
{
  if (mvOperands.size() > 0) {
    Ptr<iDataTable> dt = ni::CreateDataTable();
    for (tU32 i = 0; i < mvOperands.size(); i += 2) {
      Ptr<iExpressionVariable> key = mvOperands[i].GetVariable();
      if (key->GetType() != ni::eExpressionVariableType_String) {
        EXPRESSION_TRACE(niFmt("Object(K,V...) -> String: Key[%s] should be string", i));
        return eFalse;
      }
      else {
        cString k = key->GetString();
        if (k.IsEmpty()) {
          EXPRESSION_TRACE(niFmt("Object(K,V...) -> String: Key[%s] is empty, skip", i));
          continue;
        }

        Ptr<iExpressionVariable> val = mvOperands[i + 1].GetVariable();
        switch (val->GetType()) {
          case eExpressionVariableType_Float: {
            dt->SetVar(k.Chars(), val->GetFloat());
            break;
          }
          case eExpressionVariableType_Vec2:
          case eExpressionVariableType_Vec3:
          case eExpressionVariableType_Vec4:
          case eExpressionVariableType_Matrix:
          case eExpressionVariableType_String: {
            dt->SetString(k.Chars(), val->GetString().Chars());
            break;
          }
          case eExpressionVariableType_IUnknown:
            QPtr<iDataTable> data = val->GetIUnknown();
            Ptr<iDataTable> newDT = data.IsOK() ? data->Clone() : ni::CreateDataTable();
            newDT->SetName(k.Chars());
            dt->AddChild(newDT);
            break;
        }

      }
    }
    mptrResult->SetIUnknown(dt);
  }
  return eTrue;
}
EndOp()

BeginOpF(DTToJson,1)
tBool SetupEvaluation(iExpressionContext* apContext)
{
  mptrResult = _CreateVariable(NULL,ni::eExpressionVariableType_String);
  return eTrue;
}

tBool DoEvaluate(iExpressionContext* apContext)
{
  QPtr<iDataTable> dt = mvOperands[0].GetVariable()->GetIUnknown();
  if (!dt.IsOK()) {
    EXPRESSION_TRACE("DTToJson(): operation, input is not a valid datatable.");
    return eFalse;
  }
  Ptr<iFile> file = ni::CreateFileDynamicMemory(0, NULL);
  ni::SerializeDataTable("json", eSerializeMode_Write, dt, file);
  mptrResult->SetString(file->ReadString());
  return eTrue;
}
EndOp()

BeginOpF(DTFromJson,1)
tBool SetupEvaluation(iExpressionContext* apContext)
{
  mptrResult = _CreateVariable(NULL,ni::eExpressionVariableType_IUnknown);
  return eTrue;
}

tBool DoEvaluate(iExpressionContext* apContext)
{
  cString json = mvOperands[0].GetVariable()->GetString();
  Ptr<iDataTable> dt = CreateDataTable();
  Nonnull<iFile> fp(ni::GetLang()->CreateFileMemory(
                      (tPtr)json.data(),json.size(),eFalse,"::CreateDataTableFromJson"));
  if (!ni::SerializeDataTable("json", eSerializeMode_Read, dt, fp)) {
    EXPRESSION_TRACE("DTFromJson(): operation, input is not a valid json.");
    return eFalse;
  }
  mptrResult->SetIUnknown(dt);
  return eTrue;
}
EndOp()

BeginOpF(DTFromPath,1)
tBool SetupEvaluation(iExpressionContext* apContext)
{
  mptrResult = _CreateVariable(NULL,ni::eExpressionVariableType_IUnknown);
  return eTrue;
}

tBool DoEvaluate(iExpressionContext* apContext)
{
  cPath path = mvOperands[0].GetVariable()->GetString();
  Ptr<iFile> fp = ni::GetLang()->URLOpen(path.GetPath().Chars());
  if (!fp.IsOK()) {
    EXPRESSION_TRACE(niFmt("DTFromPath(): operation, path [%s] not found.", path.GetPath()));
    return eFalse;
  }

  Ptr<iDataTable> dt = CreateDataTable();
  if (!ni::GetLang()->SerializeDataTable(path.GetExtension().Chars(),eSerializeMode_Read,dt,fp)) {
    EXPRESSION_TRACE(niFmt("DTFromPath(): operation, path [%s] is not a valid datatable.", path.GetPath()));
    return eFalse;
  }
  mptrResult->SetIUnknown(dt);
  return eTrue;
}
EndOp()

BeginOpVF(ArrSum, 1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,ni::eExpressionVariableType_Float);
  return eTrue;
}

tBool DoEvaluate(iExpressionContext*)
{
  QPtr<iDataTable> dt = mvOperands[0].GetVariable()->GetIUnknown();
  if (dt.IsOK() && dt->GetNumChildren() > 0) {
    tF64 ret = 0;
    niLoop(i, dt->GetNumChildren()) {
      ret += dt->GetChildFromIndex(i)->GetFloat("v");
    }
    mptrResult->SetFloat(ret);
  }
  return eTrue;
}
EndOp()

BeginOpVF(ArrAdd, 2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,ni::eExpressionVariableType_IUnknown);
  return eTrue;
}

tBool DoEvaluate(iExpressionContext*)
{
  QPtr<iDataTable> dt0 = mvOperands[0].GetVariable()->GetIUnknown();
  QPtr<iDataTable> dt1 = mvOperands[1].GetVariable()->GetIUnknown();
  if (!dt0.IsOK() && !dt1.IsOK()) {
    // return eTrue;
  } else if (dt0.IsOK() && !dt1.IsOK()) {
    mptrResult->SetIUnknown(dt0);
  } else if (!dt0.IsOK() && dt1.IsOK()) {
    mptrResult->SetIUnknown(dt1);
  } else if (dt0.IsOK() && dt1.IsOK())  {
    Ptr<iDataTable> dt = CreateDataTable("sum");
    dt->SetBool("__isArray", true);
    Ptr<iDataTableWriteStack> stack = ni::CreateDataTableWriteStack(dt);
    tU32 n0 = dt0->GetNumChildren();
    tU32 n1 = dt1->GetNumChildren();
    tU32 num = Max(n0, n1);
    niLoop(i, num) {
      tF64 f0 = n0 > i ? dt0->GetChildFromIndex(i)->GetFloat("v") : 0;
      tF64 f1 = n1 > i ? dt1->GetChildFromIndex(i)->GetFloat("v") : 0;
      stack->PushNew("jnum");
      stack->SetFloat("v", f0 + f1);
      stack->Pop();
    }
    mptrResult->SetIUnknown(dt);
  }
  return eTrue;
}
EndOp()


BeginOpVF(ArrAvg, 1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,ni::eExpressionVariableType_Float);
  return eTrue;
}

tBool DoEvaluate(iExpressionContext*)
{
  QPtr<iDataTable> dt = mvOperands[0].GetVariable()->GetIUnknown();
  if (dt.IsOK() && dt->GetNumChildren() > 0) {
    tF64 ret = 0;
    niLoop(i, dt->GetNumChildren()) {
      ret += dt->GetChildFromIndex(i)->GetFloat("v");
    }
    mptrResult->SetFloat(ret / dt->GetNumChildren());
  }
  return eTrue;
}
EndOp()

BeginOpVF(ArrMax, 1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,ni::eExpressionVariableType_Float);
  return eTrue;
}

tBool DoEvaluate(iExpressionContext*)
{
  QPtr<iDataTable> dt = mvOperands[0].GetVariable()->GetIUnknown();
  if (dt.IsOK() && dt->GetNumChildren() > 0) {
    tF64 ret = dt->GetChildFromIndex(0)->GetFloat("v");
    niLoop(i, dt->GetNumChildren()) {
      ret = Max(dt->GetChildFromIndex(i)->GetFloat("v"), ret);
    }
    mptrResult->SetFloat(ret);
  }
  return eTrue;
}
EndOp()

BeginOpVF(ArrMin, 1)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,ni::eExpressionVariableType_Float);
  return eTrue;
}

tBool DoEvaluate(iExpressionContext*)
{
  QPtr<iDataTable> dt = mvOperands[0].GetVariable()->GetIUnknown();
  if (dt.IsOK() && dt->GetNumChildren() > 0) {
    tF64 ret = dt->GetChildFromIndex(0)->GetFloat("v");
    niLoop(i, dt->GetNumChildren()) {
      ret = Min(dt->GetChildFromIndex(i)->GetFloat("v"), ret);
    }
    mptrResult->SetFloat(ret);
  }
  return eTrue;
}
EndOp()

BeginOpVF(ArrValues, 2)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,ni::eExpressionVariableType_IUnknown);
  return eTrue;
}

tBool DoEvaluate(iExpressionContext*)
{
  QPtr<iDataTable> dt = mvOperands[0].GetVariable()->GetIUnknown();

  if (!dt.IsOK() || dt->GetNumChildren() == 0) return eTrue;

  auto type = mvOperands[1].GetVariable()->GetType();
  cString key = "v";
  tU32 index = eInvalidHandle;
  tBool useIndex;
  if (type == eExpressionVariableType_String) {
    useIndex = eFalse;
    key = mvOperands[1].GetVariable()->GetString();
  }
  else if (type == eExpressionVariableType_Float) {
    useIndex = eTrue;
    index = mvOperands[1].GetVariable()->GetFloat();
  }
  else {
    return eTrue;
  }

  Ptr<iDataTable> ret = CreateDataTable("Values");
  ret->SetBool("__isArray", true);
  Ptr<iDataTableWriteStack> stack = ni::CreateDataTableWriteStack(ret);
  niLoop(i, dt->GetNumChildren()) {
    Ptr<iDataTable> child = dt->GetChildFromIndex(i);
    eDataTablePropertyType type = ni::eDataTablePropertyType_Unknown;
    if (useIndex) {
      child = child->GetChildFromIndex(index);
      if (child.IsOK()) {
        type = child->GetPropertyType(key.Chars());
      }
    }
    else {
      type = child->GetPropertyType(key.Chars());
    }

    switch (type) {
      case eDataTablePropertyType_Bool: {
        stack->PushNew("jbool");
        stack->SetBool("v", child->GetBool(key.Chars()));
        stack->Pop();
        break;
      }

      case eDataTablePropertyType_Int32:
      case eDataTablePropertyType_Int64:
      case eDataTablePropertyType_Float32:
      case eDataTablePropertyType_Float64: {
        stack->PushNew("jnum");
        stack->SetFloat("v", child->GetFloat(key.Chars()));
        stack->Pop();
        break;
      }

      case eDataTablePropertyType_String:
      case eDataTablePropertyType_Vec2:
      case eDataTablePropertyType_Vec3:
      case eDataTablePropertyType_Vec4:
      case eDataTablePropertyType_Matrix: {
        stack->PushNew("jstr");
        stack->SetString("v", child->GetString(key.Chars()).Chars());
        stack->Pop();
        break;
      }

      case eDataTablePropertyType_IUnknown: {
        stack->PushNew("jobj");
        stack->SetIUnknown("v", child->GetIUnknown(key.Chars()));
        stack->Pop();
        break;
      }

      case eDataTablePropertyType_Unknown:
        stack->PushNew("jnull");
        stack->SetIUnknown("v", NULL);
        stack->Pop();
        break;
    }
  }
  mptrResult->SetIUnknown(ret);
  return eTrue;
}
EndOp()


BeginOpVF(ArrRangeI, 3)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_IUnknown);
  return eTrue;
}

tBool DoEvaluate(iExpressionContext*)
{
  tI32 start = mvOperands[0].GetVariable()->GetFloat();
  tI32 end = mvOperands[1].GetVariable()->GetFloat();
  tI32 step = mvOperands[2].GetVariable()->GetFloat();

  if (start > end) std::swap(start, end);

  Ptr<iDataTable> dt = CreateDataTable("array");
  dt->SetBool("__isArray", true);
  Ptr<iDataTableWriteStack> stack = ni::CreateDataTableWriteStack(dt);
  tU32 count = 0;
  for (tI32 f = start; f <= end; f += step) {
    if (count >= 100) {
      break;
    }
    stack->PushNew("jnum");
    stack->SetFloat("v", f);
    stack->Pop();

    ++count;
  }

  mptrResult->SetIUnknown(dt);
  return eTrue;
}
EndOp()

static inline tU32 __getDaysInMonth(tU32 month, tU32 year) {
  switch (month) {
    case 1:  // January
    case 3:  // March
    case 5:  // May
    case 7:  // July
    case 8:  // August
    case 10: // October
    case 12: // December
      return 31;
    case 4:  // April
    case 6:  // June
    case 9:  // September
    case 11: // November
      return 30;
    case 2: // February
      // Check for leap year
      if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        return 29; // Leap year
      } else {
        return 28; // Non-leap year
      }
    default:
      return eInvalidHandle; // Invalid month
  }
}

BeginOpVF(ArrMonthDays,eInvalidHandle)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_IUnknown);
  return eTrue;
}

tBool DoEvaluate(iExpressionContext*)
{
  Ptr<iTime> t = ni::GetLang()->GetCurrentTime()->Clone();
  tU32 month = t->GetMonth();
  tU32 year = t->GetYear();
  if (mvOperands.size() > 0) month = mvOperands[0].GetVariable()->GetFloat();
  if (mvOperands.size() > 1) year = mvOperands[1].GetVariable()->GetFloat();

  tU32 num = __getDaysInMonth(month,year);
  if (num != eInvalidHandle) {
    Ptr<iDataTable> dt = CreateDataTable("month");
    dt->SetBool("__isArray", true);
    Ptr<iDataTableWriteStack> stack = ni::CreateDataTableWriteStack(dt);
    for (tU32 i = 1; i <= num; ++i) {
      stack->PushNew("jnum");
      stack->SetFloat("v", i);
      stack->Pop();
    }
    mptrResult->SetIUnknown(dt);
  }

  return eTrue;
}
EndOp()

static const ni::achar* _kaszWeekdays[][7] = {
  {"Su","Mo","Tu","We","Th","Fr","Sa"},
  {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"},
  {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"},
  {"日","一","二","三","四","五","六"},
  {"星期日","星期一","星期二","星期三","星期四","星期五","星期六"}
};

BeginOpVF(ArrWeekDays, eInvalidHandle)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_IUnknown);
  return eTrue;
}

tBool DoEvaluate(iExpressionContext*)
{
  tU32 type = 0;
  if (mvOperands.size() > 0) type = mvOperands[0].GetVariable()->GetFloat();
  type = type > 4 ? 0 : type;

  Ptr<iDataTable> dt = CreateDataTable("weekdays");
  dt->SetBool("__isArray", true);
  Ptr<iDataTableWriteStack> stack = ni::CreateDataTableWriteStack(dt);
  for (tU32 i = 0; i < 7; ++i) {
    stack->PushNew("jstr");
    stack->SetString("v", _kaszWeekdays[type][i]);
    stack->Pop();
  }

  mptrResult->SetIUnknown(dt);
  return eTrue;
}
EndOp()

static const ni::achar* _kaszMonth[][12] = {
  {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"},
  {"January","February","March","April","May","June","July","August","September","October","November","December"},
  {"一月","二月","三月","四月","五月","六月","七月","八月","九月","十月","十一月","十二月"},
  {"1月","2月","3月","4月","5月","6月","7月","8月","9月","10月","11月","12月"},
};

BeginOpVF(ArrMonths, eInvalidHandle)
tBool SetupEvaluation(iExpressionContext*)
{
  mptrResult = _CreateVariable(NULL,eExpressionVariableType_IUnknown);
  return eTrue;
}

tBool DoEvaluate(iExpressionContext*)
{
  tU32 type = 0;
  if (mvOperands.size() > 0) type = mvOperands[0].GetVariable()->GetFloat();
  type = type > 3 ? 0 : type;

  Ptr<iDataTable> dt = CreateDataTable("months");
  dt->SetBool("__isArray", true);
  Ptr<iDataTableWriteStack> stack = ni::CreateDataTableWriteStack(dt);
  for (tU32 i = 0; i < 12; ++i) {
    stack->PushNew("jstr");
    stack->SetString("v", _kaszMonth[type][i]);
    stack->Pop();
  }

  mptrResult->SetIUnknown(dt);
  return eTrue;
}
EndOp()


#undef DoSwitch
#undef DoSwitch1
#undef DoSwitch2
#undef DoSwitch3

#undef OP0
#undef OP1
#undef OP2
#undef OP3
#undef DEST

//////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////
Evaluator::Evaluator(Evaluator* apParent) : mptrParent(apParent)
{
  mpDefaultEnum = NULL;
  mbGlobalEnumSearch = eFalse;
  if (!mptrParent.IsOK()) {
    mbOK = _RegisterReservedVariables();
  }
  else {
    mbOK = eTrue;
  }
}

///////////////////////////////////////////////
Evaluator::~Evaluator() {
}

///////////////////////////////////////////////
tBool Evaluator::_RegisterReservedVariables() {
  typedef ni::Var (__cdecl *tpfnGetVariable)();

  struct _Local {
    static ni::Var time() {
      return ni::TimerInSeconds();
    }
    static ni::Var frametime() {
      return OP_GET_FRAME_TIME;
    }
  };
  struct IotaRunnable : public ImplRC<iRunnable> {
    tU32 _iota = 0;
    virtual Var __stdcall Run() {
      return (tF64)_iota++;
    }
  };

  {
    Ptr<iExpressionVariable> v = CreateVariableFromRunnable(
      "time",
      eExpressionVariableType_Float,
      ni::Runnable<tpfnGetVariable>(_Local::time),
      eExpressionVariableFlags_Reserved);
    if (!AddVariable(v)) {
      niError(_A("Can't add 'time' reserved variable."));
      return eFalse;
    }
  }

  {
    Ptr<iExpressionVariable> v = CreateVariableFromRunnable(
      "frametime",
      eExpressionVariableType_Float,
      ni::Runnable<tpfnGetVariable>(_Local::frametime),
      eExpressionVariableFlags_Reserved);
    if (!AddVariable(v)) {
      niError(_A("Can't add 'frametime' reserved variable."));
      return eFalse;
    }
  }

  {
    Ptr<iExpressionVariable> v = CreateVariableFromRunnable(
      "iota",
      eExpressionVariableType_Float,
      niNew IotaRunnable(),
      eExpressionVariableFlags_Reserved);
    if (!AddVariable(v)) {
      niError(_A("Can't add 'iota' reserved variable."));
      return eFalse;
    }
  }

  {
    Ptr<iExpressionVariable> v = CreateVariable("pi",eExpressionVariableType_Float,eExpressionVariableFlags(eExpressionVariableFlags_Reserved|eExpressionVariableFlags_Constant));
    if (!AddVariable(v)) {
      niError(_A("Can't add 'pi' reserved variable."));
      return eFalse;
    }
    v->SetFloat(niPi);
  }

  {
    Ptr<iExpressionVariable> v = CreateVariable("true",eExpressionVariableType_Float,eExpressionVariableFlags(eExpressionVariableFlags_Reserved|eExpressionVariableFlags_Constant));
    if (!AddVariable(v)) {
      niError(_A("Can't add 'true' reserved variable."));
      return eFalse;
    }
    v->SetFloat(1.0);
  }

  {
    Ptr<iExpressionVariable> v = CreateVariable("false",eExpressionVariableType_Float,eExpressionVariableFlags(eExpressionVariableFlags_Reserved|eExpressionVariableFlags_Constant));
    if (!AddVariable(v)) {
      niError(_A("Can't add 'false' reserved variable."));
      return eFalse;
    }
    v->SetFloat(0.0);
  }

  {
    Ptr<iExpressionVariable> v = CreateVariable("yes",eExpressionVariableType_Float,eExpressionVariableFlags(eExpressionVariableFlags_Reserved|eExpressionVariableFlags_Constant));
    if (!AddVariable(v)) {
      niError(_A("Can't add 'yes' reserved variable."));
      return eFalse;
    }
    v->SetFloat(1.0);
  }

  {
    Ptr<iExpressionVariable> v = CreateVariable("no",eExpressionVariableType_Float,eExpressionVariableFlags(eExpressionVariableFlags_Reserved|eExpressionVariableFlags_Constant));
    if (!AddVariable(v)) {
      niError(_A("Can't add 'no' reserved variable."));
      return eFalse;
    }
    v->SetFloat(0.0);
  }

  {
    Ptr<iExpressionVariable> v = CreateVariable("pi2",eExpressionVariableType_Float,eExpressionVariableFlags(eExpressionVariableFlags_Reserved|eExpressionVariableFlags_Constant));
    if (!AddVariable(v)) {
      niError(_A("Can't add 'pi2' reserved variable."));
      return eFalse;
    }
    v->SetFloat(niPi2);
  }

  {
    Ptr<iExpressionVariable> v = CreateVariable("pi4",eExpressionVariableType_Float,eExpressionVariableFlags(eExpressionVariableFlags_Reserved|eExpressionVariableFlags_Constant));
    if (!AddVariable(v)) {
      niError(_A("Can't add 'pi4' reserved variable."));
      return eFalse;
    }
    v->SetFloat(niPi4);
  }

  {
    Ptr<iExpressionVariable> v = CreateVariable("max_float",eExpressionVariableType_Float,eExpressionVariableFlags(eExpressionVariableFlags_Reserved|eExpressionVariableFlags_Constant));
    if (!AddVariable(v)) {
      niError(_A("Can't add 'max_float' reserved variable."));
      return eFalse;
    }
    v->SetFloat(niMaxF32);
  }

  {
    Ptr<iExpressionVariable> v = CreateVariable("min_float",eExpressionVariableType_Float,eExpressionVariableFlags(eExpressionVariableFlags_Reserved|eExpressionVariableFlags_Constant));
    if (!AddVariable(v)) {
      niError(_A("Can't add 'min_float' reserved variable."));
      return eFalse;
    }
    v->SetFloat(niMinF32);
  }

  {
    Ptr<iExpressionVariable> v = CreateVariable("invalid",eExpressionVariableType_Float,eExpressionVariableFlags(eExpressionVariableFlags_Reserved|eExpressionVariableFlags_Constant));
    if (!AddVariable(v)) {
      niError(_A("Can't add 'invalid' reserved variable."));
      return eFalse;
    }
    v->SetFloat(ni::eInvalidHandle);
  }

  {
    Ptr<iExpressionVariable> v = CreateVariable("particles_epsilon",eExpressionVariableType_Float,eExpressionVariableFlags(eExpressionVariableFlags_Reserved|eExpressionVariableFlags_Constant));
    if (!AddVariable(v)) {
      niError(_A("Can't add 'particles_epsilon' reserved variable."));
      return eFalse;
    }
    v->SetFloat(niEpsilon3);
  }

  {
    Ptr<iExpressionVariable> v = CreateVariable("particles_max",eExpressionVariableType_Float,eExpressionVariableFlags(eExpressionVariableFlags_Reserved|eExpressionVariableFlags_Constant));
    if (!AddVariable(v)) {
      niError(_A("Can't add 'particles_max' reserved variable."));
      return eFalse;
    }
    v->SetFloat(1.0e16f);
  }

  {
    Ptr<iExpressionVariable> v = CreateVariable("epsilon",eExpressionVariableType_Float,eExpressionVariableFlags(eExpressionVariableFlags_Reserved|eExpressionVariableFlags_Constant));
    if (!AddVariable(v)) {
      niError(_A("Can't add 'epsilon' reserved variable."));
      return eFalse;
    }
    v->SetFloat(niEpsilon5);
  }

  {
    Ptr<iExpressionVariable> v = CreateVariable("epsilon0",eExpressionVariableType_Float,eExpressionVariableFlags(eExpressionVariableFlags_Reserved|eExpressionVariableFlags_Constant));
    if (!AddVariable(v)) {
      niError(_A("Can't add 'epsilon0' reserved variable."));
      return eFalse;
    }
    v->SetFloat(niEpsilon0);
  }

  {
    Ptr<iExpressionVariable> v = CreateVariable("epsilon1",eExpressionVariableType_Float,eExpressionVariableFlags(eExpressionVariableFlags_Reserved|eExpressionVariableFlags_Constant));
    if (!AddVariable(v)) {
      niError(_A("Can't add 'epsilon1' reserved variable."));
      return eFalse;
    }
    v->SetFloat(niEpsilon1);
  }

  {
    Ptr<iExpressionVariable> v = CreateVariable("epsilon2",eExpressionVariableType_Float,eExpressionVariableFlags(eExpressionVariableFlags_Reserved|eExpressionVariableFlags_Constant));
    if (!AddVariable(v)) {
      niError(_A("Can't add 'epsilon2' reserved variable."));
      return eFalse;
    }
    v->SetFloat(niEpsilon2);
  }

  {
    Ptr<iExpressionVariable>
        v = CreateVariable("epsilon3",eExpressionVariableType_Float,eExpressionVariableFlags(eExpressionVariableFlags_Reserved|eExpressionVariableFlags_Constant));
    if (!AddVariable(v)) {
      niError(_A("Can't add 'epsilon3' reserved variable."));
      return eFalse;
    }
    v->SetFloat(niEpsilon3);
  }

  {
    Ptr<iExpressionVariable> v = CreateVariable("epsilon4",eExpressionVariableType_Float,eExpressionVariableFlags(eExpressionVariableFlags_Reserved|eExpressionVariableFlags_Constant));
    if (!AddVariable(v)) {
      niError(_A("Can't add 'epsilon4' reserved variable."));
      return eFalse;
    }
    v->SetFloat(niEpsilon4);
  }

  {
    Ptr<iExpressionVariable> v = CreateVariable("epsilon5",eExpressionVariableType_Float,eExpressionVariableFlags(eExpressionVariableFlags_Reserved|eExpressionVariableFlags_Constant));
    if (!AddVariable(v)) {
      niError(_A("Can't add 'epsilon5' reserved variable."));
      return eFalse;
    }
    v->SetFloat(niEpsilon5);
  }

  {
    Ptr<iExpressionVariable> v = CreateVariable("epsilon6",eExpressionVariableType_Float,eExpressionVariableFlags(eExpressionVariableFlags_Reserved|eExpressionVariableFlags_Constant));
    if (!AddVariable(v)) {
      niError(_A("Can't add 'epsilon6' reserved variable."));
      return eFalse;
    }
    v->SetFloat(niEpsilon6);
  }

  {
    Ptr<iExpressionVariable> v = CreateVariable("epsilon7",eExpressionVariableType_Float,eExpressionVariableFlags(eExpressionVariableFlags_Reserved|eExpressionVariableFlags_Constant));
    if (!AddVariable(v)) {
      niError(_A("Can't add 'epsilon7' reserved variable."));
      return eFalse;
    }
    v->SetFloat(niEpsilon7);
  }

  {
    Ptr<iExpressionVariable> v = CreateVariable("epsilon8",eExpressionVariableType_Float,eExpressionVariableFlags(eExpressionVariableFlags_Reserved|eExpressionVariableFlags_Constant));
    if (!AddVariable(v)) {
      niError(_A("Can't add 'epsilon8' reserved variable."));
      return eFalse;
    }
    v->SetFloat(niEpsilon8);
  }

  {
    Ptr<iExpressionVariable> v = CreateVariable("epsilon9",eExpressionVariableType_Float,eExpressionVariableFlags(eExpressionVariableFlags_Reserved|eExpressionVariableFlags_Constant));
    if (!AddVariable(v)) {
      niError(_A("Can't add 'epsilon9' reserved variable."));
      return eFalse;
    }
    v->SetFloat(niEpsilon9);
  }

  // Register the op codes
  AddOp(OperatorAdd);
  AddOp(OperatorPlus);
  AddOp(OperatorSub);
  AddOp(OperatorMinus);
  AddOp(OperatorMul);
  AddOp(OperatorDiv);
#ifdef USE_MODULO_OP
  AddOp(OperatorMod);
#endif
  AddOp(OperatorBitNegate);
  AddOp(OperatorBitOr);
  AddOp(OperatorBitXor);
  AddOp(OperatorBitAnd);

  AddOp(OperatorLogicalAnd);
  AddOp(OperatorLogicalOr);
  AddOp(OperatorLogicalNot);
  AddOp(OperatorLogicalToBool);
  AddOp(OperatorCmpEq);
  AddOp(OperatorCmpNotEq);
  AddOp(OperatorCmpLt);
  AddOp(OperatorCmpLte);
  AddOp(OperatorCmpGt);
  AddOp(OperatorCmpGte);

  AddOp(Mod);
  AddOp(Rotate);
  AddOp(Vec2);
  AddOp(Vec3);
  AddOp(RGB);
  AddOp(Vec4);
  AddOp(Rect);
  AddOp(RGBA);
  AddOp(Quat);
  AddOp(Matrix);

  AddOp(ToReal);
  AddOp(ToFloat);
  AddOp(ToBool);
  AddOp(ToVec2);
  AddOp(ToVec3);
  AddOp(ToRGB);
  AddOp(ToVec4);
  AddOp(ToRGBA);
  AddOp(ToQuat);
  AddOp(ToMatrix);
  AddOp(ToString);
  AddOp(GetTypeString);

  AddOp(IsFloat);
  AddOp(IsVec2);
  AddOp(IsVec3);
  AddOp(IsVec4);
  AddOp(IsMatrix);
  AddOp(IsString);

  AddOp(GetX);
  AddOp(GetY);
  AddOp(GetZ);
  AddOp(GetW);
  AddOp(GetWidth);
  AddOp(GetHeight);

  AddOp(Sin);
  AddOp(ASin);
  AddOp(Cos);
  AddOp(ACos);
  AddOp(Tan);
  AddOp(ATan);
  AddOp(Rad);
  AddOp(Deg);
  AddOp(Floor);
  AddOp(Ceil);
  AddOp(Abs);
  AddOp(Sign);
  AddOp(Lerp);
  AddOp(Clamp);
  AddOp(Sat);
  AddOp(Sqrt);
  AddOp(Dot2);
  AddOp(Dot3);
  AddOp(Dot4);
  AddOp(Len2);
  AddOp(Len3);
  AddOp(Len4);
  AddOp(Cross3);
  AddOp(Cross4);
  AddOp(Normalize2);
  AddOp(Normalize3);
  AddOp(Normalize4);
  AddOp(Min);
  AddOp(Max);
  AddOp(Rand);
  AddOp(Rand2);
  AddOp(Pow);
  AddOp(Mul);
  AddOp(Bit);
  AddOp(LShift);
  AddOp(RShift);
  AddOp(MatrixRotationX);
  AddOp(MatrixRotationY);
  AddOp(MatrixRotationZ);
  AddOp(MatrixRotationAxis);
  AddOp(MatrixRotationQuat);
  AddOp(MatrixRotationYPR);
  AddOp(MatrixTranslation);
  AddOp(MatrixScale);
  AddOp(MatrixIdentity);
  AddOp(MatrixInverse);

  AddOp(QuatIdentity);
  AddOp(QuatBaryCentric);
  AddOp(QuatConjugate);
  AddOp(QuatExp);
  AddOp(QuatInverse);
  AddOp(QuatLn);
  AddOp(QuatMul);
  AddOp(QuatRotationAxis);
  AddOp(QuatRotationX);
  AddOp(QuatRotationY);
  AddOp(QuatRotationZ);
  AddOp(QuatRotationXYZ);
  AddOp(QuatRotationMatrix);
  AddOp(QuatRotationYPR);
  AddOp(QuatRotationVector);
  AddOp(QuatSlerp);
  AddOp(QuatSquad);
  AddOp(QuatToAxisAngle);
  AddOp(QuatToEuler);

  AddOp(Inc);
  AddOp(IncWrap);
  AddOp(Dec);
  AddOp(DecWrap);
  AddOp(Step);
  AddOp(StepWrap);
  AddOp(StepBack);
  AddOp(StepBackWrap);
  AddOp(SinWave);
  AddOp(CosWave);
  AddOp(TriangleWave);
  AddOp(SquareWave);
  AddOp(SawWave);
  AddOp(InvSawWave);

  AddOp(StrLen);
  AddOp(StrSlice);
  AddOp(StrMid);
  AddOp(StrLeft);
  AddOp(StrRight);
  AddOp(StrBefore);
  AddOp(StrRBefore);
  AddOp(StrAfter);
  AddOp(StrRAfter);
  AddOp(StrToLower);
  AddOp(StrToUpper);
  AddOp(StrTrim);
  AddOp(StrNormalize);
  AddOp(StrFind);
  AddOp(StrRFind);

  AddOp(Format);
  AddOp(FormatTimeSeconds);
  AddOp(FormatCurrentTime);
  AddOp(DigestHex);
  AddOp(KDFGenSaltBlowfish);
  AddOp(KDFCrypt);

  AddOp(Get);
  AddOp(Set);
  AddOp(If);
  AddOp(Eval);
  AddOp(Global);

  AddOp(DTGet);
  AddOp(DTGetChild);
  AddOp(DTFindChild);
  AddOp(DTCopyMatch);
  AddOp(DTGetChildIndex);
  AddOp(DTGetNumChildren);
  AddOp(DTGetName);
  AddOp(DTGetIndex);
  AddOp(DTArray);
  AddOp(DTObject);
  AddOp(DTToJson);
  AddOp(DTFromJson);
  AddOp(DTFromPath);

  AddOp(ArrRangeI);
  AddOp(ArrMonthDays);
  AddOp(ArrWeekDays);
  AddOp(ArrMonths);
  AddOp(ArrSum);
  AddOp(ArrAvg);
  AddOp(ArrMax);
  AddOp(ArrMin);
  AddOp(ArrAdd);
  AddOp(ArrValues);

  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall Evaluator::IsOK() const
{
  return mbOK;
}

///////////////////////////////////////////////
//! Add an operation.
tBool Evaluator::AddOperation(Op* apOperation)
{
  if (!niIsOK(apOperation)) {
    niError(_A("Invalid operation pointer."));
    return eFalse;
  }

  if (apOperation->GetType() == eMathOperationType_Operator) {
    astl::upsert(mmapOperators, apOperation->GetName(), apOperation);
  }
  else if (apOperation->GetType() == eMathOperationType_UnaryOperator) {
    astl::upsert(mmapUnaryOperators, apOperation->GetName(), apOperation);
  }
  else if (apOperation->GetType() == eMathOperationType_Function) {
    astl::upsert(mmapFunctions, apOperation->GetName(), apOperation);
  }

  return eTrue;
}

///////////////////////////////////////////////
tBool Evaluator::AddEnumDef(const sEnumDef* apEnumDef)
{
  if (!apEnumDef) return eFalse;
  astl::upsert(mmapEnums,apEnumDef->maszName,apEnumDef);
  return eTrue;
}

///////////////////////////////////////////////
iExpressionVariable* Evaluator::CreateVariable(const achar* aaszName, eExpressionVariableType aType, tExpressionVariableFlags aFlags)
{
  return _CreateVariable(aaszName,aType,aFlags);
}

///////////////////////////////////////////////
iExpressionVariable* Evaluator::CreateVariableFromRunnable(const achar* aaszName, eExpressionVariableType aType, iRunnable* apRunnable, tExpressionVariableFlags aFlags)
{
  niCheckIsOK(apRunnable,NULL);
  Ptr<iExpressionVariable> ptrVar = niNew ExprVarFromRunnable(_H(aaszName),aType,apRunnable);
  ((ExprVar*)(ptrVar.ptr()))->Flags = aFlags;
  return ptrVar.GetRawAndSetNull();
}

///////////////////////////////////////////////
iExpressionVariable* Evaluator::CreateVariableFromVar(const achar* aaszName, const Var& aVar, tExpressionVariableFlags aFlags)
{
  eExpressionVariableType type;
  const tType varType = aVar.GetType();
  switch (niType(varType)) {
    case eType_Vec2i:
    case eType_Vec2f: {
      type = eExpressionVariableType_Vec2;
      break;
    }
    case eType_Vec3i:
    case eType_Vec3f: {
      type = eExpressionVariableType_Vec3;
      break;
    }
    case eType_Vec4i:
    case eType_Vec4f: {
      type = eExpressionVariableType_Vec4;
      break;
    }
    case eType_Matrixf: {
      type = eExpressionVariableType_Matrix;
      break;
    }
    case eType_IUnknown: {
      type = ni::eExpressionVariableType_IUnknown;
      break;
    }
    default:
      if (VarIsIntType(varType) ||
          VarIsFloatType(varType)) {
        type = eExpressionVariableType_Float;
      }
      else if (VarIsString(aVar)) {
        type = eExpressionVariableType_String;
      }
      else {
        // Default to float
        type = eExpressionVariableType_Float;
      }
      break;
  }

  Ptr<ExprVar> ptrVar = _CreateVariable(aaszName,type,aFlags);
  if (!_FromVar(ptrVar,aVar)) {
    EXPRESSION_TRACE(niFmt("Can't convert variant value to variable."));
    return NULL;
  }
  ((ExprVar*)(ptrVar.ptr()))->Flags = aFlags;
  return ptrVar.GetRawAndSetNull();
}

///////////////////////////////////////////////
iExpressionVariable* Evaluator::CreateVariableFromExpr(const achar* aaszName, const achar* aaszExpr, tExpressionVariableFlags aFlags)
{
  Ptr<Op> ptrOp =  ParseExpr(aaszExpr);
  if (!niIsOK(ptrOp)) {
    niError(niFmt(_A("Can't parse '%s'."), aaszExpr));
    return NULL;
  }

  iExpressionVariable* pNew = ptrOp->Eval(this);
  if (!pNew) {
    niError(niFmt(_A("Can't evaluate '%s'."), aaszExpr));
    return NULL;
  }

  pNew = pNew->Clone();
  pNew->SetName(_H(aaszName));
  return pNew;
}

///////////////////////////////////////////////
//! Tokenize the given string and output the result in the given token vector.
tBool Evaluator::TokenizeExpr(const achar* aaszExpr, tMathExprTokenVec& avOut)
{
  Var varNum;
  cString strNum;
  tI32 nGroupDepth = 0;
  cMathEqTokenizer tokens(aaszExpr);

  const cString* nextTok = tokens.GetNextToken();
  if (!nextTok)
	  return eFalse;

  while (1)
  {
    if (!nextTok)
      break;

    const cString& tok = *nextTok;
    nextTok = tokens.GetNextToken();

    sMathExprToken token;
    // niDebugFmt(("TOK: %s",tok));

    if (tok == _A("(") || tok == _A("["))
    {
      token.Type = eMathExprTokenType_OpenGroup;
      token.nGroupDepth = nGroupDepth++;
    }
    else if (tok == _A(")") || tok == _A("]"))
    {
      token.Type = eMathExprTokenType_CloseGroup;
      token.nGroupDepth = --nGroupDepth;
    }
    else if (tok == _A(","))
    {
      token.Type = eMathExprTokenType_Separator;
    }
    else if (_IsStringDelimiterChar(tok[0])) {
      char toTrim[2] = {tok[0],0};
      token.Type = eMathExprTokenType_String;
      token.strToken = tok;
      token.strToken.TrimEx(toTrim);
    }
    else if (tok.contains("://")) {
      token.Type = eMathExprTokenType_URL;
    }
    else if (tok == _A("+") || tok == _A("-") || tok == _A("*") || tok == _A("/") ||
#ifdef USE_MODULO_OP
             tok == _A("%") ||
#endif
             tok == _A("!") || tok == _A("|") || tok == _A("^") ||
             tok == _A("&") || tok == _A("~") ||
             tok == _A("&&") || tok == _A("||") ||
             tok == _A("!") || tok == _A("!!") ||
             tok == _A("==") || tok == _A("!=") ||
             tok == _A(">") || tok == _A(">=") ||
             tok == _A("<") || tok == _A("<="))
    {
      if (avOut.empty() ||
          (avOut.back().Type != eMathExprTokenType_Unknown &&
           avOut.back().Type != eMathExprTokenType_Float &&
           avOut.back().Type != eMathExprTokenType_String &&
           avOut.back().Type != eMathExprTokenType_URL &&
           avOut.back().Type != eMathExprTokenType_CloseGroup))
      {
        token.Type = eMathExprTokenType_UnaryOperator;
        if (!GetUnaryOperator(tok.Chars())) {
          niError(niFmt(_A("Can't find operation for modifier '%s'."), tok.Chars()));
          return eFalse;
        }
      }
      else
      {
        token.Type = eMathExprTokenType_Operator;
        if (!GetOperator(tok.Chars())) {
          niError(niFmt(_A("Can't find operation for operator '%s'."), tok.Chars()));
          return eFalse;
        }
      }
    }
    else if (tok[0] == _A('.') || ReadNumber(tok.Chars(),varNum) > eNumberType_Error)
    {
      if (tok[0] == _A('.')) {
        if (!nextTok) {
          niError(_A("Isolated '.' found."));
          return eFalse;
        }
        strNum << _A(".") << *nextTok;
      }
      else if (nextTok && *nextTok == _A(".")) {
        strNum << tok << _A(".");
        nextTok = tokens.GetNextToken();
        if (nextTok) {
          strNum << *nextTok;
        }
      }
      if (strNum.IsNotEmpty()) {
        if (ReadNumber(strNum.Chars(),varNum) <= eNumberType_Error) {
          niError(niFmt(_A("Can't read number '%s'."),strNum.Chars()));
          return eFalse;
        }
        strNum.Clear(64);
        nextTok = tokens.GetNextToken(); // prepare the next token
      }

      token.Type = eMathExprTokenType_Float;
      if (varNum.GetType() == eType_U64) {
        token.fFloat = varNum.mU64;
      }
      else if (varNum.GetType() == eType_I64) {
        token.fFloat = varNum.mI64;
      }
      else if (varNum.GetType() == eType_F64) {
        token.fFloat = varNum.mF64;
      }

#ifndef USE_MODULO_OP
      if (nextTok && *nextTok == _A("%")) {
        token.fFloat /= 100.0f;
        varNum.mF32 /= 100.0f;
        nextTok = tokens.GetNextToken(); // prepare the next token
      }
#endif
    }
    else
    {
      if (nextTok && (*nextTok)[0] == _A('(') && GetFunction(tok.Chars())) {
        token.Type = eMathExprTokenType_Function;
      }
      else {
        const sEnumDef* pEnumDef =
            (tok[0] == 'e' && (tok[1] >= 'A' && tok[1] <= 'Z')) ?
            GetEnumDef(tok.Chars()) : NULL;
        if (pEnumDef) {
          if (!nextTok || *nextTok != _A(".")) {
            niError(niFmt(_A("Dot ('.') expected after enumeration '%s'."),pEnumDef->maszName));
            return eFalse;
          }

          nextTok = tokens.GetNextToken();
          if (!nextTok) {
            niError(niFmt(_A("Unexpected end of expression, enum value key expected to index enumeration '%s'."),pEnumDef->maszName));
            return eFalse;
          }

          token.strToken << pEnumDef->maszName << _A(".") << nextTok->Chars();

          tU32 i, nValue = 0;
          for (i = 0; i < pEnumDef->mnNumValues; ++i) {
            if (ni::StrEq(nextTok->Chars(),pEnumDef->mpValues[i].maszName)) {
              nValue = pEnumDef->mpValues[i].mnValue;
              break;
            }
          }
          if (i == pEnumDef->mnNumValues) {
            niError(niFmt(_A("Can't find element '%s' in enumeration '%s'."),
                          nextTok->Chars(),pEnumDef->maszName));
            return eFalse;
          }

          token.Type = eMathExprTokenType_Float;
          token.fFloat = nValue;
          nextTok = tokens.GetNextToken();
        }
        else if (mpDefaultEnum) {
          pEnumDef = mpDefaultEnum;
          tU32 i, nValue = 0;
          for (i = 0; i < pEnumDef->mnNumValues; ++i) {
            if (ni::StrEq(tok.Chars(),pEnumDef->mpValues[i].maszName)) {
              nValue = pEnumDef->mpValues[i].mnValue;
              break;
            }
          }

          if (i == pEnumDef->mnNumValues) {
            token.Type = eMathExprTokenType_Unknown;
          }
          else {
            token.strToken = pEnumDef->maszName;
            token.strToken += _A(".");
            token.strToken += tok.Chars();
            token.Type = eMathExprTokenType_Float;
            token.fFloat = nValue;
          }
        }
        else {
          token.Type = eMathExprTokenType_Unknown;
        }
      }
    }

    if (token.strToken.IsEmpty())
      token.strToken = tok;

    // niDebugFmt(("TOKEN[%d]: %d, %s, %g, %d", avOut.size(), token.Type, token.strToken, token.fFloat, token.nGroupDepth));
    avOut.push_back(token);
  }

  return eTrue;
}

///////////////////////////////////////////////
//! Parse the given string and return the result.
Ptr<Op> Evaluator::ParseExpr(const achar* aaszExpr)
{
  if (!niStringIsOK(aaszExpr)) {
    aaszExpr = _A("0");
  }
  else if (*aaszExpr == _A(':')) {
    // : marks the beginning of an expression that can be collapsed, aka that dont
    // need to keep the same string. This is a convention used by the
    // scripting languages, so for simplicity the character is considered valid
    // at the beginning of an expression, and ignored.
    while (*aaszExpr == _A(':')) {
      ++aaszExpr;
    }
  }

  tMathExprTokenVec vToks;
  vToks.reserve(128);

  if (!TokenizeExpr(aaszExpr,vToks)) {
    niError(niFmt(_A("Can't tokenize expression '%s'."), aaszExpr));
    return NULL;
  }

  Ptr<Op> ptrRet = niNew OpGroup();
  if (!niIsOK(ptrRet)) {
    niError(_A("Can't create root group operation."));
    return NULL;
  }

  tU32 nCurrentOperand = 0;
  tMathExprTokenVec::iterator itTok = vToks.begin();
  Ptr<Op> processedToken = _ProcessToken(ptrRet, nCurrentOperand, vToks, itTok);
  if (processedToken != ptrRet) {
    niError(_A("Can't process the tokens."));
    return NULL;
  }

  return ptrRet;
}

///////////////////////////////////////////////
Ptr<Op> Evaluator::_ProcessToken(Op* apCurrentOp, tU32& anCurrentOperand, tMathExprTokenVec& avToks, tMathExprTokenVec::iterator& aitTok)
{
  Ptr<iExpressionVariable> ptrVar;
  Ptr<Op> ptrOp;
  //while (1)
  {
    const sMathExprToken& token = *aitTok;
    switch (token.Type) {
      case eMathExprTokenType_Unknown: {
        // Unkown token, probably a variable.
        ptrVar = FindVariable(_H(token.strToken));
        if (!niIsOK(ptrVar)) {
          // if there's an error return 0 as result so that we can always
          // safely 'Eval()->Get'
          niWarning(niFmt("Can't find variable '%s'.", token.strToken.Chars()));
          ptrVar = niNew ExprVarFloat(_H(AZEROSTR));
          ((ExprVar*)(ptrVar.ptr()))->Flags |= eExpressionVariableFlags_Constant;
          ptrVar->SetFloat(0.0f);
        }
        break;
      }
      case eMathExprTokenType_Float: {
        // Real number token.
        ptrVar = niNew ExprVarFloat(_H(AZEROSTR));
        ((ExprVar*)(ptrVar.ptr()))->Flags |= eExpressionVariableFlags_Constant;
        ptrVar->SetFloat(token.fFloat);
        break;
      }
      case eMathExprTokenType_String: {
        // String token.
        ptrVar = niNew ExprVarString(_H(AZEROSTR));
        ((ExprVar*)(ptrVar.ptr()))->Flags |= eExpressionVariableFlags_Constant;
        ptrVar->SetString(token.strToken);
        break;
      }
      case eMathExprTokenType_Operator: {
        // Operator token.
        niError(niFmt(_A("Operator syntax error for operator '%s'."), token.strToken.Chars()));
        return NULL;
      }
      case eMathExprTokenType_UnaryOperator: {
        // UnaryOperator token.
        if ((aitTok+1) == avToks.end())
        {
          niError(niFmt(_A("Unexpected end of tokens when processing tokens of unary operator '%s'."), token.strToken.Chars()));
          return NULL;
        }

        ptrOp = _CreateOperation(token.strToken.Chars(),token.Type);
        niCheck(ptrOp.IsOK(),NULL);

        tU32 nCurrentOperand = 0;
        ptrOp = _ProcessToken(ptrOp,nCurrentOperand,avToks,++aitTok);
        if (!ptrOp.IsOK()) {
          niError(niFmt(_A("Can't process tokens of unary operator '%s'."), token.strToken.Chars()));
          return NULL;
        }
        break;
      }
      case eMathExprTokenType_Function: {
        // Function token.
        if (++aitTok == avToks.end()) {
          niError(niFmt(_A("Unexpected end of tokens when processing tokens of function '%s'."), token.strToken.Chars()));
          return NULL;
        }

        // Expect OpenGroup
        if (aitTok->Type != eMathExprTokenType_OpenGroup) {
          niError(niFmt(_A("OpenGroup expected after function '%s'."), token.strToken.Chars()));
          return NULL;
        }

        // skip 'OpenGroup'
        if (++aitTok == avToks.end()) {
          niError(niFmt(_A("Unexpected end of tokens when processing tokens of function '%s' after OpenGroup."), token.strToken.Chars()));
          return NULL;
        }

        ptrOp = _CreateOperation(token.strToken.Chars(),token.Type);
        niCheck(ptrOp.IsOK(),NULL);

        tU32 nCurrentOperand = 0;
        if (ptrOp->IsVarNumOperands()) {

          tU32 i = 0;
          if (aitTok->Type != eMathExprTokenType_CloseGroup)
            while (1) {
              Ptr<Op> processedOp = _ProcessToken(ptrOp,nCurrentOperand,avToks,aitTok);
              if (processedOp != ptrOp) {
                niError(niFmt(_A("Can't process tokens of parameter %d of function '%s'."), i, token.strToken.Chars()));
                return NULL;
              }

              ++aitTok;
              if (aitTok == avToks.end()) {
                niError(niFmt(_A("Unexpected end of tokens after processing parameter %d of function '%s'."), i, token.strToken.Chars()));
                return NULL;
              }

              if (aitTok->Type == eMathExprTokenType_CloseGroup) {
                break;
              }

              if (aitTok->Type != eMathExprTokenType_Separator) {
                niError(niFmt(_A("Separator expected after parameter %d of function '%s'."), i, token.strToken.Chars()));
                return NULL;
              }

              ++aitTok;
              if (aitTok == avToks.end()) {
                niError(niFmt(_A("Unexpected end of tokens after processing separator of parameter %d of function '%s'."), i, token.strToken.Chars()));
                return NULL;
              }

              ++i;
            }
        }
        else if (ptrOp->GetNumOperands() == 0) {
          if (aitTok->Type != eMathExprTokenType_CloseGroup) {
            niError(niFmt(_A("CloseGroup expected after OpenGroup for zero parameters function '%s'."), token.strToken.Chars()));
            return NULL;
          }
        }
        else {
          for (tU32 i = 0; i < ptrOp->GetNumOperands(); ++i)
          {
            if (aitTok == avToks.end())
            {
              niError(niFmt(_A("Unexpected end of tokens when processing parameter %d of function '%s'."), i, token.strToken.Chars()));
              return NULL;
            }

            Ptr<Op> processedOp = _ProcessToken(ptrOp,nCurrentOperand,avToks,aitTok);
            if (processedOp != ptrOp) {
              niError(niFmt(_A("Can't process tokens of parameter %d of function '%s'."), i, token.strToken.Chars()));
              return NULL;
            }

            if ((aitTok+1) == avToks.end())
            {
              niError(niFmt(_A("Unexpected end of tokens after processing parameter %d of function '%s'."), i, token.strToken.Chars()));
              return NULL;
            }

            if (i+1 == ptrOp->GetNumOperands())
            {
              if ((aitTok+1)->Type != eMathExprTokenType_CloseGroup)
              {
                niError(niFmt(_A("CloseGroup expected after last parameter %d of function '%s'."), i, token.strToken.Chars()));
                return NULL;
              }
              ++aitTok;
            }
            else
            {
              if ((aitTok+1)->Type != eMathExprTokenType_Separator)
              {
                niError(niFmt(_A("Separator expected after parameter %d of function '%s'."), i, token.strToken.Chars()));
                return NULL;
              }
              aitTok += 2;
            }
          }
        }
        break;
      }
      case eMathExprTokenType_OpenGroup: {
        // Left paranthesis token.
        if ((aitTok+1) == avToks.end()) {
          niError(niFmt(_A("Unexpected end of tokens when processing group - '%s'."), token.strToken.Chars()));
          return NULL;
        }
        ++aitTok;

        ptrOp = niNew OpGroup();

        tU32 nCurrentOperand = 0;
        Ptr<Op> processedOp = _ProcessToken(ptrOp, nCurrentOperand, avToks, aitTok);
        if (processedOp != ptrOp) {
          niError(_A("Can't parse the group's tokens."));
          return NULL;
        }

        if ((aitTok+1) == avToks.end())
        {
          niError(niFmt(_A("Unexpected end of tokens when processing group, close group expected - '%s'."), token.strToken.Chars()));
          return NULL;
        }

        if ((aitTok+1)->Type != eMathExprTokenType_CloseGroup)
        {
          niError(niFmt(_A("CloseGroup expected at the end of a group - '%s'."), token.strToken.Chars()));
          return NULL;
        }
        ++aitTok;
        break;
      }
      case eMathExprTokenType_CloseGroup: {
        // Right paranthesis token.
        niError(niFmt(_A("Unexpected close group - '%s'."), token.strToken.Chars()));
        return NULL;
      }
      case eMathExprTokenType_Separator: {
        // Separator token.
        niError(niFmt(_A("Unexpected separator - '%s'."), token.strToken.Chars()));
        return NULL;
      }
      case eMathExprTokenType_URL: {
        cString protocol = token.strToken.Before("://");
        Ptr<iExpressionURLResolver> resolver = FindURLResolver(protocol.Chars());
        if (!resolver.IsOK()) {
          niError(niFmt(_A("Can't find resolver for URL protocol '%s' : '%s'."), protocol, token.strToken));
          return NULL;
        }

        ptrOp = niNew OpURL(resolver,token.strToken.Chars());
        if (!ptrOp.IsOK()) {
          niError(niFmt(_A("Resolver '%s' couldnt create URL resolver operation : '%s'."),
                        protocol, token.strToken));
          return NULL;
        }
        break;
      }
      default: {
        niError(niFmt(_A("Unknown token '%d' (%s, %g, %d)."), token.Type, token.strToken, token.fFloat, token.nGroupDepth));
        return NULL;
      }
    }
  }

  enum {
    ModeProceed,
    ModeSaveOperationAndProceed,
    ModeSetOperand
  };

  int mode = ModeProceed;
  switch (apCurrentOp->GetType())
  {
    case eMathOperationType_UnaryOperator:
      {
        mode = ModeSetOperand;
        break;
      }

    case eMathOperationType_Operator:
      {
        mode = ModeSaveOperationAndProceed;
        break;
      }
  }

  if (ptrOp.IsOK())
  {
    if (mode != ModeSetOperand &&
        (aitTok+1) != avToks.end() &&
        (aitTok+1)->Type == eMathExprTokenType_Operator)
    {
      if (mode == ModeSaveOperationAndProceed)
      {
        if (!apCurrentOp->SetOperandOperation(anCurrentOperand++,ptrOp))
        {
          niError(niFmt(_A("Can't set operation operand '%d'."), anCurrentOperand-1));
          return NULL;
        }

        if (anCurrentOperand != apCurrentOp->GetNumOperands())
        {
          niError(_A("Not all operands set before proceeding to a new operation."));
          return NULL;
        }

        ptrOp = apCurrentOp;
      }

      ++aitTok;
      sMathExprToken& nexttoken = *aitTok;
      ++aitTok;
      if (aitTok == avToks.end())
      {
        niError(niFmt(_A("Unexpected end of expression when processing operator '%s'."), nexttoken.strToken.Chars()));
        return NULL;
      }

      Ptr<Op> ptrNewOp = _CreateOperation(nexttoken.strToken.Chars(),nexttoken.Type);
      niCheck(ptrNewOp.IsOK(),NULL);
      ptrNewOp->SetOperandOperation(0,ptrOp);

      tU32 nCurrentOperand = 1;
      ptrNewOp = _ProcessToken(ptrNewOp,nCurrentOperand,avToks,aitTok);
      if (!ptrNewOp.IsOK()) {
        niError(niFmt(_A("Can't process token for operator '%s'."), nexttoken.strToken.Chars()));
        return NULL;
      }

      if (mode != ModeSaveOperationAndProceed)
      {
        if (!apCurrentOp->SetOperandOperation(anCurrentOperand++,ptrNewOp))
        {
          niError(niFmt(_A("Can't new operation operand '%d'."), anCurrentOperand-1));
          return NULL;
        }
      }
      else
      {
        return ptrNewOp;
      }
    }
    else
    {
      if (!apCurrentOp->SetOperandOperation(anCurrentOperand++,ptrOp))
      {
        niError(niFmt(_A("Can't set operation operand '%d'."), anCurrentOperand-1));
        return NULL;
      }
    }
  }
  else if (ptrVar.IsOK())
  {
    if (mode != ModeSetOperand &&
        (aitTok+1) != avToks.end() &&
        (aitTok+1)->Type == eMathExprTokenType_Operator)
    {
      if (mode == ModeSaveOperationAndProceed)
      {
        if (!apCurrentOp->SetOperandVariable(anCurrentOperand++,ptrVar))
        {
          niError(niFmt(_A("Can't set operation operand '%d'."), anCurrentOperand-1));
          return NULL;
        }

        if (anCurrentOperand != apCurrentOp->GetNumOperands())
        {
          niError(_A("Not all operands set before proceeding to a new operation."));
          return NULL;
        }

        ptrOp = apCurrentOp;

        ++aitTok;
        sMathExprToken& nexttoken = *aitTok;
        ++aitTok;
        if (aitTok == avToks.end())
        {
          niError(niFmt(_A("Unexpected end of expression when processing operator '%s'."), nexttoken.strToken.Chars()));
          return NULL;
        }

        Ptr<Op> ptrNewOp = _CreateOperation(nexttoken.strToken.Chars(),nexttoken.Type);
        niCheck(ptrNewOp.IsOK(),NULL);
        ptrNewOp->SetOperandOperation(0,ptrOp);

        tU32 nCurrentOperand = 1;
        ptrNewOp = _ProcessToken(ptrNewOp,nCurrentOperand,avToks,aitTok);
        if (!ptrNewOp.IsOK()) {
          niError(niFmt(_A("Can't process token for operator '%s'."), nexttoken.strToken.Chars()));
          return NULL;
        }

        return ptrNewOp;
        //       if (!apCurrentOp->SetOperandOperation(anCurrentOperand++,ptrNewOp))
        //    {
        //         niError(niFmt(_A("Can't new operation operand '%d'."), anCurrentOperand-1));
        //         return eFalse;
        //       }
      }
      else
      {
        ++aitTok;
        sMathExprToken& nexttoken = *aitTok;
        ++aitTok;
        if (aitTok == avToks.end())
        {
          niError(niFmt(_A("Unexpected end of expression when processing operator '%s'."), nexttoken.strToken.Chars()));
          return NULL;
        }

        Ptr<Op> ptrNewOp = _CreateOperation(nexttoken.strToken.Chars(),nexttoken.Type);
        niCheck(ptrNewOp.IsOK(),NULL);
        ptrNewOp->SetOperandVariable(0,ptrVar);

        tU32 nCurrentOperand = 1;
        ptrNewOp = _ProcessToken(ptrNewOp,nCurrentOperand,avToks,aitTok);
        if (!ptrNewOp.IsOK()) {
          niError(niFmt(_A("Can't process token for operator '%s'."), nexttoken.strToken.Chars()));
          return NULL;
        }

        if (!apCurrentOp->SetOperandOperation(anCurrentOperand++,ptrNewOp))
        {
          niError(niFmt(_A("Can't new operation operand '%d'."), anCurrentOperand-1));
          return NULL;
        }
      }
    }
    else
    {
      if (!apCurrentOp->SetOperandVariable(anCurrentOperand++,ptrVar))
      {
        niError(niFmt(_A("Can't set variable operand '%d'."), anCurrentOperand-1));
        return NULL;
      }
    }
  }

  return apCurrentOp;
}

///////////////////////////////////////////////
tBool Evaluator::GetUnknownSymbols(const achar* aaszExpr, tStringCVec* apList)
{
  if (!tStringCVec::IsSameType(apList)) {
    niError(_A("Invalid output list."));
    return eFalse;
  }

  tStringCVec* pList = (tStringCVec*)apList;

  tMathExprTokenVec vToks;
  vToks.reserve(128);

  if (!TokenizeExpr(aaszExpr,vToks)) {
    niError(niFmt(_A("Can't tokenize expression '%s'."), aaszExpr));
    return eTrue;
  }

  for (tMathExprTokenVec::iterator itTok = vToks.begin(); itTok != vToks.end(); ++itTok) {
    sMathExprToken& token = *itTok;
    if (token.Type == eMathExprTokenType_Unknown) {
      pList->push_back(token.strToken);
    }
  }

  return eTrue;
}

///////////////////////////////////////////////
Op* __stdcall Evaluator::GetOperator(const achar* aaszName) const
{
  if (!mmapOperators.empty())
  {
    tOperationMapCIt it = mmapOperators.find(aaszName);
    if (it != mmapOperators.end())
      return it->second;
  }
  if (mptrParent.IsOK())
    return mptrParent->GetOperator(aaszName);
  return NULL;
}

///////////////////////////////////////////////
Op* __stdcall Evaluator::GetUnaryOperator(const achar* aaszName) const
{
  if (!mmapUnaryOperators.empty())
  {
    tOperationMapCIt it = mmapUnaryOperators.find(aaszName);
    if (it != mmapUnaryOperators.end())
      return it->second;
  }
  if (mptrParent.IsOK())
    return mptrParent->GetUnaryOperator(aaszName);
  return NULL;
}

///////////////////////////////////////////////
Op* __stdcall Evaluator::GetFunction(const achar* aaszName) const
{
  if (!mmapFunctions.empty())
  {
    cString strLower = aaszName;
    strLower.ToLower();
    tOperationMapCIt it = mmapFunctions.find(strLower.Chars());
    if (it != mmapFunctions.end())
      return it->second;
  }
  if (mptrParent.IsOK())
    return mptrParent->GetFunction(aaszName);
  return NULL;
}

///////////////////////////////////////////////
const sEnumDef* Evaluator::GetEnumDef(const achar* aaszName) const
{
  tEnumMap::const_iterator it = mmapEnums.find(aaszName);
  if (it != mmapEnums.end())
    return it->second;
  if (mbGlobalEnumSearch) {
    const sEnumDef* pEnumDef = ni::GetLang()->GetEnumDef(aaszName);
    if (pEnumDef) {
      const_cast<Evaluator*>(this)->AddEnumDef(pEnumDef);
      return pEnumDef;
    }
  }
  return NULL;
}

///////////////////////////////////////////////
const achar* __stdcall Evaluator::GetEnumName(tU32 anIndex) const {
  if (anIndex >= mmapEnums.size())
    return AZEROSTR;
  tU32 i = 0;
  niLoopit(tEnumMap::const_iterator,it,mmapEnums) {
    if (i++ == anIndex)
      return it->first.Chars();
  }
  return AZEROSTR;
}

///////////////////////////////////////////////
void __stdcall Evaluator::SetDefaultEnumDef(const sEnumDef* apEnumDef) {
  mpDefaultEnum = apEnumDef;
  if (mpDefaultEnum) {
    tEnumMap::const_iterator it = mmapEnums.find(mpDefaultEnum->maszName);
    if (it == mmapEnums.end()) {
      AddEnumDef(mpDefaultEnum);
    }
  }
}
const sEnumDef* __stdcall Evaluator::GetDefaultEnumDef() const {
  return mpDefaultEnum;
}

///////////////////////////////////////////////
void __stdcall Evaluator::SetGlobalEnumSearch(tBool abEnabled)
{
  mbGlobalEnumSearch = abEnabled;
}

///////////////////////////////////////////////
tBool __stdcall Evaluator::GetGlobalEnumSearch() const
{
  return mbGlobalEnumSearch;
}

///////////////////////////////////////////////
//! Create an operation.
Ptr<Op> Evaluator::_CreateOperation(const achar* aaszName, eMathExprTokenType aOpType)
{
  Ptr<Op> ptrOp = NULL;
  switch (aOpType) {
    case eMathExprTokenType_Operator: {
      ptrOp = GetOperator(aaszName);
      break;
    }
    case eMathExprTokenType_UnaryOperator: {
      ptrOp = GetUnaryOperator(aaszName);
      break;
    }
    case eMathExprTokenType_Function: {
      ptrOp = GetFunction(aaszName);
      break;
    }
    default: {
      niAssertUnreachable("Invalid operation type.");
      return NULL;
    }
  }

  if (!ptrOp.IsOK()) {
    niError(niFmt("Couldn't find the operation '%s'", aaszName));
    return NULL;
  }

  return ptrOp->Create();
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Undef macros
#undef BeginOpO
#undef BeginOpUO
#undef BeginOpF
#undef EndOp
#undef AddOp

//---------------------------------------------------------------------------
//
// cLang
//
//---------------------------------------------------------------------------
#include "Lang.h"

iExpressionContext* cLang::CreateExpressionContext() {
  return niNew Evaluator(NULL);
}

iExpressionContext* cLang::GetExpressionContext() const {
  if (!_ctx.IsOK()) {
    niThis(cLang)->_ctx = niNew Evaluator(NULL);
    _ctx->SetGlobalEnumSearch(eTrue);
  }
  return _ctx;
}

Ptr<iExpressionVariable> cLang::Eval(const achar* aaszExpr) {
  static Ptr<ExprVarString> _exprVarEvalError =
      niNew ExprVarString(_H("EVALERR"));
  Ptr<iExpressionContext> ptrCtx = GetExpressionContext();
  Ptr<iExpressionVariable> r = ptrCtx->Eval(aaszExpr);
  if (r.IsOK())
    return r;
  else
    return _exprVarEvalError.ptr();
}

tU32 cLang::StringToEnum(const achar* aExpr, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags) {
  Ptr<iExpressionContext> ptrCtx;
  if (apEnumDef) {
    ptrCtx = GetExpressionContext()->CreateContext();
    if (!ptrCtx.IsOK()) {
      return NULL;
    }
    ptrCtx->AddEnumDef(apEnumDef);
    if (!niFlagIs(aFlags,eEnumToStringFlags_Full)) {
      ptrCtx->SetDefaultEnumDef(apEnumDef);
    }
    if (niFlagIs(aFlags,eEnumToStringFlags_GlobalSearch)) {
      ptrCtx->SetGlobalEnumSearch(eTrue);
    }
  }
  else {
    ptrCtx = GetExpressionContext();
  }

  Ptr<iExpressionVariable> pVar = ptrCtx->Eval(aExpr);
  return niIsOK(pVar) ? (tU32)pVar->GetFloat() : 0;
}

cString cLang::EnumToString(tU32 anValue, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags) {
  if (apEnumDef) {
    Ptr<iExpressionContext> ptrCtx = GetExpressionContext()->CreateContext();
    if (!ptrCtx.IsOK()) {
      niWarning("Can't create expression context.");
    }
    else {
      ptrCtx->AddEnumDef(apEnumDef);
      if (!niFlagIs(aFlags,eEnumToStringFlags_Full)) {
        ptrCtx->SetDefaultEnumDef(apEnumDef);
      }
      if (niFlagIs(aFlags,eEnumToStringFlags_Flags)) {
        return ptrCtx->GetEnumFlagsString(anValue);
      }
      else {
        return ptrCtx->GetEnumValueString(anValue);
      }
    }
  }

  {
    // String to integer...
    cString str;
    str.Set((tI32)anValue);
    return str;
  }
}
