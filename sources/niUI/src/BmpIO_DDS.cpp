// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Bitmap2D.h"

/*
  #define DDPF_PALETTEINDEXED4                    0x00000008l
  #define DDPF_PALETTEINDEXEDTO8                  0x00000010l
  #define DDPF_PALETTEINDEXED8                    0x00000020l
  #define DDPF_RGB                                0x00000040l
  #define DDPF_COMPRESSED                         0x00000080l
  #define DDPF_RGBTOYUV                           0x00000100l
  #define DDPF_YUV                                0x00000200l
  #define DDPF_ZBUFFER                            0x00000400l
  #define DDPF_PALETTEINDEXED1                    0x00000800l
  #define DDPF_PALETTEINDEXED2                    0x00001000l
  #define DDPF_ZPIXELS                            0x00002000l
  #define DDPF_STENCILBUFFER                      0x00004000l
  #define DDPF_ALPHAPREMULT                       0x00008000l
  #define DDPF_LUMINANCE                          0x00020000l
  #define DDPF_BUMPLUMINANCE                      0x00040000l
  #define DDPF_BUMPDUDV                           0x00080000l
*/

const tU32 DDS_HEADER_FOURCC = niFourCC('D','D','S',' ');

const tU32 DDS_ALPHAPIXELS = 0x00000001;
const tU32 DDS_ALPHAONLY = 0x00000002;

const tU32 DDS_FOURCC   = 0x00000004;
const tU32 DDS_INDEXED4   = 0x00000008;
const tU32 DDS_INDEXEDTO8 = 0x00000010;
const tU32 DDS_INDEXED    = 0x00000020;
const tU32 DDS_RGB      = 0x00000040;
const tU32 DDS_RGBA     = 0x00000041;
const tU32 DDS_LUMINANCE  = 0x00020000;
const tU32 DDS_DEPTH    = 0x00800000;

const tU32 DDS_COMPLEX = 0x00000008;
const tU32 DDS_CUBEMAP = 0x00000200;
const tU32 DDS_CUBEMAP_POSITIVEX = 0x00000400;
const tU32 DDS_CUBEMAP_NEGATIVEX = 0x00000800;
const tU32 DDS_CUBEMAP_POSITIVEY = 0x00001000;
const tU32 DDS_CUBEMAP_NEGATIVEY = 0x00002000;
const tU32 DDS_CUBEMAP_POSITIVEZ = 0x00004000;
const tU32 DDS_CUBEMAP_NEGATIVEZ = 0x00008000;
const tU32 DDS_VOLUME  = 0x00200000;

static tU32 _kCubeMapFaceFlags[6] = {
  DDS_CUBEMAP_POSITIVEX,
  DDS_CUBEMAP_NEGATIVEX,
  DDS_CUBEMAP_POSITIVEY,
  DDS_CUBEMAP_NEGATIVEY,
  DDS_CUBEMAP_POSITIVEZ,
  DDS_CUBEMAP_NEGATIVEZ
};

const tU32 FOURCC_DXT1 = niFourCC('D','X','T','1');
const tU32 FOURCC_DXT2 = niFourCC('D','X','T','2');
const tU32 FOURCC_DXT3 = niFourCC('D','X','T','3');
const tU32 FOURCC_DXT4 = niFourCC('D','X','T','4');
const tU32 FOURCC_DXT5 = niFourCC('D','X','T','5');
const tU32 FOURCC_A16B16G16R16 = 36;
const tU32 FOURCC_R16F                 = 111;
const tU32 FOURCC_G16R16F              = 112;
const tU32 FOURCC_A16B16G16R16F        = 113;
const tU32 FOURCC_R32F                 = 114;
const tU32 FOURCC_G32R32F              = 115;
const tU32 FOURCC_A32B32G32R32F        = 116;

#pragma pack(push,1)

struct sDDSPixelFormat
{
  tU32 dwSize;
  tU32 dwFlags;
  tU32 dwFourCC;
  tU32 dwRGBBitCount;
  tU32 dwRBitMask;
  tU32 dwGBitMask;
  tU32 dwBBitMask;
  tU32 dwABitMask;
};

struct sDDSHeader
{
  tU32 dwSize;
  tU32 dwFlags;
  tU32 dwHeight;
  tU32 dwWidth;
  tU32 dwPitchOrLinearSize;
  tU32 dwDepth;
  tU32 dwMipMapCount;
  tU32 dwReserved1[11];
  sDDSPixelFormat ddspf;
  tU32 dwCaps1;
  tU32 dwCaps2;
  tU32 dwReserved2[3];
};

#pragma pack(pop)

