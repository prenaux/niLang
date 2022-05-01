#pragma once
#ifndef __IDATATABLE_15270737_H__
#define __IDATATABLE_15270737_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Types.h>
#include <niLang/Utils/SinkList.h>
#include <niLang/Utils/CollectionImpl.h>
#include <niLang/Types.h>
#include <niLang/ILang.h>
#include "Types.h"
#include "Utils/SinkList.h"
#include "Utils/CollectionImpl.h"
#include "Types.h"
#include "ILang.h"
#include "ISerializable.h"

namespace ni {

#undef GetPrevSibling
#undef GetNextSibling
struct iDataTable;

/** \addtogroup niLang
 * @{
 */

//! Data table property types.
enum eDataTablePropertyType
{
  eDataTablePropertyType_Unknown = eType_Null,
  eDataTablePropertyType_String = eType_String,
  eDataTablePropertyType_Int = eType_I64,
  eDataTablePropertyType_Int32 = eType_I32,
  eDataTablePropertyType_Int64 = eType_I64,
  eDataTablePropertyType_Bool = eType_I8,
  eDataTablePropertyType_Float = eType_F64,
  eDataTablePropertyType_Float32 = eType_F32,
  eDataTablePropertyType_Float64 = eType_F64,
  eDataTablePropertyType_Vec2 = eType_Vec2f,
  eDataTablePropertyType_Vec3 = eType_Vec3f,
  eDataTablePropertyType_Vec4 = eType_Vec4f,
  eDataTablePropertyType_Matrix = eType_Matrixf,
  eDataTablePropertyType_IUnknown = eType_IUnknown|eTypeFlags_Pointer
};

//! Data table copy mode.
enum eDataTableCopyFlags
{
  //! No special flags, will overwrite properties, and wont be recursive.
  eDataTableCopyFlags_Default = 0,
  //! If a property is already in the destination it'll be left as is.
  eDataTableCopyFlags_Skip = niBit(0),
  //! Recursive, children tables will be copied as well (and their own children).
  eDataTableCopyFlags_Recursive = niBit(1),
  //! All children of the source will be appended. By default children which are in
  //! the same sequential order and have the same name as the destination are
  //! merged.
  eDataTableCopyFlags_AppendAllChildren = niBit(2),
  //! Each child data table has a unique name.
  eDataTableCopyFlags_UniqueChild = niBit(3),
  //! Clone all appended children tables.
  eDataTableCopyFlags_Clone = niBit(4),
  //! \internal
  eDataTableCopyFlags_ForceDWORD = 0xFFFFFFF
};

//! Data table copy flags.
typedef tU32 tDataTableCopyFlags;

//! Data table sink.
//! {DispatchWrapper}
struct iDataTableSink : public iUnknown
{
  niDeclareInterfaceUUID(iDataTableSink,0x3c187c5e,0x45d3,0x42c2,0x9e,0x59,0x0e,0x2b,0x33,0x15,0xfc,0x48)
  //! The name of the data table has been set.
  //! {Optional}
  virtual void __stdcall OnDataTableSink_SetName(iDataTable* apDT) = 0;
  //! A child data table has been added.
  //! {Optional}
  virtual void __stdcall OnDataTableSink_AddChild(iDataTable* apDT, iDataTable* apChild) = 0;
  //! A child data table has been removed.
  //! {Optional}
  virtual void __stdcall OnDataTableSink_RemoveChild(iDataTable* apDT, iDataTable* apChild) = 0;
  //! A property has been set.
  //! {Optional}
  //! \remark Recall that OnDataTableSink_GetProperty will be called from any iDataTable::Get(Index)Property(Type), so it should
  //!     probably return niVarNull when SetProperty is entered.
  virtual void __stdcall OnDataTableSink_SetProperty(iDataTable* apDT, tU32 anProperty) = 0;
  //! A property's meta-data has been set.
  //! {Optional}
  virtual void __stdcall OnDataTableSink_SetMetadata(iDataTable* apDT, tU32 anProperty) = 0;
  //! A property is going to be removed.
  //! {Optional}
  virtual void __stdcall OnDataTableSink_RemoveProperty(iDataTable* apDT, tU32 anProperty) = 0;
  //! Get the value of the specified property.
  //! {Optional}
  virtual Var __stdcall OnDataTableSink_GetProperty(iDataTable* apDT, tU32 anProperty) = 0;
};

//! Data table sink list.
typedef SinkList<iDataTableSink> tDataTableSinkLst;

//! Data table.
struct iDataTable : public iUnknown
{
  niDeclareInterfaceUUID(iDataTable,0x6a1f95c1,0x4d23,0x4c62,0x96,0x86,0x56,0x1c,0xb9,0x2c,0x36,0xd5)

