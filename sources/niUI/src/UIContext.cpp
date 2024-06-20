// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Graphics.h"
#include "UIContext.h"
#include "Widget.h"
#include "WidgetCommand.h"
#include <niLang/STL/utils.h>
#include <niLang/IProf.h>
#include <niLang/Math/MathMatrix.h>
#include <niLang/Utils/UnitSnap.h>
#include "niUI_HString.h"

#ifdef niWindows
#include <niLang/Platforms/Win32/Win32_DelayLoadImpl.h>
#endif

#define RAW_VIEWPORT 1
#define RAW_VIEWPORT_SCISSOR 2

static void _ApplyViewport(iGraphicsContext* apContext,
                           const sRectf& aViewport,
                           const sRecti& aScissor,
                           const tF32 afContentsScale)
{
  const sRecti rectRT = Recti(0,0,apContext->GetWidth(),apContext->GetHeight());
  sRecti vp = ((sRectf)(aViewport * afContentsScale)).ToInt().ClipRect(rectRT);
  apContext->SetViewport(vp);

  if (aScissor != sRecti::Null()) {
    sRecti sc = sRectf(aScissor.ToFloat() * afContentsScale).ToInt().ClipRect(rectRT);
    apContext->SetScissorRect(sc);
  }
  else {
    apContext->SetScissorRect(vp);
  }
  // niDebugFmt(("rectRT: %s, aViewport %s, afContentsScale: %s, scissor: %s",rectRT,vp,afContentsScale,apContext->GetScissorRect()));

  sRectf orthoRect;
  orthoRect = sRectf(
      0,
      0,
      apContext->GetRenderTarget(0)->GetWidth(),
      apContext->GetRenderTarget(0)->GetHeight());

  iFixedStates* fs = apContext->GetFixedStates();

  sMatrixf viewportMatrix = sMatrixf::Identity();
  MatrixTranslation(viewportMatrix,Vec3f(aViewport.x,aViewport.y,0)*(afContentsScale - 1));
  MatrixScale(viewportMatrix,viewportMatrix,Vec3f(afContentsScale,afContentsScale,1));
  fs->SetCameraViewMatrix(viewportMatrix);

  // This sets the projection matrix that we use in the next
  // SetCameraProjectionMatrix...
  const tF32 fOrthoProjectionOffset = ultof(
    apContext->GetGraphics()->GetDriver()->GetCaps(eGraphicsCaps_OrthoProjectionOffset));
  fs->SetCameraProjectionMatrix(MatrixProjection2D(fOrthoProjectionOffset,orthoRect,0.0f));

  // Adjust the projection matrix to fit in the clipped viewport
  sMatrixf tmpMtx;
  orthoRect.MoveTo(aViewport.GetTopLeft());
  fs->SetCameraProjectionMatrix(
      fs->GetCameraProjectionMatrix()*
      MatrixAdjustViewport(tmpMtx,
                           apContext->GetViewport().ToFloat(),
                           orthoRect,0.0f,1.0f));
}

static inline void _ApplyTransformedScissor(iCanvas* apCanvas, const sRecti& aScissor, const sMatrixf& aBaseMatrix) {
  sRectf fsc = aScissor.ToFloat();
  sVec2f tl, br;
  VecTransformCoord(tl, Vec2f(aScissor.x, -aScissor.y), aBaseMatrix);
  VecTransformCoord(br, Vec2f(aScissor.z, -aScissor.w), aBaseMatrix);
  Ptr<iGraphicsContext> gc = apCanvas->GetGraphicsContext();
  sMatrixf vp = gc->GetFixedStates()->GetCameraViewProjectionMatrix();
  sRectf r = apCanvas->GetViewport();

  ni::sVec4f v1, v2;
  ni::VecTransform(v1,tl,vp);
  ni::VecTransform(v2,br,vp);
  tF32 fVPW2 = r.GetWidth() * 0.5f;
  tF32 fVPH2 = r.GetHeight() * 0.5f;

  tF32 RHW1 = ni::FInvert(v1.w);
  v1.x = (v1.x*RHW1)*fVPW2 + (r.x+fVPW2);
  v1.y = r.w- (1 + v1.y*RHW1)*fVPH2;

  tF32 RHW2 = ni::FInvert(v2.w);
  v2.x = (v2.x*RHW2)*fVPW2 + (r.x+fVPW2);
  v2.y = r.w- (1 + v2.y*RHW2)*fVPH2;

  gc->SetScissorRect(Vec4i(v1.x, v1.y, v2.x,v2.y));
}


static void _ApplyWidgetViewport(iCanvas* apCanvas, const sRectf& aViewport, const sRecti& aScissor, tU32 anRawViewport, const sMatrixf& aBaseMatrix, const tF32 afContentsScale) {
  apCanvas->ResetStates();
  if (anRawViewport) {
    sMatrixf viewportMatrix = sMatrixf::Identity();
    MatrixTranslation(viewportMatrix,Vec3f(aViewport.x,-aViewport.y,0));
    MatrixScale(viewportMatrix,viewportMatrix,Vec3f(1,-1,1));
    apCanvas->SetMatrix(viewportMatrix * aBaseMatrix);
    if (anRawViewport == RAW_VIEWPORT_SCISSOR) {
      _ApplyTransformedScissor(apCanvas, aScissor, aBaseMatrix);
    }
  }
  else {
    apCanvas->SetContentsScale(afContentsScale);
    _ApplyViewport(apCanvas->GetGraphicsContext(),
                   aViewport,
                   aScissor,
                   afContentsScale);
  }
}

static const tU32 kcolorDummy = ULColorBuildf(1,0.5f,0,1);
static const tU32 kcolorFocus = ULColorBuildf(1,1,0,1);
static const tU32 kcolorNormal = ULColorBuildf(0,1,0,1);
static const tU32 kcolorClient = ULColorBuildf(1,0,1,0.5);
static const tU32 kcolorDesktop = ULColorBuildf(0,1,1,1);

///////////////////////////////////////////////
static inline tBool __stdcall _IsThisOrChild(iWidget* apWidget, iWidget* apToFind)
{
  if (!apWidget)
    return eFalse;
  if (apWidget == apToFind)
    return eTrue;
  niLoop(i,apWidget->GetNumChildren()) {
    if (_IsThisOrChild(apWidget->GetChildFromIndex(i),apToFind))
      return eTrue;
  }
  return eFalse;
}

