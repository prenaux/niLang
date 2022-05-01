#pragma once
#ifndef __IJSON_H_5694692E_30C5_46B2_9423_72DD9BF023C6__
#define __IJSON_H_5694692E_30C5_46B2_9423_72DD9BF023C6__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#if niMinFeatures(20)
namespace ni {
struct iDataTable;

/** \addtogroup niLang
 * @{
 */

//! Json value types
enum eJsonType {
  //! Syntax parsing.
  eJsonType_Syntax = 0,
  //! Pair Name (string)
  eJsonType_Name = 1,
  //! String
  eJsonType_String = 2,
  //! Number
  eJsonType_Number = 3,
  //! true
  eJsonType_True = 4,
  //! false
  eJsonType_False = 5,
  //! null
  eJsonType_Null = 6,
  //! Object begining
  eJsonType_ObjectBegin = 7,
  //! Object end
  eJsonType_ObjectEnd = 8,
  //! Array begining
  eJsonType_ArrayBegin = 9,
  //! Array end
  eJsonType_ArrayEnd = 10,
  //! \internal
  eJsonType_ForceDWORD = 0xFFFFFFFF
};

//! Json parser sink interface.
struct iJsonParserSink : public iUnknown {
  niDeclareInterfaceUUID(iJsonParserSink,0xa565aba5,0x71af,0x4045,0xa9,0x99,0xf8,0xef,0x4c,0x05,0x5c,0xfa);
  //! Called when a parsing error occured.
  virtual void __stdcall OnJsonParserSink_Error(const achar* aaszReason, tU32 anLine, tU32 anCol) = 0;
  //! Called when a value is parsed
  virtual void __stdcall OnJsonParserSink_Value(eJsonType aType, const achar* aValue) = 0;
};

//! Json writer sink interface.
struct iJsonWriterSink : public iUnknown {
  niDeclareInterfaceUUID(iJsonWriterSink,0x8fb94e8c,0x6267,0x4f9a,0xad,0x55,0x1d,0x06,0x01,0xbb,0x29,0x89);
  //! Called when there's a writting error.
  virtual void __stdcall OnJsonWriterSink_Error(const achar* aaszReason) = 0;
  //! Called when to write a value.
  virtual tBool __stdcall OnJsonWriterSink_Write(eJsonType aValue, const achar* aaszString) = 0;
};

//! Json writer interface.
struct iJsonWriter : public iUnknown {
  niDeclareInterfaceUUID(iJsonWriter,0x265dc4db,0x8a0f,0x4283,0xbc,0x83,0x56,0x24,0x17,0x80,0xc2,0x17);

  //! Reset the writer's content.
  virtual void __stdcall Reset() = 0;

  //! Begin an array.
  virtual tBool __stdcall ArrayBegin() = 0;
  //! Ends an array.
  virtual tBool __stdcall ArrayEnd() = 0;
  //! Begin an object.
  virtual tBool __stdcall ObjectBegin() = 0;
  //! Ends an object.
  virtual tBool __stdcall ObjectEnd() = 0;

  //! Append a key name.
  virtual tBool __stdcall Name(const achar* aName) = 0;

  //! Append a string in the current array/object.
  virtual tBool __stdcall ValueString(const achar* aStr) = 0;
  //! Append an integer in the current array/object.
  virtual tBool __stdcall ValueNumber(const achar* aStr) = 0;
  //! Append a boolean in the current array/object.
  virtual tBool __stdcall ValueBool(tBool abItem) = 0;
  //! Append a null value in the current array/object.
  virtual tBool __stdcall ValueNull() = 0;

  //! Write a string property in the current object.
  virtual tBool __stdcall ObjectString(const achar* aName, const achar* aStr) = 0;
  //! Write a number property in the current object.
  virtual tBool __stdcall ObjectNumber(const achar* aName, const achar* aStr) = 0;
  //! Write a boolean property in the current object.
  virtual tBool __stdcall ObjectBool(const achar* aName, tBool abValue) = 0;
  //! Write a null property in the current object.
  virtual tBool __stdcall ObjectNull(const achar* aName) = 0;
};

niExportFunc(ni::tBool) JsonParseFile(ni::iFile* apFile, ni::iJsonParserSink* apSink);
niExportFunc(ni::tBool) JsonParseString(const ni::cString& aString, ni::iJsonParserSink* apSink);
niExportFunc(ni::iJsonWriter*) CreateJsonSinkWriter(ni::iJsonWriterSink* apSink, ni::tBool abPrettyPrint);
niExportFunc(ni::iJsonWriter*) CreateJsonFileWriter(ni::iFile* apFile, ni::tBool abPrettyPrint);
niExportFunc(ni::tBool) JsonParseFileToDataTable(ni::iFile* apFile, ni::iDataTable* apDT);
niExportFunc(ni::tBool) JsonParseStringToDataTable(const ni::cString& aString, ni::iDataTable* apDT);

/**@}*/
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // niMinFeatures
#endif // __IJSON_H_5694692E_30C5_46B2_9423_72DD9BF023C6__
