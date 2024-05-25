#ifndef __REND3D9_54689745_H__
#define __REND3D9_54689745_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niUI.h>

#ifdef _DEBUG
#define D3D_DEBUG_INFO  // Needed for NVPerfHUD to work
#endif
#include "D3D9.h"
#include "D3DX9.h"

namespace ni {
/** \addtogroup niUI
 * @{
 */
/** \addtogroup niUI_Direct3D9
 * @{
 */


//! D3D9 resource type.
enum eD3D9ResourceType
{
  //! Unknown/invalid type.
  eD3D9ResourceType_Invalid = 0,
  //! Surface type.
  eD3D9ResourceType_Surface = 1,
  //! Volume type.
  eD3D9ResourceType_Volume = 2,
  //! 2D texture type.
  eD3D9ResourceType_Texture2D = 3,
  //! Cube texture type.
  eD3D9ResourceType_TextureCube = 4,
  //! Volume texture type.
  eD3D9ResourceType_TextureVolume = 5,
  //! Vertex array/buffer type.
  eD3D9ResourceType_VertexArray = 6,
  //! Index array/buffer type.
  eD3D9ResourceType_IndexArray = 7,
  //! \internal
  eD3D9ResourceType_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! D3D9 texture.
struct iD3D9Texture : public iTexture
{
  niDeclareInterfaceUUID(iD3D9Texture,0x2163c5e6,0x2342,0x4d26,0x96,0x85,0xdb,0x07,0x87,0x1b,0x42,0x22)
  //! Get the resource type.
  virtual eD3D9ResourceType __stdcall GetResourceType() const = 0;
  //! Get the base texture.
  //! {NoAutomation}
  virtual IDirect3DBaseTexture9* __stdcall GetTexture() const = 0;
  //! Get the surface.
  //! {NoAutomation}
  virtual IDirect3DSurface9* __stdcall GetSurface() const = 0;
  //! Get the sub texture face/slice.
  virtual tU32 __stdcall GetSubTextureFace() const = 0;
  //! Get the sub texture level.
  virtual tU32 __stdcall GetSubTextureLevel() const = 0;
  //! Get the surface at the specified level.
  //! {NoAutomation}
  virtual IDirect3DSurface9* __stdcall GetSurfaceLevel(tU32 anLevel) const = 0;
  //! Get the volume at the specified level.
  //! {NoAutomation}
  virtual IDirect3DVolume9* __stdcall GetVolumeLevel(tU32 anLevel) const = 0;
  //! Called when the device is lost.
  //! {NoAutomation}
  virtual tBool __stdcall OnLostDevice(IDirect3DDevice9* apDevice) = 0;
  //! Called when the device is reset.
  //! {NoAutomation}
  virtual tBool __stdcall OnResetDevice(IDirect3DDevice9* apDevice) = 0;

