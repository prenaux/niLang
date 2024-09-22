#include "stdafx.h"
#include <niLang/Utils/UnitSnap.h>
#define CSS_LAYOUT_IMPLEMENTATION
#include "../src/css-layout.h"

using namespace ni;

struct FFlex {
  FFlex() {
    niDebugFmt(("FFlex::FFlex"));
  }
  ~FFlex() {
    niDebugFmt(("FFlex::~FFlex"));
  }
};

static void _FlexView_init(struct FlexView* apFlexView);

struct FlexView : public ni::ImplRC<ni::iUnknown>, public css_node_t {

  FlexView() {
    _FlexView_init(this);
    mnBackgroundColor = ~0;
  }

  tBool AddChild(FlexView* apChild) {
    niCheckSilent(niIsOK(apChild),eFalse);
    mvChildren.push_back(apChild);
    this->children_count = niUnsafeNarrowCast(int, mvChildren.size());
    return eTrue;
  }

  FlexView* AddChild() {
    Ptr<FlexView> ptrChild = niNew FlexView();
    mvChildren.push_back(ptrChild);
    this->children_count = niUnsafeNarrowCast(int, mvChildren.size());
    return ptrChild;
  }

  void ClearChildren() {
    mvChildren.clear();
    this->children_count = 0;
  }

  FlexView* GetChild(int i) {
    return mvChildren[i];
  }
  const FlexView* GetChild(int i) const {
    return mvChildren[i];
  }

  tBool IsDirty() {
    return eTrue;
  }

  const float GetLeft() const {
    return this->layout.position[0];
  }
  const float GetTop() const {
    return this->layout.position[1];
  }
  const float GetWidth() const {
    return this->layout.dimensions[0];
  }
  const float GetHeight() const {
    return this->layout.dimensions[1];
  }
  const float GetRight() const {
    return this->layout.position[0] + this->layout.dimensions[0];
  }
  const float GetBottom() const {
    return this->layout.position[1] + this->layout.dimensions[1];
  }

  const sVec2f& GetPosition() const {
    return *(const sVec2f*)this->layout.position;
  }
  const sVec2f& GetSize() const {
    return *(const sVec2f*)this->layout.dimensions;
  }

  const sRectf GetRect() const {
    return Rectf(
        this->layout.position[0],
        this->layout.position[1],
        this->layout.dimensions[0],
        this->layout.dimensions[1]);
  }

  void Draw(iCanvas* apCanvas) {
    apCanvas->BlitFill(
        ni::UnitSnapf(this->GetRect()),
        this->mnBackgroundColor);

    niLoop(i,this->children_count) {
      this->mvChildren[i]->Draw(apCanvas);
    }
  }

  tU32 mnBackgroundColor;
  astl::vector<ni::Ptr<FlexView> > mvChildren;
};

struct CssStyle {
  css_style_t& _style;
  FlexView* _view;

  CssStyle(FlexView* apFlexView) : _style(apFlexView->style), _view(apFlexView) {}

#define STYLE_PROPERTY(NAME,STYLE_NAME,TYPE)    \
  __forceinline CssStyle& NAME(TYPE _##NAME) {                \
    _style.STYLE_NAME = _##NAME;           \
    return *this;                               \
  }                                             \
  __forceinline const TYPE NAME() const {       \
    return _style.STYLE_NAME;              \
  }

