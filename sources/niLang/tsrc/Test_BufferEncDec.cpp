#include "stdafx.h"
#include "Test_String.h"
#include "../src/API/niLang/IFile.h"
#include "../src/API/niLang/STL/scope_guard.h"
#include "../src/mbedtls/include/mbedtls/base64.h"
#include "./data/montmay12.bmp.hxx"

using namespace ni;

struct FBufferEncDec {
};

TEST_FIXTURE(FBufferEncDec,Base32) {
  const ni::tU32 dataSize = GetTestDataStringSize(eTestDataString_PathFull);
  const ni::achar* const data = GetTestDataString(eTestDataString_PathFull);
  const tInt encodedLen = Base32EncodeOutputSize(dataSize);
  TEST_DEBUGFMT("... dataSize: %s -> encodedLen: %s", dataSize, encodedLen);
  char* encoded = (char*)niMalloc(encodedLen);
  ASTL_SCOPE_EXIT {
    niFree(encoded);
  };

  tInt encodeResSize = Base32Encode((tPtr)encoded, encodedLen, (tPtr)data, dataSize);
  CHECK((encodeResSize+1) <= encodedLen);
  TEST_DEBUGFMT("data: %s -> encoded: %s", data, encoded);
  CHECK_EQUAL(
    _ASTR("8cx2yh3fcdtptsbeehsj0rbecgg56sbmehmpwsvk5x7qevk5e8qmty908hqp6xbdcnq78wsfc5h66s34cnk6et39regc7ae3n31tfgxswa1asgnnrare9fd0wpjvvknhsts2yhv1f5gjyx35edte9fd0wpjvtbkmf1t00"),
    _ASTR(encoded));

  const tInt decodedLen = Base32DecodeOutputSize(encoded, StrLen(encoded));
  TEST_DEBUGFMT("... encodedLen: %s -> decodedLen: %s", encodedLen, decodedLen);
  char* decoded = (char*)niMalloc(decodedLen);
  ASTL_SCOPE_EXIT {
    niFree(decoded);
  };

  tInt decodeResSize = Base32Decode((tPtr)decoded, decodedLen, encoded, StrLen(encoded));
  CHECK_EQUAL(ni::StrSizeZ(data), decodeResSize);
  TEST_DEBUGFMT("decoded: %s", decoded);
  CHECK_EQUAL(
    _ASTR("C:/Documents and Settings/Owner/My Documents/abcddefghiàéèçù€µ°你好αβ/Gaya/test你好.txt"),
    _ASTR(decoded));

}

