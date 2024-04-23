#include <niAppLib.h>
#include "HelloUI_data.h"
#ifdef niJSCC
#include <emscripten.h>
#endif

using namespace ni;

struct HelloUI_VR : public cWidgetSinkImpl<>
{
  tBool __stdcall OnSinkAttached() niImpl {
    return eTrue;
  };

  tBool __stdcall OnCommand(iWidgetCommand* apCmd) niImpl {
    return eFalse;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
    return eFalse;
  }
};

niExportFunc(iWidgetSink*) New_HelloUI_VR() {
  return niNew HelloUI_VR();
}
