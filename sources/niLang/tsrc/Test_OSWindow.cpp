#include "stdafx.h"
#include "../src/API/niLang_ModuleDef.h"
#include "Test_String.h"

using namespace ni;

//#define TRACE_PAINT
// #define TRACE_MOUSE_MOVE
// #define TRACE_RELATIVE_MOUSE_MOVE

namespace {

struct FOSWindow {
};

struct sTestWindowSink : public ImplRC<iMessageHandler> {
  iOSWindow* w;
  Ptr<iDataTable> _clipboardDT;
  sTestWindowSink(iOSWindow* _w) : w(_w) {}

  tU64 __stdcall GetThreadID() const {
    return ni::ThreadGetCurrentThreadID();
  }

  void __stdcall HandleMessage(const tU32 anMsg, const Var& a, const Var& b) {
    switch (anMsg) {
      case eOSWindowMessage_Close:
        niDebugFmt((_A("eOSWindowMessage_Close: \n")));
        w->Invalidate();
        break;
      case eOSWindowMessage_SwitchIn:
        niDebugFmt(("eOSWindowMessage_SwitchIn: %s\n", niEnumToChars(eOSWindowSwitchReason,a.mU32)));
        break;
      case eOSWindowMessage_SwitchOut:
        niDebugFmt(("eOSWindowMessage_SwitchOut: %s\n", niEnumToChars(eOSWindowSwitchReason,a.mU32)));
        break;
      case eOSWindowMessage_Paint: {
#ifdef TRACE_PAINT
        niDebugFmt((_A("eOSWindowMessage_Paint: \n")));
#endif
        break;
      }
      case eOSWindowMessage_Size:
        niDebugFmt((_A("eOSWindowMessage_Size: %s\n"),_ASZ(w->GetSize())));
        break;
      case eOSWindowMessage_Move:
        niDebugFmt((_A("eOSWindowMessage_Move: %s\n"),_ASZ(w->GetPosition())));
        break;

      case eOSWindowMessage_KeyDown:
        niDebugFmt((_A("eOSWindowMessage_KeyDown: %d (%s)\n"),a.mU32,niEnumToChars(eKey,a.mU32)));
        switch (a.mU32) {
          case eKey_F:
            w->SetFullScreen(w->GetFullScreen() == eInvalidHandle ? 0 : eInvalidHandle);
            break;

          case eKey_Z:
            w->SetCursorPosition(sVec2i::Zero());
            break;
          case eKey_X:
            w->SetCursorPosition(w->GetClientSize()/2);
            break;
          case eKey_C:
            w->SetCursorPosition(w->GetClientSize());
            break;
          case eKey_Q:
            w->SetCursor(eOSCursor_None);
            break;
          case eKey_W:
            w->SetCursor(eOSCursor_Arrow);
            break;
          case eKey_E:
            w->SetCursor(eOSCursor_Text);
            break;
          case eKey_K:
            w->SetCursorCapture(!w->GetCursorCapture());
            break;
          case eKey_R:
            w->RedrawWindow();
            break;
          case eKey_n0:
            w->CenterWindow();
            break;

          case eKey_O: {
            if (!_clipboardDT.IsOK()) {
              _clipboardDT = ni::CreateDataTable("Clipboard");
            }
            _clipboardDT->SetString("text","Hello World from Test_OSWindow");
            niCheck_(
              ni::GetLang()->SetClipboard(eClipboardType_System,_clipboardDT),
              "CopyClipboard",;);
            niDebugFmt(("... Copied Clipboard: '%s'", _clipboardDT->GetString("text")));
            break;
          }
          case eKey_P: {
            _clipboardDT = ni::GetLang()->GetClipboard(eClipboardType_System);
            niCheck_(_clipboardDT.IsOK(),"PasteClipboard",;);
            niDebugFmt(("... Pasted Clipboard: '%s'", _clipboardDT->GetString("text")));
            break;
          }

          case eKey_n1:
            niDebugFmt(("... SetPosition: curr rect: %s", w->GetRect()));
            w->SetPosition(Vec2i(10,5));
            niDebugFmt(("... SetPosition: new rect: %s", w->GetRect()));
            break;
          case eKey_n2:
            niDebugFmt(("... SetPosition: curr rect: %s", w->GetRect()));
            w->SetPosition(Vec2i(0,0));
            niDebugFmt(("... SetPosition: new rect: %s", w->GetRect()));
            break;
          case eKey_n3:
            niDebugFmt(("... SetSize: curr rect: %s", w->GetRect()));
            w->SetSize(Vec2i(640,480));
            niDebugFmt(("... SetSize: new rect: %s, client size: %s, size: %s", w->GetRect(), w->GetClientSize(), w->GetSize()));
            break;
          case eKey_n4:
            niDebugFmt(("... SetClientSize: curr rect: %s", w->GetRect()));
            w->SetClientSize(Vec2i(640,480));
            niDebugFmt(("... SetClientSize: new rect: %s, client size: %s, size: %s", w->GetRect(), w->GetClientSize(), w->GetSize()));
            break;
          case eKey_n5:
            niDebugFmt(("... SetSize noop: curr rect: %s", w->GetRect()));
            w->SetSize(w->GetSize());
            niDebugFmt(("... SetSize noop: new rect: %s, client size: %s, size: %s", w->GetRect(), w->GetClientSize(), w->GetSize()));
            break;
          case eKey_n6:
            niDebugFmt(("... SetClientSize noop: curr rect: %s", w->GetRect()));
            w->SetClientSize(w->GetClientSize());
            niDebugFmt(("... SetClientSize noop: new rect: %s, client size: %s, size: %s", w->GetRect(), w->GetClientSize(), w->GetClientSize()));
            break;
          case eKey_n7:
            w->SetFocus();
            break;
        }
        break;

      case eOSWindowMessage_KeyUp:
        niDebugFmt((_A("eOSWindowMessage_KeyUp: %d (%s)\n"),a.mU32,niEnumToChars(eKey,a.mU32)));
        break;
      case eOSWindowMessage_KeyChar:
        niDebugFmt((_A("eOSWindowMessage_KeyChar: %c (%d) \n"),a.mU32,a.mU32));
        break;

      case eOSWindowMessage_MouseButtonDown:
        niDebugFmt((_A("eOSWindowMessage_MouseButtonDown: %d\n"),a.mU32));
        break;
      case eOSWindowMessage_MouseButtonUp:
        niDebugFmt((_A("eOSWindowMessage_MouseButtonUp: %d\n"),a.mU32));
        break;
      case eOSWindowMessage_MouseButtonDoubleClick:
        niDebugFmt((_A("eOSWindowMessage_MouseButtonDoubleClick: %d\n"),a.mU32));
        if (a.mU32 == 9 /*ePointerButton_DoubleClickRight*/) {
          //                     w->SetFullScreen(!w->GetFullScreen());
        }
        else if (a.mU32 == 8 /*ePointerButton_DoubleClickLeft*/) {
          //                     w->SetFullScreen(!w->GetFullScreen());
        }
        break;
      case eOSWindowMessage_MouseWheel:
        niDebugFmt((_A("eOSWindowMessage_MouseWheel: %f, %f\n"),a.GetFloatValue(),b.GetFloatValue()));
        break;

      case eOSWindowMessage_LostFocus: {
        niDebugFmt((_A("eOSWindowMessage_LostFocus: \n")));
        break;
      }
      case eOSWindowMessage_SetFocus:
        niDebugFmt((_A("eOSWindowMessage_SetFocus: \n")));
        break;
      case eOSWindowMessage_Drop:
        niDebugFmt((_A("eOSWindowMessage_Drop: \n")));
        break;

      case eOSWindowMessage_RelativeMouseMove:
#ifdef TRACE_RELATIVE_MOUSE_MOVE
        niDebugFmt((_A("eOSWindowMessage_RelativeMouseMove: [%d,%d]\n"),
                    a.mV2L[0],a.mV2L[1]));
#endif
        break;

      case eOSWindowMessage_MouseMove:
#ifdef TRACE_RELATIVE_RELATIVER_MOVE
        niDebugFmt((_A("eOSWindowMessage_MouseMove: [%d,%d] [%d,%d] (contentsScale: %g)\n"),
                    a.mV2L[0],a.mV2L[1],
                    b.mV2L[0],b.mV2L[1],
                    w->GetContentsScale()));
#endif
        break;
    }
  }
};

TEST_FIXTURE(FOSWindow,Create) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  // TODO: This should be some kind of "HAS_DISPLAY" flag which is false for
  // most CI that dont actually have a usable Linux display we can use to
  // create a window successfully.
#if defined niLinux
  if (isInteractive)
#endif
  {
    Ptr<iOSWindow> wnd = ni::GetLang()->CreateWindow(
      NULL,
      "HelloWindow",
      sRecti(50,50,400,300),
      0,
      eOSWindowStyleFlags_Regular);
    CHECK_RETURN_IF_FAILED(wnd.IsOK());
    wnd->SetDropTarget(eTrue);

    Ptr<sTestWindowSink> sink = niNew sTestWindowSink(wnd);
    wnd->GetMessageHandlers()->AddSink(sink.ptr());

    if (isInteractive) {
      wnd->CenterWindow();
      wnd->SetShow(eOSWindowShowFlags_Show);
      wnd->ActivateWindow();
      while (!wnd->GetRequestedClose()) {
        wnd->UpdateWindow(eTrue);
      }
      niDebugFmt(("... isInteractive end of '%s'.", m_testName));
    }
  }
}

