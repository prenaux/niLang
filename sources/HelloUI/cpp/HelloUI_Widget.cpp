#include <niAppLib.h>
#include "HelloUI_data.h"
using namespace ni;

#define TEST_ONPAINT_CANVAS
#define TEST_ONPAINT_CANVAS_TEXT
#define TEST_INDEPENDANT_CANVAS
#define TEST_CHILD_WIDGETS
// #define TEST_IPCAM 640
#define TEST_TEXT_TABS
#ifdef TEST_CHILD_WIDGETS
_HDecl(ID_Button);
_HDecl(ID_EditBox);
#endif

static const char _kText[] =
#ifdef TEST_TEXT_TABS
    "if (narf) {\n"
    "\tblup\n"
    "\tblip\n"
    "\twhile (foo) {\n"
    "\t\tyata!\n"
    "\t}\n"
    "}\n"
    "Some\tTabbed\tStuff\tBla\n"
    "Alpha\tBli\tNarf\tOmega\n"
    "\n"
#endif
    "Hello, 你好, Привет! Are you paul?\n"
    "Wrote Compiled Shader\n"
    "\n"
    "If you can keep your head when all about you\n"
    "Are losing theirs and blaming it on you,\n"
    "If you can trust yourself when all men doubt you\n"
    "But make allowance for their doubting too,\n"
    "If you can wait and not be tired by waiting,\n"
    "Or being lied about, don't deal in lies,\n"
    "Or being hated, don't give way to hating,\n"
    "And yet don’t look too good, nor talk too wise:\n"
    "\n"
    "If you can dream - and not make dreams your master,\n"
    "If you can think - and not make thoughts your aim;\n"
    "If you can meet with Triumph and Disaster\n"
    "And treat those two impostors just the same;\n"
    "If you can bear to hear the truth you've spoken\n"
    "Twisted by knaves to make a trap for fools,\n"
    "Or watch the things you gave your life to, broken,\n"
    "And stoop and build'em up with worn-out tools:\n"
    "\n"
    "If you can make one heap of all your winnings\n"
    "And risk it all on one turn of pitch-and-toss,\n"
    "And lose, and start again at your beginnings\n"
    "And never breath a word about your loss;\n"
    "If you can force your heart and nerve and sinew\n"
    "To serve your turn long after they are gone,\n"
    "And so hold on when there is nothing in you\n"
    "Except the Will which says to them: 'Hold on!'\n"
    "\n"
    "If you can talk with crowds and keep your virtue,\n"
    "Or walk with kings-nor lose the common touch,\n"
    "If neither foes nor loving friends can hurt you;\n"
    "If all men count with you, but none too much,\n"
    "If you can fill the unforgiving minute\n"
    "With sixty seconds' worth of distance run,\n"
    "Yours is the Earth and everything that's in it,\n"
    "And-which is more - you'll be a Man, my son!\n"
    "\n"
    "-- Rudyard Kipling\n";


struct HelloUIWidget : public cWidgetSinkImpl<>
{
  int _clickCount;

#ifdef TEST_ONPAINT_CANVAS
  Ptr<iOverlay> _overlay;
  float _iconX, _iconY, _velX, _velY;
#endif

#ifdef TEST_ONPAINT_CANVAS_TEXT
  Ptr<iFont> _monoFont;
  Ptr<iFont> _bmpFont;
#endif

#ifdef TEST_IPCAM
  Ptr<iOverlay> _ipcamOverlay;
#endif