struct sRootSink : public ImplRC<iWidgetSink> {
  tBool __stdcall OnWidgetSink(iWidget *apWidget, tU32 nMsg, const Var& varParam0, const Var& varParam1) {
    switch (nMsg) {
      case eUIMessage_SerializeWidget:
        {
          Ptr<iDataTable> ptrDT = VarQueryInterface<iDataTable>(varParam0);
          tU32 nFlags = varParam1.mU32;
          if (niFlagIs(nFlags,eWidgetSerializeFlags_Write)) {
            ptrDT->RemoveProperty("style");
            ptrDT->RemoveProperty("zorder");
            ptrDT->RemoveProperty("position");
            ptrDT->RemoveProperty("size");
            ptrDT->RemoveProperty("minimum_size");
            ptrDT->RemoveProperty("maximum_size");
            ptrDT->RemoveProperty("exclusive");
            ptrDT->RemoveProperty("visible");
            ptrDT->RemoveProperty("enabled");
            ptrDT->RemoveProperty("ignore_input");
            ptrDT->RemoveProperty("dock_style");
          }
          return eFalse;
        }
      case eUIMessage_SkinChanged:
        {
          cUIContext* pCtx = niStaticCast(cUIContext*,apWidget->GetUIContext());
          pCtx->mptrMouseCursor[eUIStandardCursor_Invalid] = apWidget->FindSkinCursor(NULL,NULL,_H("Invalid"));
          pCtx->mptrMouseCursor[eUIStandardCursor_DraggingMove] = apWidget->FindSkinCursor(NULL,NULL,_H("DraggingMove"));
          pCtx->mptrMouseCursor[eUIStandardCursor_DraggingCopy] = apWidget->FindSkinCursor(NULL,NULL,_H("DraggingCopy"));
          pCtx->mptrMouseCursor[eUIStandardCursor_DraggingInvalid] = apWidget->FindSkinCursor(NULL,NULL,_H("DraggingInvalid"));
          pCtx->mptrMouseCursor[eUIStandardCursor_Arrow] = apWidget->FindSkinCursor(NULL,NULL,_H("Arrow"));
          pCtx->SetCursor(pCtx->mptrMouseCursor[eUIStandardCursor_Arrow]);
          return eFalse;
        }
      case eUIMessage_Size:
        {
          cUIContext* pCtx = niStaticCast(cUIContext*,apWidget->GetUIContext());
          apWidget->ComputeAutoLayout(eWidgetAutoLayoutFlags_Relative|
                                      eWidgetAutoLayoutFlags_Dock);
          pCtx->GetRootWidget()->BroadcastMessage(eUIMessage_ContextResized,niVarNull,niVarNull);
          return eFalse;
        }
      case eWidgetDockingManagerMessage_BeginMove:
      case eWidgetDockingManagerMessage_EndMove:
      case eWidgetDockingManagerMessage_Move:
        {
          Ptr<iWidget> wDesktop = apWidget->FindWidget(_HC(ID_Desktop));
          if (niIsOK(wDesktop)) {
            wDesktop->SendMessage(nMsg,varParam0,varParam1);
          }
          break;
        }
      case eUIMessage_ContextAfterDraw: {
        cUIContext* pCtx = niStaticCast(cUIContext*,apWidget->GetUIContext());
        // Debug draw
        if (pCtx->mbDebugDraw && apWidget->GetFont())
        {
          Ptr<iCanvas> rootCanvas = VarQueryInterface<iCanvas>(varParam0);
          if (niIsOK(rootCanvas)) {
            Ptr<iFont> pFont = apWidget->GetFont()->CreateFontInstance(NULL);
            pFont->SetSizeAndResolution(sVec2f::Zero(),16,apWidget->GetUIContext()->GetContentsScale());
            pFont->SetBlendMode(eBlendMode_Translucent);

            {
              QPtr<iWidgetDockingManager> ptrDM = apWidget->FindWidget(_HC(ID_Desktop));
              if (ptrDM.IsOK()) {
                pFont->SetColor(kcolorDesktop);

                tU32 nNumDockAreas = ptrDM->GetNumDockAreas();
                for (tU32 i = 0; i < nNumDockAreas; ++i) {
                  Ptr<iWidget> ptrW = ptrDM->GetDockArea(i);
                  sRectf rectAbs = ptrW->GetAbsoluteRect();

                  rootCanvas->BlitRect(rectAbs,kcolorDesktop);

                  sVec2f pos = rectAbs.GetTopLeft();
                  rootCanvas->BlitText(
                      pFont,
                      sRectf(pos),
                      eFontFormatFlags_Border|eFontFormatFlags_CenterH,
                      niFmt(_A("ID:%s,ZO:%d,DO:%d (%s)"),
                            niHStr(ptrW->GetID()),
                            ptrW->GetZOrder(),
                            ptrW->GetDrawOrder(),
                            niHStr(ptrW->GetClassName())));

                }
              }
            }

            {
              Ptr<iWidget> ptrMT = pCtx->GetMouseMessageTarget(eFalse);
              QPtr<cWidget> ptrFT(pCtx->GetInputMessageTarget());
              if (niIsOK(ptrMT) || niIsOK(ptrFT)) {
                sRectf rectAbs, rectClient;
                sVec2f pos;

                {
                  struct _Local {
                    static void _DrawDummy(iCanvas* c, cWidget* w) {
                      if (!w->GetVisible())
                        return;
                      if (w->GetClassName() == _HC(Dummy)) {
                        c->BlitRect(w->GetAbsoluteRect(),kcolorDummy);
                      }
                      niLoop(i,w->mvecClipChildren.size()) {
                        _DrawDummy(c,w->mvecClipChildren[i]);
                      }
                    }
                  };
                  _Local::_DrawDummy(rootCanvas, (cWidget*)apWidget);
                }

                // Input message target
                if (niIsOK(ptrFT) && ptrMT.ptr() != ptrFT.ptr()) {
                  rectAbs = ptrFT->GetAbsoluteRect();
                  pos = rectAbs.GetTopLeft();
                  rectClient = ptrFT->GetClientRect()+pos;
                  pFont->SetColor(kcolorFocus);

                  rootCanvas->BlitRect(rectAbs,kcolorFocus);
                  rootCanvas->BlitRect(rectClient,kcolorClient);

                  rootCanvas->BlitText(
                      pFont,
                      sRectf(pos),
                      eFontFormatFlags_Border|0,
                      niFmt(_A("(IT) ID:%s.ZO:%d,DO:%d (%s)"),
                            niHStr(ptrFT->GetID()),
                            ptrFT->GetZOrder(),
                            ptrFT->GetDrawOrder(),
                            niHStr(ptrFT->GetClassName())));
                }

                // Mouse message target
                if (ptrMT.IsOK()) {
                  rectAbs = ptrMT->GetAbsoluteRect();
                  pos = rectAbs.GetTopLeft();
                  rectClient = ptrMT->GetClientRect()+pos;
                  tU32 color = ptrMT->GetHasFocus() ? kcolorFocus : kcolorNormal;
                  pFont->SetColor(color);

                  rootCanvas->BlitRect(rectAbs,color);
                  rootCanvas->BlitRect(rectClient,kcolorClient);

                  rootCanvas->BlitText(
                      pFont,sRectf(pos),
                      eFontFormatFlags_Border|0,
                      niFmt(_A("(MT%s) ID:%s.ZO:%d,DO:%d (%s)"),
                            (ptrFT.ptr()==ptrMT.ptr()) ? _A("/IT"):_A(""),
                            niHStr(ptrMT->GetID()),
                            ptrMT->GetZOrder(),
                            ptrMT->GetDrawOrder(),
                            niHStr(ptrMT->GetClassName())));

                  pos = rectAbs.GetBottomRight();
                  rootCanvas->BlitText(
                      pFont,sRectf(pos),
                      eFontFormatFlags_Bottom|eFontFormatFlags_Right,
                      niFmt(_A("%s (w:%g, h:%g)"),
                            rectAbs, rectAbs.GetWidth(), rectAbs.GetHeight()));
                }
              }
            }
          }
        }
        return eFalse;
      }
      default:
        return eFalse;
    }
    return eTrue;
  }
};

void _RegisterStandardWidgets();

///////////////////////////////////////////////
cUIContext::cUIContext(iGraphicsContext* apGraphicsContext, iHString* ahspDefaultSkinPath, tF32 afContentsScale)
{
  niGuardConstructor(cUIContext);

  // Register all enumerations so that the expression parser for the "style"
  // property with GlobalSearch can find the enumerations for
  // eWidgetButtonStyle, etc...
  static bool wasExecuted = false;
  if (!wasExecuted) {
    GetLang()->RegisterModuleDef(GetModuleDef_niUI());
  }
  wasExecuted = true;

  if (!niIsOK(apGraphicsContext)) {
    niError(_A("Invalid graphics context."));
    return;
  }

  mfContentsScale = afContentsScale;
  mnInputModifiers = 0;
  memset(mKeyIsDown,0,sizeof(mKeyIsDown));
  mKeyEatChar = eKey_Unknown;
  mnDefaultInputSubmitFlags = eUIInputSubmitFlags_Default;
  mbDebugDraw = eFalse;
  mbDrawOpCapture = eFalse;
  mbDrawOpCaptureDrawn = eFalse;
#ifdef niEmbedded
  mnLastPrimaryFinger = 0;
#else
  mnLastPrimaryFinger = FINGER_POINTER_BUTTON_TO_ID(ePointerButton_Left);
#endif
  mfHoverDelay = 0.5f;

  mptrGraphicsContext = apGraphicsContext;
  mptrGraphics = apGraphicsContext->GetGraphics();

  // create the default skin
  {
    mptrErrorSkin = ni::CreateDataTable(_A("UISkin"));
    SetErrorOverlay(NULL);

    if (GetSkinIndex(_H("Default")) != eInvalidHandle) {
      mhspDefaultSkin = _H("Default");
    }
    else if (HStringIsNotEmpty(ahspDefaultSkinPath) && AddSkinFromRes(ahspDefaultSkinPath)) {
      // initialized the skin, nothing else to do
    }
    else if (!AddSkinFromRes(_H("niUI://skins/default.uiskin.xml"))) {
      niError("Can't add the default skin.");
    }
    mhspDefaultSkin = _H(GetSkinDataTable(GetSkinName(0))->GetString(_A("name")));
  }

  _RegisterStandardWidgets();

  mpwRootWidget = niNew cWidget(this,_H("RootWidget"),NULL,sRectf(0,0,100,100),
                                0,_H("ID_RootWidget"),NULL,NULL);
  mpwRootWidget->AddSink(niNew sRootSink());
  mpwRootWidget->SetSkin(this->mhspDefaultSkin);
  mpwRootWidget->SetFont(mpwRootWidget->FindSkinFont(NULL,NULL,_H("Default")));
  {
    Ptr<iCanvas> ptrCanvas = mptrGraphics->CreateCanvas(mptrGraphicsContext.ptr(),NULL);
    mpwRootWidget->SetCanvas(ptrCanvas);
  }

  SetTopWidget(mpwRootWidget);

  // niDebugFmt(("... Construct UIContext: %p", (tIntPtr)this));
}

///////////////////////////////////////////////
cUIContext::~cUIContext()
{
  Invalidate();
}

///////////////////////////////////////////////
tBool __stdcall cUIContext::IsOK() const
{
  return mpwRootWidget.IsOK();
}

