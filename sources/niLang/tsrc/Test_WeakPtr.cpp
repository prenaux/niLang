#include "stdafx.h"
#include "../src/API/niLang/Utils/UnknownImpl.h"
#include "../src/API/niLang/ILang.h"
#include "../src/API/niLang/Utils/SmartPtr.h"
#include "../src/API/niLang/Utils/WeakPtr.h"
#include "../src/API/niLang/Utils/QPtr.h"

namespace {

using namespace ni;

struct FWeakPtr {
};

TEST_FIXTURE(FWeakPtr,Base) {
  Ptr<iFile> ptrFile = ni::GetLang()->CreateFileWriteDummy();
  CHECK(ptrFile.IsOK());

  Ptr<iUnknown> weakPtrFile = ni_object_get_weak_ptr(ptrFile);
  CHECK(weakPtrFile.IsOK());
  CHECK_EQUAL(2, weakPtrFile->GetNumRefs()); // 2 refs, one is held by weakPtrFile and one is held by ptrFile internally
  CHECK_EQUAL(ptrFile.ptr(),weakPtrFile->QueryInterface(niGetInterfaceUUID(iFile)));

  {
    Ptr<iUnknown> weakPtr2File = ni_object_get_weak_ptr(ptrFile);
    CHECK(weakPtr2File.IsOK());
    CHECK_EQUAL(3, weakPtrFile->GetNumRefs());
    CHECK_EQUAL(3, weakPtr2File->GetNumRefs());
    CHECK_EQUAL(ptrFile.ptr(),weakPtr2File->QueryInterface(niGetInterfaceUUID(iFile)));
    // underlying weak pointer object is always the same
    CHECK_EQUAL(weakPtrFile.ptr(),weakPtr2File.ptr());
  }
  CHECK_EQUAL(2, weakPtrFile->GetNumRefs());

  ptrFile = NULL;
  CHECK_EQUAL(1, weakPtrFile->GetNumRefs()); // Now that ptrFile is gone weakPtrFile should be the only pointer holding a reference to the weak pointer

  CHECK_EQUAL((iUnknown*)NULL,weakPtrFile->QueryInterface(niGetInterfaceUUID(iFile)));
  CHECK(!weakPtrFile.IsOK());
}

TEST_FIXTURE(FWeakPtr,UtilClass) {
  Ptr<iFile> ptrFile = ni::GetLang()->CreateFileWriteDummy();
  CHECK(ptrFile.IsOK());

  WeakPtr<iFile> weakPtrFile(ptrFile);
  CHECK(weakPtrFile.IsOK());
  CHECK_EQUAL(ptrFile.ptr(),QPtr<iFile>(weakPtrFile).ptr());

  // Should not compile
  /*{
    Ptr<iFile> lockedPtr = weakPtrFile;
  }*/

  {
    QPtr<iFile> lockedPtr(weakPtrFile);
  }

  auto lockTest = [&]() {
    Nonnull<iFile> lockedPtr = niCheckNonnullSilent(lockedPtr,weakPtrFile,eFalse);
    return eTrue;
  };

  {
    WeakPtr<iFile> weakPtr2File(ptrFile);
    CHECK(weakPtr2File.IsOK());
    CHECK_EQUAL(ptrFile.ptr(),QPtr<iFile>(weakPtr2File).ptr());
    // underlying weak pointer object is always the same
    CHECK_EQUAL(weakPtrFile.weak_object_ptr(),
                weakPtr2File.weak_object_ptr());
  }

  ptrFile = NULL;

  CHECK_EQUAL((iUnknown*)NULL,QPtr<iFile>(weakPtrFile).ptr());
  CHECK(!weakPtrFile.IsOK());
}

TEST_FIXTURE(FWeakPtr,WeakPtrOfWeakPtr) {
  Ptr<iFile> ptrFile = ni::GetLang()->CreateFileWriteDummy();
  CHECK(ptrFile.IsOK());

  Ptr<iUnknown> weakPtrFile = ni_object_get_weak_ptr(ptrFile);
  CHECK(weakPtrFile.IsOK());
  CHECK_EQUAL(ptrFile.ptr(),weakPtrFile->QueryInterface(niGetInterfaceUUID(iFile)));

  {
    // This line should always assert in DEBUG builds
    Ptr<iUnknown> weakPtr2File = ni_object_get_weak_ptr(weakPtrFile);
    CHECK(weakPtr2File.IsOK());
    CHECK_EQUAL(ptrFile.ptr(),weakPtr2File->QueryInterface(niGetInterfaceUUID(iFile)));
    // underlying weak pointer object is always the same
    CHECK_EQUAL(weakPtrFile.ptr(),weakPtr2File.ptr());
  }

  ptrFile = NULL;

  CHECK_EQUAL((iUnknown*)NULL,weakPtrFile->QueryInterface(niGetInterfaceUUID(iFile)));
  CHECK(!weakPtrFile.IsOK());
}

TEST_FIXTURE(FWeakPtr,Deref) {
  Ptr<iFile> ptrFile = ni::GetLang()->CreateFileWriteDummy();
  CHECK(ptrFile.IsOK());
  CHECK_EQUAL(1, ptrFile->GetNumRefs());

  Ptr<iUnknown> weakPtrFile = ni_object_get_weak_ptr(ptrFile);
  CHECK(weakPtrFile.IsOK());
  CHECK_EQUAL(ptrFile.ptr(), weakPtrFile->QueryInterface(niGetInterfaceUUID(iFile)));
  CHECK_EQUAL(2, weakPtrFile->GetNumRefs());
  CHECK_EQUAL(1, ptrFile->GetNumRefs());

  Ptr<iUnknown> weakPtrFileDeref = WeakPtr<iUnknown>(weakPtrFile).Deref();
  CHECK_EQUAL(ptrFile.ptr(), weakPtrFileDeref.ptr());
  CHECK_EQUAL(2, ptrFile->GetNumRefs());
  CHECK_EQUAL(2, weakPtrFileDeref->GetNumRefs());
  CHECK_EQUAL(2, weakPtrFile->GetNumRefs());

  weakPtrFileDeref = NULL; ptrFile = NULL;

  CHECK_EQUAL(1, weakPtrFile->GetNumRefs());
}

TEST_FIXTURE(FWeakPtr,Nonnull) {
  Ptr<iFile> ptrFile = ni::GetLang()->CreateFileWriteDummy();
  CHECK(ptrFile.IsOK());
  CHECK_EQUAL(1, ptrFile->GetNumRefs());

  WeakPtr<iFile> wFile = ptrFile;
  CHECK(wFile.has_value());
  CHECK_EQUAL(1, ptrFile->GetNumRefs());

  {
    Nonnull<iFile> nnFile = wFile.non_null();
    CHECK_EQUAL((tIntPtr)ptrFile.ptr(), (tIntPtr)nnFile.raw_ptr());
    CHECK_EQUAL(2, ptrFile->GetNumRefs());
    CHECK_EQUAL(2, nnFile->GetNumRefs());
  }

  {
    Nonnull<const iFile> nnFile = wFile.non_null();
    CHECK_EQUAL((tIntPtr)ptrFile.ptr(), (tIntPtr)nnFile.raw_ptr());
    CHECK_EQUAL(2, ptrFile->GetNumRefs());
    CHECK_EQUAL(2, nnFile->GetNumRefs());
  }

  ptrFile = NULL;

  CHECK_EQUAL(1, wFile.weak_object_ptr()->GetNumRefs());
}

struct sMultiInherit : public cIUnknownImpl<iRunnable,eIUnknownImplFlags_Default,iMessageHandler> {
  virtual Var __stdcall Run() niImpl { return 456; }
  virtual tU64 __stdcall GetThreadID() const niImpl { return 123; }
  virtual void __stdcall HandleMessage(const tU32 anMsg, const Var& avarA, const Var& avarB) niImpl {}
};

TEST_FIXTURE(FWeakPtr,MultiInherit) {
  Ptr<sMultiInherit> mi1 { niNew sMultiInherit() };
  WeakPtr<sMultiInherit> w1 { mi1 };

  // the base address and the address of the iMessageHandler vtable are not the same once we have multiple inheritance
  CHECK_NOT_EQUAL((tIntPtr)mi1.raw_ptr(), (tIntPtr)static_cast<iMessageHandler*>(mi1.raw_ptr()));

  // the base address and the address of the iRunnable are the same because iRunnable is first in the list
  CHECK_EQUAL((tIntPtr)mi1.raw_ptr(), (tIntPtr)static_cast<iRunnable*>(mi1.raw_ptr()));

  // get iMessageHandler* from the weak pointer naively by doing a straight unsafe cast
  Ptr<iMessageHandler> r_cast_incorrect = (iMessageHandler*)ni_object_get_weak_ptr(w1.weak_object_ptr());
  // get the iMessageHandler* from the weak pointer using QPtr which will correctly do QueryInterface<iMessageHandler>(weakptr.Deref())
  QPtr<iMessageHandler> r_qi_correct = w1;

  CHECK(r_cast_incorrect.raw_ptr() != nullptr);
  CHECK(r_qi_correct.raw_ptr() != nullptr);
  CHECK_EQUAL((tIntPtr)static_cast<iMessageHandler*>(mi1.raw_ptr()), (tIntPtr)r_qi_correct.raw_ptr());
  CHECK_NOT_EQUAL((tIntPtr)static_cast<iMessageHandler*>(mi1.raw_ptr()), (tIntPtr)r_cast_incorrect.raw_ptr());

  // They are not pointing to the same base address...
  CHECK(r_qi_correct.raw_ptr() != r_cast_incorrect.raw_ptr());

  // Check that we're calling what we expect
  CHECK_EQUAL(123, r_qi_correct->GetThreadID());

  // XXX: This would crash or call the wrong function and return a random value
  // CHECK_EQUAL(123, r_cast_incorrect->GetThreadID());

  // QueryInterface<> is needed to retrieve the correct address of the
  // iMessageHandler implementation. Obviously you should not "patch after the
  // fact" a pointer in production code and use it correctly in the first
  // place, this is here only for illustration purpose.
  CHECK_EQUAL(123, ni::QueryInterface<iMessageHandler>(r_cast_incorrect.raw_ptr())->GetThreadID());
}

}
