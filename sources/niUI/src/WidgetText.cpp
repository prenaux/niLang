// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "API/niUI/IWidgetText.h"
#include <niLang/Utils/StrBreakIt.h>
#include <niLang/Utils/ConcurrentImpl.h>
#include <niUI/Utils/WidgetSinkImpl.h>
#include "API/niUI_ModuleDef.h"
#include "niUI_HString.h"

#if niMinFeatures(20)

#define TRACE_WIDGET_TEXT(X) // niDebugFmt(X)

class cWidgetText : public ImplRC<iWidgetSink,eImplFlags_Default,iWidgetText>
{
  niBeginClass(cWidgetText);

 public:
  Ptr<iTextObject> mptrTextObject;
  tU32 mnSelBegin, mnSelEnd;

  ///////////////////////////////////////////////
  tBool __stdcall IsOK() const {
    niClassIsOK(cWidgetText);
    return eTrue;
  }

  ///////////////////////////////////////////////
  iTextObject* __stdcall GetTextObject() const {
    return mptrTextObject;
  }

  ///////////////////////////////////////////////
  tBool __stdcall OnWidgetSink(iWidget *apWidget, tU32 nMsg, const Var& aA, const Var& aB)
  {
    niGuardObject((iWidgetSink*)this);
    switch (nMsg) {
      case eUIMessage_SinkAttached: {
        mptrTextObject = apWidget->GetGraphics()->CreateTextObject(
          niHStr(apWidget->GetText()),
          apWidget->GetSize(),
          apWidget->GetUIContext()->GetContentsScale());
        {
          WeakPtr<iWidget> _widget = apWidget;
          mptrTextObject->SetLoadFontCallback(
              ni::Callback1(
                  [_widget](iHString* ahspName) -> ni::iFont* {
                    ni::QPtr<ni::iWidget> w = _widget;
                    if (w.IsOK()) {
                      // niDebugFmt(("... LoadFontCallback: '%s'", ahspName));
                      return w->FindSkinFont(NULL, NULL, ahspName);
                    }
                    else {
                      return NULL;
                    }
                  }));
        }
        mnSelBegin = mnSelEnd = eInvalidHandle;
        return eTrue;
      }

      case eUIMessage_ChildAdded: {
        Ptr<iWidget> w = VarQueryInterface<iWidget>(aA);
        if (w.IsOK()) {
          TRACE_WIDGET_TEXT(("... WidgetText: ChildAdded: %s, %d", w->GetID(), w->GetNumRefs()));
          mptrTextObject->AddOccluder(
              sRectf::Null(),
              VarWeakPtr(w.ptr()));
          TRACE_WIDGET_TEXT(("... WidgetText: ChildAdded DONE: %s, %d", w->GetID(), w->GetNumRefs()));
        }
        break;
      }
      case eUIMessage_ChildRemoved: {
        Ptr<iWidget> w = VarQueryInterface<iWidget>(aA);
        if (w.IsOK()) {
          TRACE_WIDGET_TEXT(("... WidgetText: ChildRemoved: %s", w->GetID()));
          niLoop(i,mptrTextObject->GetNumOccluders()) {
            Ptr<iTextOccluder> occ = mptrTextObject->GetOccluder(i);
            Ptr<iWidget> occWidget = VarQueryInterface<iWidget>(occ->GetUserData());
            if (occWidget.ptr() == w.ptr()) {
              TRACE_WIDGET_TEXT(("... WidgetText: ChildRemoved: %s DONE", w->GetID()));
              break;
            }
          }
        }
        break;
      }

      case eUIMessage_Layout: {
        apWidget->ComputeAutoLayout(aA.mU32);
        TRACE_WIDGET_TEXT(("... WidgetText: Layout, occluders: %d", mptrTextObject->GetNumOccluders()));
        niLoop(i,mptrTextObject->GetNumOccluders()) {
          Ptr<iTextOccluder> occ = mptrTextObject->GetOccluder(i);
          Ptr<iWidget> w = VarQueryInterface<iWidget>(occ->GetUserData());
          if (w.IsOK()) {
            occ->SetRect(w->GetRect());
            TRACE_WIDGET_TEXT((
                "... WidgetText: Layout, occluder[%d]: id: %s, rect:%s",
                i, w->GetID(), occ->GetRect()));
          }
        }
        break;
      }

      case eUIMessage_Size: {
        if (mptrTextObject.IsOK()) {
          mptrTextObject->SetSize(apWidget->GetClientSize());
        }
        return eFalse;
      }

      case eUIMessage_SkinChanged: {
        apWidget->SetFont(NULL);
        niFallthrough;
      }
      case eUIMessage_FontChanged: {
        if (mptrTextObject.IsOK()) {
          mptrTextObject->SetDefaultFont(apWidget->GetFont());
        }
        break;
      }

      case eUIMessage_TextChanged: {
        if (mptrTextObject.IsOK()) {
          mptrTextObject->SetText(niHStr(apWidget->GetText()));
        }
        break;
      }
      case eUIMessage_SetText: {
        if (aB.mBool || !mptrTextObject.IsOK()) {
          apWidget->SetText(_H(""));
        }
        else {
          apWidget->SetText(_H(mptrTextObject->GetText()));
        }
        break;
      }

      case eUIMessage_Paint: {
        QPtr<iCanvas> c = VarQueryInterface<iCanvas>(aB);
        if (!c.IsOK())
          return eTrue;
        if (mptrTextObject.IsOK()) {
          mptrTextObject->Draw(c, apWidget->GetClippedRect());
        }
        return eTrue;
      }

      case eUIMessage_LeftClickDown: {
        if (mptrTextObject.IsOK()) {
          const sVec2f vMousePos = aA.GetVec2fValue();
          apWidget->SetCapture(eTrue);
          mptrTextObject->ClearSelection();
          mnSelBegin = mptrTextObject->FindWordIndexFromPosition(vMousePos);
          mnSelEnd = eInvalidHandle;
        }
        return eFalse;
      }

      case eUIMessage_NCLeftClickUp:
      case eUIMessage_LeftClickUp: {
        apWidget->SetCapture(eFalse);
        mnSelBegin = mnSelEnd = eInvalidHandle;
        return eFalse;
      }
      case eUIMessage_MouseMove: {
        if (niFlagIs(apWidget->GetStyle(),eWidgetTextStyle_MouseSelect) &&
            apWidget->GetCapture() &&
            mptrTextObject.IsOK() &&
            mnSelBegin != eInvalidHandle)
        {
          const sVec2f vMousePos = aA.GetVec2fValue();
          const tU32 wasSelEnd = mnSelEnd;
          mnSelEnd = mptrTextObject->FindWordIndexFromPosition(vMousePos);
          if (mnSelEnd != wasSelEnd) {
            mptrTextObject->SelectRange(mnSelBegin, mnSelEnd);
          }
        }
        return eFalse;
      }

      case eUIMessage_Copy: {
        QPtr<ni::iDataTable> ptrDT = aA;
        if (ptrDT.IsOK() && mptrTextObject.IsOK()) {
          cString strText = mptrTextObject->GetSelectedString();
          if (!strText.empty()) {
            ptrDT->SetString("text",strText.Chars());
          }
        }
        break;
      }

      case eUIMessage_SerializeWidget: {
        QPtr<iDataTable> ptrDT = aA;
        if (ptrDT.IsOK() && mptrTextObject.IsOK()) {
          const tU32 nFlags = aB.mU32;
          if (nFlags & eWidgetSerializeFlags_Write) {
            ptrDT->SetBool("trim_leading_spaces",mptrTextObject->GetTrimLeadingSpaces());
            ptrDT->SetBool("kerning",mptrTextObject->GetKerning());
            ptrDT->SetEnum("truncation",niEnumExpr(eTextTruncation),mptrTextObject->GetTruncation());
            ptrDT->SetString("truncation_text",mptrTextObject->GetTruncationText());
          }
          else if (nFlags & eWidgetSerializeFlags_Read) {
            if (ptrDT->HasProperty("_data")) {
              apWidget->SetText(ptrDT->GetHString("_data"));
            }
            mptrTextObject->SetTrimLeadingSpaces(ptrDT->GetBoolDefault("trim_leading_spaces",mptrTextObject->GetTrimLeadingSpaces()));
            mptrTextObject->SetKerning(ptrDT->GetBoolDefault("kerning",mptrTextObject->GetKerning()));
            mptrTextObject->SetTruncation((eTextTruncation)ptrDT->GetEnumDefault("truncation",niEnumExpr(eTextTruncation),mptrTextObject->GetTruncation()));
            mptrTextObject->SetTruncationText(ptrDT->GetStringDefault("truncation_text",mptrTextObject->GetTruncationText()).Chars());
          }
        }
        break;
      }

    }
    return eFalse;
  }

  niEndClass(cWidgetText);
};

ni::iWidgetSink* __stdcall New_WidgetText() {
  return niNew cWidgetText();
}

#endif