  //########################################################################################
  //! \name System
  //########################################################################################
  //! @{

  //! Set the table name.
  //! {Property}
  virtual void __stdcall SetName(const achar* aaszName) = 0;
  //! Get the table name.
  //! {Property}
  virtual const achar* __stdcall GetName() const = 0;

  //! Get the root table.
  //! {Property}
  virtual iDataTable* __stdcall GetRoot() const = 0;
  //! Get the parent table.
  //! {Property}
  virtual iDataTable* __stdcall GetParent() const = 0;
  //! Get the previous sibling of this data table.
  //! {Property}
  virtual iDataTable* __stdcall GetPrevSibling() const = 0;
  //! Get the next sibling of this data table.
  //! {Property}
  virtual iDataTable* __stdcall GetNextSibling() const = 0;
  //! Get the index of this datatable.
  //! {Property}
  virtual tU32 __stdcall GetIndex() const = 0;

  //! Clone this table.
  //! \remark Shortcut for CloneEx(eDataTableCopyFlags_Clone|eDataTableCopyFlags_Recursive|eDataTableCopyFlags_AppendAllChildren).
  virtual iDataTable* __stdcall Clone() const = 0;
  //! Clone this table.
  virtual iDataTable* __stdcall CloneEx(tDataTableCopyFlags aMode) const = 0;

  //! Copy the source table inside this table.
  //! \remark Automatically convert to this type if the type is not the same as this table.
  virtual tBool __stdcall Copy(const iDataTable* apSource, tDataTableCopyFlags aMode) = 0;

  //! Clear the children datatables and properties.
  virtual void __stdcall Clear() = 0;

  //! Check if any sink is inside the data table.
  //! {Property}
  virtual tBool __stdcall GetHasSink() const = 0;
  //! Get the data table's sink list.
  //! {Property}
  virtual tDataTableSinkLst* __stdcall GetSinkList() const = 0;
  //! @}


  //########################################################################################
  //! \name Children DataTables
  //########################################################################################
  //! @{

  //! Clear the child tables.
  virtual void __stdcall ClearChildren() = 0;
  //! Get the number of child tables.
  //! {Property}
  virtual tU32 __stdcall GetNumChildren() const = 0;
  //! Get the index of the first child table with the specified name.
  //! {Property}
  virtual tU32 __stdcall GetChildIndex(const achar* aaszName) const = 0;
  //! Get the child table with the specified name.
  //! {Property}
  virtual iDataTable* __stdcall GetChild(const achar* aaszName) const = 0;
  //! Get the child table at the specified index.
  //! {Property}
  virtual iDataTable* __stdcall GetChildFromIndex(tU32 anIndex) const = 0;
  //! Append a child table.
  virtual tBool __stdcall AddChild(iDataTable* apTable) = 0;
  //! Remove the child table at the specified index.
  virtual tBool __stdcall RemoveChild(tU32 anIndex) = 0;
  //! @}

  //########################################################################################
  //! \name Properties
  //########################################################################################
  //! @{

  //! Clear the properties.
  virtual void __stdcall ClearProperties() = 0;
  //! Remove the property with the specified name.
  virtual tBool __stdcall RemoveProperty(const achar* aaszName) = 0;
  //! Remove the property at the specified index.
  virtual tBool __stdcall RemovePropertyFromIndex(tU32 anIndex) = 0;
  //! Get the number of properties in the table.
  //! {Property}
  virtual tU32 __stdcall GetNumProperties() const = 0;
  //! Get the index of the specified property.
  //! {Property}
  virtual tU32 __stdcall GetPropertyIndex(const achar* aaszName) const = 0;
  //! Get the name of the property at the specified index.
  //! {Property}
  virtual const achar* __stdcall GetPropertyName(tU32 anIndex) const = 0;
  //! Get the type of a property.
  //! {Property}
  virtual eDataTablePropertyType __stdcall GetPropertyType(const achar* aaszProp) const = 0;
  //! Get the type of the property at the specified index.
  //! {Property}
  virtual eDataTablePropertyType __stdcall GetPropertyTypeFromIndex(tU32 anIndex) const = 0;
  //! Check whether the data table has a property with the specified name.
  virtual tBool __stdcall HasProperty(const achar* aaszName) const = 0;
  //! @}

