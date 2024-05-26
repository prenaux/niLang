// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"

#ifdef GDRV_DX9

#include "Graphics.h"
#include "GDRV_DX9.h"
#include "GDRV_DX9_ModeSelector.h"
#include "GDRV_DX9_FormatMap.h"
#include "GDRV_StateCache.h"
#include "GDRV_Utils.h"
#include <niLang/Math/MathMatrix.h>
#include <niLang/Utils/TimerSleep.h>
#include "API/niUI_ModuleDef.h"
#include "FixedShaders.h"

#pragma comment(lib,"Advapi32.lib")
// #pragma comment(lib,"Msimg32.lib") // For TransparentBlt

Ptr<iBitmap2D> CaptureWindowToBitmap(iGraphics* apGraphics, HWND capture) {
  // get window dimensions
  RECT rect;
  ::GetClientRect(capture,&rect);

  LONG dx = rect.right - rect.left;
  LONG dy = rect.bottom - rect.top;

  WINDOWINFO winInfo;
  winInfo.cbSize = sizeof(winInfo);
  ::GetWindowInfo(capture, &winInfo);

  LONG leftOffset = winInfo.rcClient.left - winInfo.rcWindow.left;
  LONG topOffset = winInfo.rcClient.top - winInfo.rcWindow.top;

  // create BITMAPINFO structure
  // used by CreateDIBSection
  BITMAPINFO info;
  info.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
  info.bmiHeader.biWidth         = dx;
  info.bmiHeader.biHeight        = dy;
  info.bmiHeader.biPlanes        = 1;
  info.bmiHeader.biBitCount      = 24;
  info.bmiHeader.biCompression   = BI_RGB;
  info.bmiHeader.biSizeImage     = 0;
  info.bmiHeader.biXPelsPerMeter = 0;
  info.bmiHeader.biYPelsPerMeter = 0;
  info.bmiHeader.biClrUsed       = 0;
  info.bmiHeader.biClrImportant  = 0;

  // a bitmap handle and a pointer its bit data
  HBITMAP bitmap = 0;
  BYTE*   memory = 0;

  // create bitmap
  HDC device = GetDC(capture);
  bitmap = CreateDIBSection(device, &info, DIB_RGB_COLORS, (void**)&memory, 0, 0);
  ReleaseDC(capture, device);
  niCheck(bitmap && memory, NULL);

  // blit the contents of the desktop (winDC) to the bitmap (selected in memDC)
  HDC winDC = GetWindowDC(capture);
  HDC memDC = CreateCompatibleDC(winDC);
  SelectObject(memDC, bitmap);
  BitBlt(memDC, 0, 0, dx, dy, winDC, leftOffset, topOffset, SRCCOPY);
  DeleteDC(memDC);
  ReleaseDC(capture, winDC);

  // windows bitmap width which is aligned on 32bits
  int wbytes = (((24*dx + 31) & (~31))/8);

  // write the bitmap
  Ptr<iBitmap2D> ptrBmp = apGraphics->CreateBitmap2D(dx,dy,_A("B8G8R8"));
  {
    const tPtr ptrData = ptrBmp->GetData();
    const tU32 pitch = ptrBmp->GetPitch();
    niLoop(i, ptrBmp->GetHeight()) {
      tPtr sptr = memory + (wbytes*(dy-i-1));
      tPtr dptr = ptrData + (pitch * i);
      memcpy(dptr,sptr,pitch);
    }
  }

  // delete bitmap
  DeleteObject(bitmap);
  bitmap = 0;
  memory = 0;

  return ptrBmp;
}

// #pragma comment(lib,"D3DX9.lib")
typedef IDirect3D9* (WINAPI* tpfnDirect3DCreate9)(UINT SDKVersion);

#define MAX_TEXTUREUNIT     16
#define MAX_RT              4

// Required for device restore to work
//#define D3D9_CLEAR_BUFFERS_ON_CREATE_VA   // vertex array
#define D3D9_CLEAR_BUFFERS_ON_CREATE_IA // index array
#define D3D9_PRESENT_FLAGS          0
#define D3D9_SWAP_EFFECT            D3DSWAPEFFECT_DISCARD

