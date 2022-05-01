#include "stdafx.h"
#include "../src/API/niLang/ILang.h"
#include "../src/API/niLang/Utils/UnknownImpl.h"
#include "../src/API/niLang/Utils/ConcurrentImpl.h"
#include "../src/API/niLang_ModuleDef.h"

using namespace ni;

struct FMessageQueue {
};

TEST_FIXTURE(FMessageQueue,Base) {
  const tU64 tid = GetConcurrent()->GetCurrentThreadID();
  Ptr<iMessageQueue> mq1 = GetConcurrent()->CreateMessageQueue(tid,~0);
  CHECK(mq1.IsOK());
  Ptr<iMessageQueue> mq2 = GetConcurrent()->CreateMessageQueue(tid,~0);
  CHECK_EQUAL((iMessageQueue*)NULL,mq2.ptr());

  CHECK_EQUAL(mq1.ptr(), GetConcurrent()->GetMessageQueue(tid).ptr());
}