  //########################################################################################
  //! \name Properties metadata
  //########################################################################################
  //! @{

  //! Set a property meta-data.
  //! {Property}
  virtual tBool __stdcall SetMetadata(const achar* aaszProp, iHString* ahspData) = 0;
  //! Get a property meta-data.
  //! {Property}
  virtual Ptr<iHString> __stdcall GetMetadata(const achar* aaszProp) const = 0;
  //! Set a property meta-data.
  //! {Property}
  virtual tBool __stdcall SetMetadataFromIndex(tU32 anIndex, iHString* ahspData) = 0;
  //! Get a property meta-data.
  //! {Property}
  virtual Ptr<iHString> __stdcall GetMetadataFromIndex(tU32 anIndex) const = 0;
  //! @}

  //########################################################################################
  //! \name Get Properties
  //########################################################################################
  //! @{

  //! Get a Var property.
  //! {Property}
  virtual Var __stdcall GetVar(const achar* aaszName) const = 0;
  //! Get a Var property from the specified index.
  //! {Property}
  virtual Var __stdcall GetVarFromIndex(tU32 anIndex) const = 0;
  //! Get a Var property.
  //! \remark Allow to specify the default value.
  virtual Var __stdcall GetVarDefault(const achar* aaszName, const Var& v) const = 0;

  //! Get a String property.
  //! {Property}
  virtual cString __stdcall GetString(const achar* aaszName) const = 0;
  //! Get a String property from the specified index.
  //! {Property}
  virtual cString __stdcall GetStringFromIndex(tU32 anIndex) const = 0;
  //! Get a String property.
  //! \remark Allow to specify the default value.
  virtual cString __stdcall GetStringDefault(const achar* aaszName, const achar* v) const = 0;

  //! Get a HString property.
  //! {Property}
  virtual Ptr<iHString> __stdcall GetHString(const achar* aaszName) const = 0;
  //! Get a HString property from the specified index.
  //! {Property}
  virtual Ptr<iHString> __stdcall GetHStringFromIndex(tU32 anIndex) const = 0;
  //! Get a HString property.
  //! \remark Allow to specify the default value.
  virtual Ptr<iHString> __stdcall GetHStringDefault(const achar* aaszName, iHString* v) const = 0;

  //! Get a Int property.
  //! {Property}
  virtual tI64 __stdcall GetInt(const achar* aaszName) const = 0;
  //! Get a Int property from the specified index.
  //! {Property}
  virtual tI64 __stdcall GetIntFromIndex(tU32 anIndex) const = 0;
  //! Get a Int property.
  //! \remark Allow to specify the default value.
  virtual tI64 __stdcall GetIntDefault(const achar* aaszName, tI64 v) const = 0;

  //! Get a Bool property.
  //! {Property}
  virtual tBool __stdcall GetBool(const achar* aaszName) const = 0;
  //! Get a Bool property from the specified index.
  //! {Property}
  virtual tBool __stdcall GetBoolFromIndex(tU32 anIndex) const = 0;
  //! Get a Bool property.
  //! \remark Allow to specify the default value.
  virtual tBool __stdcall GetBoolDefault(const achar* aaszName, tBool v) const = 0;

  //! Get a Float property.
  //! {Property}
  virtual tF64 __stdcall GetFloat(const achar* aaszName) const = 0;
  //! Get a Float property from the specified index.
  //! {Property}
  virtual tF64 __stdcall GetFloatFromIndex(tU32 anIndex) const = 0;
  //! Get a Float property.
  //! \remark Allow to specify the default value.
  virtual tF64 __stdcall GetFloatDefault(const achar* aaszName, tF64 v) const = 0;

  //! Get a Vec2 property.
  //! {Property}
  virtual sVec2f __stdcall GetVec2(const achar* aaszName) const = 0;
  //! Get a Vec2 property from the specified index.
  //! {Property}
  virtual sVec2f __stdcall GetVec2FromIndex(tU32 anIndex) const = 0;
  //! Get a Vec2 property.
  //! \remark Allow to specify the default value.
  virtual sVec2f __stdcall GetVec2Default(const achar* aaszName, const sVec2f& v) const = 0;