  STYLE_PROPERTY(flexDirection,flex_direction,css_flex_direction_t);
  STYLE_PROPERTY(justifyContent,justify_content,css_justify_t);
  STYLE_PROPERTY(alignItems,align_items,css_align_t);
  STYLE_PROPERTY(alignContent,align_content,css_align_t);
  STYLE_PROPERTY(alignSelf,align_self,css_align_t);
  STYLE_PROPERTY(position,position_type,css_position_type_t);
  STYLE_PROPERTY(flexWrap,flex_wrap,css_wrap_type_t);
  STYLE_PROPERTY(flex,flex,float);

#define STYLE_PROPERTY_BOX(STYLE_NAME,MAIN)                       \
  __forceinline CssStyle& MAIN(const float _##NAME) {             \
    _style.STYLE_NAME[0] = _##NAME;                               \
    _style.STYLE_NAME[1] = _##NAME;                               \
    _style.STYLE_NAME[2] = _##NAME;                               \
    _style.STYLE_NAME[3] = _##NAME;                               \
    return *this;                                                 \
  }                                                               \
  __forceinline const sVec4f& MAIN() const {                      \
    return *(sVec4f*)_style.STYLE_NAME;                           \
  }                                                               \
  __forceinline CssStyle& MAIN##Left(const float aValue) {        \
    _style.STYLE_NAME[CSS_LEFT] = aValue;                         \
    return *this;                                                 \
  }                                                               \
  __forceinline CssStyle& MAIN##Right(const float aValue) {       \
    _style.STYLE_NAME[CSS_RIGHT] = aValue;                        \
    return *this;                                                 \
  }                                                               \
  __forceinline CssStyle& MAIN##Horizontal(const float aValue) {  \
    _style.STYLE_NAME[CSS_LEFT] = aValue;                         \
    _style.STYLE_NAME[CSS_RIGHT] = aValue;                        \
    return *this;                                                 \
  }                                                               \
  __forceinline CssStyle& MAIN##Top(const float aValue) {         \
    _style.STYLE_NAME[CSS_TOP] = aValue;                          \
    return *this;                                                 \
  }                                                               \
  __forceinline CssStyle& MAIN##Bottom(const float aValue) {      \
    _style.STYLE_NAME[CSS_BOTTOM] = aValue;                       \
    return *this;                                                 \
  }                                                               \
  __forceinline CssStyle& MAIN##Vertical(const float aValue) {    \
    _style.STYLE_NAME[CSS_TOP] = aValue;                          \
    _style.STYLE_NAME[CSS_BOTTOM] = aValue;                       \
    return *this;                                                 \
  }

  STYLE_PROPERTY_BOX(margin,margin);
  STYLE_PROPERTY_BOX(padding,padding);
  STYLE_PROPERTY_BOX(border,border);

  STYLE_PROPERTY(width,dimensions[0],float);
  STYLE_PROPERTY(height,dimensions[1],float);

  STYLE_PROPERTY(minWidth,minDimensions[0],float);
  STYLE_PROPERTY(minHeight,minDimensions[1],float);

  STYLE_PROPERTY(maxWidth,maxDimensions[0],float);
  STYLE_PROPERTY(maxHeight,maxDimensions[1],float);

  __forceinline const sVec4f& absolutePosition() const {
    return *(sVec4f*)_style.position;
  }
  __forceinline CssStyle& left(const float aValue) {
    _style.position[CSS_LEFT] = aValue;
    return *this;
  }
  __forceinline CssStyle& right(const float aValue) {
    _style.position[CSS_RIGHT] = aValue;
    return *this;
  }
  __forceinline CssStyle& top(const float aValue) {
    _style.position[CSS_TOP] = aValue;
    return *this;
  }
  __forceinline CssStyle& bottom(const float aValue) {
    _style.position[CSS_BOTTOM] = aValue;
    return *this;
  }

  CssStyle& backgroundColor(tU32 anColor) {
    _view->mnBackgroundColor = anColor;
    return *this;
  }
  CssStyle& backgroundColor(tU8 r, tU8 g, tU8 b) {
    _view->mnBackgroundColor = ULColorBuild(r,g,b,ULColorGetA(_view->mnBackgroundColor));
    return *this;
  }
};

struct css_node* _FlexView_get_child(void *context, int i) {
  return ((FlexView*)context)->GetChild(i);
}
static bool _FlexView_is_dirty(void *context) {
  return ((FlexView*)context)->IsDirty();
}
static void _FlexView_init(FlexView* apFlexView) {
  init_css_node(apFlexView);
  apFlexView->context = (void*)apFlexView;
  apFlexView->get_child = _FlexView_get_child;
  apFlexView->is_dirty = _FlexView_is_dirty;
}

_HDecl(ID_BtAlignItems);
_HDecl(ID_BtAlignItems_FlexStart);
_HDecl(ID_BtAlignItems_Center);
_HDecl(ID_BtAlignItems_FlexEnd);
_HDecl(ID_BtAlignItems_Stretch);

_HDecl(ID_BtAlignContent);
_HDecl(ID_BtAlignContent_FlexStart);
_HDecl(ID_BtAlignContent_Center);
_HDecl(ID_BtAlignContent_FlexEnd);
_HDecl(ID_BtAlignContent_Stretch);

struct BaseWidget : public ni::cWidgetSinkImpl<> {

  Ptr<FlexView> _root;
  css_align_t _alignItems;
  css_align_t _alignContent;

  TEST_CONSTRUCTOR(BaseWidget) {
    niDebugFmt(("BaseWidget::BaseWidget"));
  }

  ~BaseWidget() {
    niDebugFmt(("BaseWidget::~BaseWidget"));
  }

