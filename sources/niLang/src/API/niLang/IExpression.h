#pragma once
#ifndef __IEXPRESSION_H_5FF07DF5_2A37_436B_BCF5_7C09A8AA989D__
#define __IEXPRESSION_H_5FF07DF5_2A37_436B_BCF5_7C09A8AA989D__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "Types.h"
#include "IConcurrent.h"

namespace ni {
struct iExpressionContext;

/** \addtogroup niLang
 * @{
 */

//! Expression variables types.
enum eExpressionVariableType
{
  //! Real number variable type.
  //! Constructors: -x, x, x.y, -x.y
  eExpressionVariableType_Float = 0,
  //! 2D vector variable type.
  //! Constructors: Vec2()
  eExpressionVariableType_Vec2 = 1,
  //! 3D vector variable type.
  //! Constructors: Vec3(), RGB()
  eExpressionVariableType_Vec3 = 2,
  //! 4D vector variable type.
  //! Constructors: Vec4(), RGBA(), Quat(), Plane()
  eExpressionVariableType_Vec4 = 3,
  //! Matrix variable type.
  //! Constructors: Matrix()
  eExpressionVariableType_Matrix = 4,
  //! String type.
  //! Constructors: "string", 'string'
  eExpressionVariableType_String = 5,
  //! \internal
  eExpressionVariableType_ForceDWORD = 0xFFFFFFFF
};

//! Expression variables flags.
enum eExpressionVariableFlags
{
  //! Default flags value.
  eExpressionVariableFlags_Default = 0,
  //! Variable is reserved.
  //! \remark Reserved variables cant be removed.
  eExpressionVariableFlags_Reserved = niBit(0),
  //! Constant variable, read-only.
  eExpressionVariableFlags_Constant = niBit(1),
  //! Variable is the result of a color function.
  eExpressionVariableFlags_Color = niBit(2),
  //! Variable is the result of a quaternion function.
  eExpressionVariableFlags_Quat = niBit(3),
  //! \internal
  eExpressionVariableFlags_ForceDWORD = 0xFFFFFFFF,
};

//! Expression variables flags type. \see ni::eExpressionVariableFlags
typedef tU32 tExpressionVariableFlags;

//! Expression variable interface.
struct iExpressionVariable : public iUnknown
{
  niDeclareInterfaceUUID(iExpressionVariable,0x69569e99,0x6b3a,0x444c,0xb3,0x75,0xd2,0xce,0xb3,0x74,0x9b,0x4b)
  //! Set the variable name.
  virtual void __stdcall SetName(iHString* ahspString) = 0;
  //! Get the variable name.
  virtual iHString* __stdcall GetName() const = 0;
  //! Copy the specified variable.
  virtual tBool __stdcall Copy(const iExpressionVariable* apVar) = 0;
  //! Clone this variable.
  virtual iExpressionVariable* __stdcall Clone() const = 0;
  //! Get the variable's type.
  //! {Property}
  virtual eExpressionVariableType __stdcall GetType() const = 0;
  //! Get the variable's flags.
  //! {Property}
  virtual tExpressionVariableFlags __stdcall GetFlags() const = 0;
  //! Set the float value of the variable.
  //! {Property}
  virtual void __stdcall SetFloat(tF64 aV) = 0;
  //! Get the float value of the variable.
  //! {Property}
  virtual tF64 __stdcall GetFloat() const = 0;
  //! Set the vector2 value of the variable.
  //! {Property}
  virtual void __stdcall SetVec2(const sVec2f& aV) = 0;
  //! Get the vector2 value of the variable.
  //! {Property}
  virtual sVec2f __stdcall GetVec2() const = 0;
  //! Get the vector3 value of the variable.
  //! {Property}
  virtual void __stdcall  SetVec3(const sVec3f& aV) = 0;
  //! Get the vector3 value of the variable.
  //! {Property}
  virtual sVec3f __stdcall GetVec3() const = 0;
  //! Get the vector4 value of the variable.
  //! {Property}
  virtual void __stdcall SetVec4(const sVec4f& aV) = 0;
  //! Get the vector4 value of the variable.
  //! {Property}
  virtual sVec4f __stdcall GetVec4() const = 0;
  //! Get the matrix value of the variable.
  //! {Property}
  virtual void __stdcall SetMatrix(const sMatrixf& aV) = 0;
  //! Get the matrix value of the variable.
  //! {Property}
  virtual sMatrixf __stdcall GetMatrix() const = 0;
  //! Get the string value of the variable.
  //! {Property}
  virtual void __stdcall SetString(const cString& aV) = 0;
  //! Get the string value of the variable.
  //! {Property}
  virtual cString __stdcall GetString() const = 0;
};

//! Expression interface.
struct iExpression : public iUnknown
{
  niDeclareInterfaceUUID(iExpression,0x0506909d,0x5223,0x4d8b,0x87,0x6b,0x01,0xf5,0xa4,0x0c,0x92,0xf9)
  //! Eval the expression and returns the result.
  virtual iExpressionVariable* __stdcall Eval() = 0;
  //! Get the result returned by the last Eval.
  //! {Property}
  virtual iExpressionVariable* __stdcall GetEvalResult() const = 0;
  //! Get the context.
  //! {Property}
  virtual iExpressionContext* __stdcall GetContext() const = 0;
};

//! Expression URL resolver interface.
//! {DispatchWrapper}
struct iExpressionURLResolver : public iUnknown {
  niDeclareInterfaceUUID(iExpressionURLResolver,0x7802dc51,0xfdaf,0x4de0,0xb0,0x7e,0x8b,0x5d,0x08,0x90,0x13,0x68)