///////////////////////////////////////////////
void __stdcall cUIContext::Invalidate()
{
  if (niIsNullPtr(mpwRootWidget))
    return;

  _WindowUpdateCursorStates();
#if niMinFeatures(20)
  mptrToolbar = NULL;
#endif

  mpwRootWidget->Invalidate();
  mpwRootWidget = NULL;

  // niAssert(mFreeWidgets.zmapInput.IsEmpty());
  if (!mFreeWidgets.zmapInput.IsEmpty()) {
    for (tWidgetZMapRIt zit = mFreeWidgets.zmapInput.RBegin(); zit != mFreeWidgets.zmapInput.REnd(); ++zit) {
      for (tWidgetPtrDeqIt lit = zit->second.begin(); lit != zit->second.end(); ++lit) {
        Ptr<cWidget> w = *lit;
        niWarning(niFmt(_A("Destroying zmap input widget '%s' (%s) (%p)."),
                        w->GetID(), w->GetClassName(), (tIntPtr)w.ptr()));
        w->Destroy();
      }
    }
  }

  // niAssert(mFreeWidgets.zmapDraw.IsEmpty());
  if (!mFreeWidgets.zmapDraw.IsEmpty()) {
    for (tWidgetZMapRIt zit = mFreeWidgets.zmapDraw.RBegin(); zit != mFreeWidgets.zmapDraw.REnd(); ++zit) {
      for (tWidgetPtrDeqIt lit = zit->second.begin(); lit != zit->second.end(); ++lit) {
        Ptr<cWidget> w = *lit;
        niWarning(niFmt(_A("Destroying draw widget '%s' (%s) (%p)."),
                        w->GetID(), w->GetClassName(), (tIntPtr)w.ptr()));
        w->Destroy();
      }
    }
  }

  mFreeWidgets.Clear();

  // niAssert(mlstWidgets.empty());
  if (!mlstWidgets.empty()) {
    niWarning(niFmt("'%d' garbage widget(s) found.", mlstWidgets.size()));
    astl::reverse(mlstWidgets.begin(),mlstWidgets.end());
    niLoop(i,mlstWidgets.size()) {
      QPtr<cWidget> w(mlstWidgets[i]);
      if (niIsOK(w)) {
        niWarning(niFmt("Destroying garbage widget '%s' (%s) (%p).",
                        w->GetID(), w->GetClassName(), (tIntPtr)w.ptr()));
        w->Invalidate();
      }
      else {
        niAssertUnreachable("Widget shouldnt be in the list if its invalid.");
      }
    }
  }
}

///////////////////////////////////////////////
iGraphicsContext* __stdcall cUIContext::GetGraphicsContext() const
{
  return mptrGraphicsContext;
}

///////////////////////////////////////////////
iWidget* __stdcall cUIContext::GetRootWidget() const
{
  return mpwRootWidget;
}

///////////////////////////////////////////////
void __stdcall cUIContext::SetDefaultInputSubmitFlags(tUIInputSubmitFlags aSubmitFlags)
{
  mnDefaultInputSubmitFlags = aSubmitFlags;
}

///////////////////////////////////////////////
tUIInputSubmitFlags __stdcall cUIContext::GetDefaultInputSubmitFlags() const
{
  return mnDefaultInputSubmitFlags;
}

///////////////////////////////////////////////
iWidget* cUIContext::CreateWidget(const achar *aszClassName, iWidget *apwParent, const sRectf &arectPos, tU32 anStyle, iHString* ahspID)
{
  if (this->HasWidgetSinkClass(aszClassName))
  {
    if (!apwParent)
      apwParent = mpwRootWidget;

    Ptr<cWidget> ret = niNew cWidget(this,_H(aszClassName),(cWidget*)apwParent,arectPos,anStyle,ahspID,NULL,NULL);
    niCheck(ret.IsOK(),NULL);
    // ret->SetSkin(this->mhspDefaultSkin);
    ret->SetFont(ret->FindSkinFont(NULL,NULL,_H("Default")));

    {
      Ptr<iWidgetSink> sink = this->CreateWidgetSink(aszClassName,ret);
      if (niIsOK(sink)) {
        ret->AddSink(sink);
      }
    }

    if (apwParent)
      ((cWidget*)apwParent)->AddChild(ret);

    return ret.GetRawAndSetNull();
  }

  niError(niFmt(_A("Can't find widget class '%s'."),aszClassName));
  return NULL;
}

///////////////////////////////////////////////
iWidget *cUIContext::CreateWidgetRaw(const achar *aszClassName, iWidget *apwParent, const sRectf &arectPos, tU32 anStyle, iHString* ahspID)
{
  Ptr<cWidget> ret;
  if (!apwParent)
    apwParent = mpwRootWidget;
  ret = niNew cWidget(this,_H(aszClassName),(cWidget*)apwParent,arectPos,anStyle,ahspID,NULL,NULL);
  niCheck(ret.IsOK(),NULL);
  ret->SetFont(ret->FindSkinFont(NULL,NULL,_H("Default")));
  if (apwParent) ((cWidget*)apwParent)->AddChild(ret);
  return ret.GetRawAndSetNull();
}


///////////////////////////////////////////////
iWidget* __stdcall cUIContext::CreateWidgetFromDataTable(iDataTable* apDT, iWidget* apwParent, iHString* ahspID, iHString* ahspTitle)
{
  niCheckIsOK(apDT,NULL);

  Ptr<iDataTable> dt = apDT;
  if (HStringIsNotEmpty(ahspID) || HStringIsNotEmpty(ahspTitle)) {
    dt = dt->Clone();
    if (HStringIsNotEmpty(ahspID)) {
      dt->SetString(_A("id"),niHStr(ahspID));
    }
    if (HStringIsNotEmpty(ahspTitle)) {
      dt->SetString(_A("title"),niHStr(ahspTitle));
    }
  }

  ni::tBool bIsWidgetTable = ni::StrICmp(_A("Widget"),apDT->GetName()) == 0;

  ni::tU32 nStyle = 0;
  tHStringPtr hspID = _H(dt->GetString(_A("id")));
  cString strClass = dt->GetString(_A("class"));
  if (strClass.IsEmpty()) {
    if (!bIsWidgetTable) {
      strClass = apDT->GetName();
    }
    else if (niIsOK(apwParent)) {
      strClass = _A("Canvas");
    }
    else {
      strClass = _A("Form");
    }
  }
  if (strClass == _A("Form")) {
    nStyle = eWidgetFormStyle_DefaultButtons|eWidgetStyle_HoldFocus|eWidgetStyle_FocusActivate;
  }

  Ptr<iWidget> newWidget = CreateWidget(strClass.Chars(),
                                        niIsOK(apwParent)?apwParent:NULL,
                                        sRectf(0,0,100,100),
                                        nStyle,
                                        hspID);
  niCheck(newWidget.IsOK(),NULL);

  const tU32 serializeFlags = eWidgetSerializeFlags_Read|
      (bIsWidgetTable?0:eWidgetSerializeFlags_NoRoot)|
      eWidgetSerializeFlags_Children;

  if (!SerializeWidget(newWidget,dt,serializeFlags,NULL)) {
    niError(_A("Can't read widget datatable."));
    return NULL;
  }

  return newWidget.GetRawAndSetNull();
}

///////////////////////////////////////////////
iWidget* __stdcall cUIContext::CreateWidgetFromResource(iHString* ahspRes, iWidget* apwParent, iHString* ahspID, iHString* ahspTitle)
{
  ni::Ptr<ni::iFile> ptrFP = ni::GetLang()->URLOpen(niHStr(ahspRes));
  if (!niIsOK(ptrFP)) {
    niError(niFmt(_A("Can't open the datatable file '%s'."),niHStr(ahspRes)));
    return NULL;
  }

  ni::Ptr<iDataTable> dt = ni::CreateDataTable(_A("Widget"));
  if (!SerializeDataTable(NULL,eSerializeMode_Read,dt,ptrFP)) {
    niError(niFmt(_A("Can't read a datatable from file '%s'."),niHStr(ahspRes)));
    return NULL;
  }

  ni::Ptr<iWidget> newWidget = CreateWidgetFromDataTable(dt,apwParent,ahspID,ahspTitle);
  if (!newWidget.IsOK()) {
    niError(niFmt(_A("Can't create widget from file '%s'."),niHStr(ahspRes)));
    return NULL;
  }

  return newWidget.GetRawAndSetNull();
}

///////////////////////////////////////////////
tBool cUIContext::Update(tF32 afFrameTime)
{
  niProf(UIContext_Update);
  niAssert(afFrameTime >= 0.0f && afFrameTime <= 1000.0f);

  // process timers
  UpdateTimers(afFrameTime);

  // update layout
  tU32 numLayoutPass = 0;
  while (mnRelayoutCount && numLayoutPass < 5) {
    ++numLayoutPass;
    if (PRINT_RELAYOUT) {
      niDebugFmt(("W/Layout Pass[%d]: %d",numLayoutPass,mnRelayoutCount));
    }
    mnRelayoutCount = 0;
    mpwRootWidget->Layout(eTrue);
  }

  // send context update message
  if (mpwRootWidget.IsOK()) {
    mpwRootWidget->SendMessage(eUIMessage_ContextUpdate,afFrameTime,niVarNull);
  }

  return eTrue;
}

///////////////////////////////////////////////
tBool cUIContext::Resize(const sRectf& aRootRect, const tF32 afContentsScale)
{
  if (!mpwRootWidget.IsOK())
    return eFalse;

  // deskrect is in UI units, aRootRect is in screen device (pixels) units
  const sRectf deskrect = ni::UnitSnapf(aRootRect / afContentsScale);
  if (deskrect.GetWidth() < 10 ||
      deskrect.GetHeight() < 10 ||
      deskrect.GetWidth() > 100000 ||
      deskrect.GetHeight() > 100000)
    return eFalse; // if not a sane size, we don't update the screen rect

  const tF32 prevContentsScale = mfContentsScale;
  const tF32 newContentsScale = ni::Clamp(afContentsScale,0.1f,10.0f);
  const sVec2f curSize = mpwRootWidget->GetSize();
  const sVec2f newSize = deskrect.GetSize();
  if (curSize == newSize && prevContentsScale == newContentsScale)
    return eTrue;

  mfContentsScale = newContentsScale;

  mpwRootWidget->SetSize(newSize+sVec2f::One());

  // sVec2f toNewSize = newSize/curSize;
  mpwRootWidget->SetSize(newSize);
  _WindowUpdateCursorStates();

  return eFalse;
}

