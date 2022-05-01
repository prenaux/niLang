#ifndef __ITEXTURE_28471243_H__
#define __ITEXTURE_28471243_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {

enum eLock;

/** \addtogroup niUI
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
//! Texture flags.
enum eTextureFlags
{
  //! Texture default.
  eTextureFlags_Default = 0,
  //! Texture is in system memory.
  //! \remark Textures in system memory also implements the iBitmap interface matching it's
  //!     type.
  eTextureFlags_SystemMemory = niBit(0),
  //! Texture is a render target.
  //! \remark Excludes Dynamic and DepthStencil flags.
  eTextureFlags_RenderTarget = niBit(1),
  //! Texture is a depth stencil.
  //! \remark Excludes Dynamic and RenderTarget flags.
  eTextureFlags_DepthStencil = niBit(2),
  //! Texture is a dynamic and so can be locked.
  //! \remark Excludes DepthStencil and RenderTarget flags.
  eTextureFlags_Dynamic = niBit(3),
  //! Texture has mip maps.
  eTextureFlags_MipMaps = niBit(4),
  //! Texture has automatically generated mip maps.
  //! \remark This will automatically add eTextureFlags_MipMaps.
  //! \remark Mipmaps will be generated only when the level 0 surface is modified.
  eTextureFlags_AutoGenMipMaps = niBit(5),
  //! The texture is a surface. It cant be bound as a source texture to be rasterized,
  //! but can be used for blitting, locked and as render target.
  eTextureFlags_Surface = niBit(6),
  //! Is a sub texture.
  eTextureFlags_SubTexture = niBit(7),
  //! Overlay texture. An overlay texture is a texture that will use only clamp
  //! addressing without mipmaps, and that is generaly non-pow2.
  eTextureFlags_Overlay = niBit(8),
  //! Virtual texture. The actual texture data are provided through bind only.
  //! \see ni::iDeviceResource::Bind
  eTextureFlags_Virtual = niBit(9),
  //! Texture is an antialiased render target with 4 samples.
  eTextureFlags_RTAA4Samples = niBit(10),
  //! Texture is an antialiased render target with 8 samples.
  eTextureFlags_RTAA8Samples = niBit(11),
  //! Texture is an antialiased render target with 16 samples.
  eTextureFlags_RTAA16Samples = niBit(12),
  //! All RTAA flags, can be used to test whether a texture is antialiased.
  eTextureFlags_RTAA_All = eTextureFlags_RTAA4Samples|eTextureFlags_RTAA8Samples|eTextureFlags_RTAA16Samples,
  //! Render target is flipped.
  //! \remark Shader projection matrices should be flipped when rendering in a
  //!         render target that has this flags set. The fixed/plain rendering
  //!         pipeline does the flipping automatically.
  eTextureFlags_RTFlipped = niBit(15),
  //! \internal
  eTextureFlags_ForceDWORD = 0xFFFFFFFF
};

//! Texture flags. \see ni::eTextureFlags
typedef tU32 tTextureFlags;

//! Texture blit flags.
enum eTextureBlitFlags
{
  //! No flags
  eTextureBlitFlags_None = 0,
  //! Bilinear filtering.
  eTextureBlitFlags_BilinearFilter = niBit(0),
  //! Optimization flags that notify that the bliting will happen frequently.
  //! \remark The flag is used as an hint for the implementation to cache some data.
  eTextureBlitFlags_Frequent = niBit(1),
  //! \internal
  eTextureBlitFlags_ForceDWORD = 0xFFFFFFFF
};

//! Texture blit flags. \see ni::eTextureBlitFlags
typedef tU32 tTextureBlitFlags;

//////////////////////////////////////////////////////////////////////////////////////////////
//! Texture interface
//! \remark Bindable
struct iTexture : public iDeviceResource
{
  niDeclareInterfaceUUID(iTexture,0x604af5db,0x7590,0x444a,0x94,0x7c,0xd3,0x20,0xbb,0x95,0xf7,0x8c)

  //! Get the texture type.
  //! {Property}
  virtual eBitmapType __stdcall GetType() const = 0;
  //! Get the texture width.
  //! {Property}
  virtual tU32 __stdcall GetWidth() const = 0;
  //! Get the texture height.
  //! {Property}
  virtual tU32 __stdcall GetHeight() const = 0;
  //! Get the texture depth.
  //! {Property}
  virtual tU32 __stdcall GetDepth() const = 0;
  //! Get the texture's pixel format.
  //! {Property}
  virtual iPixelFormat* __stdcall GetPixelFormat() const = 0;
  //! Get the number of mipmaps of the texture.
  //! {Property}
  virtual tU32 __stdcall GetNumMipMaps() const = 0;
  //! Get the texture flags.
  //! {Property}
  virtual tTextureFlags __stdcall GetFlags() const = 0;
  //! Get a sub texture.
  //! {Property}
  virtual iTexture* __stdcall GetSubTexture(tU32 anIndex) const = 0;
};

//===========================================================================
//
//  OpenGL interfaces
//
//===========================================================================
struct iGLTexture : public iTexture
{
  niDeclareInterfaceUUID(iGLTexture,0xa960dac0,0x7f0e,0x0000,0xc8,0x91,0xb3,0xb1,0xff,0x7f,0x00,0x00);
  //! Get the OpenGL handle.
  //! {Property}
  virtual tU32 __stdcall GetGLHandle() const = 0;
  //! Get the OpenGL FBO handle.
  //! {Property}
  virtual tU32 __stdcall GetGLFBOHandle() const = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __ITEXTURE_28471243_H__
