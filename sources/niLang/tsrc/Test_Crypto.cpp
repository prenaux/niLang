#include "stdafx.h"
#include "../src/API/niLang/ICrypto.h"
#include "Test_String.h"

using namespace ni;

struct FCrypto {
};

iFile* Rewind(iFile* apFile, tSize aSeekSetAt = 0) {
  /*tBool r = */ apFile->SeekSet(aSeekSetAt);
  return apFile;
}

TEST_FIXTURE(FCrypto,Rand) {
  tInt outputCount = 0;

  CHECK(niIsOK(ni::GetCrypto()->GetRand()));

  Ptr<iCryptoRand> rnd = ni::GetCrypto()->CreateRand();
  CHECK(rnd.IsOK());
  CHECK(rnd.ptr() != ni::GetCrypto()->GetRand());

  {
    cString s1, s2, s3;
    Ptr<iFile> fp = ni::CreateFileDynamicMemory(0,"");
    CHECK_EQUAL(10, rnd->RandFile(Rewind(fp), 10));
    s1 = Rewind(fp)->ReadRawToString(eRawToStringEncoding_Hex, 10);
    TEST_DEBUGFMT("s1: %s", s1);
    CHECK_EQUAL(10, rnd->RandFile(Rewind(fp), 10));
    s2 = Rewind(fp)->ReadRawToString(eRawToStringEncoding_Hex, 10);
    TEST_DEBUGFMT("s2: %s", s2);
    CHECK_EQUAL(10, rnd->RandFile(Rewind(fp), 10));
    s3 = Rewind(fp)->ReadRawToString(eRawToStringEncoding_Hex, 10);
    TEST_DEBUGFMT("s3: %s", s3);
    CHECK_NOT_EQUAL(s1, s2);
    CHECK_NOT_EQUAL(s1, s3);
    CHECK_NOT_EQUAL(s2, s3);
  }

  {
    cString s1, s2, s3;
    const cString pathOut = UnitTest::GetTestOutputFilePath(niFmt("%s_%d.bin", m_testName, ++outputCount));
    {
      Ptr<iFile> fpOut = GetRootFS()->FileOpen(pathOut.Chars(),eFileOpenMode_Write);
      CHECK_EQUAL(10, rnd->RandFile(fpOut, 10));
      CHECK_EQUAL(10, rnd->RandFile(fpOut, 10));
      CHECK_EQUAL(10, rnd->RandFile(fpOut, 10));
    }

    Ptr<iFile> fp = GetRootFS()->FileOpen(pathOut.Chars(),eFileOpenMode_Read);
    s1 = fp->ReadRawToString(eRawToStringEncoding_Hex, 10);
    TEST_DEBUGFMT("s1: %s", s1);
    s2 = fp->ReadRawToString(eRawToStringEncoding_Hex, 10);
    TEST_DEBUGFMT("s2: %s", s2);
    s3 = fp->ReadRawToString(eRawToStringEncoding_Hex, 10);
    TEST_DEBUGFMT("s3: %s", s3);
    CHECK_NOT_EQUAL(s1, s2);
    CHECK_NOT_EQUAL(s1, s3);
    CHECK_NOT_EQUAL(s2, s3);
  }

  {
    const tUUID r1 = rnd->RandUUID();
    TEST_DEBUGFMT("r1: %s", r1);
    const tUUID r2 = rnd->RandUUID();
    TEST_DEBUGFMT("r2: %s", r2);
    const tUUID r3 = rnd->RandUUID();
    TEST_DEBUGFMT("r3: %s", r3);
    CHECK_NOT_EQUAL(r1, kuuidZero);
    CHECK_NOT_EQUAL(r2, kuuidZero);
    CHECK_NOT_EQUAL(r3, kuuidZero);
    CHECK_NOT_EQUAL(r1, r2);
    CHECK_NOT_EQUAL(r1, r3);
    CHECK_NOT_EQUAL(r2, r3);
  }

  {
    const tI64 i1 = rnd->RandInt();
    TEST_DEBUGFMT("i1: %s", i1);
    const tI64 i2 = rnd->RandInt();
    TEST_DEBUGFMT("i2: %s", i2);
    const tI64 i3 = rnd->RandInt();
    TEST_DEBUGFMT("i3: %s", i3);
    CHECK_NOT_EQUAL(i1, 0);
    CHECK_NOT_EQUAL(i2, 0);
    CHECK_NOT_EQUAL(i3, 0);
    CHECK_NOT_EQUAL(i1, i2);
    CHECK_NOT_EQUAL(i1, i3);
    CHECK_NOT_EQUAL(i2, i3);
  }

  {
    const tF64 f1 = rnd->RandFloat();
    TEST_DEBUGFMT("f1: %s", f1);
    const tF64 f2 = rnd->RandFloat();
    TEST_DEBUGFMT("f2: %s", f2);
    const tF64 f3 = rnd->RandFloat();
    TEST_DEBUGFMT("f3: %s", f3);
    CHECK_NOT_EQUAL(f1, 0.0);
    CHECK_NOT_EQUAL(f2, 0.0);
    CHECK_NOT_EQUAL(f3, 0.0);
    CHECK_NOT_EQUAL(f1, f2);
    CHECK_NOT_EQUAL(f1, f3);
    CHECK_NOT_EQUAL(f2, f3);
  }
}