///////////////////////////////////////////////
tF32 __stdcall cUIContext::GetContentsScale() const {
  return mfContentsScale;
}

///////////////////////////////////////////////
void cUIContext::Draw()
{
  niProf(UIContext_Draw);

  if (!mpwRootWidget.IsOK())
    return;

  Ptr<iCanvas> rootCanvas = mpwRootWidget->GetCanvas();
  niAssert(rootCanvas.IsOK());
  if (!rootCanvas.IsOK())
    return;

  sMatrixf transformMatrix = sMatrixf::Identity();
  MatrixScaling(transformMatrix, mfContentsScale, mfContentsScale, 1.f);

  const sRectf rectContext(
      0,0,
      rootCanvas->GetGraphicsContext()->GetRenderTarget(0)->GetWidth(),
      rootCanvas->GetGraphicsContext()->GetRenderTarget(0)->GetHeight());

  // Draw Op Capture
  mbDrawOpCaptureDrawn = eFalse;
  Ptr<cWidget> drawOpCaptureHUD;
  astl::vector<Ptr<cWidget> > drawOpCaptureHUDToDraw;
  if (mbDrawOpCapture) {
    iGraphicsDrawOpCapture* capture = mptrGraphics->GetDrawOpCapture();
    if (capture) {
      drawOpCaptureHUD = (cWidget*)mpwRootWidget->GetChildFromID(_HC(__ID_DrawOpCaptureHUD__));
      capture->ClearCapture();
      capture->BeginCapture();
      drawOpCaptureHUDToDraw.push_back(drawOpCaptureHUD);
    }
  }

  // Before Draw
  if (mpwRootWidget.IsOK()) {
    _ApplyWidgetViewport(rootCanvas,
                         mpwRootWidget->GetAbsoluteRect(),
                         mpwRootWidget->GetAbsoluteRect().ToInt(),
                         eFalse,transformMatrix,
                         mfContentsScale);
    mpwRootWidget->SendMessage(eUIMessage_ContextBeforeDraw,rootCanvas.ptr(),niVarNull);
    rootCanvas->Flush();
  }

  // Draw the widgets
  {
    _DrawWidget(mpwRootWidget,rootCanvas,rectContext,eFalse,transformMatrix);
    if (!mFreeWidgets.zmapDraw.IsEmpty()) {
      for (tWidgetZMapRIt zit = mFreeWidgets.zmapDraw.RBegin();
           zit != mFreeWidgets.zmapDraw.REnd(); ++zit)
      {
        for (tWidgetPtrDeqIt lit = zit->second.begin(); lit != zit->second.end(); ++lit) {
          Ptr<cWidget> w = *lit;
          if (_IsThisOrChild(drawOpCaptureHUD,w)) {
            drawOpCaptureHUDToDraw.push_back(w);
            continue;
          }
          if (w->GetVisible() && w != mpwRootWidget && w->GetVisible()) {
            _DrawWidget(w,rootCanvas,rectContext,eFalse,transformMatrix);
          }
        }
      }
    }
  }

  // After Draw
  if (mpwRootWidget.IsOK()) {
    _ApplyWidgetViewport(rootCanvas,
                         mpwRootWidget->GetAbsoluteRect(),
                         mpwRootWidget->GetAbsoluteRect().ToInt(),
                         eFalse,transformMatrix,
                         mfContentsScale);
    mpwRootWidget->SendMessage(eUIMessage_ContextAfterDraw,rootCanvas.ptr(),niVarNull);
  }
  rootCanvas->Flush();

  // Draw Op Capture
  if (mbDrawOpCapture) {
    iGraphicsDrawOpCapture* capture = mptrGraphics->GetDrawOpCapture();
    if (capture) {
      capture->EndCapture();
      if (niIsOK(drawOpCaptureHUD)) {
        drawOpCaptureHUD->SetVisible(eTrue);
        // Draw the active RT
        {
          tU32 index = capture->GetCaptureStopAt();
          Ptr<iGraphicsContext> cc = capture->GetCapturedDrawOpContext(index);
          if (cc.IsOK()) {
            Ptr<iTexture> rt = cc->GetRenderTarget(0);
            if (rt != rootCanvas->GetGraphicsContext()->GetRenderTarget(0)) {
              Ptr<iFont> font = mpwRootWidget->GetFont();
              const tF32 h = niRound(mpwRootWidget->GetSize().y / 2.0f);
              const tF32 rtRatio = ni::FDiv((tF32)rt->GetWidth(), (tF32)rt->GetHeight());
              sRectf rect = Rectf(10,10,(h*rtRatio)+4,h+4);
              rootCanvas->SetColorA(0xFFFFFFFF);
              rootCanvas->BlitFill(rect,~0);
              rootCanvas->SetDefaultMaterial(rt,eBlendMode_NoBlending,eCompiledStates_SS_SmoothClamp);
              rect.Inflate(Vec2f(-4,-4));
              rect.Move(Vec2f(2,2));
              rootCanvas->Rect(rect.GetTopLeft(),rect.GetBottomRight(),0);
              rootCanvas->BlitText(
                font,
                Rectf(rect.GetLeft(),rect.GetBottom()+3,0,0),
                eFontFormatFlags_Border,
                niFmt("%s, %dx%d, %s",
                      niHStr(rt->GetDeviceResourceName()),
                      rt->GetWidth(), rt->GetHeight(),
                      rt->GetPixelFormat()->GetFormat()));
              rootCanvas->Flush();
            }
          }
        }
        // Draw the HUD's widgets
        niLoop(i,drawOpCaptureHUDToDraw.size()) {
          _DrawWidget(drawOpCaptureHUDToDraw[i],rootCanvas,rectContext,eFalse,transformMatrix);
        }
        mbDrawOpCaptureDrawn = eTrue;
      }
      rootCanvas->Flush();
    }
  }
}

///////////////////////////////////////////////
void __stdcall cUIContext::DrawCursor(iOSWindow* apWindow) {
  Ptr<iOSWindow> ptrOSWindow = apWindow;

  Ptr<iOverlay> ptrCursor = mptrMouseCursor[eUIStandardCursor_Current];
  if (!ptrCursor.IsOK()) {
    if (ptrOSWindow.IsOK())
      ptrOSWindow->SetCursor(eOSCursor_None);
    return;
  }

  // Try to set a hardware cursor....
  if (_UpdateOSCursor(apWindow, ptrCursor)) {
    return;
  }
  else
  {
    const sVec2f& mp = GetMousePos();
    Ptr<iCanvas> rootCanvas = mpwRootWidget->GetCanvas();
    rootCanvas->BlitOverlay(
      sRectf(mp.x,mp.y,
             ptrCursor->GetSize().x,
             ptrCursor->GetSize().y),
      ptrCursor);
    rootCanvas->Flush();
    if (ptrOSWindow.IsOK()) {
      ptrOSWindow->SetCursor(eOSCursor_None);
    }
  }
}

///////////////////////////////////////////////
tBool cUIContext::_UpdateOSCursor(iOSWindow* apWindow, iOverlay* apCursor) {
  static tBool _bCanUseOSCursor =
#if defined niWindows || defined niOSX
      eTrue
#else
      eFalse
#endif
      ;

  if (!niIsOK(apCursor)) {
    return eFalse;
  }
  if (!_bCanUseOSCursor) {
    return eFalse;
  }

  Ptr<iOSWindow> ptrOSWindow = apWindow;
  if (!ptrOSWindow.IsOK()) {
    return eFalse;
  }

  iTexture* cursorTex = apCursor->GetMaterial()->GetChannelTexture(eMaterialChannel_Base);
  if (!cursorTex) {
    return eFalse;
  }

  if (ptrOSWindow->GetCustomCursorID() == (tIntPtr)apCursor) {
    if (ptrOSWindow->GetCursor() != eOSCursor_Custom) {
      // don't change the cursor if its already a custom cursor, this avoid
      // cursor flickering
      ptrOSWindow->SetCursor(eOSCursor_Custom);
    }
    return eTrue;
  }

  const sVec2f cursorPivot = apCursor->GetPivot();
  const sRectf cursorMapping = apCursor->GetMapping();

  sRecti rect;
  if (cursorMapping.GetWidth() <= 1.0f || cursorMapping.GetHeight() <= 1.0f) {
    sVec2f size = Vec2((tF32)cursorTex->GetWidth(),(tF32)cursorTex->GetHeight());
    rect = sRectf(cursorMapping.GetTopLeft()*size,
                  cursorMapping.GetBottomRight()*size).ToInt();
  }
  else {
    rect = cursorMapping.ToInt();
  }
  tU32 cursorBmpW = 0, cursorBmpH = 0;
  if (ptrOSWindow->InitCustomCursor((tIntPtr)cursorTex,rect.GetWidth(),rect.GetHeight(),0,0,NULL)) {
    cursorBmpW = rect.GetWidth();
    cursorBmpH = rect.GetHeight();
  }
  else {
    if ((tU32)rect.GetWidth() <= 32 && (tU32)rect.GetHeight() <= 32) {
      cursorBmpW = 32;
      cursorBmpH = 32;
    }
  }
  if (cursorBmpW && cursorBmpH &&
      ptrOSWindow->InitCustomCursor((tIntPtr)cursorTex,cursorBmpW,cursorBmpH,0,0,NULL))
  {
    Ptr<iBitmap2D> bmp = mptrGraphics->CreateBitmap2D(
      cursorBmpW,cursorBmpH,_A("R8G8B8A8"));
    bmp->Clear();
    tBool canInit = eFalse;
    iImage* img = apCursor->GetImage();
    if (img) {
      canInit = bmp->Blit(img->GrabBitmap(eImageUsage_Source,sRecti::Null()),
                          rect.x,rect.y,
                          0,0,
                          rect.GetWidth(),rect.GetHeight());
    }
    else {
      canInit = mptrGraphics->BlitTextureToBitmap(
        cursorTex,0,bmp,rect,sRecti(0,0,rect.GetWidth(),rect.GetHeight()),
        eTextureBlitFlags_None);
    }

    if (canInit) {
      sVec2i pivot;
      if (cursorPivot.x <= 1.0f || cursorPivot.y <= 1.0f) {
        pivot.x = tI32(cursorPivot.x*tF32(rect.GetWidth()));
        pivot.y = tI32(cursorPivot.y*tF32(rect.GetHeight()));
      }
      else {
        pivot.x = tI32(cursorPivot.x);
        pivot.y = tI32(cursorPivot.y);
      }
      if (ptrOSWindow->InitCustomCursor(
            (tIntPtr)apCursor,
            bmp->GetWidth(),
            bmp->GetHeight(),
            pivot.x,pivot.y,
            (tU32*)bmp->GetData()))
      {
        ptrOSWindow->SetCursor(eOSCursor_Custom);
        return eTrue;
      }
    }
    else {
      _bCanUseOSCursor = eFalse;
      niWarning("DrawCursor: Couldn't blit the cursor texture, falling back to using manual cursor rendering.");
    }
  }

  return eFalse;
}