  //! Get a Vec3 property.
  //! {Property}
  virtual sVec3f __stdcall GetVec3(const achar* aaszName) const = 0;
  //! Get a Vec3 property from the specified index.
  //! {Property}
  virtual sVec3f __stdcall GetVec3FromIndex(tU32 anIndex) const = 0;
  //! Get a Vec3 property.
  //! \remark Allow to specify the default value.
  virtual sVec3f __stdcall GetVec3Default(const achar* aaszName, const sVec3f& v) const = 0;

  //! Get a Vec4 property.
  //! {Property}
  virtual sVec4f __stdcall GetVec4(const achar* aaszName) const = 0;
  //! Get a Vec4 property from the specified index.
  //! {Property}
  virtual sVec4f __stdcall GetVec4FromIndex(tU32 anIndex) const = 0;
  //! Get a Vec4 property.
  //! \remark Allow to specify the default value.
  virtual sVec4f __stdcall GetVec4Default(const achar* aaszName, const sVec4f& v) const = 0;

  //! Get a Color3 property.
  //! {Property}
  //! \remark Returns sVec3f::One() if the property cannot found.
  virtual sVec3f __stdcall GetCol3(const achar* aaszName) const = 0;
  //! Get a Color3 property from the specified index.
  //! {Property}
  //! \remark Returns sVec3f::One() if the property cannot found.
  virtual sVec3f __stdcall GetCol3FromIndex(tU32 anIndex) const = 0;

  //! Get a Color4 property.
  //! {Property}
  //! \remark Returns sVec4f::One() if the property cannot found.
  virtual sVec4f __stdcall GetCol4(const achar* aaszName) const = 0;
  //! Get a Color4 property from the specified index.
  //! {Property}
  //! \remark Returns sVec4f::One() if the property cannot found.
  virtual sVec4f __stdcall GetCol4FromIndex(tU32 anIndex) const = 0;

  //! Get a Matrix property.
  //! {Property}
  virtual sMatrixf __stdcall GetMatrix(const achar* aaszName) const = 0;
  //! Get a Matrix property from the specified index.
  //! {Property}
  virtual sMatrixf __stdcall GetMatrixFromIndex(tU32 anIndex) const = 0;
  //! Get a Matrix property.
  //! \remark Allow to specify the default value.
  virtual sMatrixf __stdcall GetMatrixDefault(const achar* aaszName, const sMatrixf& v) const = 0;

  //! Get a IUnknown property.
  //! {Property}
  virtual iUnknown* __stdcall GetIUnknown(const achar* aaszName) const = 0;
  //! Get a IUnknown property from the specified index.
  //! {Property}
  virtual iUnknown* __stdcall GetIUnknownFromIndex(tU32 anIndex) const = 0;
  //! Get a IUnknown property.
  //! \remark Allow to specify the default value.
  virtual iUnknown* __stdcall GetIUnknownDefault(const achar* aaszName, iUnknown* v) const = 0;

  //! Get a Enum property.
  virtual tU32 __stdcall GetEnum(const achar* aaszName, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags) const = 0;
  //! Get a Enum property from the specified index.
  virtual tU32 __stdcall GetEnumFromIndex(tU32 anIndex, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags) const = 0;
  //! Get a Enum property.
  //! \remark Allow to specify the default value.
  virtual tU32 __stdcall GetEnumDefault(const achar* aaszName, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags, tU32 v) const = 0;
  //! @}

  //########################################################################################
  //! \name Set Properties
  //########################################################################################
  //! @{

  //! Set a Var property.
  //! {Property}
  virtual tU32 __stdcall SetVar(const achar* aaszName, const Var& v) = 0;
  //! Set a Var property from the specified index.
  //! {Property}
  virtual void __stdcall SetVarFromIndex(tU32 anIndex, const Var& v) = 0;

  //! Set a String property.
  //! {Property}
  virtual tU32 __stdcall SetString(const achar* aaszName, const achar* v) = 0;
  //! Set a String property from the specified index.
  //! {Property}
  virtual void __stdcall SetStringFromIndex(tU32 anIndex, const achar* v) = 0;

