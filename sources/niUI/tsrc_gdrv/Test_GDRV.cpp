#include "stdafx.h"
#include "FGDRV.h"

using namespace ni;

namespace {

TEST_FIXTURE(FGDRV,Clear) {
  CHECK_RETURN_IF_FAILED(_InitTest(m_testName));

  if (_isInteractive) {
    _window->CenterWindow();
    _window->SetShow(eOSWindowShowFlags_Show);
    _window->ActivateWindow();
    while (!_window->GetRequestedClose()) {
      _window->UpdateWindow(eTrue);
      _mq->PollAndDispatch();
    }
  }
}

}
