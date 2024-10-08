#include "stdafx.h"
#ifdef niJSCC

#include <emscripten.h>
#include <emscripten/html5.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

namespace {

using namespace ni;

struct FJSCC {
};

// Shader sources
static const GLchar* vertexSource =
    "attribute vec4 position;\n"
    "void main() {\n"
    "  gl_Position = vec4(position.xyz, 1.0);\n"
    "}\n";
static const GLchar* fragmentSource =
    "precision mediump float;\n"
    "void main() {\n"
    "  gl_FragColor = vec4 (1.0, 1.0, 1.0, 1.0 );\n"
    "}\n";

static astl::function<void()> _MainLoopFn;
static void CallMainLoop() {
  _MainLoopFn();
}

struct sOpenGLWindowSink : public ImplRC<iMessageHandler> {
  const tU32 _threadId;
  WeakPtr<iOSWindow> _wnd;

  sOpenGLWindowSink(iOSWindow* apWnd)
    : _wnd(apWnd), _threadId((tU32)ni::ThreadGetCurrentThreadID())
  {}

  tU64 __stdcall GetThreadID() const {
    return _threadId;
  }

  tBool _Init()
  {
    // Create a Vertex Buffer Object and copy the vertex data to it
    GLuint vbo;
    glGenBuffers(1, &vbo);

    GLfloat vertices[] = {0.0f, 0.5f, 0.5f, -0.5f, -0.5f, -0.5f};

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Create and compile the vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    // Create and compile the fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    // Link the vertex and fragment shader into a shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    // glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    // Specify the layout of the vertex data
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    return eTrue;
  }

  ///
  // Draw a triangle using the shader pair created in Init()
  //
  void _Draw(iOSWindow* wnd)
  {
    const sVec2i size = wnd->GetClientSize();

    // Set the viewport
    glViewport(0, 0, size.x, size.y);

    // Clear the color buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    // UpdateWindow() does the swap buffer atm...
    // eglSwapBuffers(esContext->eglDisplay, esContext->eglSurface);
  }

  void __stdcall HandleMessage(const tU32 anMsg, const Var& a, const Var& b) {
    QPtr<iOSWindow> wnd = _wnd;
    if (!wnd.IsOK())
      return;

    switch (anMsg) {
      case eOSWindowMessage_Close:
        niDebugFmt((_A("eOSWindowMessage_Close: \n")));
        wnd->Invalidate();
        break;
      case eOSWindowMessage_SwitchIn:
        niDebugFmt((_A("eOSWindowMessage_SwitchIn: \n")));
        break;
      case eOSWindowMessage_SwitchOut:
        niDebugFmt((_A("eOSWindowMessage_SwitchOut: \n")));
        break;
        // case eOSWindowMessage_Paint: {
        // niDebugFmt((_A("eOSWindowMessage_Paint: \n")));
        // break;
        // }
      case eOSWindowMessage_Size:
        niDebugFmt((_A("eOSWindowMessage_Size: %s\n"),_ASZ(wnd->GetSize())));
        break;
      case eOSWindowMessage_Move:
        niDebugFmt((_A("eOSWindowMessage_Move: %s\n"),_ASZ(wnd->GetPosition())));
        break;
      case eOSWindowMessage_KeyDown:
        niDebugFmt((_A("eOSWindowMessage_KeyDown: %d\n"),a.mU32));
        if (a.mU32 == eKey_F) {
          wnd->SetFullScreen(wnd->GetFullScreen() == eInvalidHandle ? 1 : eInvalidHandle);
        }
        break;
      case eOSWindowMessage_KeyUp:
        niDebugFmt((_A("eOSWindowMessage_KeyUp: %d\n"),a.mU32));
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
          //                     wnd->SetFullScreen(!wnd->GetFullScreen());
        }
        else if (a.mU32 == 8 /*ePointerButton_DoubleClickLeft*/) {
          //                     wnd->SetFullScreen(!wnd->GetFullScreen());
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
        niDebugFmt((_A("eOSWindowMessage_RelativeMouseMove: [%d,%d]\n"),
                    a.mV2L[0],a.mV2L[1]));
        break;

      case eOSWindowMessage_MouseMove:
        niDebugFmt((_A("eOSWindowMessage_MouseMove: [%d,%d] [%d,%d]\n"),
                    a.mV2L[0],a.mV2L[1],
                    b.mV2L[0],b.mV2L[1]));
        break;

      case eOSWindowMessage_Paint: {
        _Draw(wnd.ptr());
        break;
      }
    }
  }
};

TEST_FIXTURE(FJSCC,OpenGL) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  Ptr<iMessageQueue> mq = ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());

  Ptr<iOSWindow> wnd = ni::GetLang()->CreateWindow(
    NULL,
    "HelloWindow",
    sRecti(50,50,400,300),
    0,
    eOSWindowStyleFlags_Regular);
  CHECK(wnd.IsOK());

  Ptr<sOpenGLWindowSink> sink = niNew sOpenGLWindowSink(wnd);
  CHECK(sink->_Init());
  wnd->GetMessageHandlers()->AddSink(sink.ptr());


  if (isInteractive) {
    wnd->CenterWindow();
    wnd->SetShow(eOSWindowShowFlags_Show);
    wnd->ActivateWindow();

    _MainLoopFn = [&]{
      wnd->RedrawWindow();
      wnd->UpdateWindow(eTrue);
      mq->PollAndDispatch();
    };
    emscripten_set_main_loop(CallMainLoop, 30, true);
  }
}

} // end of anonymous namespace
#endif