TEST_FIXTURE(FOSWindow,MessageBox) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  if (isInteractive) {
    Ptr<iOSWindow> wnd = ni::GetLang()->CreateWindow(
      NULL,
      "HelloWindow",
      sRecti(50,50,400,300),
      0,
      eOSWindowStyleFlags_Regular);
    CHECK(wnd.IsOK());

    eOSMessageBoxReturn r = ni::GetLang()->MessageBox(wnd,NULL,"Hello World !",0);
    niDebugFmt(("... MessageBox Returned: %s", niEnumToChars(eOSMessageBoxReturn,r)));
  }
}

TEST_FIXTURE(FOSWindow,MessageBoxWarning) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  if (isInteractive) {
    Ptr<iOSWindow> wnd = ni::GetLang()->CreateWindow(
      NULL,
      "HelloWindow",
      sRecti(50,50,400,300),
      0,
      eOSWindowStyleFlags_Regular);
    CHECK(wnd.IsOK());

    eOSMessageBoxReturn r = ni::GetLang()->MessageBox(wnd,NULL,"Hello Warning !",eOSMessageBoxFlags_IconWarning);
    niDebugFmt(("... MessageBox Returned: %s", niEnumToChars(eOSMessageBoxReturn,r)));
    niDebugFmt(("... isInteractive end of '%s'.", m_testName));
  }
}