static inline DWORD _GetD3D9BehaviorFlags(const D3DCAPS9& caps) {
  tU32 bf = D3DCREATE_MULTITHREADED|D3DCREATE_FPU_PRESERVE|D3DCREATE_DISABLE_DRIVER_MANAGEMENT;
  if ((caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 ||
      caps.VertexShaderVersion < D3DVS_VERSION(1,1))
  {
    bf |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
  }
  else {
    bf |= D3DCREATE_HARDWARE_VERTEXPROCESSING;

    //      if (GetD3D9Caps().DevCaps & D3DDEVCAPS_PUREDEVICE) {
    //          dwBehaviorFlags |= D3DCREATE_PUREDEVICE;
    //      }
  }
  return bf;
}

#define D3DRESET_TRACE      niPrintln

#define CAPS_INITIALIZED        niBit(31) // has the driver been initialized ??
#define CAPS_SUCCEEDED          niBit(30) // caps init setup
#define CAPS_OCCLUSIONQUERIES   niBit(0)
#define CAPS_DEPTHTEXTURE       niBit(1)
#define CAPS_HWINST             niBit(2)
#define CAPS_I16PPOPS           niBit(3)
#define CAPS_F16PPOPS           niBit(4)
#define CAPS_F32PPOPS           niBit(5)
#define CAPS_I16FILTER          niBit(6)
#define CAPS_F16FILTER          niBit(7)
#define CAPS_F32FILTER          niBit(8)
#define CAPS_OVERLAYTEXTURE     niBit(9)
#define CAPS_NONPOW2TEXTURE2D   niBit(10)
#define CAPS_NONPOW2TEXTURE3D   niBit(11)
#define CAPS_NONPOW2TEXTURECUBE niBit(12)
#define CAPS_TRANSPARENT_AA     niBit(14)

#define D3D9_FVFIA(FVF,IA)        ((tU64(FVF))|(tU64(IA)<<32))
#define D3D9_FVFIA_GetFVF(FVFIA)  ((tU32)(((tU64)(FVFIA))&0xFFFFFFFFULL))
#define D3D9_FVFIA_GetIA(FVFIA)   ((tU32)(((tU64)(FVFIA))>>32))

#define STATE_PROFILE_BLOCK(X)     niProfileBlock(X)
// #define STATE_PROFILE_BLOCK(X)

static const achar* _kaszD3D9Name = _A("D3D9");
static const achar* _kaszD3D9Desc = _A("Direct3D9 Graphics Driver");
static const achar* _kaszD3D9Device = _A("Default");

static D3DMATERIAL9 _d3d9Material;

#define D3D9_IsDeviceTexture(TEX) eTrue

#ifdef _DEBUG
#define D3D9_CheckContext(X)  if (mpD3D9Device == NULL) { niError(_A("Driver not initialized.")); return X; }
#else
#define D3D9_CheckContext(X)  if (mpD3D9Device == NULL) { return X; }
#endif

#ifdef _DEBUG
#define DevCallA(Msg, Func, Params)             \
  hr = apDev->Func Params ;                     \
  if (FAILED(hr)) {                             \
    D3D9ERROR();                                \
    niError(Msg);                               \
    return eFalse;                              \
  }
#define DevCall(Msg, Func, Params)              \
  hr = mpD3D9Device->Func Params ;              \
  if (FAILED(hr)) {                             \
    D3D9ERROR();                                \
    niError(Msg);                               \
    return eFalse;                              \
  }
#define DevCallBreak(Msg, Func, Params)         \
  hr = mpD3D9Device->Func Params ;              \
  if (FAILED(hr)) {                             \
    D3D9ERROR();                                \
    niError(Msg);                               \
    ni_debug_break();                           \
    return eFalse;                              \
  }
#else
#define DevCallA(Msg, Func, Params) apDev->Func Params
#define DevCall(Msg, Func, Params)  mpD3D9Device->Func Params
#define DevCallBreak(Msg, Func, Params) mpD3D9Device->Func Params
#endif

struct sD3D9Context : public sGraphicsContext<4,ImplRC<iGraphicsContextRT,eImplFlags_DontInherit1,iGraphicsContext> > {
  sRecti            mrectScissor;
  sRecti            mrectViewport;
  tU32              mnSyncCounter;

  sD3D9Context(iGraphics* apGraphics)
      : tGraphicsContextBase(apGraphics)
  {
    mrectViewport = sRecti::Null();
    mrectScissor = sRecti::Null();
    mnSyncCounter = 0;
  }

  ///////////////////////////////////////////////
  void __stdcall SetViewport(const sRecti& aVal) {
    mrectViewport = aVal;
    mnSyncCounter++;
  }
  sRecti __stdcall GetViewport() const {
    return mrectViewport;
  }

  ///////////////////////////////////////////////
  void __stdcall SetScissorRect(const sRecti& aVal) {
    mrectScissor = aVal;
    mnSyncCounter++;
  }
  sRecti __stdcall GetScissorRect() const {
    return mrectScissor;
  }
};

class cD3D9;

void D3D9_Clear(iGraphicsContext* apContext, const achar* aaszMsg);
void D3D9_FinalizeInitialize(iGraphicsContext* apContext, tBool abReset);
HRESULT D3D9_ResetContext(iGraphicsContext* apContext);
void D3D9_ContextDeviceLost(iGraphicsContext* apContext);
void D3D9_UpdateWindow(iGraphicsContext* apContext);
IDirect3D9* D3D9_GetD3D9(iGraphicsDriver* apDriver);
IDirect3DDevice9* D3D9_GetD3D9Device(iGraphicsDriver* apDriver);

/**************************************************************
 * MakeWindowTransparent(window, factor)
 *
 * A function that will try to make a window transparent
 * (layered) under versions of Windows that support that kind
 * of thing. Gracefully fails on versions of Windows that
 * don't.
 *
 * Returns FALSE if the operation fails.
 */

typedef DWORD (WINAPI *PSLWA)(HWND, DWORD, BYTE, DWORD);

static BOOL MakeWindowTransparent(HWND hWnd, COLORREF colorKey, unsigned char factor)
{
  static PSLWA pSetLayeredWindowAttributes = NULL;
  static BOOL initialized = FALSE;

  /* First, see if we can get the API call we need. If we've tried
   * once, we don't need to try again. */
  if (!initialized) {
    HMODULE hDLL = ::LoadLibrary (_A("user32"));
    pSetLayeredWindowAttributes =  (PSLWA) GetProcAddress(hDLL, "SetLayeredWindowAttributes");
    initialized = TRUE;
  }

  if (pSetLayeredWindowAttributes == NULL)
    return FALSE;

  /* Windows need to be layered to be made transparent. This is done
   * by modifying the extended style bits to contain WS_EX_LAYARED. */
  SetLastError(0);

  SetWindowLong(hWnd,
                GWL_EXSTYLE ,
                GetWindowLong(hWnd, GWL_EXSTYLE) |
                WS_EX_LAYERED);

  if (GetLastError())
    return FALSE;

  /* Now, we need to set the 'layered window attributes'. This
   * is where the alpha values get set. */
  return pSetLayeredWindowAttributes (hWnd,
                                      colorKey,
                                      factor,
                                      LWA_COLORKEY|LWA_ALPHA);
}

//--------------------------------------------------------------------------------------------
//
//  Utilities
//
//--------------------------------------------------------------------------------------------
#include <niLang/Platforms/Win32/Win32_DC.h>

enum eD3D9Cache {
  eD3D9Cache_VertexShader,
  eD3D9Cache_PixelShader,
  eD3D9Cache_VertexArray,
  eD3D9Cache_IndexArray,
  eD3D9Cache_InstArray,
  eD3D9Cache_Context,
  eD3D9Cache_ContextSyncCounter,
  eD3D9Cache_DepthStencil,
  eD3D9Cache_Rasterizer,
  eD3D9Cache_RasterizerDoubleSided,
  eD3D9Cache_RasterizerDepthOnly,
  eD3D9Cache_SamplerState0,
  eD3D9Cache_SamplerState1,
  eD3D9Cache_SamplerState2,
  eD3D9Cache_SamplerState3,
  eD3D9Cache_SamplerState4,
  eD3D9Cache_SamplerState5,
  eD3D9Cache_SamplerState6,
  eD3D9Cache_SamplerState7,
  eD3D9Cache_SamplerState8,
  eD3D9Cache_SamplerState9,
  eD3D9Cache_SamplerState10,
  eD3D9Cache_SamplerState11,
  eD3D9Cache_SamplerState12,
  eD3D9Cache_SamplerState13,
  eD3D9Cache_SamplerState14,
  eD3D9Cache_SamplerState15,
  eD3D9Cache_AlwaysOn, // ALWAYS ON CACHING...
  eD3D9Cache_MaterialChannel,
  eD3D9Cache_Material,
  eD3D9Cache_FixedStates,
  eD3D9Cache_Last,
};
struct sD3D9StateCache : sStateCache {
  sD3D9StateCache() : sStateCache(eD3D9Cache_Last,eD3D9Cache_AlwaysOn) {}
};

///////////////////////////////////////////////
#ifdef _DEBUG
inline void D3D9_SetRenderState(IDirect3DDevice9* apDevice, D3DRENDERSTATETYPE anRS, tU32 anVal) {
  DWORD val;
  apDevice->GetRenderState(anRS, &val);
  if (val == anVal) return;
  apDevice->SetRenderState(anRS, anVal);
}
inline void D3D9_SetSamplerState(IDirect3DDevice9* apDevice, tU32 anTextureUnit, D3DSAMPLERSTATETYPE anRS, tU32 anVal) {
  DWORD val;
  apDevice->GetSamplerState(anTextureUnit, anRS, &val);
  if (val == anVal) return;
  apDevice->SetSamplerState(anTextureUnit,anRS,anVal);
}
inline void D3D9_SetTextureStageState(IDirect3DDevice9* apDevice, tU32 anTextureUnit, D3DTEXTURESTAGESTATETYPE anRS, tU32 anVal) {
  DWORD val;
  apDevice->GetTextureStageState(anTextureUnit, anRS, &val);
  if (val == anVal) return;
  apDevice->SetTextureStageState(anTextureUnit,anRS,anVal);
}
#else
#define D3D9_SetRenderState(apDevice,anRS,anVal)                      apDevice->SetRenderState(anRS, anVal)
#define D3D9_SetSamplerState(apDevice,anTextureUnit,anRS,anVal)       apDevice->SetSamplerState(anTextureUnit,anRS,anVal)
#define D3D9_SetTextureStageState(apDevice,anTextureUnit,anRS,anVal)  apDevice->SetTextureStageState(anTextureUnit,anRS,anVal)
#endif

///////////////////////////////////////////////
void D3D9_SetDefaultStates(IDirect3DDevice9* apDevice, tU32 anNumTextureUnits)
{
  // Set default parameters of D3D explicitly
  D3D9_SetRenderState(apDevice,D3DRS_CLIPPLANEENABLE,FALSE);
  D3D9_SetRenderState(apDevice,D3DRS_FOGENABLE, FALSE);
  D3D9_SetRenderState(apDevice,D3DRS_LIGHTING, FALSE);
  D3D9_SetRenderState(apDevice,D3DRS_SPECULARENABLE, FALSE);
  D3D9_SetRenderState(apDevice,D3DRS_NORMALIZENORMALS, FALSE);
  D3D9_SetRenderState(apDevice,D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
  D3D9_SetRenderState(apDevice,D3DRS_LASTPIXEL, TRUE);
  D3D9_SetRenderState(apDevice,D3DRS_DITHERENABLE, TRUE);
  D3D9_SetRenderState(apDevice,D3DRS_FOGTABLEMODE, FALSE);
  D3D9_SetRenderState(apDevice,D3DRS_FOGVERTEXMODE, FALSE);
  D3D9_SetRenderState(apDevice,D3DRS_RANGEFOGENABLE, FALSE);
  D3D9_SetRenderState(apDevice,D3DRS_ENABLEADAPTIVETESSELLATION, FALSE);
  D3D9_SetRenderState(apDevice,D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);

  D3D9_SetRenderState(apDevice,D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
  D3D9_SetRenderState(apDevice,D3DRS_BLENDOP, D3DBLENDOP_ADD);

  D3D9_SetRenderState(apDevice,D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
  D3D9_SetRenderState(apDevice,D3DRS_SRCBLENDALPHA, D3DBLEND_ZERO);
  D3D9_SetRenderState(apDevice,D3DRS_DESTBLENDALPHA, D3DBLEND_ZERO);

  D3DXMATRIX identity; D3DXMatrixIdentity(&identity);
  apDevice->SetTransform(D3DTS_WORLD,&identity);
  apDevice->SetTransform(D3DTS_WORLD1,&identity);
  apDevice->SetTransform(D3DTS_WORLD2,&identity);
  apDevice->SetTransform(D3DTS_WORLD3,&identity);
  apDevice->SetTransform(D3DTS_VIEW,&identity);
  apDevice->SetTransform(D3DTS_PROJECTION,&identity);
  niLoop(i,ni::Min(8,anNumTextureUnits)) {
    apDevice->SetTransform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+i),
                           D3DXMatrixIdentity(&identity));
    apDevice->SetTextureStageState(i,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE);
  }
}

namespace ni {
///////////////////////////////////////////////
// Return a string from a D3D error code.
const achar* GetD3D9ErrorString(HRESULT hr)
{
  static cString strErr;

#define ERRORCASE(error) case error: strErr = _A(#error); break

  switch(hr)
  {
    ERRORCASE(D3DERR_WRONGTEXTUREFORMAT);
    ERRORCASE(D3DERR_UNSUPPORTEDCOLOROPERATION);
    ERRORCASE(D3DERR_UNSUPPORTEDCOLORARG);
    ERRORCASE(D3DERR_UNSUPPORTEDALPHAOPERATION);
    ERRORCASE(D3DERR_UNSUPPORTEDALPHAARG);
    ERRORCASE(D3DERR_TOOMANYOPERATIONS);
    ERRORCASE(D3DERR_CONFLICTINGTEXTUREFILTER);
    ERRORCASE(D3DERR_UNSUPPORTEDFACTORVALUE);
    ERRORCASE(D3DERR_CONFLICTINGRENDERSTATE);
    ERRORCASE(D3DERR_UNSUPPORTEDTEXTUREFILTER);
    ERRORCASE(D3DERR_CONFLICTINGTEXTUREPALETTE);
    ERRORCASE(D3DERR_DRIVERINTERNALERROR);
    ERRORCASE(D3DERR_NOTFOUND);
    ERRORCASE(D3DERR_MOREDATA);
    ERRORCASE(D3DERR_DEVICELOST);
    ERRORCASE(D3DERR_DEVICENOTRESET);
    ERRORCASE(D3DERR_NOTAVAILABLE);
    ERRORCASE(D3DERR_OUTOFVIDEOMEMORY);
    ERRORCASE(D3DERR_INVALIDDEVICE);
    ERRORCASE(D3DERR_INVALIDCALL);
    ERRORCASE(D3DERR_DRIVERINVALIDCALL);
    default: strErr = _A("D3DERR_UNKNOWN"); break;
  }

#undef ERRORCASE

  return strErr.Chars();
}
}

///////////////////////////////////////////////
void D3D9_GetFFPMaterial(const sMaterialDesc* apMaterial, D3DMATERIAL9& aMAT) {
  if (niFlagIs(apMaterial->mFlags,eMaterialFlags_DiffuseModulate)) {
    aMAT.Ambient = aMAT.Diffuse =
        (D3DCOLORVALUE&)(apMaterial->mChannels[eMaterialChannel_Base].mColor);
  }
  else {
    aMAT.Ambient = aMAT.Diffuse = (D3DCOLORVALUE&)(sColor4f::One());
  }
  aMAT.Emissive = (D3DCOLORVALUE&)(apMaterial->mChannels[eMaterialChannel_Emissive].mColor);
  aMAT.Specular = (D3DCOLORVALUE&)(apMaterial->mChannels[eMaterialChannel_Specular].mColor);

  //  aMAT.Specular.r = 1.0f;
  //  aMAT.Specular.g = 1.0f;
  //  aMAT.Specular.b = 1.0f;
  //  aMAT.Specular.a = 1.0f;
  aMAT.Power = apMaterial->mChannels[eMaterialChannel_Specular].mColor.w * 250.0f;
  //  aMAT.Power = 20;
}

///////////////////////////////////////////////
D3DPRIMITIVETYPE D3D9_GetPrimitivePRIMITIVETYPE(eGraphicsPrimitiveType aType) {
  niConvTableBegin(aType)
      niConvTableItem(eGraphicsPrimitiveType_PointList, D3DPT_POINTLIST);
  niConvTableItem(eGraphicsPrimitiveType_LineList, D3DPT_LINELIST);
  niConvTableItem(eGraphicsPrimitiveType_LineListAdjacency, D3DPT_LINELIST);
  niConvTableItem(eGraphicsPrimitiveType_LineStrip, D3DPT_LINESTRIP);
  niConvTableItem(eGraphicsPrimitiveType_LineStripAdjacency, D3DPT_LINESTRIP);
  niConvTableItem(eGraphicsPrimitiveType_TriangleList, D3DPT_TRIANGLELIST);
  niConvTableItem(eGraphicsPrimitiveType_TriangleListAdjacency, D3DPT_TRIANGLELIST);
  niConvTableItem(eGraphicsPrimitiveType_TriangleStrip, D3DPT_TRIANGLESTRIP);
  niConvTableItem(eGraphicsPrimitiveType_TriangleStripAdjacency, D3DPT_TRIANGLESTRIP);
  niConvTableItem(eGraphicsPrimitiveType_BlitList, D3DPT_TRIANGLELIST);
  niConvTableEnd(D3DPT_FORCE_DWORD)
      }

///////////////////////////////////////////////
tU32 D3D9_GetPrimitiveCount(D3DPRIMITIVETYPE pt, tU32 nNumVertex)
{
  tU32 nPrimCount = 0;
  switch(pt)
  {
    case D3DPT_POINTLIST:
      nPrimCount = nNumVertex;
      break;

    case D3DPT_LINELIST:
      {
        tU32 i = nNumVertex%2;
        if(i) nNumVertex -= i;
        nPrimCount = nNumVertex/2;
      }
      break;

    case D3DPT_LINESTRIP:
      nPrimCount = nNumVertex-1;
      break;

    case D3DPT_TRIANGLELIST:
      {
        tU32 i = nNumVertex%3;
        if(i) nNumVertex -= i;
        nPrimCount = nNumVertex/3;
      }
      break;

    case D3DPT_TRIANGLESTRIP:
      nPrimCount = nNumVertex-2;
      break;

    case D3DPT_TRIANGLEFAN:
      nPrimCount = nNumVertex-2;
      break;
  }
  return nPrimCount;
}

///////////////////////////////////////////////
tU32 D3D9_GetD3D9FVF(tFVF aFVF)
{
  tU32 ulFVF = 0;
  if (niFlagTest(aFVF, eFVF_PositionB4))
    ulFVF |= (niFlagTest(aFVF, eFVF_Indices))?D3DFVF_XYZB5|D3DFVF_LASTBETA_UBYTE4:D3DFVF_XYZB4;
  else if (niFlagTest(aFVF, eFVF_PositionB3))
    ulFVF |= (niFlagTest(aFVF, eFVF_Indices))?D3DFVF_XYZB4|D3DFVF_LASTBETA_UBYTE4:D3DFVF_XYZB3;
  else if (niFlagTest(aFVF, eFVF_PositionB2))
    ulFVF |= (niFlagTest(aFVF, eFVF_Indices))?D3DFVF_XYZB3|D3DFVF_LASTBETA_UBYTE4:D3DFVF_XYZB2;
  else if (niFlagTest(aFVF, eFVF_PositionB1))
    ulFVF |= (niFlagTest(aFVF, eFVF_Indices))?D3DFVF_XYZB2|D3DFVF_LASTBETA_UBYTE4:D3DFVF_XYZB1;
  else if (niFlagTest(aFVF, eFVF_Position))
    ulFVF |= (niFlagTest(aFVF, eFVF_Indices))?D3DFVF_XYZB1|D3DFVF_LASTBETA_UBYTE4:D3DFVF_XYZ;

  if (niFlagTest(aFVF, eFVF_Normal))     ulFVF |= D3DFVF_NORMAL;
  if (niFlagTest(aFVF, eFVF_PointSize))  ulFVF |= D3DFVF_PSIZE;
  if (niFlagTest(aFVF, eFVF_ColorA))     ulFVF |= D3DFVF_DIFFUSE;

  tU32 ulNumDims;
  tU32 ulNumTexCoos = eFVF_TexNumCoo(aFVF);
  for (tU32 i = 0; i < ulNumTexCoos; ++i)
  {
    ulNumDims = eFVF_TexCooDim(aFVF,i);
    if (ulNumDims == 1) ulFVF |= D3DFVF_TEXCOORDSIZE1(i);
    else if (ulNumDims == 3) ulFVF |= D3DFVF_TEXCOORDSIZE3(i);
    else if (ulNumDims == 4) ulFVF |= D3DFVF_TEXCOORDSIZE4(i);
    else /*if (ulNumDims == 2)*/ ulFVF |= D3DFVF_TEXCOORDSIZE2(i);
  }

  ulFVF |= ulNumTexCoos<<D3DFVF_TEXCOUNT_SHIFT;
  return ulFVF;
}

///////////////////////////////////////////////
inline sRecti D3D9_ClipBlitRect(tU32 anWidth, tU32 anHeight, const sRecti& aRect) {
  return sRecti(aRect.GetTopLeft().x, aRect.GetTopLeft().y,
                     (aRect.GetWidth() == 0) ? anWidth : aRect.GetWidth(),
                     (aRect.GetHeight() == 0) ? anHeight : aRect.GetHeight()).ClipRect(sRecti(0,0,anWidth,anHeight));
}

///////////////////////////////////////////////
inline eArrayUsage D3D9_GetArrayUsage(DWORD aUsage, D3DPOOL aPool) {
  if (aPool == D3DPOOL_SYSTEMMEM) {
    return eArrayUsage_SystemMemory;
  }
  else {
    if (niFlagIs(aUsage,D3DUSAGE_DYNAMIC)) {
      if (niFlagIs(aUsage,D3DUSAGE_WRITEONLY)) {
        return eArrayUsage_Dynamic;
      }
      else {
        return eArrayUsage_DynamicReadWrite;
      }
    }
    else {
      return eArrayUsage_Static;
    }
  }
  return eArrayUsage_ForceDWORD;
}

///////////////////////////////////////////////
inline DWORD D3D9_GetArrayUsageUSAGE(eArrayUsage aUsage) {
  niConvTableBegin(aUsage)
      niConvTableItem(eArrayUsage_Dynamic,D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY)
      niConvTableItem(eArrayUsage_DynamicReadWrite,D3DUSAGE_DYNAMIC)
      niConvTableItem(eArrayUsage_Static,0/*D3DUSAGE_WRITEONLY*/)
      niConvTableItem(eArrayUsage_SystemMemory,0)
      niConvTableEnd(0);
}

///////////////////////////////////////////////
inline D3DPOOL D3D9_GetArrayUsagePOOL(eArrayUsage aUsage) {
#pragma niNote("Here change the Arrays POOL")
  niConvTableBegin(aUsage)
      niConvTableItem(eArrayUsage_Dynamic,D3DPOOL_DEFAULT)
      niConvTableItem(eArrayUsage_DynamicReadWrite,D3DPOOL_DEFAULT)
      niConvTableItem(eArrayUsage_Static,D3DPOOL_MANAGED)
      niConvTableItem(eArrayUsage_SystemMemory,D3DPOOL_SYSTEMMEM)
      niConvTableEnd(D3DPOOL_FORCE_DWORD);
}

///////////////////////////////////////////////
inline DWORD D3D9_GetArrayUsageLockLOCK(eArrayUsage aUsage, eLock aLock) {
  tU32 nFlags = 0;

  if (aUsage == eArrayUsage_Dynamic || aUsage == eArrayUsage_DynamicReadWrite) {
    if (niFlagTest(aLock,eLock_Discard))
      nFlags |= D3DLOCK_DISCARD;
    if (niFlagTest(aLock,eLock_NoOverwrite))
      nFlags |= D3DLOCK_NOOVERWRITE;
  }

  if (niFlagTest(aLock,eLock_ReadOnly))
    nFlags |= D3DLOCK_READONLY;

  return nFlags;
}

///////////////////////////////////////////////
inline tTextureFlags D3D9_GetUSAGETextureFlags(DWORD aUsage)
{
  tTextureFlags ret = 0;
  if (niFlagIs(aUsage,D3DUSAGE_AUTOGENMIPMAP)) {
    ret |= eTextureFlags_AutoGenMipMaps;
  }
  if (niFlagIs(aUsage,D3DUSAGE_DEPTHSTENCIL)) {
    ret |= eTextureFlags_DepthStencil;
  }
  if (niFlagIs(aUsage,D3DUSAGE_DYNAMIC))
  {
    ret |= eTextureFlags_Dynamic;
  }
  if (niFlagIs(aUsage,D3DUSAGE_RENDERTARGET)) {
    ret |= eTextureFlags_RenderTarget;
  }
  return ret;
}

///////////////////////////////////////////////
inline DWORD D3D9_GetTextureFlagsUSAGE(tTextureFlags aFlags)
{
  DWORD ret = 0;
  if (niFlagIs(aFlags,eTextureFlags_AutoGenMipMaps)) {
    ret |= D3DUSAGE_AUTOGENMIPMAP;
  }
  if (niFlagIs(aFlags,eTextureFlags_DepthStencil)) {
    ret |= D3DUSAGE_DEPTHSTENCIL;
  }
  if (niFlagIs(aFlags,eTextureFlags_Dynamic)) {
    ret |= D3DUSAGE_DYNAMIC;
  }
  if (niFlagIs(aFlags,eTextureFlags_RenderTarget)) {
    ret |= D3DUSAGE_RENDERTARGET;
  }
  return ret;
}

///////////////////////////////////////////////
inline D3DPOOL D3D9_GetTextureFlagsPOOL(tTextureFlags aFlags) {
  if (niFlagIs(aFlags,eTextureFlags_Dynamic)) {
    return D3DPOOL_DEFAULT;
  }
  else if (niFlagIs(aFlags,eTextureFlags_RenderTarget) || niFlagIs(aFlags,eTextureFlags_DepthStencil)) {
    return D3DPOOL_DEFAULT;
  }
  else {
    return D3DPOOL_MANAGED;
  }
}

///////////////////////////////////////////////
inline D3DMULTISAMPLE_TYPE D3D9_GetD3DMULTISAMPLE(tTextureFlags aTexFlags)
{
  D3DMULTISAMPLE_TYPE ms = D3DMULTISAMPLE_NONE;
  if (niFlagIs(aTexFlags,eTextureFlags_RTAA4Samples)) {
    ms = D3DMULTISAMPLE_4_SAMPLES;
  }
  else if (niFlagIs(aTexFlags,eTextureFlags_RTAA8Samples)) {
    ms = D3DMULTISAMPLE_8_SAMPLES;
  }
  else if (niFlagIs(aTexFlags,eTextureFlags_RTAA16Samples)) {
    ms = D3DMULTISAMPLE_16_SAMPLES;
  }
  return ms;
}

///////////////////////////////////////////////
inline tTextureFlags D3D9_GetD3DMULTISAMPLETYPETextureFlags(D3DMULTISAMPLE_TYPE aType) {
  tTextureFlags nFlags = 0;
  switch (aType) {
    case D3DMULTISAMPLE_2_SAMPLES:
    case D3DMULTISAMPLE_3_SAMPLES:
    case D3DMULTISAMPLE_4_SAMPLES:  nFlags |= eTextureFlags_RTAA4Samples;   break;
    case D3DMULTISAMPLE_5_SAMPLES:
    case D3DMULTISAMPLE_6_SAMPLES:
    case D3DMULTISAMPLE_7_SAMPLES:
    case D3DMULTISAMPLE_8_SAMPLES:  nFlags |= eTextureFlags_RTAA8Samples;   break;
    case D3DMULTISAMPLE_9_SAMPLES:
    case D3DMULTISAMPLE_10_SAMPLES:
    case D3DMULTISAMPLE_11_SAMPLES:
    case D3DMULTISAMPLE_12_SAMPLES:
    case D3DMULTISAMPLE_13_SAMPLES:
    case D3DMULTISAMPLE_14_SAMPLES:
    case D3DMULTISAMPLE_15_SAMPLES:
    case D3DMULTISAMPLE_16_SAMPLES: nFlags |= eTextureFlags_RTAA16Samples;  break;
  }
  return nFlags;
}

///////////////////////////////////////////////
inline D3DRESOURCETYPE D3D9_GetD3DRESOURCETYPE(eD3D9ResourceType aResType) {
  niConvTableBegin(aResType)
      niConvTableItem(eD3D9ResourceType_Surface,       D3DRTYPE_SURFACE)
      niConvTableItem(eD3D9ResourceType_Texture2D,     D3DRTYPE_TEXTURE)
      niConvTableItem(eD3D9ResourceType_TextureCube,   D3DRTYPE_CUBETEXTURE)
      niConvTableItem(eD3D9ResourceType_TextureVolume, D3DRTYPE_VOLUMETEXTURE)
      niConvTableItem(eD3D9ResourceType_VertexArray,   D3DRTYPE_VERTEXBUFFER)
      niConvTableItem(eD3D9ResourceType_IndexArray,    D3DRTYPE_INDEXBUFFER)
      niConvTableEnd(D3DRTYPE_FORCE_DWORD);
}

///////////////////////////////////////////////
inline eD3D9ResourceType D3D9_GetResourceType(D3DRESOURCETYPE aResType) {
  niConvTableBegin(aResType)
      niConvTableItem(D3DRTYPE_SURFACE,          eD3D9ResourceType_Surface)
      niConvTableItem(D3DRTYPE_TEXTURE,          eD3D9ResourceType_Texture2D)
      niConvTableItem(D3DRTYPE_CUBETEXTURE,      eD3D9ResourceType_TextureCube)
      niConvTableItem(D3DRTYPE_VOLUMETEXTURE,    eD3D9ResourceType_TextureVolume)
      niConvTableItem(D3DRTYPE_VERTEXBUFFER,     eD3D9ResourceType_VertexArray)
      niConvTableItem(D3DRTYPE_INDEXBUFFER,      eD3D9ResourceType_IndexArray)
      niConvTableEnd(eD3D9ResourceType_Invalid);
}

//--------------------------------------------------------------------------------------------
//
//  D3D9 Vertex Declarations
//
//--------------------------------------------------------------------------------------------
typedef astl::vector<D3DVERTEXELEMENT9>         tD3DVERTEXELEMENT9Vec;
typedef tD3DVERTEXELEMENT9Vec::iterator         tD3DVERTEXELEMENT9VecIt;
typedef tD3DVERTEXELEMENT9Vec::const_iterator   tD3DVERTEXELEMENT9VecCIt;

inline void D3D9_VertexElement(tD3DVERTEXELEMENT9Vec& vDecl, tU32 aOffset, tU32 aType, tU32 aUsage, tU32 aUsageIndex, WORD anStream = 0)
{
  D3DVERTEXELEMENT9 mElement;
  mElement.Stream = anStream;
  mElement.Method = 0;
  mElement.Offset = (WORD)aOffset;
  mElement.Type = (BYTE)aType;
  mElement.Usage = (BYTE)aUsage;
  mElement.UsageIndex = (BYTE)aUsageIndex;
  vDecl.push_back(mElement);
}

inline void D3D9_VertexElementEnd(tD3DVERTEXELEMENT9Vec& vDecl)
{
  D3DVERTEXELEMENT9 mElement = D3DDECL_END();
  vDecl.push_back(mElement);
}

//////////////////////////////////////////////////////////////////////////////////////////////
class cD3D9VertexDeclarations
{
 public:
  struct sVertexDeclaration
  {
    tU64                            nFVFIA;
    tU32                            nD3D9FVF;
    IDirect3DVertexDeclaration9*    pVertexDeclaration9;

    ///////////////////////////////////////////////
    sVertexDeclaration() {
      nFVFIA = 0;
      pVertexDeclaration9 = NULL;
    }

    ///////////////////////////////////////////////
    ~sVertexDeclaration() {
    }

    ///////////////////////////////////////////////
    tBool Initialize(IDirect3DDevice9* pD3D9Device, tU64 anFVFIA) {
#define _VTDebug(X) //niPrintln(X)
#define _VTStream(OFFSET,TYPE,INDEX)                                    \
      D3D9_VertexElement(vDeclaration, OFFSET, D3DDECLTYPE_##TYPE, D3DDECLUSAGE_TEXCOORD, eVertexStreamIndex_##INDEX)

      tD3DVERTEXELEMENT9Vec           vDeclaration;

      _VTDebug(niFmt(_A("FVF, VertDecl: %s - %d\n"),FVFToString(D3D9_FVFIA_GetFVF(anFVFIA)).Chars(),(tU32)D3D9_FVFIA_GetIA(anFVFIA)));

      cFVFDescription FVF(D3D9_FVFIA_GetFVF(anFVFIA));

      vDeclaration.clear();   // Make sure no previous declaration are there
      if (FVF.HasPosition()) {
        _VTStream(0, FLOAT3, Position);
        if (FVF.HasWeights4()) {
          _VTStream(12, FLOAT4, Weights);
        }
        else if (FVF.HasWeights3()) {
          _VTStream(12, FLOAT3, Weights);
        }
        else if (FVF.HasWeights2()) {
          _VTStream(12, FLOAT2, Weights);
        }
        else if (FVF.HasWeights1()) {
          _VTStream(12, FLOAT1, Weights);
        }
      }

      if (FVF.HasIndices()) {
        _VTStream(FVF.GetIndicesOffset(), UBYTE4, Indices);
      }

      if (FVF.HasNormal()) {
        _VTStream(FVF.GetNormalOffset(), FLOAT3, Normal);
      }

      if (FVF.HasColorA()) {
        _VTStream(FVF.GetColorAOffset(), D3DCOLOR, ColorA);
      }

      //! Number of texture coordinates.
      for (tU32 i = 0; i < FVF.GetNumTexCoos(); ++i)
      {
        BYTE ulType = D3DDECLTYPE_FLOAT2;
        if (FVF.GetTexCooDim(i) == 1)   ulType = D3DDECLTYPE_FLOAT1;
        else if (FVF.GetTexCooDim(i) == 3)  ulType = D3DDECLTYPE_FLOAT3;
        else if (FVF.GetTexCooDim(i) == 4)  ulType = D3DDECLTYPE_FLOAT4;
        D3D9_VertexElement(vDeclaration,
                           FVF.GetTexCooOffset(i),
                           ulType,
                           D3DDECLUSAGE_TEXCOORD,
                           eVertexStreamIndex_Tex1+i);
      }

      D3D9_VertexElementEnd(vDeclaration);
      _VTDebug("D3D9_VertexElementEND\n");

      HRESULT hr = pD3D9Device->CreateVertexDeclaration(&vDeclaration[0], &pVertexDeclaration9);
      if (FAILED(hr))  {
        const achar* szError = GetD3D9ErrorString(hr);
        niWarning(niFmt("D3DERROR: '%s'. Cant initialize the vertex declaration: %s.",szError,niFlagsToChars(eFVF,FVF.GetFVF())));
        return eFalse;
      }

      nFVFIA = anFVFIA;
      nD3D9FVF = D3D9_GetD3D9FVF(FVF.GetFVF());
      return eTrue;
    }

    ///////////////////////////////////////////////
    tU64 GetFVFIA() const {
      return nFVFIA;
    }
  };
  typedef astl::map<tU64,sVertexDeclaration>      tVertexDeclarationMap;
  typedef tVertexDeclarationMap::iterator         tVertexDeclarationMapIt;
  typedef tVertexDeclarationMap::const_iterator   tVertexDeclarationMapCIt;

  tVertexDeclarationMap   mmapVertexDeclaration;
  sVertexDeclaration*     mpCurrentDecl;

  cD3D9VertexDeclarations() {
    mpCurrentDecl = NULL;
  }
  ~cD3D9VertexDeclarations() {
  }

  ///////////////////////////////////////////////
  void ResetVertexDeclaration() {
    mpCurrentDecl = NULL;
  }

  ///////////////////////////////////////////////
  void ClearVertexDeclarations() {
    for (tVertexDeclarationMapIt it = mmapVertexDeclaration.begin();
         it != mmapVertexDeclaration.end();
         ++it)
    {
      niSafeRelease(it->second.pVertexDeclaration9);
    }
    mmapVertexDeclaration.clear();
    mpCurrentDecl = NULL;
  }

  ///////////////////////////////////////////////
  sVertexDeclaration* GetVertexDeclaration(IDirect3DDevice9* apDevice, tU64 anFVFIA) {
    tVertexDeclarationMapIt it = mmapVertexDeclaration.find(anFVFIA);
    if (it == mmapVertexDeclaration.end()) {
      sVertexDeclaration decl;
      if (!decl.Initialize(apDevice,anFVFIA)) {
        niError(_A("Can't initialize new vertex declaration."));
        return NULL;
      }
      it = astl::upsert(mmapVertexDeclaration, decl.GetFVFIA(), decl);
    }
    return &it->second;
  }

  ///////////////////////////////////////////////
  tU32 UpdateVertexDeclaration(IDirect3DDevice9* apDevice, tFVF aFVF, tBool abHWInst)
  {
    tU64 nFVFIA = D3D9_FVFIA(aFVF,abHWInst?0:eInvalidHandle);

    if (!mpCurrentDecl || mpCurrentDecl->nFVFIA != nFVFIA) {
      sVertexDeclaration* pVertexDeclaration = GetVertexDeclaration(apDevice, nFVFIA);
      niAssert(pVertexDeclaration != NULL);
      apDevice->SetFVF(0);
      apDevice->SetVertexDeclaration(pVertexDeclaration->pVertexDeclaration9);
      mpCurrentDecl = pVertexDeclaration;
    }

    return mpCurrentDecl->nD3D9FVF;
  }
};

//--------------------------------------------------------------------------------------------
//
//  D3D9 Vertex Array
//
//--------------------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////////
// Vertex array
class cD3D9VertexArray : public ImplRC<iD3D9VertexArray,eImplFlags_DontInherit1,iVertexArray>
{
 public:
  cD3D9VertexArray(iGraphics* apGraphics, IDirect3DDevice9* apDevice, tU32 ulNumVert, tFVF aFVF, eArrayUsage Usage)
  {
    mpGraphics = apGraphics;
    mFVF.Setup(aFVF);
    mpVB = NULL;
    mnNumVertices = ulNumVert;
    mUsage = Usage;
    mbLocked = eFalse;
    mulD3D9FVF = D3D9_GetD3D9FVF(aFVF);

    niCheck(Create(apDevice,eFalse),;);

    mpGraphics->GetGenericDeviceResourceManager()->Register(this);
  }

  tBool __stdcall Create(IDirect3DDevice9* apDevice, tBool abReset) {
    IDirect3DDevice9* pDevice = D3D9_GetD3D9Device(mpGraphics->GetDriver());

    D3DPOOL Pool = D3D9_GetArrayUsagePOOL(mUsage);
    tU32 ulUsage = D3D9_GetArrayUsageUSAGE(mUsage);

    HRESULT hr = pDevice->CreateVertexBuffer(mFVF.GetStride()*mnNumVertices, ulUsage, mulD3D9FVF, Pool, &mpVB, NULL);
    if (FAILED(hr)) {
      D3D9ERROR();
      niError(niFmt(_A("Can't create vertex buffer, FVF:%s, NumVertex: %d, Stride: %d, D3D9FVF: %d, d3dPool: %d, d3dUsage: %d."),FVFToString(mFVF.GetFVF()).Chars(),mnNumVertices,mFVF.GetStride(),mulD3D9FVF,Pool,ulUsage));
      return eFalse;
    }

#ifdef D3D9_CLEAR_BUFFERS_ON_CREATE_VA
    if (!Clear()) {
      niWarning(niFmt(_A("Can't clear %s vertex array."),abReset?_A("reset"):_A("created")));
    }
#endif

    niInfo(niFmt(_A(">>> D3D9 Vertex buffer %s: FVF:%s, NumVertex: %d, Stride: %d, D3D9FVF: %d, d3dPool: %d, d3dUsage: %d."),abReset?_A("reset"):_A("created"),FVFToString(mFVF.GetFVF()).Chars(),mnNumVertices,mFVF.GetStride(),mulD3D9FVF,Pool,ulUsage));
    return eTrue;
  }

  tBool Clear() {
    tPtr lockPtr = Lock(0,0,eLock_Discard);
    if (lockPtr) {
      ni::MemSet(lockPtr,0,mFVF.GetStride()*mnNumVertices);
      Unlock();
    }
    return lockPtr != NULL;
  }

  ~cD3D9VertexArray()
  {
    Invalidate();
  }

  void __stdcall Invalidate()
  {
    if (!mpGraphics) return;
    Unlock();
    mpGraphics->GetGenericDeviceResourceManager()->Unregister(this);
    niSafeRelease(mpVB);
    mpGraphics = NULL;
  }

  ///////////////////////////////////////////////
  iDeviceResource* __stdcall Bind(iUnknown*) {
    if (!_Check())
      return NULL;
    return this;
  }

  iHString* __stdcall GetDeviceResourceName() const {
    return NULL;
  }

  tBool __stdcall IsOK() const {
    return mpGraphics != NULL;
  }

  //! Get the FVF of the vertex array.
  tFVF __stdcall GetFVF() const {
    return mFVF.GetFVF();
  }

  //! Get the number of vertices in the vertex array.
  tU32 __stdcall GetNumVertices() const {
    return mnNumVertices;
  }

  //! Get the usage of the given vertex array.
  eArrayUsage __stdcall GetUsage() const {
    return mUsage;
  }

  //! Lock vertex array memory to enable writting and reading in it.
  tPtr __stdcall Lock(tU32 ulFirst, tU32 ulNum, eLock aLock)
  {
    if (!_Check() || mbLocked)
      return NULL;

    if (ulNum == 0) ulNum = GetNumVertices()-ulFirst;

    tPtr pRet = NULL;
    tU32 ulLock = D3D9_GetArrayUsageLockLOCK(mUsage,aLock);

    HRESULT hr = mpVB->Lock(mFVF.GetStride()*ulFirst, mFVF.GetStride()*ulNum, (void**)&pRet, ulLock);
    if (FAILED(hr)) {
      niWarning(niFmt(_A("D3D9 Lock Error: %s."), GetD3D9ErrorString(hr)));
      return NULL;
    }

    mbLocked = eTrue;
    return pRet;
  }

  //! Unlock vertex array memory.
  tBool __stdcall Unlock()
  {
    if (!mbLocked) return eFalse;
    mpVB->Unlock();
    mbLocked = eFalse;
    return eTrue;
  }

  //! Return true if the array is locked.
  tBool __stdcall GetIsLocked() const
  {
    return mbLocked;
  }

  IDirect3DVertexBuffer9* __stdcall GetVertexBuffer() const { return mpVB; }
  tU32 __stdcall GetD3D9FVF() const { return mulD3D9FVF; }

  virtual tBool __stdcall OnLostDevice(IDirect3DDevice9* apDevice) {
    D3DPOOL pool = D3D9_GetArrayUsagePOOL(mUsage);
    if (pool != D3DPOOL_MANAGED && pool != D3DPOOL_SYSTEMMEM) {
      if (mpVB) {
        tU32 numRefs = mpVB->Release();
        niUnused(numRefs); niAssert(numRefs == 0);
        mpVB = NULL;
      }
    }
    return eTrue;
  }
  virtual tBool __stdcall OnResetDevice(IDirect3DDevice9* apDevice) {
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) {
    return eFalse;
  }
  virtual tBool __stdcall ResetDeviceResource() {
    return eFalse;
  }

  ///////////////////////////////////////////////
  tBool _Check() {
    if (mpGraphics && mpVB == NULL) {
      return Create(D3D9_GetD3D9Device(mpGraphics->GetDriver()), eTrue);
    }
    return mpGraphics != NULL;
  }

 private:
  iGraphics*              mpGraphics;
  IDirect3DVertexBuffer9* mpVB;
  cFVFDescription         mFVF;
  tU32                    mnNumVertices;
  tU32                    mulD3D9FVF;
  eArrayUsage             mUsage;
  tBool                   mbLocked;
};

//--------------------------------------------------------------------------------------------
//
//  D3D9 Index Array
//
//--------------------------------------------------------------------------------------------

inline void D3D9_Copy16To32(tU16* apSrc, tU32* apDest, tU32 anCount, const tU32 anBase) {
  while (anCount--) {
    *apDest++ = ((tU32)(*apSrc++))+anBase;
  }
}

inline void D3D9_Copy32To16(tU32* apSrc, tU16* apDest, tU32 anCount, const tU32 anBase) {
  while (anCount--) {
    *apDest++ = (tU16)((*apSrc++)-anBase);
  }
}

class cD3D9IndexArray : public ImplRC<iD3D9IndexArray,eImplFlags_DontInherit1,iIndexArray>
{
 public:
  iGraphics*              mpGraphics;
  eGraphicsPrimitiveType  mPrimitiveType;
  eArrayUsage             mUsage;
  tU32                    mnNumIndices;
  tU32                    mnMaxVertexIndex;
  const tU32              mnHwMaxVertexIndex;
  IDirect3DIndexBuffer9*  mpIB;
  struct {
    tU32*   pTemp;
    tPtr    pLocked;
    tU32    nFirstIndex;
    tU32    nNumIndices;
    tBool   bReadOnly;
  } mLock;
  struct sBlock {
    void reset(tU32 _firstIndex) {
      minVert = ~0;
      maxVert = 0;
      firstIndex = _firstIndex;
      numInds = 0;
    }
    tU32 vertRange() const {
      return maxVert-minVert;
    }
    tU32   minVert;
    tU32   maxVert;
    tU32   numInds;
    tU32   firstIndex;
  };
  astl::vector<sBlock> mvBlocks;

  inline tBool __stdcall IsIndex16() const {
    return
        (mnHwMaxVertexIndex <= 0xFFFF) ||
        ((mUsage == eArrayUsage_Static) && (mnMaxVertexIndex <= 0xFFFF));
  }
  inline tBool __stdcall IsSplitted() const {
    return IsIndex16() && (mnMaxVertexIndex >= (0xFFFF-2)) && (mnNumIndices > 10000);
  }

  cD3D9IndexArray(iGraphics* apGraphics, IDirect3DDevice9* apDevice,
                  eGraphicsPrimitiveType aPrimitiveType, tU32 anNumIndices,
                  tU32 anMaxVertexIndex, eArrayUsage aUsage,
                  const tU32 anHwMaxVertexIndex)
      : mnHwMaxVertexIndex(anHwMaxVertexIndex)
  {
    mPrimitiveType = eGraphicsPrimitiveType_Last;
    mUsage = eArrayUsage_Static;
    mnNumIndices = 0;
    mnMaxVertexIndex = 0;
    mpIB = NULL;
    mLock.pTemp = NULL;
    mLock.pLocked = NULL;
    mLock.nNumIndices = 0;
    mLock.bReadOnly = eFalse;

    mpGraphics = apGraphics;
    mUsage = aUsage;
    mnNumIndices = anNumIndices;
    mnMaxVertexIndex = anMaxVertexIndex;
    mPrimitiveType = aPrimitiveType;

    niCheck(Create(apDevice,eFalse),;);

    mpGraphics->GetGenericDeviceResourceManager()->Register(this);
  }
  ~cD3D9IndexArray() {
    Invalidate();
  }

  tBool __stdcall Create(IDirect3DDevice9* apDevice, tBool abReset) {
    D3DFORMAT format = IsIndex16() ? D3DFMT_INDEX16 : D3DFMT_INDEX32;
    DWORD usage = D3D9_GetArrayUsageUSAGE(mUsage);
    D3DPOOL pool = D3D9_GetArrayUsagePOOL(mUsage);
    HRESULT hr = apDevice->CreateIndexBuffer(mnNumIndices * (IsIndex16()?2:4),
                                             usage, format, pool, &mpIB, NULL);
    if (FAILED(hr)) {
      D3D9ERROR();
      niError(niFmt(_A("Can't create index buffer, NumIndices: %d, d3dPool: %d, d3dUsage: %d."),
                    mnNumIndices,pool,usage));
      return eFalse;
    }

#ifdef D3D9_CLEAR_BUFFERS_ON_CREATE_IA
    if (!Clear()) {
      niWarning(niFmt(_A("Can't clear %s index array."),abReset?_A("reset"):_A("created")));
    }
#endif

    mLock.pLocked = NULL;
    niInfo(niFmt(_A(">>> D3D9 Index buffer %s: %d bits, NumIndex: %d, d3dPool: %d, d3dUsage: %d."),abReset?_A("reset"):_A("created"),(IsIndex16()?16:32), mnNumIndices,pool,usage));
    return eTrue;
  }

  tBool Clear() {
    tPtr lockPtr = Lock(0,0,eLock_Discard);
    if (lockPtr) {
      ni::MemSet(lockPtr,0,mnNumIndices * (IsIndex16()?2:4));
      Unlock();
    }
    return lockPtr != NULL;
  }

  ///////////////////////////////////////////////
  iDeviceResource* __stdcall Bind(iUnknown*) {
    if (!_Check())
      return NULL;
    return this;
  }

  tBool __stdcall IsOK() const {
    return mpGraphics != NULL;
  }

  void __stdcall Invalidate() {
    if (!mpGraphics) return;
    Unlock();
    mnNumIndices = mnMaxVertexIndex = 0;
    mpGraphics->GetGenericDeviceResourceManager()->Unregister(this);
    niSafeRelease(mpIB);
    mpGraphics = NULL;
  }

  iHString* __stdcall GetDeviceResourceName() const {
    return NULL;
  }
  virtual tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) {
    return eFalse;
  }
  virtual tBool __stdcall ResetDeviceResource() {
    return eFalse;
  }

  virtual eGraphicsPrimitiveType __stdcall GetPrimitiveType() const {
    return mPrimitiveType;
  }
  virtual tU32 __stdcall GetNumIndices() const {
    return mnNumIndices;
  }
  virtual tU32 __stdcall GetMaxVertexIndex() const {
    return mnMaxVertexIndex;
  }
  virtual eArrayUsage __stdcall GetUsage() const {
    return mUsage;
  }

  virtual tPtr __stdcall Lock(tU32 anFirstIndex, tU32 anNumIndices, eLock aLock) {
    if (!_Check() || mLock.pLocked != NULL)
      return NULL;

    if (anNumIndices == 0)
      anNumIndices = GetNumIndices()-anFirstIndex;

    mLock.nFirstIndex = anFirstIndex;
    mLock.nNumIndices = anNumIndices;

    tU32 nLockFlags = D3D9_GetArrayUsageLockLOCK(mUsage,aLock);
    tU32 nIndexSize = IsIndex16() ? sizeof(tU16) : sizeof(tU32);
    HRESULT hr = mpIB->Lock(anFirstIndex*nIndexSize, anNumIndices*nIndexSize, (void**)&mLock.pLocked, nLockFlags);
    if (FAILED(hr)) {
      niWarning(niFmt(_A("D3D9 Lock Error: %s."), GetD3D9ErrorString(hr)));
      return NULL;
    }

    if (IsIndex16()) {
      mLock.pTemp = niTMalloc(tU32,mLock.nNumIndices);
      if (!niFlagIs(aLock,eLock_Discard)) {
        if (IsSplitted()) {
          tU32 doNumIndices = anNumIndices;
          tU32 doFirstIndex = anFirstIndex;
          niLoop(i,mvBlocks.size()) {
            const cD3D9IndexArray::sBlock& b = mvBlocks[i];
            if (b.firstIndex >= doFirstIndex+doNumIndices)
              continue;
            if (b.firstIndex+b.numInds < doFirstIndex)
              continue;

            const tU32 numIndices = ni::Min(b.numInds,doNumIndices);
            const tU32 firstIndex = ni::Max(b.firstIndex,doFirstIndex);
            D3D9_Copy16To32(
                ((tU16*)mLock.pLocked)+firstIndex,
                mLock.pTemp+firstIndex,
                doNumIndices,
                b.minVert);
            if (numIndices >= doNumIndices)
              break;
            doFirstIndex += numIndices;
            doNumIndices -= numIndices;
          }
        }
        else {
          D3D9_Copy16To32((tU16*)mLock.pLocked,mLock.pTemp,mLock.nNumIndices,0);
        }
      }
      mLock.bReadOnly = niFlagIs(aLock,eLock_ReadOnly);
    }

    return IsIndex16()? (tPtr)mLock.pTemp : mLock.pLocked;
  }

  virtual tBool __stdcall Unlock() {
    if (!mLock.pLocked) return eFalse;

    if (IsIndex16()) {
      if (!mLock.bReadOnly) {
        if (IsSplitted()) {
          niDebugFmt((_A("# SPLITTED: %d"),mLock.nNumIndices));
          sBlock b,p;
          mvBlocks.clear();
          tU32* l = mLock.pTemp;
          b.reset(mLock.nFirstIndex);
          niLoop(i,mLock.nNumIndices/3) {
            // process triangle
            tU32 tmin = ~0, tmax = 0;
            tU32 v = 0;
            niLoop(j,3) {
              v = *l++;
              tmin = ni::Min(tmin,v);
              tmax = ni::Max(tmax,v);
            }
            // invalid triangle to be in this index array ?
            if (tmax-tmin > mnHwMaxVertexIndex) {
              // degen triangle so nothing is drawn, better a hole
              // than a teared up triangle...
              (*(l-3)) = v;
              (*(l-3)) = v;
              (*(l-3)) = v;
              continue;
            }
            // add to the current block
            p = b;
            b.minVert = ni::Min(b.minVert,tmin);
            b.maxVert = ni::Max(b.maxVert,tmax);
            b.numInds += 3;
            // add the block ?
            const tBool isLast = (((tU32)(i+1)) == (mLock.nNumIndices/3));
            if (isLast || b.vertRange() >= mnHwMaxVertexIndex) {
              if (isLast) {
                p = b;
              }
              //                             niDebugFmt((_A("ADDED BLOCK[%d]: {F:%d,T:%d (%d),v:%d,V:%d}"),
              //                                       isLast,
              //                                       p.firstIndex,p.numInds/3,p.numInds,
              //                                        p.minVert,p.vertRange()));
              D3D9_Copy32To16(mLock.pTemp+p.firstIndex,
                              ((tU16*)mLock.pLocked)+p.firstIndex,
                              p.numInds,p.minVert);
              mvBlocks.push_back(p);
              b.reset(mLock.nFirstIndex+(i*3));
            }
          }
        }
        else {
          D3D9_Copy32To16(mLock.pTemp,(tU16*)mLock.pLocked,mLock.nNumIndices,0);
        }
      }
      niSafeFree(mLock.pTemp);
    }

    mpIB->Unlock();
    mLock.pLocked = NULL;
    return eTrue;
  }

  virtual tBool __stdcall GetIsLocked() const {
    return mLock.pLocked != NULL;
  }

  virtual IDirect3DIndexBuffer9* __stdcall GetIndexBuffer() const {
    return mpIB;
  }

  virtual tBool __stdcall OnLostDevice(IDirect3DDevice9* apDevice) {
    D3DPOOL pool = D3D9_GetArrayUsagePOOL(mUsage);
    if (pool != D3DPOOL_MANAGED && pool != D3DPOOL_SYSTEMMEM) {
      if (mpIB) {
        tU32 numRefs = mpIB->Release();
        niUnused(numRefs); niAssert(numRefs == 0);
        mpIB = NULL;
      }
    }
    return eTrue;
  }
  virtual tBool __stdcall OnResetDevice(IDirect3DDevice9* apDevice) {
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool _Check() {
    if (mpGraphics && mpIB == NULL) {
      return Create(D3D9_GetD3D9Device(mpGraphics->GetDriver()), eTrue);
    }
    return mpGraphics != NULL;
  }
};

//--------------------------------------------------------------------------------------------
//
//  D3D9 Textures
//
//--------------------------------------------------------------------------------------------
iD3D9Texture* _BindCastTex(iTexture* apTex) {
  return apTex ? (iD3D9Texture*)apTex->Bind(NULL) : NULL;
}

///
/// Bitmap Lock
///
struct sGDD3DTextureBitmapLock
{
 private:
  IDirect3DDevice9*       mpDev;
  QPtr<iD3D9Texture>    mptrTex;
  Ptr<iBitmap2D>    mptrBitmap;
  IDirect3DSurface9*      mpSurface;
  IDirect3DSurface9*      mpUpdateSurface;
  sRecti             mRect;
  D3DSURFACE_DESC         mDesc;
  DWORD                   mLock;

 public:
  ///////////////////////////////////////////////
  tBool IsOK() const {
    return mptrBitmap.IsOK();
  }

  iBitmap2D* operator -> () {
    return mptrBitmap;
  }
  iBitmap2D* GetBitmap() {
    return mptrBitmap;
  }

  ///////////////////////////////////////////////
  sGDD3DTextureBitmapLock(
      IDirect3DDevice9* apDev, iGraphics* apGraphics, iTexture* apTex,
      const sRecti& aRect, tU32 anLevel, DWORD aLock)
  {
    if (aLock != D3DLOCK_DISCARD && aLock != D3DLOCK_READONLY) {
      niError(_A("Only D3DLOCK_DISCARD (write-only) or D3DLOCK_READONLY lock modes can be used."));
      return;
    }

    mpDev = apDev;
    mRect = aRect;
    mLock = aLock;

    mptrTex = _BindCastTex(apTex);
    if (!mptrTex.IsOK())
      return;

    mpUpdateSurface = NULL;

    mpSurface = mptrTex->GetSurfaceLevel(anLevel);
    if (!mpSurface) return;
    mpSurface->GetDesc(&mDesc);

    HRESULT hr;
    sRecti lockRect = aRect;
    IDirect3DSurface9* pSurfaceToLock = mpSurface;
    D3DLOCKED_RECT locked;

    if (mDesc.Pool == D3DPOOL_DEFAULT/* && (!(mDesc.Usage & D3DUSAGE_DYNAMIC))*/) {
      tU32 nWidth  = (aLock == D3DLOCK_READONLY) ? apTex->GetWidth() : aRect.GetWidth();
      tU32 nHeight = (aLock == D3DLOCK_READONLY) ? apTex->GetHeight() : aRect.GetHeight();

      const achar* aszFmt = apTex->GetPixelFormat()->GetFormat();
      D3DFORMAT fmt = GetD3DFormatMap()->GetD3DFormat(aszFmt);
      hr = apDev->CreateOffscreenPlainSurface(nWidth,nHeight,
                                              fmt,
                                              D3DPOOL_SYSTEMMEM,
                                              &mpUpdateSurface,
                                              NULL);
      if (FAILED(hr)) {
        D3D9ERROR();
        niError(niFmt(_A("Can't create update surface (%dx%d - %s)."),
                      nWidth,nHeight,aszFmt));
        return;
      }
      pSurfaceToLock = mpUpdateSurface;

      if (aLock == D3DLOCK_READONLY) {
        hr = mpDev->GetRenderTargetData(mpSurface,mpUpdateSurface);
        if (FAILED(hr)) {
          D3D9ERROR();
          niError(_A("Can't copy initial surface content."));
          return;
        }
      }
      else {
        lockRect.MoveTo(sVec2i::Zero());
      }
    }
    else {
      if (aLock == D3DLOCK_DISCARD) {
        if (!(mDesc.Usage & D3DUSAGE_DYNAMIC))
          aLock = 0;
      }
    }

    hr = pSurfaceToLock->LockRect(&locked,(CONST RECT*)&lockRect,aLock);
    if (FAILED(hr)) {
      D3D9ERROR();
      niError(_A("Can't lock surface."));
      return;
    }

    mptrBitmap = apGraphics->CreateBitmap2DMemoryEx(
        aRect.GetWidth(),aRect.GetHeight(),apTex->GetPixelFormat(),
        locked.Pitch,(tPtr)locked.pBits,eFalse);

  }

  ///////////////////////////////////////////////
  ~sGDD3DTextureBitmapLock() {
    if (mpUpdateSurface) {
      mpUpdateSurface->UnlockRect();
      // update the surface with the new surface data
      if (mLock == D3DLOCK_DISCARD)  {
        HRESULT hr = mpDev->UpdateSurface(mpUpdateSurface,NULL,mpSurface,
                                          (CONST POINT*)&mRect.x);
        if (FAILED(hr)) {
          D3D9ERROR();
          niError(_A("Can't update texture content."));
          return;
        }
      }
      niSafeRelease(mpUpdateSurface);
      niSafeRelease(mpSurface);
    }
    else {
      if (mpSurface) {
        mpSurface->UnlockRect();
        niSafeRelease(mpSurface);
      }
    }
  }
};

struct sGDD3DTextureVolumeLock
{
  IDirect3DDevice9*       mpDev;
  QPtr<iD3D9Texture>    mptrTex;
  Ptr<iBitmap3D>    mptrBitmap;
  IDirect3DVolume9*       mpVolume;
  sVec3i               mPos;
  sVec3i               mSize;
  D3DVOLUME_DESC          mDesc;
  DWORD                   mLock;

  ///////////////////////////////////////////////
  tBool IsOK() const {
    return mptrBitmap.IsOK();
  }

  ///////////////////////////////////////////////
  iBitmap3D* operator -> () const {
    return mptrBitmap.ptr();
  }
  iBitmap3D* operator -> () {
    return mptrBitmap.ptr();
  }

  ///////////////////////////////////////////////
  sGDD3DTextureVolumeLock(IDirect3DDevice9* apDev,
                          iGraphics* apGraphics,
                          iTexture* apTex,
                          const sVec3i& aPos,
                          const sVec3i& aSize,
                          tU32 anLevel, DWORD aLock) {
    if (aLock != D3DLOCK_DISCARD && aLock != D3DLOCK_READONLY) {
      niError(_A("Only D3DLOCK_DISCARD (write-only) or D3DLOCK_READONLY lock modes can be used."));
      return;
    }

    mpDev = apDev;
    mLock = aLock;
    mPos = aPos;
    mSize = aSize;

    mptrTex = _BindCastTex(apTex);
    if (!mptrTex.IsOK())
      return;

    mpVolume = mptrTex->GetVolumeLevel(anLevel);
    if (!mpVolume) return;
    mpVolume->GetDesc(&mDesc);

    HRESULT hr;
    D3DBOX box;
    D3DLOCKED_BOX locked;
    if (aLock == D3DLOCK_DISCARD) {
      if (!(mDesc.Usage & D3DUSAGE_DYNAMIC))
        aLock = 0;
    }

    box.Left = mPos.x;
    box.Right = mPos.x+mSize.x;
    box.Top = mPos.y;
    box.Bottom = mPos.y+mSize.y;
    box.Front = mPos.z;
    box.Back = mPos.z+mSize.z;

    hr = mpVolume->LockBox(&locked,&box,aLock);
    if (FAILED(hr)) {
      D3D9ERROR();
      niError(_A("Can't lock volume."));
      return;
    }

    mptrBitmap = apGraphics->CreateBitmap3DMemoryEx(
        mSize.x,mSize.y,mSize.z,apTex->GetPixelFormat(),
        locked.RowPitch,locked.SlicePitch,(tPtr)locked.pBits,eFalse);

  }

  ///////////////////////////////////////////////
  ~sGDD3DTextureVolumeLock() {
    if (mpVolume) {
      mpVolume->UnlockBox();
      niSafeRelease(mpVolume);
    }
  }
};

class cD3D9Texture : public ImplRC<iD3D9Texture,eImplFlags_DontInherit1|eImplFlags_DontInherit2,iTexture,iDeviceResource>
{
  niBeginClass(cD3D9Texture);
 public:
  iGraphics*                          mpGraphics;
  eBitmapType                         mBitmapType;
  eD3D9ResourceType                 mResourceType;
  Ptr<iHString>                 mhspName;
  union {
    IDirect3DBaseTexture9*          mpTextureBase;
    IDirect3DTexture9*              mpTexture2D;
    IDirect3DCubeTexture9*          mpTextureCube;
    IDirect3DVolumeTexture9*        mpTexture3D;
  };
  union {
    IDirect3DSurface9*              mpSurface;
    IDirect3DVolume9*               mpVolume;
  };
  Ptr<iPixelFormat>             mptrPixelFormat;
  tU32                                mnSubTextureFace;
  tU32                                mnSubTextureLevel;
  tU32                                mnWidth, mnHeight, mnDepth;
  tU32                                mnNumMipMaps;
  tTextureFlags                       mnFlags;
  tBool                               mbJustReset;

  ///////////////////////////////////////////////
  cD3D9Texture(iGraphics* apGraphics, iHString* ahspName)
  {
    ZeroMembers();
    mpGraphics = apGraphics;
    mhspName = ahspName;
    apGraphics->GetTextureDeviceResourceManager()->Register(this);
  }

  ///////////////////////////////////////////////
  ~cD3D9Texture() {
    Invalidate();
  }

  ///////////////////////////////////////////////
  void __stdcall ZeroMembers() {
    mbJustReset = eFalse;
    mpGraphics = NULL;
    mhspName = NULL;
    mpTextureBase = NULL;
    mpSurface = NULL;
    mBitmapType = eBitmapType_ForceDWORD;
    mResourceType = eD3D9ResourceType_Invalid;
    mnNumMipMaps = mnWidth = mnHeight = mnDepth = 0;
    mnFlags = 0;
    mnSubTextureFace = 0;
    mnSubTextureLevel = 0;
  }

  ///////////////////////////////////////////////
  iDeviceResource* __stdcall Bind(iUnknown*) {
    return this;
  }

  ///////////////////////////////////////////////
  tBool __stdcall IsOK() const {
    return mpGraphics != NULL;
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
    if (mpGraphics) {
      mpGraphics->GetTextureDeviceResourceManager()->Unregister(this);
    }
    Cleanup(); // still might need mpGraphics here
    mpGraphics = NULL;
  }

  ///////////////////////////////////////////////
  void __stdcall Cleanup() {
    if (mpTextureBase) {
      mpTextureBase->Release();
      mpTextureBase = NULL;
    }
    if (mpSurface) {
      mpSurface->Release();
      mpSurface = NULL;
    }
    mBitmapType = eBitmapType_ForceDWORD;
    mResourceType = eD3D9ResourceType_Invalid;
    mnNumMipMaps = mnWidth = mnHeight = mnDepth = 0;
    mnFlags = 0;
    mnSubTextureFace = 0;
    mnSubTextureLevel = 0;
    mptrPixelFormat = NULL;
  }

  ///////////////////////////////////////////////
  tBool __stdcall InitializeBaseTexture(IDirect3DBaseTexture9* apTexture) {
    niCheck(niIsOK(mpGraphics),eFalse);

    Cleanup();

    HRESULT hr;
    mpTextureBase = apTexture;
    switch (mpTextureBase->GetType()) {
      case D3DRTYPE_TEXTURE:
        {
          mBitmapType = eBitmapType_2D;
          D3DSURFACE_DESC desc;
          hr = mpTexture2D->GetLevelDesc(0,&desc);
          if (FAILED(hr)) {
            D3D9ERROR();
            niError(_A("Can't get the 2d texture's level 0 description."));
            return eFalse;
          }
          mnWidth = desc.Width;
          mnHeight = desc.Height;
          mnFlags |= D3D9_GetUSAGETextureFlags(desc.Usage);
          mnFlags |= D3D9_GetD3DMULTISAMPLETYPETextureFlags(desc.MultiSampleType);
          mptrPixelFormat = mpGraphics->CreatePixelFormat(GetD3DFormatMap()->GetPixelFormat(desc.Format));
          if (!mptrPixelFormat.IsOK()) {
            niError(niFmt(_A("Can't create pixel format for D3DFormat '%d'."),desc.Format));
            return eFalse;
          }
          mnNumMipMaps = mpTexture2D->GetLevelCount()-1;
          mResourceType = eD3D9ResourceType_Texture2D;
          if (niFlagIs(mnFlags,eTextureFlags_RenderTarget) ||
              niFlagIs(mnFlags,eTextureFlags_DepthStencil))
          {
            hr = mpTexture2D->GetSurfaceLevel(0,&mpSurface);
            if (FAILED(hr)) {
              D3D9ERROR();
              niError(_A("Can't get the 2d texture's level 0 surface."));
              return eFalse;
            }
          }
          break;
        };
      case D3DRTYPE_CUBETEXTURE:
        {
          mBitmapType = eBitmapType_Cube;
          D3DSURFACE_DESC desc;
          hr = mpTextureCube->GetLevelDesc(0,&desc);
          if (FAILED(hr)) {
            D3D9ERROR();
            niError(_A("Can't get the cube texture's level 0 description."));
            return eFalse;
          }
          mnWidth = desc.Width;
          mnHeight = desc.Height;
          mnFlags |= D3D9_GetUSAGETextureFlags(desc.Usage);
          mnFlags |= D3D9_GetD3DMULTISAMPLETYPETextureFlags(desc.MultiSampleType);
          mptrPixelFormat = mpGraphics->CreatePixelFormat(GetD3DFormatMap()->GetPixelFormat(desc.Format));
          if (!mptrPixelFormat.IsOK()) {
            niError(niFmt(_A("Can't create pixel format for D3DFormat '%d'."),desc.Format));
            return eFalse;
          }
          mnNumMipMaps = mpTextureCube->GetLevelCount()-1;
          mResourceType = eD3D9ResourceType_TextureCube;
          break;
        };
      case D3DRTYPE_VOLUMETEXTURE:
        {
          mBitmapType = eBitmapType_3D;
          D3DVOLUME_DESC desc;
          hr = mpTexture3D->GetLevelDesc(0,&desc);
          if (FAILED(hr)) {
            D3D9ERROR();
            niError(_A("Can't get the 3d texture's level 0 description."));
            return eFalse;
          }
          mnWidth = desc.Width;
          mnHeight = desc.Height;
          mnDepth = desc.Depth;
          mnFlags |= D3D9_GetUSAGETextureFlags(desc.Usage);
          mptrPixelFormat = mpGraphics->CreatePixelFormat(GetD3DFormatMap()->GetPixelFormat(desc.Format));
          if (!mptrPixelFormat.IsOK()) {
            niError(niFmt(_A("Can't create pixel format for D3DFormat '%d'."),desc.Format));
            return eFalse;
          }
          mnNumMipMaps = mpTexture3D->GetLevelCount()-1;
          mResourceType = eD3D9ResourceType_TextureVolume;
          break;
        };
      default:
        niAssertUnreachable("Unreachable code.");
        return eFalse;
    }
    mpTextureBase->AddRef();
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall InitializeSurface(IDirect3DBaseTexture9* apTexture, IDirect3DSurface9* apSurface, tU32 anFace, tU32 anLevel) {
    niCheck(niIsOK(mpGraphics),eFalse);

    Cleanup();

    D3DSURFACE_DESC desc;
    mpSurface = apSurface;

    mBitmapType = eBitmapType_2D;
    HRESULT hr = mpSurface->GetDesc(&desc);
    if (FAILED(hr)) {
      D3D9ERROR();
      niError(_A("Can't get the surface's description."));
      return eFalse;
    }
    mnWidth = desc.Width;
    mnHeight = desc.Height;
    mnFlags = eTextureFlags_Surface;
    mnFlags |= D3D9_GetUSAGETextureFlags(desc.Usage);
    mnFlags |= D3D9_GetD3DMULTISAMPLETYPETextureFlags(desc.MultiSampleType);
    mptrPixelFormat = mpGraphics->CreatePixelFormat(GetD3DFormatMap()->GetPixelFormat(desc.Format));
    if (!mptrPixelFormat.IsOK()) {
      niError(niFmt(_A("Can't create pixel format for D3DFormat '%d'."),desc.Format));
      return eFalse;
    }

    mResourceType = eD3D9ResourceType_Surface;
    mpSurface->AddRef();

    mpTextureBase = apTexture;
    if (mpTextureBase) {
      mnNumMipMaps = mpTextureBase->GetLevelCount()-1;
      niAssert(mnNumMipMaps >= 0 && mnNumMipMaps < 0xFFFF);
      mpTextureBase->AddRef();
    }
    else {
      mnNumMipMaps = 0;
    }

    mnSubTextureFace = anFace;
    mnSubTextureLevel = anLevel;

    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall InitializeVolume(IDirect3DVolumeTexture9* apTexture, IDirect3DVolume9* apVolume, tU32 anLevel) {
    niCheck(niIsOK(mpGraphics),eFalse);

    Cleanup();

    mBitmapType = eBitmapType_3D;
    mpVolume = apVolume;
    D3DVOLUME_DESC desc;
    HRESULT hr = mpVolume->GetDesc(&desc);
    if (FAILED(hr)) {
      D3D9ERROR();
      niError(_A("Can't get the volume's description."));
      return eFalse;
    }

    mnWidth = desc.Width;
    mnHeight = desc.Height;
    mnFlags = eTextureFlags_Surface;
    mnFlags |= D3D9_GetUSAGETextureFlags(desc.Usage);
    mptrPixelFormat = mpGraphics->CreatePixelFormat(GetD3DFormatMap()->GetPixelFormat(desc.Format));
    if (!mptrPixelFormat.IsOK()) {
      niError(niFmt(_A("Can't create pixel format for D3DFormat '%d'."),desc.Format));
      return eFalse;
    }

    mResourceType = eD3D9ResourceType_Volume;
    mpVolume->AddRef();

    mpTextureBase = apTexture;
    if (mpTextureBase) {
      mnNumMipMaps = mpTextureBase->GetLevelCount()-1;
      niAssert(mnNumMipMaps >= 0 && mnNumMipMaps < 0xFFFF);
      mpTextureBase->AddRef();
    }
    else {
      mnNumMipMaps = 0;
    }

    mnSubTextureLevel = anLevel;
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall Create(IDirect3DDevice9* apDevice, eBitmapType aType, D3DFORMAT aFormat, tU32 anWidth, tU32 anHeight, tU32 anDepth, tU32 anNumMipMaps, tTextureFlags aFlags)
  {
    niCheck(niIsOK(mpGraphics),eFalse);

    Cleanup();

    D3DFORMAT fmt = aFormat;
    DWORD usage = D3D9_GetTextureFlagsUSAGE(aFlags);
    D3DPOOL pool = D3D9_GetTextureFlagsPOOL(aFlags);

    HRESULT hr;
    switch (aType) {
      case eBitmapType_2D:
        {
          IDirect3DTexture9* pTex;
          hr = apDevice->CreateTexture(anWidth,anHeight,anNumMipMaps+1,usage,fmt,pool,&pTex,NULL);
          if (FAILED(hr)) {
            D3D9ERROR();
            niError(_A("Create texture failed."));
            return eFalse;
          }
          if (!InitializeBaseTexture(pTex)) {
            niError(_A("Can't initialize 2d texture."));
            return eFalse;
          }
          pTex->Release(); // refcount in mpTexture*
          break;
        }
      case eBitmapType_Cube:
        {
          IDirect3DCubeTexture9* pCube;
          hr = apDevice->CreateCubeTexture(anWidth,anNumMipMaps+1,usage,fmt,pool,&pCube,NULL);
          if (FAILED(hr)) {
            D3D9ERROR();
            niError(_A("Create cube texture failed."));
            return eFalse;
          }
          if (!InitializeBaseTexture(pCube)) {
            niError(_A("Can't initialize cube texture."));
            return eFalse;
          }
          pCube->Release(); // refcount in mpTexture*
          break;
        }
      case eBitmapType_3D:
        {
          IDirect3DVolumeTexture9* pVol;
          hr = apDevice->CreateVolumeTexture(anWidth,anHeight,anDepth,anNumMipMaps+1,usage,fmt,pool,&pVol,NULL);
          if (FAILED(hr)) {
            D3D9ERROR();
            niError(_A("Create volume texture failed."));
            return eFalse;
          }
          if (!InitializeBaseTexture(pVol)) {
            niError(_A("Can't initialize volume texture."));
            return eFalse;
          }
          pVol->Release(); // refcount in mpTexture*
          break;
        }
    }

    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual eD3D9ResourceType __stdcall GetResourceType() const {
    return mResourceType;
  }

  ///////////////////////////////////////////////
  virtual IDirect3DBaseTexture9* __stdcall GetTexture() const {
    return mpTextureBase;
  }

  ///////////////////////////////////////////////
  virtual IDirect3DSurface9* __stdcall GetSurface() const {
    return mpSurface;
  }

  ///////////////////////////////////////////////
  virtual tU32 __stdcall GetSubTextureFace() const {
    return mnSubTextureFace;
  }

  ///////////////////////////////////////////////
  virtual tU32 __stdcall GetSubTextureLevel() const {
    return mnSubTextureLevel;
  }

  ///////////////////////////////////////////////
  iHString* __stdcall GetDeviceResourceName() const {
    return mhspName;
  }

  ///////////////////////////////////////////////
  virtual eBitmapType __stdcall GetType() const {
    return mBitmapType;
  }

  ///////////////////////////////////////////////
  virtual tU32 __stdcall GetWidth() const {
    return mnWidth;
  }

  ///////////////////////////////////////////////
  virtual tU32 __stdcall GetHeight() const {
    return mnHeight;
  }

  ///////////////////////////////////////////////
  virtual tU32 __stdcall GetDepth() const {
    return mnDepth;
  }

  ///////////////////////////////////////////////
  virtual iPixelFormat* __stdcall GetPixelFormat() const {
    return mptrPixelFormat;
  }

  ///////////////////////////////////////////////
  virtual tU32 __stdcall GetNumMipMaps() const {
    return mnNumMipMaps;
  }

  ///////////////////////////////////////////////
  virtual tTextureFlags __stdcall GetFlags() const {
    return mnFlags;
  }

  ///////////////////////////////////////////////
  virtual iTexture* __stdcall GetSubTexture(tU32 anIndex) const {
    niCheck(niIsOK(mpGraphics),NULL);

#pragma niTodo("To be 100% clean these should be tracked into the parent texture and not recreated unnecessarly")

    Ptr<cD3D9Texture> ptrTex;
    HRESULT hr;

    switch (mResourceType) {
      case eD3D9ResourceType_TextureVolume:
      case eD3D9ResourceType_Texture2D:
        {   // no sub texture
          break;
        }
      case eD3D9ResourceType_TextureCube:
        {
          niCheck(anIndex < 6, NULL);
          IDirect3DSurface9* pSurf;
          hr = mpTextureCube->GetCubeMapSurface((D3DCUBEMAP_FACES)anIndex,0,&pSurf);
          if (FAILED(hr)) {
            D3D9ERROR();
            niError(niFmt(_A("Can't get cube map face '%d'."),anIndex));
            return NULL;
          }
          ptrTex = niNew cD3D9Texture(mpGraphics,NULL);
          if (!ptrTex->InitializeSurface(mpTextureCube,pSurf,anIndex,0)) {
            niError(niFmt(_A("Can't init cube map face '%d'."),anIndex));
            return NULL;
          }
          pSurf->Release();
          break;
        }
    }

    if (ptrTex.IsOK())
      ptrTex->mnFlags |= eTextureFlags_SubTexture;

    return ptrTex.GetRawAndSetNull();
  }

  ///////////////////////////////////////////////
  virtual IDirect3DSurface9* __stdcall GetSurfaceLevel(tU32 anLevel) const {
    niCheck(niIsOK(mpGraphics),NULL);

    IDirect3DSurface9* pSurf = NULL;
    HRESULT hr;
    switch (mResourceType) {
      case eD3D9ResourceType_Surface:
        if (!mpTextureBase || anLevel == 0) {
          if (anLevel != 0) {
            niError(_A("Can't get another level than zero for single surfaces."));
            return NULL;
          }
          pSurf = mpSurface;
          if (pSurf) pSurf->AddRef();
        }
        else {
          if (anLevel == mnSubTextureLevel) {
            pSurf = mpSurface;
            if (pSurf) pSurf->AddRef();
          }
          else {
            D3DRESOURCETYPE resType = mpTextureBase->GetType();
            if (resType == D3DRTYPE_TEXTURE) {
              hr = ((IDirect3DTexture9*)mpTextureBase)->GetSurfaceLevel(anLevel,&pSurf);
              if (FAILED(hr)) {
                D3D9ERROR();
                niError(niFmt(_A("Can't get surface level '%d'."),anLevel));
                return NULL;
              }
            }
            else if (resType == D3DRTYPE_CUBETEXTURE) {
              niAssert(mnSubTextureFace < 6);
              hr = ((IDirect3DCubeTexture9*)mpTextureBase)->GetCubeMapSurface((D3DCUBEMAP_FACES)mnSubTextureFace,anLevel,&pSurf);
              if (FAILED(hr)) {
                D3D9ERROR();
                niError(niFmt(_A("Can't get cube map face [%d] level [%d]."),mnSubTextureFace,anLevel));
                return NULL;
              }
              break;
            }
            else {
              niError(niFmt(_A("Can't get another level than zero for this resource type '%d'."),resType));
              return NULL;
            }
          }
        }
        break;
      case eD3D9ResourceType_Texture2D:
        hr = mpTexture2D->GetSurfaceLevel(anLevel,&pSurf);
        if (FAILED(hr)) {
          D3D9ERROR();
          niError(niFmt(_A("Can't get surface level '%d'."),anLevel));
          return NULL;
        }
        break;
      default:
        niError(niFmt(_A("Can't get a surface for the resource type '%d'."),anLevel));
        return NULL;
    }
    return pSurf;
  }

  ///////////////////////////////////////////////
  virtual IDirect3DVolume9* __stdcall GetVolumeLevel(tU32 anLevel) const {
    niCheck(niIsOK(mpGraphics),NULL);

    IDirect3DVolume9* pVol = NULL;
    HRESULT hr;
    switch (mResourceType) {
      case eD3D9ResourceType_TextureVolume:
        {
          hr = mpTexture3D->GetVolumeLevel(anLevel,&pVol);
          if (FAILED(hr)) {
            D3D9ERROR();
            niError(_A("Can't get the volume."));
            return NULL;
          }
          break;
        }
        break;
      default:
        niError(niFmt(_A("Can't get a volume for the resource type '%d'."),mResourceType));
        return NULL;
    }
    return pVol;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SetPriority(tU32 anPriority) {
    if (!mpTextureBase)
      return eFalse;
    mpTextureBase->SetPriority(anPriority);
    return eTrue;
  }
  virtual tU32 __stdcall GetPriority() const {
    if (!mpTextureBase)
      return 0;
    return mpTextureBase->GetPriority();
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SetLOD(tU32 anLOD) {
    if (!mpTextureBase)
      return eFalse;
    mpTextureBase->SetLOD(anLOD);
    return eTrue;
  }
  virtual tU32 __stdcall GetLOD() const {
    if (!mpTextureBase)
      return 0;
    return mpTextureBase->GetLOD();
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall ShouldResetWhenDeviceLost() {
    if (!niIsOK(mpGraphics) || mResourceType == eD3D9ResourceType_Invalid)
      return eFalse;
    if (D3D9_GetTextureFlagsPOOL(mnFlags) == D3DPOOL_DEFAULT) {
      return eTrue;
    }
    if (StrStartsWith(niHStr(mhspName),_A("__MainRT")) ||
        StrStartsWith(niHStr(mhspName),_A("__MainDS"))) {
      return eFalse; // managed by the context...
    }
    return eFalse;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) {
    tBool bJustReset = mbJustReset;
    if (abClearFlag)
      mbJustReset = eFalse;
    return bJustReset;
  }
  virtual tBool __stdcall ResetDeviceResource() {
    return eFalse;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall OnLostDevice(IDirect3DDevice9* apDevice) {
    if (ShouldResetWhenDeviceLost()) {
      if (mpSurface) {
        tU32 numRefs = mpSurface->Release();
        if (!(niFlagIs(mnFlags,eTextureFlags_RenderTarget) ||
              niFlagIs(mnFlags,eTextureFlags_DepthStencil))) {
          niUnused(numRefs); niAssert(numRefs == 0);
          // a ref is in the texture base if its a RT or DS...
        }
        mpSurface = NULL;
      }
      if (mpTextureBase) {
        tU32 numRefs = mpTextureBase->Release();
        niUnused(numRefs); niAssert(numRefs == 0);
        mpTextureBase = NULL;
      }
    }
    return eTrue;
  }


  ///////////////////////////////////////////////
  virtual tBool __stdcall OnResetDevice(IDirect3DDevice9* apDevice) {
    if (!ShouldResetWhenDeviceLost())
      return eTrue;

    mbJustReset = eTrue;
    if (mResourceType != eD3D9ResourceType_Texture2D &&
        mResourceType != eD3D9ResourceType_TextureCube &&
        mResourceType != eD3D9ResourceType_TextureVolume) {
      niError(_A("Can recreate only 2d, cube and volume textures."));
      return eFalse;
    }

    if (!mptrPixelFormat.IsOK()) {
      niError(_A("Can't recreate textures without a valid pixel format."));
      return eFalse;
    }

    D3DFORMAT fmt = GetD3DFormatMap()->GetD3DFormat(mptrPixelFormat->GetFormat());
    if (!Create(apDevice,mBitmapType,fmt,mnWidth,mnHeight,mnDepth,mnNumMipMaps,mnFlags)) {
      niError(_A("Can't recreate texture."));
      return eFalse;
    }
    return eTrue;
  }

  niEndClass(cD3D9Texture);
};

//--------------------------------------------------------------------------------------------
//
//  D3D9 Shaders
//
//--------------------------------------------------------------------------------------------

#define DevSamplerParamCall(Param,Val) DevCall(niFmt(_A("TU%d: Param %s (%d) = %d\n"),Unit,_A(#Param),D3DSAMP_##Param,Val),SetSamplerState,(Unit,D3DSAMP_##Param,Val))

static inline tBool SupportsVSHLSL(IDirect3DDevice9* apD3DDevice, const cchar* acszProfile)
{
  static const cchar* _cszHLSLVSProfiles[] = {
    "vs_1_1",
    "vs_2_0",
    "vs_2_a",
    "vs_3_0",
    NULL
  };
  cString strHLSLProfile = D3DXGetVertexShaderProfile(apD3DDevice);
  tU32 i;
  for (i = 0; _cszHLSLVSProfiles[i]; ++i) {
    if (StrEq(_cszHLSLVSProfiles[i],strHLSLProfile.Chars())) {
      break;
    }
  }
  if (_cszHLSLVSProfiles[i]) {
    for (tU32 j = 0; j <= i; ++j) {
      if (StrEq(_cszHLSLVSProfiles[j],acszProfile)) {
        return eTrue;
      }
    }
  }
  return eFalse;
}

static inline tBool SupportsPSHLSL(IDirect3DDevice9* apD3DDevice, const cchar* acszProfile)
{
  static const cchar* _cszHLSLPSProfiles[] = {
    "ps_1_1",
    "ps_1_2",
    "ps_1_3",
    "ps_1_4",
    "ps_2_0",
    "ps_2_a",
    "ps_2_b",
    "ps_3_0",
    NULL
  };
  cString strHLSLProfile = D3DXGetPixelShaderProfile(apD3DDevice);
  tU32 i;
  for (i = 0; _cszHLSLPSProfiles[i]; ++i) {
    if (StrEq(_cszHLSLPSProfiles[i],strHLSLProfile.Chars())) {
      break;
    }
  }
  if (_cszHLSLPSProfiles[i]) {
    for (tU32 j = 0; j <= i; ++j) {
      if (StrEq(_cszHLSLPSProfiles[j],acszProfile)) {
        return eTrue;
      }
    }
  }
  return eFalse;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Base
template <typename T>
class cD3D9ShaderBase : public T, public sShaderDesc
{
 public:
  typedef cD3D9ShaderBase tD3D9ShaderBase;

  cD3D9ShaderBase(iGraphics* apGraphics, iHString* ahspName, iDeviceResourceManager* apDevResMan, iHString* ahspProfileName, ID3DXBuffer* apData)
  {
    mpGraphics = apGraphics;
    mhspName = ahspName;
    mhspProfile = ahspProfileName;
    mpData = apData;
    mptrDevResMan = apDevResMan;
    mptrDevResMan->Register(this);
  }

  ~cD3D9ShaderBase()
  {
    Invalidate();
  }

  iHString* __stdcall GetDeviceResourceName() const
  {
    return mhspName;
  }
  virtual tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) {
    return eFalse;
  }
  virtual tBool __stdcall ResetDeviceResource() {
    return eFalse;
  }

  virtual void __stdcall Invalidate()
  {
    mptrConstants = NULL;
    if (!mptrDevResMan.IsOK()) return;
    mptrDevResMan->Unregister(this);
    mptrDevResMan = NULL;
    niSafeRelease(mpData);
  }

  iHString* __stdcall GetProfile() const
  {
    return this->mhspProfile;
  }

  iShaderConstants* __stdcall GetConstants() const
  {
    if (!mptrConstants.IsOK()) {
      niThis(cD3D9ShaderBase)->mptrConstants = mpGraphics->CreateShaderConstants(4096);
    }
    return mptrConstants;
  }

  ID3DXBuffer* __stdcall GetData() const {
    return mpData;
  }

  tBool __stdcall GetHasConstants() const {
    return mptrConstants.IsOK();
  }

  tPtr __stdcall GetDescStructPtr() const {
    const sShaderDesc* d = this;
    return (tPtr)d;
  }

 private:
  iGraphics*      mpGraphics;
  Ptr<iDeviceResourceManager>   mptrDevResMan;
  ID3DXBuffer*    mpData;
};

// Vertex program
class cD3D9ShaderVertex :
    public cD3D9ShaderBase<ImplRC<iD3D9ShaderVertex,eImplFlags_DontInherit1|eImplFlags_DontInherit2,iShader,iDeviceResource> >
{
 public:
  cD3D9ShaderVertex(iGraphics* apGraphics, iHString* ahspName, iDeviceResourceManager* apDevResMan, IDirect3DVertexShader9* pVS, iHString* ahspProfile, ID3DXBuffer* apData)
      : tD3D9ShaderBase(apGraphics,ahspName,apDevResMan,ahspProfile,apData)
  {
    mpVS = pVS;
  }

  ~cD3D9ShaderVertex()
  {
  }

  void __stdcall Invalidate()
  {
    niSafeRelease(mpVS);
    tD3D9ShaderBase::Invalidate();
  }

  //! Return the Shader unit on which it runs.
  eShaderUnit __stdcall GetUnit() const { return eShaderUnit_Vertex; }

  iHString* __stdcall GetCode() const { return mhspCode; }

  ///////////////////////////////////////////////
  iDeviceResource* __stdcall Bind(iUnknown*) {
    return this;
  }

  virtual IDirect3DVertexShader9* __stdcall GetVertexShader() const {
    return mpVS;
  }

 public:
  IDirect3DVertexShader9* mpVS;
  tHStringPtr mhspCode;
};


//////////////////////////////////////////////////////////////////////////////////////////////
// Pixel program
class cD3D9ShaderPixel :
    public cD3D9ShaderBase<ImplRC<iD3D9ShaderPixel,eImplFlags_DontInherit1|eImplFlags_DontInherit2,iShader,iDeviceResource> >
{
 public:
  cD3D9ShaderPixel(iGraphics* apGraphics, iHString* ahspName, iDeviceResourceManager* apDevResMan, IDirect3DPixelShader9* pPS, iHString* ahspProfile, ID3DXBuffer* apData)
      : tD3D9ShaderBase(apGraphics,ahspName,apDevResMan,ahspProfile,apData)
  {
    mpPS = pPS;
  }

  ~cD3D9ShaderPixel()
  {
  }

  void __stdcall Invalidate()
  {
    niSafeRelease(mpPS);
    tD3D9ShaderBase::Invalidate();
  }

  //! Return the Shader unit on which it runs.
  eShaderUnit __stdcall GetUnit() const { return eShaderUnit_Pixel; }

  iHString* __stdcall GetCode() const { return mhspCode; }

  ///////////////////////////////////////////////
  iDeviceResource* __stdcall Bind(iUnknown*) {
    return this;
  }

  virtual IDirect3DPixelShader9* __stdcall GetPixelShader() const {
    return mpPS;
  }

 public:
  IDirect3DPixelShader9*  mpPS;
  tHStringPtr             mhspCode;
};

//--------------------------------------------------------------------------------------------
//
//  D3D9 Occlusion Queries
//
//--------------------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////////
// cD3D9OcclusionQuery declaration.
class cD3D9OcclusionQuery : public ni::ImplRC<ni::iD3D9OcclusionQuery,ni::eImplFlags_DontInherit1,iOcclusionQuery,iDeviceResource>
{
  niBeginClass(cD3D9OcclusionQuery);

 public:
  //! Constructor.
  cD3D9OcclusionQuery(iGraphics* apGraphics) {
    mpGraphics = apGraphics;
    mpQuery = NULL;
    mStatus = eOcclusionQueryStatus_NotIssued;
    mnResult = eInvalidHandle;
    if (!_Create(D3D9_GetD3D9Device(mpGraphics->GetDriver()))) {
      niError(_A("Can't create occlusion query."));
      return;
    }
    mpGraphics->GetGenericDeviceResourceManager()->Register(this);
  }
  //! Destructor.
  ~cD3D9OcclusionQuery() {
    Invalidate();
  }

  //! Sanity check.
  ni::tBool __stdcall IsOK() const {
    niClassIsOK(cD3D9OcclusionQuery);
    return mpGraphics != NULL;
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
    if (!mpGraphics) return;
    mpGraphics->GetGenericDeviceResourceManager()->Unregister(this);
    niSafeRelease(mpQuery);
    mpGraphics = NULL;
  }

  ///////////////////////////////////////////////
  ni::tBool __stdcall _Create(IDirect3DDevice9* apDevice) {
    HRESULT hr = apDevice->CreateQuery(D3DQUERYTYPE_OCCLUSION, &mpQuery);
    return SUCCEEDED(hr);
  }

  ///////////////////////////////////////////////
  iHString* __stdcall GetDeviceResourceName() const {
    return NULL;
  }
  virtual tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) {
    return eFalse;
  }
  virtual tBool __stdcall ResetDeviceResource() {
    return eFalse;
  }

  ///////////////////////////////////////////////
  tBool __stdcall Begin() {
    if (!_Check()) return eFalse;
    HRESULT hr = mpQuery->Issue(D3DISSUE_BEGIN);
    if (SUCCEEDED(hr)) {
      mStatus = eOcclusionQueryStatus_Began;
      return eTrue;
    }
    else {
      mStatus = eOcclusionQueryStatus_NotIssued;
      return eFalse;
    }
  }

  ///////////////////////////////////////////////
  tBool __stdcall End() {
    if (!mpQuery) return eFalse;
    HRESULT hr = mpQuery->Issue(D3DISSUE_END);
    if (SUCCEEDED(hr)) {
      mStatus = eOcclusionQueryStatus_Pending;
      return eTrue;
    }
    else {
      mStatus = eOcclusionQueryStatus_NotIssued;
      return eFalse;
    }
  }

  ///////////////////////////////////////////////
  ni::eOcclusionQueryStatus __stdcall GetStatus(tBool abWait) {
    if (!mpQuery)
      mStatus = eOcclusionQueryStatus_Failed;
    if (mStatus == eOcclusionQueryStatus_Pending)
    {
      if (abWait)
      {
        while (1)
        {
          HRESULT hr = mpQuery->GetData(&mnResult,sizeof(mnResult),D3DGETDATA_FLUSH);
          if (hr == S_OK)
          {
            mStatus = eOcclusionQueryStatus_Successful;
            break;
          }
          else if (hr != S_FALSE)
          {
            D3D9ERROR();
            niError(_A("Query Direct3D error."));
            mStatus = eOcclusionQueryStatus_Failed;
            break;
          }
        }
      }
      else
      {
        HRESULT hr = mpQuery->GetData(&mnResult,sizeof(mnResult),D3DGETDATA_FLUSH);
        if (hr == S_OK)
        {
          mStatus = eOcclusionQueryStatus_Successful;
        }
        else if (hr == S_FALSE)
        {
          mStatus = eOcclusionQueryStatus_Pending;
        }
        else
        {
          D3D9ERROR();
          niError(_A("Query Direct3D error."));
          mStatus = eOcclusionQueryStatus_Failed;
        }
      }
    }
    return mStatus;
  }

  ///////////////////////////////////////////////
  tU32 __stdcall GetResult() const {
    if (!mpQuery || niThis(cD3D9OcclusionQuery)->GetStatus(eFalse) != eOcclusionQueryStatus_Successful)
      return eInvalidHandle;
    return mnResult;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall OnLostDevice(IDirect3DDevice9* apDevice) {
    if (mpQuery) {
      tU32 numRefs = mpQuery->Release();
      niUnused(numRefs); niAssert(numRefs == 0);
      mpQuery = NULL;
    }
    return eTrue;
  }
  virtual tBool __stdcall OnResetDevice(IDirect3DDevice9* apDevice) {
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual IDirect3DQuery9* __stdcall GetQuery() const {
    return mpQuery;
  }

  ///////////////////////////////////////////////
  iDeviceResource* __stdcall Bind(iUnknown*) {
    if (!_Check())
      return NULL;
    return this;
  }

  ///////////////////////////////////////////////
  tBool _Check() {
    if (mpGraphics && !mpQuery) {
      mnResult = 0;
      mStatus = eOcclusionQueryStatus_NotIssued;
      return _Create(D3D9_GetD3D9Device(mpGraphics->GetDriver()));
    }
    return mpGraphics != NULL;
  }

 private:
  iGraphics*              mpGraphics;
  IDirect3DQuery9*        mpQuery;
  eOcclusionQueryStatus   mStatus;
  tU32                    mnResult;
  niEndClass(cD3D9OcclusionQuery);
};

//--------------------------------------------------------------------------------------------
//
//  D3D9 Sampler states
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
D3DTEXTUREADDRESS D3D9_GetSamplerWarpTEXTUREADDRESS(const D3DCAPS9& aCaps, eSamplerWrap aWrap) {
  niConvTableBegin(aWrap)
      niConvTableItem(eSamplerWrap_Repeat,D3DTADDRESS_WRAP);
  niConvTableItem(eSamplerWrap_Mirror,D3DTADDRESS_MIRROR);
  niConvTableItem(eSamplerWrap_Clamp,D3DTADDRESS_CLAMP);
  niConvTableItem(eSamplerWrap_Border,D3DTADDRESS_BORDER);
  niConvTableEnd(D3DTADDRESS_WRAP);
}

///////////////////////////////////////////////
eSamplerWrap D3D9_GetTEXTUREADDRESSSamplerWarp(const D3DCAPS9& aCaps, D3DTEXTUREADDRESS aWrap) {
  niConvTableBegin(aWrap)
      niConvTableItem(D3DTADDRESS_WRAP,eSamplerWrap_Repeat);
  niConvTableItem(D3DTADDRESS_MIRROR,eSamplerWrap_Mirror);
  niConvTableItem(D3DTADDRESS_CLAMP,eSamplerWrap_Clamp);
  niConvTableItem(D3DTADDRESS_BORDER,eSamplerWrap_Border);
  niConvTableEnd(eSamplerWrap_Repeat);
}

static const tF32 _knFilterPoint_LodBias = 0;
static const tF32 _knFilterSmooth_LodBias = 0;
static const tU32 _knFilterSmooth_Anisotropy = 4;
static const tF32 _knFilterSharp_LodBias = -1;
static const tU32 _knFilterSharp_Anisotropy = 8;

///////////////////////////////////////////////
tBool D3D9_ApplySamplerStates(
    iGraphics* apGraphics,
    sD3D9StateCache& aCache,
    IDirect3DDevice9* apDev,
    const D3DCAPS9& aCaps,
    tU32 anTextureUnit,
    iTexture* apTexture,
    tIntPtr ahSS)
{
  if (!ahSS) {
    ahSS = eCompiledStates_SS_PointClamp;
  }

  const tBool hasMipMaps = (apTexture && apTexture->GetNumMipMaps());
  tBool bShouldUpdate = eFalse;
  aCache.ShouldUpdate(&bShouldUpdate,eD3D9Cache_SamplerState0+anTextureUnit,ahSS|(hasMipMaps<<31));
  if (!bShouldUpdate)
    return eTrue;

  iSamplerStates* pChanSS = apGraphics->GetCompiledSamplerStates(ahSS);
  niCheck(pChanSS != NULL,eFalse);
  const sSamplerStatesDesc* pStates = (const sSamplerStatesDesc*)pChanSS->GetDescStructPtr();

  {
    D3DTEXTUREFILTERTYPE filter, mipfilter;
    if (pStates->mFilter == eSamplerFilter_Point) {
      filter = D3DTEXF_POINT;
      mipfilter = D3DTEXF_POINT;
      D3D9_SetSamplerState(apDev,anTextureUnit,D3DSAMP_MIPMAPLODBIAS,ftoul(_knFilterPoint_LodBias));
    }
    else {
      const tU32 nAnisotropy = ni::Min(aCaps.MaxAnisotropy,_knFilterSmooth_Anisotropy);
      if (!nAnisotropy) {
        filter = D3DTEXF_LINEAR;
        mipfilter = D3DTEXF_LINEAR;
      }
      else {
        filter = D3DTEXF_ANISOTROPIC;
        mipfilter = D3DTEXF_ANISOTROPIC;
        D3D9_SetSamplerState(apDev,anTextureUnit,D3DSAMP_MAXANISOTROPY,
                             (pStates->mFilter == eSamplerFilter_Sharp || pStates->mFilter == eSamplerFilter_SharpPoint) ?
                             _knFilterSharp_Anisotropy :
                             _knFilterSmooth_Anisotropy);
      }
      D3D9_SetSamplerState(apDev,anTextureUnit,D3DSAMP_MIPMAPLODBIAS,
                           // Lod bias is only for sharp, SharpPoint doesnt need a lod bias
                           ftoul((pStates->mFilter == eSamplerFilter_Sharp) ?
                                 _knFilterSharp_LodBias :
                                 _knFilterSmooth_LodBias));
    }
    D3D9_SetSamplerState(apDev,anTextureUnit,D3DSAMP_MINFILTER,filter);
    D3D9_SetSamplerState(apDev,anTextureUnit,D3DSAMP_MAGFILTER,filter);
    D3D9_SetSamplerState(apDev,anTextureUnit,D3DSAMP_MIPFILTER,
                         hasMipMaps ? mipfilter : D3DTEXF_NONE);
  }

  D3D9_SetSamplerState(apDev,anTextureUnit,D3DSAMP_ADDRESSU,D3D9_GetSamplerWarpTEXTUREADDRESS(aCaps,pStates->mWrapS));
  D3D9_SetSamplerState(apDev,anTextureUnit,D3DSAMP_ADDRESSV,D3D9_GetSamplerWarpTEXTUREADDRESS(aCaps,pStates->mWrapT));
  D3D9_SetSamplerState(apDev,anTextureUnit,D3DSAMP_ADDRESSW,D3D9_GetSamplerWarpTEXTUREADDRESS(aCaps,pStates->mWrapR));

  D3D9_SetSamplerState(apDev,anTextureUnit,D3DSAMP_BORDERCOLOR,ULColorBuild(pStates->mcolBorder));
  return eTrue;
}

//--------------------------------------------------------------------------------------------
//
//  D3D9 DepthStencil states
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
D3DCMPFUNC D3D9_GetGraphicsCompareCMPFUNC(eGraphicsCompare aCmp) {
  niConvTableBegin(aCmp)
      niConvTableItem(eGraphicsCompare_Never, D3DCMP_NEVER);
  niConvTableItem(eGraphicsCompare_Equal, D3DCMP_EQUAL);
  niConvTableItem(eGraphicsCompare_NotEqual, D3DCMP_NOTEQUAL);
  niConvTableItem(eGraphicsCompare_Less, D3DCMP_LESS);
  niConvTableItem(eGraphicsCompare_LessEqual, D3DCMP_LESSEQUAL);
  niConvTableItem(eGraphicsCompare_Greater, D3DCMP_GREATER);
  niConvTableItem(eGraphicsCompare_GreaterEqual, D3DCMP_GREATEREQUAL);
  niConvTableItem(eGraphicsCompare_Always, D3DCMP_ALWAYS);
  niConvTableEnd(D3DCMP_ALWAYS);
}

///////////////////////////////////////////////
eGraphicsCompare D3D9_GetCMPFUNCGraphicsCompare(D3DCMPFUNC aCmp) {
  niConvTableBegin(aCmp)
      niConvTableItem(D3DCMP_NEVER, eGraphicsCompare_Never);
  niConvTableItem(D3DCMP_EQUAL, eGraphicsCompare_Equal);
  niConvTableItem(D3DCMP_NOTEQUAL, eGraphicsCompare_NotEqual);
  niConvTableItem(D3DCMP_LESS, eGraphicsCompare_Less);
  niConvTableItem(D3DCMP_LESSEQUAL, eGraphicsCompare_LessEqual);
  niConvTableItem(D3DCMP_GREATER, eGraphicsCompare_Greater);
  niConvTableItem(D3DCMP_GREATEREQUAL, eGraphicsCompare_GreaterEqual);
  niConvTableItem(D3DCMP_ALWAYS, eGraphicsCompare_Always);
  niConvTableEnd(eGraphicsCompare_Always);
}

///////////////////////////////////////////////
D3DSTENCILOP D3D9_GetStencilOpSTENCILOP(eStencilOp aOp) {
  niConvTableBegin(aOp)
      niConvTableItem(eStencilOp_Keep, D3DSTENCILOP_KEEP)
      niConvTableItem(eStencilOp_Zero, D3DSTENCILOP_ZERO)
      niConvTableItem(eStencilOp_Replace, D3DSTENCILOP_REPLACE)
      niConvTableItem(eStencilOp_IncrWrap, D3DSTENCILOP_INCR)
      niConvTableItem(eStencilOp_DecrWrap, D3DSTENCILOP_DECR)
      niConvTableItem(eStencilOp_IncrSat, D3DSTENCILOP_INCRSAT)
      niConvTableItem(eStencilOp_DecrSat, D3DSTENCILOP_DECRSAT)
      niConvTableItem(eStencilOp_Invert, D3DSTENCILOP_INVERT)
      niConvTableEnd(D3DSTENCILOP_ZERO)
      }

///////////////////////////////////////////////
eStencilOp D3D9_GetSTENCILOPStencilOp(D3DSTENCILOP aOp) {
  niConvTableBegin(aOp)
      niConvTableItem(D3DSTENCILOP_KEEP, eStencilOp_Keep)
      niConvTableItem(D3DSTENCILOP_ZERO, eStencilOp_Zero)
      niConvTableItem(D3DSTENCILOP_REPLACE, eStencilOp_Replace)
      niConvTableItem(D3DSTENCILOP_INCR, eStencilOp_IncrWrap)
      niConvTableItem(D3DSTENCILOP_DECR, eStencilOp_DecrWrap)
      niConvTableItem(D3DSTENCILOP_INCRSAT, eStencilOp_IncrSat)
      niConvTableItem(D3DSTENCILOP_DECRSAT, eStencilOp_DecrSat)
      niConvTableItem(D3DSTENCILOP_INVERT, eStencilOp_Invert)
      niConvTableEnd(eStencilOp_Zero)
      }

///////////////////////////////////////////////
tBool D3D9_ApplyDepthStencilStates(
    sD3D9StateCache& aCache, IDirect3DDevice9* apDev, const D3DCAPS9& aCaps,
    const sDepthStencilStatesDesc* pStates)
{
  STATE_PROFILE_BLOCK(D3D9_ApplyDepthStencilStates);

  D3D9_SetRenderState(apDev,D3DRS_ZENABLE, pStates->mbDepthTest);
  if (pStates->mbDepthTest) {
    D3D9_SetRenderState(apDev,D3DRS_ZWRITEENABLE, pStates->mbDepthTestWrite);
    D3D9_SetRenderState(apDev,D3DRS_ZFUNC, D3D9_GetGraphicsCompareCMPFUNC(pStates->mDepthTestCompare));
  }

  if (pStates->mStencilMode == eStencilMode_None) {
    D3D9_SetRenderState(apDev,D3DRS_STENCILENABLE, FALSE);
  }
  else {
    D3D9_SetRenderState(apDev,D3DRS_STENCILENABLE, TRUE);

    D3D9_SetRenderState(apDev,D3DRS_STENCILREF, pStates->mnStencilRef);
    D3D9_SetRenderState(apDev,D3DRS_STENCILMASK, pStates->mnStencilMask);

    if (pStates->mStencilMode == eStencilMode_TwoSided) {
      D3D9_SetRenderState(apDev,D3DRS_TWOSIDEDSTENCILMODE, TRUE);
      D3D9_SetRenderState(apDev,D3DRS_CCW_STENCILFUNC, D3D9_GetGraphicsCompareCMPFUNC(pStates->mStencilBackCompare));
      D3D9_SetRenderState(apDev,D3DRS_CCW_STENCILFAIL, D3D9_GetStencilOpSTENCILOP(pStates->mStencilBackFail));
      D3D9_SetRenderState(apDev,D3DRS_CCW_STENCILPASS, D3D9_GetStencilOpSTENCILOP(pStates->mStencilBackPassDepthPass));
      D3D9_SetRenderState(apDev,D3DRS_CCW_STENCILZFAIL, D3D9_GetStencilOpSTENCILOP(pStates->mStencilBackPassDepthFail));
    }
    else {
      D3D9_SetRenderState(apDev,D3DRS_TWOSIDEDSTENCILMODE, FALSE);
    }

    D3D9_SetRenderState(apDev,D3DRS_STENCILFUNC, D3D9_GetGraphicsCompareCMPFUNC(pStates->mStencilFrontCompare));
    D3D9_SetRenderState(apDev,D3DRS_STENCILFAIL, D3D9_GetStencilOpSTENCILOP(pStates->mStencilFrontFail));
    D3D9_SetRenderState(apDev,D3DRS_STENCILPASS, D3D9_GetStencilOpSTENCILOP(pStates->mStencilFrontPassDepthPass));
    D3D9_SetRenderState(apDev,D3DRS_STENCILZFAIL, D3D9_GetStencilOpSTENCILOP(pStates->mStencilFrontPassDepthFail));

  }

  return eTrue;
}

//--------------------------------------------------------------------------------------------
//
//  D3D9 Rasterizer states
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
tBool D3D9_ApplyViewport(IDirect3DDevice9* apDev, const sRecti& aVal, const tU32 rtWidth, const tU32 rtHeight) {
  D3DVIEWPORT9 newViewport;
  newViewport.X = aVal.GetTopLeft().x;
  newViewport.Y = aVal.GetTopLeft().y;
  newViewport.Width  = aVal.GetWidth();
  newViewport.Height = aVal.GetHeight();
  newViewport.MinZ = 0.0f;
  newViewport.MaxZ = 1.0f;
  if (newViewport.Width == 0)
    newViewport.Width = rtWidth;
  if (newViewport.Height == 0)
    newViewport.Height = rtHeight;
  if (newViewport.X+newViewport.Width > rtWidth)
    newViewport.Width -= (newViewport.X+newViewport.Width)-rtWidth;
  if (newViewport.Y+newViewport.Height > rtHeight)
    newViewport.Height -= (newViewport.Y+newViewport.Height)-rtHeight;
  HRESULT hr = apDev->SetViewport(&newViewport);
  if (FAILED(hr)) {
    D3D9ERROR();
    return eFalse;
  }
  return eTrue;
}

///////////////////////////////////////////////
tBool D3D9_ApplyScissor(IDirect3DDevice9* apDev,
                        const sRecti& aScissor,
                        const sRecti& aViewport)
{
  sRecti r = aScissor.ClipRect(aViewport);
  apDev->SetScissorRect((CONST RECT*)&r);
  return eTrue;
}

///////////////////////////////////////////////
tBool D3D9_ApplyRenderTarget(sD3D9StateCache& aCache,
                             IDirect3DDevice9* apDev,
                             const D3DCAPS9& aCaps,
                             Ptr<iTexture>* apRT,
                             iTexture* apDS)
{
  IDirect3DDevice9* mpD3D9Device = apDev;

  HRESULT hr; niUnused(hr);
  for (tU32 i = 0; i < MAX_TEXTUREUNIT; ++i) {
    mpD3D9Device->SetTexture(i,NULL);
  }

  tU32 mainRTFlags = 0;
  tI32 i;
  for (i = 0; i < ni::Min(MAX_RT,aCaps.NumSimultaneousRTs); ++i) {
    iD3D9Texture* rt = _BindCastTex(apRT[i]);
    if (i == 0 && rt) {
      mainRTFlags = rt->GetFlags();
    }
    DevCall(niFmt(_A("Can't set render target '%d'."),i),
            SetRenderTarget,
            (i,rt==NULL?NULL:rt->GetSurface()));
  }

  {
    iD3D9Texture* ds = _BindCastTex(apDS);
    if (ds) {
      if ((ds->GetFlags()&0xFFFF0000) !=
          (mainRTFlags&0xFFFF0000))
      {
        // #ifdef _DEBUG
        niAssertUnreachable("Incompatible AA render targets.");
        niError(niFmt(_A("Incompatible AA render targets 'RT:%s' & 'DS:%s'"),
                      apRT[0].IsOK()?niHStr(apRT[0]->GetDeviceResourceName()):_A("NA"),
                      niHStr(ds->GetDeviceResourceName())));
        // #endif
        ds = NULL;
      }
    }
    DevCall(_A("Can't set depth stencil surface."),
            SetDepthStencilSurface,
            (ds==NULL?NULL:ds->GetSurface()));
  }
  return eTrue;
}

///////////////////////////////////////////////
tBool D3D9_ApplyContext(sD3D9StateCache& aCache,
                        IDirect3DDevice9* apDev,
                        const D3DCAPS9& aCaps,
                        sD3D9Context* apCtx)
{
  tBool bShouldUpdate = eFalse;
  aCache.ShouldUpdate(&bShouldUpdate,eD3D9Cache_Context,(tIntPtr)apCtx);
  aCache.ShouldUpdate(&bShouldUpdate,eD3D9Cache_ContextSyncCounter,apCtx->mnSyncCounter);
  if (bShouldUpdate)
  {
    if (!D3D9_ApplyRenderTarget(
            aCache,
            apDev,
            aCaps,
            apCtx->mptrRT,
            apCtx->mptrDS))
      return eFalse;
    if (!D3D9_ApplyViewport(
            apDev,
            apCtx->mrectViewport,
            apCtx->mptrRT[0]->GetWidth(),
            apCtx->mptrRT[0]->GetHeight()))
      return eFalse;
    if (!D3D9_ApplyScissor(
            apDev,
            apCtx->mrectScissor,
            apCtx->mrectViewport))
      return eFalse;
  }
  return eTrue;
}

///////////////////////////////////////////////
D3DCULL D3D9_GetCullingModeCULL(eCullingMode aCull) {
  niConvTableBegin(aCull)
      niConvTableItem(eCullingMode_None,D3DCULL_NONE)
      niConvTableItem(eCullingMode_CW,D3DCULL_CW)
      niConvTableItem(eCullingMode_CCW,D3DCULL_CCW)
      niConvTableEnd(D3DCULL_NONE);
}

///////////////////////////////////////////////
eCullingMode D3D9_GetCULLCullingMode(D3DCULL aCull) {
  niConvTableBegin(aCull)
      niConvTableItem(D3DCULL_NONE, eCullingMode_None)
      niConvTableItem(D3DCULL_CW, eCullingMode_CW)
      niConvTableItem(D3DCULL_CCW, eCullingMode_CCW)
      niConvTableEnd(eCullingMode_None);
}

///////////////////////////////////////////////
tBool D3D9_GetFirstStageBlendMode(eBlendMode aMode, D3DBLEND& aSrc, D3DBLEND& aDest) {
  switch (aMode) {
    default:
    case eBlendMode_NoBlending:
      aSrc = D3DBLEND_ONE;
      aDest = D3DBLEND_ZERO;
      break;
    case eBlendMode_ReplaceAlpha:
      aSrc = D3DBLEND_ZERO;
      aDest = D3DBLEND_ONE;
      break;
    case eBlendMode_Additive:
      aSrc = D3DBLEND_ONE;
      aDest = D3DBLEND_ONE;
      break;
    case eBlendMode_Modulate:
    case eBlendMode_ModulateReplaceAlpha:
      aSrc = D3DBLEND_DESTCOLOR;
      aDest = D3DBLEND_ZERO;
      break;
    case eBlendMode_Modulate2x:
    case eBlendMode_Modulate2xReplaceAlpha:
      aSrc = D3DBLEND_DESTCOLOR;
      aDest = D3DBLEND_SRCCOLOR;
      break;
    case eBlendMode_Translucent:
      aSrc = D3DBLEND_SRCALPHA;
      aDest = D3DBLEND_INVSRCALPHA;
      break;
    case eBlendMode_TranslucentInvAlpha:
      aSrc = D3DBLEND_INVSRCALPHA;
      aDest = D3DBLEND_SRCALPHA;
      break;
    case eBlendMode_TintedGlass:
      aSrc = D3DBLEND_SRCCOLOR;
      aDest = D3DBLEND_INVSRCCOLOR;
      break;
    case eBlendMode_PreMulAlpha:
      aSrc = D3DBLEND_ONE;
      aDest = D3DBLEND_INVSRCALPHA;
      break;
  }
  return eTrue;
}

///////////////////////////////////////////////
tU32 D3D9_GetColorWriteMaskCOLORWRITE(eColorWriteMask aMask) {
  tU32 nRef = 0;
  if (niFlagIs(aMask,eColorWriteMask_Alpha)) nRef |= D3DCOLORWRITEENABLE_ALPHA;
  if (niFlagIs(aMask,eColorWriteMask_Red))   nRef |= D3DCOLORWRITEENABLE_RED;
  if (niFlagIs(aMask,eColorWriteMask_Green)) nRef |= D3DCOLORWRITEENABLE_GREEN;
  if (niFlagIs(aMask,eColorWriteMask_Blue))  nRef |= D3DCOLORWRITEENABLE_BLUE;
  return nRef;
}

///////////////////////////////////////////////
eColorWriteMask D3D9_GetCOLORWRITEColorWriteMask(tU32 aMask) {
  tU32 nRef = 0;
  if (niFlagIs(aMask,D3DCOLORWRITEENABLE_ALPHA)) nRef |= eColorWriteMask_Alpha;
  if (niFlagIs(aMask,D3DCOLORWRITEENABLE_RED))   nRef |= eColorWriteMask_Red;
  if (niFlagIs(aMask,D3DCOLORWRITEENABLE_GREEN)) nRef |= eColorWriteMask_Green;
  if (niFlagIs(aMask,D3DCOLORWRITEENABLE_BLUE))  nRef |= eColorWriteMask_Blue;
  return (eColorWriteMask)nRef;
}

///////////////////////////////////////////////
tBool D3D9_ApplyRSBlendMode(IDirect3DDevice9* apDev, eBlendMode aMode) {
  if (aMode == eBlendMode_NoBlending) {
    D3D9_SetRenderState(apDev,D3DRS_ALPHABLENDENABLE, FALSE);
  }
  else {
    D3D9_SetRenderState(apDev,D3DRS_ALPHABLENDENABLE, TRUE);
    D3DBLEND src, dst;
    D3D9_GetFirstStageBlendMode(aMode,src,dst);
    D3D9_SetRenderState(apDev,D3DRS_SRCBLEND, src);
    D3D9_SetRenderState(apDev,D3DRS_DESTBLEND, dst);
  }
  return eTrue;
}

///////////////////////////////////////////////
tBool D3D9_ApplyRasterizerStates(
    sD3D9StateCache& aCache, IDirect3DDevice9* apDev, const D3DCAPS9& aCaps,
    const sRasterizerStatesDesc* pStates,
    tBool abDoubleSided,
    tBool abDepthOnly)
{
  STATE_PROFILE_BLOCK(D3D9_ApplyRasterizerStates);

  D3D9_SetRenderState(apDev,D3DRS_SCISSORTESTENABLE, pStates->mbScissorTest?TRUE:FALSE);
  D3D9_SetRenderState(apDev,D3DRS_FILLMODE, pStates->mbWireframe?D3DFILL_WIREFRAME:D3DFILL_SOLID);

  // - OpenGL defs (ref for SDK) :
  // factor: Specifies a scale factor that is used to create
  // a variable depth offset for each polygon. The initial value
  // is zero.
  // units: Specifies a value that is multiplied by an
  // implementation-specific value to create a constant depth
  // offset. The initial value is 0.
  // - D3D def:
  //  Offset = m * D3DRS_SLOPESCALEDEPTHBIAS + D3DRS_DEPTHBIAS
  D3D9_SetRenderState(apDev,D3DRS_SLOPESCALEDEPTHBIAS, ftoul(pStates->mfDepthBiasUnitScale));
  D3D9_SetRenderState(apDev,D3DRS_DEPTHBIAS,
                      (tU32)(ni::ClampZeroOne(pStates->mfDepthBiasFactor)*16.0f));

  D3D9_SetRenderState(apDev,D3DRS_CULLMODE,
                      abDoubleSided ?
                      D3DCULL_NONE :
                      D3D9_GetCullingModeCULL(pStates->mCullingMode)
                      );
  tU32 nColorWriteMask = abDepthOnly ? 0 : D3D9_GetColorWriteMaskCOLORWRITE(pStates->mColorWriteMask);
  D3D9_SetRenderState(apDev,D3DRS_COLORWRITEENABLE,nColorWriteMask);
  if (aCaps.PrimitiveMiscCaps & D3DPMISCCAPS_INDEPENDENTWRITEMASKS) {
    D3D9_SetRenderState(apDev,D3DRS_COLORWRITEENABLE1,nColorWriteMask);
    D3D9_SetRenderState(apDev,D3DRS_COLORWRITEENABLE2,nColorWriteMask);
    D3D9_SetRenderState(apDev,D3DRS_COLORWRITEENABLE3,nColorWriteMask);
  }

  return eTrue;
}

//--------------------------------------------------------------------------------------------
//
//  D3D9 Fixed pipeline states.
//
//--------------------------------------------------------------------------------------------
tBool D3D9_ApplyFixedStates(sD3D9StateCache& aCache,
                            IDirect3DDevice9* apDev, const D3DCAPS9& aCaps,
                            tBool abVSShader,
                            tBool abPSShader,
                            const sMaterialDesc* apMaterial,
                            const sFixedStatesDesc* apFixedStates,
                            sMatrixf& viewMatrix,
                            sMatrixf& projMatrix,
                            const sMatrixf& aWorldMatrix)
{
  STATE_PROFILE_BLOCK(D3D9_ApplyFixedStates);

  {
    sMatrixf worldMatrix, texMatrix;

    viewMatrix = apFixedStates->mViewMatrix;
    projMatrix = apFixedStates->mProjectionMatrix;

    if (!abVSShader) {

      worldMatrix = aWorldMatrix;

      if (!abVSShader) {
        {
          STATE_PROFILE_BLOCK(D3D9_ApplyFS_WVP_Mtx);
          apDev->SetTransform(D3DTS_WORLD,(D3DMATRIX*)worldMatrix.ptr());
          apDev->SetTransform(D3DTS_VIEW,(D3DMATRIX*)viewMatrix.ptr());
          apDev->SetTransform(D3DTS_PROJECTION,(D3DMATRIX*)projMatrix.ptr());
        }

        {
          STATE_PROFILE_BLOCK(D3D9_ApplyFS_TexMatrix);

#define TEXMATRIX(I)                                                    \
          {                                                             \
            iExpression* expr = apMaterial->mptrExpressions[eMaterialExpression_Tex##I]; \
            if (expr) {                                                 \
              Ptr<iExpressionVariable> pVar = expr->Eval();             \
              if (pVar.IsOK()) {                                        \
                texMatrix = pVar->GetMatrix();                          \
                texMatrix._31 = texMatrix._41;                          \
                texMatrix._32 = texMatrix._42;                          \
                apDev->SetTransform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0+I),(D3DMATRIX*)texMatrix.ptr()); \
                apDev->SetTextureStageState(I,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2); \
              }                                                         \
              else {                                                    \
                apDev->SetTextureStageState(I,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE); \
              }                                                         \
            }                                                           \
            else {                                                      \
              apDev->SetTextureStageState(I,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE); \
            }                                                           \
          }

          TEXMATRIX(1);
          TEXMATRIX(0);

#undef TEXMATRIX
        }
      }
    }
  }

  {
    D3D9_SetRenderState(apDev,D3DRS_CLIPPLANEENABLE,FALSE);
    D3D9_SetRenderState(apDev,D3DRS_FOGENABLE, FALSE);
    D3D9_SetRenderState(apDev,D3DRS_LIGHTING, FALSE);
    // D3D9_SetRenderState(apDev,D3DRS_NORMALIZENORMALS, FALSE);
    // D3D9_SetRenderState(apDev,D3DRS_SPECULARENABLE, FALSE);
    // D3D9_SetRenderState(apDev,D3DRS_LOCALVIEWER, FALSE);
  }

  return eTrue;
}

//--------------------------------------------------------------------------------------------
//
//  D3D9 Material
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
tBool D3D9_ApplySamplerBlendMode(IDirect3DDevice9* apDev, tU32 anTSS, tBool abTextureRGB, tBool abTextureAlpha, eBlendMode aBlendMode, tBool abVertexLit) {
  // D3DTA_TFACTOR: RGB(Channel color), A(Material Color alpha)
  // D3DTA_TEXTURE: Texture color
  // D3DTA_DIFFUSE: Vertex color
  tU32 nSourceRGB = abTextureRGB?D3DTA_TEXTURE:D3DTA_TFACTOR;
  tU32 nSourceAlpha = abTextureAlpha?D3DTA_TEXTURE:D3DTA_TFACTOR;
  tU32 nCurrentRGB = abTextureRGB?D3DTA_CURRENT:D3DTA_DIFFUSE;
  // tU32 nCurrentAlpha = abTextureAlpha?D3DTA_CURRENT:D3DTA_DIFFUSE;
  switch (aBlendMode) {
    case eBlendMode_NoBlending:
      {
        if (anTSS == 0) {
          if (abVertexLit) {
            D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLOROP, D3DTOP_MODULATE);
            D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG1, nSourceRGB);
            D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
            D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
            D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG1, nSourceAlpha);
            D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
          }
          else {
            if (nSourceRGB == D3DTA_TFACTOR) {
              D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
              D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG1, D3DTA_TFACTOR);
            }
            else {
              D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLOROP, D3DTOP_MODULATE);
              D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG1, nSourceRGB);
              D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG2, D3DTA_TFACTOR);
            }
            if (nSourceAlpha == D3DTA_TFACTOR) {
              D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
              D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
            }
            else {
              D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
              D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG1, nSourceAlpha);
              D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
            }
          }
        }
        else {
          D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
          D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG1, nSourceRGB);
          D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
          D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG1, nSourceAlpha);
        }
        break;
      }
    case eBlendMode_ReplaceAlpha:
      {
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG1, nCurrentRGB);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG1, nSourceAlpha|D3DTA_ALPHAREPLICATE);
        break;
      };
    case eBlendMode_Additive:
      {
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLOROP, D3DTOP_ADD);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG1, nSourceRGB);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG2, D3DTA_CURRENT);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAOP, D3DTOP_ADD);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG1, nSourceAlpha);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
        break;
      }
    case eBlendMode_Modulate:
      {
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLOROP, D3DTOP_MODULATE);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG1, nSourceRGB);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG2, D3DTA_CURRENT);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG1, nSourceAlpha);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
        break;
      }
    case eBlendMode_ModulateReplaceAlpha:
      {
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLOROP, D3DTOP_MODULATE);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG1, nSourceRGB);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG2, D3DTA_CURRENT);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG1, nSourceAlpha|D3DTA_ALPHAREPLICATE);
        break;
      }
    case eBlendMode_Modulate2x:
      {
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG1, nSourceRGB);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG2, D3DTA_CURRENT);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAOP, D3DTOP_MODULATE2X);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG1, nSourceAlpha);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
        break;
      }
    case eBlendMode_Modulate2xReplaceAlpha:
      {
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG1, nSourceRGB);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG2, D3DTA_CURRENT);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG1, nSourceAlpha|D3DTA_ALPHAREPLICATE);
        break;
      }
    case eBlendMode_Translucent:
      {
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLOROP, D3DTOP_LERP);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG1, nSourceRGB);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG2, D3DTA_CURRENT);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG0, nSourceRGB|D3DTA_ALPHAREPLICATE);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAOP, D3DTOP_LERP);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG1, nSourceAlpha);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG0, nSourceAlpha);
        break;
      }
    case eBlendMode_TranslucentInvAlpha:
      {
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLOROP, D3DTOP_LERP);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG1, nSourceRGB);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG2, D3DTA_CURRENT);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG0, nSourceRGB|D3DTA_ALPHAREPLICATE|D3DTA_COMPLEMENT);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAOP, D3DTOP_LERP);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG1, nSourceAlpha);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG0, nSourceAlpha|D3DTA_COMPLEMENT);
        break;
      }
    case eBlendMode_TintedGlass:
      {
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLOROP, D3DTOP_MULTIPLYADD);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG1, D3DTA_CURRENT);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG2, nSourceRGB|D3DTA_COMPLEMENT);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG0, nSourceRGB);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAOP, D3DTOP_MULTIPLYADD);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG2, nSourceAlpha|D3DTA_COMPLEMENT);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG0, nSourceAlpha);
        break;
      }
    case eBlendMode_PreMulAlpha:
      {
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLOROP, D3DTOP_MULTIPLYADD);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG1, nSourceRGB|D3DTA_ALPHAREPLICATE);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG2, D3DTA_CURRENT);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_COLORARG0, nSourceRGB);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAOP, D3DTOP_MULTIPLYADD);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG1, nSourceAlpha|D3DTA_ALPHAREPLICATE);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
        D3D9_SetTextureStageState(apDev, anTSS, D3DTSS_ALPHAARG0, nSourceAlpha);
        break;
      }
    default:
      niAssertUnreachable("Unreachable");
      return eFalse;
  }

  return eTrue;
}

