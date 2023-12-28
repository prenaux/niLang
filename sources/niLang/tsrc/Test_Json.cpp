#include "stdafx.h"
#include "../src/API/niLang/STL/scope_guard.h"
#include "../src/API/niLang/Utils/DataTableUtils.h"
#include "./data/json_crash.html.hxx"

using namespace ni;

static const cString _dtJson = R"""(
  {"string":"bar","number":123,"isnull":null,"bool":true}
)""";

static const cString _dtJsonArray = R"""(
  [{"string":"bar","number":123,"isnull":null,"bool":true},{"string":"bar","number":321,"isnull":null,"bool":false}]
)""";

static const cString _dtJsonArray1 = R"(["bar",123,null,true,false,{"S":1,"arr":[11,222],"b":{"a":1}}])";

struct FJson {
};

TEST_FIXTURE(FJson,Basic) {
  Ptr<iDataTable> dt = ni::CreateDataTable("dt");
  CHECK_EQUAL(eTrue, JsonParseStringToDataTable(_dtJson.Chars(), dt));
  CHECK_EQUAL(_ASTR("bar"), dt->GetString("string"));
  CHECK_EQUAL(123, dt->GetInt("number"));
  CHECK_EQUAL(0, dt->GetInt("isnull"));

  Ptr<iDataTable> dt2 = ni::CreateDataTable();
  ni::Ptr<iFile> fp2 = ni::CreateFileMemory((tPtr)_dtJsonArray1.Chars(),_dtJsonArray1.size(),eFalse,NULL);
  ni::GetLang()->SerializeDataTable("json", ni::eSerializeMode_Read, dt2, fp2);

  Ptr<iFile> file = ni::CreateFileDynamicMemory(0, NULL);
  ni::GetLang()->SerializeDataTable("json", ni::eSerializeMode_Write, dt2, file);
  CHECK_EQUAL(file->ReadString(), _dtJsonArray1);
}

TEST_FIXTURE(FJson,JsonCrash) {
  Ptr<iDataTable> dt = ni::CreateDataTable("dt");
  Ptr<iFile> fp = niFileOpenBin2H(json_crash_html);
  CHECK_EQUAL(eFalse, JsonParseFileToDataTable(fp, dt));
}


TEST_FIXTURE(FJson,JSONWriter) {
  Ptr<iDataTable> dt = ni::CreateDataTable("");
  ni::Ptr<iFile> fp = ni::CreateFileMemory((tPtr)_dtJson.Chars(),_dtJson.size(),eFalse,NULL);
  ni::GetLang()->SerializeDataTable("json", ni::eSerializeMode_Read, dt,
                                        fp);
  Ptr<iFile> file = ni::CreateFileDynamicMemory(0, NULL);
  ni::SerializeDataTable("json", eSerializeMode_Write, dt, file);
  CHECK_EQUAL(file->ReadString(), R"""({"string":"bar","number":123,"isnull":null,"bool":1})""");

  Ptr<iDataTable> dt2 = ni::CreateDataTable("");
  ni::Ptr<iFile> fp2 = ni::CreateFileMemory((tPtr)_dtJsonArray.Chars(),_dtJsonArray.size(),eFalse,NULL); ni::GetLang()->SerializeDataTable("json", ni::eSerializeMode_Read, dt2,
                                        fp2);
  Ptr<iFile> file2 = ni::CreateFileDynamicMemory(0, NULL);
  ni::SerializeDataTable("json", eSerializeMode_Write, dt2, file2);
  CHECK_EQUAL(file2->ReadString(), R"""([{"string":"bar","number":123,"isnull":null,"bool":1},{"string":"bar","number":321,"isnull":null,"bool":0}])""");


}