  //! {NoAutomation}
  virtual void __stdcall Cleanup() = 0;
  //! {NoAutomation}
  virtual tBool __stdcall InitializeBaseTexture(IDirect3DBaseTexture9* apTexture) = 0;
  //! {NoAutomation}
  virtual tBool __stdcall InitializeSurface(IDirect3DBaseTexture9* apTexture, IDirect3DSurface9* apSurface, tU32 anFace, tU32 anLevel) = 0;
  //! {NoAutomation}
  virtual tBool __stdcall InitializeVolume(IDirect3DVolumeTexture9* apTexture, IDirect3DVolume9* apVolume, tU32 anLevel) = 0;
  //! {NoAutomation}
  virtual tBool __stdcall Create(IDirect3DDevice9* apDevice, eBitmapType aType, D3DFORMAT aFormat, tU32 anWidth, tU32 anHeight, tU32 anDepth, tU32 anNumMipMaps, tTextureFlags aFlags) = 0;
};

//! D3D9 Lost device sink.
struct iD3D9LostDeviceSink : public iUnknown
{
  niDeclareInterfaceUUID(iD3D9LostDeviceSink,0x63eeea72,0x48df,0x48dd,0xbb,0x84,0xa4,0xc2,0x92,0xcf,0x69,0x14)
  //! Called when a device is lost.
  //! {NoAutomation}
  virtual void __stdcall OnD3D9LostDeviceSink_LostDevice(IDirect3DDevice9* apDevice) = 0;
  //! Called when a device is reset.
  //! {NoAutomation}
  virtual void __stdcall OnD3D9LostDeviceSink_ResetDevice(IDirect3DDevice9* apDevice) = 0;
};

//! D3D9 Lost device sink list.
typedef SinkList<iD3D9LostDeviceSink> tD3D9LostDeviceSinkLst;

//! D3D9 Graphics
struct iD3D9Graphics : public iUnknown
{
  niDeclareInterfaceUUID(iD3D9Graphics,0xf5541e92,0xf729,0x4e43,0x92,0x8b,0x83,0xd5,0x67,0x32,0xea,0xe2)
  //! Get the D3D9 object.
  //! {NoAutomation}
  virtual IDirect3D9* __stdcall GetD3D9() const = 0;
  //! Get the D3D9 device.
  //! {NoAutomation}
  virtual IDirect3DDevice9* __stdcall GetD3D9Device() const = 0;
  //! Get the device's caps.
  //! {NoAutomation}
  virtual const D3DCAPS9& __stdcall GetD3D9Caps() const = 0;
  //! Get the default d3d9 device states.
  //! {NoAutomation}
  virtual IDirect3DStateBlock9* __stdcall GetD3D9DefaultDeviceStates() const = 0;
  //! Get the default d3d9 pixel states.
  //! {NoAutomation}
  virtual IDirect3DStateBlock9* __stdcall GetD3D9DefaultPixelStates() const = 0;
  //! Get the default d3d9 vertex states.
  //! {NoAutomation}
  virtual IDirect3DStateBlock9* __stdcall GetD3D9DefaultVertexStates() const = 0;
  //! Creates a D3D9 surface.
  //! {NoAutomation}
  virtual IDirect3DSurface9* __stdcall CreateD3D9Surface(tU32 anW, tU32 anH, D3DFORMAT aFmt, D3DPOOL aPool) = 0;
  //! Lock a full D3D9 surface an return it has a memory bitmap.
  //! \remark IDirect3DSurface9::UnlockRect still needs to be called when you are done with the memory bitmap.
  //! \remark Once IDirect3DSurface9::UnlockRect is called the bitmap is considered invalid and using it will
  //!     result in access violations.
  //! {NoAutomation}
  virtual iBitmap2D* __stdcall LockD3D9Surface(IDirect3DSurface9* apSurf, DWORD aLockFlags) = 0;
  //! Creates a D3D9 surface from a bitmap.
  //! {NoAutomation}
  virtual IDirect3DSurface9* __stdcall CreateD3D9SurfaceFromBitmap(iBitmap2D* apBmp, D3DFORMAT aFmt, D3DPOOL aPool) = 0;
  //! Get the lost device sink list.
  //! {NoAutomation}
  virtual tD3D9LostDeviceSinkLst* __stdcall GetLostDeviceSinkList() const = 0;
  //! Creates an empty texture for which the appropriate iD3D9Texture Initialize method has still to be called.
  //! {NoAutomation}
  virtual iTexture* __stdcall CreateD3D9TextureEmpty(iHString* ahspName) = 0;
  //! Creates a texture that 'wraps' the specified IDirect3DTexture9 object.
  //! {NoAutomation}
  virtual iTexture* __stdcall CreateD3D9TextureBase(iHString* ahspName, IDirect3DBaseTexture9* apTexture) = 0;
};

//! Direct3D9 vertex array.
struct iD3D9VertexArray : public iVertexArray
{
  niDeclareInterfaceUUID(iD3D9VertexArray,0x7464cd37,0xbca9,0x4827,0xb6,0x58,0x38,0xb1,0x8b,0xa2,0xff,0x4f)
  //! Get the D3D vertex buffer object.
  //! {NoAutomation}
  virtual IDirect3DVertexBuffer9* __stdcall GetVertexBuffer() const = 0;
  //! Get the D3D9 FVF.
  virtual tU32 __stdcall GetD3D9FVF() const = 0;
  //! Called when a device is lost.
  //! {NoAutomation}
  virtual tBool __stdcall OnLostDevice(IDirect3DDevice9* apDevice) = 0;
  //! Called when a device is reset.
  //! {NoAutomation}
  virtual tBool __stdcall OnResetDevice(IDirect3DDevice9* apDevice) = 0;
};

//! Direct3D9 index array.
struct iD3D9IndexArray : public iIndexArray
{
  niDeclareInterfaceUUID(iD3D9IndexArray,0x327adec2,0x7718,0x4956,0xb3,0x8e,0xb8,0xe6,0x1d,0x0d,0x67,0x23)
  //! Get the D3D index buffer object.
  //! {NoAutomation}
  virtual IDirect3DIndexBuffer9* __stdcall GetIndexBuffer() const = 0;
  //! Called when a device is lost.
  //! {NoAutomation}
  virtual tBool __stdcall OnLostDevice(IDirect3DDevice9* apDevice) = 0;
  //! Called when a device is reset.
  //! {NoAutomation}
  virtual tBool __stdcall OnResetDevice(IDirect3DDevice9* apDevice) = 0;
};

//! Direct3D9 vertex shader.
struct iD3D9ShaderVertex : public iShader
{
  niDeclareInterfaceUUID(iD3D9ShaderVertex,0x929e82b2,0x2559,0x4aa5,0x82,0x26,0x7c,0x04,0x8e,0x6d,0xb9,0x63)
  //! Get the D3D vertex shader object.
  //! {NoAutomation}
  virtual IDirect3DVertexShader9* __stdcall GetVertexShader() const = 0;
  //! Get the shader data object.
  //! {NoAutomation}
  virtual ID3DXBuffer* __stdcall GetData() const = 0;
  //! Get whether the shader has constants.
  virtual tBool __stdcall GetHasConstants() const = 0;
};

//! Direct3D9 pixel shader.
struct iD3D9ShaderPixel : public iShader
{
  niDeclareInterfaceUUID(iD3D9ShaderPixel,0x46ee01f3,0x30b6,0x4222,0x90,0x4d,0x21,0x97,0xa1,0x1e,0xf2,0x37)
  //! Get the D3D pixel shader object.
  //! {NoAutomation}
  virtual IDirect3DPixelShader9* __stdcall GetPixelShader() const = 0;
  //! Get the shader data object.
  //! {NoAutomation}
  virtual ID3DXBuffer* __stdcall GetData() const = 0;
  //! Get whether the shader has constants.
  //! {NoAutomation}
  virtual tBool __stdcall GetHasConstants() const = 0;
};

//! Direct3D9 occlusion query.
struct iD3D9OcclusionQuery : public iOcclusionQuery
{
  niDeclareInterfaceUUID(iD3D9OcclusionQuery,0xb2c9c9e7,0x9ee0,0x4814,0xbd,0x93,0x45,0xa7,0xf9,0x78,0xc8,0x87)
  //! Get the D3D query object.
  //! {NoAutomation}
  virtual IDirect3DQuery9* __stdcall GetQuery() const = 0;
  //! Called when a device is lost.
  //! {NoAutomation}
  virtual tBool __stdcall OnLostDevice(IDirect3DDevice9* apDevice) = 0;
  //! Called when a device is reset.
  //! {NoAutomation}
  virtual tBool __stdcall OnResetDevice(IDirect3DDevice9* apDevice) = 0;
};

}

//--------------------------------------------------------------------------------------------
//
//  D3DX Utils
//
//--------------------------------------------------------------------------------------------
#ifdef niD3DXUtils

#include "d3dx9.h"
#pragma comment(lib,"d3dx9.lib")

namespace ni {

///////////////////////////////////////////////
#define NI_D3DX_INITFP                                                  \
  niCheckSilent(niIsOK(apFile),D3DERR_INVALIDCALL);                     \
  ni::Ptr<iFile> fpMem = ni::CreateFileMemoryAlloc((tSize)apFile->GetSize(),apFile->GetSourcePath()); \
  niCheckSilent(fpMem.IsOK(),D3DERR_NOTAVAILABLE);                      \
  fpMem->WriteFile(apFile->GetFileBase());

#define NI_D3DX_FP  fpMem->GetBase(), (UINT)fpMem->GetSize()

#define NI_D3DX_INITDATA                              \
  niCheckSilent(niIsOK(apFile),D3DERR_INVALIDCALL);   \
  ni::cCString  dataBuffer;                           \
  ni::cCString* data = apData ? apData : &dataBuffer; \
  while (1) {                                         \
    ni::tU32 c = apFile->ReadChar();                  \
    if (!c) break;                                    \
    *data += (c <= 255) ? (ni::cchar)c : '?';         \
  }

#define NI_D3DX_DATA  data->Chars(), data->Len()

///////////////////////////////////////////////
inline HRESULT D3DXCreateTexture(LPDIRECT3DDEVICE9 pDevice, iFile* apFile, LPDIRECT3DTEXTURE9 * ppTexture) {
  niCheckSilent(pDevice != NULL,D3DERR_INVALIDDEVICE);
  NI_D3DX_INITFP;
  return D3DXCreateTextureFromFileInMemory(pDevice, NI_D3DX_FP, ppTexture);
}

///////////////////////////////////////////////
inline HRESULT D3DXCreateTextureEx(LPDIRECT3DDEVICE9 pDevice,
                                   iFile* apFile,
                                   UINT Width,
                                   UINT Height,
                                   UINT MipLevels,
                                   DWORD Usage,
                                   D3DFORMAT Format,
                                   D3DPOOL Pool,
                                   DWORD Filter,
                                   DWORD MipFilter,
                                   D3DCOLOR ColorKey,
                                   D3DXIMAGE_INFO * pSrcInfo,
                                   PALETTEENTRY * pPalette,
                                   LPDIRECT3DTEXTURE9 * ppTexture)
{
  niCheckSilent(pDevice != NULL,D3DERR_INVALIDDEVICE);
  NI_D3DX_INITFP;
  return D3DXCreateTextureFromFileInMemoryEx(
      pDevice,
      NI_D3DX_FP,
      Width,
      Height,
      MipLevels,
      Usage,
      Format,
      Pool,
      Filter,
      MipFilter,
      ColorKey,
      pSrcInfo,
      pPalette,
      ppTexture);
}

///////////////////////////////////////////////
inline HRESULT D3DXCreateVolumeTexture(LPDIRECT3DDEVICE9 pDevice,
                                       iFile* apFile,
                                       LPDIRECT3DVOLUMETEXTURE9* ppVolumeTexture)
{
  niCheckSilent(pDevice != NULL,D3DERR_INVALIDDEVICE);
  NI_D3DX_INITFP;
  return D3DXCreateVolumeTextureFromFileInMemory(pDevice,NI_D3DX_FP,ppVolumeTexture);
}

///////////////////////////////////////////////
inline HRESULT D3DXCreateVolumeTextureEx(LPDIRECT3DDEVICE9 pDevice,
                                         iFile* apFile,
                                         UINT Width,
                                         UINT Height,
                                         UINT Depth,
                                         UINT MipLevels,
                                         DWORD Usage,
                                         D3DFORMAT Format,
                                         D3DPOOL Pool,
                                         DWORD Filter,
                                         DWORD MipFilter,
                                         D3DCOLOR ColorKey,
                                         D3DXIMAGE_INFO * pSrcInfo,
                                         PALETTEENTRY * pPalette,
                                         LPDIRECT3DVOLUMETEXTURE9 * ppVolumeTexture)
{
  niCheckSilent(pDevice != NULL,D3DERR_INVALIDDEVICE);
  NI_D3DX_INITFP;
  return D3DXCreateVolumeTextureFromFileInMemoryEx(
      pDevice,
      NI_D3DX_FP,
      Width,
      Height,
      Depth,
      MipLevels,
      Usage,
      Format,
      Pool,
      Filter,
      MipFilter,
      ColorKey,
      pSrcInfo,
      pPalette,
      ppVolumeTexture);
}

///////////////////////////////////////////////
inline HRESULT D3DXCreateEffect(LPDIRECT3DDEVICE9 pDevice,
                                iFile* apFile,
                                CONST D3DXMACRO * pDefines,
                                LPD3DXINCLUDE pInclude,
                                DWORD Flags,
                                LPD3DXEFFECTPOOL pPool,
                                LPD3DXEFFECT * ppEffect,
                                LPD3DXBUFFER * ppCompilationErrors,
                                cCString* apData = NULL)
{
  niCheckSilent(pDevice != NULL,D3DERR_INVALIDDEVICE);
  NI_D3DX_INITDATA;
  return ::D3DXCreateEffect(pDevice,
                            NI_D3DX_DATA,
                            pDefines,
                            pInclude,
                            Flags,
                            pPool,
                            ppEffect,
                            ppCompilationErrors);
}

///////////////////////////////////////////////
inline HRESULT D3DXCreateEffectEx(LPDIRECT3DDEVICE9 pDevice,
                                  iFile* apFile,
                                  CONST D3DXMACRO * pDefines,
                                  LPD3DXINCLUDE pInclude,
                                  LPCSTR pSkipConstants,
                                  DWORD Flags,
                                  LPD3DXEFFECTPOOL pPool,
                                  LPD3DXEFFECT * ppEffect,
                                  LPD3DXBUFFER * ppCompilationErrors,
                                  cCString* apData = NULL)
{
  niCheckSilent(pDevice != NULL,D3DERR_INVALIDDEVICE);
  NI_D3DX_INITDATA;
  return ::D3DXCreateEffectEx(pDevice,
                              NI_D3DX_DATA,
                              pDefines,
                              pInclude,
                              pSkipConstants,
                              Flags,
                              pPool,
                              ppEffect,
                              ppCompilationErrors);
}

///////////////////////////////////////////////
inline HRESULT D3DXCreateEffectCompiler(iFile* apFile,
                                        CONST D3DXMACRO * pDefines,
                                        LPD3DXINCLUDE pInclude,
                                        DWORD Flags,
                                        LPD3DXEFFECTCOMPILER * ppEffectCompiler,
                                        LPD3DXBUFFER * ppParseErrors,
                                        cCString* apData = NULL)
{
  NI_D3DX_INITDATA;
  return ::D3DXCreateEffectCompiler(NI_D3DX_DATA,
                                    pDefines,
                                    pInclude,
                                    Flags,
                                    ppEffectCompiler,
                                    ppParseErrors);
}

///////////////////////////////////////////////
inline HRESULT D3DXCompileShader(iFile* apFile,
                                 CONST D3DXMACRO* pDefines,
                                 LPD3DXINCLUDE pInclude,
                                 LPCSTR pFunctionName,
                                 LPCSTR pProfile,
                                 DWORD Flags,
                                 LPD3DXBUFFER* ppShader,
                                 LPD3DXBUFFER* ppErrorMsgs,
                                 LPD3DXCONSTANTTABLE * ppConstantTable,
                                 cCString* apData = NULL)
{
  NI_D3DX_INITDATA;
  return ::D3DXCompileShader(NI_D3DX_DATA,pDefines,pInclude,pFunctionName,pProfile,Flags,ppShader,ppErrorMsgs,ppConstantTable);
}

#undef NI_D3DX_INITFP
#undef NI_D3DX_FP
#undef NI_D3DX_INITDATA
#undef NI_D3DX_DATA

}
#endif

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
#endif // __REND3D9_54689745_H__
