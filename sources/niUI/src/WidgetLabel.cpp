// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "API/niUI_ModuleDef.h"

struct cWidgetLabel : public ImplRC<iWidgetSink,eImplFlags_Default,iWidgetLabel>
{
  niBeginClass(cWidgetLabel);

  tU32 mFontFormatFlags;

  ///////////////////////////////////////////////
  cWidgetLabel() {
    mFontFormatFlags = eFontFormatFlags_CenterV;
  }

  ///////////////////////////////////////////////
  ~cWidgetLabel() {
  }

  ///////////////////////////////////////////////
  tBool __stdcall IsOK() const {
    niClassIsOK(cWidgetLabel);
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetFontFormatFlags(tFontFormatFlags aFlags) {
    mFontFormatFlags = aFlags;
  }
  virtual tFontFormatFlags __stdcall GetFontFormatFlags() const {
    return mFontFormatFlags;
  }

  ///////////////////////////////////////////////
  tBool __stdcall OnWidgetSink(iWidget *apWidget, tU32 nMsg, const Var& aA, const Var& aB)
  {
    niGuardObject((iWidgetSink*)this);
    switch (nMsg) {
      case eUIMessage_SkinChanged: {
        apWidget->SetFont(NULL);
        niFallthrough;
      }
      case eUIMessage_TextChanged: {
        niFallthrough;
      }
      case eUIMessage_Layout: {
        if (niFlagIs(aA.mU32,eWidgetAutoLayoutFlags_Size)) {
          tBool canComputeW = eTrue;
          tBool canComputeH = eTrue;
          switch (apWidget->GetDockStyle()) {
            case eWidgetDockStyle_DockLeft:
            case eWidgetDockStyle_DockRight:
            case eWidgetDockStyle_DockFillHeight:
              canComputeH = eFalse;
              break;
            case eWidgetDockStyle_DockTop:
            case eWidgetDockStyle_DockBottom:
            case eWidgetDockStyle_DockFillWidth:
              canComputeW = eFalse;
              break;
            case eWidgetDockStyle_DockFill:
            case eWidgetDockStyle_DockFillOverlay:
              canComputeW = eFalse;
              canComputeH = eFalse;
              break;
            default: break;
          }
          sRectf rect(0,0,0,0);
          rect = apWidget->GetFont()->ComputeTextSize(
              rect,niHStr(apWidget->GetLocalizedText()),
              mFontFormatFlags);
          const sVec2f newSize = {
            (canComputeW
             ? rect.GetWidth()+(apWidget->GetFont()->GetCharWidth(' ')/2.0f)
             : apWidget->GetSize().x),
            (canComputeH
             ? rect.GetHeight()
             : apWidget->GetSize().y)
          };
          apWidget->SetSize(newSize);
        }
        return eFalse;
      }

      case eUIMessage_Paint: {
        iCanvas* c = VarQueryInterface<iCanvas>(aB);
        if (c) {
          iFont* font = apWidget->GetFont();
          sRectf rect = apWidget->GetClientRect();
          rect.MoveTo(sVec2f::Zero());
          c->BlitText(
              font,
              rect,
              mFontFormatFlags,
              niHStr(apWidget->GetLocalizedText()));
        }
        return eTrue;
      }

      case eUIMessage_SerializeWidget: {
        QPtr<iDataTable> ptrDT = aA;
        if (ptrDT.IsOK()) {
          const tU32 nFlags = aB.mU32;
          if (nFlags & eWidgetSerializeFlags_Write) {
            ptrDT->SetEnum(_A("font_format_flags"),niFlagsExpr(eFontFormatFlags),mFontFormatFlags);
            if (nFlags & eSerializeFlags_TypeInfoMetadata) {
              ptrDT->SetMetadata(_A("font_format_flags"),_H("flags[*eFontFormatFlags]"));
            }
          }
          else if (nFlags & eWidgetSerializeFlags_Read) {
            mFontFormatFlags = ptrDT->GetEnumDefault("font_format_flags",niFlagsExpr(eFontFormatFlags),mFontFormatFlags);
            if (ptrDT->HasProperty("_data")) {
              apWidget->SetText(ptrDT->GetHString("_data"));
            }
          }
        }
        break;
      }
    }
    return eFalse;
  }

 private:
  niEndClass(cWidgetLabel);
};

ni::iWidgetSink* __stdcall New_WidgetLabel() {
  return niNew cWidgetLabel();
}
