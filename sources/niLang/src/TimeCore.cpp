// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "API/niLang/Types.h"
#include "API/niLang/StringDef.h"
#include "API/niLang/StringBase.h"
#include "API/niLang/ITime.h"
#include "API/niLang/IRegex.h"
#include "API/niLang/IToString.h"
#include "API/niLang/Utils/UnknownImpl.h"
#include "Lang.h"
#include <time.h>

using namespace ni;

struct sUTCTM {
  sUTCTM() { tm_sec = tm_min = tm_hour = tm_mday = tm_mon = tm_year = 0; }
  int tm_sec;         /* seconds */
  int tm_min;         /* minutes */
  int tm_hour;        /* hours */
  int tm_mday;        /* day of the month */
  int tm_mon;         /* month */
  int tm_year;        /* year */
};

struct sFullTM : public sUTCTM {
  sFullTM() { tm_wday = tm_yday = tm_gmtoff = 0; }
  int tm_wday;
  int tm_yday;
  int tm_gmtoff;
};

typedef tI64 Time64_T;
typedef tI64 Year;
#define TM sUTCTM

#define WRAP(a,b,m)     ((a) = ((a) <  0  ) ? ((b)--, (a) + (m)) : (a))
/* Let's assume people are going to be looking for dates in the future.
   Let's provide some cheats so you can skip ahead.
   This has a 4x speed boost when near 2008.
*/
/* Number of days since epoch on Jan 1st, 2008 GMT */
#define CHEAT_DAYS  (1199145600 / 24 / 60 / 60)
#define CHEAT_YEARS 108
#define IS_LEAP(n)  ((!(((n) + 1900) % 400) || (!(((n) + 1900) % 4) && (((n) + 1900) % 100))) != 0)

static const Year years_in_gregorian_cycle   = 400;
#define days_in_gregorian_cycle      ((365 * 400) + 100 - 4 + 1)

static const int length_of_year[2] = { 365, 366 };

static const int days_in_month[2][12] = {
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
};