TEST_FIXTURE(FOSWindow,MessageBoxInfoYesNo) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  if (isInteractive) {
    Ptr<iOSWindow> wnd = ni::GetLang()->CreateWindow(
      NULL,
      "HelloWindow",
      sRecti(50,50,400,300),
      0,
      eOSWindowStyleFlags_Regular);
    CHECK(wnd.IsOK());

    eOSMessageBoxReturn r = ni::GetLang()->MessageBox(
      wnd,NULL,"Hello Info !",eOSMessageBoxFlags_IconInfo|eOSMessageBoxFlags_YesNo);
    niDebugFmt(("... MessageBox Returned: %s", niEnumToChars(eOSMessageBoxReturn,r)));
    niDebugFmt(("... isInteractive end of '%s'.", m_testName));
  }
}

TEST_FIXTURE(FOSWindow,MessageBoxLong) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  if (isInteractive) {
    Ptr<iOSWindow> wnd = ni::GetLang()->CreateWindow(
      NULL,
      "HelloWindow",
      sRecti(50,50,400,300),
      0,
      eOSWindowStyleFlags_Regular);
    CHECK(wnd.IsOK());

    cString txt;
    txt << _MultiLineText;
    txt << AEOL;
    txt << _MultiLineText;
    txt << AEOL;
    txt << _MultiLineText;

    eOSMessageBoxReturn r = ni::GetLang()->MessageBox(
      wnd,NULL,txt.Chars(),eOSMessageBoxFlags_IconInfo|eOSMessageBoxFlags_YesNo);
    niDebugFmt(("... MessageBox Returned: %s", niEnumToChars(eOSMessageBoxReturn,r)));
    niDebugFmt(("... isInteractive end of '%s'.", m_testName));
  }
}

