#include "stdafx.h"
#include "../src/API/niLang/Utils/Buffer.h"

using namespace ni;

//
// We generally don't use C++ RTTI but some thirdparty libraries we might
// wanna use could use it. This test is just meant to test that the compiler
// can actually compile and run basic RTTI code correctly. (This used to not
// be the case on the first few iteration of the Android NDK for example...)
//
namespace {

struct FRTTI {
};

// initialization of base class
class B {
 public:
  // Fix "error: '(anonymous namespace)::B' is not polymorphic". You got to
  // have at least one virtual function in your base class to use dynamic_cast
  // or to make it polymorphic.
  virtual ~B() {}
};

// initialization of derived class
class D : public B {
 public:
  virtual ~D() {}
};

TEST_FIXTURE(FRTTI,Basic) {
    B* b = new D; // Base class pointer
    D* d = dynamic_cast<D*>(b); // Derived class pointer
    CHECK_NOT_EQUAL(nullptr, d);
    delete b;
}

}