  //! Set a HString property.
  //! {Property}
  virtual tU32 __stdcall SetHString(const achar* aaszName, iHString* v) = 0;
  //! Set a HString property from the specified index.
  //! {Property}
  virtual void __stdcall SetHStringFromIndex(tU32 anIndex, iHString* v) = 0;

  //! Set a Int property.
  //! {Property}
  virtual tU32 __stdcall SetInt(const achar* aaszName, tI64 v) = 0;
  //! Set a Int property from the specified index.
  //! {Property}
  virtual void __stdcall SetIntFromIndex(tU32 anIndex, tI64 v) = 0;

  //! Set a Bool property.
  //! {Property}
  virtual tU32 __stdcall SetBool(const achar* aaszName, tBool v) = 0;
  //! Set a Bool property from the specified index.
  //! {Property}
  virtual void __stdcall SetBoolFromIndex(tU32 anIndex, tBool v) = 0;

  //! Set a Float property.
  //! {Property}
  virtual tU32 __stdcall SetFloat(const achar* aaszName, tF64 v) = 0;
  //! Set a Float property from the specified index.
  //! {Property}
  virtual void __stdcall SetFloatFromIndex(tU32 anIndex, tF64 v) = 0;

  //! Set a Vec2 property.
  //! {Property}
  virtual tU32 __stdcall SetVec2(const achar* aaszName, const sVec2f& v) = 0;
  //! Set a Vec2 property from the specified index.
  //! {Property}
  virtual void __stdcall SetVec2FromIndex(tU32 anIndex, const sVec2f& v) = 0;

  //! Set a Vec3 property.
  //! {Property}
  virtual tU32 __stdcall SetVec3(const achar* aaszName, const sVec3f& v) = 0;
  //! Set a Vec3 property from the specified index.
  //! {Property}
  virtual void __stdcall SetVec3FromIndex(tU32 anIndex, const sVec3f& v) = 0;

  //! Set a Vec4 property.
  //! {Property}
  virtual tU32 __stdcall SetVec4(const achar* aaszName, const sVec4f& v) = 0;
  //! Set a Vec4 property from the specified index.
  //! {Property}
  virtual void __stdcall SetVec4FromIndex(tU32 anIndex, const sVec4f& v) = 0;

  //! Set a Matrix property.
  //! {Property}
  virtual tU32 __stdcall SetMatrix(const achar* aaszName, const sMatrixf& v) = 0;
  //! Set a Matrix property from the specified index.
  //! {Property}
  virtual void __stdcall SetMatrixFromIndex(tU32 anIndex, const sMatrixf& v) = 0;

  //! Set a IUnknown property.
  //! {Property}
  virtual tU32 __stdcall SetIUnknown(const achar* aaszName, iUnknown* v) = 0;
  //! Set a IUnknown property from the specified index.
  //! {Property}
  virtual void __stdcall SetIUnknownFromIndex(tU32 anIndex, iUnknown* v) = 0;

  //! Set an Enum property.
  virtual tU32 __stdcall SetEnum(const achar* aaszName, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags, tU32 anVal) = 0;
  //! Set an Enum property from the specified index.
  virtual void __stdcall SetEnumFromIndex(tU32 anIndex, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags, tU32 anVal) = 0;
  //! @}

  //########################################################################################
  //! \name Raw access && Path Properties
  //########################################################################################
  //! @{

  //! Set a Var property without ever calling the callback sink.
  //! {Property}
  virtual tU32 __stdcall SetRawVar(const achar* aaszName, const Var& v) = 0;
  //! Set a Var property from the specified index  without ever calling the callback sink.
  //! {Property}
  virtual void __stdcall SetRawVarFromIndex(tU32 anIndex, const Var& v) = 0;

  //! Get a Var property without ever calling the callback sink.
  //! {Property}
  virtual Var __stdcall GetRawVar(const achar* aaszName) const = 0;
  //! Get a Var property from the specified index without ever calling the callback sink..
  //! {Property}
  virtual Var __stdcall GetRawVarFromIndex(tU32 anIndex) const = 0;
  //! Get a Var property without ever calling the callback sink.
  //! \remark Allow to specify the default value.
  virtual Var __stdcall GetRawVarDefault(const achar* aaszName, const Var& v) const = 0;

