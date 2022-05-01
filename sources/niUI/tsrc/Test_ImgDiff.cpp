#include "stdafx.h"

#include "../Image.h"

using namespace ni;

struct FImgDiff {
  QPtr<iGraphics> graphics;

  FImgDiff() {
    graphics = niCreateInstance(niUI,Graphics,niVarNull,niVarNull);
  }
  ~FImgDiff() {
  }
};


TEST_FIXTURE(FImgDiff,Basic) {
  QPtr<iBitmap2D> imgA = graphics->LoadBitmapFromRes(_H("test/img_diff/AnimMixer_A.jpg"),NULL);
  CHECK_RETURN_IF_FAILED(imgA.IsOK());
  QPtr<iBitmap2D> imgB = graphics->LoadBitmapFromRes(_H("test/img_diff/AnimMixer_B.jpg"),NULL);
  CHECK_RETURN_IF_FAILED(imgB.IsOK());

  Ptr<iBitmap2D> imgDiff = graphics->CreateBitmap2D(imgA->GetWidth(), imgA->GetHeight(),
                                                    IMAGE_PIXELFORMAT);
  CHECK_RETURN_IF_FAILED(imgDiff.IsOK());

  const tI64 numDiff = graphics->ComputeBitmapDiff(imgA, imgB, imgDiff, 0.1, eTrue);
  niDebugFmt(("... numDiff: %s", numDiff));
  CHECK_EQUAL(24209, numDiff);

  const tF64 diffPct = (tF64)numDiff / (tF64)(imgA->GetWidth()*imgA->GetHeight());
  niDebugFmt(("... diffPct: %.1f%%", diffPct*100.0));

  const cString outputPath = UnitTest::GetTestOutputFilePath(niFmt("%s_imgDiff.png", m_testName));
  graphics->SaveBitmap(outputPath.Chars(), imgDiff);
  niDebugFmt(("... imgDiff saved to: %s", outputPath));
}
