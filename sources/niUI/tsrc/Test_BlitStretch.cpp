#include "stdafx.h"
#include "../src/API/niUI_ModuleDef.h"
#include <niLang/Math/MathRect.h>
#include <niUI/Utils/ULColorBlend.h>
#include "data/LogoPlugin512.png.hxx"
#include "data/A.jpg.hxx"

using namespace ni;

struct FBlitStretch {
  QPtr<iGraphics> graphics;
  QPtr<iBitmap2D> bmp;
  const tF32 UPSAMPLE_X;
  const tF32 UPSAMPLE_Y;
  const tF32 DOWNSAMPLE_X;
  const tF32 DOWNSAMPLE_Y;

  FBlitStretch() : UPSAMPLE_X(2.73f),
                   UPSAMPLE_Y(1.93f),
                   DOWNSAMPLE_X(0.43f),
                   DOWNSAMPLE_Y(0.87f)
  {
    niDebugFmt(("FBlitStretch::FBlitStretch"));
    graphics = niCreateInstance(niUI,Graphics,niVarNull,niVarNull);
  }
  ~FBlitStretch() {
    niDebugFmt(("FBlitStretch::~FBlitStretch"));
  }

  tBool LoadTestBitmapLogo() {
    Ptr<iFile> fp = niFileOpenBin2H(LogoPlugin512_png);
    niCheck(fp.IsOK(),eFalse);
    bmp = graphics->LoadBitmap(fp);
    niCheck(bmp.IsOK(),eFalse);
    return eTrue;
  }

  tBool LoadTestBitmapA() {
    Ptr<iFile> fp = niFileOpenBin2H(A_jpg);
    niCheck(fp.IsOK(),eFalse);
    bmp = graphics->LoadBitmap(fp);
    niCheck(bmp.IsOK(),eFalse);
    return eTrue;
  }

  void ResampleAndSave(
      const iBitmap2D* apSrc,
      const achar* aaszPxfmt,
      const achar* aaszPath,
      const tU32 anOutWidth,
      const tU32 anOutHeight)
  {
    Ptr<iBitmap2D> outBmp = graphics->CreateBitmap2D(anOutWidth,anOutHeight,aaszPxfmt);
    sVec4f c = {1.0f, 0.0f, 0.0f, 1.0f};
    outBmp->Clearf(c);
    outBmp->BlitStretch(apSrc,0,0,0,0,apSrc->GetWidth(),apSrc->GetHeight(),anOutWidth,anOutHeight);
    graphics->SaveBitmap(aaszPath,outBmp);
  }
};

TEST_FIXTURE(FBlitStretch, BasicAtZeroZeroRGBA) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapLogo());
  Ptr<iBitmap2D> outBmp = graphics->CreateBitmap2D(1024,713,"B8G8R8A8");
  outBmp->BlitStretch(bmp,0,0,0,0,512,512,1024,713);
  graphics->SaveBitmap(TEST_FILEPATH(niFmt("%s_Basic.png",m_testName)),outBmp);
}

TEST_FIXTURE(FBlitStretch, BasicAtZeroZeroRGB) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapA());
  Ptr<iBitmap2D> outBmp = graphics->CreateBitmap2D(1024,713,"B8G8R8A8");
  outBmp->BlitStretch(bmp,0,0,0,0,512,512,1024,713);
  graphics->SaveBitmap(TEST_FILEPATH(niFmt("%s_Basic.jpg",m_testName)),outBmp);
}

TEST_FIXTURE(FBlitStretch, UpsampleAtZeroZeroBGRA) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapLogo());
  ResampleAndSave(bmp,_A("B8G8R8A8"),
                  TEST_FILEPATH(niFmt("%s.png",m_testName)),
                  (tU32)((tF32)bmp->GetWidth()*UPSAMPLE_X),
                  (tU32)((tF32)bmp->GetHeight()*UPSAMPLE_Y));
}

TEST_FIXTURE(FBlitStretch, UpsampleAtZeroZeroRGBA) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapLogo());
  ResampleAndSave(bmp,_A("R8G8B8A8"),
                  TEST_FILEPATH(niFmt("%s.png",m_testName)),
                  (tU32)((tF32)bmp->GetWidth()*UPSAMPLE_X),
                  (tU32)((tF32)bmp->GetHeight()*UPSAMPLE_Y));
}