  void _CreateNodes(tBool abCreateOnlyIfExisting = eTrue) {
    if (abCreateOnlyIfExisting && !_root.IsOK())
      return;
    _root = niNew FlexView();
    CssStyle(_root)
        .flexDirection(CSS_FLEX_DIRECTION_ROW)
        .flexWrap(CSS_WRAP)
        .alignContent(_alignContent)
        .alignItems(_alignItems)
        .width(300)
        .height(380)
        .backgroundColor(0x333333)
        .padding(3)
        ;
    {
      niLoop(i,15) {
        FlexView* child = _root->AddChild();
        CssStyle(child)
            .width(50)
            .height(((i == 4) || (i == 7)) ? 100 : 50)
            .backgroundColor(
                (i < 5) ? (0xFF0000) :
                (i < 10) ? (0x00FF00) :
                0x00FFFF)
            .margin(10)
            ;
        if (i == 5 || i == 10 || i == 13) {
          CssStyle(child)
              .alignSelf(CSS_ALIGN_FLEX_START);
        }
        else if (i == 14) {
          CssStyle(child)
              // .height(CSS_UNDEFINED)
              .alignSelf(CSS_ALIGN_STRETCH);
        }
      }
    }
  }

  void _LayoutNodes() {
    if (!_root.IsOK())
      return;
    layoutNode(_root.ptr(),4000,4000,(css_direction_t)-1);
    print_css_node(_root.ptr(), (css_print_options_t)(CSS_PRINT_LAYOUT|CSS_PRINT_STYLE|CSS_PRINT_CHILDREN));
    fflush(stdout);
  }

  virtual tBool __stdcall OnSinkAttached() niImpl {

    {
      Ptr<iWidget> right = mpWidget->GetUIContext()->CreateWidget(
          "Dummy", mpWidget, Rectf(0,0,90,36), 0, NULL);
      right->SetDockStyle(eWidgetDockStyle_DockRight);
      right->SetMargin(Vec4f(0,0,10,0));
      right->SetPadding(Vec4f(0,5,0,0));

      {
        Ptr<iWidget> label = mpWidget->GetUIContext()->CreateWidget(
            "Label",right,Rectf(0,0,100,26),0,NULL);
        label->SetText(_H("AlignContent"));
        label->SetDockStyle(eWidgetDockStyle_DockTop);
      }
      {
        Ptr<iWidget> bt = mpWidget->GetUIContext()->CreateWidget(
            "Button",right,Rectf(0,0,90,36),
            eWidgetButtonStyle_OnOff|eWidgetStyle_HoldFocus,
            _HC(ID_BtAlignContent_FlexStart));
        bt->SetText(_H("FlexStart"));
        bt->SetDockStyle(eWidgetDockStyle_DockTop);
        QPtr<iWidgetButton>(bt)->SetGroupID(_HC(ID_BtAlignContent));
      }
      {
        Ptr<iWidget> bt = mpWidget->GetUIContext()->CreateWidget(
            "Button",right,Rectf(0,0,90,36),
            eWidgetButtonStyle_OnOff|eWidgetStyle_HoldFocus,
            _HC(ID_BtAlignContent_Center));
        bt->SetText(_H("Center"));
        bt->SetDockStyle(eWidgetDockStyle_DockTop);
        QPtr<iWidgetButton>(bt)->SetGroupID(_HC(ID_BtAlignContent));
      }
      {
        Ptr<iWidget> bt = mpWidget->GetUIContext()->CreateWidget(
            "Button",right,Rectf(0,0,90,36),
            eWidgetButtonStyle_OnOff|eWidgetStyle_HoldFocus,
            _HC(ID_BtAlignContent_FlexEnd));
        bt->SetText(_H("FlexEnd"));
        bt->SetDockStyle(eWidgetDockStyle_DockTop);
        QPtr<iWidgetButton>(bt)->SetGroupID(_HC(ID_BtAlignContent));
      }
      {
        Ptr<iWidget> bt = mpWidget->GetUIContext()->CreateWidget(
            "Button",right,Rectf(0,0,90,36),
            eWidgetButtonStyle_OnOff|eWidgetStyle_HoldFocus,
            _HC(ID_BtAlignContent_Stretch));
        bt->SetText(_H("Stretch"));
        bt->SetDockStyle(eWidgetDockStyle_DockTop);
        QPtr<iWidgetButton>(bt)->SetGroupID(_HC(ID_BtAlignContent));
        bt->SetMargin(Vec4f(0,0,0,10));
      }

      {
        Ptr<iWidget> label = mpWidget->GetUIContext()->CreateWidget(
            "Label",right,Rectf(0,0,100,26),0,NULL);
        label->SetText(_H("AlignItems"));
        label->SetDockStyle(eWidgetDockStyle_DockTop);
      }
      {
        Ptr<iWidget> bt = mpWidget->GetUIContext()->CreateWidget(
            "Button",right,Rectf(0,0,90,36),
            eWidgetButtonStyle_OnOff|eWidgetStyle_HoldFocus,
            _HC(ID_BtAlignItems_FlexStart));
        bt->SetText(_H("FlexStart"));
        bt->SetDockStyle(eWidgetDockStyle_DockTop);
        QPtr<iWidgetButton>(bt)->SetGroupID(_HC(ID_BtAlignItems));
      }
      {
        Ptr<iWidget> bt = mpWidget->GetUIContext()->CreateWidget(
            "Button",right,Rectf(0,0,90,36),
            eWidgetButtonStyle_OnOff|eWidgetStyle_HoldFocus,
            _HC(ID_BtAlignItems_Center));
        bt->SetText(_H("Center"));
        bt->SetDockStyle(eWidgetDockStyle_DockTop);
        QPtr<iWidgetButton>(bt)->SetGroupID(_HC(ID_BtAlignItems));
      }
      {
        Ptr<iWidget> bt = mpWidget->GetUIContext()->CreateWidget(
            "Button",right,Rectf(0,0,90,36),
            eWidgetButtonStyle_OnOff|eWidgetStyle_HoldFocus,
            _HC(ID_BtAlignItems_FlexEnd));
        bt->SetText(_H("FlexEnd"));
        bt->SetDockStyle(eWidgetDockStyle_DockTop);
        QPtr<iWidgetButton>(bt)->SetGroupID(_HC(ID_BtAlignItems));
      }
      {
        Ptr<iWidget> bt = mpWidget->GetUIContext()->CreateWidget(
            "Button",right,Rectf(0,0,90,36),
            eWidgetButtonStyle_OnOff|eWidgetStyle_HoldFocus,
            _HC(ID_BtAlignItems_Stretch));
        bt->SetText(_H("Stretch"));
        bt->SetDockStyle(eWidgetDockStyle_DockTop);
        QPtr<iWidgetButton>(bt)->SetGroupID(_HC(ID_BtAlignItems));
        bt->SetMargin(Vec4f(0,0,0,10));
      }
    }

    QPtr<iWidgetButton>(mpWidget->FindWidget(_HC(ID_BtAlignContent_FlexStart)))->SetCheck(eTrue);
    QPtr<iWidgetButton>(mpWidget->FindWidget(_HC(ID_BtAlignItems_FlexStart)))->SetCheck(eTrue);

    _CreateNodes(eFalse);
    _LayoutNodes();
    return eTrue;
  }

