// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"
#include "UIContext.h"
#include "WidgetCanvasSink.h"
#include "WidgetButton.h"
#include "WidgetScrollBar.h"
#include "WidgetListBox.h"
#include "WidgetEditBox.h"
#include "WidgetComboBox.h"
#include "WidgetProgressBar.h"
#include "WidgetGroup.h"
#include "WidgetTab.h"
#include "WidgetMenu.h"
#include "WidgetTree.h"
#include "WidgetForm.h"
#include "WidgetDockingManager.h"
#include "WidgetSplitter.h"
#include "WidgetPropertyBox.h"
#include "WidgetSearchBar.h"
#include <niLang/Utils/ConcurrentImpl.h>

ni::iWidgetSink *__stdcall New_WidgetColorPicker(iWidget *apWidget);
ni::iWidgetSink *__stdcall New_WidgetLabel();
ni::iWidgetSink *__stdcall New_WidgetToolbar();
ni::iWidgetSink *__stdcall New_WidgetToolbarGroup();
ni::iWidgetSink *__stdcall New_WidgetText();

template <typename T>
__forceinline void _RegisterStandardWidget(tCreateInstanceCMap *map, const achar *aName, const T &aFun)
{
  astl::upsert(*map, niFmt("UIWidget.%s", aName), ni::Callback1(aFun));
}

void _RegisterStandardWidgets()
{
  Ptr<tCreateInstanceCMap> map = ni::GetLang()->GetCreateInstanceMap();

  _RegisterStandardWidget(map, "Dummy", [](iWidget *apWidget) -> iWidgetSink * {
    return NULL;
  });

  _RegisterStandardWidget(map, "Canvas", [](iWidget *apWidget) -> iWidgetSink * {
    return niNew cWidgetCanvasSink(apWidget);
  });

  _RegisterStandardWidget(map, "Button", [](iWidget *apWidget) -> iWidgetSink * {
    return niNew cButtonWidget(apWidget);
  });

  _RegisterStandardWidget(map, "ScrollBar", [](iWidget *apWidget) -> iWidgetSink * {
    return niNew cScrollBarWidget(apWidget);
  });

  _RegisterStandardWidget(map, "ListBox", [](iWidget *apWidget) -> iWidgetSink * {
    return niNew cWidgetListBox(apWidget);
  });

  _RegisterStandardWidget(map, "EditBox", [](iWidget *apWidget) -> iWidgetSink * {
    return niNew cEditBoxWidget(apWidget);
  });

  _RegisterStandardWidget(map, "ComboBox", [](iWidget *apWidget) -> iWidgetSink * {
    return niNew cComboBoxWidget(apWidget);
  });

  _RegisterStandardWidget(map, "ProgressBar", [](iWidget *apWidget) -> iWidgetSink * {
    return niNew cProgressBarWidget(apWidget);
  });

  _RegisterStandardWidget(map, "Group", [](iWidget *apWidget) -> iWidgetSink * {
    return niNew cWidgetGroup(apWidget);
  });

  _RegisterStandardWidget(map, "Tab", [](iWidget *apWidget) -> iWidgetSink * {
    return niNew cTabWidget(apWidget);
  });

  _RegisterStandardWidget(map, "Label", [](iWidget *apWidget) -> iWidgetSink * {
    return New_WidgetLabel();
  });

  _RegisterStandardWidget(map, "Menu", [](iWidget *apWidget) -> iWidgetSink * {
    return niNew cWidgetMenu(apWidget);
  });
  _RegisterStandardWidget(map, "Tree", [](iWidget *apWidget) -> iWidgetSink * {
    return niNew cWidgetTree(apWidget);
  });

  _RegisterStandardWidget(map, "Form", [](iWidget *apWidget) -> iWidgetSink * {
    return niNew cWidgetForm(apWidget);
  });

  _RegisterStandardWidget(map, "Splitter", [](iWidget *apWidget) -> iWidgetSink * {
    return niNew cWidgetSplitter(apWidget);
  });

  _RegisterStandardWidget(map, "DockingManager", [](iWidget *apWidget) -> iWidgetSink * {
    return niNew cWidgetDockingManager(apWidget);
  });

#if niMinFeatures(20)

  _RegisterStandardWidget(map, "PropertyBox", [](iWidget *apWidget) -> iWidgetSink * {
#if niMinFeatures(20)
    return niNew cWidgetPropertyBox(apWidget);
#else
        return NULL;
#endif
  });

  _RegisterStandardWidget(map, "ColorPicker", [](iWidget *apWidget) -> iWidgetSink * {
    return New_WidgetColorPicker(apWidget);
  });

  _RegisterStandardWidget(map, "Toolbar", [](iWidget *apWidget) -> iWidgetSink * {
    return New_WidgetToolbar();
  });

  _RegisterStandardWidget(map, "ToolbarGroup", [](iWidget *apWidget) -> iWidgetSink * {
    return New_WidgetToolbarGroup();
  });

  _RegisterStandardWidget(map, "SearchBar", [](iWidget *apWidget) -> iWidgetSink * {
#if niMinFeatures(20)
    return niNew cWidgetSearchBar(apWidget);
#else
        return NULL;
#endif
  });

  _RegisterStandardWidget(map, "Text", [](iWidget *apWidget) -> iWidgetSink * {
    return New_WidgetText();
  });
#endif
}