TEST_FIXTURE(FBufferEncDec,Base32DecodeFormatted) {
  {
    const char* encoded = "8CX2YH3FCDTPTSBEEHSJ0RBECGG56SBMEHMPWSVK5X7QEVK5E8QMTY908HQP6XBDCNQ78WSFC5H66S34CNK6ET39REGC7AE3N31TFGXSWA1ASGNNRARE9FD0WPJVVKNHSTS2YHV1F5GJYX35EDTE9FD0WPJVTBKMF1T00";

    const tInt decodedLen = Base32DecodeOutputSize(encoded, 0);
    TEST_DEBUGFMT("... decodedLen: %s", decodedLen);
    char* decoded = (char*)niMalloc(decodedLen);
    ASTL_SCOPE_EXIT {
      niFree(decoded);
    };

    tInt decodeResSize = Base32Decode((tPtr)decoded, decodedLen, encoded, 0);
    CHECK_EQUAL(103, decodeResSize);
    TEST_DEBUGFMT("decoded: %s", decoded);
    CHECK_EQUAL(
      _ASTR("C:/Documents and Settings/Owner/My Documents/abcddefghiàéèçù€µ°你好αβ/Gaya/test你好.txt"),
      _ASTR(decoded));
  }

  {
    const char* encoded = "8CX2YH3FCDTPTSBEEHSJ0RBECGG56SBMEHMPWSVK5X7QEVK5E8QMTY908HQP6XBDCNQ78WSFC5H66S34CNK6ET39REGC7AE3N31TFGXSWA1ASGNNRARE9FD0WPJVVKNHSTS2YHV1F5GJYX35EDTE9FD0WPJVTBKMF1T00===";

    const tInt decodedLen = Base32DecodeOutputSize(encoded, 0);
    TEST_DEBUGFMT("... decodedLen: %s", decodedLen);
    char* decoded = (char*)niMalloc(decodedLen);
    ASTL_SCOPE_EXIT {
      niFree(decoded);
    };

    tInt decodeResSize = Base32Decode((tPtr)decoded, decodedLen, encoded, 0);
    CHECK_EQUAL(103, decodeResSize);
    TEST_DEBUGFMT("decoded: %s", decoded);
    CHECK_EQUAL(
      _ASTR("C:/Documents and Settings/Owner/My Documents/abcddefghiàéèçù€µ°你好αβ/Gaya/test你好.txt"),
      _ASTR(decoded));
  }

  {
    const char* encoded = "8CX2YH3FCDTPTSBEEHSJ0RBECGG56SBMEHMPWSVK5X7QEVK5E8QMTY908HQP6XBDCNQ78WSFC5H66S34CNK6ET39REGC7AE3N31TFGXSWA1ASGNNRARE9FD0WPJVVKNHSTS2YHV1F5GJYX35EDTE9FD0WPJVTBKMF1T00====================";

    const tInt decodedLen = Base32DecodeOutputSize(encoded, 0);
    TEST_DEBUGFMT("... decodedLen: %s", decodedLen);
    char* decoded = (char*)niMalloc(decodedLen);
    ASTL_SCOPE_EXIT {
      niFree(decoded);
    };

    tInt decodeResSize = Base32Decode((tPtr)decoded, decodedLen, encoded, 0);
    CHECK_EQUAL(103, decodeResSize);
    TEST_DEBUGFMT("decoded: %s", decoded);
    CHECK_EQUAL(
      _ASTR("C:/Documents and Settings/Owner/My Documents/abcddefghiàéèçù€µ°你好αβ/Gaya/test你好.txt"),
      _ASTR(decoded));
  }

  {
    const char* encoded = "=====8CX2YH3FCDTPTSB--EEHSJ0RBECG-G56SBMEHMPWSVK5X7QEV-K5E8QMTY908HQ\nP6XBDC_______________________________________NQ78WSFC5H66S34C__NK6ET39REGC7AE3N31TFGXSWA1ASGNNRARE9FD0WPJVVKNHSTS2YHV1F5GJYX35EDTE9FD0WPJVTBKMF1T00====================";

    const tInt decodedLen = Base32DecodeOutputSize(encoded, 0);
    TEST_DEBUGFMT("... decodedLen: %s", decodedLen);
    char* decoded = (char*)niMalloc(decodedLen);
    ASTL_SCOPE_EXIT {
      niFree(decoded);
    };

    tInt decodeResSize = Base32Decode((tPtr)decoded, decodedLen, encoded, 0);
    CHECK_EQUAL(103, decodeResSize);
    TEST_DEBUGFMT("decoded: %s", decoded);
    CHECK_EQUAL(
      _ASTR("C:/Documents and Settings/Owner/My Documents/abcddefghiàéèçù€µ°你好αβ/Gaya/test你好.txt"),
      _ASTR(decoded));
  }

  {
    const char* encoded = "8cx2yh3fcdtptsbeehsj0rbecgg56sbmehmpwsvk5x7qevk5e8qmty908hqp6xbdcnq78wsfc5h66s34cnk6et39regc7ae3n31tfgxswa1asgnnrare9fd0wpjvvknhsts2yhv1f5gjyx35edte9fd0wpjvtbkmf1t0O";

    const tInt decodedLen = Base32DecodeOutputSize(encoded, 0);
    TEST_DEBUGFMT("... decodedLen: %s", decodedLen);
    char* decoded = (char*)niMalloc(decodedLen);
    ASTL_SCOPE_EXIT {
      niFree(decoded);
    };

    tInt decodeResSize = Base32Decode((tPtr)decoded, decodedLen, encoded, 0);
    CHECK_EQUAL(103, decodeResSize);
    TEST_DEBUGFMT("decoded: %s", decoded);
    CHECK_EQUAL(
      _ASTR("C:/Documents and Settings/Owner/My Documents/abcddefghiàéèçù€µ°你好αβ/Gaya/test你好.txt"),
      _ASTR(decoded));
  }

  {
    const char* encoded = "8cx2yh3fcdtptsbeehsj0rbecgg56sbmehmpwsvk5x7qevk5e8qmty908hqp6xbdcnq78wsfc5h66s34cnk6et39regc7ae3n3Itfgxswaiasgnnrare9fd0wpjvvknhsts2yhvLf5gjyx35edte9fd0wpjvtbkmfltOo";

    const tInt decodedLen = Base32DecodeOutputSize(encoded, 0);
    TEST_DEBUGFMT("... decodedLen: %s", decodedLen);
    char* decoded = (char*)niMalloc(decodedLen);
    ASTL_SCOPE_EXIT {
      niFree(decoded);
    };

    tInt decodeResSize = Base32Decode((tPtr)decoded, decodedLen, encoded, 0);
    CHECK_EQUAL(103, decodeResSize);
    TEST_DEBUGFMT("decoded: %s", decoded);
    CHECK_EQUAL(
      _ASTR("C:/Documents and Settings/Owner/My Documents/abcddefghiàéèçù€µ°你好αβ/Gaya/test你好.txt"),
      _ASTR(decoded));
  }
}