///////////////////////////////////////////////
tBool D3D9_BindTexture(iGraphics* apGraphics, iTexture*& apTex, const tU32 anExtraCaps) {
  if (!apTex) return eFalse;
  apTex = (iTexture*)apTex->Bind(apGraphics);
  if (!niIsOK(apTex)) return eFalse;

  tTextureFlags nFlags = apTex->GetFlags();
  return
      niFlagIsNot(nFlags,eTextureFlags_SystemMemory) &&
      niFlagIsNot(nFlags,eTextureFlags_Virtual) &&
      (niFlagIs(anExtraCaps,CAPS_DEPTHTEXTURE) ||
       niFlagIsNot(nFlags,eTextureFlags_DepthStencil)) &&
      niFlagIsNot(nFlags,eTextureFlags_Surface);
}

///////////////////////////////////////////////
tBool D3D9_ApplyMaterialChannel(
    iGraphics* apGraphics,
    sD3D9StateCache& aCache, IDirect3DDevice9* apDev, const D3DCAPS9& aCaps, const tU32 anExtraCaps,
    tU32 anTSS, eMaterialChannel aChannel, const tU32 anTexCooIndex,
    const sMaterialDesc* apMaterial)
{
  const sMaterialChannel& ch = apMaterial->mChannels[aChannel];

  HRESULT hr; niUnused(hr);
  D3D9_SetTextureStageState(apDev,anTSS,D3DTSS_TEXCOORDINDEX,anTexCooIndex);
  iTexture* pTex = ch.mTexture;
  tBool bTexBindable = D3D9_BindTexture(apGraphics,pTex,anExtraCaps);

  D3D9_ApplySamplerStates(apGraphics,aCache,apDev,aCaps,anTSS,pTex,ch.mhSS);

  const tMaterialFlags matFlags = apMaterial->mFlags;
  const tBool vertexLighting =
      (!niFlagIs(matFlags,eMaterialFlags_DiffuseModulate)) &&
      niFlagIs(matFlags,eMaterialFlags_Vertex);

  {
    tU32 bipp = bTexBindable?pTex->GetPixelFormat()->GetBitsPerPixel():0;
    tU32 abits = bTexBindable?pTex->GetPixelFormat()->GetNumABits():0;
    D3D9_ApplySamplerBlendMode(
        apDev,
        anTSS,
        bTexBindable?!!(bipp-abits):eFalse,
        bTexBindable?!!abits:eFalse,
        _kFixedPipelineChannelBlendModes[aChannel],
        vertexLighting);
  }
  if (bTexBindable) {
#pragma niTodo("Use a safe way of casting with error reporting.")
    iD3D9Texture* pD3D9Tex = niStaticCast(iD3D9Texture*,pTex);
    //apDev->SetTexture(anTSS,pD3D9Tex->GetTexture());
    DevCallA(_A("Can't set texture."),SetTexture, (anTSS,pD3D9Tex->GetTexture()));
  }
  else {
    apDev->SetTexture(anTSS,NULL);
  }

  return eTrue;
}