  tBool __stdcall OnSinkAttached() niImpl {
    niAppLib_SetBuildText();

#ifdef TEST_ONPAINT_CANVAS
    _velX = 15;
    _velY = 10;
    // niDebugFmt(("... HelloUIWidget OnSinkAttached"));

    {
      Ptr<iFile> fp = niFileOpenBin2H(placeholder_png);
      if (niIsOK(fp)) {
        QPtr<iBitmap2D> bmp = mpWidget->GetGraphics()->LoadBitmap(fp);
        if (niIsOK(bmp)) {
          Ptr<iTexture> tex = mpWidget->GetGraphics()->CreateTextureFromBitmap(_H(fp->GetSourcePath()),bmp,0);
          _overlay = mpWidget->GetGraphics()->CreateOverlayTexture(tex);
        }
      }
    }
#endif

#ifdef TEST_ONPAINT_CANVAS_TEXT
    {
      _monoFont = mpWidget->GetGraphics()->LoadFont(_H("Monospace"));
      _monoFont->SetSizeAndResolution(Vec2f(12,12),12,mpWidget->GetUIContext()->GetContentsScale());
      _bmpFont = mpWidget->GetGraphics()->LoadFont(_H("8x8"));
      _bmpFont->SetSizeAndResolution(Vec2f(16,16),8,mpWidget->GetUIContext()->GetContentsScale());
    }
#endif

#ifdef TEST_IPCAM
    {
      Ptr<iFile> fp = niFileOpenBin2H(test_ipcam);
      if (niIsOK(fp)) {
        Ptr<iVideoDecoder> decoder =  mpWidget->GetGraphics()->CreateVideoDecoder(
            _H(fp->GetSourcePath()),
            fp,
            eVideoDecoderFlags_TargetTexture);
        if (decoder.IsOK()) {
          Ptr<iTexture> tex = decoder->GetTargetTexture();
          _ipcamOverlay = mpWidget->GetGraphics()->CreateOverlayTexture(tex);
        }
        else {
          niWarning(niFmt("Can't create video decoder for '%s'.",
                          fp->GetSourcePath()));
        }
      }
    }
#endif

#ifdef TEST_CHILD_WIDGETS
    Ptr<iWidget> w = mpWidget->GetUIContext()->CreateWidget(
        "Button",
        mpWidget,
        Vec4f(0,0,200,200),
        eWidgetStyle_HoldFocus,
        _HC(ID_Button));
    w->SetRelativeSize(Vec2f(0.4f,0.4f));
    w->SetMaximumSize(Vec2f(150,100));
    w->SetText(_H("HelloUI 你好! Are you paul?"));
    w->SetDockStyle(eWidgetDockStyle_SnapCenter);

    w = mpWidget->GetUIContext()->CreateWidget(
        "EditBox",
        mpWidget,
        Vec4f(0,0,350,300),
        eWidgetStyle_HoldFocus|eWidgetEditBoxStyle_MultiLine,
        _HC(ID_EditBox));
    w->SetRelativeSize(Vec2f(0.75,0.0));
    w->SetMaximumSize(Vec2f(350,0));
    w->SetText(_H(_kText));
    w->SetDockStyle(eWidgetDockStyle_SnapTopCenter);
#endif

#ifdef TEST_INDEPENDANT_CANVAS
    bottom_canvas(mpWidget, 150, ni::Callback1(
        [](iCanvas* apCanvas) {
          // flush any rendering command queued in the canvas
          apCanvas->Flush();
          apCanvas->GetGraphicsContext()->Display(0, sRecti::Zero());
          QPtr<iImage> image = apCanvas->GetGraphicsContext();
          if (image.IsOK()) {
            // (grey rectangle in bitmap)
            Ptr<iBitmap2D> bmp = image->GrabBitmap(eImageUsage_Target,sRecti::Null());
            bmp->ClearRectf(Recti(0,0,200,100), Vec4f(0.5,0.5,0.5,1));

            // get the bitmap content into the RT
            image->GrabTexture(eImageUsage_Target,sRecti::Null());
            // (yellow in canvas)
            apCanvas->BlitFill(Rectf(10,10,50,50), ULColorBuild(Vec3f(1,1,0)));
            apCanvas->Flush();
            apCanvas->GetGraphicsContext()->Display(0, sRecti::Zero());

            // (green in bitmap)
            bmp = image->GrabBitmap(eImageUsage_Target,sRecti::Null());
            bmp->ClearRectf(Recti(15,55,75,40), Vec4f(0,1,0,1));
          }
          else {
            niWarning("No image in canvas' context");
          }
          apCanvas->GetGraphicsContext()->Display(0, sRecti::Zero());
          return ni::eTrue;
        }));
#endif

    return eTrue;
  };

#ifdef TEST_INDEPENDANT_CANVAS
  void bottom_canvas(iWidget* apWidget, tU32 anHeight, iCallback* apCallback) {
    Ptr<iGraphics> graphics = apWidget->GetGraphics();
    Ptr<iGraphicsContext> graphicsContext = apWidget->GetGraphicsContext();
    Ptr<iImage> img = graphics->CreateImage(300,anHeight);
    Ptr<iGraphicsContext> gc = graphics->CreateImageGraphicsContext(img);
    Ptr<iCanvas> canvas = graphics->CreateCanvas(gc,NULL);
    {
      canvas->BlitFill(canvas->GetViewport().ToFloat(),~0);
      if (apCallback) {
        apCallback->RunCallback(canvas.ptr(), niVarNull);
      }
      canvas->Flush();
    }
    Ptr<iOverlay> ovr = apWidget->GetGraphics()->CreateOverlayImage(img);

    Ptr<iWidget> w = apWidget->GetUIContext()->CreateWidget(
        "Button",
        apWidget,
        Vec4f(0,0,img->GetWidth(),img->GetHeight()),
        eWidgetStyle_HoldFocus|
        eWidgetButtonStyle_NoFrame|
        eWidgetButtonStyle_IconCenter|
        eWidgetButtonStyle_IconFit,
        NULL);
    w->GetFont()->SetSizeAndResolution(Vec2f(18,18), 18, mpWidget->GetUIContext()->GetContentsScale());
    w->SetDockStyle(eWidgetDockStyle_SnapBottomCenter);

    QPtr<iWidgetButton> bt = w;
    bt->SetIcon(ovr);
  }
#endif