TEST_FIXTURE(FBufferEncDec,Base64) {
  const ni::tU32 dataSize = GetTestDataStringSize(eTestDataString_PathFull);
  const ni::achar* const data = GetTestDataString(eTestDataString_PathFull);
  const tInt encodedLen = Base64EncodeOutputSize(dataSize);
  TEST_DEBUGFMT("... dataSize: %s -> encodedLen: %s", dataSize, encodedLen);
  char* encoded = (char*)niMalloc(encodedLen);
  ASTL_SCOPE_EXIT {
    niFree(encoded);
  };

  tInt encodeResSize = Base64Encode((tPtr)encoded, encodedLen, (tPtr)data, dataSize);
  CHECK_EQUAL(encodedLen, encodeResSize+1);
  TEST_DEBUGFMT("data: %s -> encoded: %s", data, encoded);
  CHECK_EQUAL(
    _ASTR("QzovRG9jdW1lbnRzIGFuZCBTZXR0aW5ncy9Pd25lci9NeSBEb2N1bWVudHMvYWJjZGRlZmdoacOgw6nDqMOnw7nigqzCtcKw5L2g5aW9zrHOsi9HYXlhL3Rlc3TkvaDlpb0udHh0AA=="),
    _ASTR(encoded));

  const tInt decodedLen = Base64DecodeOutputSize(encoded, StrLen(encoded));
  TEST_DEBUGFMT("... encodedLen: %s -> decodedLen: %s", encodedLen, decodedLen);
  char* decoded = (char*)niMalloc(decodedLen);
  ASTL_SCOPE_EXIT {
    niFree(decoded);
  };

  tInt decodeResSize = Base64Decode((tPtr)decoded, decodedLen, encoded, StrLen(encoded));
  CHECK_EQUAL(decodedLen, decodeResSize);
  CHECK_EQUAL(ni::StrSizeZ(data), decodeResSize);
  TEST_DEBUGFMT("decoded: %s", decoded);
  CHECK_EQUAL(
    _ASTR("C:/Documents and Settings/Owner/My Documents/abcddefghiàéèçù€µ°你好αβ/Gaya/test你好.txt"),
    _ASTR(decoded));

}

TEST_FIXTURE(FBufferEncDec,Base64DecodeFormatted) {
  {
    const char* encoded = "QzovRG9jdW1lbnRzIGFuZCBTZXR0aW5ncy9Pd25lci9NeSBEb2N1bWVudHMvYWJjZGRlZmdoacOgw6nDqMOnw7nigqzCtcKw5L2g5aW9zrHOsi9HYXlhL3Rlc3TkvaDlpb0udHh0AA==";

    const tInt decodedLen = Base64DecodeOutputSize(encoded, 0);
    TEST_DEBUGFMT("... decodedLen: %s", decodedLen);
    char* decoded = (char*)niMalloc(decodedLen);
    ASTL_SCOPE_EXIT {
      niFree(decoded);
    };

    tInt decodeResSize = Base64Decode((tPtr)decoded, decodedLen, encoded, 0);
    CHECK_EQUAL(decodedLen, decodeResSize);
    TEST_DEBUGFMT("decoded: %s", decoded);
    CHECK_EQUAL(
      _ASTR("C:/Documents and Settings/Owner/My Documents/abcddefghiàéèçù€µ°你好αβ/Gaya/test你好.txt"),
      _ASTR(decoded));
  }

  // Add an explicit "end zero"
  {
    const char* encoded = "QzovRG9jdW1lbnRzIGFuZCBTZXR0aW5ncy9Pd25lci9NeSBEb2N1bWVudHMvYWJjZGRlZmdoacOgw6nDqMOnw7nigqzCtcKw5L2g5aW9zrHOsi9HYXlhL3Rlc3TkvaDlpb0udHh0AA==\0";
    const tInt encodedLen = StrLen(encoded)+1; // +1 for the end zero

    const tInt decodedLen = Base64DecodeOutputSize(encoded, encodedLen);
    TEST_DEBUGFMT("... decodedLen: %s", decodedLen);
    char* decoded = (char*)niMalloc(decodedLen);
    ASTL_SCOPE_EXIT {
      niFree(decoded);
    };

    tInt decodeResSize = Base64Decode((tPtr)decoded, decodedLen, encoded, encodedLen);
    CHECK_EQUAL(decodedLen, decodeResSize);
    TEST_DEBUGFMT("decoded: %s", decoded);
    CHECK_EQUAL(
      _ASTR("C:/Documents and Settings/Owner/My Documents/abcddefghiàéèçù€µ°你好αβ/Gaya/test你好.txt"),
      _ASTR(decoded));
  }

  // Add some spaces, new lines, etc.. in the string
  {
    const char* encoded = "QzovRG9jdW1lbnRzIGFuZCB  TZXR0aW5ncy9Pd25lc \n\ri9NeSBEb2N1bWVudHMvYWJjZGRlZmdoacOgw6nDqMOnw7nigqzCtcKw5L2g5aW\t9zrHOsi9HYXlhL3Rlc3TkvaDlpb0udHh0AA==\0";
    const tInt encodedLen = StrLen(encoded)+1; // +1 for the end zero

    const tInt decodedLen = Base64DecodeOutputSize(encoded, encodedLen);
    TEST_DEBUGFMT("... decodedLen: %s", decodedLen);
    char* decoded = (char*)niMalloc(decodedLen);
    ASTL_SCOPE_EXIT {
      niFree(decoded);
    };

    tInt decodeResSize = Base64Decode((tPtr)decoded, decodedLen, encoded, encodedLen);
    CHECK_EQUAL(decodedLen, decodeResSize);
    TEST_DEBUGFMT("decoded: %s", decoded);
    CHECK_EQUAL(
      _ASTR("C:/Documents and Settings/Owner/My Documents/abcddefghiàéèçù€µ°你好αβ/Gaya/test你好.txt"),
      _ASTR(decoded));
  }
}

