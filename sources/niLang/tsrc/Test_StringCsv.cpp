#include "stdafx.h"
#include "../src/API/niLang/StringDef.h"
#include "../src/API/niLang/Utils/UTFImpl.h"

using namespace ni;

struct FStringCsv {
};

///////////////////////////////////////////////
TEST_FIXTURE(FStringCsv,SplitSep) {
  const char* _csv[] = {
    /* 0 */ "level,name,cmd,comments,type,address,abb_status,comments_multiple",
    /* 1 */ ",,,,,,,",
    /* 2 */ "BL,,,,,,,",
    /* 3 */ ",BMS,GROUP,,,,,",
    /* 4 */ ",\"Chiller, \"\", Management,\\\" L5\",,,Alarm,TBD,,",
    /* 4 */ ",\"Chiller, \"\", Management,\\\" L5\",foo,,Alarm,TBD,,",
    /* 5 */ ",PAHU-L1,, \"Precooled AHU\" ,Alarm,TBD,,",
  };

  {
    astl::vector<ni::cString> toks;
    StringSplitSep(_ASTR(_csv[0]), ",", &toks);
    CHECK_EQUAL(8, toks.size());
    CHECK_EQUAL(_ASTR("name"), toks[1]);
    CHECK_EQUAL(_ASTR("comments_multiple"), toks[7]);
  }

  {
    astl::vector<ni::cString> toks;
    StringSplitSep(_ASTR(_csv[3]), ",", &toks);
    CHECK_EQUAL(8, toks.size());
    CHECK_EQUAL(_ASTR("BMS"), toks[1]);
    CHECK_EQUAL(_ASTR("GROUP"), toks[2]);
    CHECK_EQUAL(_ASTR(""), toks[5]);
    CHECK_EQUAL(_ASTR(""), toks[7]);
  }

  {
    astl::vector<ni::cString> toks;
    StringSplitSep(_ASTR(_csv[4]), ",", &toks);
    CHECK_EQUAL(11, toks.size());
    CHECK_EQUAL(_ASTR("\"Chiller"), toks[1]);
  }

  {
    astl::vector<ni::cString> toks;
    StringSplitSepQuoted(_ASTR(_csv[4]), ",", '"', &toks);
    CHECK_EQUAL(8, toks.size());
    CHECK_EQUAL(_ASTR("\"Chiller, \"\", Management,\\\" L5\""), toks[1]);
    CHECK_EQUAL(_ASTR(""), toks[2]);
    CHECK_EQUAL(_ASTR("Alarm"), toks[4]);
    CHECK_EQUAL(_ASTR("TBD"), toks[5]);
  }

  {
    astl::vector<ni::cString> toks;
    StringSplitSepQuoted(_ASTR(_csv[5]), ",", '"', &toks);
    CHECK_EQUAL(8, toks.size());
    CHECK_EQUAL(_ASTR("\"Chiller, \"\", Management,\\\" L5\""), toks[1]);
    CHECK_EQUAL(_ASTR("foo"), toks[2]);
    CHECK_EQUAL(_ASTR("Alarm"), toks[4]);
    CHECK_EQUAL(_ASTR("TBD"), toks[5]);
  }
}

TEST_FIXTURE(FStringCsv,ReadCSVLine) {
  const char csvFile[] = {
    ",RDU-001-A,\"Replace:001:001,002,003,004,005,006,007,008,009,010,011,012,013,014,015,016,017,018,019\",\"Need to show consumption of the RDUs kW\n"
    "Need to show status of all the RDUs - Eg. On/ off \",OnOff,[Control Structure]Root/Control Network_PMS/DF21_Optimus_PMS/Applications/PMS2_Phase1/Control Modules/Level_2/SG-DF-01-02-RDU-001-A:OnOff3.Value,\"1 = on , 0 = off\",\"RDU-001-A to RDU-019A &\n"
    "RDU-001-B to RDU-019-B\"\n"
    ",RDU-001-A,\"Replace:001:001,002,003,004,005,006,007,008,009,010,011,012,013,014,015,016,017,018,019\",,kW,[Control Structure]Root/KEP_OPC_Network_EMS2/DF02RDU001A_Channel/SG-DF-01-02-RDU-001-A-PM:P_KW,,\"RDU-001-A to RDU-019A &\n"
    "RDU-001-B to RDU-019-B\"\n"
    ",RDU-001-B,\"Replace:001:001,002,003,004,005,006,007,008,009,010,011,012,013,014,015,016,017,018,019\",\"Need to show consumption of the RDUs kW\n"
    "Need to show status of all the RDUs - Eg. On/ off \",OnOff,[Control Structure]Root/Control Network_PMS/DF21_Optimus_PMS/Applications/PMS2_Phase1/Control Modules/Level_2/SG-DF-01-02-RDU-001-B:OnOff3.Value,\"1 = on , 0 = off\",\"RDU-001-A to RDU-019A &\n"
    "RDU-001-B to RDU-019-B\"\n"
  };

  ni::Ptr<ni::iFile> fp = ni::CreateFileMemory((ni::tPtr)csvFile, sizeof(csvFile), ni::eFalse, "");
  while (!fp->GetPartialRead()) {
    ni::cString line = fp->ReadQuotedLine();
    if (line.empty())
      continue;
    niDebugFmt(("LINE: %s", line));
    astl::vector<ni::cString> toks;
    StringSplitSepQuoted(line, ",", '"', &toks);
    CHECK_EQUAL(8, toks.size());
    CHECK(toks[1].StartsWith("RDU-001"));
    CHECK(toks[7].EndsWith("RDU-019-B\""));
    niLoop(i,toks.size()) {
      toks[i].TrimEx("\" ");
    }
    CHECK(toks[1].StartsWith("RDU-001"));
    CHECK(toks[7].EndsWith("RDU-019-B"));
  }
}

TEST_FIXTURE(FStringCsv,Write) {
  const ni::tU32 numValues = 5;
  const ni::cString expectedCsv = "hello world,123,\"foo \"\"bar\"\"\",,  spaced  ";
  const ni::achar* values[numValues] = {
    "hello world", "123", "foo \"bar\"", "", "  spaced  "
  };

  {
    ni::cString o;
    ni::StringAppendCsvValues<const ni::achar*>(o, ',', '"', values, numValues);
    CHECK_EQUAL(expectedCsv, o);
  }

  {
    astl::vector<ni::cString> toks;
    StringSplitSepQuoted(expectedCsv, ",", '"', &toks);
    CHECK_EQUAL(numValues, toks.size());
    niLoop(i,numValues) {
      cString col;
      StringDecodeCsvUnquote(col, '"', toks[i]);
      CHECK_EQUAL(_ASTR(values[i]), col);
    }
  }

  {
    astl::vector<ni::cString> cols;
    StringSplitCsvFields(expectedCsv, ",", '"', &cols);
    CHECK_EQUAL(numValues, cols.size());
    niLoop(i,numValues) {
      CHECK_EQUAL(_ASTR(values[i]), cols[i]);
    }
  }
}