  tBool __stdcall OnCommand(iWidgetCommand* apCmd) niImpl {
#ifdef TEST_CHILD_WIDGETS
    if (apCmd->GetSender()->GetID() == _HC(ID_Button) &&
        apCmd->GetID() == eWidgetButtonCmd_Clicked)
    {
      niLog(Info,"Clicked the button.");
      QPtr<iWidget> w = apCmd->GetSender();
      if (niIsOK(w)) {
        w->SetText(_H(niFmt("HelloUI, clickCount: %d", ++_clickCount)));
      }
    }
    else if (apCmd->GetSender()->GetID() == _HC(ID_EditBox) &&
             apCmd->GetID() == eWidgetEditBoxCmd_Modified)
    {
      QPtr<iWidget> wEB = apCmd->GetSender();
      if (niIsOK(wEB)) {
        QPtr<iWidget> wButton = wEB->GetUIContext()->GetRootWidget()->FindWidget(_HC(ID_Button));
        if (niIsOK(wButton)) {
          wButton->SetText(wEB->GetText());
        }
      }
    }
#endif

    return eFalse;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
#ifdef TEST_ONPAINT_CANVAS_TEXT
    {
      const tU32 blitTextFlags =
          0|
          // eFontFormatFlags_Border|
          // eFontFormatFlags_CenterH|
          // eFontFormatFlags_Bottom|
          eFontFormatFlags_ClipH|
          eFontFormatFlags_ClipV
          ;
      const tU32 blitTextLoopCount = 1;
      const tF32 textWidth = mpWidget->GetSize().x/3;
      const tF32 textHeight = mpWidget->GetSize().y-200;

      niLoop(i,blitTextLoopCount) {
        apCanvas->BlitFill(
          Rectf(20,120,textWidth,textHeight),
          0xFF555555);
        apCanvas->BlitText(
          mpWidget->GetFont(),
          Rectf(20,120,textWidth,textHeight),
          blitTextFlags, _kText);
        apCanvas->Flush();
      }

      niLoop(i,blitTextLoopCount) {
        const tF32 r = mpWidget->GetSize().x - 20;
        const tF32 x = r - (mpWidget->GetSize().x/3);
        const tF32 w = mpWidget->GetSize().x/3;

        apCanvas->BlitFill(
          Rectf(r-12,20,12,12),
          0xFF558855);
        const tF32 th = apCanvas->BlitText(
          _monoFont,
          Rectf(x, 20, w, 0),
          eFontFormatFlags_Right, _kText).GetHeight();

        apCanvas->BlitFill(
          Rectf(r-16,20+th,16,16),
          0xFF555588);
        apCanvas->BlitText(
          _bmpFont,
          Rectf(x, 20 + th, w, 0),
          eFontFormatFlags_Right, _kText).GetHeight();

        apCanvas->Flush();
      }
    }
#endif

#ifdef TEST_ONPAINT_CANVAS
    if (niIsOK(_overlay)) {
      apCanvas->Flush();
      int i = 0;
      _overlay->SetFiltering(eTrue);
      _overlay->SetBlendMode(eBlendMode_NoBlending);
      apCanvas->BlitOverlay(sRectf(i*64,0,64,64),_overlay);
      apCanvas->Flush();
      ++i;
      _overlay->SetFiltering(eFalse);
      _overlay->SetBlendMode(eBlendMode_NoBlending);
      apCanvas->BlitOverlay(sRectf(i*64,0,64,64),_overlay);
      apCanvas->Flush();
      ++i;
      _overlay->SetFiltering(eTrue);
      _overlay->SetBlendMode(eBlendMode_Translucent);
      apCanvas->BlitOverlay(sRectf(i*64,0,64,64),_overlay);
      apCanvas->Flush();
      ++i;
      _overlay->SetFiltering(eFalse);
      _overlay->SetBlendMode(eBlendMode_Translucent);
      apCanvas->BlitOverlay(sRectf(i*64,0,64,64),_overlay);
      apCanvas->Flush();
      ++i;

      const int w = 64;
      const int h = 64;
      _overlay->SetFiltering(eTrue);
      _overlay->SetBlendMode(eBlendMode_Translucent);
      const float dt = ni::GetLang()->GetFrameTime();
      _iconX += _velX * dt * 25.0f;
      _iconY += _velY * dt * 25.0f;
      const sRectf widgetRect = mpWidget->GetWidgetRect();
      if (_iconX < widgetRect.x) {
        _iconX = widgetRect.x;
        _velX = -_velX;
      }
      else if (_iconX > (widgetRect.z-w)) {
        _iconX = (widgetRect.z-w);
        _velX = -_velX;
      }

      if (_iconY < widgetRect.y) {
        _iconY = widgetRect.y;
        _velY = -_velY;
      }
      else if (_iconY > (widgetRect.w-h)) {
        _iconY = (widgetRect.w-h);
        _velY = -_velY;
      }

      apCanvas->BlitOverlay(sRectf(_iconX,_iconY,w,h),_overlay);
    }
#endif

#ifdef TEST_IPCAM
    if (niIsOK(_ipcamOverlay)) {
      _overlay->SetFiltering(eTrue);
      _overlay->SetBlendMode(eBlendMode_NoBlending);
      apCanvas->BlitOverlay(
          Rectf(mpWidget->GetSize().x-TEST_IPCAM-10,
                mpWidget->GetSize().y-(TEST_IPCAM/4*3)-32,
                TEST_IPCAM,TEST_IPCAM/4*3),
          _ipcamOverlay);
      apCanvas->Flush();
    }
#endif

    return eFalse;
  }
};

niExportFunc(ni::iWidgetSink*) New_HelloUI_Widget() {
  return niNew HelloUIWidget();
}