TEST_FIXTURE(FBufferEncDec,Hex) {
  const ni::tU32 dataSize = GetTestDataStringSize(eTestDataString_PathFull);
  const ni::achar* const data = GetTestDataString(eTestDataString_PathFull);
  const tInt encodedLen = HexEncodeOutputSize(dataSize);
  TEST_DEBUGFMT("... dataSize: %s -> encodedLen: %s", dataSize, encodedLen);
  char* encoded = (char*)niMalloc(encodedLen); *encoded = 0;
  ASTL_SCOPE_EXIT {
    niFree(encoded);
  };

  tInt encodeResSize = HexEncode((tPtr)encoded, encodedLen, (tPtr)data, dataSize, eFalse);
  CHECK_EQUAL(encodedLen, encodeResSize+1);
  TEST_DEBUGFMT("data: %s -> encoded: %s", data, encoded);
  CHECK_EQUAL(
    _ASTR("433a2f446f63756d656e747320616e642053657474696e67732f4f776e65722f4d7920446f63756d656e74732f61626364646566676869c3a0c3a9c3a8c3a7c3b9e282acc2b5c2b0e4bda0e5a5bdceb1ceb22f476179612f74657374e4bda0e5a5bd2e74787400"),
    //_ASTR("666f6f00"),
    _ASTR(encoded));

  const tInt decodedLen = HexDecodeOutputSize(encoded, StrLen(encoded));
  TEST_DEBUGFMT("... encodedLen: %s -> decodedLen: %s", encodedLen, decodedLen);
  char* decoded = (char*)niMalloc(decodedLen); *decoded = 0;
  ASTL_SCOPE_EXIT {
    niFree(decoded);
  };

  tInt decodeResSize = HexDecode((tPtr)decoded, decodedLen, encoded, StrLen(encoded));
  CHECK_EQUAL(decodedLen, decodeResSize);
  CHECK_EQUAL(ni::StrSizeZ(data), decodeResSize);
  TEST_DEBUGFMT("decoded: %s", decoded);
  CHECK_EQUAL(
    _ASTR("C:/Documents and Settings/Owner/My Documents/abcddefghiàéèçù€µ°你好αβ/Gaya/test你好.txt"),
    //_ASTR("foo"),
    _ASTR(decoded));

}