TEST_FIXTURE(FBlitStretch, DownsampleAtZeroZeroBGRA) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapLogo());
  ResampleAndSave(bmp,_A("B8G8R8A8"),
                  TEST_FILEPATH(niFmt("%s.png",m_testName)),
                  (tU32)((tF32)bmp->GetWidth()*DOWNSAMPLE_X),
                  (tU32)((tF32)bmp->GetHeight()*DOWNSAMPLE_Y));
}

TEST_FIXTURE(FBlitStretch, DownsampleAtZeroZeroRGBA) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapA());
  ResampleAndSave(bmp,_A("R8G8B8A8"),
                  TEST_FILEPATH(niFmt("%s.png",m_testName)),
                  (tU32)((tF32)bmp->GetWidth()*DOWNSAMPLE_X),
                  (tU32)((tF32)bmp->GetHeight()*DOWNSAMPLE_Y));
}

TEST_FIXTURE(FBlitStretch, UpsampleAtZeroZeroBGR) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapA());
  ResampleAndSave(bmp,_A("B8G8R8A8"),
                  TEST_FILEPATH(niFmt("%s.jpg",m_testName)),
                  (tU32)((tF32)bmp->GetWidth()*UPSAMPLE_X),
                  (tU32)((tF32)bmp->GetHeight()*UPSAMPLE_Y));
}

TEST_FIXTURE(FBlitStretch, UpsampleAtZeroZeroRGB) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapA());
  ResampleAndSave(bmp,_A("R8G8B8A8"),
                  TEST_FILEPATH(niFmt("%s.jpg",m_testName)),
                  (tU32)((tF32)bmp->GetWidth()*UPSAMPLE_X),
                  (tU32)((tF32)bmp->GetHeight()*UPSAMPLE_Y));
}

TEST_FIXTURE(FBlitStretch, DownsampleAtZeroZeroBGR) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapA());
  ResampleAndSave(bmp,_A("B8G8R8A8"),
                  TEST_FILEPATH(niFmt("%s.jpg",m_testName)),
                  (tU32)((tF32)bmp->GetWidth()*DOWNSAMPLE_X),
                  (tU32)((tF32)bmp->GetHeight()*DOWNSAMPLE_Y));
}

TEST_FIXTURE(FBlitStretch, DownsampleAtZeroZeroRGB) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapA());
  ResampleAndSave(bmp,_A("R8G8B8A8"),
                  TEST_FILEPATH(niFmt("%s.jpg",m_testName)),
                  (tU32)((tF32)bmp->GetWidth()*DOWNSAMPLE_X),
                  (tU32)((tF32)bmp->GetHeight()*DOWNSAMPLE_Y));
}

TEST_FIXTURE(FBlitStretch, DownsampleAtZeroZeroBGR_24) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapLogo());
  ResampleAndSave(bmp,_A("B8G8R8"),
                  TEST_FILEPATH(niFmt("%s.png",m_testName)),
                  (tU32)((tF32)bmp->GetWidth()*DOWNSAMPLE_X),
                  (tU32)((tF32)bmp->GetHeight()*DOWNSAMPLE_Y));
}

TEST_FIXTURE(FBlitStretch, DownsampleAtZeroZeroRGB_24) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapLogo());
  ResampleAndSave(bmp,_A("R8G8B8"),
                  TEST_FILEPATH(niFmt("%s.png",m_testName)),
                  (tU32)((tF32)bmp->GetWidth()*DOWNSAMPLE_X),
                  (tU32)((tF32)bmp->GetHeight()*DOWNSAMPLE_Y));
}

TEST_FIXTURE(FBlitStretch, NoResampleRGB_24) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapLogo());
  ResampleAndSave(bmp,_A("R8G8B8"),
                  TEST_FILEPATH(niFmt("%s.png",m_testName)),
                  (bmp->GetWidth()),(bmp->GetWidth()));
}

TEST_FIXTURE(FBlitStretch, NoResampleBGR_24) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapLogo());
  ResampleAndSave(bmp,_A("B8G8R8"),
                  TEST_FILEPATH(niFmt("%s.png",m_testName)),
                  (bmp->GetWidth()),(bmp->GetWidth()));
}