///////////////////////////////////////////////
tBool D3D9_ApplyMaterial(iGraphics* apGraphics,
                         sD3D9StateCache& aCache, IDirect3DDevice9* apDev,
                         const D3DCAPS9& aCaps,
                         const tU32 anExtraCaps,
                         const sMaterialDesc* apMaterial,
                         iShader* apVS,
                         iShader* apPS,
                         sVec4f& texFactor,
                         const tU32 anMaxTU,
                         const tU32 anFVF)
{
  const sMaterialDesc* m = apMaterial;
  {
    tBool bShouldUpdate = eFalse;
    aCache.ShouldUpdate(&bShouldUpdate,eD3D9Cache_Material,(tIntPtr)m);
    if (!bShouldUpdate)
      return ni::eTrue;
  }

  tBool bEnableAlphaTest = eFalse;
  const tMaterialFlags matFlags = m ? m->mFlags : 0;

  if (!m) {
    STATE_PROFILE_BLOCK(D3D9_ApplyMaterial_NoMaterial);
    for (tU32 i = 0; i < MAX_TEXTUREUNIT; ++i) {
      apDev->SetTexture(i,NULL);
    }
    D3D9_SetTextureStageState(apDev, 1, D3DTSS_COLOROP, D3DTOP_DISABLE);
  }
  else if (apPS) {
    //STATE_PROFILE_BLOCK(D3D9_ApplyMaterial_PS);

    // If a pixel shader is set, just bind all texture channels and set the samplers
    {
      STATE_PROFILE_BLOCK(D3D9_ApplyMaterial_PS_Texs);
      niLoop(i,eMaterialChannel_Last) {
        const sMaterialChannel& ch = m->mChannels[i];
        iTexture* pTex = ch.mTexture;
        tBool bTexBindable = D3D9_BindTexture(apGraphics,pTex,anExtraCaps);
        if (bTexBindable) {
          const tI8 nSamplerIndex = (tI8)i;
          iD3D9Texture* pD3D9Tex = niStaticCast(iD3D9Texture*,pTex);
          apDev->SetTexture(nSamplerIndex,pD3D9Tex->GetTexture());
          D3D9_ApplySamplerStates(
              apGraphics,aCache,apDev,aCaps,nSamplerIndex,pTex,ch.mhSS);
        }
        else {
          apDev->SetTexture(i,NULL);  // Necessary ??
        }
      }
    }
  }
  else {
    STATE_PROFILE_BLOCK(D3D9_ApplyMaterial_Fixed);

#pragma niNote("Most of the fixed pipeline rendering setup")
    tU32 nLastStage = 0;

    // Texture factor, base color
    if (!m->mChannels[eMaterialChannel_Base].mTexture.IsOK() ||
        niFlagIs(matFlags,eMaterialFlags_DiffuseModulate))
    {
      texFactor = m->mChannels[eMaterialChannel_Base].mColor;
      D3D9_SetRenderState(apDev,D3DRS_TEXTUREFACTOR, ULColorBuild(texFactor));
    }
    else {
      texFactor = sColor4f::One();
      // Always preserve the alpha channel
      texFactor.w = m->mChannels[eMaterialChannel_Base].mColor.w;
      D3D9_SetRenderState(apDev,D3DRS_TEXTUREFACTOR, ULColorBuild(texFactor));
    }

    // Base texture
    D3D9_ApplyMaterialChannel(apGraphics,aCache,apDev,aCaps,anExtraCaps,nLastStage,eMaterialChannel_Base,0,m);
    ++nLastStage;

    // Translucent/transparent textures
    if (nLastStage < anMaxTU) {
      if (niFlagIs(matFlags,eMaterialFlags_Translucent) ||
          niFlagIs(matFlags,eMaterialFlags_Transparent))
      {
        if (niFlagIs(matFlags,eMaterialFlags_Transparent)) {
          D3D9_SetRenderState(
              apDev,D3DRS_ALPHAREF,
              ni::Clamp<tI32>(tI32(m->mChannels[eMaterialChannel_Opacity].mColor.w*255.0f),0,255));
          D3D9_SetRenderState(apDev,D3DRS_ALPHAFUNC, D3DCMP_GREATER);
          bEnableAlphaTest = eTrue;
        }

        if (m->mChannels[eMaterialChannel_Opacity].mTexture.IsOK()) {
          D3D9_ApplyMaterialChannel(
              apGraphics,aCache,apDev,aCaps,anExtraCaps,nLastStage,eMaterialChannel_Opacity,0,m);
          ++nLastStage;
        }
      }
    }

    D3D9_SetTextureStageState(apDev, nLastStage, D3DTSS_COLOROP, D3DTOP_DISABLE);
    for (tU32 i = nLastStage; i < MAX_TEXTUREUNIT; ++i) {
      apDev->SetTexture(i,NULL);
    }
  }

  D3D9_SetRenderState(apDev,D3DRS_ALPHATESTENABLE, bEnableAlphaTest);
  const eBlendMode blendMode = m ? m->mBlendMode : eBlendMode_NoBlending;
  if (blendMode == eBlendMode_NoBlending) {
    D3D9_ApplyRSBlendMode(
        apDev,
        (niFlagIs(matFlags,eMaterialFlags_Translucent)) ?
        eBlendMode_Translucent : blendMode);
  }
  else {
    D3D9_ApplyRSBlendMode(apDev,blendMode);
  }

  if (anExtraCaps & CAPS_TRANSPARENT_AA) {
    if (niFlagIs(matFlags,eMaterialFlags_TransparentAA)) {
      D3D9_SetRenderState(
          apDev,D3DRS_ADAPTIVETESS_Y,
          (D3DFORMAT)MAKEFOURCC('S', 'S', 'A', 'A'));
    }
    else {
      D3D9_SetRenderState(
          apDev,D3DRS_ADAPTIVETESS_Y,
          D3DFMT_UNKNOWN);
    }
  }

  return eTrue;
}