/*
  Reproduce:
  SELECT gen_salt('bf',8);
  -> $2a$08$VXQoSyZF288.82vVBGuEN.
  SELECT crypt('12345', '$2a$08$VXQoSyZF288.82vVBGuEN.');
  -> $2a$08$VXQoSyZF288.82vVBGuEN.1NasswyF6K7sDZxyH/xGuLP4BEg26yK
  SELECT crypt('12345', '$2a$08$VXQoSyZF288.82vVBGuEN.1NasswyF6K7sDZxyH/xGuLP4BEg26yK');
  -> $2a$08$VXQoSyZF288.82vVBGuEN.1NasswyF6K7sDZxyH/xGuLP4BEg26yK
  SELECT crypt('12346', '$2a$08$VXQoSyZF288.82vVBGuEN.1NasswyF6K7sDZxyH/xGuLP4BEg26yK');
  -> $2a$08$VXQoSyZF288.82vVBGuEN.M.IXPe1VRXW.buvGhP/HJXQfPew.soe
*/
TEST_FIXTURE(FCrypto,KDF) {
  char buff[knCryptoKDFCryptMaxLen] = {0};
  const char* salt = "$2a$08$VXQoSyZF288.82vVBGuEN.";

  const cString hash12345 = "$2a$08$VXQoSyZF288.82vVBGuEN.1NasswyF6K7sDZxyH/xGuLP4BEg26yK";
  const cString hash12346 = "$2a$08$VXQoSyZF288.82vVBGuEN.M.IXPe1VRXW.buvGhP/HJXQfPew.soe";

  cString r1 = CryptoKDFCrypt("12345", salt, buff);
  CHECK_EQUAL(hash12345, r1);

  cString r2 = CryptoKDFCrypt("12345", r1.Chars(), buff);
  CHECK_EQUAL(hash12345, r2);

  cString r3 = CryptoKDFCrypt("12346", r1.Chars(), buff);
  CHECK_EQUAL(hash12346, r3);

  cString salt1 = CryptoKDFGenSaltBlowfish(GetCryptoRand(), 4, buff);
  TEST_DEBUGFMT("salt1: %s", salt1);
  CHECK(salt1.StartsWith("$2a$04$"));
  cString salt2 = CryptoKDFGenSaltBlowfish(GetCryptoRand(), 8, buff);
  TEST_DEBUGFMT("salt2: %s", salt2);
  CHECK(salt2.StartsWith("$2a$08$"));
  cString salt3 = CryptoKDFGenSaltBlowfish(GetCryptoRand(), 31, buff);
  TEST_DEBUGFMT("salt3: %s", salt3);
  CHECK(salt3.StartsWith("$2a$31$"));
  CHECK_NOT_EQUAL(salt1, salt2);
  CHECK_NOT_EQUAL(salt1, salt3);
  CHECK_NOT_EQUAL(salt2, salt3);

  cString saltInvalid = CryptoKDFGenSaltBlowfish(GetCryptoRand(), 1, buff);
  CHECK(saltInvalid.IsEmpty());
  saltInvalid = CryptoKDFGenSaltBlowfish(GetCryptoRand(), 3, buff);
  CHECK(saltInvalid.IsEmpty());
  saltInvalid = CryptoKDFGenSaltBlowfish(GetCryptoRand(), 32, buff);
  CHECK(saltInvalid.IsEmpty());
}

