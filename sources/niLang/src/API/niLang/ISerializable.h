#pragma once
#ifndef __ISERIALIZABLE_20417058_H__
#define __ISERIALIZABLE_20417058_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Types.h>

namespace ni {
struct iFile;

/** \addtogroup niLang
 * @{
 */

//! Serialization flags.
enum eSerializeFlags
{
  //! Serialize write.
  eSerializeFlags_Write = niBit(0),
  //! Serialize read.
  eSerializeFlags_Read = niBit(1),
  //! Serialization category 1.
  eSerializeFlags_Category1 = niBit(2),
  //! Serialization category 2.
  eSerializeFlags_Category2 = niBit(3),
  //! Serialization category 3.
  eSerializeFlags_Category3 = niBit(4),
  //! Serialization category 4.
  eSerializeFlags_Category4 = niBit(5),
  //! Serialization category 5.
  eSerializeFlags_Category5 = niBit(6),
  //! Serialization category 6.
  eSerializeFlags_Category6 = niBit(7),
  //! Serialization category 7.
  eSerializeFlags_Category7 = niBit(8),
  //! Serialization category 8.
  eSerializeFlags_Category8 = niBit(9),
  //! Serialization category 1 to 4 (base).
  eSerializeFlags_CategoryBase = eSerializeFlags_Category1|eSerializeFlags_Category2|eSerializeFlags_Category3|eSerializeFlags_Category4,
  //! Serialization category 4 to 8 (extended).
  eSerializeFlags_CategoryExtended = eSerializeFlags_Category5|eSerializeFlags_Category6|eSerializeFlags_Category7|eSerializeFlags_Category8,
  //! Serialization all category.
  eSerializeFlags_CategoryAll = eSerializeFlags_CategoryBase|eSerializeFlags_CategoryExtended,
  //! Serialize write will also write type information metadata.
  //! \remark The 11th bit is reserved to set whether or not 'edit' metadata should be serialized.
  //!     This can be, and is, assumed by serialization flags for other object types.
  //!     This allow the C++ serialization code to be shortened a lot with the use of the
  //!     DataTable serialization macros (see DataTableSerialize.h).
  eSerializeFlags_TypeInfoMetadata = niBit(10),
  //! Raw serialization, dont put object serialization header.
  //! \remark This is relevant for file/stream serialization.
  eSerializeFlags_Raw = niBit(11),
  //! \internal
  eSerializeFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Serialization mode
enum eSerializeMode
{
  //! Write mode.
  eSerializeMode_Write = eSerializeFlags_Write,
  //! Write raw mode.
  //! \remark Means that no object header will be written.
  eSerializeMode_WriteRaw = eSerializeFlags_Write|eSerializeFlags_Raw,
  //! Read mode.
  eSerializeMode_Read = eSerializeFlags_Read,
  //! Read raw mode.
  //! \remark Means that no object header will be checked for.
  eSerializeMode_ReadRaw = eSerializeFlags_Read|eSerializeFlags_Raw,
  //! \internal
  eSerializeMode_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Serialization flags type.
typedef tU32 tSerializeFlags;

//! iSerializable interface.
struct iSerializable : public iUnknown
{
  niDeclareInterfaceUUID(iSerializable,0x1e9334fc,0xe622,0x4932,0xbb,0x73,0xed,0xf0,0xc9,0xdc,0x1c,0x90)

  //! Get the object type ID.
  //! \return NULL if there is no object type.
  virtual const achar* __stdcall GetSerializeObjectTypeID() const = 0;
  //! Serialize the object.
  //! \param  apFile is the file interface where the object must be serialized.
  //! \param  aMode
  //! \return The number of bytes serialized, eInvalidHandle on error.
  virtual tSize __stdcall Serialize(iFile* apFile, eSerializeMode aMode) = 0;
};

/**@}*/
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __ISERIALIZABLE_20417058_H__