  //! Set a new variant property. Property name being a path.
  //! \return false if the the path doesnt exist and cant be created.
  virtual tBool __stdcall NewVarFromPath(const achar* aPath, const Var& aVal) = 0;
  //! Set a variant property. Property name being a path. Return false if the path doesnt exist.
  virtual tBool __stdcall SetVarFromPath(const achar* aPath, const Var& aVal) = 0;
  //! Get a variant property. Property name being a path.
  virtual Var __stdcall GetVarFromPath(const achar* aPath, const Var& aDefault) const = 0;
  //! @}
};

//! Data table read stack interface.
struct iDataTableReadStack : public iUnknown
{
  niDeclareInterfaceUUID(iDataTableReadStack,0x8fd58593,0xf9bc,0x4368,0x82,0x57,0x99,0x75,0xca,0xbe,0x19,0x3f)

  //! Get the top of the data table stack.
  //! {Property}
  virtual iDataTable* __stdcall GetTop() const = 0;
  //! Get the name of the top data table.
  //! {Property}
  virtual const achar* __stdcall GetName() const = 0;
  //! Get the number of child data table of the top data table.
  //! {Property}
  virtual tU32 __stdcall GetNumChildren() const = 0;
  //! Push the specified child data table on top of the stack.
  //! Push a NULL data table if it doesn't exists.
  virtual tBool __stdcall PushChild(tU32 anIndex) = 0;
  //! Push the specified child data table on top of the stack.
  //! Do not push a data table if it doesn't exists and returns eFalse.
  virtual tBool __stdcall PushChildFail(tU32 anIndex) = 0;
  //! Push the specified data table on top of the stack.
  //! Push a NULL data table if it doesn't exists.
  virtual tBool __stdcall Push(const achar* aaszName) = 0;
  //! Push the specified data table on top of the stack.
  //! Do not push a data table if it doesn't exists and returns eFalse.
  virtual tBool __stdcall PushFail(const achar* aaszName) = 0;
  //! Push the specified data table on top of the stack.
  //! Push a NULL data table if it doesn't exists.
  virtual tBool __stdcall PushEx(const achar* aaszName, const achar* aaszProp, const achar* aaszVal) = 0;
  //! Push the specified data table on top of the stack.
  //! Do not push a data table if it doesn't exists and returns eFalse.
  virtual tBool __stdcall PushFailEx(const achar* aaszName, const achar* aaszProp, const achar* aaszVal) = 0;
  //! Pop the top data table.
  virtual tBool __stdcall Pop() = 0;

  //! Check whether the top data table has a property with the specified name.
  virtual tBool __stdcall HasProperty(const achar* aaszName) const = 0;
  //! Remove a property of the top data table.
  virtual tBool __stdcall RemoveProperty(const achar* aaszName) = 0;