  virtual Var __stdcall ResolveURL(const achar* aURL) = 0;
};

//! Expression context.
struct iExpressionContext : public iUnknown
{
  niDeclareInterfaceUUID(iExpressionContext,0xa870ed3f,0xd227,0x467c,0x98,0xa2,0x8a,0xf1,0x41,0x1a,0xbe,0xe2)

  //########################################################################################
  //! \name General
  //########################################################################################
  //! @{

  //! Get the parent context.
  //! {Property}
  virtual iExpressionContext* __stdcall GetParentContext() const = 0;
  //! Create a child context.
  virtual iExpressionContext* __stdcall CreateContext() = 0;
  //! @}

  //########################################################################################
  //! \name Variables
  //########################################################################################
  //! @{

  //! Create a new variable.
  virtual iExpressionVariable* __stdcall CreateVariable(const achar* aaszName, eExpressionVariableType aType, tExpressionVariableFlags aFlags = eExpressionVariableFlags_Default) = 0;
  //! Create a variable from an expression.
  virtual iExpressionVariable* __stdcall CreateVariableFromExpr(const achar* aaszName, const achar* aaszExpr, tExpressionVariableFlags aFlags = eExpressionVariableFlags_Default) = 0;
  //! Create a variable whos value is fetched from a runnable.
  virtual iExpressionVariable* __stdcall CreateVariableFromRunnable(const achar* aaszName, eExpressionVariableType aType, iRunnable* apRunnable, tExpressionVariableFlags aFlags = eExpressionVariableFlags_Default) = 0;
  //! Create a variable initialized with the value of the specified variant.
  virtual iExpressionVariable* __stdcall CreateVariableFromVar(const achar* aaszName, const Var& aInitialValue, tExpressionVariableFlags aFlags = eExpressionVariableFlags_Default) = 0;

  //! Add a variable to the expression context.
  //! \remark Not duplicate or empty variable names are allowed.
  virtual tBool __stdcall AddVariable(iExpressionVariable* apVar) = 0;
  //! Remove a variable from the expression context.
  virtual tBool __stdcall RemoveVariable(iExpressionVariable* apVariable) = 0;
  //! Find the variable with the specified name.
  virtual iExpressionVariable* __stdcall FindVariable(iHString* ahspName) const = 0;
  //! @}

  //########################################################################################
  //! \name Expressions
  //########################################################################################
  //! @{

  //! Create a new expression.
  virtual iExpression* __stdcall CreateExpression(const achar* aaszExpr) = 0;
  //! Evaluate an expression.
  virtual iExpressionVariable* __stdcall Eval(const achar* aaszExpr) = 0;
  //! Get the unknown symbols into the specified expression.
  //! \param aaszExpr the expression to retrieve the symbols from.
  //! \param apList the string list where the symbols will be place \see ni::tStringCVec
  virtual tBool __stdcall GetUnknownSymbols(const achar* aaszExpr, tStringCVec* apList) = 0;
  //! @}

  //########################################################################################
  //! \name Enumerations
  //########################################################################################
  //! @{

  //! Allows the parser to search in all registered modules enumerations.
  //! {Property}
  //! \remark Disabled by default.
  virtual void __stdcall SetGlobalEnumSearch(tBool abEnabled) = 0;
  //! Get the global enum search status.
  //! {Property}
  virtual tBool __stdcall GetGlobalEnumSearch() const = 0;
  //! Add an enumeration definition.
  virtual tBool __stdcall AddEnumDef(const sEnumDef* apEnumDef) = 0;
  //! Get the specified enumeration.
  //! {Property}
  virtual const sEnumDef* __stdcall GetEnumDef(const achar* aaszName) const = 0;
  //! Set the default enumeration definition.
  //! {Property}
  virtual void __stdcall SetDefaultEnumDef(const sEnumDef* apEnumDef) = 0;
  //! Get the default enumeration definition.
  //! {Property}
  virtual const sEnumDef* __stdcall GetDefaultEnumDef() const = 0;
  //! Get an enumeration value string.
  //! \remark Uses the registered enumerations.
  virtual cString __stdcall GetEnumValueString(tU32 anValue) const = 0;
  //! Get an enumeration flags string.
  //! \remark Uses the registered enumerations.
  virtual cString __stdcall GetEnumFlagsString(tU32 anValue) const = 0;
  //! @}

  //########################################################################################
  //! \name URL Resolver
  //########################################################################################
  //! @{

  virtual tBool __stdcall RegisterURLResolver(const achar* aaszProtocol, iExpressionURLResolver* apResolver) = 0;
  virtual tBool __stdcall UnregisterURLResolver(const achar* aaszProtocol) = 0;
  virtual iExpressionURLResolver* __stdcall FindURLResolver(const achar* aaszProtocol) const = 0;
  //! @}
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IEXPRESSION_H_5FF07DF5_2A37_436B_BCF5_7C09A8AA989D__