TEST_FIXTURE(FOSWindow,FatalError) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  if (isInteractive) {
    ni_set_show_fatal_error_message_box(eTrue);

    // create a dummy window so that the error doesnt go to the console
    Ptr<iOSWindow> wnd = ni::GetLang()->CreateWindow(
      NULL,
      "HelloWindow",
      sRecti(50,50,400,300),
      0,
      eOSWindowStyleFlags_Regular);
    CHECK(wnd.IsOK());

    ni::GetLang()->FatalError("Test Fatal Error.");
    niDebugFmt(("... isInteractive end of '%s'.", m_testName));
  }
}

TEST_FIXTURE(FOSWindow,FatalErrorLong) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  if (isInteractive) {
    ni_set_show_fatal_error_message_box(eTrue);

    // create a dummy window so that the error doesnt go to the console
    Ptr<iOSWindow> wnd = ni::GetLang()->CreateWindow(
      NULL,
      "HelloWindow",
      sRecti(50,50,400,300),
      0,
      eOSWindowStyleFlags_Regular);
    CHECK(wnd.IsOK());

    cString txt;
    txt << _MultiLineText;
    txt << AEOL;
    txt << _MultiLineText;
    txt << AEOL;
    txt << _MultiLineText;
    ni::GetLang()->FatalError(txt.Chars());
    niDebugFmt(("... isInteractive end of '%s'.", m_testName));
  }
}

TEST_FIXTURE(FOSWindow,OpenFileDialog) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  if (isInteractive) {
    Ptr<iOSWindow> wnd = ni::GetLang()->CreateWindow(
      NULL,
      "HelloWindow",
      sRecti(50,50,400,300),
      0,
      eOSWindowStyleFlags_Regular);
    CHECK(wnd.IsOK());

    cString r = ni::GetLang()->OpenFileDialog(
      wnd,
      "My Open File",
      "pdf;txt",
      ni::GetLang()->GetProperty("ni.dirs.home").Chars());
    niDebugFmt(("... OpenFileDialog: '%s'", r));
    // The user should have selected something and the returned file shouldn't
    // contained "not implemented" (which would mean that dialog isnt
    // implemented on the platform).
    CHECK(r.IsNotEmpty() && !r.icontains("implemented"));
    niDebugFmt(("... isInteractive end of '%s'.", m_testName));
  }
}

TEST_FIXTURE(FOSWindow,SaveFileDialog) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  if (isInteractive) {
    Ptr<iOSWindow> wnd = ni::GetLang()->CreateWindow(
      NULL,
      "HelloWindow",
      sRecti(50,50,400,300),
      0,
      eOSWindowStyleFlags_Regular);
    CHECK(wnd.IsOK());

    cString r = ni::GetLang()->SaveFileDialog(
      wnd,
      "My Save File",
      "pdf;txt",
      ni::GetLang()->GetProperty("ni.dirs.home").Chars());
    niDebugFmt(("... SaveFileDialog: '%s'", r));
    // The user should have selected something and the returned file shouldn't
    // contained "not implemented" (which would mean that dialog isnt
    // implemented on the platform).
    CHECK(r.IsNotEmpty() && !r.icontains("implemented"));
  }
}

TEST_FIXTURE(FOSWindow,PickDirectoryDialog) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  if (isInteractive) {
    Ptr<iOSWindow> wnd = ni::GetLang()->CreateWindow(
      NULL,
      "HelloWindow",
      sRecti(50,50,400,300),
      0,
      eOSWindowStyleFlags_Regular);
    CHECK(wnd.IsOK());

    cString r = ni::GetLang()->PickDirectoryDialog(
      wnd,
      "Pick a test Directory",
      ni::GetLang()->GetProperty("ni.dirs.documents").Chars());
    niDebugFmt(("... PickDirectoryDialog: '%s'", r));
    // The user should have selected something and the returned file shouldn't
    // contained "not implemented" (which would mean that dialog isnt
    // implemented on the platform).
    CHECK(r.IsNotEmpty() && !r.icontains("implemented"));
    niDebugFmt(("... isInteractive end of '%s'.", m_testName));
  }
}

} // end of anonymous namespace