TEST_FIXTURE(FBufferEncDec,FileReadWriteRawToString) {
  const char* encodedHex = "433a2f446f63756d656e747320616e642053657474696e67732f4f776e65722f4d7920446f63756d656e74732f61626364646566676869c3a0c3a9c3a8c3a7c3b9e282acc2b5c2b0e4bda0e5a5bdceb1ceb22f476179612f74657374e4bda0e5a5bd2e74787400";
  const char* encodedBase64 = "QzovRG9jdW1lbnRzIGFuZCBTZXR0aW5ncy9Pd25lci9NeSBEb2N1bWVudHMvYWJjZGRlZmdoacOgw6nDqMOnw7nigqzCtcKw5L2g5aW9zrHOsi9HYXlhL3Rlc3TkvaDlpb0udHh0AA==";
  const char* encodedBase32 = "8cx2yh3fcdtptsbeehsj0rbecgg56sbmehmpwsvk5x7qevk5e8qmty908hqp6xbdcnq78wsfc5h66s34cnk6et39regc7ae3n31tfgxswa1asgnnrare9fd0wpjvvknhsts2yhv1f5gjyx35edte9fd0wpjvtbkmf1t00";
  const ni::tU32 dataSize = GetTestDataStringSize(eTestDataString_PathFull);
  const ni::achar* const data = GetTestDataString(eTestDataString_PathFull);

  {
    Ptr<iFile> fp = ni::GetLang()->CreateFileDynamicMemory();
    fp->WriteRawFromString(eRawToStringEncoding_Base64, encodedBase64);
    CHECK_EQUAL(103, fp->Tell());
    fp->SeekSet(0);
    CHECK_EQUAL(
      fp->ReadString(),
      _ASTR("C:/Documents and Settings/Owner/My Documents/abcddefghiàéèçù€µ°你好αβ/Gaya/test你好.txt"));
  }

  {
    Ptr<iFile> fp = ni::GetLang()->CreateFileDynamicMemory();
    fp->WriteRawFromString(eRawToStringEncoding_Base32, encodedBase32);
    CHECK_EQUAL(103, fp->Tell());
    fp->SeekSet(0);
    CHECK_EQUAL(
      fp->ReadString(),
      _ASTR("C:/Documents and Settings/Owner/My Documents/abcddefghiàéèçù€µ°你好αβ/Gaya/test你好.txt"));
  }

  {
    Ptr<iFile> fp = ni::GetLang()->CreateFileDynamicMemory();
    fp->WriteRawFromString(eRawToStringEncoding_Hex, encodedHex);
    CHECK_EQUAL(103, fp->Tell());
    fp->SeekSet(0);
    CHECK_EQUAL(
      fp->ReadString(),
      _ASTR("C:/Documents and Settings/Owner/My Documents/abcddefghiàéèçù€µ°你好αβ/Gaya/test你好.txt"));
  }

  {
    Ptr<iFile> fp = ni::GetLang()->CreateFileDynamicMemory();
    fp->WriteRaw(data,dataSize);
    fp->SeekSet(0);
    cString encoded = fp->ReadRawToString(eRawToStringEncoding_Base32, dataSize);
    CHECK_EQUAL(dataSize, fp->Tell());
    CHECK_EQUAL(_ASTR(encodedBase32), encoded);
  }

  {
    Ptr<iFile> fp = ni::GetLang()->CreateFileDynamicMemory();
    fp->WriteRaw(data,dataSize);
    fp->SeekSet(0);
    cString encoded = fp->ReadRawToString(eRawToStringEncoding_Base64, dataSize);
    CHECK_EQUAL(dataSize, fp->Tell());
    CHECK_EQUAL(_ASTR(encodedBase64), encoded);
  }

  {
    Ptr<iFile> fp = ni::GetLang()->CreateFileDynamicMemory();
    fp->WriteRaw(data,dataSize);
    fp->SeekSet(0);
    cString encoded = fp->ReadRawToString(eRawToStringEncoding_Hex, dataSize);
    CHECK_EQUAL(dataSize, fp->Tell());
    CHECK_EQUAL(_ASTR(encodedHex), encoded);
  }
}

void EncodeBlockInvert(tU8* apData, const tInt anSize) {
  niLoop(i, anSize) {
    apData[i] = ~apData[i];
  }
};

TEST_FIXTURE(FBufferEncDec,EncInvert) {
  tInt outputCount = 0;
  const cString pathOut = UnitTest::GetTestOutputFilePath(niFmt("%s_%d.bmp", m_testName, ++outputCount));
  Ptr<iFile> fpIn = niFileOpenBin2H(montmay12_bmp);
  Ptr<iFile> fpOut = GetRootFS()->FileOpen(pathOut.Chars(),eFileOpenMode_Write);
  fpOut->WriteFile(fpIn->GetFileBase(), 54); // copy the header verbatim
  const tInt blockSize = 16;
  tU8* blockData = niTMalloc(tU8,blockSize);
  niDefer { niFree(blockData); };

  for (;;) {
    const tInt readBytes = fpIn->ReadRaw(blockData, blockSize);
    if (readBytes > 0) {
      EncodeBlockInvert(blockData, blockSize);
      fpOut->WriteRaw(blockData,readBytes);
    }
    if (fpIn->GetPartialRead()) {
      break;
    }
  }
}
