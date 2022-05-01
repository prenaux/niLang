#include "stdafx.h"
#include "Test_String.h"
#include "../src/API/niLang/ICrypto.h"

struct FCryptoHash {
};

using namespace ni;

TEST_FIXTURE(FCryptoHash,MD5) {
  const ni::tU32 dataSize = GetTestDataStringSize(eTestDataString_PathFull)-1; // -1 don't include the terminating zero
  const ni::achar* const data = GetTestDataString(eTestDataString_PathFull);
  QPtr<iCryptoHash> hash = New_niLang_CryptoHash("MD5",niVarNull);
  hash->UpdateRaw((tPtr)data,dataSize);
  cString res = hash->FinalString(eRawToStringEncoding_Hex);
  TEST_DEBUGFMT("MD5: %s -> %s", data, res);
  CHECK_EQUAL(_ASTR("1a6b48c841b77528c637d428ee3b7b80"), res);
}

TEST_FIXTURE(FCryptoHash,RIPEMD160) {
  const ni::tU32 dataSize = GetTestDataStringSize(eTestDataString_PathFull)-1; // -1 don't include the terminating zero
  const ni::achar* const data = GetTestDataString(eTestDataString_PathFull);
  QPtr<iCryptoHash> hash = New_niLang_CryptoHash("RIPEMD160",niVarNull);
  hash->UpdateRaw((tPtr)data,dataSize);
  cString res = hash->FinalString(eRawToStringEncoding_Hex);
  TEST_DEBUGFMT("RIPEMD160: %s -> %s", data, res);
  CHECK_EQUAL(_ASTR("cf7432c273dbeb22cafbbd9053cb0b7e408b37f3"), res);
}

TEST_FIXTURE(FCryptoHash,SHA1) {
  const ni::tU32 dataSize = GetTestDataStringSize(eTestDataString_PathFull)-1; // -1 don't include the terminating zero
  const ni::achar* const data = GetTestDataString(eTestDataString_PathFull);
  QPtr<iCryptoHash> hash = New_niLang_CryptoHash("SHA1",niVarNull);
  hash->UpdateRaw((tPtr)data,dataSize);
  cString res = hash->FinalString(eRawToStringEncoding_Hex);
  TEST_DEBUGFMT("SHA1: %s -> %s", data, res);
  CHECK_EQUAL(_ASTR("ab3b8d91ca977cbc8ff1e617427dc9e0f63b782b"), res);
}

TEST_FIXTURE(FCryptoHash,SHA256) {
  const ni::tU32 dataSize = GetTestDataStringSize(eTestDataString_PathFull)-1; // -1 don't include the terminating zero
  const ni::achar* const data = GetTestDataString(eTestDataString_PathFull);
  QPtr<iCryptoHash> hash = New_niLang_CryptoHash("SHA256",niVarNull);
  hash->UpdateRaw((tPtr)data,dataSize);
  cString res = hash->FinalString(eRawToStringEncoding_Hex);
  TEST_DEBUGFMT("SHA256: %s -> %s", data, res);
  CHECK_EQUAL(_ASTR("e770ff443471bb0e279a0c8ce793d556ea81a87bb6740b6d5daa591361ce04e5"), res);
}

TEST_FIXTURE(FCryptoHash,SHA384) {
  const ni::tU32 dataSize = GetTestDataStringSize(eTestDataString_PathFull)-1; // -1 don't include the terminating zero
  const ni::achar* const data = GetTestDataString(eTestDataString_PathFull);
  QPtr<iCryptoHash> hash = New_niLang_CryptoHash("SHA384",niVarNull);
  hash->UpdateRaw((tPtr)data,dataSize);
  cString res = hash->FinalString(eRawToStringEncoding_Hex);
  TEST_DEBUGFMT("SHA384: %s -> %s", data, res); CHECK_EQUAL(_ASTR("f785f66618308d1cc08fc1bc7c86483d6f02e5111370b5bd3c9c2e4db4aba7efc641a89d7a89a969f5974f05fbca42a6"), res);
}

TEST_FIXTURE(FCryptoHash,SHA512) {
  const ni::tU32 dataSize = GetTestDataStringSize(eTestDataString_PathFull)-1; // -1 don't include the terminating zero
  const ni::achar* const data = GetTestDataString(eTestDataString_PathFull);
  QPtr<iCryptoHash> hash = New_niLang_CryptoHash("SHA512",niVarNull);
  hash->UpdateRaw((tPtr)data,dataSize);
  cString res = hash->FinalString(eRawToStringEncoding_Hex);
  TEST_DEBUGFMT("SHA512: %s -> %s", data, res); CHECK_EQUAL(_ASTR("625408ee195a4a6881acf1ac3d7413eadacced4427321069c4c41d5d6e9685a9cfb49bf68b640580d3c14acb31b427a79bb823839f96f9b164a47fe5534f88d9"), res);

  // test the truncated variants
  {
    CHECK_EQUAL(_ASTR("625408ee195a4a6881acf1ac3d7413ea"),
                ni::GetCrypto()->DigestRaw(
                  (ni::tPtr)data,dataSize,"sha512_128",eRawToStringEncoding_Hex));

    CHECK_EQUAL(_ASTR("625408ee195a4a6881acf1ac3d7413eadacced44"),
                ni::GetCrypto()->DigestRaw(
                  (ni::tPtr)data,dataSize,"sha512_160",eRawToStringEncoding_Hex));

    CHECK_EQUAL(_ASTR("625408ee195a4a6881acf1ac3d7413eadacced4427321069c4c41d5d6e9685a9"),
                ni::GetCrypto()->DigestRaw(
                  (ni::tPtr)data,dataSize,"sha512_256",eRawToStringEncoding_Hex));
  }
}

TEST_FIXTURE(FCryptoHash,FinalStringEncode) {
  const ni::tU32 dataSize = GetTestDataStringSize(eTestDataString_PathFull)-1; // -1 don't include the terminating zero
  const ni::achar* const data = GetTestDataString(eTestDataString_PathFull);
  QPtr<iCryptoHash> hash = New_niLang_CryptoHash("MD5",niVarNull);

  {
    hash->UpdateRaw((tPtr)data,dataSize);
    cString res = hash->FinalString(eRawToStringEncoding_Hex);
    niDebugFmt(("... res Hex: %s", res));
    CHECK_EQUAL(res.size(), StrLen(res.c_str()));
  }

  {
    hash->UpdateRaw((tPtr)data,dataSize);
    cString res = hash->FinalString(eRawToStringEncoding_Base32);
    niDebugFmt(("... res Base32: %s", res));
    CHECK_EQUAL(res.size(), StrLen(res.c_str()));
  }

  {
    hash->UpdateRaw((tPtr)data,dataSize);
    cString res = hash->FinalString(eRawToStringEncoding_Base64);
    niDebugFmt(("... res Base64: %s", res));
    CHECK_EQUAL(res.size(), StrLen(res.c_str()));
  }
}
