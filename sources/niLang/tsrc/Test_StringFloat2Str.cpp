#include "stdafx.h"
#include <niLang/StringDef.h>
#include <niLang/Utils/CollectionImpl.h>
#include <niLang/STL/utils.h>
#include <niLang/Utils/StringIntToStr.h>
#include <niLang/Math/MathFloat.h>

struct FStringFloat2Str {};

TEST_FIXTURE(FStringFloat2Str,DetectNeedCorrectDouble) {
  // Double operations detection based on target architecture.
  // Linux uses a 80bit wide floating point stack on x86. This induces double
  // rounding, which in turn leads to wrong results.
  // An easy way to test if the floating-point operations are correct is to
  // evaluate: 89255.0/1e22. If the floating-point stack is 64 bits wide then
  // the result is equal to 89255e-22.
  // The best way to test this, is to create a division-function and to compare
  // the output of the division with the expected result. (Inlining must be
  // disabled.)
  // On Linux,x86 89255e-22 != Div_double(89255.0/1e22)
  niDebugFmt(("NEED CORRECT DOUBLE: %s, 89255.0/1e22 = %f",
              (double(89255.0/1e22) == 89255e-22) ? "no" : "yes",
              double(89255.0/1e22)));
}

TEST_FIXTURE(FStringFloat2Str,Base) {

  CHECK_EQUAL(_ASTR("123.45600128173828"),ni::cString().SetDouble(123.456f,ni::eDoubleToStringMode_ShortestSingle));
  CHECK_EQUAL(_ASTR("123.456"),ni::cString().SetDouble(123.456f,ni::eDoubleToStringMode_Precision,6));
  CHECK_EQUAL(_ASTR("-123.45600128173828"),ni::cString().SetDouble(-123.456f,ni::eDoubleToStringMode_ShortestSingle));
  CHECK_EQUAL(_ASTR("1237894700"),ni::cString().SetDouble((ni::tF64)1237894598.78978978));

  CHECK_EQUAL(_ASTR("-1237894700"),ni::cString().SetDouble((ni::tF64)-1237894598.78978978));

  CHECK_EQUAL(1.23789e+9, ni::StringToDouble("1.23789e+9"));

  CHECK_EQUAL(_ASTR("1237894598"),ni::cString().SetDouble((ni::tF64)1237894598));
  CHECK_EQUAL(1237894598, ni::StringToDouble("1237894598"));
  CHECK_EQUAL(_ASTR("-1237894598"),ni::cString().SetDouble((ni::tF64)-1237894598));

  const ni::tF64 fActionScriptMaxDouble = 1.79769313486231e+308;
  const ni::tF64 fActionScriptMinDouble = 4.94065645841247e-324;

  CHECK_EQUAL(fActionScriptMaxDouble, ni::StringToDouble("1.79769313486231e+308"));
  CHECK_EQUAL(fActionScriptMinDouble, ni::StringToDouble("4.94065645841247e-324"));

  CHECK_EQUAL(_ASTR("1.79769313486231e+308"), ni::cString().SetDouble(fActionScriptMaxDouble,ni::eDoubleToStringMode_Exponential,14));
  CHECK_EQUAL(_ASTR("1.79769313486231e+308"), ni::cString().Format("%.14e",fActionScriptMaxDouble));
  CHECK_EQUAL(_ASTR("4.94065645841247e-324"), ni::cString().SetDouble(fActionScriptMinDouble,ni::eDoubleToStringMode_Exponential,14));
  CHECK_EQUAL(_ASTR("4.94065645841247e-324"), ni::cString().Format("%.14e",fActionScriptMinDouble));

  CHECK(ni::IsNaN(ni::StringToDouble("nan")));
  CHECK(ni::IsNaN(ni::StringToDouble("NaN")));
  CHECK(ni::IsNaN(ni::StringToDouble("NAN")));
  CHECK_EQUAL(ni::TypeInfinity<ni::tF64>(), ni::StringToDouble("Infinity"));
  CHECK_EQUAL(ni::TypeInfinity<ni::tF64>(), ni::StringToDouble("infinity"));
  CHECK_EQUAL(ni::TypeInfinity<ni::tF64>(), ni::StringToDouble("INFINITY"));
  CHECK_EQUAL(-ni::TypeInfinity<ni::tF64>(), ni::StringToDouble("-Infinity"));
  CHECK_EQUAL(-ni::TypeInfinity<ni::tF64>(), ni::StringToDouble("-infinity"));
  CHECK_EQUAL(-ni::TypeInfinity<ni::tF64>(), ni::StringToDouble("-INFINITY"));

  CHECK(ni::IsNaN(_ASTR("NaN").Double()));
  CHECK(ni::IsNaN(_ASTR("NaN").Float()));
  CHECK_EQUAL(ni::TypeInfinity<ni::tF64>(), _ASTR("Infinity").Double());
  CHECK_EQUAL(ni::TypeInfinity<ni::tF32>(), _ASTR("Infinity").Float());

  const char* testNum = "123e3hello";
  const char* endp = NULL;
  CHECK_EQUAL((ni::tF64)123000,ni::StrToD(testNum,&endp));
  CHECK_EQUAL('h',*endp);

  ni::tInt charCount = ni::eInvalidHandle;
  CHECK_EQUAL((ni::tF64)123000,ni::StringToDouble(testNum,0,&charCount));
  CHECK_EQUAL(5,charCount);
  CHECK_EQUAL('h',testNum[charCount]);
}
