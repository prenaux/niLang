#include "stdafx.h"
#include "../src/API/niLang/Utils/UnknownImpl.h"
#include "../src/API/niLang/ILang.h"
#include "../src/API/niLang/Utils/SmartPtr.h"
#include "../src/API/niLang/Utils/WeakPtr.h"
#include "../src/API/niLang/Utils/QPtr.h"

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

  // the next lines should fail to compile
  // weakPtrFile = ptrFile;
  // weakPtrFile = ptrFile.ptr();

  {
    QPtr<iFile> lockedPtr(weakPtrFile);
    if (lockedPtr.IsOK()) {
      // the next line should fail to compile
      // lockedPtr = NULL;
    }
  }

  {
    WeakPtr<iFile> weakPtr2File(ptrFile);
    CHECK(weakPtr2File.IsOK());
    CHECK_EQUAL(ptrFile.ptr(),QPtr<iFile>(weakPtr2File).ptr());
    // underlying weak pointer object is always the same
    CHECK_EQUAL(weakPtrFile.__GetWeakObjecIntPtr(),weakPtr2File.__GetWeakObjecIntPtr());
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

  Ptr<iUnknown> weakPtrFileDeref = WeakPtr<iUnknown>(weakPtrFile);
  CHECK_EQUAL(ptrFile.ptr(), weakPtrFileDeref.ptr());
  CHECK_EQUAL(2, ptrFile->GetNumRefs());
  CHECK_EQUAL(2, weakPtrFile->GetNumRefs());

  weakPtrFileDeref = NULL; ptrFile = NULL;

  CHECK_EQUAL(1, weakPtrFile->GetNumRefs());
}