///////////////////////////////////////////////
tBool __stdcall cUIContext::DrawWidget(iWidget* apWidget, iCanvas* apCanvas) {
  if (!niIsOK(apWidget)) {
    niError(niFmt("Invalid Widget '%s' (%p).",
                  (apWidget?niHStr(apWidget->GetID()):""),
                  (void*)apWidget));
    return eFalse;
  }
  if (!niIsOK(apCanvas)) {
    niError("Invalid Canvas.");
    return eFalse;
  }
  cWidget* w = (cWidget*)apWidget;
  const sRectf rectContext = apWidget->GetWidgetRect();
  tBool r = _DrawWidget(w,apCanvas,rectContext,eFalse,sMatrixf::Identity());
  apCanvas->Flush();
  return r;
}

///////////////////////////////////////////////
tBool __stdcall cUIContext::DrawTransformedWidget(iWidget* apWidget, iCanvas* apCanvas, const sMatrixf& aBaseMatrix, tBool abUseScissor) {
  if (!niIsOK(apWidget)) {
    niError(niFmt("Invalid Widget '%s' (%p).",
                  (apWidget?niHStr(apWidget->GetID()):""),
                  (void*)apWidget));
    return eFalse;
  }
  if (!niIsOK(apCanvas)) {
    niError("Invalid Canvas.");
    return eFalse;
  }
  cWidget* w = (cWidget*)apWidget;
  const sRectf rectContext = apWidget->GetWidgetRect();
  tBool r = _DrawWidget(w,apCanvas,rectContext,abUseScissor ? RAW_VIEWPORT_SCISSOR : RAW_VIEWPORT,aBaseMatrix);
  apCanvas->Flush();
  return r;
}

///////////////////////////////////////////////
tBool cUIContext::_DrawWidget(cWidget* w, iCanvas* apCanvas, const sRectf& aParentRect, tU32 anRawViewport, const sMatrixf& aBaseMatrix)
{
  niAssert(niFlagIsNot(w->mStatus,WDGSTATUS_INVALID));
  niCheck(niFlagIsNot(w->mStatus,WDGSTATUS_INVALID),eFalse);

  niProf(UI_Widget_Draw);
  if (!w->GetVisible())
    return eTrue;

  const tWidgetStyleFlags widgetStyle = w->GetStyle();

  niGuardObject(w);
  if (!niFlagIs(widgetStyle,eWidgetStyle_Free) &&
      !niFlagIs(widgetStyle,eWidgetStyle_NoClip) &&
      w->mpwParent.IsOK())
  {
    QPtr<cWidget> ptrParent(w->mpwParent);
    if (ptrParent.IsOK()) {
      // CULLING
      if (niFlagIs(widgetStyle,eWidgetStyle_NCRelative)) {
        sRectf parentRect = ptrParent->GetAbsoluteClippedRect();
        if (!parentRect.IntersectRect(w->GetAbsoluteRect()))
          return eTrue;
      }
      else {
        sRectf parentRect = ptrParent->GetAbsoluteClippedClientRect();
        if (!parentRect.IntersectRect(w->GetAbsoluteRect()))
          return eTrue;
      }
    }
  }

  tBool bDrawError = eFalse;
  tBool inheritCanvas = eFalse;
  iCanvas* customCanvas = w->GetCanvas();
  iCanvas* canvas = customCanvas;

  sRectf nonClientVP;
  if (!customCanvas || customCanvas == apCanvas) {
    inheritCanvas = eTrue;
    canvas = apCanvas;
    // move the non-client viewport to its position in the parent canvas
    nonClientVP = w->GetWidgetRect();
    nonClientVP.Move(aParentRect.GetTopLeft()+w->GetPosition());
  }
  else {
    // the non-client VP is at origin for custom canvas
    nonClientVP = w->GetWidgetRect();
  }

  sRectf clientVP = w->GetClientRect();
  clientVP.Move(nonClientVP.GetTopLeft());

  sRecti nonClientSc, clientSc;
  if (niFlagIs(widgetStyle,eWidgetStyle_NoClip)) {
    nonClientSc = clientSc = sRecti::Null();
  }
  else {
    sRectf r = w->GetAbsoluteClippedRect();
    r.Move(-w->GetAbsolutePosition());
    r.Move(nonClientVP.GetTopLeft());
    nonClientSc = r.ToInt();
    r = w->GetClippedClientRect();
    r.Move(nonClientVP.GetTopLeft()); // move to 'absolute' position, relative to the context
    clientSc = r.ToInt();
  }

#define CANVAS_SET_VIEWPORT_AND_SCISSOR(NEW_VP,NEW_SCISSOR)             \
  _ApplyWidgetViewport(canvas, NEW_VP, NEW_SCISSOR, anRawViewport, aBaseMatrix, mfContentsScale);

#define CANVAS_RESTORE_VIEWPORT_AND_SCISSOR()

  do {
    tBool shouldDraw = eFalse;
    if (inheritCanvas) {
      shouldDraw = eTrue;
    }
    else if (customCanvas) {
      shouldDraw = niFlagIs(w->mStatus,WDGSTATUS_REDRAW);
      niFlagOff(w->mStatus,WDGSTATUS_REDRAW);
    }
    else {
      shouldDraw = eTrue;
    }

    if (shouldDraw) {
      tU32 numNonClientChildren = 0;

      /// NC-AREA
      if (niFlagIsNot(w->mStatus,WDGSTATUS_DUMMY)) {
        niProf(UI_Widget_Draw_NCPaint);

        CANVAS_SET_VIEWPORT_AND_SCISSOR(nonClientVP,nonClientSc);

        Var mousepos = this->GetMousePos() - w->GetAbsolutePosition();

        w->SendMessage(eUIMessage_NCPaint,mousepos,canvas);
        if (niFlagIs(w->mStatus,WDGSTATUS_INVALID)) {
          bDrawError = eTrue;
          break;
        }
      }

      /// WIDGET's CLIENT AREA
      {
        if (niFlagIsNot(w->mStatus,WDGSTATUS_DUMMY)) {
          niProf(UI_Widget_Draw_Paint);

          CANVAS_SET_VIEWPORT_AND_SCISSOR(clientVP,clientSc);

          Var mousepos = this->GetMousePos() -
              (w->GetAbsolutePosition() + w->mClientRect.GetTopLeft());

          w->SendMessage(eUIMessage_Paint,mousepos,canvas);
          if (niFlagIs(w->mStatus,WDGSTATUS_INVALID)) {
            bDrawError = eTrue;
            break;
          }
        }

        const tBool shouldDrawnClientChildren = niFlagIsNot(w->mStatus,WDGSTATUS_HIDECHILDREN);
        if (shouldDrawnClientChildren) {
          /// CLIENT CHILDREN
          if (!w->mZMap.IsEmpty()) {
            // CANVAS_SET_VIEWPORT_AND_SCISSOR(clientVP, clientSc);
            const tU32 zmapTouch = w->mZMap.GetTouchCount();
            for (tWidgetZMapRIt zit = w->mZMap.RBegin(); zit != w->mZMap.REnd(); ++zit) {
              for (tWidgetPtrDeqIt lit = zit->second.begin(); lit != zit->second.end(); ++lit) {
                if (niFlagIs((*lit)->GetStyle(),eWidgetStyle_NCRelative)) {
                  ++numNonClientChildren;
                  continue;
                }
                if (!_DrawWidget(*lit,canvas,clientVP,anRawViewport,aBaseMatrix) || zmapTouch != w->mZMap.GetTouchCount()) {
                  bDrawError = eTrue;
                  break;
                }
              }
              if (bDrawError)
                break;
            }
            if (bDrawError)
              break;
          }
        }
      }

      /// NON-CLIENT CHILDREN
      if (numNonClientChildren && !w->mZMap.IsEmpty()) {
        // CANVAS_SET_VIEWPORT_AND_SCISSOR(nonClientVP,nonClientSc);
        const tU32 zmapTouch = w->mZMap.GetTouchCount();
        for (tWidgetZMapRIt zit = w->mZMap.RBegin(); zit != w->mZMap.REnd(); ++zit) {
          for (tWidgetPtrDeqIt lit = zit->second.begin(); lit != zit->second.end(); ++lit) {
            if (!niFlagIs((*lit)->GetStyle(),eWidgetStyle_NCRelative)) {
              continue;
            }
            if (!_DrawWidget(*lit,canvas,nonClientVP,anRawViewport,aBaseMatrix) || zmapTouch != w->mZMap.GetTouchCount()) {
              bDrawError = eTrue;
              break;
            }
          }
          if (bDrawError)
            break;
        }
        if (bDrawError)
          break;
      }
    } // if (shouldDraw)
  } while (0);

  CANVAS_RESTORE_VIEWPORT_AND_SCISSOR();

  if (!inheritCanvas) {
    canvas->Flush(); // flush the widget's own canvas
    _ApplyWidgetViewport(apCanvas, aParentRect, sRecti::Null(),anRawViewport,aBaseMatrix,mfContentsScale);
    apCanvas->BlitStretch(w->GetRect(),canvas->GetGraphicsContext()->GetRenderTarget(0),w->GetWidgetRect());
  }

  return !bDrawError;
}