  virtual tBool __stdcall OnCommand(iWidgetCommand* apCmd) niImpl {
    if (apCmd->GetID() != eWidgetButtonCmd_Checked) {
      return eFalse;
    }

    iHString* hspSenderID = apCmd->GetSender()->GetID();
    if (hspSenderID == _HC(ID_BtAlignItems_FlexStart)) {
      _alignItems = (CSS_ALIGN_FLEX_START);
    }
    else if (hspSenderID == _HC(ID_BtAlignItems_Center)) {
      _alignItems = (CSS_ALIGN_CENTER);
    }
    else if (hspSenderID == _HC(ID_BtAlignItems_FlexEnd)) {
      _alignItems = (CSS_ALIGN_FLEX_END);
    }
    else if (hspSenderID == _HC(ID_BtAlignItems_Stretch)) {
      _alignItems = (CSS_ALIGN_STRETCH);
    }
    else if (hspSenderID == _HC(ID_BtAlignContent_FlexStart)) {
      _alignContent = (CSS_ALIGN_FLEX_START);
    }
    else if (hspSenderID == _HC(ID_BtAlignContent_Center)) {
      _alignContent = (CSS_ALIGN_CENTER);
    }
    else if (hspSenderID == _HC(ID_BtAlignContent_FlexEnd)) {
      _alignContent = (CSS_ALIGN_FLEX_END);
    }
    else if (hspSenderID == _HC(ID_BtAlignContent_Stretch)) {
      _alignContent = (CSS_ALIGN_STRETCH);
    }
    else {
      return eFalse;
    }

    _CreateNodes();
    _LayoutNodes();
    return eFalse;
  }

  virtual tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
    apCanvas->BlitFill(ni::Rectf(5,5,100,50), 0xFF00FF00);
    _root->Draw(apCanvas);
    return eFalse;
  }
};
TEST_FIXTURE_WIDGET(FFlex,BaseWidget);
