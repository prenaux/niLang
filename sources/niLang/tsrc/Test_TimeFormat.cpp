#include "stdafx.h"
#include "../src/API/niLang/Utils/CollectionImpl.h"

struct FTimeFormat {
};

using namespace ni;

TEST_FIXTURE(FTimeFormat,Time) {
  // 1604673928 seconds since Jan 01 1970. (UTC)
  // 11/06/2020 @ 2:45pm (UTC)
  // 2020-11-06T14:45:28+00:00 in ISO 8601
  // Fri, 06 Nov 2020 14:45:28 +0000 in RFC 822, 1036, 1123, 2822
  // Friday, 06-Nov-20 14:45:28 UTC in RFC 2822
  // 2020-11-06T14:45:28+00:00 in RFC 3339
  Ptr<iTime> dt = ni::CreateTimeZero();
  dt->SetUnixTimeSecs(1604673928);
  niDebugFmt(("DT: %s", dt->Format(NULL)));
  CHECK_EQUAL(_ASTR("2020-11-06T14:45:28Z"), dt->Format(NULL));
  CHECK_EQUAL(1604673928, dt->GetUnixTimeSecs());
  CHECK_EQUAL(2020, dt->GetYear());
  CHECK_EQUAL(11, dt->GetMonth());
  CHECK_EQUAL(6, dt->GetDay());
  CHECK_EQUAL(5, dt->GetWeekday());
  CHECK_EQUAL(14, dt->GetHour());
  CHECK_EQUAL(45, dt->GetMinute());
  CHECK_EQUAL(28, dt->GetSecond());
}

TEST_FIXTURE(FTimeFormat,FormatSingle) {
  const iTime* t = ni::GetLang()->GetCurrentTime();
  niDebugFmt(("# YYYY    year          2011"));
  niDebugFmt(("%s", t->Format("YYYY")));
  niDebugFmt(("# YY      year          99"));
  niDebugFmt(("%s", t->Format("YY")));
  niDebugFmt(("# M       month         1 2 ... 30 31"));
  niDebugFmt(("%s", t->Format("M")));
  niDebugFmt(("# MM      month         01 02 ... 30 31"));
  niDebugFmt(("%s", t->Format("MM")));
  niDebugFmt(("# MMM     month         Mar"));
  niDebugFmt(("%s", t->Format("MMM")));
  niDebugFmt(("# MMMM    month         March"));
  niDebugFmt(("%s", t->Format("MMMM")));
  niDebugFmt(("# D       day of month  1 2 ... 30 31"));
  niDebugFmt(("%s", t->Format("D")));
  niDebugFmt(("# Do      day of month  1st 2nd ... 30th 31st"));
  niDebugFmt(("%s", t->Format("Do")));
  niDebugFmt(("# DD      day of month  01 02 ... 30 31"));
  niDebugFmt(("%s", t->Format("DD")));
  niDebugFmt(("# d       day of week   0 1 ... 5 6"));
  niDebugFmt(("%s", t->Format("d")));
  niDebugFmt(("# do      day of week   0th 1st ... 5th 6th"));
  niDebugFmt(("%s", t->Format("do")));
  niDebugFmt(("# dd      day of week   Su"));
  niDebugFmt(("%s", t->Format("dd")));
  niDebugFmt(("# ddd     day of week   Sun"));
  niDebugFmt(("%s", t->Format("ddd")));
  niDebugFmt(("# dddd    day of week   Sunday"));
  niDebugFmt(("%s", t->Format("dddd")));
  niDebugFmt(("# e       day of week   0 1 ... 5 6"));
  niDebugFmt(("%s", t->Format("e")));
  niDebugFmt(("# E       day of week   1 2 ... 6 7"));
  niDebugFmt(("%s", t->Format("E")));
  niDebugFmt(("# a       am/pm marker  am/pm"));
  niDebugFmt(("%s", t->Format("a")));
  niDebugFmt(("# A       AM/PM marker  AM/PM"));
  niDebugFmt(("%s", t->Format("A")));
  niDebugFmt(("# H       hour(0-23)    16"));
  niDebugFmt(("%s", t->Format("H")));
  niDebugFmt(("# HH      hour(0-23)    06"));
  niDebugFmt(("%s", t->Format("HH")));
  niDebugFmt(("# h       hour(1-12)    4"));
  niDebugFmt(("%s", t->Format("h")));
  niDebugFmt(("# hh      hour(1-12)    04"));
  niDebugFmt(("%s", t->Format("hh")));
  niDebugFmt(("# m       minute        6"));
  niDebugFmt(("%s", t->Format("m")));
  niDebugFmt(("# mm      minute        06"));
  niDebugFmt(("%s", t->Format("mm")));
  niDebugFmt(("# s       second        6"));
  niDebugFmt(("%s", t->Format("s")));
  niDebugFmt(("# ss      second        06"));
  niDebugFmt(("%s", t->Format("ss")));
}