TEST_FIXTURE(FCrypto,Digest) {
  const ni::achar* const data = GetTestDataString(eTestDataString_PathFull);

  CHECK_EQUAL(_ASTR("1a6b48c841b77528c637d428ee3b7b80"),
              ni::GetCrypto()->Digest(data,"md5"));
  CHECK_EQUAL(_ASTR("cf7432c273dbeb22cafbbd9053cb0b7e408b37f3"),
              ni::GetCrypto()->Digest(data,"ripemd160"));
  CHECK_EQUAL(_ASTR("ab3b8d91ca977cbc8ff1e617427dc9e0f63b782b"),
              ni::GetCrypto()->Digest(data,"sha1"));
}

TEST_FIXTURE(FCrypto,SigVerify) {
  const cString lic_plan = "plan_vlk_free";
  const cString keyver = "v1";
  const cString digest = "checklic";
  const cString sig_tm = "1539332420";
  const cString username = "thexash";
  cString sig = "1d3c2df1f18a9ea2bf389c9217ab01807c07f3fe11ba4672a9298708506c365bff496c04c04374628fd7a244519f44e2ab89f3a67e2f4d750b426363e83473f67ea239ce7abf3e2673b34966f24e4ecb2a6d11cd2847af2b590390dbf34463f099092effb0f883b7f483eefcc5752933949a0d896439e2f33682c5ef3de0486302030559d2068374d7d43a4ddd56c6f8d37289e7f51056cd19645e952be8cba7c52a5801046a3252c0d2245e52dd640c6698396cb24c9fcc16c6a2f703a23ea596964be9c095c1597d2d77e5101747fcfbe88b48c581c83308499ac48717ab7cf898b470af3fd0a6cc773be299d9bdeef75a72d778ad317d2a7911724141e0df";

  const char* pubkey =
    "-----BEGIN PUBLIC KEY-----\n"
    "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAq3rVPL46aEIJBCxkiVmL\n"
    "h2zMCQRQ3qMXxMK5+gF3IwnZmmTy4ZVvswJ9TYvJbRs9Fdsf+ylXZyIV5YxiwQel\n"
    "SVOPvEJgwbtwc7548nfX+dGJd9IKiCFzWfD9ni8LsxOXLOmlorTVAk2euILiT6f9\n"
    "yIKnykdRsTP8gr3Op+zr3wecx4t7VHB95cGw055sYCVM5WvsF7DlIB+z+bgB5qN0\n"
    "M1Wu2S/1MxbV9b4irElnwiLiWWOkVDIuP8PZ/ukRm/+QCfBoRt4gORl8VtXbiMrb\n"
    "cYkVL9lwC39fDe5PBgXH5vfFeSg4FI8bSIe5QMB0kmsCzIL/Jho/qnThyk/limrE\n"
    "SwIDAQAB\n"
    "-----END PUBLIC KEY-----\n";

  const cString payload = lic_plan + ";" + keyver + ";" + digest + ";" + sig_tm + ";" + username + ";";

  int result;

  // Verify should succeed
  result = ni::GetCrypto()->SigVerify(sig.Chars(), pubkey, payload.begin());
  CHECK_EQUAL(1, result);

  // Mess around with our signature, verify should fail
  *sig.data() = 'a';
  result = ni::GetCrypto()->SigVerify(sig.Chars(), pubkey, payload.begin());
  CHECK_EQUAL(0, result);
}
