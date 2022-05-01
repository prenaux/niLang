#include <niAppLib.h>
#include "HelloUI_data.h"
#ifdef niJSCC
#include <emscripten.h>
#endif
using namespace ni;

struct HelloUI_AR : public cWidgetSinkImpl<>
{
  Ptr<iOverlay> _overlay;
  float _iconX = 0, _iconY = 0, _velX = 0, _velY = 0;

  tBool __stdcall OnSinkAttached() niImpl {
    niAppLib_SetBuildText();

    _velX = 15;
    _velY = 10;
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

#ifdef niJSCC
    emscripten_run_script("niApp.startAR();");
#endif

    return eTrue;
  };

  tBool __stdcall OnCommand(iWidgetCommand* apCmd) niImpl {
    return eFalse;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
    if (niIsOK(_overlay)) {
      apCanvas->Flush();

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

    return eFalse;
  }
};

niExportFunc(iWidgetSink*) New_HelloUI_AR() {
  return niNew HelloUI_AR();
}
