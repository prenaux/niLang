#ifndef __IMATERIALLIBRARY_2996328_H__
#define __IMATERIALLIBRARY_2996328_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {

struct iMaterialLibrary;

/** \addtogroup niUI
 * @{
 */

//! Material Serialization flags.
enum eMaterialSerializeFlags
{
  //! Serialize write.
  eMaterialSerializeFlags_Write = niBit(0),
  //! Serialize read.
  eMaterialSerializeFlags_Read = niBit(1),
  //! Serialize all the textures data in the file.
  eMaterialSerializeFlags_Textures = niBit(2),
  //! Compress the content for best hardware usage.
  eMaterialSerializeFlags_Compress = niBit(3),
  //! Serialize only the name of the materials.
  eMaterialSerializeFlags_NamesOnly = niBit(4),
  //! Read only if the material was alreadu in the library.
  eMaterialSerializeFlags_ReadExistingOnly = niBit(5),
  //! Don't serialize the texture's mip maps.
  //! \remark Mip maps will be recomputed when the library/material is loaded.
  eMaterialSerializeFlags_TextureNoMipMaps = niBit(19),
  //! Texture max res value lower bit. (4 bits value)
  //! \remark (FLAGS|(RES<<20)), RES is the maximum resolution
  //!         between 0 and 15 the maximum resolution is 2^RES.
  eMaterialSerializeFlags_TextureMaxResLo = niBit(20),
  //! Texture max res value higher bit. (4 bits value)
  eMaterialSerializeFlags_TextureMaxResHi = niBit(24),
  //! Texture compression value lower bit. (7 bits value)
  //! \remark (FLAGS|(COMP<<25)), COMP should be a value between 0 and 100, 0 == no loss compression
  eMaterialSerializeFlags_TextureCompressionLo = niBit(25),
  //! Texture compression value higher bit. (7 bits value)
  eMaterialSerializeFlags_TextureCompressionHi = niBit(31),
  //! \internal
  eMaterialSerializeFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Material Serialization flags type.
typedef ni::tU32 tMaterialSerializeFlags;

#define eMaterialSerializeFlags_TextureCompression(VAL) (ni::Clamp(VAL,0,100)<<25)
#define eMaterialSerializeFlags_TextureMaxResIndex(VAL) (ni::Clamp(VAL,0,15)<<20)
#define eMaterialSerializeFlags_GetTextureCompression(FLAGS) (((FLAGS)>>25)&127)
#define eMaterialSerializeFlags_GetTextureMaxResIndex(FLAGS) (((FLAGS)>>20)&15)

//! Material library sink.
//! {DispatchWrapper}
struct iMaterialLibrarySink : public iUnknown
{
  niDeclareInterfaceUUID(iMaterialLibrarySink,0xdd70858c,0xd1f5,0x459a,0x99,0x63,0x7f,0xe6,0x1d,0xb4,0x36,0x98)

  //! Called when a material is added.
  //! {Optional}
  virtual void __stdcall OnMaterialLibrarySink_MaterialAdded(iMaterialLibrary* apLib, iMaterial* apMat) = 0;
  //! Called when a material is removed.
  //! {Optional}
  virtual void __stdcall OnMaterialLibrarySink_MaterialRemoved(iMaterialLibrary* apLib, iMaterial* apMat) = 0;
};

//! Material library sink list;
typedef ni::SinkList<iMaterialLibrarySink>  tMaterialLibrarySinkLst;


//! Material library interface.
struct iMaterialLibrary : public iUnknown
{
  niDeclareInterfaceUUID(iMaterialLibrary,0xbded3ee6,0xbb4c,0x45a8,0x94,0x97,0xe5,0x8c,0x38,0x71,0xb6,0x09)

  //! Get the sink list.
  //! {Property}
  virtual tMaterialLibrarySinkLst* __stdcall GetSinkList() const = 0;

  //! Copy the material library.
  //! \param apSrc is the source library to copy
  //! \param abCloneMats sets whether the materials will be cloned, or just their pointer transfered.
  virtual ni::tBool __stdcall Copy(const iMaterialLibrary* apSrc, tBool abCloneMats) = 0;
  //! Clone the material library.
  virtual iMaterialLibrary* __stdcall Clone(tBool abCloneMats) const = 0;

  //! Remove all materials.
  virtual ni::tBool __stdcall ClearMaterials() = 0;
  //! Add a material.
  virtual ni::tBool __stdcall AddMaterial(ni::iMaterial* apMaterial) = 0;
  //! Remove a material.
  virtual ni::tBool __stdcall RemoveMaterial(ni::iMaterial* apMaterial) = 0;

  //! Get the number of materials.
  //! {Property}
  virtual ni::tU32 __stdcall GetNumMaterials() const = 0;
  //! Get the material at the specified index.
  //! {Property}
  virtual ni::iMaterial* __stdcall GetMaterial(ni::tU32 anIndex) const = 0;
  //! Get the material from the specified name.
  //! {Property}
  virtual ni::iMaterial* __stdcall GetMaterialFromName(ni::iHString* ahspName) const = 0;
  //! Get the index of the specified material.
  //! {Property}
  virtual ni::tU32 __stdcall GetMaterialIndex(ni::iMaterial* apMaterial) const = 0;

  //! Serialize the library in a datatable.
  virtual ni::tBool __stdcall SerializeDataTable(ni::iDataTable* apDT, tMaterialSerializeFlags aFlags) = 0;
  //! Serialize the library in a file.
  virtual ni::tBool __stdcall SerializeFile(ni::iFile* apFile, tMaterialSerializeFlags aFlags) = 0;

  //! Set the material library's base path.
  //! {Property}
  virtual void __stdcall SetBasePath(iHString* ahspBasePath) = 0;
  //! Get the material library's base path.
  //! {Property}
  virtual iHString* __stdcall GetBasePath() const = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IMATERIALLIBRARY_2996328_H__