///////////////////////////////////////////////
inline tBool __stdcall D3D9_SetConstFloat_(IDirect3DDevice9* apDev, eShaderUnit aUnit, tU32 ulConst, const sVec4f* pV, tU32 ulNum)
{
  niAssert(aUnit == eShaderUnit_Pixel  || aUnit == eShaderUnit_Vertex);
  niAssert(ulConst >= 0 && ulConst < 256);
  if (aUnit == eShaderUnit_Pixel)
    apDev->SetPixelShaderConstantF(ulConst, pV->ptr(), ulNum);
  else
    apDev->SetVertexShaderConstantF(ulConst, pV->ptr(), ulNum);
  return eTrue;
}

///////////////////////////////////////////////
inline tBool __stdcall D3D9_SetConstFloatMatrix_(IDirect3DDevice9* apDev, eShaderUnit aUnit, tU32 ulConst, const sMatrixf* pMtx, tU32 ulNum)
{
  //sMatrixf mtx;
  for (tU32 i = 0; i < ulNum; ++i)
  {
    //MatrixTranspose(mtx, *pMtx);
    //SetConstFloat(ulConst+(i*4), (sVec4f*)&mtx, 4);
    D3D9_SetConstFloat_(apDev,aUnit,ulConst+(i*4), (sVec4f*)pMtx, 4);
    ++pMtx;
  }
  return eTrue;
}

///////////////////////////////////////////////
inline tBool __stdcall D3D9_SetConstInt_(IDirect3DDevice9* apDev, eShaderUnit aUnit, tU32 ulConst, const sVec4i* pV, tU32 ulNum)
{
  niAssert(aUnit == eShaderUnit_Pixel  || aUnit == eShaderUnit_Vertex);
  niAssert(ulConst >= 0 && ulConst < 256);
  if (aUnit == eShaderUnit_Pixel)
    apDev->SetPixelShaderConstantI(ulConst, (const int*)pV->ptr(), ulNum);
  else
    apDev->SetVertexShaderConstantI(ulConst, (const int*)pV->ptr(), ulNum);

  return eTrue;
}

///////////////////////////////////////////////
inline tBool __stdcall D3D9_SetConstBool_(IDirect3DDevice9* apDev, eShaderUnit aUnit, tU32 ulConst, const tBool* pV, tU32 ulNum)
{
  niAssert(aUnit == eShaderUnit_Pixel  || aUnit == eShaderUnit_Vertex);
  niAssert(ulConst >= 0 && ulConst < 256);
  if (aUnit == eShaderUnit_Pixel) {
    BOOL tmp;
    for (tU32 i = 0; i < ulNum; ++i) {
      tmp = *pV++;
      apDev->SetPixelShaderConstantB(ulConst+i, &tmp, 1);
    }
  }
  else {
    BOOL tmp;
    for (tU32 i = 0; i < ulNum; ++i) {
      tmp = *pV++;
      apDev->SetVertexShaderConstantB(ulConst+i, &tmp, 1);
    }
  }

  return eTrue;
}

///////////////////////////////////////////////
inline tBool __stdcall D3D9_SetConstFloatArray(IDirect3DDevice9* apDev, eShaderUnit aUnit, tU32 ulConst, const tVec4fCVec* apV, tU32 ulNum)
{
  return D3D9_SetConstFloat_(apDev,aUnit,ulConst,&(*apV)[0],ulNum);
}

///////////////////////////////////////////////
inline tBool __stdcall D3D9_SetConstFloat(IDirect3DDevice9* apDev, eShaderUnit aUnit, tU32 ulConst, const sVec4f& aV)
{
  return D3D9_SetConstFloat_(apDev,aUnit,ulConst,&aV,1);
}

///////////////////////////////////////////////
inline tBool __stdcall D3D9_SetConstFloatMatrixArray(IDirect3DDevice9* apDev, eShaderUnit aUnit, tU32 ulConst, const tMatrixfCVec* apV, tU32 ulNum)
{
  return D3D9_SetConstFloatMatrix_(apDev,aUnit,ulConst,&(*apV)[0],ulNum);
}

///////////////////////////////////////////////
inline tBool __stdcall D3D9_SetConstFloatMatrix(IDirect3DDevice9* apDev, eShaderUnit aUnit, tU32 ulConst, const sMatrixf& aV)
{
  return D3D9_SetConstFloatMatrix_(apDev,aUnit,ulConst,&aV,1);
}

///////////////////////////////////////////////
inline tBool __stdcall D3D9_SetConstIntArray(IDirect3DDevice9* apDev, eShaderUnit aUnit, tU32 ulConst, const tVec4iCVec *apV, tU32 ulNum)
{
  return D3D9_SetConstInt_(apDev,aUnit,ulConst,&(*apV)[0],ulNum);
}

///////////////////////////////////////////////
inline tBool __stdcall D3D9_SetConstInt(IDirect3DDevice9* apDev, eShaderUnit aUnit, tU32 ulConst, const sVec4i& aV)
{
  return D3D9_SetConstInt_(apDev,aUnit,ulConst,&aV,1);
}

///////////////////////////////////////////////
inline tBool __stdcall D3D9_SetConstBoolArray(IDirect3DDevice9* apDev, eShaderUnit aUnit, tU32 ulConst, const tI8CVec* apV, tU32 ulNum)
{
  return D3D9_SetConstBool_(apDev,aUnit,ulConst,&(*apV)[0],ulNum);
}

///////////////////////////////////////////////
inline tBool __stdcall D3D9_SetConstBool(IDirect3DDevice9* apDev, eShaderUnit aUnit, tU32 ulConst, tBool aV)
{
  return D3D9_SetConstBool_(apDev,aUnit,ulConst,&aV,1);
}

static __forceinline tBool D3D9_UploadConstant(
    IDirect3DDevice9* apDev,
    eShaderUnit aUnit,
    const sShaderConstantsDesc* constBuffer,
    const tU16 cbType,
    const tU32 anConstIndex,
    const tU32 anConstSize,
    const tU32 anConstHwIndex)
{
  switch (cbType) {
    case eShaderRegisterType_ConstFloat: {
      const sVec4f* rv = &constBuffer->mvFloatRegisters[anConstIndex];
      D3D9_SetConstFloat_(apDev,aUnit,anConstHwIndex,rv,anConstSize);
      break;
    }
    case eShaderRegisterType_ConstInt: {
      const sVec4i* rv = &constBuffer->mvIntRegisters[anConstIndex];
      D3D9_SetConstInt_(apDev,aUnit,anConstHwIndex,rv,anConstSize);
      break;
    }
    case eShaderRegisterType_ConstBool: {
      const sVec4i* rv = &constBuffer->mvIntRegisters[anConstIndex];
      niLoop(j,anConstSize) {
        D3D9_SetConstBool(apDev,aUnit,
                          anConstHwIndex+j,
                          rv[j].x ? eTrue : eFalse);
      }
      break;
    }
    default:
      niAssertUnreachable("Unreachable.");
      return eFalse;
  }
  return eTrue;
}
static __forceinline tBool D3D9_UploadUniform(
    IDirect3DDevice9* apDev,
    eShaderUnit aUnit,
    iHString* ahspName,
    iShaderConstants* apConsts,
    const tU32 anConstSize,
    const tU32 anConstHwIndex)
{
  if (!apConsts) return eFalse;
  const sShaderConstantsDesc* ct = (const sShaderConstantsDesc*)apConsts->GetDescStructPtr();
  sShaderConstantsDesc::tConstMap::const_iterator it = ct->mmapConstants.find(ahspName);
  if (it != ct->mmapConstants.end() && it->second != eInvalidHandle) {
    const sShaderConstantsDesc::sConstant& c = ct->mvConstants[it->second];
    return D3D9_UploadConstant(apDev,aUnit,ct,
                               c.Type,
                               c.nDataIndex,
                               ni::Min(c.nSize,anConstSize),
                               anConstHwIndex);
  }
  return eFalse;
}

///////////////////////////////////////////////
tBool D3D9_ApplyShaderConstants(
    sD3D9Context* apContext,
    iShaderConstants* apShaderConsts,
    IDirect3DDevice9* apDev, iGraphics* apGraphics,
    eShaderUnit aUnit, iShader* apProg, iDrawOperation* apDrawOp,
    const sMaterialDesc* apMaterial)
{
  if (!apShaderConsts)
    return ni::eFalse;

  sShaderConstantsDesc* const constDesc = (sShaderConstantsDesc*)apShaderConsts->GetDescStructPtr();
  niLoop(i,constDesc->mvConstants.size()) {
    const sShaderConstantsDesc::sConstant& c = constDesc->mvConstants[i];
    iHString* hspName = c.hspName;
    if (hspName) {
      // Look in the material's constants
      if (D3D9_UploadUniform(apDev,aUnit,hspName,apMaterial->mptrConstants,c.nSize,c.nHwIndex))
        continue;
    }

    // Use default value...
    D3D9_UploadConstant(apDev,aUnit,constDesc,
                        c.Type,
                        c.nDataIndex,
                        c.nSize,
                        c.nHwIndex);
  }

  return ni::eTrue;
}

///////////////////////////////////////////////
tBool D3D9_ApplyVertexShader(sD3D9Context* apContext, sD3D9StateCache& aCache, IDirect3DDevice9* apDev, iGraphics* apGraphics, iShader* apProg, iDrawOperation* apDrawOp, const sMaterialDesc* apMaterial) {
  tBool bShouldUpdate = eFalse;
  aCache.ShouldUpdate(&bShouldUpdate,eD3D9Cache_VertexShader,(tIntPtr)apProg);
  if (bShouldUpdate) {
    if (!apProg) {
      apDev->SetVertexShader(NULL);
      return eTrue;
    }
    niProfileBlock(D3D9_SetVertexShader);
    iD3D9ShaderVertex* pVert = niStaticCast(iD3D9ShaderVertex*,apProg);
    apDev->SetVertexShader(pVert->GetVertexShader());
  }
  else if (!apProg) {
    return ni::eTrue;
  }

  D3D9_ApplyShaderConstants(
      apContext,
      (iShaderConstants*)apProg->GetConstants(),
      apDev,
      apGraphics,
      eShaderUnit_Vertex,apProg,
      apDrawOp,apMaterial);
  return eTrue;
}

///////////////////////////////////////////////
tBool D3D9_ApplyPixelShader(sD3D9Context* apContext, sD3D9StateCache& aCache, IDirect3DDevice9* apDev, iGraphics* apGraphics, iShader* apProg, iDrawOperation* apDrawOp, const sMaterialDesc* apMaterial) {
  tBool bShouldUpdate = eFalse;
  aCache.ShouldUpdate(&bShouldUpdate,eD3D9Cache_PixelShader,(tIntPtr)apProg);
  if (bShouldUpdate) {
    if (!apProg) {
      apDev->SetPixelShader(NULL);
      return eTrue;
    }
    niProfileBlock(D3D9_SetPixelShader);
    iD3D9ShaderPixel* pFrag = niStaticCast(iD3D9ShaderPixel*,apProg);
    apDev->SetPixelShader(pFrag->GetPixelShader());
  }
  else if (!apProg) {
    return ni::eTrue;
  }

  D3D9_ApplyShaderConstants(
      apContext,
      (iShaderConstants*)apProg->GetConstants(),
      apDev,apGraphics,eShaderUnit_Pixel,apProg,apDrawOp,apMaterial);
  return eTrue;
}

//--------------------------------------------------------------------------------------------
//
//  D3D9 Graphics Driver
//
//--------------------------------------------------------------------------------------------
class cD3D9 : public ImplRC<iGraphicsDriver,eImplFlags_Default,iD3D9Graphics>
{
  niBeginClass(cD3D9);

 public:
  sD3D9StateCache             mCache;
  iGraphics*                  mpGraphics;
  IDirect3D9*                 mpD3D9;
  IDirect3DDevice9*           mpD3D9Device;
  Ptr<sD3D9Context>           mptrPrimaryContext;
  cModeSelector               mModeSelector;
  IDirect3DStateBlock9*       mpD3D9DefaultDeviceStates;
  IDirect3DStateBlock9*       mpD3D9DefaultPixelStates;
  IDirect3DStateBlock9*       mpD3D9DefaultVertexStates;
  Nonnull<tD3D9LostDeviceSinkLst> mlstLostDeviceSinks;
  tBool                       mbBeganRendering;
  astl::vector<sD3D9Context*> mvContexts;
  Ptr<iGraphicsDrawOpCapture> mptrDOCapture;
  struct {
    tBool mbIA16;
    tBool mbNoCubeMap;
    tBool mbNoVolume;
    tU32  mnMaxVS;
    tU32  mnMaxPS;
    tU32  mnMaxTU;
  } mCompTest;
  sFixedShaders mFixedShaders;

  tBool _InitCaps() {
    if (mnExtraCaps&CAPS_INITIALIZED)
      return eTrue;

    mnExtraCaps = CAPS_INITIALIZED;

    // Create the D3D9 object
    {
      mpD3D9 = mpDirect3DCreate9(D3D_SDK_VERSION);
      if (!mpD3D9) {
        niError(_A("Can't create D3D9 object."));
        return eFalse;
      }
    }

    // Get the capabilities of the main adapter
    mModeSelector.Initialize(mpD3D9,eFalse);
    mModeSelector.LogModes();

    DWORD dwBehaviorFlags = _GetD3D9BehaviorFlags(this->GetD3D9Caps());

    IDirect3DDevice9* pD3DDevice9 = NULL;
    D3DPRESENT_PARAMETERS D3DPP;
    ZeroMemory(&D3DPP, sizeof(D3DPP));
    D3DPP.Windowed = TRUE;
    D3DPP.SwapEffect = D3D9_SWAP_EFFECT;
    D3DPP.BackBufferWidth = 0;
    D3DPP.BackBufferHeight = 0;
    D3DPP.BackBufferCount = 0;
    D3DPP.BackBufferFormat = D3DFMT_UNKNOWN;
    D3DPP.EnableAutoDepthStencil = FALSE;
    D3DPP.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
    D3DPP.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
    D3DPP.Flags                = D3D9_PRESENT_FLAGS;

    {
      HRESULT hr = S_FALSE;
      {
        Ptr<iOSWindow> ptrWindow = ni::GetLang()->CreateWindow(
            NULL,_A("null"),sRecti(0,0,100,100),
            eFalse,eOSWindowStyleFlags_Toolbox);
        HWND hTestWindow = (HWND)ptrWindow->GetHandle();
        hr = mpD3D9->CreateDevice(GetD3D9Caps().AdapterOrdinal,
                                  GetD3D9Caps().DeviceType,
                                  hTestWindow,
                                  dwBehaviorFlags,
                                  &D3DPP, &pD3DDevice9);
      }
      if (!SUCCEEDED(hr)) {
        niError(_A("Can't create any device."));
        return eFalse;
      }

      {
#define CF(USAGE,FORMAT) mpD3D9->CheckDeviceFormat(GetD3D9Caps().AdapterOrdinal,GetD3D9Caps().DeviceType,D3DPP.BackBufferFormat,USAGE,D3DRTYPE_TEXTURE,FORMAT)

        // check occlusion queries support
        hr = pD3DDevice9->CreateQuery(D3DQUERYTYPE_OCCLUSION,NULL);
        niFlagOnIf(mnExtraCaps,CAPS_OCCLUSIONQUERIES,SUCCEEDED(hr));

        // check depth stencil support
        {
          UINT ulW = 256;
          D3DFORMAT D3DFormat = D3DFMT_D24S8;
          hr = D3DXCheckTextureRequirements(pD3DDevice9, &ulW, &ulW, NULL, D3DUSAGE_DEPTHSTENCIL, &D3DFormat, D3DPOOL_DEFAULT);
          niFlagOnIf(mnExtraCaps,CAPS_DEPTHTEXTURE,SUCCEEDED(hr));
        }

        // check hardware instancing support
        if (D3DSHADER_VERSION_MAJOR(GetD3D9Caps().VertexShaderVersion) < 3) {
          hr = mpD3D9->CheckDeviceFormat(D3DADAPTER_DEFAULT,
                                         GetD3D9Caps().DeviceType,
                                         D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE,
                                         (D3DFORMAT)MAKEFOURCC('I','N','S','T'));
        }
        else {
          // VS 3.0
          hr = D3D_OK;
        }
        niFlagOnIf(mnExtraCaps,CAPS_HWINST,SUCCEEDED(hr));

        // check transparency SSAA
        {
          hr = mpD3D9->CheckDeviceFormat(
              D3DADAPTER_DEFAULT,
              D3DDEVTYPE_HAL,
              D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE,
              (D3DFORMAT)MAKEFOURCC('S', 'S', 'A', 'A')
              // for MSAA Transparency AA, we use the supersampling because it
              // can easily augment existing scenes whereas the MSAA version
              // needs the alpha channel to be tweaked accordingly
              // (D3DFORMAT)MAKEFOURCC('A', 'T', 'O', 'C')
                                         );
          niFlagOnIf(mnExtraCaps,CAPS_TRANSPARENT_AA,SUCCEEDED(hr));
        }

        // check I16 alpha ops
        hr = CF(D3DUSAGE_RENDERTARGET|D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,D3DFMT_A16B16G16R16);
        niFlagOnIf(mnExtraCaps,CAPS_I16PPOPS,SUCCEEDED(hr));

        // check F16 post pixel ops
        hr = CF(D3DUSAGE_RENDERTARGET|D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,D3DFMT_A16B16G16R16F);
        niFlagOnIf(mnExtraCaps,CAPS_F16PPOPS,SUCCEEDED(hr));

        // check F32 post pixel ops
        hr = CF(D3DUSAGE_RENDERTARGET|D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,D3DFMT_A32B32G32R32F);
        niFlagOnIf(mnExtraCaps,CAPS_F32PPOPS,SUCCEEDED(hr));

        // check I16 filtering
        hr = CF(D3DUSAGE_RENDERTARGET|D3DUSAGE_QUERY_FILTER,D3DFMT_A16B16G16R16);
        niFlagOnIf(mnExtraCaps,CAPS_I16FILTER,SUCCEEDED(hr));

        // check F16 filtering
        hr = CF(D3DUSAGE_RENDERTARGET|D3DUSAGE_QUERY_FILTER,D3DFMT_A16B16G16R16F);
        niFlagOnIf(mnExtraCaps,CAPS_F16FILTER,SUCCEEDED(hr));

        // check F32 filtering
        hr = CF(D3DUSAGE_RENDERTARGET|D3DUSAGE_QUERY_FILTER,D3DFMT_A32B32G32R32F);
        niFlagOnIf(mnExtraCaps,CAPS_F32FILTER,SUCCEEDED(hr));

        // check non pow2 textures
        if (GetD3D9Caps().TextureCaps&D3DPTEXTURECAPS_NONPOW2CONDITIONAL &&
            GetD3D9Caps().TextureCaps&D3DPTEXTURECAPS_POW2)
        {
          niFlagOn(mnExtraCaps,CAPS_OVERLAYTEXTURE);
        }
        else if (GetD3D9Caps().TextureCaps&D3DPTEXTURECAPS_POW2) {
        }
        else {
          niFlagOn(mnExtraCaps,CAPS_OVERLAYTEXTURE);
          niFlagOn(mnExtraCaps,CAPS_NONPOW2TEXTURE2D);
        }

        if (!(GetD3D9Caps().TextureCaps&D3DPTEXTURECAPS_CUBEMAP_POW2)) {
          niFlagOn(mnExtraCaps,CAPS_NONPOW2TEXTURECUBE);
        }
        if (!(GetD3D9Caps().TextureCaps&D3DPTEXTURECAPS_VOLUMEMAP_POW2)) {
          niFlagOn(mnExtraCaps,CAPS_NONPOW2TEXTURECUBE);
        }


#undef CF

        //             pD3DDevice9->Release();
      }

    }


    // Renderer caps
    {
      cString str;
      str += _A("=== D3D9 Graphics Driver Caps ===\n");
      cString impl = ni::GetProperty("d3d9.Impl","Default");
      str += niFmt(_A("NumTextureUnits       = %d\n"),GetCaps(eGraphicsCaps_NumTextureUnits      ));
      str += niFmt(_A("Resize                = %d\n"),GetCaps(eGraphicsCaps_Resize               ));
      str += niFmt(_A("MultiContext          = %d\n"),GetCaps(eGraphicsCaps_MultiContext         ));
      str += niFmt(_A("Texture2DMaxSize      = %d\n"),GetCaps(eGraphicsCaps_Texture2DMaxSize     ));
      str += niFmt(_A("TextureCubeMaxSize    = %d\n"),GetCaps(eGraphicsCaps_TextureCubeMaxSize   ));
      str += niFmt(_A("Texture3DMaxSize      = %d\n"),GetCaps(eGraphicsCaps_Texture3DMaxSize     ));
      str += niFmt(_A("NumRenderTargetTextures = %d\n"),GetCaps(eGraphicsCaps_NumRenderTargetTextures  ));
      str += niFmt(_A("DepthStencilTexture   = %d\n"),GetCaps(eGraphicsCaps_DepthStencilTexture  ));
      str += niFmt(_A("StencilTwoSideded     = %d\n"),GetCaps(eGraphicsCaps_StencilTwoSideded        ));
      str += niFmt(_A("StencilWrap           = %d\n"),GetCaps(eGraphicsCaps_StencilWrap          ));
      str += niFmt(_A("OcclusionQueries      = %d\n"),GetCaps(eGraphicsCaps_OcclusionQueries     ));
      str += niFmt(_A("MaxPointSize          = %.3f\n"),ultof(GetCaps(eGraphicsCaps_MaxPointSize         )));
      str += niFmt(_A("HardwareInstancing    = %d\n"),GetCaps(eGraphicsCaps_HardwareInstancing   ));
      str += niFmt(_A("ScissorTest           = %d\n"),GetCaps(eGraphicsCaps_ScissorTest          ));
      str += niFmt(_A("MaxVertexIndex        = %d\n"),GetCaps(eGraphicsCaps_MaxVertexIndex));
      str += niFmt(_A("OverlayTexture        = %d\n"),GetCaps(eGraphicsCaps_OverlayTexture));
      str += niFmt(_A("OrthoProjectionOffset = %f\n"),ni::ultof(GetCaps(eGraphicsCaps_OrthoProjectionOffset)));
      str += niFmt(_A("TransparentAA         = %s\n"),niFlagIs(mnExtraCaps,CAPS_TRANSPARENT_AA)?"yes":"no");
      niLog(Info,str.Chars());
    }

    mnExtraCaps |= CAPS_SUCCEEDED;
    return eTrue;
  }

  // D3D9 DLL
  HINSTANCE           mhD3D9Dll;
  tpfnDirect3DCreate9 mpDirect3DCreate9;
  tU32                mnExtraCaps;

  cD3D9VertexDeclarations   mVertexDeclarations;

  astl::vector<Ptr<iHString> >    mvProfiles[eShaderUnit_Last];

  cD3D9(iGraphics* apGraphics);
  ~cD3D9();

  IDirect3D9* __stdcall GetD3D9() const;
  IDirect3DDevice9* __stdcall GetD3D9Device() const;

