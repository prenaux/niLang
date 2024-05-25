#ifndef __IIMAGEMAP_9138729_H__
#define __IIMAGEMAP_9138729_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */

//! Image map serialization flags.
enum eImageMapSerializeFlags {
  //! Serialize write
  eImageMapSerializeFlags_Write = niBit(0),
  //! Serialize read
  eImageMapSerializeFlags_Read = niBit(1),
  //! \internal
  eImageMapSerializeFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Image map serialization flags type. \see ni::eImageMapSerializeFlags
typedef tU32 tImageMapSerializeFlags;

//! Image map interface.
struct iImageMap : public iUnknown
{
  niDeclareInterfaceUUID(iImageMap,0xa5a4c988,0xbc75,0x4a36,0xb0,0x54,0xb9,0x6c,0xb9,0xd2,0x54,0x1b)

  //########################################################################################
  //! \name Pages
  //########################################################################################
  //! @{

  //! Set the maximum number of pages.
  //! {Property}
  //! \remark Zero, the default value, means no limit.
  virtual void __stdcall SetMaxNumPages(tU32 anMax) = 0;
  //! Get the maximum number of pages.
  //! {Property}
  virtual tU32 __stdcall GetMaxNumPages() const = 0;

  //! Set the resolution of a page.
  //! \remark Default is 1024 or the maximum texture size on the current hardware.
  //! {Property}
  virtual void __stdcall SetPageSize(tU32 anSize) = 0;
  //! Get the resolution of a page.
  //! {Property}
  virtual tU32 __stdcall GetPageSize() const = 0;

  //! Set the number of mipmaps of a page.
  //! {Property}
  //! \remark Reset by set page size if the number of mipmaps is != 0 (the default)
  virtual void __stdcall SetPageMipMaps(tU32 anNumMipMaps) = 0;
  //! Get the number of mipmaps of a page.
  //! {Property}
  virtual tU32 __stdcall GetPageMipMaps() const = 0;

  //! Set whether the mipmaps are computed for the whole page after an image
  //! has been added. That is regardless of whether the image added had mipmap
  //! itself.
  //! \remark Default is eFalse.
  //! {Property}
  virtual void __stdcall SetComputeMipMapsPerPage(tBool abComputeMipMapsPerPage) = 0;
  //! Get whether the mipmaps are computed for the whole page after an image
  //! has been added.
  //! {Property}
  virtual tBool __stdcall GetComputeMipMapsPerPage() const = 0;

  //! Set the page format.
  //! {Property}
  //! \remark The default is R8G8B8A8
  virtual void __stdcall SetPageFormat(iHString* ahspFormat) = 0;
  //! Get the page format.
  //! {Property}
  virtual iHString* __stdcall GetPageFormat() const = 0;
  //! Get the number of pages in the image map.
  //! {Property}
  virtual tU32 __stdcall GetNumPages() const = 0;
  //! Get the specified page in the image map.
  //! {Property}
  virtual iTexture* __stdcall GetPage(tU32 anIndex) const = 0;
  //! @}

  //########################################################################################
  //! \name Map
  //########################################################################################
  //! @{

  //! Clear the image map.
  virtual void __stdcall Clear() = 0;

  //! Set the default blend mode.
  //! {Property}
  virtual void __stdcall SetDefaultImageBlendMode(eBlendMode aMode) = 0;
  //! Get the default blend mode.
  //! {Property}
  virtual eBlendMode __stdcall GetDefaultImageBlendMode() const = 0;

  //! Set the default filtering.
  //! {Property}
  virtual void __stdcall SetDefaultImageFilter(tBool abFiltering) = 0;
  //! Get the default filtering.
  //! {Property}
  virtual tBool __stdcall GetDefaultImageFilter() const = 0;

  //! Get the number of images.
  //! {Property}
  virtual tU32 __stdcall GetNumImages() const = 0;
  //! Get the image at the specified index.
  //! {Property}
  virtual iOverlay* __stdcall GetImage(tU32 anIndex) const = 0;
  //! Get the index of the image at the specified index.
  //! {Property}
  virtual tU32 __stdcall GetImageIndex(iOverlay* apImage) const = 0;
  //! Get the first image with the given name.
  //! {Property}
  virtual iOverlay* __stdcall GetImageFromName(iHString* ahspName) const = 0;

  //! Add an image to the image map.
  //! \return An new image packed in the image map, if failed returns NULL.
  //! \remark The returned overlay is owned by the image map, when the image map is released or invalidated the overlay will be invalidated.
  virtual iOverlay* __stdcall AddImage(iHString* ahspName, iBitmap2D* apBitmap) = 0;
  //! Add an image to the image map loading from a resource.
  //! \param ahspName is the name of the image, if the name isnt not specified the resource path is used as name.
  //! \param ahspRes is the path of the resource.
  //! \return An new image packed in the image map, if failed returns NULL.
  //! \remark The returned overlay is owned by the image map, when the image map is released or invalidated the overlay will be invalidated.
  virtual iOverlay* __stdcall AddImageFromResource(iHString* ahspName, iHString* ahspRes) = 0;
  //! Add an image to the image map loading from a icon set folder.
  //! \param ahspName is the name of the image, if the name isnt not specified the resource path is used as name.
  //! \param ahspFolder is the base folder name.
  //! \param ahspRes is the path of the resource.
  //! \param anMaxRes is the maximum icon resolution to use.
  //! \param anMinRes is the minimum icon resolution to use.
  //! \return An new image packed in the image map, if failed returns NULL.
  //! \remark The returned overlay is owned by the image map, when the image map is released or invalidated the overlay will be invalidated.
  //! \remark This method will try to load all mip map levels from several folders, up to maxres down to minres, missing versions will
  //!     be generated automatically.
  //! \remark The path for the images is folder/[res]x[res]/res
  virtual iOverlay* __stdcall AddImageFromIconSet(iHString* ahspName, iHString* ahspFolder, iHString* ahspRes, tU32 anMaxRes, tU32 anMinRes) = 0;
  //! Remove an image from the image map.
  //! \remark Removing an image will invalidate the overlay and repack the cache, this operation
  //!     should be considered slow.
  virtual tBool __stdcall RemoveImage(iOverlay* apImage) = 0;
  //! @}

  //########################################################################################
  //! \name Serialize
  //########################################################################################
  //! @{

  //! Get whether the image map has been modified since the last serialize read.
  //! {Property}
  virtual tBool __stdcall GetShouldSerialize() const = 0;
  //! Serialize the image map to/from the specified file.
  virtual tBool __stdcall Serialize(ni::iFile* apFile, tImageMapSerializeFlags aFlags) = 0;
  //! @}
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IIMAGEMAP_9138729_H__