//////////////////////////////////////////////////////////////////////////////////////////////
struct BitmapLoader_DDS : public cIUnknownImpl<iBitmapLoader> {
  virtual iBitmapBase* __stdcall LoadBitmap(iGraphics* apGraphics, iFile* pFile) niImpl {
    sDDSHeader ddsHeader;
    cString strFormat;

    // read the header fourcc
    if (pFile->ReadLE32() != DDS_HEADER_FOURCC) {
      niError(_A("Invalid DDS file."));
      return NULL;
    }

    // read in DDS header
    pFile->ReadRaw(tPtr(&ddsHeader), sizeof(ddsHeader));

    // figure out what the image format is
    if (ddsHeader.ddspf.dwFlags & DDS_FOURCC)
    {
      switch(ddsHeader.ddspf.dwFourCC)
      {
        case FOURCC_DXT1: strFormat = _A("DXT1"); break;
        case FOURCC_DXT2: strFormat = _A("DXT2"); break;
        case FOURCC_DXT3: strFormat = _A("DXT3"); break;
        case FOURCC_DXT4: strFormat = _A("DXT4"); break;
        case FOURCC_DXT5: strFormat = _A("DXT5"); break;
        case FOURCC_A16B16G16R16: strFormat = _A("R16G16B16A16"); break;
        case FOURCC_R16F:     strFormat = _A("FR16"); break;
        case FOURCC_G16R16F:    strFormat = _A("FR16G16"); break;
        case FOURCC_A16B16G16R16F:  strFormat = _A("FR16G16B16A16"); break;
        case FOURCC_R32F:     strFormat = _A("FR32"); break;
        case FOURCC_G32R32F:    strFormat = _A("FR32G32"); break;
        case FOURCC_A32B32G32R32F:  strFormat = _A("FR32G32B32A32"); break;
      }
    }
    else if (ddsHeader.ddspf.dwFlags == DDS_RGBA && ddsHeader.ddspf.dwRGBBitCount == 32)
    {
      switch (ddsHeader.ddspf.dwRBitMask)
      {
        case 1023:      strFormat = _A("B10G10R10A2");  break;
        case 1072693248:  strFormat = _A("R10G10B10A2");  break;
        case 16711680:    strFormat = _A("B8G8R8A8");   break;
        case 255:     strFormat = _A("R8G8B8A8");   break;
      }
    }
    else if (ddsHeader.ddspf.dwFlags == DDS_RGB  && ddsHeader.ddspf.dwRGBBitCount == 32)
    {
      switch (ddsHeader.ddspf.dwRBitMask)
      {
        case 16711680:    strFormat = _A("B8G8R8X8");   break;
        case 255:     strFormat = _A("R8G8B8X8");   break;
      }
    }
    else if (ddsHeader.ddspf.dwFlags == DDS_RGB && ddsHeader.ddspf.dwRGBBitCount == 24)
    {
      switch (ddsHeader.ddspf.dwRBitMask)
      {
        case 16711680:    strFormat = _A("B8G8R8");   break;
      }
    }
    else if (ddsHeader.ddspf.dwFlags == DDS_RGBA  && ddsHeader.ddspf.dwRGBBitCount == 16)
    {
      switch (ddsHeader.ddspf.dwRBitMask)
      {
        case 224: strFormat = _A("B2G3R3A8"); break;
        case 31744: strFormat = _A("B5G5R5A1"); break;
        case 3840:  strFormat = _A("B4G4R4A4");   break;
      }
    }
    else if (ddsHeader.ddspf.dwFlags == DDS_RGB  && ddsHeader.ddspf.dwRGBBitCount == 16)
    {
      switch (ddsHeader.ddspf.dwRBitMask)
      {
        case 63488: strFormat = _A("B5G6R5"); break;
        case 31744: strFormat = _A("B5G5R5X1"); break;
        case 3840:  strFormat = _A("B4G4R4X4");   break;
      }
    }
    else if (ddsHeader.ddspf.dwFlags == DDS_RGB && ddsHeader.ddspf.dwRGBBitCount == 8)
    {
      switch (ddsHeader.ddspf.dwRBitMask)
      {
        case 224: strFormat = _A("B2G3R3"); break;
      }
    }
    else if (ddsHeader.ddspf.dwFlags == DDS_ALPHAONLY && ddsHeader.ddspf.dwRGBBitCount == 8)
    {
      strFormat = _A("A8");
    }
    else if (ddsHeader.ddspf.dwFlags == DDS_LUMINANCE && ddsHeader.ddspf.dwRGBBitCount == 8)
    {
      switch (ddsHeader.ddspf.dwRBitMask)
      {
        case 255: strFormat = _A("L8"); break;
      }
    }
    else if (ddsHeader.ddspf.dwFlags == (DDS_LUMINANCE|DDS_ALPHAPIXELS))
    {
      if (ddsHeader.ddspf.dwRGBBitCount == 8)
      {
        switch (ddsHeader.ddspf.dwRBitMask)
        {
          case 15:  strFormat = _A("A4L4"); break;
        }
      }
      else if (ddsHeader.ddspf.dwRGBBitCount == 16)
      {
        switch (ddsHeader.ddspf.dwRBitMask)
        {
          case 255: strFormat = _A("A8L8"); break;
        }
      }
    }
    else if (niFlagIs(ddsHeader.ddspf.dwFlags,DDS_INDEXED))
    {
      if (ddsHeader.ddspf.dwRGBBitCount == 8) {
        // No indexed support, we store A8 first, at the end if there's a palette
        // the bitmap will be converted to B8G8R8A8
        strFormat = _A("A8");
      }
    }

    if (strFormat.IsEmpty())
    {
      niError(niFmt("Can't load the format of the DDS image : %c%c%c%c, Flags: 0x%x, Bits: %d, RedMask:0x%x, GreenMask:0x%x, BlueMask:0x%x, AlphaMask:0x%x.",
                    niFourCCA(ddsHeader.ddspf.dwFourCC)?niFourCCA(ddsHeader.ddspf.dwFourCC):'#',
                    niFourCCB(ddsHeader.ddspf.dwFourCC)?niFourCCB(ddsHeader.ddspf.dwFourCC):'#',
                    niFourCCC(ddsHeader.ddspf.dwFourCC)?niFourCCC(ddsHeader.ddspf.dwFourCC):'#',
                    niFourCCD(ddsHeader.ddspf.dwFourCC)?niFourCCD(ddsHeader.ddspf.dwFourCC):'#',
                    ddsHeader.ddspf.dwFlags,
                    ddsHeader.ddspf.dwRGBBitCount,
                    ddsHeader.ddspf.dwRBitMask,
                    ddsHeader.ddspf.dwGBitMask,
                    ddsHeader.ddspf.dwBBitMask,
                    ddsHeader.ddspf.dwABitMask));
      return NULL;
    }

    Ptr<iBitmapBase> ptrRet;

    // check if image is a volume texture
    if ((ddsHeader.dwCaps2 & DDS_VOLUME) && (ddsHeader.dwDepth > 0))
    {
#pragma niTodo("Implement FULL support for 3D textures.")
      niError(_A("3D Textures not supported."));
      return NULL;
    }
    // check if image is a cubemap
    else if (ddsHeader.dwCaps2 & DDS_CUBEMAP)
    {
      Ptr<iBitmapCube> ptrCube = apGraphics->CreateBitmapCubeEx(ddsHeader.dwWidth, apGraphics->CreatePixelFormat(strFormat.Chars()));
      niCheck(ptrCube.IsOK(), NULL);

      if (ddsHeader.dwMipMapCount) {
        ptrCube->CreateMipMaps(ddsHeader.dwMipMapCount-1, eFalse);
      }

      // load all surfaces for the image (6 surfaces for cubemaps)
      for (tU32 i = 0; i < 6; ++i)
      {
        iBitmap2D* pFace = ptrCube->GetFace(eBitmapCubeFace(i));
        if (ddsHeader.dwCaps2 & _kCubeMapFaceFlags[i]) {
          pFile->ReadRaw(pFace->GetData(), pFace->GetSize());
          if (ddsHeader.dwMipMapCount)
          {
            for (tU32 mip = 0; mip < pFace->GetNumMipMaps(); ++mip)
            {
              pFile->ReadRaw(pFace->GetMipMap(mip)->GetData(), pFace->GetMipMap(mip)->GetSize());
            }
          }
        }
      }

      ptrRet = ptrCube;
    }
    // 2d image
    else
    {
      Ptr<iBitmap2D> ptr2D = apGraphics->CreateBitmap2DEx(ddsHeader.dwWidth, ddsHeader.dwHeight, apGraphics->CreatePixelFormat(strFormat.Chars()));
      niCheck(ptr2D.IsOK(),NULL);

      astl::vector<tU32> palette;
      if (ddsHeader.ddspf.dwFlags == DDS_INDEXED) {
        palette.resize(256);
        pFile->ReadRaw(palette.data(),256*4);
      }

      if (ddsHeader.dwMipMapCount) {
        ptr2D->CreateMipMaps(ddsHeader.dwMipMapCount-1, eFalse);
      }

      pFile->ReadRaw(ptr2D->GetData(), ptr2D->GetSize());
      if (ddsHeader.dwMipMapCount) {
        for (tU32 mip = 0; mip < ptr2D->GetNumMipMaps(); ++mip) {
          pFile->ReadRaw(ptr2D->GetMipMap(mip)->GetData(), ptr2D->GetMipMap(mip)->GetSize());
        }
      }

      if (!palette.empty()) {
        Ptr<iBitmap2D> convertedBmp = apGraphics->CreateBitmap2D(
            ptr2D->GetWidth(), ptr2D->GetHeight(), "B8G8R8A8");
        BmpUtils_BlitPaletteTo32Bits(
            convertedBmp->GetData(), ptr2D->GetData(), ptr2D->GetWidth() * ptr2D->GetHeight(), palette.data());
        ptrRet = convertedBmp;
      }
      else {
        ptrRet = ptr2D;
      }
    }

    return ptrRet.GetRawAndSetNull();
  }
};

niExportFunc(iUnknown*) New_BitmapLoader_dds(const Var&,const Var&) {
  return niNew BitmapLoader_DDS();
}