TEST_FIXTURE(FBlitStretch, DoubleRGBA) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapLogo());
  ResampleAndSave(bmp,_A("R8G8B8A8"),
                  TEST_FILEPATH(niFmt("%s.png",m_testName)),
                  (bmp->GetWidth()*2),(bmp->GetWidth()*2));
}

TEST_FIXTURE(FBlitStretch, DoubleBGRA) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapLogo());
  ResampleAndSave(bmp,_A("B8G8R8A8"),
                  TEST_FILEPATH(niFmt("%s.png",m_testName)),
                  (bmp->GetWidth()*2),(bmp->GetWidth()*2));
}

TEST_FIXTURE(FBlitStretch, DoubleBGR_24) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapLogo());
  ResampleAndSave(bmp,_A("B8G8R8"),
                  TEST_FILEPATH(niFmt("%s.png",m_testName)),
                  (bmp->GetWidth()*2),(bmp->GetWidth()*2));
}

TEST_FIXTURE(FBlitStretch, DoubleRGB_24) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapLogo());
  ResampleAndSave(bmp,_A("R8G8B8"),
                  TEST_FILEPATH(niFmt("%s.png",m_testName)),
                  (bmp->GetWidth()*2),(bmp->GetWidth()*2));
}

TEST_FIXTURE(FBlitStretch, HalfRGBA) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapLogo());
  ResampleAndSave(bmp,_A("R8G8B8A8"),
                  TEST_FILEPATH(niFmt("%s.png",m_testName)),
                  (bmp->GetWidth()/2),(bmp->GetWidth()/2));
}

TEST_FIXTURE(FBlitStretch, HalfBGRA) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapLogo());
  ResampleAndSave(bmp,_A("B8G8R8A8"),
                  TEST_FILEPATH(niFmt("%s.png",m_testName)),
                  (bmp->GetWidth()/2),(bmp->GetWidth()/2));
}

TEST_FIXTURE(FBlitStretch, HalfBGR_24) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapLogo());
  ResampleAndSave(bmp,_A("B8G8R8"),
                  TEST_FILEPATH(niFmt("%s.png",m_testName)),
                  (bmp->GetWidth()/2),(bmp->GetWidth()/2));
}

TEST_FIXTURE(FBlitStretch, HalfRGB_24) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapLogo());
  ResampleAndSave(bmp,_A("R8G8B8"),
                  TEST_FILEPATH(niFmt("%s.png",m_testName)),
                  (bmp->GetWidth()/2),(bmp->GetWidth()/2));
}

TEST_FIXTURE(FBlitStretch, BasicOffsetRGBA) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapLogo());
  Ptr<iBitmap2D> outBmp = graphics->CreateBitmap2D(1024,1024,"B8G8R8A8");
  outBmp->BlitStretch(bmp,39,52,0,0,439,439,1024,1024);
  graphics->SaveBitmap(TEST_FILEPATH(niFmt("%s.png",m_testName)),outBmp);
}

TEST_FIXTURE(FBlitStretch, BasicOffsetResampleRGBA) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapLogo());
  Ptr<iBitmap2D> outBmp = graphics->CreateBitmap2D(1024,1024,"B8G8R8A8");
  outBmp->BlitStretch(bmp,39,52,0,0,439,439,1024,1024);
  graphics->SaveBitmap(TEST_FILEPATH(niFmt("%s.png",m_testName)),outBmp);
}

TEST_FIXTURE(FBlitStretch, BasicOffsetSrcAndDestResampleRGBA) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapLogo());
  Ptr<iBitmap2D> outBmp = graphics->CreateBitmap2D(1024,1024,"B8G8R8A8");
  outBmp->BlitStretch(bmp,39,52,256,256,439,439,512,512);
  graphics->SaveBitmap(TEST_FILEPATH(niFmt("%s.png",m_testName)),outBmp);
}

TEST_FIXTURE(FBlitStretch, UndersampleOffsetSrcAndDestResampleRGBA) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapLogo());
  Ptr<iBitmap2D> outBmp = graphics->CreateBitmap2D(1024,1024,"B8G8R8A8");
  outBmp->BlitStretch(bmp,39,52,256,256,439,439,256,256);
  graphics->SaveBitmap(TEST_FILEPATH(niFmt("%s.png",m_testName)),outBmp);
}