///////////////////////////////////////////////
cWidget *cUIContext::GetMessageTargetByPos(const sVec2f &pos)
{
  QPtr<cWidget> ptrTopWidget(mpwTopWidget);
  Ptr<cWidget> target = mFreeWidgets.zmapInput.GetTargetByPos(
      pos,Callback_ExcludeWidget_UserDataAndIgnoreInput,(tIntPtr)ptrTopWidget.ptr());
  if (!target.IsOK() && ptrTopWidget.IsOK()) {
    target = ptrTopWidget->GetMessageTargetByPos(
        pos,Callback_ExcludeWidget_IgnoreInput,0);
  }
  return target.IsOK()?target.ptr():ptrTopWidget.ptr();
}

///////////////////////////////////////////////
cWidget *cUIContext::GetDragDestinationByPos(const sVec2f &pos)
{
  cWidget* pW = mpwRootWidget->GetMessageTargetByPos(pos,Callback_ExcludeWidget_IgnoreInputNotDragDest,0);
  while (pW && niFlagIsNot(pW->GetStyle(),eWidgetStyle_DragDestination)) {
    pW = (cWidget*)pW->GetParent();
  }
  return pW;
}

///////////////////////////////////////////////
cWidget *cUIContext::GetMouseMessageTarget(tBool abUpdate)
{
  return GetFingerMessageTarget(GetPrimaryFingerID(),abUpdate);
}

///////////////////////////////////////////////
cWidget *cUIContext::GetFingerMessageTarget(tU32 anFinger, tBool abUpdate)
{
  CHECK_FINGER(anFinger,NULL);

  sFinger& f = GET_FINGER(anFinger);
  {
    QPtr<cWidget> dragWidget(f.mpDragWidget);
    if (niIsOK(dragWidget))
      return dragWidget;
  }

  QPtr<cWidget> captureAll(mpwCaptureAll);
  QPtr<cWidget> captureFinger(f.mpwCapture);

  if (abUpdate) {
    f.mpwHover = GetMessageTargetByPos((sVec2f&)f.mvPosition);
    QPtr<cWidget> hoverWidget(f.mpwHover);
    if (IsPrimaryFinger(anFinger) &&
        !captureAll.IsOK() &&
        !captureFinger.IsOK() &&
        hoverWidget.IsOK() &&
        niFlagIs(hoverWidget->GetStyle(),eWidgetStyle_OverFocus))
    {
      _UIFocusTrace(niFmt(_A("### UICONTEXT GetFingerMessageTarget SetFocusInput to HoverWidget: %p (ID:%s)."),(tIntPtr)hoverWidget.ptr(),hoverWidget->GetID()));
      SetFocusInput(hoverWidget,eFalse);
    }
  }

  if (niIsOK(captureAll)) {
    return captureAll;
  }
  else if (niIsOK(captureFinger)) {
    return captureFinger;
  }
  else {
    QPtr<cWidget> hoverWidget(f.mpwHover);
    return hoverWidget;
  }
}

///////////////////////////////////////////////
const WeakPtr<cWidget>& cUIContext::GetInputMessageTarget() const
{
  return mpwFocusInput;
}

///////////////////////////////////////////////
iWidget* __stdcall cUIContext::GetFocusedWidget() const {
  QPtr<cWidget> ptrFocusInput(mpwFocusInput);
  return ptrFocusInput.ptr();
}

///////////////////////////////////////////////
void cUIContext::InvalidateTarget(cWidget *pWidget)
{
  {
    QPtr<cWidget> ptrActive(mpwActive);
    if (ptrActive == pWidget) {
      SetActiveWidget(NULL);
    }
  }

  InvalidateHandlerTimers(pWidget);

  tBool bWasFocusInput = eFalse;
  {
    QPtr<cWidget> ptrFocusInput(mpwFocusInput);
    if (ptrFocusInput == pWidget) {
      bWasFocusInput = eTrue;
      mpwFocusInput.SetNull();
    }
  }

  mFreeWidgets.RemoveOfZMap(pWidget);
  _RemoveExclusive(pWidget,eFalse);
  _RemoveCaptureAll(pWidget,eFalse);
  niLoop(i,knNumFingers) {
    _RemoveFingerCapture(i,pWidget,eFalse);
  }
  if (bWasFocusInput) {
    Ptr<cWidget> ptrNewTarget = GetMessageTargetByPos(GetMousePos());
    if (ptrNewTarget.IsOK()) {
      if (bWasFocusInput) {
      _UIFocusTrace(niFmt(_A("### UICONTEXT-INVALIDATE-TARGET-SetFocusTo: %p (ID:%s)."),(tIntPtr)ptrNewTarget.ptr(),ptrNewTarget->GetID()));
        SetFocusInput(ptrNewTarget,eFalse);
      }
    }
  }
}

///////////////////////////////////////////////
void cUIContext::RegisterWidget(cWidget* apWdg)
{
  WeakPtr<cWidget> w(apWdg);
  niAssert(astl::find(mlstWidgets,w) == mlstWidgets.end());
  mlstWidgets.push_back(w);
}

///////////////////////////////////////////////
void cUIContext::UnregisterWidget(cWidget* apWdg)
{
  tBool res = astl::find_erase(mlstWidgets,WeakPtr<cWidget>(apWdg));
  niUnused(res);
  niAssert(res);
}

///////////////////////////////////////////////
tBool __stdcall cUIContext::SetActiveWidget(iWidget* apWidget)
{
  {
    QPtr<cWidget> ptrActive(mpwActive);
    if (ptrActive.ptr() == apWidget)
      return eTrue;

    if (ptrActive.IsOK()) {
      ptrActive->SendMessage(eUIMessage_Deactivate,niVarNull,niVarNull);
      //    niDebugFmt((_A("### UICONTEXT-DEACTIVATE: %p (ID:%s)."),ptrActive,ptrActive?niHStr(ptrActive->GetID()):AZEROSTR));
    }
  }

  if (!apWidget) {
    mpwActive.SetNull();
    return eFalse;
  }
  else {
    mpwActive = static_cast<cWidget*>(apWidget);
    apWidget->SendMessage(eUIMessage_Activate,niVarNull,niVarNull);
    //    niDebugFmt((_A("### UICONTEXT-ACTIVATE: %p (ID:%s)."),mpwActive,mpwActive?niHStr(mpwActive->GetID()):AZEROSTR));
    return eTrue;
  }
}

///////////////////////////////////////////////
iWidget* __stdcall cUIContext::GetActiveWidget() const
{
  QPtr<cWidget> ptrActive(mpwActive);
  return ptrActive;
}

///////////////////////////////////////////////
tU32 __stdcall cUIContext::GetNumWidgets() const
{
  return (tU32)mlstWidgets.size();
}

///////////////////////////////////////////////
iWidget* __stdcall cUIContext::GetWidget(tU32 anIndex) const
{
  if (anIndex >= mlstWidgets.size())
    return NULL;
  tU32 nCount = 0;
  for (tWeakWidgetList::const_iterator cit = mlstWidgets.begin(); cit != mlstWidgets.end(); ++cit, ++nCount) {
    if (nCount == anIndex) {
      return QPtr<cWidget>(*cit);
    }
  }
  return NULL;
}

