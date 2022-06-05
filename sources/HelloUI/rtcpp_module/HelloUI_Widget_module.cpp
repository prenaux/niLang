#include "stdafx.h"
#include <niUI/IWidgetEditBox.h>
#include <niUI/IWidgetButton.h>
#include <niUI/Utils/WidgetSinkImpl.h>
#include <niScriptCpp/Utils/ScriptCppImpl.h>

using namespace ni;

_HDecl(ID_Button);
_HDecl(ID_EditBox);

static const char _kText[] =
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
  Ptr<iFont> _monoFont;

  tBool __stdcall OnSinkAttached() niImpl {
    niDebugFmt(("... OnSinkAttached"));

    {
      _monoFont = mpWidget->GetGraphics()->LoadFont(_H("Monospace"));
      _monoFont->SetSizeAndResolution(
        Vec2f(12,12),12,
        mpWidget->GetUIContext()->GetContentsScale());
    }

    Ptr<iWidget> w = mpWidget->GetUIContext()->CreateWidget(
        "Button",
        mpWidget,
        Vec4f(0,0,250,250),
        eWidgetStyle_HoldFocus,
        _HC(ID_Button));
    w->SetRelativeSize(Vec2f(0.4f,0.4f));
    w->SetMaximumSize(Vec2f(200,200));
    w->SetText(_H("HelloUI\n你好!\nПривет!\nAre you paul?"));
    w->SetDockStyle(eWidgetDockStyle_SnapCenter);

    w = mpWidget->GetUIContext()->CreateWidget(
        "EditBox",
        mpWidget,
        Vec4f(0,0,350,100),
        eWidgetStyle_HoldFocus|eWidgetEditBoxStyle_MultiLine,
        _HC(ID_EditBox));
    w->SetRelativeSize(Vec2f(0.75,0.0));
    w->SetMaximumSize(Vec2f(350,0));
    w->SetText(_H(_kText));
    w->SetDockStyle(eWidgetDockStyle_SnapTopCenter);

    return eTrue;
  };

  tBool __stdcall OnSinkDetached() niImpl {
    niDebugFmt(("... OnSinkDetached"));
    mpWidget->InvalidateChildren();
    return eTrue;
  }

  tBool __stdcall OnCommand(iWidgetCommand* apCmd) niImpl {
    if (apCmd->GetSender()->GetID() == _HC(ID_Button) &&
        apCmd->GetID() == eWidgetButtonCmd_Clicked)
    {
      niLog(Info,"Clicked the button.");
      QPtr<iWidget> w = apCmd->GetSender();
      if (niIsOK(w)) {
        w->SetText(_H(niFmt("Click count: %d", ++_clickCount)));
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

    return eFalse;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
    apCanvas->BlitFill(
      mpWidget->GetWidgetRect(),
      0xFF252525);

    const tU32 blitTextFlags =
        0|
        eFontFormatFlags_Border|
        // eFontFormatFlags_CenterH|
        // eFontFormatFlags_Bottom|
        eFontFormatFlags_ClipH|
        eFontFormatFlags_ClipV
        ;
    const tU32 blitTextLoopCount = 1;
    const tF32 textWidth = mpWidget->GetSize().x/3;
    const tF32 textHeight = mpWidget->GetSize().y-200;

    apCanvas->BlitText(
      _monoFont,
      Rectf(20, 20,
            mpWidget->GetSize().x/3, mpWidget->GetSize().y-20),
      0, _kText);
    apCanvas->Flush();
    return eFalse;
  }
};

SCRIPTCPP_EXPORT_CLASS(UIWidget,HelloUI,HelloUIWidget);