  void __stdcall ZeroMembers();

  virtual tBool __stdcall IsOK() const;
  virtual void __stdcall Invalidate();

  const D3DCAPS9& __stdcall GetD3D9Caps() const;

  IDirect3DStateBlock9* __stdcall GetD3D9DefaultDeviceStates() const;
  IDirect3DStateBlock9* __stdcall GetD3D9DefaultPixelStates() const;
  IDirect3DStateBlock9* __stdcall GetD3D9DefaultVertexStates() const;
  tD3D9LostDeviceSinkLst* __stdcall GetLostDeviceSinkList() const { return mlstLostDeviceSinks; }
  iTexture* __stdcall CreateD3D9TextureEmpty(iHString* ahspName);
  iTexture* __stdcall CreateD3D9TextureBase(iHString* ahspName, IDirect3DBaseTexture9* apTexture);

  virtual iGraphics* __stdcall GetGraphics() const;
  virtual const achar* __stdcall GetName() const;
  virtual const achar* __stdcall GetDesc() const;
  virtual const achar* __stdcall GetDeviceName() const;

  virtual void __stdcall SetDrawOpCapture(iGraphicsDrawOpCapture* apCapture) {
    mptrDOCapture = niGetIfOK(apCapture);
  }
  virtual iGraphicsDrawOpCapture* __stdcall GetDrawOpCapture() const {
    return mptrDOCapture;
  }

  virtual iGraphicsContext* __stdcall CreateContextForWindow(
      iOSWindow* apWindow,
      const achar* aaszBBFormat, const achar* aaszDSFormat,
      tU32 anSwapInterval,
      tTextureFlags aBackBufferFlags);
  virtual iGraphicsContextRT* __stdcall CreateContextForRenderTargets(
    iTexture* apRT0, iTexture* apRT1, iTexture* apRT2, iTexture* apRT3, iTexture* apDS);
  void ReleaseStateBlocks();
  void InitStateBlocks();
  virtual tInt __stdcall GetCaps(eGraphicsCaps aCaps) const;
  virtual tGraphicsDriverImplFlags __stdcall GetGraphicsDriverImplFlags() const;

  void __stdcall _BeginScene();
  void __stdcall _TryEndScene();
  tBool __stdcall ResetAllCaches();

  void _DispatchDeviceLost();
  void _DispatchDeviceReset();
  void _ResetDevice(tBool abDoReset);

  virtual tBool __stdcall CheckTextureFormat(iBitmapFormat* apFormat, tTextureFlags aFlags);
  virtual iTexture* __stdcall CreateTexture(iHString* ahspName, eBitmapType aType, const achar* aaszFormat, tU32 anNumMipMaps, tU32 anWidth, tU32 anHeight, tU32 anDepth, tTextureFlags aFlags);
  virtual tBool __stdcall BlitTextureToBitmap(iTexture* apSrc, tU32 anSrcLevel, iBitmap2D* apDest, const sRecti& aSrcRect, const sRecti& aDestRect, eTextureBlitFlags aFlags);
  virtual tBool __stdcall BlitBitmapToTexture(iBitmap2D* apSrc, iTexture* apDest, tU32 anDestLevel, const sRecti& aSrcRect = sRecti(0,0), const sRecti& aDestRect = sRecti(0,0), eTextureBlitFlags aFlags = eTextureBlitFlags_None);
  virtual tBool __stdcall BlitTextureToTexture(iTexture* apSrc, tU32 anSrcLevel, iTexture* apDest, tU32 anDestLevel, const sRecti& aSrcRect = sRecti(0,0), const sRecti& aDestRect = sRecti(0,0), eTextureBlitFlags aFlags = eTextureBlitFlags_None);
  virtual tBool __stdcall BlitBitmap3DToTexture(iBitmap3D* apSrc, iTexture* apDest, tU32 anDestLevel, const sVec3i& aSrcMin, const sVec3i& aDestMin, const sVec3i& avSize, eTextureBlitFlags aFlags = eTextureBlitFlags_None);
  virtual tBool __stdcall BlitTextureToBitmap3D(iTexture* apSrc, tU32 anSrcLevel, iBitmap3D* apDest, const sVec3i& aSrcMin, const sVec3i& aDestMin, const sVec3i& avSize, eTextureBlitFlags aFlags = eTextureBlitFlags_None);

  tBool _DetectShaderProfiles();
  virtual tU32 __stdcall GetNumShaderProfile(eShaderUnit aUnit) const;
  virtual iHString* __stdcall GetShaderProfile(eShaderUnit aUnit, tU32 anIndex) const;
  virtual iShader* __stdcall CreateShader(iHString* ahspName, iFile* apFile);

  virtual iOcclusionQuery* __stdcall CreateOcclusionQuery();

  inline tBool DoDrawOp(iDrawOperation* apDrawOp,
                        tBool abSWInst,
                        const sMatrixf& aViewMatrix,
                        const sMatrixf& aProjMatrix,
                        const sVec4f& aBaseColor);
  void __stdcall _ClearBuffers(sD3D9Context* apContext, tClearBuffersFlags clearBuffer, tU32 anColor, tF32 afDepth, tI32 anStencil);
  tBool __stdcall _DrawOperation(sD3D9Context* apContext, iDrawOperation* apDrawOp);

  ///////////////////////////////////////////////
  virtual iVertexArray* __stdcall CreateVertexArray(tU32 anNumVertices, tFVF anFVF, eArrayUsage aUsage) {
    D3D9_CheckContext(NULL);
    return niNew cD3D9VertexArray(mpGraphics,mpD3D9Device,anNumVertices,anFVF,aUsage);
  }
  ///////////////////////////////////////////////
  virtual iIndexArray* __stdcall CreateIndexArray(eGraphicsPrimitiveType aPrimitiveType, tU32 anNumIndex, tU32 anMaxVertexIndex, eArrayUsage aUsage) {
    D3D9_CheckContext(NULL);
    return niNew cD3D9IndexArray(
        mpGraphics,mpD3D9Device,
        aPrimitiveType,anNumIndex,anMaxVertexIndex,aUsage,GetCaps(eGraphicsCaps_MaxVertexIndex));
  }

  ///////////////////////////////////////////////
  virtual iShaderConstants* __stdcall CreateShaderConstants(tU32) const {
    niAssertUnreachable("Not implemented.");
    return NULL;
  }
  virtual tIntPtr __stdcall CompileSamplerStates(iSamplerStates* apStates) {
    niAssertUnreachable("Not implemented.");
    return 0;
  }
  virtual tIntPtr __stdcall CompileRasterizerStates(iRasterizerStates* apStates) {
    niAssertUnreachable("Not implemented.");
    return 0;
  }
  virtual tIntPtr __stdcall CompileDepthStencilStates(iDepthStencilStates* apStates) {
    niAssertUnreachable("Not implemented.");
    return 0;
  }

  ///////////////////////////////////////////////
  virtual IDirect3DSurface9* __stdcall CreateD3D9Surface(tU32 anW, tU32 anH, D3DFORMAT aFmt, D3DPOOL aPool);
  virtual iBitmap2D* __stdcall LockD3D9Surface(IDirect3DSurface9* apSurf, DWORD aLockFlags);
  virtual IDirect3DSurface9* __stdcall CreateD3D9SurfaceFromBitmap(iBitmap2D* apBmp, D3DFORMAT aFmt, D3DPOOL aPool);
  virtual IDirect3DSurface9* __stdcall CreateD3D9SurfaceFromBitmapRect(iBitmap2D* apBmp, D3DFORMAT aFmt, D3DPOOL aPool, const sRecti& aRect);

  niEndClass(cD3D9);
};

//--------------------------------------------------------------------------------------------
//
//
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
cD3D9::cD3D9(iGraphics* apGraphics)
    : mlstLostDeviceSinks(tD3D9LostDeviceSinkLst::Create())
{
  ZeroMembers();
  mpGraphics = apGraphics;

  memset(&mCompTest,0,sizeof(mCompTest));
  mCompTest.mbIA16 = ni::GetProperty("d3d9.CompIA16","0").Bool();
  mCompTest.mbNoCubeMap = ni::GetProperty("d3d9.CompNoCubeMap","0").Bool();
  mCompTest.mbNoVolume = ni::GetProperty("d3d9.CompNoVolume","0").Bool();
  mCompTest.mnMaxVS = ni::GetProperty("d3d9.CompMaxVS","9999999").Long();
  mCompTest.mnMaxPS = ni::GetProperty("d3d9.CompMaxPS","9999999").Long();
  mCompTest.mnMaxTU = ni::GetProperty("d3d9.CompMaxTU","32").Long();

  mhD3D9Dll = LoadLibrary(_A("d3d9.dll"));
  if (!mhD3D9Dll) {
    niError(_A("Can't load D3D9 DLL."));
    return;
  }

  mpDirect3DCreate9 = (tpfnDirect3DCreate9)GetProcAddress(mhD3D9Dll, "Direct3DCreate9");
  if (!mpDirect3DCreate9) {
    niError(_A("Can't get Direct3DCreate9 function."));
    return;
  }
}

///////////////////////////////////////////////
cD3D9::~cD3D9() {
  Invalidate();
  if (mhD3D9Dll) {
    ::FreeLibrary(mhD3D9Dll);
  }
}

///////////////////////////////////////////////
IDirect3D9* __stdcall cD3D9::GetD3D9() const {
  if (!niThis(cD3D9)->_InitCaps()) return NULL;
  return mpD3D9;
}

///////////////////////////////////////////////
IDirect3DDevice9* __stdcall cD3D9::GetD3D9Device() const {
  return mpD3D9Device;
}

///////////////////////////////////////////////
void __stdcall cD3D9::ZeroMembers() {
  mbBeganRendering = eFalse;
  mpGraphics = NULL;
  mpDirect3DCreate9 = NULL;
  mpD3D9 = NULL;
  mpD3D9Device = NULL;
  mpD3D9DefaultDeviceStates = NULL;
  mpD3D9DefaultPixelStates = NULL;
  mpD3D9DefaultVertexStates = NULL;
  mnExtraCaps = 0;
}

///////////////////////////////////////////////
tBool __stdcall cD3D9::IsOK() const {
#pragma niNote("Dont put _InitCaps, otherwise a D3D9 call will be made from DLL main which seems to crash drivers randomly... robust ^_^")
  return mpDirect3DCreate9 != NULL;
}

///////////////////////////////////////////////
void __stdcall cD3D9::Invalidate() {
  if (mpD3D9Device) {
    astl::vector<sD3D9Context*> ctxs = mvContexts;
    niLoop(i,ctxs.size()) {
      ctxs[i]->Invalidate();
    }
    niAssert(mvContexts.size() == 0);

    tU32 i;
    for (i = 0; i < eShaderUnit_Last; ++i) {
      mvProfiles[i].clear();
    }

    ReleaseStateBlocks();
    niSafeRelease(mpD3D9Device);
    mptrPrimaryContext = NULL;
  }

  niSafeRelease(mpD3D9);
}

///////////////////////////////////////////////
const D3DCAPS9& __stdcall cD3D9::GetD3D9Caps() const {
  return mModeSelector.GetDefaultAdapter()->mCaps;
}

///////////////////////////////////////////////
IDirect3DStateBlock9* __stdcall cD3D9::GetD3D9DefaultDeviceStates() const {
  return mpD3D9DefaultDeviceStates;
}

///////////////////////////////////////////////
IDirect3DStateBlock9* __stdcall cD3D9::GetD3D9DefaultPixelStates() const {
  return mpD3D9DefaultPixelStates;
}

///////////////////////////////////////////////
IDirect3DStateBlock9* __stdcall cD3D9::GetD3D9DefaultVertexStates() const {
  return mpD3D9DefaultVertexStates;
}

///////////////////////////////////////////////
iTexture* __stdcall cD3D9::CreateD3D9TextureEmpty(iHString* ahspName)
{
  return niNew cD3D9Texture(mpGraphics,ahspName);
}

///////////////////////////////////////////////
iTexture* __stdcall cD3D9::CreateD3D9TextureBase(iHString* ahspName, IDirect3DBaseTexture9* apTexture)
{
  Ptr<cD3D9Texture> ptrTex = niNew cD3D9Texture(mpGraphics,ahspName);
  niCheck(ptrTex->InitializeBaseTexture(apTexture),NULL);
  return ptrTex.GetRawAndSetNull();
}

//--------------------------------------------------------------------------------------------
//
//  D3D9 System
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
iGraphics* __stdcall cD3D9::GetGraphics() const {
  return mpGraphics;
}

///////////////////////////////////////////////
const achar* __stdcall cD3D9::GetName() const {
  return _kaszD3D9Name;
}

///////////////////////////////////////////////
const achar* __stdcall cD3D9::GetDesc() const {
  return _kaszD3D9Desc;
}

///////////////////////////////////////////////
const achar* __stdcall cD3D9::GetDeviceName() const {
  return _kaszD3D9Device;
}

///////////////////////////////////////////////
void cD3D9::ReleaseStateBlocks() {
  niSafeRelease(mpD3D9DefaultDeviceStates);
  niSafeRelease(mpD3D9DefaultPixelStates);
  niSafeRelease(mpD3D9DefaultVertexStates);
}

///////////////////////////////////////////////
void cD3D9::InitStateBlocks() {
  // Get the state blocks
  mpD3D9Device->CreateStateBlock(D3DSBT_ALL,&mpD3D9DefaultDeviceStates);
  mpD3D9Device->CreateStateBlock(D3DSBT_PIXELSTATE,&mpD3D9DefaultPixelStates);
  mpD3D9Device->CreateStateBlock(D3DSBT_VERTEXSTATE,&mpD3D9DefaultVertexStates);
}

///////////////////////////////////////////////
tInt __stdcall cD3D9::GetCaps(eGraphicsCaps aCaps) const {
  if (!niThis(cD3D9)->_InitCaps()) {
    return 0;
  }

  niAssert(aCaps < eGraphicsCaps_Last);
  switch (aCaps)
  {
    case eGraphicsCaps_NumTextureUnits: {
      if (mCompTest.mnMaxTU)
        return mCompTest.mnMaxTU;
      return (D3DSHADER_VERSION_MAJOR(GetD3D9Caps().PixelShaderVersion) >= 2) ?
          16 : GetD3D9Caps().MaxSimultaneousTextures;
    }
    case eGraphicsCaps_Resize: {
      return eTrue;
    }
    case eGraphicsCaps_MultiContext: {
      return eTrue;
    }
    case eGraphicsCaps_Texture2DMaxSize: {
      return ni::Min(GetD3D9Caps().MaxTextureWidth, GetD3D9Caps().MaxTextureHeight);
    }
    case eGraphicsCaps_TextureCubeMaxSize: {
      if (mCompTest.mbNoCubeMap || !(GetD3D9Caps().TextureCaps&D3DPTEXTURECAPS_CUBEMAP))
        return 0;
      return ni::Min(GetD3D9Caps().MaxTextureWidth, GetD3D9Caps().MaxTextureHeight);
    }
    case eGraphicsCaps_Texture3DMaxSize: {
      if (mCompTest.mbNoVolume || !(GetD3D9Caps().TextureCaps&D3DPTEXTURECAPS_VOLUMEMAP))
        return 0;
      return GetD3D9Caps().MaxVolumeExtent;
    }
    case eGraphicsCaps_NumRenderTargetTextures: {
      return GetD3D9Caps().NumSimultaneousRTs;
    }
    case eGraphicsCaps_DepthStencilTexture: {
      return niFlagIs(mnExtraCaps,CAPS_DEPTHTEXTURE);
    }
    case eGraphicsCaps_StencilTwoSideded: {
      return niFlagIs(GetD3D9Caps().StencilCaps,D3DSTENCILCAPS_TWOSIDED);
    }
    case eGraphicsCaps_StencilWrap: {
      return niFlagIs(GetD3D9Caps().StencilCaps,D3DSTENCILCAPS_INCR) &&
          niFlagIs(GetD3D9Caps().StencilCaps,D3DSTENCILCAPS_DECR);
    }
    case eGraphicsCaps_OcclusionQueries: {
      return niFlagIs(mnExtraCaps,CAPS_OCCLUSIONQUERIES);
    }
    case eGraphicsCaps_MaxPointSize: {
      return ni::ftoul(GetD3D9Caps().MaxPointSize);
    }
    case eGraphicsCaps_HardwareInstancing: {
      return niFlagIs(mnExtraCaps,CAPS_HWINST);
    }
    case eGraphicsCaps_ScissorTest: {
      return niFlagIs(GetD3D9Caps().RasterCaps,D3DPRASTERCAPS_SCISSORTEST);
    }
    case eGraphicsCaps_MaxVertexIndex: {
      // use 65534 for IA16 compatibility mode because it'll be a more robust test
      // since some Intel GPU return that even with 16bit index support... (hint 0xFFFF-1)
      return mCompTest.mbIA16 ? 65534 : GetD3D9Caps().MaxVertexIndex;
    }
    case eGraphicsCaps_OverlayTexture: {
      return niFlagIs(mnExtraCaps,CAPS_OVERLAYTEXTURE);
    }
    case eGraphicsCaps_OrthoProjectionOffset: {
      return ftoul(-0.5f);
    }
    case eGraphicsCaps_BlitBackBuffer: {
      return eTrue;
    }
  }

  return 0;
}

///////////////////////////////////////////////
tGraphicsDriverImplFlags __stdcall cD3D9::GetGraphicsDriverImplFlags() const {
  return eGraphicsDriverImplFlags_VertexArrayObject|
      eGraphicsDriverImplFlags_IndexArrayObject;
}

///////////////////////////////////////////////
void cD3D9::_BeginScene() {
  if (!mbBeganRendering) {
    HRESULT hr = mpD3D9Device->BeginScene();
    if (FAILED(hr)) {
      D3D9ERROR();
    }
    mbBeganRendering = eTrue;
  }
}

void cD3D9::_TryEndScene() {
  if (mbBeganRendering) {
    HRESULT hr = mpD3D9Device->EndScene();
    if (FAILED(hr)) {
      D3D9ERROR();
    }
    mbBeganRendering = eFalse;
  }
}

///////////////////////////////////////////////
tBool __stdcall cD3D9::ResetAllCaches() {
  _TryEndScene();
  mVertexDeclarations.ResetVertexDeclaration();
  mCache.Reset();
  return eTrue;
}

///////////////////////////////////////////////
void cD3D9::_DispatchDeviceLost()
{
  D3DRESET_TRACE(_A("### D3D-RESET: _DispatchDeviceLost ###\n"));

  // Unbind
  mpD3D9Device->SetVertexDeclaration(NULL);
  mpD3D9Device->SetStreamSource(0,NULL,0,0);
  mpD3D9Device->SetIndices(NULL);
  mpD3D9Device->SetPixelShader(NULL);
  mpD3D9Device->SetVertexShader(NULL);
  niLoop(i,MAX_TEXTUREUNIT) {
    mpD3D9Device->SetTexture(i,NULL);
  }
  niLoop(i,ni::Min(MAX_RT,GetD3D9Caps().NumSimultaneousRTs)) {
    mpD3D9Device->SetRenderTarget(i,NULL);
  }
  mpD3D9Device->SetDepthStencilSurface(NULL);

  // Notify the contexts that the device has been lost
  niLoop(i,mvContexts.size()) {
    D3D9_ContextDeviceLost(mvContexts[i]);
    D3D9_Clear(mvContexts[i],_A("Restoring Device..."));
  }

  // Release the state block
  ReleaseStateBlocks();

  // reset textures
  {
    Ptr<iDeviceResourceManager> texResMan = mpGraphics->GetTextureDeviceResourceManager();
    niLoop(i,texResMan->GetSize()) {
      Ptr<iDeviceResource> res = texResMan->GetFromIndex(i);
      QPtr<iD3D9Texture> texI = res.ptr();
      if (texI.IsOK()) {
        texI->OnLostDevice(mpD3D9Device);
      }
    }
  }
  // generic resources
  {
    Ptr<iDeviceResourceManager> genResMan = mpGraphics->GetGenericDeviceResourceManager();
    niLoop(i,genResMan->GetSize()) {
      Ptr<iDeviceResource> res = genResMan->GetFromIndex(i);
      QPtr<iD3D9VertexArray> vaI = res.ptr();
      if (vaI.IsOK()) {
        vaI->OnLostDevice(mpD3D9Device);
        continue;
      }
      QPtr<iD3D9IndexArray> iaI = res.ptr();
      if (iaI.IsOK()) {
        iaI->OnLostDevice(mpD3D9Device);
        continue;
      }
      QPtr<iD3D9OcclusionQuery> ocI = res.ptr();
      if (ocI.IsOK()) {
        ocI->OnLostDevice(mpD3D9Device);
        continue;
      }
    }
  }
  // sinks
  {
    niCallSinkVoid_(D3D9LostDeviceSink,mlstLostDeviceSinks,LostDevice,(mpD3D9Device));
  }
}

///////////////////////////////////////////////
void cD3D9::_DispatchDeviceReset()
{
  D3DRESET_TRACE(_A("### D3D-RESET: _DispatchDeviceReset ###\n"));

  // reset textures
  {
    Ptr<iDeviceResourceManager> texResMan = mpGraphics->GetTextureDeviceResourceManager();
    niLoop(i,texResMan->GetSize()) {
      iDeviceResource* res = texResMan->GetFromIndex(i);
      QPtr<iD3D9Texture> texI = res;
      if (texI.IsOK()) {
        cD3D9Texture* tex = niStaticCast(cD3D9Texture*,texI.ptr());
        if (!tex->OnResetDevice(mpD3D9Device)) {
          niWarning(niFmt(_A("Can't restore texture resource '%s'."),niHStr(tex->GetDeviceResourceName())));
        }
      }
    }
  }
  // generic resources
  {
    Ptr<iDeviceResourceManager> genResMan = mpGraphics->GetGenericDeviceResourceManager();
    niLoop(i,genResMan->GetSize()) {
      iDeviceResource* res = genResMan->GetFromIndex(i);
      {QPtr<iD3D9VertexArray> vaI = res;
        if (vaI.IsOK()) {
          if (!vaI->OnResetDevice(mpD3D9Device)) {
            niWarning(niFmt(_A("Can't restore vertex array resource '%s'."),niHStr(res->GetDeviceResourceName())));
          }
          continue;
        }}
      {QPtr<iD3D9IndexArray> iaI = res;
        if (iaI.IsOK()) {
          if (!iaI->OnResetDevice(mpD3D9Device)) {
            niWarning(niFmt(_A("Can't restore index array resource '%s'."),niHStr(res->GetDeviceResourceName())));
          }
          continue;
        }}
      {QPtr<iD3D9OcclusionQuery> ocI = res;
        if (ocI.IsOK()) {
          if (!ocI->OnResetDevice(mpD3D9Device)) {
            niWarning(niFmt(_A("Can't restore occlusion query resource '%s'."),niHStr(res->GetDeviceResourceName())));
            continue;
          }
        }}
    }
  }
  // sinks
  {
    niCallSinkVoid_(D3D9LostDeviceSink,mlstLostDeviceSinks,ResetDevice,(mpD3D9Device));
  }

  niLoop(i,mvContexts.size()) {
    D3D9_Clear(mvContexts[i],_A("Restoring Device..."));
    D3D9_FinalizeInitialize(mvContexts[i],eTrue);
    D3D9_Clear(mvContexts[i],_A("Restoring Device..."));
  }
  niLoop(i,mvContexts.size()) {
    D3D9_UpdateWindow(mvContexts[i]);
  }
}

///////////////////////////////////////////////
void cD3D9::_ResetDevice(tBool abDoReset) {
  niLoop(i,mvContexts.size()) {
    D3D9_Clear(mvContexts[i],_A("Restoring Device..."));
  }

  tBool bDispatchedDeviceLost = ni::eFalse;
  HRESULT hr = mpD3D9Device->TestCooperativeLevel();
  if (SUCCEEDED(hr) && abDoReset) {
    D3DRESET_TRACE(_A("### D3D-RESET: <MANUAL> ###\n"));
    if (!bDispatchedDeviceLost) {
      _DispatchDeviceLost();
      bDispatchedDeviceLost = ni::eTrue;
    }
    hr = D3DERR_DEVICENOTRESET; // Call DEVICENOTRESET handler
  }
  else {
    D3DRESET_TRACE(_A("### D3D-RESET: <AUTO> ###\n"));
  }

  const tF64 maxSecs = 15.0;
  tI32 minTrys = 5;
  tF64 start = ni::TimerInSeconds();
  do {
    ni::SleepMs(300); // make sure we wait a bit between each retry

    switch (hr) {
      case D3DERR_DEVICELOST:
        {
          D3DRESET_TRACE(_A("### D3D-RESET: DEVICELOST ###\n"));
          if (!bDispatchedDeviceLost) {
            _DispatchDeviceLost();
            bDispatchedDeviceLost = ni::eTrue;
          }
          hr = D3DERR_DEVICENOTRESET;
          break;
        }
      case D3DERR_DEVICENOTRESET:
        {
          D3DRESET_TRACE(_A("### D3D-RESET: DEVICENOTRESET ###\n"));
          if (!bDispatchedDeviceLost) {
            _DispatchDeviceLost();
            bDispatchedDeviceLost = ni::eTrue;
          }
          hr = D3D9_ResetContext(mptrPrimaryContext); // Call DEVICENOTRESET handler
          if (SUCCEEDED(hr)) {
            _DispatchDeviceReset();
            D3DRESET_TRACE(niFmt(_A("### D3D-RESET: |SUCCEDED| ###\n")));
            hr = S_OK;  // SUCCEDED, continue
          }
          else {
            bDispatchedDeviceLost = ni::eFalse;
            hr = mpD3D9Device->TestCooperativeLevel();
          }
          break;
        }
    }
    niLoop(i,mvContexts.size()) {
      D3D9_UpdateWindow(mvContexts[i]);
    }
    if (minTrys > 0) {
      --minTrys;
    }
    if (minTrys <= 0 && (ni::TimerInSeconds()-start) > maxSecs) {
      D3D9ERROR();
      D3DRESET_TRACE(niFmt(_A("### D3D-RESET: |FAILED| ###\n")));
      ni::GetLang()->FatalError(niFmt(_A("Can't reset DirectX9 graphics device.")));
    }
  } while (!SUCCEEDED(hr));
}

///////////////////////////////////////////////
tBool __stdcall cD3D9::CheckTextureFormat(iBitmapFormat* apFormat, tTextureFlags aFlags)
{
  D3D9_CheckContext(eFalse);
  niCheckIsOK(apFormat,eFalse);

  UINT w = apFormat->GetWidth() ? apFormat->GetWidth() : 128;
  UINT h = apFormat->GetHeight() ? apFormat->GetHeight() : 128;
  UINT d = apFormat->GetDepth() ? apFormat->GetDepth() : 128;
  D3DFORMAT fmt = GetD3DFormatMap()->GetNearestD3DFormat(apFormat->GetPixelFormat(),NULL);
  if (fmt == D3DFMT_UNKNOWN) {
    niWarning(niFmt("Unknown format '%s'.",apFormat->GetPixelFormat()->GetFormat()));
    return eFalse;
  }
  UINT numMips = ((apFormat->GetNumMipMaps() == 0) && niFlagIs(aFlags,eTextureFlags_MipMaps)) ?
      ComputeNumPow2Levels(w>>1,h>>1) :
      apFormat->GetNumMipMaps();

  switch (apFormat->GetType()) {
    case eBitmapType_2D:
      {
        if (FAILED(D3DXCheckTextureRequirements(
                mpD3D9Device,
                &w,&h,&numMips,
                D3D9_GetTextureFlagsUSAGE(aFlags),
                &fmt,
                D3D9_GetTextureFlagsPOOL(aFlags))))
          return eFalse;
        break;
      }
    case eBitmapType_Cube:
      {
        if (FAILED(D3DXCheckCubeTextureRequirements(
                mpD3D9Device,
                &w,&numMips,
                D3D9_GetTextureFlagsUSAGE(aFlags),
                &fmt,
                D3D9_GetTextureFlagsPOOL(aFlags))))
          return eFalse;
        break;
      }
    case eBitmapType_3D:
      {
        if (FAILED(D3DXCheckVolumeTextureRequirements(
                mpD3D9Device,
                &w,&h,&d,&numMips,
                D3D9_GetTextureFlagsUSAGE(aFlags),
                &fmt,
                D3D9_GetTextureFlagsPOOL(aFlags))))
          return eFalse;
        break;
      }
  }

  //     apFormat->SetWidth(w);
  //     apFormat->SetHeight(h);
  //     apFormat->SetDepth(d);
  //     apFormat->SetNumMipMaps(numMips);
  // apFormat->SetPixelFormat(mpGraphics->CreatePixelFormat(GetD3DFormatMap()->GetPixelFormat(fmt)));
  return eTrue;
}

///////////////////////////////////////////////
iTexture* __stdcall cD3D9::CreateTexture(iHString* ahspName, eBitmapType aType, const achar* aaszFormat, tU32 anNumMipMaps, tU32 anWidth, tU32 anHeight, tU32 anDepth, tTextureFlags aFlags) {
  D3D9_CheckContext(NULL);

  D3DFORMAT fmt = GetD3DFormatMap()->GetNearestD3DFormat(mpGraphics,aaszFormat);
  niCheck(fmt != D3DFMT_UNKNOWN, NULL);
  DWORD usage = D3D9_GetTextureFlagsUSAGE(aFlags);
  D3DPOOL pool = D3D9_GetTextureFlagsPOOL(aFlags);

  HRESULT hr;
  switch (aType) {
    case eBitmapType_2D:
      {
        tU32 nMaxSize = GetCaps(eGraphicsCaps_Texture2DMaxSize);

        // Correct the texture size
        if (niFlagIs(aFlags,eTextureFlags_Overlay))
        {
          if (!niFlagIs(mnExtraCaps,CAPS_OVERLAYTEXTURE)) {
            anWidth = ni::GetNearestPow2(anWidth);
            anHeight = ni::GetNearestPow2(anHeight);
          }
        }
        else {
          if (!niFlagIs(mnExtraCaps,CAPS_NONPOW2TEXTURE2D)) {
            anWidth = ni::GetNearestPow2(anWidth);
            anHeight = ni::GetNearestPow2(anHeight);
          }
        }

        if (anWidth > nMaxSize) {
          anWidth = nMaxSize;
        }
        if (anHeight > nMaxSize) {
          anHeight = nMaxSize;
        }

        anNumMipMaps = ((anNumMipMaps == 0) && niFlagIs(aFlags,eTextureFlags_MipMaps)) ?
            ComputeNumPow2Levels(anWidth>>1,anHeight>>1) : anNumMipMaps;

        niAssert(mpD3D9Device != NULL);
        hr = D3DXCheckTextureRequirements(mpD3D9Device,NULL,NULL,NULL,usage,&fmt,pool);
        if (FAILED(hr)) {
          D3D9ERROR();
          niError(_A("Check texture requirements failed."));
        }
        break;
      }
    case eBitmapType_Cube:
      {
        tU32 nMaxSize = GetCaps(eGraphicsCaps_TextureCubeMaxSize);
        if (!nMaxSize) {
          niError(_A("Cube textures not supported."));
          return NULL;
        }

        if (!niFlagIs(mnExtraCaps,CAPS_NONPOW2TEXTURECUBE)) {
          anWidth = ni::GetNearestPow2(anWidth);
        }
        if (anWidth > nMaxSize) {
          anWidth = nMaxSize;
        }

        anNumMipMaps = ((anNumMipMaps == 0) && niFlagIs(aFlags,eTextureFlags_MipMaps)) ?
            ComputeNumPow2Levels(anWidth>>1,anWidth>>1) : anNumMipMaps;

        IDirect3DCubeTexture9* pCube;
        niAssert(mpD3D9Device != NULL);
        hr = D3DXCheckCubeTextureRequirements(mpD3D9Device,NULL,NULL,usage,&fmt,pool);
        if (FAILED(hr)) {
          D3D9ERROR();
          niError(_A("Check cube texture requirements failed."));
        }
        else {
          hr = mpD3D9Device->CreateCubeTexture(anWidth,anNumMipMaps+1,usage,fmt,pool,&pCube,NULL);
          if (FAILED(hr)) {
            D3D9ERROR();
            niError(_A("Create cube texture failed."));
          }
        }
        break;
      }
    case eBitmapType_3D:
      {
        tU32 nMaxSize = GetCaps(eGraphicsCaps_Texture3DMaxSize);
        if (!nMaxSize) {
          niError(_A("3D textures not supported."));
          return NULL;
        }

        if (!niFlagIs(mnExtraCaps,CAPS_NONPOW2TEXTURE3D)) {
          anWidth = ni::GetNearestPow2(anWidth);
          anHeight = ni::GetNearestPow2(anHeight);
          anDepth = ni::GetNearestPow2(anDepth);
        }
        if (anWidth > nMaxSize) {
          anWidth = nMaxSize;
        }
        if (anHeight > nMaxSize) {
          anHeight = nMaxSize;
        }
        if (anDepth > nMaxSize) {
          anDepth = nMaxSize;
        }

        anNumMipMaps = ((anNumMipMaps == 0) && niFlagIs(aFlags,eTextureFlags_MipMaps)) ?
            ComputeNumPow2Levels(anWidth>>1,anHeight>>1,anDepth>>1) : anNumMipMaps;

        IDirect3DVolumeTexture9* pVol;

        niAssert(mpD3D9Device != NULL);
        hr = D3DXCheckVolumeTextureRequirements(mpD3D9Device,NULL,NULL,NULL,NULL,usage,&fmt,pool);
        if (FAILED(hr)) {
          D3D9ERROR();
          niError(_A("Check volume texture requirements failed."));
        }
        else {
          hr = mpD3D9Device->CreateVolumeTexture(anWidth,anHeight,anDepth,anNumMipMaps+1,usage,fmt,pool,&pVol,NULL);
          if (FAILED(hr)) {
            D3D9ERROR();
            niError(_A("Create volume texture failed."));
          }
        }
        break;
      }
  }

  Ptr<cD3D9Texture> ptrTex = niNew cD3D9Texture(mpGraphics,ahspName);
  niAssert(mpD3D9Device != NULL);
  if (!ptrTex->Create(mpD3D9Device,aType,fmt,anWidth,anHeight,anDepth,anNumMipMaps,aFlags)) {
    niError(niFmt(_A("Can't create texture (type: %s, mips: %d, size: %dx%dx%d, format: %s, flags: %s)."),
                  niFlagsToChars(eBitmapType,aType),
                  anNumMipMaps,
                  anWidth,anHeight,anDepth,
                  aaszFormat,
                  niFlagsToChars(eTextureFlags,aFlags)));
    return NULL;
  }

  return ptrTex.GetRawAndSetNull();
}