///////////////////////////////////////////////
tBool __stdcall cUIContext::SerializeWidget(iWidget* apWidget, iDataTable* apDT, tWidgetSerializeFlags anFlags, iRegex* apRegex)
{
  if (!niIsOK(apWidget)) {
    niError(_A("Invalid widget."));
    return eFalse;
  }
  if (!niIsOK(apDT)) {
    niError(_A("Invalid data table."));
    return eFalse;
  }
  if (niIsOK(apRegex) && !apRegex->DoesMatch(niHStr(apWidget->GetClassName()))) {
    niWarning(niFmt(_A("Widget with class '%s' doesn't match the filter."),niHStr(apWidget->GetClassName())));
    return eFalse;
  }

  if (niFlagIsNot(anFlags,eWidgetSerializeFlags_Read) && niFlagIsNot(anFlags,eWidgetSerializeFlags_Write)) {
    niError(_A("Nor read nor write serialize flags specified."));
    return eFalse;
  }
  if (niFlagIs(anFlags,eWidgetSerializeFlags_Read) && niFlagIs(anFlags,eWidgetSerializeFlags_Write)) {
    niError(_A("Read and write can't be specified togheter."));
    return eFalse;
  }

  Ptr<iWidgetSink> ptrSink;
  if (niFlagIs(anFlags,eWidgetSerializeFlags_Read))
  {
    ni::cString strScriptPath = apDT->GetString(_A("code"));
    if (strScriptPath.IsNotEmpty()) {
#if niMinFeatures(15)
      ptrSink = this->CreateWidgetSinkFromScript(_H(strScriptPath));
      if (!ptrSink.IsOK()) {
        niError(niFmt(_A("Can't create the widget sink from code file '%s'."),strScriptPath.Chars()));
        return eFalse;
      }
#else
      niWarning(niFmt("Scripting Host not available to load code file '%s'.",strScriptPath));
#endif
    }
  }

  {
    if (niFlagIs(anFlags,eWidgetSerializeFlags_Read)) {
      if (niFlagIsNot(anFlags,eWidgetSerializeFlags_NoRoot)) {
        if (!ni::StrEq(apDT->GetName(),_A("Widget")) &&
            (_H(apDT->GetName()) != apWidget->GetClassName()))
        {
          niError(_A("Not a valid widget data table."));
          return eFalse;
        }
      }
      //      tHStringPtr hspClass = _H(apDT->GetString(_A("class")));
      //      if (HStringIsNotEmpty(hspClass) && hspClass != apWidget->GetClassName()) {
      //        niWarning(niFmt(_A("Trying to read a widget class '%s' in a a widget of class '%s'."),niHStr(hspClass),niHStr(apWidget->GetClassName())));
      //      }
      tHStringPtr hspID = _H(apDT->GetString(_A("id")));
      if (HStringIsNotEmpty(hspID) && HStringIsNotEmpty(apWidget->GetID())) {
        if (hspID != apWidget->GetID()) {
          niWarning(niFmt(_A("Trying to read widget id '%s' in widget with id '%s'."),niHStr(hspID),niHStr(apWidget->GetID())));
        }
      }
      else {
        apWidget->SetID(hspID);
      }
    }
    else if (niFlagIs(anFlags,eWidgetSerializeFlags_Write)) {
      if (niFlagIsNot(anFlags,eWidgetSerializeFlags_NoRoot)) {
        apDT->SetName(_A("Widget"));
      }
      apDT->SetString(_A("class"),niHStr(apWidget->GetClassName()));
      apDT->SetString(_A("id"),niHStr(apWidget->GetID()));
    }
    apWidget->SendMessage(eUIMessage_SerializeLayout,apDT,anFlags);
  }

  apWidget->SendMessage(eUIMessage_SerializeWidget,apDT,anFlags);

  if (niFlagIs(anFlags,eWidgetSerializeFlags_Children))
  {
    if (niFlagIs(anFlags,eWidgetSerializeFlags_Read)) {
      Ptr<iDataTable> dtChildren = apDT->GetChild(_A("Children"));
      if (!dtChildren.IsOK()) dtChildren = apDT;
      if (niIsOK(apRegex)) dtChildren->SetIUnknown(_A("_regex_filter"),apRegex);
      apWidget->SendMessage(eUIMessage_SerializeChildren,dtChildren.ptr(),anFlags);
      if (niIsOK(apRegex)) dtChildren->RemoveProperty("_regex_filter");
    }
    else /*if (niFlagIs(anFlags,eWidgetSerializeFlags_Read))*/ {
      if (apWidget->GetNumChildren()) {
        Ptr<iDataTable> dtChildren = ni::CreateDataTable(_A("Children"));
        if (niIsOK(apRegex)) dtChildren->SetIUnknown(_A("_regex_filter"),apRegex);
        apWidget->SendMessage(eUIMessage_SerializeChildren,dtChildren.ptr(),anFlags);
        if (niIsOK(apRegex)) dtChildren->RemoveProperty("_regex_filter");
        if (dtChildren->GetNumChildren())
          apDT->AddChild(dtChildren);
      }
    }
  }

  {
    apWidget->SendMessage(eUIMessage_SerializeFinalize,apDT,anFlags);
  }

  if (niFlagIs(anFlags,eWidgetSerializeFlags_Read) && ptrSink.IsOK()) {
    apWidget->AddSink(ptrSink);

    // Send the SerializeWidget & SerializeFinalize messages explicitly so
    // that the code sink has a chance to do custom serialization. The code
    // sink hasn't received any serialization message since the sink is added
    // after all serialization occured.
    ptrSink->OnWidgetSink(apWidget,eUIMessage_SerializeWidget,apDT,anFlags);
    ptrSink->OnWidgetSink(apWidget,eUIMessage_SerializeFinalize,apDT,anFlags);
  }

  return eTrue;
}

///////////////////////////////////////////////
iWidgetCommand* __stdcall cUIContext::CreateWidgetCommand()
{
  return niNew cWidgetCommand();
}

///////////////////////////////////////////////
tBool __stdcall cUIContext::SendCommand(iWidget* apDest, iWidgetCommand* apCmd)
{
  if (!niIsOK(apDest)) return eFalse;
  if (!niIsOK(apCmd)) return eFalse;
  return apDest->SendMessage(eUIMessage_Command,apCmd,niVarNull);
}

///////////////////////////////////////////////
void _GatherDrawFreeWidgets(cWidgetZMap& aMap, cWidget* apWidget, cWidget* apTopWidget) {
  if (!niIsOK(apWidget)) return;
  if (niFlagIs(apWidget->GetStyle(),eWidgetStyle_Free)) {
    if (apTopWidget && apTopWidget->HasChild(apWidget,eTrue)) {
      aMap.AddToZMap(apWidget,eWidgetZOrder_Top);
    }
    else {
      aMap.AddToZMap(apWidget,apWidget->GetZOrder());
    }
  }
  for (tU32 i = 0; i < apWidget->GetNumChildren(); ++i) {
    _GatherDrawFreeWidgets(aMap,(cWidget*)apWidget->GetChildFromIndex(i),apTopWidget);
  }
}

void _GatherInputFreeWidgets(cWidgetZMap& aMap, cWidget* apWidget) {
  if (!niIsOK(apWidget)) return;
  if (niFlagIs(apWidget->GetStyle(),eWidgetStyle_Free))
    aMap.AddToZMap(apWidget,apWidget->GetZOrder());
  for (tU32 i = 0; i < apWidget->GetNumChildren(); ++i) {
    _GatherInputFreeWidgets(aMap,(cWidget*)apWidget->GetChildFromIndex(i));
  }
}

///////////////////////////////////////////////
void cUIContext::UpdateFreeWidgets()
{
  mFreeWidgets.Clear();
  if (!mpwRootWidget.IsOK())
    return;
  QPtr<cWidget> ptrTopWidget(mpwTopWidget);
  _GatherInputFreeWidgets(mFreeWidgets.zmapInput,ptrTopWidget);
  _GatherDrawFreeWidgets(
      mFreeWidgets.zmapDraw,
      mpwRootWidget,
      (ptrTopWidget.ptr() == mpwRootWidget.ptr()) ?
      NULL : ptrTopWidget.ptr());
}

///////////////////////////////////////////////
void __stdcall cUIContext::SetDebugDraw(tBool abDebug)
{
  mbDebugDraw = abDebug;
}

///////////////////////////////////////////////
tBool __stdcall cUIContext::GetDebugDraw() const
{
  return mbDebugDraw;
}

#if niMinFeatures(20)
///////////////////////////////////////////////
void __stdcall cUIContext::InitializeDefaultToolbar() {
  Ptr<iWidget> w = CreateWidget(_A("Toolbar"),GetRootWidget(),ni::sRectf(0,0,mpwRootWidget->GetSize().x,120.0f),eWidgetStyle_HoldFocus,_H("ID_RootToolbar"));
  if (w.IsOK()) {
    w->SetZOrder(eWidgetZOrder_BackgroundBottom);
    w->SetDockStyle(eWidgetDockStyle_DockTop);
    SetToolbar(w);
  }
}

///////////////////////////////////////////////
ni::tBool __stdcall cUIContext::SetToolbar(iWidget* apToolbar) {
  //  QPtr<iWidgetToolbar> tb = apToolbar;
  //  if (!tb.IsOK()) return ni::eFalse;
  //  mptrToolbar = apToolbar;
  //  tb->UpdateToolbar();
  mptrToolbar = apToolbar;
  QPtr<iWidgetToolbar> tb = mptrToolbar.ptr();
  if (tb.IsOK()) {
    tb->UpdateToolbar();
    return ni::eTrue;
  }
  else
  {
    return ni::eFalse;
  }
}

///////////////////////////////////////////////
ni::iWidget* __stdcall cUIContext::GetToolbar() const {
  return mptrToolbar;
}
#endif