  //! Get a String of the top data table.
  //! {Property}
  virtual cString __stdcall GetString(const achar* aaszName) const = 0;
  //! Get a String of the top data table.
  //! \remark Allow to specify the default value.
  virtual cString __stdcall GetStringDefault(const achar* aaszName, const achar* v) const = 0;
  //! Get a HString of the top data table.
  //! {Property}
  virtual Ptr<iHString> __stdcall GetHString(const achar* aaszName) const = 0;
  //! Get a HString of the top data table.
  //! \remark Allow to specify the default value.
  virtual Ptr<iHString> __stdcall GetHStringDefault(const achar* aaszName, iHString* v) const = 0;
  //! Get a Int of the top data table.
  //! {Property}
  virtual tI64 __stdcall GetInt(const achar* aaszName) const = 0;
  //! Get a Int of the top data table.
  //! \remark Allow to specify the default value.
  virtual tI64 __stdcall GetIntDefault(const achar* aaszName, tI64 v) const = 0;
  //! Get a Bool of the top data table.
  //! {Property}
  virtual tBool __stdcall GetBool(const achar* aaszName) const = 0;
  //! Get a Bool of the top data table.
  //! \remark Allow to specify the default value.
  virtual tBool __stdcall GetBoolDefault(const achar* aaszName, tBool v) const = 0;
  //! Get a Float of the top data table.
  //! {Property}
  virtual tF64 __stdcall GetFloat(const achar* aaszName) const = 0;
  //! Get a Float of the top data table.
  //! \remark Allow to specify the default value.
  virtual tF64 __stdcall GetFloatDefault(const achar* aaszName, tF64 v) const = 0;
  //! Get a Vec2 of the top data table.
  //! {Property}
  virtual sVec2f __stdcall GetVec2(const achar* aaszName) const = 0;
  //! Get a Vec2 of the top data table.
  //! \remark Allow to specify the default value.
  virtual sVec2f __stdcall GetVec2Default(const achar* aaszName, const sVec2f& v) const = 0;
  //! Get a Vec3 of the top data table.
  //! {Property}
  virtual sVec3f __stdcall GetVec3(const achar* aaszName) const = 0;
  //! Get a Vec3 of the top data table.
  //! \remark Allow to specify the default value.
  virtual sVec3f __stdcall GetVec3Default(const achar* aaszName, const sVec3f& v) const = 0;
  //! Get a Vec4 of the top data table.
  //! {Property}
  virtual sVec4f __stdcall GetVec4(const achar* aaszName) const = 0;
  //! Get a Vec4 of the top data table.
  //! \remark Allow to specify the default value.
  virtual sVec4f __stdcall GetVec4Default(const achar* aaszName, const sVec4f& v) const = 0;
  //! Get a Color3 of the top data table.
  //! {Property}
  //! \remark This is equivalent to GetVec3Default(name,sVec3f(1,1,1))
  virtual sVec3f __stdcall GetCol3(const achar* aaszName) const = 0;
  //! Get a Color4 of the top data table.
  //! {Property}
  //! \remark This is equivalent to GetVec3Default(name,sVec3f(1,1,1,1))
  virtual sVec4f __stdcall GetCol4(const achar* aaszName) const = 0;
  //! Get a Matrix of the top data table.
  //! {Property}
  virtual sMatrixf __stdcall GetMatrix(const achar* aaszName) const = 0;
  //! Get a Matrix of the top data table.
  //! \remark Allow to specify the default value.
  virtual sMatrixf __stdcall GetMatrixDefault(const achar* aaszName, const sMatrixf& v) const = 0;
  //! Get a IUnknown of the top data table.
  //! {Property}
  virtual iUnknown* __stdcall GetIUnknown(const achar* aaszName) const = 0;
  //! Get a IUnknown of the top data table.
  //! \remark Allow to specify the default value.
  virtual iUnknown* __stdcall GetIUnknownDefault(const achar* aaszName, iHString* v) const = 0;
  //! Get a Enum of the top data table.
  virtual tU32 __stdcall GetEnum(const achar* aaszName, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags) const = 0;
  //! Get a Enum of the top data table.
  //! \remark Allow to specify the default value.
  virtual tU32 __stdcall GetEnumDefault(const achar* aaszName, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags, tU32 v) const = 0;

  //! Get a property meta-data.
  //! {Property}
  virtual Ptr<iHString> __stdcall GetMetadata(const achar* aaszProp) const = 0;
};

//! Data table write stack interface.
struct iDataTableWriteStack : public iUnknown
{
  niDeclareInterfaceUUID(iDataTableWriteStack,0x3f35c253,0x8a23,0x4d2f,0x9a,0xf9,0x47,0x15,0xbc,0x80,0x7f,0x31)

  //! Get the top of the data table stack.
  //! {Property}
  virtual iDataTable* __stdcall GetTop() const = 0;

  //! Set the name of the top data table.
  //! {Property}
  virtual void __stdcall SetName(const achar* aaszName) const = 0;
  //! Get the name of the top data table.
  //! {Property}
  virtual const achar* __stdcall GetName() const = 0;

