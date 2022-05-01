#include "stdafx.h"
#include "../src/API/niLang/IFile.h"
#include "../src/API/niLang/ILang.h"

struct FStringURI {
};

using namespace ni;

TEST_FIXTURE(FStringURI,FilePath1) {
  const cString pathExpectedEncoding = "E%3a%5cDocuments%20and%20Settings";
  const cString path = "E:\\Documents and Settings";
  cString pathEncoded;
  cString pathDecoded;
  TEST_PRINT(niFmt("pathEncoded: %s", StringEncodeUrl(pathEncoded, path)));
  CHECK_EQUAL(pathExpectedEncoding, pathEncoded);
  TEST_PRINT(niFmt("pathDecoded: %s", StringDecodeUrl(pathDecoded, pathEncoded)));
  CHECK_EQUAL(path, pathDecoded);
}

TEST_FIXTURE(FStringURI,FilePath2) {
  const cString pathExpectedEncoding = "E%3a%5cDocuments%20and%20Settings%2fstuff%20with%20space%20and%20%28%29%20other%20crap%20%e4%bd%a0%e5%a5%bd%e5%90%97%3f%2f";
  const cString path = "E:\\Documents and Settings/stuff with space and () other crap 你好吗?/";
  cString pathEncoded;
  cString pathDecoded;
  TEST_PRINT(niFmt("pathEncoded: %s", StringEncodeUrl(pathEncoded, path)));
  CHECK_EQUAL(pathExpectedEncoding, pathEncoded);
  TEST_PRINT(niFmt("pathDecoded: %s", StringDecodeUrl(pathDecoded, pathEncoded)));
  CHECK_EQUAL(path, pathDecoded);
}