static const int julian_days_by_month[2][12] = {
  {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
  {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335},
};

static int check_tm(struct TM *tm)
{
  /* Don't forget leap seconds */
  niAssert(tm->tm_sec >= 0);
  niAssert(tm->tm_sec <= 61);

  niAssert(tm->tm_min >= 0);
  niAssert(tm->tm_min <= 59);

  niAssert(tm->tm_hour >= 0);
  niAssert(tm->tm_hour <= 23);

  niAssert(tm->tm_mday >= 1);
  niAssert(tm->tm_mday <= days_in_month[IS_LEAP(tm->tm_year)][tm->tm_mon-1]);

  niAssert(tm->tm_mon  >= 1);
  niAssert(tm->tm_mon  <= 12);

  return 1;
}

static TM* SecondsToUTCTM(const Time64_T in_time, struct TM *p)
{
  int v_tm_sec, v_tm_min, v_tm_hour, v_tm_mon;
  Time64_T v_tm_tday;
  int leap;
  Time64_T m;
  Time64_T time = in_time;
  Year year = 70;
  int cycles = 0;

  niAssert(p != NULL);

  v_tm_sec =  (int)(time % 60);
  time /= 60;
  v_tm_min =  (int)(time % 60);
  time /= 60;
  v_tm_hour = (int)(time % 24);
  time /= 24;
  v_tm_tday = time;

  WRAP (v_tm_sec, v_tm_min, 60);
  WRAP (v_tm_min, v_tm_hour, 60);
  WRAP (v_tm_hour, v_tm_tday, 24);
  m = v_tm_tday;

  if (m >= CHEAT_DAYS) {
    year = CHEAT_YEARS;
    m -= CHEAT_DAYS;
  }

  if (m >= 0) {
    /* Gregorian cycles, this is huge optimization for distant times */
    cycles = (int)(m / (Time64_T) days_in_gregorian_cycle);
    if( cycles ) {
      m -= (cycles * (Time64_T) days_in_gregorian_cycle);
      year += (cycles * years_in_gregorian_cycle);
    }

    /* Years */
    leap = IS_LEAP (year);
    while (m >= (Time64_T) length_of_year[leap]) {
      m -= (Time64_T) length_of_year[leap];
      year++;
      leap = IS_LEAP (year);
    }

    /* Months */
    v_tm_mon = 0;
    while (m >= (Time64_T) days_in_month[leap][v_tm_mon]) {
      m -= (Time64_T) days_in_month[leap][v_tm_mon];
      v_tm_mon++;
    }
  } else {
    year--;

    /* Gregorian cycles */
    cycles = (int)((m / (Time64_T) days_in_gregorian_cycle) + 1);
    if( cycles ) {
      m -= (cycles * (Time64_T) days_in_gregorian_cycle);
      year += (cycles * years_in_gregorian_cycle);
    }

    /* Years */
    leap = IS_LEAP (year);
    while (m < (Time64_T) -length_of_year[leap]) {
      m += (Time64_T) length_of_year[leap];
      year--;
      leap = IS_LEAP (year);
    }

    /* Months */
    v_tm_mon = 11;
    while (m < (Time64_T) -days_in_month[leap][v_tm_mon]) {
      m += (Time64_T) days_in_month[leap][v_tm_mon];
      v_tm_mon--;
    }
    m += (Time64_T) days_in_month[leap][v_tm_mon];
  }

  p->tm_year = year;
  if( p->tm_year != year ) {
    return NULL;
  }

  /* At this point m is less than a year so casting to an int is safe */
  p->tm_mday = (int)m + 1;
  p->tm_sec  = v_tm_sec;
  p->tm_min  = v_tm_min;
  p->tm_hour = v_tm_hour;
  p->tm_mon  = v_tm_mon+1; // v_tm_mon is computed in base 0, we store in base 1

  niAssert((julian_days_by_month[leap][p->tm_mon-1] + (int)m) <= length_of_year[IS_LEAP(p->tm_year)]);
  niAssert(check_tm(p));niUnused(check_tm);

  return p;
}

/* timegm() is not in the C or POSIX spec, but it is such a useful
   extension I would be remiss in leaving it out.  Also I need it
   for localtime64()
*/
static Time64_T UTCTMToSeconds(const struct TM *date) {
  Time64_T days    = 0;
  Time64_T seconds = 0;
  Year     year;
  Year     orig_year = (Year)date->tm_year;
  int      cycles  = 0;

  if( orig_year > 100 ) {
    cycles = (orig_year - 100) / 400;
    orig_year -= cycles * 400;
    days      += (Time64_T)cycles * days_in_gregorian_cycle;
  }
  else if( orig_year < -300 ) {
    cycles = (orig_year - 100) / 400;
    orig_year -= cycles * 400;
    days      += (Time64_T)cycles * days_in_gregorian_cycle;
  }

  if( orig_year > 70 ) {
    year = 70;
    while( year < orig_year ) {
      days += length_of_year[IS_LEAP(year)];
      year++;
    }
  }
  else if ( orig_year < 70 ) {
    year = 69;
    do {
      days -= length_of_year[IS_LEAP(year)];
      year--;
    } while( year >= orig_year );
  }

  days += julian_days_by_month[IS_LEAP(orig_year)][date->tm_mon-1];
  days += date->tm_mday - 1;

  seconds = days * 60 * 60 * 24;

  seconds += date->tm_hour * 60 * 60;
  seconds += date->tm_min * 60;
  seconds += date->tm_sec;

  return(seconds);
}

void UTCTMAddSeconds(sUTCTM& aUTC, tI64 aSeconds) {
  tI64 secs = UTCTMToSeconds(&aUTC);
  secs += aSeconds;
  SecondsToUTCTM(secs,&aUTC);
}

static const ni::achar* _kaszMonth[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
static const ni::achar* _kaszMonthFull[] = {"January","February","March","April","May","June","July","August","September","October","November","December"};
static const ni::achar* _kaszWeekday2[] = {"Su","Mo","Tu","We","Th","Fr","Sa"};
static const ni::achar* _kaszWeekday3[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
static const ni::achar* _kaszWeekdayFull[] = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};

static const char* _getNumeralTh(const tU32 aNumberCharCode) {
  switch (aNumberCharCode) {
    case '1': return "st";
    case '2': return "nd";
    case '3': return "rd";
    default:  return "th";
  }
}

static void __stdcall _formatDateTime(const iTime* apTime,cString& aFormat,cString& aRet) {

  if (aFormat.StartsWith("[")) {
    const char* p = aFormat.begin()+1;
    int c = 1;
    while (*p) {
      if (*p == '[') {
        ++c;
      }
      else if (*p == ']') {
        --c;
        if (c == 0) break;
      }
      ++p;
    }
    tSize len = (p-aFormat.begin())+1;
    aRet.appendEx(aFormat.begin()+1, len-2);
    aFormat.erase(0,len);
  }

  else if (aFormat.StartsWith("ISO8601")) {
    aRet << apTime->Format("YYYY-MM-DD[T]HH:mm:ssZ");
    aFormat.erase(0,7);
  }

  else if (aFormat.StartsWith("RFC1123")) {
    aRet << apTime->Format("ddd, DD MMM YYYY HH:mm:ss Z");
    aFormat.erase(0,7);
  }

  else if (aFormat.StartsWith("RFC850")) {
    aRet << apTime->Format("dddd, DD-MMM-YY HH:mm:ss Z");
    aFormat.erase(0,6);
  }

  else if (aFormat.StartsWith("ASCTIME")) {
    aRet << apTime->Format("ddd MMM DD HH:mm:ss YYYY");
    aFormat.erase(0,7);
  }

  else if (aFormat.StartsWith("YYYY")) {
    aRet << apTime->GetYear();
    aFormat.erase(0,4);
  }

  else if (aFormat.StartsWith("YY")) {
    aRet << apTime->GetYear() % 100;
    aFormat.erase(0,2);
  }

  else if (aFormat.StartsWith("MMMM")) {
    aFormat.erase(0,4);
    tU32 m = apTime->GetMonth();
    niCheck(m > 0 && m <= niCountOf(_kaszMonthFull),;);
    aRet << _kaszMonthFull[m-1];
  }

  else if (aFormat.StartsWith("MMM")) {
    aFormat.erase(0,3);
    tU32 m = apTime->GetMonth();
    niCheck(m > 0 && m <= niCountOf(_kaszMonth),;);
    aRet << _kaszMonth[m-1];
  }

  else if (aFormat.StartsWith("MM")) {
    aFormat.erase(0,2);
    tU8 month = apTime->GetMonth();
    if (month < 10) {
      aRet << (tU32)0L;
    }
    aRet << month;
  }

  else if (aFormat.StartsWith("M")) {
    aFormat.erase(0,1);
    aRet << apTime->GetMonth();
  }

  else if (aFormat.StartsWith("DD")) {
    tU8 day = apTime->GetDay();
    if (day < 10) {
      aRet << (tU32)0L;
    }
    aRet << day;
    aFormat.erase(0,2);
  }

  else if (aFormat.StartsWith("Do")) {
    aFormat.erase(0,2);
    aRet << apTime->GetDay();
    aRet << _getNumeralTh(aRet.back());
  }

  else if (aFormat.StartsWith("D")) {
    aRet << apTime->GetDay();
    aFormat.erase(0,1);
  }

  else if (aFormat.StartsWith("dddd")) {
    aFormat.erase(0,4);
    tU32 d = apTime->GetWeekday();
    niCheck(d>=0 && d<niCountOf(_kaszWeekdayFull),;);
    aRet << _kaszWeekdayFull[d];
  }

  else if (aFormat.StartsWith("ddd")) {
    aFormat.erase(0,3);
    tU32 d = apTime->GetWeekday();
    niCheck(d>=0 && d<niCountOf(_kaszWeekday3),;);
    aRet << _kaszWeekday3[d];
  }

  else if (aFormat.StartsWith("dd")) {
    aFormat.erase(0,2);
    tU32 d = apTime->GetWeekday();
    niCheck(d>=0 && d<niCountOf(_kaszWeekday2),;);
    aRet << _kaszWeekday2[d];
  }

  else if (aFormat.StartsWith("do")) {
    aFormat.erase(0,2);
    aRet << apTime->GetWeekday();
    aRet << _getNumeralTh(aRet.back());
  }

  else if (aFormat.StartsWith("d")) {
    aFormat.erase(0,1);
    aRet << apTime->GetWeekday();
  }

  else if (aFormat.StartsWith("a")) {
    tU8 hour = apTime->GetHour();
    if (hour < 12) {
      aRet << "am";
    } else {
      aRet << "pm";
    }
    aFormat.erase(0,1);
  }

  else if (aFormat.StartsWith("A")) {
    tU8 hour = apTime->GetHour();
    if (hour < 12) {
      aRet << "AM";
    } else {
      aRet << "PM";
    }
    aFormat.erase(0,1);
  }

  else if (aFormat.StartsWith("hh")) {
    tU8 hour = apTime->GetHour();
    if (hour > 12) {
      hour -= 12;
    }
    if (hour < 10) {
      aRet << (tU32)0L;
    }
    aRet << hour;
    aFormat.erase(0,2);
  }

  else if (aFormat.StartsWith("h")) {
    tU8 hour = apTime->GetHour();
    if (hour > 12) {
      aRet << (tU32)(hour - 12L);
    } else {
      aRet << hour;
    }
    aFormat.erase(0,1);
  }

  else if (aFormat.StartsWith("HH")) {
    tU8 hour = apTime->GetHour();
    if (hour < 10) {
      aRet << (tU32)0L;
    }
    aRet << hour;
    aFormat.erase(0,2);
  }

  else if (aFormat.StartsWith("H")) {
    aRet << apTime->GetHour();
    aFormat.erase(0,1);
  }

  else if (aFormat.StartsWith("mm")) {
    tU8 minute = apTime->GetMinute();
    if (minute < 10) {
      aRet << (tU32)0L;
    }
    aRet << minute;
    aFormat.erase(0,2);
  }

  else if (aFormat.StartsWith("m")) {
    aRet << apTime->GetMinute();
    aFormat.erase(0,1);
  }

  else if (aFormat.StartsWith("ss")) {
    tU8 second = apTime->GetSecond();
    if (second < 10) {
      aRet << (tU32)0L;
    }
    aRet << second;
    aFormat.erase(0,2);
  }

  else if (aFormat.StartsWith("s")) {
    aRet << apTime->GetSecond();
    aFormat.erase(0,1);
  }

  else if (aFormat.StartsWith("ZZ") || aFormat.StartsWith("Z")) {
    const tBool isZZ = aFormat.StartsWith("ZZ");
    if (isZZ) {
      aFormat.erase(0,2);
    }
    else {
      aFormat.erase(0,1);
    }

    const tI32 tz = apTime->GetTimeZone();
    if (tz == 0) {
      aRet << "Z";
    }
    else {
      // Note that the sign is inverted because TimeZone is the difference
      // (offset) between UTC and local time.
      // Such that LocalTime + TimeZone = UTC.
      if (tz > 0) {
        aRet << "-";
      }
      else {
        aRet << "+";
      }

      const tI32 hour = ni::Abs(tz) / 3600;
      if (hour < 10) {
        aRet << (tU32)0L;
      }
      aRet << hour;

      if (!isZZ) {
        aRet << ":";
      }

      const tI32 mins = (ni::Abs(tz)-(hour*3600)) / 60;
      if (mins < 10) {
        aRet << (tU32)0L;
      }
      aRet << mins;
    }
  }

  else {
    aRet << aFormat.Mid(0,1);
    aFormat.erase(0,1);
  }
}

static bool _StrFormatTimeAToI(const char * & s, int & result, int low, int high, int offset)
{
  bool worked = false;
  const char* end;
  unsigned long num = StrToUL(s, & end, 10);
  if (num >= (unsigned long)low && num <= (unsigned long)high) {
    result = (int)(num + offset);
    s = end;
    worked = true;
  }
  return worked;
}

static const char* StrParseTime(const char *s, const char *format, struct sFullTM *tm) {
  if (!niStringIsOK(format) || StrIEq(format,"ISO8601")) {
    return StrParseTime(s, "%FT%T%z", tm);
  }

  bool working = true;
  while (working && *format && *s)
  {
    switch (*format)
    {
      case '%':
        {
          ++format;
          switch (*format)
          {
            case 'a':
            case 'A': // weekday name
              tm->tm_wday = -1;
              working = false;
              for (int i = 0; i < 7; ++ i)
              {
                size_t len = StrLen(_kaszWeekdayFull[i]);
                if (ni::StrNICmp(_kaszWeekdayFull[i], s, len) == 0) {
                  tm->tm_wday = i;
                  s += len;
                  working = true;
                  break;
                }
                else if (ni::StrNICmp(_kaszWeekdayFull[i], s, 3) == 0) {
                  tm->tm_wday = i;
                  s += 3;
                  working = true;
                  break;
                }
              }
              break;
            case 'b':
            case 'B':
            case 'h': // month name
              tm->tm_mon = -1;
              working = false;
              for (int i = 0; i < 12; ++ i)
              {
                size_t len = strlen(_kaszMonthFull[i]);
                if (ni::StrNICmp(_kaszMonthFull[i], s, len) == 0) {
                  tm->tm_mon = i;
                  s += len;
                  working = true;
                  break;
                }
                else if (ni::StrNICmp(_kaszMonthFull[i], s, 3) == 0) {
                  tm->tm_mon = i;
                  s += 3;
                  working = true;
                  break;
                }
              }
              break;
            case 'd':
            case 'e': // day of month number
              working = _StrFormatTimeAToI(s, tm->tm_mday, 1, 31, 0);
              break;
            case 'H': // hour
              working = _StrFormatTimeAToI(s, tm->tm_hour, 0, 23, 0);
              break;
            case 'I': // hour 12-hour clock
              working = _StrFormatTimeAToI(s, tm->tm_hour, 1, 12, 0);
              break;
            case 'j': // day number of year
              working = _StrFormatTimeAToI(s, tm->tm_yday, 1, 366, -1);
              break;
            case 'm': // month number
              working = _StrFormatTimeAToI(s, tm->tm_mon, 1, 12, 0);
              break;
            case 'M': // minute
              working = _StrFormatTimeAToI(s, tm->tm_min, 0, 59, 0);
              break;
            case 'n': // arbitrary whitespace
            case 't':
              while (StrIsSpace(*s))
                ++s;
              break;
            case 'p': // am / pm
              if (!ni::StrNICmp(s, "am", 2))
              { // the hour will be 1 -> 12 maps to 12 am, 1 am .. 11 am, 12 noon 12 pm .. 11 pm
                if (tm->tm_hour == 12) // 12 am == 00 hours
                  tm->tm_hour = 0;
              }
              else if (!ni::StrNICmp(s, "pm", 2))
              {
                if (tm->tm_hour < 12) // 12 pm == 12 hours
                  tm->tm_hour += 12; // 1 pm -> 13 hours, 11 pm -> 23 hours
              }
              else
                working = false;
              break;
            case 'r': // 12 hour clock %I:%M:%S %p
              {
                const char * s_save = s;
                working = _StrFormatTimeAToI(s, tm->tm_hour, 1, 12, 0);
                if (working && *s == ':')
                {
                  ++ s;
                  working = _StrFormatTimeAToI(s, tm->tm_min, 0, 59, 0);
                  if (working && *s == ':')
                  {
                    ++ s;
                    working = _StrFormatTimeAToI(s, tm->tm_sec, 0, 60, 0);
                    if (working && StrIsSpace(*s))
                    {
                      ++ s;
                      while (StrIsSpace(*s))
                        ++s;
                      if (!ni::StrNICmp(s, "am", 2))
                      { // the hour will be 1 -> 12 maps to 12 am, 1 am .. 11 am, 12 noon 12 pm .. 11 pm
                        if (tm->tm_hour == 12) // 12 am == 00 hours
                          tm->tm_hour = 0;
                      }
                      else if (!ni::StrNICmp(s, "pm", 2))
                      {
                        if (tm->tm_hour < 12) // 12 pm == 12 hours
                          tm->tm_hour += 12; // 1 pm -> 13 hours, 11 pm -> 23 hours
                      }
                      else
                        working = false;
                    }
                  }
                }
                if (!working)
                  s = s_save;
              }
              break;
            case 'R': // %H:%M
              {
                const char * s_save = s;
                working = _StrFormatTimeAToI(s, tm->tm_hour, 0, 23, 0);
                if (working && *s == ':')
                {
                  ++ s;
                  working = _StrFormatTimeAToI(s, tm->tm_min, 0, 59, 0);
                }
                if (!working)
                  s = s_save;
              }
              break;
            case 'S': // seconds
              working = _StrFormatTimeAToI(s, tm->tm_sec, 0, 60, 0);
              break;
            case 'w': // weekday number 0->6 sunday->saturday
              working = _StrFormatTimeAToI(s, tm->tm_wday, 0, 6, 0);
              break;
            case 'Y': // year
              working = _StrFormatTimeAToI(s, tm->tm_year, 1900, 65535, -1900);
              break;
            case 'y': // 2-digit year
              working = _StrFormatTimeAToI(s, tm->tm_year, 0, 99, 0);
              if (working && tm->tm_year < 69)
                tm->tm_year += 100;
              break;

            case 'z': {
              int val = 0;
              while (StrIsSpace(*s)) {
                ++s;
              }
              if (*s == 'Z') {
                ++s;
                tm->tm_gmtoff = 0;
              }
              else {
                if ((*s != '+') && (*s != '-')) {
                  return NULL;
                }
                bool neg = (*s++ == '-');
                {
                  int n = 0;
                  // +/-01 & +/-0130
                  while ((n < 4) && (*s >= '0') && (*s <= '9')) {
                    val = val * 10 + (*s++) - '0';
                    ++n;
                  }

                  if (n == 2) {
                    // +/-01:30
                    if (*s == ':') {
                      ++s;
                      while ((n < 4) && (*s >= '0') && (*s <= '9')) {
                        val = val * 10 + (*s++) - '0';
                        ++n;
                      }
                    }
                    else {
                      // to hours
                      val *= 100;
                    }
                  }

                  if (n == 4) {
                    /* We have to convert the minutes into decimal.  */
                    if (val % 100 >= 60)
                      return NULL;
                    val = (val / 100) * 100 + ((val % 100) * 50) / 30;
                  }
                  else if ((n != 4) && (n != 2)) {
                    /* Only two or four digits recognized.  */
                    return NULL;
                  }
                }

                if (val > 1200)
                  return NULL;

                tm->tm_gmtoff = (val * 3600) / 100;
                if (neg) {
                  tm->tm_gmtoff = -tm->tm_gmtoff;
                }
              }
              break;
            }

              // %Y-%m-%d
            case 'F':  {
              const char * s_save = s;
              working = _StrFormatTimeAToI(s, tm->tm_year, 1900, 65535, -1900);
              if (working && *s == '-') {
                ++ s;
                working = _StrFormatTimeAToI(s, tm->tm_mon, 1, 12, 0);
                if (working && *s == '-') {
                  ++ s;
                  working = _StrFormatTimeAToI(s, tm->tm_mday, 1, 31, 0);
                }
              }
              if (!working)
                s = s_save;
              break;
            }

              // %H:%M:%S
            case 'T':  {
              const char * s_save = s;
              working = _StrFormatTimeAToI(s, tm->tm_hour, 0, 23, 0);
              if (working && *s == ':') {
                ++ s;
                working = _StrFormatTimeAToI(s, tm->tm_min, 0, 59, 0);
                if (working && *s == ':') {
                  ++ s;
                  working = _StrFormatTimeAToI(s, tm->tm_sec, 0, 60, 0);
                }
              }
              if (!working)
                s = s_save;
              break;
            }

            case '%': // escaped
              if (*s != '%')
                working = false;
              ++s;
              break;

            default:
              working = false;
          }
        }
        break;

      case ' ':
      case '\t':
      case '\r':
      case '\n':
      case '\f':
      case '\v': {
        // zero or more whitespaces:
        while (StrIsSpace(*s)) {
          ++s;
        }
        break;
      }

      default: {
        // match character
        if (*s != *format)
          working = false;
        else
          ++s;
        break;
      }
    }
    ++format;
  }
  return (working?s:0);
}

class cTime : public ImplRC<iTime,eImplFlags_Default,iToString>
{
 public:
  cTime() {
    ZeroMembers();
  }

  void ZeroMembers() {
    memset(&mUTC,0,sizeof(mUTC));
    mUTC.tm_sec = 0;
    mUTC.tm_min = 0;
    mUTC.tm_hour = 0;
    mUTC.tm_mday = 1;
    mUTC.tm_mon = 1;
    mUTC.tm_year = 0;
    mnTimeZone = 0;
    mbDayLightSaving = ni::eFalse;
  }

  void __stdcall SetFromTimeT(const time_t* apTimeT) {
    struct tm *newtime;
    newtime = localtime(apTimeT);

    SetYear(newtime->tm_year+1900);
    SetMonth(newtime->tm_mon+1);
    SetDay(newtime->tm_mday);
    SetHour(newtime->tm_hour);
    SetMinute(newtime->tm_min);
    SetSecond(newtime->tm_sec);
    mbDayLightSaving = newtime->tm_isdst ? eTrue : eFalse;
#if defined niAndroid
    mnTimeZone = 0;
#elif defined niWindows || defined niJSCC
    mnTimeZone = _timezone;
#else
    mnTimeZone = timezone;
#endif
  }

  void __stdcall UpdateFromCurrentSystemTime() {
    time_t long_time;
    time (&long_time);
    SetFromTimeT(&long_time);
  }

  void __stdcall SetYear(tI32 anYear) {
    mUTC.tm_year = anYear-1900;
  }
  tI32 __stdcall GetYear() const {
    return mUTC.tm_year+1900;
  }
  void __stdcall SetMonth(tU8 anMonth) {
    mUTC.tm_mon = anMonth;
  }
  tU8 __stdcall GetMonth() const {
    return mUTC.tm_mon;
  }
  void __stdcall SetDay(tU8 anDay) {
    mUTC.tm_mday = anDay;
  }
  tU8 __stdcall GetDay() const {
    return mUTC.tm_mday;
  }

  void __stdcall SetHour(tU8 anHour) {
    mUTC.tm_hour = anHour;
  }
  tU8 __stdcall GetHour() const {
    return mUTC.tm_hour;
  }
  void __stdcall SetMinute(tU8 anMinute) {
    mUTC.tm_min = anMinute;
  }
  tU8 __stdcall GetMinute() const {
    return mUTC.tm_min;
  }
  void __stdcall SetSecond(tU8 anSecond) {
    mUTC.tm_sec = anSecond;
  }
  tU8 __stdcall GetSecond() const {
    return mUTC.tm_sec;
  }

  tBool __stdcall Copy(const iTime* apTime) {
    SetYear(apTime->GetYear());
    SetMonth(apTime->GetMonth());
    SetDay(apTime->GetDay());
    SetHour(apTime->GetHour());
    SetMinute(apTime->GetMinute());
    SetSecond(apTime->GetSecond());
    SetDayLightSaving(apTime->GetDayLightSaving());
    SetTimeZone(apTime->GetTimeZone());
    return eTrue;
  }

  iTime* __stdcall Clone() const {
    cTime* pNew = niNew cTime();
    pNew->Copy(this);
    return pNew;
  }

  void __stdcall SetDayLightSaving(ni::tBool abDayLightSaving) {
    mbDayLightSaving = abDayLightSaving;
  }
  tBool __stdcall GetDayLightSaving() const {
    return mbDayLightSaving;
  }

  inline void __stdcall SetTimeZone(tI32 anTimeZone) {
    mnTimeZone = anTimeZone;
  }
  inline tI32 __stdcall GetTimeZone() const {
    return mnTimeZone;
  }

  tU8 __stdcall GetWeekday() const {
    const static tU8 weekOffset[] = {0,3,3,6,1,4,6,2,5,0,3,5};
    const static tU8 weekOffsetLeap[] = {6,2,3,6,1,4,6,2,5,0,3,5};
    tU32 century = (3 - (GetYear() / 100 % 4)) * 2;
    tU32 year = century + GetYear() % 100 + GetYear() % 100 / 4;
    tU32 month = weekOffset[GetMonth()-1] + year;
    if (GetYear() % 4 == 0) {
      month = weekOffsetLeap[GetMonth()-1] + year;
    }
    return (month + GetDay()) % 7;
  }

  iTime* __stdcall AddSeconds(tI64 anSeconds) {
    tI64 secs = UTCTMToSeconds(&mUTC);
    secs += anSeconds;
    SecondsToUTCTM(secs,&mUTC);
    return this;
  }

  //! Compare to another time object.
  //! \return 0 if equal, -1 if this is before apTime, +1 if this is after apTime.
  tI32 __stdcall Compare(const iTime* apTime) const {
    if (!niIsOK(apTime)) return 2;
    const tI64 thisSecsZ = this->GetUnixTimeSecs();
    const tI64 rightSecsZ = apTime->GetUnixTimeSecs();
    if (thisSecsZ < rightSecsZ) return -1;
    if (thisSecsZ > rightSecsZ) return  1;
    return 0; // thisSecsZ == rightSecsZ
  }

  void __stdcall SetUnixTimeSecs(tI64 anTimestamp) {
    tI64 secs = anTimestamp;
    secs -= mnTimeZone; // To TZ
    SecondsToUTCTM(secs,&mUTC);
  }
  tI64 __stdcall GetUnixTimeSecs() const {
    tI64 secs = UTCTMToSeconds(&mUTC);
    secs += mnTimeZone; // To UTC
    return secs;
  }

  iTime* __stdcall ParseString(const achar* aszTime, const achar* aszFormat) {
    sFullTM tm;
    StrParseTime(aszTime, aszFormat, &tm);
    mUTC = tm;
    mnTimeZone = -tm.tm_gmtoff;
    return this;
  }

  cString __stdcall Format(const achar* aszFormat) const {
    if (!niStringIsOK(aszFormat)) {
      aszFormat = "ISO8601";
    }
    cString format(aszFormat);
    cString ret;
    while (!format.empty()) {
      _formatDateTime(this,format,ret);
    }
    return ret;
  }

  cString __stdcall ToString() const {
    return niFmt("%d/%02d/%02d-%02d:%02d:%02d",
                 GetYear(), GetMonth(), GetDay(),
                 GetHour(), GetMinute(), GetSecond());
  }

 private:
  sUTCTM mUTC;
  tI32   mnTimeZone;
  tBool  mbDayLightSaving;
};

namespace ni {

niExportFunc(ni::iTime*) CreateTimeZero() {
  return niNew cTime();
}

niExportFunc(ni::iTime*) GetCurrentTime() {
  static Ptr<iTime> _currenTime;
  if (niIsNullPtr(_currenTime)) {
    _currenTime = niNew cTime();
  }
  _currenTime->UpdateFromCurrentSystemTime();
  return _currenTime;
}

niExportFunc(tBool) SetTimeFromTimeT(ni::iTime* apTime, const void* apTimeT) {
  if (!apTimeT || !niIsOK(apTime))
    return eFalse;
  ((cTime*)apTime)->SetFromTimeT((const time_t*)apTimeT);
  return eTrue;
}

}