TEST_FIXTURE(FTimeFormat,FormatSquareBrackets) {
  const iTime* t = ni::GetLang()->GetCurrentTime();
  niDebugFmt(("Day at Hour AM/PM: %s", t->Format("dddd [at] hA")));
  niDebugFmt(("Day 你好 Hour AM/PM: %s", t->Format("dddd [你好] hA")));
  niDebugFmt(("Day [nested [[brackets] should] work] Hour AM/PM: %s", t->Format("dddd [nested [[brackets] should] work] hA")));
  niDebugFmt(("[At the start] Day Hour AM/PM: %s", t->Format("[At the start] dddd hA")));
  niDebugFmt(("Day Hour AM/PM [At the end]: %s", t->Format("dddd hA [At the end]")));
  niDebugFmt(("Day Hour AM/PM [At the end not closed: %s", t->Format("dddd hA [At the end not closed")));
}

TEST_FIXTURE(FTimeFormat,FormatStd) {
  const iTime* t = ni::GetLang()->GetCurrentTime();
  niDebugFmt(("RFC1123 (Wed, 16 Feb 2011 03:34:56 Z): %s", t->Format("RFC1123")));
  niDebugFmt(("RFC850 (Wednesday, 16-Feb-11 03:34:56 Z): %s", t->Format("RFC850")));
  niDebugFmt(("ASCTIME (Wed Feb 16 03:34:56 2011): %s", t->Format("ASCTIME")));
  niDebugFmt(("ISO8601 (2014-09-08T08:02:17-05:00): %s", t->Format("ISO8601")));
}

TEST_FIXTURE(FTimeFormat,FormatSingleTimezone) {
  const iTime* t = ni::GetLang()->GetCurrentTime();
  niDebugFmt(("Z: %s", t->Format("Z")));
  niDebugFmt(("ZZ: %s", t->Format("ZZ")));
}

TEST_FIXTURE(FTimeFormat,ParseSingleTimeZone) {
  Ptr<iTime> t = ni::GetLang()->GetCurrentTime()->Clone();
  {
    const tI32 tz = t->ParseString("+0800","%z")->GetTimeZone();
    niDebugFmt(("tz (+0800): %s", tz));
    CHECK_EQUAL(-28800, tz);
  }
  {
    const tI32 tz = t->ParseString("+08:00","%z")->GetTimeZone();
    niDebugFmt(("tz (+08:00): %s", tz));
    CHECK_EQUAL(-28800, tz);
  }
  {
    const tI32 tz = t->ParseString("+08","%z")->GetTimeZone();
    niDebugFmt(("tz (+08): %s", tz));
    CHECK_EQUAL(-28800, tz);
  }
  {
    const tI32 tz = t->ParseString("-0800","%z")->GetTimeZone();
    niDebugFmt(("tz (-0800): %s", tz));
    CHECK_EQUAL(28800, tz);
  }
  {
    const tI32 tz = t->ParseString("-08:00","%z")->GetTimeZone();
    niDebugFmt(("tz (-08:00): %s", tz));
    CHECK_EQUAL(28800, tz);
  }
  {
    const tI32 tz = t->ParseString("-08","%z")->GetTimeZone();
    niDebugFmt(("tz (-08): %s", tz));
    CHECK_EQUAL(28800, tz);
  }
}

TEST_FIXTURE(FTimeFormat,ParseISO8601) {
  Ptr<iTime> t = ni::GetLang()->GetCurrentTime()->Clone();
  {
    cString strISO = t->ParseString("2016-07-24T14:38:47+08:00",NULL)->Format(NULL);
    niDebugFmt(("ISO8601: %s", strISO));
    CHECK_EQUAL(_ASTR("2016-07-24T14:38:47+08:00"), strISO);
  }
  {
    cString strISO = t->ParseString("2016-07-24T14:38:47+0800",NULL)->Format(NULL);
    niDebugFmt(("ISO8601: %s", strISO));
    CHECK_EQUAL(_ASTR("2016-07-24T14:38:47+08:00"), strISO);
  }
  {
    cString strISO = t->ParseString("2016-07-24T06:38:47Z",NULL)->Format(NULL);
    niDebugFmt(("ISO8601: %s", strISO));
    CHECK_EQUAL(_ASTR("2016-07-24T06:38:47Z"), strISO);
    CHECK_EQUAL(2016, t->GetYear());
    CHECK_EQUAL( 7, t->GetMonth());
    CHECK_EQUAL(24, t->GetDay());
    CHECK_EQUAL( 6, t->GetHour());
    CHECK_EQUAL(38, t->GetMinute());
    CHECK_EQUAL(47, t->GetSecond());
    CHECK_EQUAL(0, t->GetTimeZone());
    CHECK_EQUAL(1469342327, t->GetUnixTimeSecs());
  }
  {
    cString strISO = t->ParseString("2016-07-24T14:38:47+08",NULL)->Format(NULL);
    niDebugFmt(("ISO8601: %s", strISO));
    CHECK_EQUAL(_ASTR("2016-07-24T14:38:47+08:00"), strISO);
    CHECK_EQUAL(2016, t->GetYear());
    CHECK_EQUAL( 7, t->GetMonth());
    CHECK_EQUAL(24, t->GetDay());
    CHECK_EQUAL(14, t->GetHour());
    CHECK_EQUAL(38, t->GetMinute());
    CHECK_EQUAL(47, t->GetSecond());
    CHECK_EQUAL(-28800, t->GetTimeZone());
    CHECK_EQUAL(1469342327, t->GetUnixTimeSecs());
  }
}