///////////////////////////////////////////////
tBool cUIContext::_RedrawWidget(cWidget* apWidget) {
  Ptr<cWidget> w = apWidget;
  do {
    //         if (w->mStatus&WDGSTATUS_REDRAW)
    //             return eTrue; // flag already set we dont need to walk up anymore
    niFlagOn(w->mStatus,WDGSTATUS_REDRAW);
    w = QPtr<cWidget>(w->mpwParent);
  } while(w.IsOK());
  return eFalse;
}

///////////////////////////////////////////////
void __stdcall cUIContext::SetDrawOpCapture(tBool abEnabled) {
  if (mbDrawOpCapture == abEnabled) return;
  mbDrawOpCapture = abEnabled;

  if (mbDrawOpCapture && !mptrDrawOpCapture.IsOK()) {
    mptrDrawOpCapture = mptrGraphics->CreateDrawOpCapture();
  }

  mptrGraphics->SetDrawOpCapture(
      mbDrawOpCapture?mptrDrawOpCapture.ptr():NULL);

  Ptr<cWidget> drawOpCaptureHUD = (cWidget*)mpwRootWidget->GetChildFromID(_HC(__ID_DrawOpCaptureHUD__));
  if (!drawOpCaptureHUD.IsOK()) {
    drawOpCaptureHUD = (cWidget*)CreateWidgetFromResource(
        _H("script://forms/dop_hud.form.xml"),
        NULL,
        _HC(__ID_DrawOpCaptureHUD__),
        NULL);
  }
  if (drawOpCaptureHUD.IsOK()) {
    if (mbDrawOpCapture) {
      drawOpCaptureHUD->SetStyle(
          drawOpCaptureHUD->GetStyle()|eWidgetStyle_Free);
      drawOpCaptureHUD->SetZOrder(eWidgetZOrder_TopMost);
      this->SetActiveWidget(drawOpCaptureHUD);
      drawOpCaptureHUD->SetFocus();
      drawOpCaptureHUD->SetEnabled(eTrue);
      drawOpCaptureHUD->SetVisible(eTrue);
      // drawOpCaptureHUD->SetExclusive(eTrue);
    }
    else {
      // drawOpCaptureHUD->SetExclusive(eFalse);
      drawOpCaptureHUD->SetVisible(eFalse);
      drawOpCaptureHUD->SetEnabled(eFalse);
    }
  }
}
ni::tBool __stdcall cUIContext::GetDrawOpCapture() const {
  return mbDrawOpCapture;
}

///////////////////////////////////////////////
void __stdcall cUIContext::SetShowTerminal(tBool abEnabled) {
  if (mbShowTerminal == abEnabled) return;
  mbShowTerminal = abEnabled;

  Ptr<iWidget> ptrTerminal = mpwRootWidget->GetChildFromID(_HC(__ID_Terminal__));
  if (!ptrTerminal.IsOK()) {
    ptrTerminal = (cWidget*)CreateWidgetFromResource(
        _H("script://forms/terminal.form.xml"),
        NULL,
        _HC(__ID_Terminal__),
        NULL);
  }
  if (ptrTerminal.IsOK()) {
    if (mbShowTerminal) {
      ptrTerminal->SetStyle(
          ptrTerminal->GetStyle()|eWidgetStyle_Free);
      ptrTerminal->SetZOrder(eWidgetZOrder_TopMost);
      this->SetActiveWidget(ptrTerminal);
      ptrTerminal->SetFocus();
      ptrTerminal->SetEnabled(eTrue);
      ptrTerminal->SetVisible(eTrue);
    }
    else {
      ptrTerminal->SetVisible(eFalse);
      ptrTerminal->SetEnabled(eFalse);
    }
  }
}
ni::tBool __stdcall cUIContext::GetShowTerminal() const {
  return mbShowTerminal;
}

///////////////////////////////////////////////
const achar* __stdcall cUIContext::GetTimersName() const {
  return "UIContextTimers";
}

///////////////////////////////////////////////
void __stdcall cUIContext::TimerTriggered(iMessageHandler* apHandler, tU32 anId, tF32 afDuration) {
  ni::SendMessage(apHandler,eUIMessage_Timer,anId,afDuration);
}

///////////////////////////////////////////////

struct UIProfDraw : public ImplRC<iProfDraw> {
  Ptr<iCanvas> _canvas;
  Ptr<iFont> _font;
  tBool _translucent;

  UIProfDraw(iCanvas* canvas_, iFont* font_)
      : _canvas(canvas_)
      , _font(font_)
  {
  }

  virtual void __stdcall BeginDraw(tBool abTranslucent) niImpl {
    _translucent = abTranslucent;
  }
  virtual void __stdcall EndDraw() niImpl {
  }

  virtual void __stdcall DrawRect(tF32 x0, tF32 y0, tF32 x1, tF32 y1, tU32 anColor) niImpl {
    if (_translucent)
      _canvas->BlitFillAlpha(sRectf(x0,y0,x1-x0,y1-y0), anColor);
    else
      _canvas->BlitFill(sRectf(x0,y0,x1-x0,y1-y0), anColor);
  }

  virtual void __stdcall DrawLine(tF32 x0, tF32 y0, tF32 x1, tF32 y1, tU32 anColor) niImpl {
    if (_translucent)
      _canvas->BlitLineAlpha(Vec2f(x0,y0),Vec2f(x1,y1),anColor);
    else
      _canvas->BlitLine(Vec2f(x0,y0),Vec2f(x1,y1),anColor);
  }

  virtual tF32 __stdcall GetTextHeight() niImpl {
    return _font->GetHeight() + 4;
  }
  virtual tF32 __stdcall GetTextWidth(const achar* aText) niImpl {
    return _font->ComputeTextSize(sRectf::Null(), aText, 0).GetWidth();
  }

  virtual void __stdcall BeginText() niImpl {
  }
  virtual void __stdcall EndText() niImpl {
  }
  virtual void __stdcall Text(tF32 x, tF32 y, const achar* aText, tU32 anColor) niImpl {
    _font->SetColor(anColor);
    _canvas->BlitText(_font, sRectf(x,y+2), 0, aText);
  }
};

iProfDraw* __stdcall cUIContext::CreateProfDraw(iCanvas* apCanvas, iFont* apFont) const {
  niCheckIsOK(apCanvas,NULL);
  niCheckIsOK(apFont,NULL);
  return niNew UIProfDraw(apCanvas,apFont);
}

///////////////////////////////////////////////
void __stdcall cUIContext::SetImageMap(iImageMap* apImageMap) {
  mptrImageMap = niGetIfOK(apImageMap);
}

///////////////////////////////////////////////
iImageMap* __stdcall cUIContext::GetImageMap() const
{
  if (!mptrImageMap.IsOK()) {
    niThis(cUIContext)->mptrImageMap = mptrGraphics->CreateImageMap(
        niFmt("UIContext_%p",this),NULL);
    niThis(cUIContext)->mptrImageMap->SetDefaultImageFilter(eFalse);
    niThis(cUIContext)->mptrImageMap->SetDefaultImageBlendMode(eBlendMode_Translucent);
  }
  return mptrImageMap;
}

///////////////////////////////////////////////
tBool __stdcall cUIContext::HasWidgetSinkClass(const achar *aszClassName) const
{
  Ptr<tCreateInstanceCMap> map = ni::GetLang()->GetCreateInstanceMap();
  return map->find(niFmt("UIWidget.%s",aszClassName)) != map->end();
}

///////////////////////////////////////////////
iWidgetSink* __stdcall cUIContext::CreateWidgetSink(const achar *aszClassName, iWidget* apWidget)
{
  QPtr<iWidgetSink> sink = ni::GetLang()->CreateInstance(niFmt("UIWidget.%s",aszClassName),apWidget);
  return sink.GetRawAndSetNull();
}

#if niMinFeatures(15)
///////////////////////////////////////////////
iWidgetSink* cUIContext::CreateWidgetSinkFromScript(iHString* ahspRes)
{
  niCheck(ni::HStringIsNotEmpty(ahspRes),NULL);

  ni::tHStringPtr hspCodeResource = ahspRes;
  ni::Ptr<ni::iScriptingHost> host = ni::GetLang()->FindScriptingHost(_HC(UIWidget),hspCodeResource);
  if (!host.IsOK()) {
    niError(niFmt("Can't find scripting host for code file '%s'.",hspCodeResource));
    return NULL;
  }

  ni::Ptr<ni::iWidgetSink> ptrSink = (ni::iWidgetSink*)host->EvalImpl(
      _HC(UIWidget),hspCodeResource,niGetInterfaceUUID(ni::iWidgetSink));
  if (!niIsOK(ptrSink)) {
    niError(niFmt(_A("No widget sink in the code file '%s'."),ahspRes));
    return NULL;
  }

  return ptrSink.GetRawAndSetNull();
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
niExportFunc(iUnknown*) New_niUI_UIContext(const Var& avarA, const Var& avarB)
{
  QPtr<iGraphicsContext> ptrGraphicsContext = avarA;
  if (!ptrGraphicsContext.IsOK()) {
    niError(niFmt("Can't get the graphics context object."));
    return NULL;
  }

  tHStringPtr hspDefaultSkinPath;
  tF32 contentsScale = 1.0f;
  if (avarB.IsFloatValue()) {
    contentsScale = avarB.GetFloatValue();
  }
  else {
    hspDefaultSkinPath = VarGetHString(avarB);
  }

  return niNew cUIContext(ptrGraphicsContext,hspDefaultSkinPath,contentsScale);
}