TEST_FIXTURE(FBlitStretch, ExtremeUndersample) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapLogo());
  Ptr<iBitmap2D> outBmp = graphics->CreateBitmap2D(2,2,"B8G8R8A8");
  outBmp->BlitStretch(bmp,39,52,0,0,439,439,2,2);
  graphics->SaveBitmap(TEST_FILEPATH(niFmt("%s.png",m_testName)),outBmp);
}

TEST_FIXTURE(FBlitStretch, MipMapLogo) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapLogo());
  CHECK(bmp->CreateMipMaps(0,eTrue));
  niLoop(i,bmp->GetNumMipMaps()+1) {
    Ptr<iBitmap2D> l = bmp->GetLevel(i);
    graphics->SaveBitmap(TEST_FILEPATH(niFmt("%s_l%02d.png",m_testName,i)),l);
  }
}

TEST_FIXTURE(FBlitStretch, MipMapA) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapA());
  CHECK(bmp->CreateMipMaps(0,eTrue));
  niLoop(i,bmp->GetNumMipMaps()+1) {
    Ptr<iBitmap2D> l = bmp->GetLevel(i);
    graphics->SaveBitmap(TEST_FILEPATH(niFmt("%s_l%02d.png",m_testName,i)),l);
  }
}

TEST_FIXTURE(FBlitStretch, MipMapAToPow2) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapA());
  bmp = bmp->CreateResized(ni::GetNextPow2(bmp->GetWidth()),ni::GetNextPow2(bmp->GetHeight()));
  CHECK(bmp->CreateMipMaps(0,eTrue));
  niLoop(i,bmp->GetNumMipMaps()+1) {
    Ptr<iBitmap2D> l = bmp->GetLevel(i);
    graphics->SaveBitmap(TEST_FILEPATH(niFmt("%s_l%02d.png",m_testName,i)),l);
  }
}


TEST_FIXTURE(FBlitStretch, CleanupRGBA_DoubleBGR_24) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapLogo());

  // One method to cleanup the alpha, blit alpha in a new bitmap with
  // BlitAlpha. This is needed because the RGB channel of our source bitmap
  // isnt clean (aka its not a single uniform color when the alpha is zero).
  Ptr<iBitmap2D> cleanedUpBmp = graphics->CreateBitmap2DEx(bmp->GetWidth(),bmp->GetHeight(),bmp->GetPixelFormat());
  // Set our background to green... most likely we'd want to be black, but its
  // more explicit for the test case.
  cleanedUpBmp->Clearf(sColor4f::Green());
  // Blit using translucent alpha blending
  CHECK(cleanedUpBmp->BlitAlpha(bmp,0,0,0,0,bmp->GetWidth(),bmp->GetHeight(),
                                ePixelFormatBlit_Normal,
                                sColor4f::One(),sColor4f::One(),
                                eBlendMode_Translucent));

  // Do the final scaling
  ResampleAndSave(cleanedUpBmp,_A("B8G8R8"),
                  TEST_FILEPATH(niFmt("%s.png",m_testName)),
                  (bmp->GetWidth()*2),(bmp->GetWidth()*2));
}

TEST_FIXTURE(FBlitStretch, CleanupRGBA_2_DoubleBGR_24) {
  CHECK_RETURN_IF_FAILED(LoadTestBitmapLogo());

  // This is another way to cleanup the bitmap 'in-place' will be better than
  // creating a temporary bitmap in general.

  // We assume that the bitmap is R8G8B8A8 or B8G8R8A8
  CHECK(StrIEq(bmp->GetPixelFormat()->GetFormat(),"R8G8B8A8") ||
        StrIEq(bmp->GetPixelFormat()->GetFormat(),"B8G8R8A8"));

  // The actual cleanup code
  {
    // color we'll blend with... green to be explicit
    const tU32 blendWith = ULColorBuild(0,255,0,0);
    tU32* dest = (tU32*)bmp->GetData();
    niLoop(i,bmp->GetSize()/sizeof(tU32)) {
      const tU32 c = *dest;
      *dest = ULColorBlend24(c,blendWith,ULColorGetA(c));
      ++dest;
    }
  }

  ResampleAndSave(bmp,_A("B8G8R8"),
                  TEST_FILEPATH(niFmt("%s.png",m_testName)),
                  (bmp->GetWidth()*2),(bmp->GetWidth()*2));
}