  //! Get the number of child data table of the top data table.
  //! {Property}
  virtual tU32 __stdcall GetNumChildren() const = 0;
  //! Push the specified child data table on top of the stack.
  //! Push a NULL data table if it doesn't exists.
  virtual tBool __stdcall PushChild(tU32 anIndex) = 0;
  //! Push the specified child data table on top of the stack.
  //! Do not push a data table if it doesn't exists and returns eFalse.
  virtual tBool __stdcall PushChildFail(tU32 anIndex) = 0;
  //! Push a data table with the given name on top of the stack.
  //! If a child data table with the given name doesn't exists, creates it.
  virtual tBool __stdcall Push(const achar* aaszName) = 0;
  //! Push a data table with the given name on top of the stack.
  //! If a child data table with the given name doesn't exists, return eFalse.
  virtual tBool __stdcall PushFail(const achar* aaszName) = 0;
  //! Push a new data table on top of the stack.
  virtual tBool __stdcall PushNew(const achar* aaszName) = 0;
  //! Append the given data table, and push it on top of the stack.
  virtual tBool __stdcall PushAppend(iDataTable* apDT) = 0;
  //! Push the specified data table on top of the stack.
  //! Push a NULL data table if it doesn't exists.
  virtual tBool __stdcall PushEx(const achar* aaszName, const achar* aaszProp, const achar* aaszVal) = 0;
  //! Push the specified data table on top of the stack.
  //! Do not push a data table if it doesn't exists and returns eFalse.
  virtual tBool __stdcall PushFailEx(const achar* aaszName, const achar* aaszProp, const achar* aaszVal) = 0;
  //! Pop the top data table.
  virtual tBool __stdcall Pop() = 0;

  //! Check whether the top data table has a property with the specified name.
  virtual tBool __stdcall HasProperty(const achar* aaszName) const = 0;
  //! Remove a property of the top data table.
  virtual tBool __stdcall RemoveProperty(const achar* aaszName) = 0;

  //! Set a String in the top data table.
  //! {Property}
  virtual void __stdcall SetString(const achar* aaszName, const achar* v) = 0;
  //! Set a HString in the top data table.
  //! {Property}
  virtual void __stdcall SetHString(const achar* aaszName, iHString* v) = 0;
  //! Set a Int in the top data table.
  //! {Property}
  virtual void __stdcall SetInt(const achar* aaszName, tI64 v) = 0;
  //! Set a Bool in the top data table.
  //! {Property}
  virtual void __stdcall SetBool(const achar* aaszName, tBool v) = 0;
  //! Set a Float in the top data table.
  //! {Property}
  virtual void __stdcall SetFloat(const achar* aaszName, tF64 v) = 0;
  //! Set a Vec2 in the top data table.
  //! {Property}
  virtual void __stdcall SetVec2(const achar* aaszName, const sVec2f& v) = 0;
  //! Set a Vec3 in the top data table.
  //! {Property}
  virtual void __stdcall SetVec3(const achar* aaszName, const sVec3f& v) = 0;
  //! Set a Vec4 in the top data table.
  //! {Property}
  virtual void __stdcall SetVec4(const achar* aaszName, const sVec4f& v) = 0;
  //! Set a Matrix in the top data table.
  //! {Property}
  virtual void __stdcall SetMatrix(const achar* aaszName, const sMatrixf& v) = 0;
  //! Set a IUnknown in the top data table.
  //! {Property}
  virtual void __stdcall SetIUnknown(const achar* aaszName, iUnknown* v) = 0;
  //! Set a Enum in the top data table.
  virtual void __stdcall SetEnum(const achar* aaszName, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags, tU32 anVal) = 0;

  //! Set a property meta-data.
  //! {Property}
  virtual tBool __stdcall SetMetadata(const achar* aaszProp, iHString* ahspData) = 0;
};

niExportFunc(iDataTable*) CreateDataTable(const achar* aaszName = NULL);
niExportFunc(iUnknown*) New_niLang_DataTable(const Var& avarA, const Var& avarB);

///////////////////////////////////////////////
inline iDataTableWriteStack* __stdcall CreateDataTableWriteStack(iDataTable* apDT) {
  return GetLang()->CreateDataTableWriteStack(apDT);
}

///////////////////////////////////////////////
inline iDataTableWriteStack* __stdcall CreateDataTableWriteStackFromName(const achar* aaszName) {
  return GetLang()->CreateDataTableWriteStackFromName(aaszName);
}

///////////////////////////////////////////////
inline iDataTableReadStack* __stdcall CreateDataTableReadStack(iDataTable* apDT) {
  return GetLang()->CreateDataTableReadStack(apDT);
}

///////////////////////////////////////////////
inline tBool __stdcall SerializeDataTable(const achar* aaszSerName, eSerializeMode aMode, iDataTable* apTable, iFile* apFile) {
  return GetLang()->SerializeDataTable(aaszSerName,aMode,apTable,apFile);
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IDATATABLE_15270737_H__