///////////////////////////////////////////////
tBool __stdcall cD3D9::BlitBitmapToTexture(iBitmap2D* apSrc, iTexture* apDest, tU32 anDestLevel, const sRecti& aSrcRect, const sRecti& aDestRect, eTextureBlitFlags aFlags) {
  D3D9_CheckContext(eFalse);
  niCheck(niIsOK(apDest) && D3D9_IsDeviceTexture(apDest),eFalse);
  niCheck(apDest->GetType() == eBitmapType_2D,eFalse);

  sGDD3DTextureBitmapLock bmpDest(mpD3D9Device,mpGraphics,apDest,aDestRect,anDestLevel,D3DLOCK_DISCARD);
  niCheck(bmpDest.IsOK(),eFalse);

  if (!bmpDest->BlitStretch(apSrc,
                            aSrcRect.Left(),
                            aSrcRect.Top(),
                            /*aDestRect.Left()*/0,  // zero, the locked rectangle starts here
                            /*aDestRect.Top()*/0,
                            aSrcRect.GetWidth(),
                            aSrcRect.GetHeight(),
                            aDestRect.GetWidth(),
                            aDestRect.GetHeight()))
  {
    niError(_A("Blitting failed."));
    return eFalse;
  }

  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cD3D9::BlitTextureToBitmap(iTexture* apSrc, tU32 anSrcLevel, iBitmap2D* apDest, const sRecti& aSrcRect, const sRecti& aDestRect, eTextureBlitFlags aFlags) {
  D3D9_CheckContext(eFalse);
  niCheck(niIsOK(apSrc) && D3D9_IsDeviceTexture(apSrc),eFalse);
  niCheck(apSrc->GetType() == eBitmapType_2D,eFalse);

  sGDD3DTextureBitmapLock bmpSrc(mpD3D9Device,mpGraphics,apSrc,aSrcRect,anSrcLevel,D3DLOCK_READONLY);
  niCheck(bmpSrc.IsOK(),eFalse);

  if (!apDest->BlitStretch(bmpSrc.GetBitmap(),
                           /*aSrcRect.Left()*/0,   // zero, the locked rectangle starts here
                           /*aSrcRect.Top()*/0,
                           aDestRect.Left(),   // zero, the locked rectangle starts here
                           aDestRect.Top(),
                           aSrcRect.GetWidth(),
                           aSrcRect.GetHeight(),
                           aDestRect.GetWidth(),
                           aDestRect.GetHeight()))
  {
    niError(_A("Blitting failed."));
    return eFalse;
  }

  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cD3D9::BlitTextureToTexture(iTexture* apSrc, tU32 anSrcLevel, iTexture* apDest, tU32 anDestLevel, const sRecti& aSrcRect, const sRecti& aDestRect, eTextureBlitFlags aFlags) {
  D3D9_CheckContext(eFalse);
  niCheck(niIsOK(apSrc) && D3D9_IsDeviceTexture(apSrc),eFalse);
  niCheck(apSrc->GetType() == eBitmapType_2D,eFalse);
  niCheck(niIsOK(apDest) && D3D9_IsDeviceTexture(apDest),eFalse);
  niCheck(apDest->GetType() == eBitmapType_2D,eFalse);

  if (niFlagIs(GetD3D9Caps().DevCaps2,D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES) &&
      niFlagIs(apSrc->GetFlags(),eTextureFlags_RenderTarget) &&
      niFlagIs(apDest->GetFlags(),eTextureFlags_RenderTarget) &&
      apSrc->GetPixelFormat()->IsSamePixelFormat(apDest->GetPixelFormat()))
  {
    QPtr<iD3D9Texture> srcTex = _BindCastTex(apSrc);
    QPtr<iD3D9Texture> dstTex = _BindCastTex(apDest);
    niCheck(srcTex.IsOK() && dstTex.IsOK(), eFalse);

    IDirect3DSurface9* pSrcSurf = srcTex->GetSurfaceLevel(anSrcLevel);
    IDirect3DSurface9* pDestSurf = dstTex->GetSurfaceLevel(anSrcLevel);
    if (!pSrcSurf || !pDestSurf) {
      niError(_A("Can't get the source and destionation surfaces."));
      niSafeRelease(pSrcSurf);
      niSafeRelease(pDestSurf);
      return eFalse;
    }

    HRESULT hr = mpD3D9Device->StretchRect(pSrcSurf,(CONST RECT*)&aSrcRect,
                                           pDestSurf,(CONST RECT*)&aDestRect,
                                           niFlagIs(aFlags,eTextureBlitFlags_BilinearFilter)?D3DTEXF_LINEAR:D3DTEXF_POINT);
    if (FAILED(hr)) {
      D3D9ERROR();
      niError(niFmt("D3D9Device::StretchRect(%d,%s,%d,%s) failed.",anSrcLevel,aSrcRect,anDestLevel,aDestRect));
      niSafeRelease(pSrcSurf);
      niSafeRelease(pDestSurf);
      return eFalse;
    }

    pSrcSurf->Release();
    pDestSurf->Release();
  }
  else {
    sGDD3DTextureBitmapLock bmpSrc(mpD3D9Device,mpGraphics,apSrc,aSrcRect,anSrcLevel,D3DLOCK_READONLY);
    niCheck(bmpSrc.IsOK(),eFalse);
    sGDD3DTextureBitmapLock bmpDest(mpD3D9Device,mpGraphics,apDest,aDestRect,anDestLevel,D3DLOCK_DISCARD);
    niCheck(bmpDest.IsOK(),eFalse);
    if (!bmpDest->BlitStretch(bmpSrc.GetBitmap(),
                              0,0,0,0,
                              aSrcRect.GetWidth(),aSrcRect.GetHeight(),
                              aDestRect.GetWidth(),aDestRect.GetHeight()))
    {
      niError(_A("Blitting failed."));
      return eFalse;
    }
  }

  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cD3D9::BlitBitmap3DToTexture(iBitmap3D* apSrc, iTexture* apDest, tU32 anDestLevel, const sVec3i& aSrcMin, const sVec3i& aDestMin, const sVec3i& avSize, eTextureBlitFlags aFlags)
{
  niCheck(niIsOK(apDest) && D3D9_IsDeviceTexture(apDest),eFalse);
  niCheck(apDest->GetType() == eBitmapType_3D,eFalse);

  sGDD3DTextureVolumeLock bmpDest(mpD3D9Device,mpGraphics,apDest,aDestMin,avSize,anDestLevel,D3DLOCK_DISCARD);
  niCheck(bmpDest.IsOK(),eFalse);

  if (!bmpDest->Blit(apSrc,aSrcMin,sVec3i::Zero(),avSize)) {
    niError(_A("Blitting failed."));
    return eFalse;
  }

  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cD3D9::BlitTextureToBitmap3D(iTexture* apSrc, tU32 anSrcLevel, iBitmap3D* apDest, const sVec3i& aSrcMin, const sVec3i& aDestMin, const sVec3i& avSize, eTextureBlitFlags aFlags)
{
  niCheck(niIsOK(apDest) && D3D9_IsDeviceTexture(apDest),eFalse);
  niCheck(apDest->GetType() == eBitmapType_3D,eFalse);

  sGDD3DTextureVolumeLock bmpSrc(mpD3D9Device,mpGraphics,apSrc,aSrcMin,avSize,anSrcLevel,D3DLOCK_READONLY);
  niCheck(bmpSrc.IsOK(),eFalse);

  if (!apDest->Blit(bmpSrc.mptrBitmap,aDestMin,sVec3i::Zero(),avSize)) {
    niError(_A("Blitting failed."));
    return eFalse;
  }

  return eTrue;
}


///////////////////////////////////////////////
tBool cD3D9::_DetectShaderProfiles()
{
  tU32 i;
  for (i = 0; i < eShaderUnit_Last; ++i)
    mvProfiles[i].clear();

  if (mCompTest.mnMaxVS >= 10 &&
      D3DSHADER_VERSION_MAJOR(GetD3D9Caps().VertexShaderVersion) >= 1 &&
      (D3DSHADER_VERSION_MINOR(GetD3D9Caps().VertexShaderVersion) >= 1 ||
       D3DSHADER_VERSION_MAJOR(GetD3D9Caps().VertexShaderVersion) > 1))
  {
    if (SupportsVSHLSL(mpD3D9Device,"vs_1_1")) {
      mvProfiles[eShaderUnit_Vertex].push_back(_H("hlsl_vs_1_1"));
    }
  }
  if (mCompTest.mnMaxVS >= 20 &&
      D3DSHADER_VERSION_MAJOR(GetD3D9Caps().VertexShaderVersion) >= 2)
  {
    if (SupportsVSHLSL(mpD3D9Device,"vs_2_0")) {
      mvProfiles[eShaderUnit_Vertex].push_back(_H("hlsl_vs_2_0"));
    }

    if (mCompTest.mnMaxVS >= 21 &&
        GetD3D9Caps().VS20Caps.NumTemps &&
        GetD3D9Caps().VS20Caps.DynamicFlowControlDepth &&
        niFlagIs(GetD3D9Caps().VS20Caps.Caps,D3DVS20CAPS_PREDICATION))
    {
      if (SupportsVSHLSL(mpD3D9Device,"vs_2_a")) {
        mvProfiles[eShaderUnit_Vertex].push_back(_H("hlsl_vs_2_a"));
      }
    }
  }

  if (mCompTest.mnMaxVS >= 30 &&
      D3DSHADER_VERSION_MAJOR(GetD3D9Caps().VertexShaderVersion) >= 3)
  {
    if (SupportsVSHLSL(mpD3D9Device,"vs_3_0")) {
      mvProfiles[eShaderUnit_Vertex].push_back(_H("hlsl_vs_3_0"));
    }
  }

  if (mCompTest.mnMaxPS >= 20 && D3DSHADER_VERSION_MAJOR(GetD3D9Caps().PixelShaderVersion) >= 2)
  {
    if (SupportsVSHLSL(mpD3D9Device,"ps_2_0")) {
      mvProfiles[eShaderUnit_Pixel].push_back(_H("hlsl_ps_2_0"));
    }

    if (mCompTest.mnMaxPS >= 21 &&
        GetD3D9Caps().PS20Caps.NumTemps >= 32 &&
        niFlagIs(GetD3D9Caps().PS20Caps.Caps,D3DPS20CAPS_NOTEXINSTRUCTIONLIMIT) &&
        SupportsPSHLSL(mpD3D9Device,"ps_2_b"))
    {
      mvProfiles[eShaderUnit_Pixel].push_back(_H("hlsl_ps_2_b"));
    }

    if (mCompTest.mnMaxPS >= 22 &&
        GetD3D9Caps().PS20Caps.NumTemps &&
        //GetD3D9Caps().PS20Caps.DynamicFlowControlDepth && // not supported by FX5xxx
        //GetD3D9Caps().PS20Caps.StaticFlowControlDepth &&      // not supported by FX5xxx
        niFlagIs(GetD3D9Caps().PS20Caps.Caps,D3DPS20CAPS_ARBITRARYSWIZZLE) &&
        niFlagIs(GetD3D9Caps().PS20Caps.Caps,D3DPS20CAPS_PREDICATION) &&
        niFlagIs(GetD3D9Caps().PS20Caps.Caps,D3DPS20CAPS_NODEPENDENTREADLIMIT))
    {
      if (SupportsPSHLSL(mpD3D9Device,"ps_2_a")) {
        mvProfiles[eShaderUnit_Pixel].push_back(_H("hlsl_ps_2_a"));
      }
    }
  }
  if (mCompTest.mnMaxPS >= 30 && D3DSHADER_VERSION_MAJOR(GetD3D9Caps().PixelShaderVersion) >= 3)
  {
    if (SupportsPSHLSL(mpD3D9Device,"ps_3_0")) {
      mvProfiles[eShaderUnit_Pixel].push_back(_H("hlsl_ps_3_0"));
    }
  }

  if (mvProfiles[eShaderUnit_Vertex].empty()) {
      niError("No Vertex Shader profile.");
      return eFalse;
  }
  if (mvProfiles[eShaderUnit_Pixel].empty()) {
      niError("No Pixel Shader profile.");
      return eFalse;
  }
  return eTrue;
}

///////////////////////////////////////////////
tU32 __stdcall cD3D9::GetNumShaderProfile(eShaderUnit aUnit) const {
  D3D9_CheckContext(0);
  if (aUnit >= eShaderUnit_Last) return 0;
  return (tU32)mvProfiles[aUnit].size();
}

///////////////////////////////////////////////
iHString* __stdcall cD3D9::GetShaderProfile(eShaderUnit aUnit, tU32 anIndex) const {
  D3D9_CheckContext(NULL);
  if (anIndex >= GetNumShaderProfile(aUnit)) return NULL;
  return mvProfiles[aUnit][anIndex];
}

///////////////////////////////////////////////
iShader* __stdcall cD3D9::CreateShader(iHString* ahspName, iFile* apFile) {
  if (!niIsOK(apFile)) {
    niError(_A("Invalid file."));
    return NULL;
  }

  if (apFile->ReadLE32() != kfccD3DShader) {
    niError(_A("The file is not a D3D shader."));
    return NULL;
  }

  if (apFile->ReadLE32() != niMakeVersion(1,0,0)) {
    niError(_A("Incompatible version."));
    return NULL;
  }

  apFile->BeginReadBits();
  tU8 nUnit = apFile->ReadBitsPackedU8();
  /* cString strName =*/ apFile->ReadBitsString();
  tHStringPtr hspProfileName = _H(apFile->ReadBitsString());
  tU32 nBufferSize = apFile->ReadBitsPackedU32();
  tBool bHasConstants = apFile->ReadBit();
  apFile->EndReadBits();

  astl::vector<tU8> vData;
  vData.resize(nBufferSize);
  apFile->ReadRaw(&vData[0],nBufferSize);

  if (!HasShaderProfile(this,nUnit,hspProfileName)) {
    niError(niFmt(_A("Profile '%s' is not available, can't load the program."),hspProfileName));
    return NULL;
  }

  iShader* pProg = NULL;
  HRESULT hr;
  if (nUnit == eShaderUnit_Vertex)
  {
    IDirect3DVertexShader9* pVS;
    hr = mpD3D9Device->CreateVertexShader((DWORD*)&vData[0], &pVS);
    if (FAILED(hr)) {
      D3D9ERROR();
      niError(_A("Can't create vertex shader."));
      return NULL;
    }
    pProg = niNew cD3D9ShaderVertex(
        mpGraphics,ahspName,mpGraphics->GetShaderDeviceResourceManager(),
        pVS,hspProfileName,NULL);
  }
  else if (nUnit == eShaderUnit_Pixel)
  {
    IDirect3DPixelShader9* pPS;
    hr = mpD3D9Device->CreatePixelShader((DWORD*)&vData[0], &pPS);
    if (FAILED(hr)) {
      D3D9ERROR();
      niError(_A("Can't create pixel shader:\n%s\n"));
      return NULL;
    }
    pProg = niNew cD3D9ShaderPixel(
        mpGraphics,ahspName,mpGraphics->GetShaderDeviceResourceManager(),
        pPS,hspProfileName,NULL);
  }

  if (!niIsOK(pProg)) {
    niSafeRelease(pProg);
    niError(_A("Can't create the shader instance."));
    return NULL;
  }

  if (bHasConstants) {
    if (!ni::GetLang()->SerializeObject(apFile,(iShaderConstants*)pProg->GetConstants(),eSerializeMode_ReadRaw,NULL)) {
      niSafeRelease(pProg);
      niError(_A("Can't load the shader constants."));
      return NULL;
    }
  }

  return pProg;
}

///////////////////////////////////////////////
iOcclusionQuery* __stdcall cD3D9::CreateOcclusionQuery() {
  D3D9_CheckContext(NULL);
  if (!niFlagIs(mnExtraCaps,CAPS_OCCLUSIONQUERIES)) return NULL;
  return niNew cD3D9OcclusionQuery(mpGraphics);
}

///////////////////////////////////////////////
inline void _DrawIndexed(IDirect3DDevice9* apDev,
                         D3DPRIMITIVETYPE pt,
                         iDrawOperation* apDrawOp,
                         iVertexArray* pVA,
                         cD3D9IndexArray* pD3D9IA,
                         tU32 doNumIndices)
{
  if (!pD3D9IA->mvBlocks.empty()) {
    tU32 doFirstIndex = apDrawOp->GetFirstIndex();
    niLoop(i,pD3D9IA->mvBlocks.size()) {
      const cD3D9IndexArray::sBlock& b = pD3D9IA->mvBlocks[i];
      if (b.firstIndex >= doFirstIndex+doNumIndices)
        continue;
      if (b.firstIndex+b.numInds < doFirstIndex)
        continue;

      const tU32 numIndices = ni::Min(b.numInds,doNumIndices);
      apDev->DrawIndexedPrimitive(
          pt,
          apDrawOp->GetBaseVertexIndex()+b.minVert,
          0,
          b.vertRange(),
          ni::Max(b.firstIndex,doFirstIndex),
          D3D9_GetPrimitiveCount(pt,numIndices));
      if (numIndices >= doNumIndices)
        break;
      doFirstIndex += numIndices;
      doNumIndices -= numIndices;
    }
  }
  else {
    apDev->DrawIndexedPrimitive(
        pt,
        apDrawOp->GetBaseVertexIndex(),
        0,
        pVA->GetNumVertices()-apDrawOp->GetBaseVertexIndex(),
        apDrawOp->GetFirstIndex(),
        D3D9_GetPrimitiveCount(pt,doNumIndices));
  }
}

inline tBool cD3D9::DoDrawOp(iDrawOperation* apDrawOp,
                             tBool abSWInst,
                             const sMatrixf& aViewMatrix,
                             const sMatrixf& aProjMatrix,
                             const sVec4f& aBaseColor) {
  niProfileBlock(renD3D9_DoDrawOp);

  // Geometry description buffers
  iVertexArray* pVA = apDrawOp->GetVertexArray();
  iIndexArray* pIA = apDrawOp->GetIndexArray();
  if (pVA) pVA = niStaticCast(iVertexArray*,pVA->Bind(mpGraphics));
  if (pIA) pIA = niStaticCast(iIndexArray*,pIA->Bind(mpGraphics));

  tFVF nFVF = pVA->GetFVF();
  /*tU32 nD3D9FVF =*/ mVertexDeclarations.UpdateVertexDeclaration(mpD3D9Device,nFVF,eFalse);
  D3DPRIMITIVETYPE pt = D3D9_GetPrimitivePRIMITIVETYPE((eGraphicsPrimitiveType)apDrawOp->GetPrimitiveType());

  // System memory drawing
  if (pVA->GetUsage() == eArrayUsage_SystemMemory) {
    niProfileBlock(renD3D9_DoDrawOp_SysMem);

    niCheckSilent(!pIA || pIA->GetUsage() == eArrayUsage_SystemMemory, eFalse);

    tU32 nStride = FVFGetStride(nFVF);
    tU32 nNumVerts = pVA->GetNumVertices()-apDrawOp->GetBaseVertexIndex();
    tPtr pVABase = (tPtr)pVA->Lock(apDrawOp->GetBaseVertexIndex(),
                                   nNumVerts,
                                   eLock_ReadOnly);
    if (pIA) {
      tU32 nNumIndices = apDrawOp->GetNumIndices();
      if (!nNumIndices) nNumIndices = pIA->GetNumIndices()-apDrawOp->GetFirstIndex();
      tPtr pIABase = (tPtr)pIA->Lock(apDrawOp->GetFirstIndex(),nNumIndices,eLock_ReadOnly);
      mpD3D9Device->DrawIndexedPrimitiveUP(
          pt,0,nNumVerts,
          D3D9_GetPrimitiveCount(pt,nNumIndices),
          pIABase,D3DFMT_INDEX32,
          pVABase,nStride);
      pIA->Unlock();
    }
    else {
      tU32 nNumIndices = apDrawOp->GetNumIndices();
      if (!nNumIndices) nNumIndices = pVA->GetNumVertices()-apDrawOp->GetFirstIndex();
      mpD3D9Device->DrawPrimitiveUP(
          pt,
          D3D9_GetPrimitiveCount(pt,nNumIndices),
          pVABase,
          nStride);
    }
    pVA->Unlock();
  }
  // Video memory drawing
  else {
#pragma niTodo("Provide a safe way to check the owner of the arrays. -- Add a OwnerHandle to the iVertex/Index/Texture/Material, etc... --- might do that in iUnknown - can be used by a garbage collector")

    niProfileBlock(renD3D9_DoDrawOp_VidMem);

    iD3D9VertexArray* pD3D9VA = niStaticCast(iD3D9VertexArray*,pVA);
    {
      tBool bShouldUpdate = eFalse;
      mCache.ShouldUpdate(&bShouldUpdate,eD3D9Cache_VertexArray,(tIntPtr)pD3D9VA);
      if (bShouldUpdate) {
        niProfileBlock(renD3D9_DoDrawOp_VidMem_SetStreamSource);
        mpD3D9Device->SetStreamSource(0,pD3D9VA->GetVertexBuffer(),0,FVFGetStride(nFVF));
      }
    }
    if (pIA) {
      niProfileBlock(renD3D9_DoDrawOp_VidMem_Indexed);
      cD3D9IndexArray* pD3D9IA = niStaticCast(cD3D9IndexArray*,pIA);
      {
        tBool bShouldUpdate = eFalse;
        mCache.ShouldUpdate(&bShouldUpdate,eD3D9Cache_IndexArray,(tIntPtr)pD3D9IA);
        if (bShouldUpdate) {
          niProfileBlock(renD3D9_DoDrawOp_VidMem_SetIndices);
          mpD3D9Device->SetIndices(pD3D9IA->GetIndexBuffer());
        }
      }

      tU32 nNumIndices = apDrawOp->GetNumIndices();
      if (!nNumIndices)
        nNumIndices = pIA->GetNumIndices()-apDrawOp->GetFirstIndex();

      _DrawIndexed(
          mpD3D9Device,pt,apDrawOp,
          pVA,pD3D9IA,nNumIndices);
    }
    else {
      niProfileBlock(renD3D9_DoDrawOp_VidMem_NotIndexed);
      {
        tBool bShouldUpdate = eFalse;
        mCache.ShouldUpdate(&bShouldUpdate,eD3D9Cache_IndexArray,0);
        if (bShouldUpdate) {
          mpD3D9Device->SetIndices(NULL);
        }
      }
      tU32 nNumIndices = apDrawOp->GetNumIndices();
      if (!nNumIndices)
        nNumIndices = pVA->GetNumVertices()-apDrawOp->GetFirstIndex();

      mpD3D9Device->DrawPrimitive(pt,
                                  apDrawOp->GetFirstIndex(),
                                  D3D9_GetPrimitiveCount(pt,nNumIndices));
    }
  }

  return eTrue;
}

///////////////////////////////////////////////
void __stdcall cD3D9::_ClearBuffers(sD3D9Context* apContext, tClearBuffersFlags clearBuffer, tU32 anColor, tF32 afDepth, tI32 anStencil) {
  D3D9_CheckContext(;);

  if (mptrDOCapture.IsOK()) {
    if (!mptrDOCapture->BeginCaptureDrawOp(
            apContext,NULL,
            Vec4<tI32>(clearBuffer,anColor,ftoul(afDepth),anStencil)))
      return;
  }

  if (!D3D9_ApplyContext(mCache,mpD3D9Device,GetD3D9Caps(),apContext)) {
    niError(_A("Can't apply context."));
    return;
  }

  tU32 Flags = 0;
  if (clearBuffer & eClearBuffersFlags_Color)
    Flags |= D3DCLEAR_TARGET;
  if (apContext->mptrDS.IsOK()) {
    if (clearBuffer & eClearBuffersFlags_Depth)
      Flags |= D3DCLEAR_ZBUFFER;
    niFlagOnIf(Flags,D3DCLEAR_STENCIL,
               (clearBuffer & eClearBuffersFlags_Stencil) &&
               apContext->mptrDS->GetPixelFormat()->GetNumABits());
  }
  if (Flags) {
    mpD3D9Device->Clear(0,NULL,Flags,anColor,afDepth,anStencil);
  }

  if (mptrDOCapture.IsOK()) {
    mptrDOCapture->EndCaptureDrawOp(
        apContext,NULL,
        Vec4<tI32>(clearBuffer,anColor,ftoul(afDepth),anStencil));
  }
}

///////////////////////////////////////////////
tBool __stdcall cD3D9::_DrawOperation(sD3D9Context* apContext, iDrawOperation* apDrawOp) {
  D3D9_CheckContext(eFalse);
  niCheck(niIsOK(apDrawOp),eFalse);
  niCheck(niIsOK(apDrawOp->GetVertexArray()),eFalse);
  niProf(D3D9_DrawOperation);

  if (mptrDOCapture.IsOK()) {
    if (!mptrDOCapture->BeginCaptureDrawOp(apContext,apDrawOp,sVec4i::Zero()))
      return eTrue;
  }

  _BeginScene();

  if (!D3D9_ApplyContext(mCache,mpD3D9Device,GetD3D9Caps(),apContext)) {
    niError(_A("Can't apply context."));
    return eFalse;
  }

  sMatrixf viewMatrix, projMatrix;
  tBool bSWInst = eFalse;
  sVec4f texFactor;

  {
    niProf(D3D9_DrawOperation_ApplyStates);

    Ptr<iMaterial> mat = apDrawOp->GetMaterial();
    niAssert(mat.IsOK() && "No valid material in the draw operation.");
    niCheck(mat.IsOK(),eFalse);

    {
      const sMaterialDesc* pMatDesc = (const sMaterialDesc*)mat->GetDescStructPtr();
      const tU32 matFlags = pMatDesc->mFlags;
      const tFVF fvf = apDrawOp->GetVertexArray()->GetFVF();
      const D3DCAPS9& caps = this->GetD3D9Caps();

      {
        tIntPtr hDS = pMatDesc->mhDS;
        if (!hDS) {
          hDS = eCompiledStates_DS_NoDepthTest;
        }
        tBool bShouldUpdate = eFalse;
        mCache.ShouldUpdate(&bShouldUpdate,eD3D9Cache_DepthStencil,hDS);
        if (bShouldUpdate) {
          iDepthStencilStates* pDSStates = mpGraphics->GetCompiledDepthStencilStates(hDS);
          niCheck(pDSStates,eFalse);
          D3D9_ApplyDepthStencilStates(
              this->mCache,mpD3D9Device,caps,
              (const sDepthStencilStatesDesc*)pDSStates->GetDescStructPtr());
        }
      }

      {
        tIntPtr hRS = pMatDesc->mhRS;
        if (!hRS) {
          hRS = eCompiledStates_RS_NoCullingFilledScissor;
        }
        tBool bShouldUpdate = eFalse;
        mCache.ShouldUpdate(&bShouldUpdate,eD3D9Cache_Rasterizer,hRS);
        mCache.ShouldUpdate(&bShouldUpdate,eD3D9Cache_RasterizerDoubleSided,
                            niFlagIs(matFlags,eMaterialFlags_DoubleSided));
        mCache.ShouldUpdate(&bShouldUpdate,eD3D9Cache_RasterizerDepthOnly,
                            niFlagIs(matFlags,eMaterialFlags_DepthOnly));
        if (bShouldUpdate) {
          iRasterizerStates* pRSStates = mpGraphics->GetCompiledRasterizerStates(hRS);
          niCheck(pRSStates,eFalse);
          D3D9_ApplyRasterizerStates(
              this->mCache,mpD3D9Device,caps,
              (const sRasterizerStatesDesc*)pRSStates->GetDescStructPtr(),
              niFlagIs(matFlags,eMaterialFlags_DoubleSided),
              niFlagIs(matFlags,eMaterialFlags_DepthOnly));
        }
      }

      iShader* pVS = pMatDesc->mShaders[eShaderUnit_Vertex];
      if (!pVS) {
        pVS = mFixedShaders.GetVertexShader(fvf,*pMatDesc);
        if (!pVS) {
          niError("Can't get fixed vertex shader.");
          return eFalse;
        }
        FixedShaders_UpdateConstants(apContext, (iShaderConstants*)pVS->GetConstants(), apDrawOp);
      }

      iShader* pPS = pMatDesc->mShaders[eShaderUnit_Pixel];
      if (!pPS) {
        pPS = mFixedShaders.GetPixelShader(*pMatDesc);
        if (!pPS) {
          niError("Can't get fixed pixel shader.");
          return eFalse;
        }
        FixedShaders_UpdateConstants(apContext, (iShaderConstants*)pPS->GetConstants(), apDrawOp);
      }

      D3D9_ApplyFixedStates(
          this->mCache,mpD3D9Device,caps,
          pVS != NULL,
          pPS != NULL,
          pMatDesc,
          (const sFixedStatesDesc*)apContext->mptrFS->GetDescStructPtr(),
          viewMatrix,
          projMatrix,
          apDrawOp->GetMatrix());

      // Vertex shader
      D3D9_ApplyVertexShader(
        apContext,
        this->mCache,mpD3D9Device,GetGraphics(),
        pVS,
        apDrawOp,
        pMatDesc);

      // Pixel shader
      D3D9_ApplyPixelShader(
        apContext,
        this->mCache,mpD3D9Device,mpGraphics,
        pPS,
        apDrawOp,
        pMatDesc);

      {
        const tU32 maxTU = GetCaps(eGraphicsCaps_NumTextureUnits);
        D3D9_ApplyMaterial(
            mpGraphics,
            this->mCache,mpD3D9Device,caps,this->mnExtraCaps,
            pMatDesc,
            pVS,
            pPS,
            texFactor,
            maxTU,
            fvf);
      }
    }
  }

  tBool r = this->DoDrawOp(apDrawOp,bSWInst,viewMatrix,projMatrix,texFactor);
  if (mptrDOCapture.IsOK()) {
    mptrDOCapture->EndCaptureDrawOp(apContext,apDrawOp,sVec4i::Zero());
  }
  return r;
}

///////////////////////////////////////////////
IDirect3DSurface9* __stdcall cD3D9::CreateD3D9Surface(tU32 anW, tU32 anH, D3DFORMAT aFmt, D3DPOOL aPool)
{
  IDirect3DSurface9* s;
  HRESULT hr = mpD3D9Device->CreateOffscreenPlainSurface(anW,anH,aFmt,aPool,&s,NULL);
  if (FAILED(hr)) {
    D3D9ERROR();
    niError(_A("Can't create the d3d9 surface."));
    return NULL;
  }

  return s;
}

///////////////////////////////////////////////
iBitmap2D* __stdcall cD3D9::LockD3D9Surface(IDirect3DSurface9* apSurf, DWORD aLockFlags)
{
  niCheck(apSurf != NULL,NULL);

  D3DSURFACE_DESC desc;
  HRESULT hr = apSurf->GetDesc(&desc);
  if (FAILED(hr)) {
    D3D9ERROR();
    niError(_A("Can't get desc of the d3d9 surface."));
    return NULL;
  }

  D3DLOCKED_RECT lockedRect;
  hr = apSurf->LockRect(&lockedRect,NULL,aLockFlags);
  if (FAILED(hr)) {
    D3D9ERROR();
    niError(_A("Can't lock the d3d9 surface."));
    return NULL;
  }

  Ptr<iBitmap2D> bmp = mpGraphics->CreateBitmap2DMemory(desc.Width,desc.Height,
                                                        GetD3DFormatMap()->GetPixelFormat(desc.Format,NULL,NULL),
                                                        lockedRect.Pitch,(tPtr)lockedRect.pBits,eFalse);
  if (!bmp.IsOK()) {
    apSurf->UnlockRect();
    niError(_A("Can't create the lock bitmap."));
    return NULL;
  }

  return bmp.GetRawAndSetNull();
}

///////////////////////////////////////////////
IDirect3DSurface9* __stdcall cD3D9::CreateD3D9SurfaceFromBitmap(iBitmap2D* apBmp, D3DFORMAT aFmt, D3DPOOL aPool)
{
  niCheck(niIsOK(apBmp),NULL);

  if (aFmt == D3DFMT_UNKNOWN) {
    aFmt = GetD3DFormatMap()->GetD3DFormat(apBmp->GetPixelFormat()->GetFormat());
  }
  niCheck(aFmt != D3DFMT_UNKNOWN,NULL);

  IDirect3DSurface9* s = CreateD3D9Surface(apBmp->GetWidth(),apBmp->GetHeight(),aFmt,aPool);
  if (!s) {
    niError(_A("Can't create the destination surface."));
    return NULL;
  }

  Ptr<iBitmap2D> dest = LockD3D9Surface(s,0);
  if (!dest.IsOK()) {
    niSafeRelease(s);
    niError(_A("Can't lock the d3d9 surface."));
    return NULL;
  }

  dest->Blit(apBmp,0,0,0,0,apBmp->GetWidth(),apBmp->GetHeight());

  s->UnlockRect();
  return s;
}

///////////////////////////////////////////////
IDirect3DSurface9* __stdcall cD3D9::CreateD3D9SurfaceFromBitmapRect(iBitmap2D* apBmp, D3DFORMAT aFmt, D3DPOOL aPool, const sRecti& aRect)
{
  niCheck(niIsOK(apBmp),NULL);

  if (aFmt == D3DFMT_UNKNOWN) {
    aFmt = GetD3DFormatMap()->GetD3DFormat(apBmp->GetPixelFormat()->GetFormat());
  }
  niCheck(aFmt != D3DFMT_UNKNOWN,NULL);

  IDirect3DSurface9* s = CreateD3D9Surface(aRect.GetWidth(),aRect.GetHeight(),aFmt,aPool);
  if (!s) {
    niError(_A("Can't create the destination surface."));
    return NULL;
  }

  Ptr<iBitmap2D> dest = LockD3D9Surface(s,0);
  if (!dest.IsOK()) {
    niSafeRelease(s);
    niError(_A("Can't lock the d3d9 surface."));
    return NULL;
  }

  dest->Blit(apBmp,aRect.Left(),aRect.Top(),0,0,aRect.GetWidth(),aRect.GetHeight());

  s->UnlockRect();
  return s;
}

//--------------------------------------------------------------------------------------------
//
//  D3D9 Graphics Context
//
//--------------------------------------------------------------------------------------------
class cD3D9ContextWindow : public sD3D9Context
{
 public:
  cD3D9*                mpDrv;
  tU32                  mnWidth, mnHeight;
  tBool                 mbFullScreen;
  tU32                  mnSwapInterval;
  tTextureFlags         mnBBFlags;
  Ptr<iOSWindow>        mptrWindow;
  IDirect3DDevice9*     mpD3D9Device;
  IDirect3DSwapChain9*  mpSwapChain;
  D3DPRESENT_PARAMETERS mPP;
  Ptr<cD3D9Texture>     mptrMainRT;
  Ptr<cD3D9Texture>     mptrMainDS;
  cString               mstrDSFormat;
  tBool                 mbIsPrimary;
  sRecti                mLastParentRect;

  cD3D9ContextWindow(cD3D9* apParent)
      : sD3D9Context(apParent->GetGraphics())
      , mpDrv(apParent)
      , mnWidth(0), mnHeight(0)
      , mbFullScreen(eFalse)
      , mnSwapInterval(0)
      , mnBBFlags(0)
      , mpSwapChain(NULL)
      , mbIsPrimary(eFalse)
      , mpD3D9Device(NULL)
  {
    mpDrv->mvContexts.push_back(this);
    mLastParentRect = sRecti::Null();
  }

  ~cD3D9ContextWindow() {
    Invalidate();
  }

  virtual void __stdcall Invalidate() {
    if (!mpDrv) return;
    astl::find_erase(mpDrv->mvContexts,this);
    mptrWindow = NULL;
    niSafeRelease(mpSwapChain);
    niSafeRelease(mpD3D9Device);
    mptrMainRT = NULL;
    mptrMainDS = NULL;
    mpDrv = NULL;
  }

  virtual tBool __stdcall IsOK() const {
    return mpSwapChain != NULL;
  }

  virtual iGraphics* __stdcall GetGraphics() const {
    if (!mpDrv) return NULL;
    return mpDrv->GetGraphics();
  }
  virtual iGraphicsDriver* __stdcall GetDriver() const {
    return mpDrv;
  }
  virtual tU32 __stdcall GetWidth() const {
    return mnWidth;
  }
  virtual tU32 __stdcall GetHeight() const {
    return mnHeight;
  }
  virtual iTexture* __stdcall GetMainRenderTarget() const {
    return mptrMainRT;
  }
  virtual iTexture* __stdcall GetMainDepthStencil() const {
    return mptrMainDS;
  }
  virtual iOSWindow* __stdcall GetWindow() const {
    return mptrWindow;
  }

  ///////////////////////////////////////////////
  tBool __stdcall _InitializeContext(
      iOSWindow* apWindow,
      tU32 anWidth, tU32 anHeight,
      const achar* aaszBBPxf,
      const achar* aaszDSPxf,
      tU32 anSwapInterval,
      tTextureFlags aBackBufferFlags,
      tBool abFullScreen,
      tBool abPrimaryContext,
      tU32 anBackBufferCount)
  {
    if (!mpDrv) return eFalse;

    if (mptrWindow.IsOK()) {
      niError(_A("Context already initialized."));
      return eFalse;
    }

    if (!abPrimaryContext && abFullScreen) {
      niError(_A("Only the primary context can be created as full screen."));
      return eFalse;
    }

    if (!mpDrv->_InitCaps()) {
      niError(_A("Can't get Direct3D9 caps."));
      return eFalse;
    }

    IDirect3D9* pD3D9 = mpDrv->mpD3D9;
    if (!pD3D9) {
      niError(_A("No valid D3D9 object."));
      return eFalse;
    }

    HWND hWnd = NULL;

    if (!apWindow && abPrimaryContext) {
      mptrWindow = ni::GetLang()->CreateWindow(
          NULL,_A("null"),sRecti(0,0,1,1),
          eFalse,eOSWindowStyleFlags_Toolbox);
    }
    else if (niIsOK(apWindow)) {
      mptrWindow = apWindow;
    }
    if (!mptrWindow.IsOK()) {
      niError(_A("No valid global window instance."));
      return eFalse;
    }
    //         mpDrv->_RemoveWindowSink(mptrWindow);
    hWnd = (HWND)mptrWindow->GetHandle();
    niAssert(::IsWindow(hWnd));

    mstrDSFormat = aaszDSPxf;

    // Setup the creation parameters
    ZeroMemory(&mPP, sizeof(mPP));
    mPP.Windowed = abFullScreen?FALSE:TRUE;
    mPP.SwapEffect = D3D9_SWAP_EFFECT;
    mPP.BackBufferWidth = anWidth;
    mPP.BackBufferHeight = anHeight;
    mPP.BackBufferCount = anBackBufferCount;
    mPP.MultiSampleQuality = 0;
    mPP.EnableAutoDepthStencil = FALSE;
    mPP.hDeviceWindow = hWnd;
    mPP.Flags = D3D9_PRESENT_FLAGS;
    if (mptrWindow.IsOK() && (!mPP.hDeviceWindow || !::IsWindow(mPP.hDeviceWindow))) {
      niError(niFmt(_A("No valid device window application handle '%x'."),mPP.hDeviceWindow));
      return eFalse;
    }

    // multi sample types
    astl::vector<D3DMULTISAMPLE_TYPE> vMSTypes;
    vMSTypes.push_back(D3D9_GetD3DMULTISAMPLE(aBackBufferFlags));
    if (vMSTypes[0] != D3DMULTISAMPLE_NONE) {
      vMSTypes.push_back(D3DMULTISAMPLE_NONE);
    }

    iGraphics* pGraphics = mpDrv->mpGraphics;

    // back buffer formats
    astl::vector<D3DFORMAT> vBBFormats;
    if (!abFullScreen) {
      vBBFormats.push_back(D3DFMT_UNKNOWN);
    }
    else
    {
      Ptr<iPixelFormat> ptrBBPxf = pGraphics->CreatePixelFormat(aaszBBPxf);
      if (!ptrBBPxf.IsOK()) {
        niError(niFmt(_A("Can't create the back buffer's pixel format '%s'."),aaszBBPxf));
        return eFalse;
      }
      vBBFormats.push_back(GetD3DFormatMap()->GetD3DFormat(aaszBBPxf));
      if (ptrBBPxf->GetNumABits()) {
        vBBFormats.push_back(D3DFMT_A8R8G8B8);
        vBBFormats.push_back(D3DFMT_A8B8G8R8);
        vBBFormats.push_back(D3DFMT_A2R10G10B10);
        vBBFormats.push_back(D3DFMT_A2B10G10R10);
        vBBFormats.push_back(D3DFMT_X8R8G8B8);
        vBBFormats.push_back(D3DFMT_X8B8G8R8);
        vBBFormats.push_back(D3DFMT_R8G8B8);
        vBBFormats.push_back(D3DFMT_R5G6B5);
        vBBFormats.push_back(D3DFMT_X1R5G5B5);
      }
      else {
        vBBFormats.push_back(D3DFMT_A2R10G10B10);
        vBBFormats.push_back(D3DFMT_A2B10G10R10);
        vBBFormats.push_back(D3DFMT_A8R8G8B8);
        vBBFormats.push_back(D3DFMT_A8B8G8R8);
        vBBFormats.push_back(D3DFMT_X8R8G8B8);
        vBBFormats.push_back(D3DFMT_X8B8G8R8);
        vBBFormats.push_back(D3DFMT_R8G8B8);
        vBBFormats.push_back(D3DFMT_R5G6B5);
        vBBFormats.push_back(D3DFMT_X1R5G5B5);
      }
    }

    if (anSwapInterval) {
      if (anSwapInterval == 4)
        mPP.PresentationInterval = D3DPRESENT_INTERVAL_FOUR;
      else if (anSwapInterval == 3)
        mPP.PresentationInterval = D3DPRESENT_INTERVAL_THREE;
      else if (anSwapInterval == 2)
        mPP.PresentationInterval = D3DPRESENT_INTERVAL_TWO;
      else
        mPP.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    }
    else {
      mPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    }

    DWORD dwBehaviorFlags = _GetD3D9BehaviorFlags(mpDrv->GetD3D9Caps());

    // Get the capabilities of the main adapter
    HRESULT hr = -1;
    if (abPrimaryContext) {
      mbIsPrimary = eTrue;

      tU32 nAdapter = mpDrv->GetD3D9Caps().AdapterOrdinal;
      D3DDEVTYPE nDevType = mpDrv->GetD3D9Caps().DeviceType;

      // Creates the first possible device
      for (tU32 m = 0; m < vMSTypes.size(); ++m) {
        mPP.MultiSampleType = vMSTypes[m];
        for (tU32 i = 0; i < vBBFormats.size(); ++i) {
          mPP.BackBufferFormat = vBBFormats[i];
          hr = mpDrv->mpD3D9->CreateDevice(nAdapter,nDevType,
                                           hWnd, dwBehaviorFlags,
                                           &mPP, &mpD3D9Device);
          if (SUCCEEDED(hr)) {
            break;
          }
        }
        if (SUCCEEDED(hr))
          break;
      }
      if (FAILED(hr)) {
        D3D9ERROR();
        niError(_A("Can't create d3d9 device."));
        return eFalse;
      }
      else {
        niLog(Info,niFmt(
          _A("Created Regular Direct3D9Device (%p)."),
          mpD3D9Device));
      }
      mpDrv->mpD3D9Device = mpD3D9Device;

      hr = mpD3D9Device->GetSwapChain(0,&mpSwapChain);
      if (FAILED(hr)) {
        D3D9ERROR();
        niError(_A("Can't get the main swap chain."));
        return eFalse;
      }
    }
    else {
      mbIsPrimary = eFalse;

      mpD3D9Device = mpDrv->mpD3D9Device;
      if (!mpD3D9Device) {
        niError(_A("No D3D9 Device available from the driver."));
        return eFalse;
      }

      // tU32 nAdapter = mpDrv->GetD3D9Caps().AdapterOrdinal;
      // D3DDEVTYPE nDevType = mpDrv->GetD3D9Caps().DeviceType;

      // Creates the first possible device
      for (tU32 m = 0; m < vMSTypes.size(); ++m) {
        mPP.MultiSampleType = vMSTypes[m];
        for (tU32 i = 0; i < vBBFormats.size(); ++i) {
          mPP.BackBufferFormat = vBBFormats[i];
          hr = mpD3D9Device->CreateAdditionalSwapChain(&mPP, &mpSwapChain);
          if (SUCCEEDED(hr)) {
            break;
          }
        }
        if (SUCCEEDED(hr))
          break;
      }
      if (FAILED(hr)) {
        D3D9ERROR();
        niError(_A("Can't create d3d9 device."));
        return eFalse;
      }
    }

    return FinalizeInitialize(eFalse);
  }

  ///////////////////////////////////////////////
  tBool __stdcall FinalizeInitialize(tBool abReset) {
    if (!mpDrv)
      return eFalse;

    niCheck(mpD3D9Device != NULL, eFalse);

    HRESULT hr;
    iGraphics* pGraphics = mpDrv->mpGraphics;

    if (abReset) {
      if (mpDrv->mbBeganRendering) {
        mpD3D9Device->EndScene();
      }

      niSafeRelease(mpSwapChain);
      if (mbIsPrimary) {
        hr = mpD3D9Device->GetSwapChain(0,&mpSwapChain);
        if (FAILED(hr)) {
          D3D9ERROR();
          niError(_A("Can't restore the main swap chain."));
          return eFalse;
        }
      }
      else {
        hr = mpD3D9Device->CreateAdditionalSwapChain(&mPP, &mpSwapChain);
        if (FAILED(hr)) {
          D3D9ERROR();
          niError(_A("Can't restore additional swap chain."));
          return eFalse;
        }
      }
    }

    niCheck(mpSwapChain != NULL,eFalse);

    hr = mpSwapChain->GetPresentParameters(&mPP);
    if (FAILED(hr)) {
      D3D9ERROR();
      niError(_A("Can't get the swap chain's present parameters."));
      return eFalse;
    }

    mpDrv->InitStateBlocks();

    HWND hWnd = mPP.hDeviceWindow;
    mnWidth = mPP.BackBufferWidth;
    mnHeight = mPP.BackBufferHeight;
    if (mnWidth == 0 || mnHeight == 0) {
      sRecti rect;
      GetClientRect(hWnd,(LPRECT)&rect);
      mnWidth = rect.GetWidth()>=1?rect.GetWidth():1;
      mnHeight = rect.GetHeight()>=1?rect.GetHeight():1;
    }

    switch (mPP.PresentationInterval) {
      case D3DPRESENT_INTERVAL_FOUR:  mnSwapInterval = 4; break;
      case D3DPRESENT_INTERVAL_THREE: mnSwapInterval = 3; break;
      case D3DPRESENT_INTERVAL_TWO:   mnSwapInterval = 2; break;
      case D3DPRESENT_INTERVAL_ONE:   mnSwapInterval = 1; break;
      default:                        mnSwapInterval = 0; break;
    }
    mnBBFlags |= D3D9_GetD3DMULTISAMPLETYPETextureFlags(mPP.MultiSampleType);
    mbFullScreen = mPP.Windowed?eFalse:eTrue;

    {
      // Get the main render target (back buffer)
      {
        IDirect3DSurface9* pBB;
        hr = mpSwapChain->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO,&pBB);
        if (FAILED(hr)) {
          D3D9ERROR();
          niError(_A("Can't retrieve the main back buffer surface."));
          return eFalse;
        }

        if (!mptrMainRT.IsOK()) {
          mptrMainRT = niNew cD3D9Texture(pGraphics,
                                          _H(niFmt(_A("__MainRT_%p"),this))
                                         );
          mptrMainRT->mnFlags |= eTextureFlags_RenderTarget;
        }

        if (!mptrMainRT->InitializeSurface(NULL,pBB,0,0)) {
          pBB->Release();
          niError(_A("Can't initialize main back buffer surface texture."));
          return eFalse;
        }

        mptrRT[0] = mptrMainRT;

        pBB->Release(); // release ref, owned by the texture
      }

      // Get the main depth stencil
      if (!mstrDSFormat.empty()) {
        if (!mptrMainDS.IsOK()) {
          mptrMainDS = niNew cD3D9Texture(pGraphics,
                                          _H(niFmt(_A("__MainDS_%p"),this))
                                         );
          mptrMainDS->mnFlags |= eTextureFlags_DepthStencil;
        }

        astl::vector<D3DFORMAT> vDSFormats;
        {
          Ptr<iPixelFormat> ptrDSPxf = pGraphics->CreatePixelFormat(mstrDSFormat.Chars());
          if (!ptrDSPxf.IsOK()) {
            niError(niFmt(_A("Can't create the depth buffer's pixel format '%s'."),
                          mstrDSFormat.Chars()));
            return eFalse;
          }
          vDSFormats.push_back(GetD3DFormatMap()->GetD3DFormat(mstrDSFormat.Chars()));
          if (ptrDSPxf->GetNumABits()) {
            vDSFormats.push_back(D3DFMT_D24S8);
            vDSFormats.push_back(D3DFMT_D24X4S4);
            vDSFormats.push_back(D3DFMT_D32);
            vDSFormats.push_back(D3DFMT_D24X8);
            vDSFormats.push_back(D3DFMT_D16);
            vDSFormats.push_back(D3DFMT_D15S1);
          }
          else {
            vDSFormats.push_back(D3DFMT_D24S8);
            vDSFormats.push_back(D3DFMT_D32);
            vDSFormats.push_back(D3DFMT_D24X8);
            vDSFormats.push_back(D3DFMT_D24X4S4);
            vDSFormats.push_back(D3DFMT_D16);
            vDSFormats.push_back(D3DFMT_D15S1);
          }
        }

        D3DFORMAT dsFormat = D3DFMT_UNKNOWN;
        tU32 nAdapter = mpDrv->GetD3D9Caps().AdapterOrdinal;
        D3DDEVTYPE nDevType = mpDrv->GetD3D9Caps().DeviceType;
        const D3DFORMAT adapterFormat = mpDrv->mModeSelector.GetDefaultAdapter()->mDispMode.Format;
        niLoop(i,vDSFormats.size()) {
          if (D3D9_IsDepthFormatOk(mpDrv->mpD3D9,
                                   nAdapter,nDevType,vDSFormats[i],
                                   adapterFormat,mPP.BackBufferFormat))
          {
            dsFormat = vDSFormats[i];
            break;
          }
        }
        if (dsFormat == D3DFMT_UNKNOWN) {
          niError(niFmt(_A("Can't find a compatible depth stencil format '%s'."),
                        mstrDSFormat.Chars()));
          return eFalse;
        }

        IDirect3DSurface9* pDS;
        hr = mpD3D9Device->CreateDepthStencilSurface(
            mPP.BackBufferWidth,mPP.BackBufferHeight,dsFormat,
            mPP.MultiSampleType,mPP.MultiSampleQuality,
            TRUE,&pDS,NULL);
        if (FAILED(hr)) {
          D3D9ERROR();
          niError(_A("Can't create the main depth stencil surface."));
          return eFalse;
        }

        if (!mptrMainDS->InitializeSurface(NULL,pDS,0,0)) {
          pDS->Release();
          niError(_A("Can't initialize main depth stencil surface texture."));
          return eFalse;
        }
        mptrDS = mptrMainDS;

        pDS->Release(); // release ref, owned by the texture
      }
      else {
        mptrDS = NULL;
      }
    }

    // Set the viewport and scissor recangle
    SetViewport(sRecti(0,0,mnWidth,mnHeight));
    SetScissorRect(sRecti(0,0,mnWidth,mnHeight));

    // Enabled HW instancing on ATI hardware
    if (niFlagIs(mpDrv->mnExtraCaps,CAPS_HWINST) &&
        D3DSHADER_VERSION_MAJOR(mpDrv->GetD3D9Caps().VertexShaderVersion) < 3)
    {
#pragma niNote("Enable HW instancing on ATI hardware")
      mpD3D9Device->SetRenderState(D3DRS_POINTSIZE, MAKEFOURCC('I','N','S','T'));
    }

    mpDrv->mVertexDeclarations.ClearVertexDeclarations();
    mpDrv->ResetAllCaches();

    niLog(Info,niFmt(_A("D3D9 Context %s %dx%d (BB: %s) (DS: %s) (FS: %d) (VS: %d) (Flags: %s)"),
                     abReset?_A("reset"):_A("initialized"),
                     this->GetWidth(),
                     this->GetHeight(),
                     mptrMainRT.IsOK()?mptrMainRT->GetPixelFormat()->GetFormat():_A("NA"),
                     mptrMainDS.IsOK()?mptrMainDS->GetPixelFormat()->GetFormat():_A("NA"),
                     this->mbFullScreen,
                     this->mnSwapInterval,
                     niFlagsToChars(eTextureFlags,this->mnBBFlags)));

    mLastParentRect = sRecti::Null();
    return eTrue;
  }

  ///////////////////////////////////////////////
  void __stdcall OnDeviceLost() {
    if (mptrMainRT.IsOK()) {
      mptrMainRT->Cleanup();
    }
    if (mptrMainDS.IsOK()) {
      mptrMainDS->Cleanup();
    }
    niSafeRelease(mpSwapChain);
  }

  /////////////////////////////////////////////
  tBool __stdcall _DoResizeContext(iOSWindow* apWindow) {
    niCheck(niIsOK(apWindow),eFalse);
    niProfileBlock(_DoResizeContext);

    mptrWindow = apWindow;

    HWND hWnd = (HWND)mptrWindow->GetHandle();
    RECT rc;
    GetClientRect(hWnd,&rc);
    tU32 wndWidth = rc.right-rc.left;
    tU32 wndHeight = rc.bottom-rc.top;
    mPP.hDeviceWindow = hWnd;
    if (wndWidth > 64 && wndHeight > 64 &&
        (wndWidth != this->GetWidth() || wndHeight != this->GetHeight()))
    {
      mPP.BackBufferWidth = wndWidth;
      mPP.BackBufferHeight = wndHeight;
      if (mbIsPrimary) {
        mpDrv->_ResetDevice(eTrue);
        mpDrv->ResetAllCaches();
      }
      else {
        this->FinalizeInitialize(eTrue);
      }
    }
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall Display(tGraphicsDisplayFlags aFlags, const sRecti& aRect) {
    niCheck(mpDrv!=NULL,eFalse);
    niCheck(mptrWindow.IsOK(),eFalse);
    D3D9_CheckContext(eFalse);

    mpDrv->_TryEndScene();

    HWND hWnd = (HWND)mptrWindow->GetHandle();

    niProfileBlock(D3D9_Display);
    {
      if (!niFlagIs(aFlags,eGraphicsDisplayFlags_Skip)) {
        HRESULT hr;
        if (niFlagIs(aFlags,eGraphicsDisplayFlags_Transparent) ||
            niFlagIs(aFlags,eGraphicsDisplayFlags_Translucent)) {
          DWORD exStyle = GetWindowLong(hWnd,GWL_EXSTYLE);
          if (!(exStyle & WS_EX_LAYERED)) {
            MakeWindowTransparent(hWnd,0,255);
          }

          {
            IDirect3DSurface9*  pSrcSurface = mptrMainRT->GetSurface();
            IDirect3DSurface9*  pTempSurface;
            IDirect3DSurface9*  pDestSurface;
            D3DSURFACE_DESC  pDesc;
            pSrcSurface->GetDesc(&pDesc);

            HDC hDestDC;

            //Create temp surface;
            if (FAILED(mpD3D9Device->CreateRenderTarget(pDesc.Width,pDesc.Height,D3DFMT_X8R8G8B8,D3DMULTISAMPLE_NONE,0,FALSE,&pTempSurface,NULL)))
              return eFalse;
            if(FAILED(mpD3D9Device->CreateOffscreenPlainSurface(pDesc.Width,pDesc.Height,D3DFMT_X8R8G8B8,D3DPOOL_SYSTEMMEM,&pDestSurface,NULL)))
              return eFalse;
            if(FAILED(mpD3D9Device->StretchRect(pSrcSurface,NULL,pTempSurface,NULL,D3DTEXF_NONE )))
              return eFalse;
            if(FAILED(mpD3D9Device->GetRenderTargetData(pTempSurface,pDestSurface)))
              return eFalse;

            pDestSurface->GetDC(&hDestDC);

            //                     pDC->BitBlt(0,0,800,600,&cdc,0,0,SRCCOPY);
            HDC hWndDC = ::GetDC(hWnd);

            ::BitBlt(hWndDC,0,0,GetWidth(),GetHeight(),
                     hDestDC,0,0,
                     SRCCOPY);
            //                         ::TransparentBlt(hWndDC,0,0,GetWidth(),GetHeight(),
            //                                          hDestDC,0,0,GetWidth(),GetHeight(),
            //                                          0);

            ::ReleaseDC(hWnd,hWndDC);

            pDestSurface->ReleaseDC(hDestDC);
            pDestSurface->Release();
            pTempSurface->Release();
          }

          hr = mpD3D9Device->TestCooperativeLevel();
        }
        else {
          RECT clientRect;
          ::GetClientRect(hWnd,&clientRect);
          hr = mpSwapChain->Present(&clientRect,&clientRect,hWnd,NULL,0);
        }
        if (hr == D3DERR_DEVICELOST) {
          mpDrv->_ResetDevice(eFalse);
        }
      }
    }

    // Resize context if the window size changed
    const sVec2i newSize = mptrWindow->GetClientSize();
    if ((newSize.x != (tI32)this->GetWidth()) || (newSize.y != (tI32)this->GetHeight())) {
      _DoResizeContext(mptrWindow);
    }
    return eTrue;
  }

  ///////////////////////////////////////////////
  void __stdcall ClearBuffers(tClearBuffersFlags clearBuffer, tU32 anColor, tF32 afDepth, tI32 anStencil)
  {
    if (!mpDrv) return;
    return mpDrv->_ClearBuffers(this,clearBuffer,anColor,afDepth,anStencil);
  }
  tBool __stdcall DrawOperation(iDrawOperation* apDrawOp) {
    if (!mpDrv) return eFalse;
    return mpDrv->_DrawOperation(this,apDrawOp);
  }

  ///////////////////////////////////////////////
  iBitmap2D* __stdcall CaptureFrontBuffer() const {
    if (!mpDrv) return NULL;
    Ptr<iBitmap2D> ptrBmp = CaptureWindowToBitmap(mpDrv->mpGraphics, (HWND)mptrWindow->GetHandle());
    // mpDrv->mpGraphics->SaveBitmap("d:/_screencap_dx9.png", ptrBmp, 0);
    return ptrBmp.GetRawAndSetNull();
  }
};

///////////////////////////////////////////////
iGraphicsContext* __stdcall cD3D9::CreateContextForWindow(
    iOSWindow* apWindow,
    const achar* aaszBBPxf,
    const achar* aaszDSPxf,
    tU32 anSwapInterval,
    tTextureFlags aBackBufferFlags)
{
  niCheckIsOK(apWindow, NULL);

  // windowed, we create a primary 1x1 dummy context if not yet created
  if (!mptrPrimaryContext.IsOK()) {
    Ptr<cD3D9ContextWindow> ctx = niNew cD3D9ContextWindow(this);
    if (!ctx->_InitializeContext(
          NULL,
          1,1,aaszBBPxf,NULL,
          eFalse,0,0,eTrue,0))
    {
      niError(_A("Can't create primary context."));
      return NULL;
    }
    mptrPrimaryContext = ctx;
    if (!_DetectShaderProfiles()) {
      niError("Can't detect shader profiles.");
      return NULL;
    }
  }

  Ptr<cD3D9ContextWindow> ctx = niNew cD3D9ContextWindow(this);
  if (!ctx->_InitializeContext(
        apWindow,
        apWindow->GetSize().x,apWindow->GetSize().y,
        aaszBBPxf,aaszDSPxf,
        eFalse,anSwapInterval,aBackBufferFlags,
        eFalse,1))
  {
    niError(_A("Can't create additional context."));
    return NULL;
  }

  if (!mFixedShaders.Initialize(mpGraphics)) {
    niError(_A("Can't initialize fixed shaders."));
    return NULL;
  }

  return ctx.GetRawAndSetNull();
}

class cD3D9ContextRT : public sD3D9Context
{
  niBeginClass(cD3D9ContextRT);

 public:
  cD3D9* mpDrv;

  cD3D9ContextRT(
      cD3D9* apParent,
      iTexture* apRT0,
      iTexture* apRT1,
      iTexture* apRT2,
      iTexture* apRT3,
      iTexture* apDS)
      : sD3D9Context(apParent->GetGraphics())
  {
    mpDrv = apParent;
    niAssert(mpDrv != NULL);
    mptrRT[0] = apRT0;
    mptrRT[1] = apRT1;
    mptrRT[2] = apRT2;
    mptrRT[3] = apRT3;
    mptrDS = apDS;
    mrectViewport = Recti(0,0,apRT0->GetWidth(),apRT0->GetHeight());
    mrectScissor = mrectViewport;
  }
  ~cD3D9ContextRT() {
    Invalidate();
  }

  const tU32 GetNumAASamples() const {
    return (mptrRT[0]->GetFlags()&eTextureFlags_RTAA_All)?4:0;
  }

  virtual tBool __stdcall IsOK() const {
    return mptrRT[0].IsOK();
  }

  virtual void __stdcall Invalidate() {
  }

  virtual iGraphics* __stdcall GetGraphics() const {
    return mpDrv?mpDrv->GetGraphics():NULL;
  }
  virtual iGraphicsDriver* __stdcall GetDriver() const {
    return mpDrv;
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall Display(tGraphicsDisplayFlags aFlags, const sRecti& aRect) {
    ++mnSyncCounter; // Make sure the Viewport and scissor will be set next frame
    return eTrue;
  }

  /////////////////////////////////////////////
  virtual iBitmap2D* __stdcall CaptureFrontBuffer() const {
    return NULL;
  }

  /////////////////////////////////////////////
  virtual void __stdcall ClearBuffers(tClearBuffersFlags clearBuffer, tU32 anColor, tF32 afDepth, tI32 anStencil) {
    if (!mpDrv) return;
    return mpDrv->_ClearBuffers(this,clearBuffer,anColor,afDepth,anStencil);
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall DrawOperation(iDrawOperation* apDrawOp) {
    if (!mpDrv) return eFalse;
    return mpDrv->_DrawOperation(this,apDrawOp);
  }

  niEndClass(cD3D9ContextRT);
};

/////////////////////////////////////////////
iGraphicsContextRT* __stdcall cD3D9::CreateContextForRenderTargets(
  iTexture* apRT0, iTexture* apRT1, iTexture* apRT2, iTexture* apRT3, iTexture* apDS)
{
  niCheckIsOK(apRT0,NULL);

  Ptr<iGraphicsContextRT> ctx = niNew cD3D9ContextRT(
    this, apRT0, apRT1, apRT2, apRT3, apDS);
  niCheckIsOK(ctx, NULL);

  return ctx.GetRawAndSetNull();
}

HRESULT D3D9_ResetContext(iGraphicsContext* apContext) {
  cD3D9ContextWindow* ctx = (cD3D9ContextWindow*)apContext;
  return ctx->mpD3D9Device->Reset(&ctx->mPP);
}
void D3D9_Clear(iGraphicsContext* apContext, const achar* aaszMsg) {
  /* Do Nothing... */
}

void D3D9_ContextDeviceLost(iGraphicsContext* apContext) {
  cD3D9ContextWindow* ctx = (cD3D9ContextWindow*)apContext;
  ctx->OnDeviceLost();
}
void D3D9_UpdateWindow(iGraphicsContext* apContext) {
  cD3D9ContextWindow* ctx = (cD3D9ContextWindow*)apContext;
  if (ctx->mptrWindow.IsOK()) {
    ctx->mptrWindow->RedrawWindow();
    ctx->mptrWindow->UpdateWindow(eFalse);
  }
}
void D3D9_FinalizeInitialize(iGraphicsContext* apContext, tBool abReset) {
  cD3D9ContextWindow* ctx = (cD3D9ContextWindow*)apContext;
  ctx->FinalizeInitialize(abReset);
}

IDirect3D9* D3D9_GetD3D9(iGraphicsDriver* apDriver) {
  cD3D9* d3d9 = (cD3D9*)apDriver;
  return d3d9->GetD3D9();
}

IDirect3DDevice9* D3D9_GetD3D9Device(iGraphicsDriver* apDriver) {
  cD3D9* d3d9 = (cD3D9*)apDriver;
  return d3d9->GetD3D9Device();
}

//--------------------------------------------------------------------------------------------
//
//  Instanciation of the Direct3D9 Graphics Driver
//
//--------------------------------------------------------------------------------------------

niExportFunc(iUnknown*) New_GraphicsDriver_D3D9(const Var& avarA, const Var&) {
  QPtr<iGraphics> ptrGraphics = avarA;
  niCheckIsOK(ptrGraphics,NULL);
  return (iGraphicsDriver*)niNew cD3D9(ptrGraphics);
}

#endif
