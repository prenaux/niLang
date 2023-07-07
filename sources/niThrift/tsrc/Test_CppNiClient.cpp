#include "stdafx.h"
#include <niAppLibConsole.h>
#include <niThrift/TClientImpl.h>
#include "gen-cpp/Calculator.h"

// TODO: Obviously not a great way to build the thrift service's library
#include "gen-cpp/Calculator.cpp"
#include "gen-cpp/shared_types.cpp"
#include "gen-cpp/SharedService.cpp"
#include "gen-cpp/tutorial_constants.cpp"
#include "gen-cpp/tutorial_types.cpp"

using namespace ni;

niConstValue char* _kDefaultServiceUrl = "https://ham-test-thrift-master-dev-xywazuhpsq-as.a.run.app/";

struct sCalculatorClient : public TClientImpl<tutorial::CalculatorClient> {
  tU32 _msgSent = 0;
  tU32 _msgConsumed = 0;
  tU32 _exceptions = 0;

  sCalculatorClient(const achar* aURL, const achar* aServiceName)
      : tBase(aURL, aServiceName) {
  }

  void __stdcall OnThriftClientConnected() override {
    // niDebugFmt(("sCalculatorClient::OnThriftClientConnected"));

    // do an addition
    niPrintln("add()");
    _client->send_add(5, 50);
    ++_msgSent;
    // send the ping, which will end the application
    niPrintln("ping()");
    _client->send_ping();
    ++_msgSent;

    // do an addition
    niPrintln("add()");
    _client->send_add(123, 1000);
    ++_msgSent;

    // do an addition
    niPrintln("add()");
    _client->send_add(123, 2000);
    ++_msgSent;
  }

  tBool __stdcall OnThriftClientResponse(
    const achar* aFuncName, ni::tI32 aSeqId, TProtocol* apReadProtocol) override {
    niDebugFmt(("... Received name: %s, seq: %d", aFuncName, aSeqId));
    if (ni::StrEq(aFuncName, "ping")) {
      tutorial::Calculator_ping_result r;
      r.read(apReadProtocol);
      niPrintln("recv_ping");
      ++_msgConsumed;
    }
    else if (ni::StrEq(aFuncName, "add")) {
      TResultT<tutorial::Calculator_add_result> r(apReadProtocol);
      if (r.HasResult()) {
        niPrintln(niFmt("recv_add: %d", r.GetResult()));
      }
      else {
        niPrintln("recv_add: FAILED");
      }
      ++_msgConsumed;
    }
    else {
      niError(niFmt("Couldn't handle function '%s'.", aFuncName));
      return eFalse;
    }
    return eTrue;
  }

  virtual tBool __stdcall OnThriftClientException(const ::apache::thrift::TException& aException) {
    tBase::OnThriftClientException(aException);
    ++_exceptions;
    return eFalse;
  }
};

void UpdateLoop(Nonnull<iMessageQueue> mq) {
  // service all scripting hosts
  ni::GetLang()->ServiceAllScriptingHosts(false);

  // update the main executor
  ni::GetConcurrent()->GetExecutorMain()->Update(10);

  // update the console
  // ni::GetConsole()->PopAndRunAllCommands();

  // update the system message queue
  sMessageDesc msg;
  while (mq->Poll(&msg)) {
    // dispatch
    msg.mptrHandler->HandleMessage(msg.mnMsg, msg.mvarA, msg.mvarB);
    if (msg.mnMsg == eOSWindowMessage_Paint)
      break;
  }
}

struct FCppNiClient {
};

TEST_FIXTURE(FCppNiClient, Calculator) {
  const cString url = GetProperty(
    "url", _kDefaultServiceUrl
  );

  niPrintln(niFmt("... url: %s", url));
  niPrintln(niFmt("... Main Thread: %d", ni::ThreadGetCurrentThreadID()));

  ni::Nonnull<ni::iMessageQueue> mq =
    ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID()).non_null();

  ni::Nonnull<sCalculatorClient> calculator{niNew sCalculatorClient(url.c_str(),"")};
  CHECK_RETURN_IF_FAILED(calculator->Connect());

  app::AppConsoleLoop(
    "CppNiClient",
    ni::Runnable([mq,calculator]() {
      UpdateLoop(mq);
      if ((calculator->_msgSent > 0)
          && (calculator->_msgSent == calculator->_msgConsumed))
        return eFalse;
      if (calculator->_exceptions > 0)
        return eFalse;
      return eTrue;
    }),
    NULL);

  CHECK_EQUAL(0, calculator->_exceptions);
  CHECK_EQUAL(calculator->_msgConsumed, calculator->_msgSent);
}
