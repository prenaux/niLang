#pragma once
#ifndef __TIME_20124180_H__
#define __TIME_20124180_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "Types.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */

//! Time zone enumeration
enum eTimeZone
{
  //! Coordinated Universal Time  Europe  UTC
  eTimeZone_UTC = (tU32)0,
  //! Western European Time Europe  UTC
  eTimeZone_WET = (tU32)0,
  //! Greenwich Mean Time Europe  UTC
  eTimeZone_GMT = (tU32)0,
  //! Alpha Time Zone Military  UTC + 1 hour
  eTimeZone_A = (tU32)+ 1 * 3600,
  //! Atlantic Daylight Time  North America UTC - 3 hours
  eTimeZone_ADT = (tU32)- 3 * 3600,
  //! Alaska Daylight Time  North America UTC - 8 hours
  eTimeZone_AKDT  = (tU32)- 8 * 3600,
  //! Alaska Standard Time  North America UTC - 9 hours
  eTimeZone_AKST  = (tU32)- 9 * 3600,
  //! Atlantic Standard Time  North America UTC - 4 hours
  eTimeZone_AST = (tU32)- 4 * 3600,
  //! Bravo Time Zone Military  UTC + 2 hours
  eTimeZone_B = (tU32)+ 2 * 3600,
  //! British Summer Time Europe  UTC + 1 hour
  eTimeZone_BST = (tU32)+ 1 * 3600,
  //! Charlie Time Zone Military  UTC + 3 hours
  eTimeZone_C = (tU32)+ 3 * 3600,
  //! Central Daylight Time North America UTC - 5 hours
  eTimeZone_CDT = (tU32)- 5 * 3600,
  //! Central European Daylight Time  Europe  UTC + 2 hours
  eTimeZone_CEDT  = (tU32)+ 2 * 3600,
  //! Central European Summer Time  Europe  UTC + 2 hours
  eTimeZone_CEST  = (tU32)+ 2 * 3600,
  //! Central European Time Europe  UTC + 1 hour
  eTimeZone_CET = (tU32)+ 1 * 3600,
  //! Central Standard Time North America UTC - 6 hours
  eTimeZone_CST = (tU32)- 6 * 3600,
  //! Delta Time Zone Military  UTC + 4 hours
  eTimeZone_D = (tU32)+ 4 * 3600,
  //! Echo Time Zone  Military  UTC + 5 hours
  eTimeZone_E = (tU32)+ 5 * 3600,
  //! Eastern Daylight Time North America UTC - 4 hours
  eTimeZone_EDT = (tU32)- 4 * 3600,
  //! Eastern European Daylight Time  Europe  UTC + 3 hours
  eTimeZone_EEDT  = (tU32)+ 3 * 3600,
  //! Eastern European Summer Time  Europe  UTC + 3 hours
  eTimeZone_EEST  = (tU32)+ 3 * 3600,
  //! Eastern European Time Europe  UTC + 2 hours
  eTimeZone_EET = (tU32)+ 2 * 3600,
  //! Eastern Standard Time North America UTC - 5 hours
  eTimeZone_EST = (tU32)- 5 * 3600,
  //! Foxtrot Time Zone Military  UTC + 6 hours
  eTimeZone_F = (tU32)+ 6 * 3600,
  //! Golf Time Zone  Military  UTC + 7 hours
  eTimeZone_G = (tU32)+ 7 * 3600,
  //! Hotel Time Zone Military  UTC + 8 hours
  eTimeZone_H = (tU32)+ 8 * 3600,
  //! Heure Avancee de l'Atlantique North America UTC - 3 hours
  eTimeZone_HAA = (tU32)- 3 * 3600,
  //! Heure Avancee du Centre North America UTC - 5 hours
  eTimeZone_HAC = (tU32)- 5 * 3600,
  //! Hawaii-Aleutian Daylight Time North America UTC - 9 hours
  eTimeZone_HADT  = (tU32)- 9 * 3600,
  //! Heure Avancee de l'Est  North America UTC - 4 hours
  eTimeZone_HAE = (tU32)- 4 * 3600,
  //! Heure Avancee du Pacifique  North America UTC - 7 hours
  eTimeZone_HAP = (tU32)- 7 * 3600,
  //! Heure Avancee des Rocheuses North America UTC - 6 hours
  eTimeZone_HAR = (tU32)- 6 * 3600,
  //! Hawaii-Aleutian Standard Time North America UTC - 10 hours
  eTimeZone_HAST  = (tU32)- 10 * 3600 - 1800,
  //! Heure Avancee de Terre-Neuve  North America UTC - 2:30 hours
  eTimeZone_HAT = (tU32)- 2 * 3600 - 1800,
  //! Heure Avancee du Yukon  North America UTC - 8 hours
  eTimeZone_HAY = (tU32)- 8 * 3600,
  //! Heure Normale de l'Atlantique North America UTC - 4 hours
  eTimeZone_HNA = (tU32)- 4 * 3600,
  //! Heure Normale du Centre North America UTC - 6 hours
  eTimeZone_HNC = (tU32)- 6 * 3600,
  //! Heure Normale de l'Est  North America UTC - 5 hours
  eTimeZone_HNE = (tU32)- 5 * 3600,
  //! Heure Normale du Pacifique  North America UTC - 8 hours
  eTimeZone_HNP = (tU32)- 8 * 3600,
  //! Heure Normale des Rocheuses North America UTC - 7 hours
  eTimeZone_HNR = (tU32)- 7 * 3600,
  //! Heure Normale de Terre-Neuve  North America UTC - 3:30 hours
  eTimeZone_HNT = (tU32)- 3 * 3600 - 1800,
  //! Heure Normale du Yukon  North America UTC - 9 hours
  eTimeZone_HNY = (tU32)- 9 * 3600,
  //! India Time Zone Military  UTC + 9 hours
  eTimeZone_I = (tU32)+ 9 * 3600,
  //! Irish Summer Time Europe  UTC + 1 hour
  eTimeZone_IST = (tU32)+ 1 * 3600,
  //! Kilo Time Zone  Military  UTC + 10 hours
  eTimeZone_K = (tU32)+ 10 * 3600,
  //! Lima Time Zone  Military  UTC + 11 hours
  eTimeZone_L = (tU32)+ 11 * 3600,
  //! Mike Time Zone  Military  UTC + 12 hours
  eTimeZone_M = (tU32)+ 12 * 3600,
  //! Mountain Daylight Time  North America UTC - 6 hours
  eTimeZone_MDT = (tU32)- 6 * 3600,
  //! Mitteleuroaische Sommerzeit Europe  UTC + 2 hours
  eTimeZone_MESZ  = (tU32)+ 2 * 3600,
  //! Mitteleuropaische Zeit  Europe  UTC + 1 hour
  eTimeZone_MEZ = (tU32)+ 1 * 3600,
  //! Mountain Standard Time  North America UTC - 7 hours
  eTimeZone_MST = (tU32)- 7 * 3600,
  //! November Time Zone  Military  UTC - 1 hour
  eTimeZone_N = (tU32)- 1 * 3600,
  //! Newfoundland Daylight Time  North America UTC - 2:30 hours
  eTimeZone_NDT = (tU32)- 2 * 3600 - 1800,
  //! Newfoundland Standard Time  North America UTC - 3:30 hours
  eTimeZone_NST = (tU32)- 3 * 3600 - 1800,
  //! Oscar Time Zone Military  UTC - 2 hours
  eTimeZone_O = (tU32)- 2 * 3600,
  //! Papa Time Zone  Military  UTC - 3 hours
  eTimeZone_P = (tU32)- 3 * 3600,
  //! Pacific Daylight Time North America UTC - 7 hours
  eTimeZone_PDT = (tU32)- 7 * 3600,
  //! Pacific Standard Time North America UTC - 8 hours
  eTimeZone_PST = (tU32)- 8 * 3600,
  //! Quebec Time Zone  Military  UTC - 4 hours
  eTimeZone_Q = (tU32)- 4 * 3600,
  //! Romeo Time Zone Military  UTC - 5 hours
  eTimeZone_R = (tU32)- 5 * 3600,
  //! Sierra Time Zone  Military  UTC - 6 hours
  eTimeZone_S = (tU32)- 6 * 3600,
  //! Tango Time Zone Military  UTC - 7 hours
  eTimeZone_T = (tU32)- 7 * 3600,
  //! Uniform Time Zone Military  UTC - 8 hours
  eTimeZone_U = (tU32)- 8 * 3600,
  //! Victor Time Zone  Military  UTC - 9 hours
  eTimeZone_V = (tU32)- 9 * 3600,
  //! Whiskey Time Zone Military  UTC - 10 hours
  eTimeZone_W = (tU32)- 10 * 3600,
  //! Western European Daylight Time  Europe  UTC + 1 hour
  eTimeZone_WEDT  = (tU32)+ 1 * 3600,
  //! Western European Summer Time  Europe  UTC + 1 hour
  eTimeZone_WEST  = (tU32)+ 1 * 3600,
  //! X-ray Time Zone Military  UTC - 11 hours
  eTimeZone_X = (tU32)- 11 * 3600,
  //! Yankee Time Zone  Military  UTC - 12 hours
  eTimeZone_Y = (tU32)- 12 * 3600,
  //! Zulu Time Zone  Military  UTC
  eTimeZone_Z = (tU32)0,
  //! \internal
  eTimeZone_ForceDWORD = 0xFFFFFFFF
};

//! Minimum/Maximum year that the time object can hold. (+/- ~2.1 billion years)
const tI32 knTimeYearRange = 2147000000;
//! Internal format seconds per minute.
const tI64 knTimeSecsPerMinute = 60;
//! Internal format minutes per hour.
const tI64 knTimeMinPerHour = 60;
//! Internal format seconds per hour.
const tI64 knTimeSecsPerHour = knTimeMinPerHour*knTimeSecsPerMinute;
//! Internal format hours per day.
const tI64 knTimeHoursPerDay = 24;
//! Internal format minutes per day.
const tI64 knTimeMinPerDay = knTimeMinPerHour*knTimeHoursPerDay;
//! Internal format seconds per day.
const tI64 knTimeSecsPerDay = knTimeMinPerDay*knTimeSecsPerMinute;

//! Time interface.
struct iTime : public iUnknown
{
  niDeclareInterfaceUUID(iTime,0x2b3e468b,0x96fc,0x4abd,0x86,0x94,0x48,0x10,0xb3,0x17,0xd4,0x2d)

  //! Update the time class from the current system's time.
  virtual void __stdcall UpdateFromCurrentSystemTime() = 0;
  //! Set the year.
  //! {Property}
  virtual void __stdcall SetYear(tI32 anYear) = 0;
  //! Get the year.
  //! {Property}
  virtual tI32 __stdcall GetYear() const = 0;
  //! Set the month.
  //! {Property}
  virtual void __stdcall SetMonth(tU8 anMonth) = 0;
  //! Get the month.
  //! {Property}
  virtual tU8 __stdcall GetMonth() const = 0;
  //! Set the day.
  //! {Property}
  virtual void __stdcall SetDay(tU8 anDay) = 0;
  //! Get the day.
  //! {Property}
  virtual tU8 __stdcall GetDay() const = 0;
  //! Get the weekday.
  //! {Property}
  virtual tU8 __stdcall GetWeekday() const = 0;
  //! Set the hour.
  //! {Property}
  virtual void __stdcall SetHour(tU8 anHour) = 0;
  //! Get the hour.
  //! {Property}
  virtual tU8 __stdcall GetHour() const = 0;
  //! Set the minute.
  //! {Property}
  virtual void __stdcall SetMinute(tU8 anMinute) = 0;
  //! Get the minute.
  //! {Property}
  virtual tU8 __stdcall GetMinute() const = 0;
  //! Set the second.
  //! {Property}
  virtual void __stdcall SetSecond(tU8 anSecond) = 0;
  //! Get the second.
  //! {Property}
  virtual tU8 __stdcall GetSecond() const = 0;

  //! Copy the specified time.
  virtual tBool __stdcall Copy(const iTime* apTime) = 0;
  //! Creates a clone of this time.
  virtual iTime* __stdcall Clone() const = 0;

  //! Set whether the time contains day light saving.
  //! {Property}
  virtual void __stdcall SetDayLightSaving(ni::tBool abDayLightSaving) = 0;
  //! Get whether the time contains day light saving.
  //! {Property}
  virtual tBool __stdcall GetDayLightSaving() const = 0;

  //! Set the time zone.
  //! {Property}
  //! \remark Offset from UTC in seconds.
  virtual void __stdcall SetTimeZone(tI32 anTimeZone) = 0;
  //! Get the time zone.
  //! {Property}
  //! \remark Offset from UTC in seconds.
  virtual tI32 __stdcall GetTimeZone() const = 0;

  //! Set the time from the unix time.
  //! {Property}
  virtual void __stdcall SetUnixTimeSecs(tI64 anFmt) = 0;
  //! Get the unix time.
  //! {Property}
  virtual tI64 __stdcall GetUnixTimeSecs() const = 0;

  //! Compare to another time object.
  //! \return 0 if equal, -1 if this is before apTime, +1 if this is after apTime, 2 if apTime is invalid.
  virtual tI32 __stdcall Compare(const iTime* apTime) const = 0;

  //! Parse a time string.
  //! \remark Parse format:
  //          %a    Abbreviated weekday name. Ex: Thu
  //!         %A    Full weekday name. Ex: Thursday
  //!         %b    Abbreviated month name. Ex: Aug
  //!         %B    Full month name. Ex: August
  //!         %d    Day of the month, zero-padded (01-31). Ex: 23
  //!         %e    Day of the month, space-padded ( 1-31). Ex: 23
  //!         %h    Abbreviated month name (same as %b). Ex: Aug
  //!         %H    Hour in 24h format (00-23). Ex: 14
  //!         %I    Hour in 12h format (01-12). Ex: 02
  //!         %j    Day of the year (001-366). Ex: 235
  //!         %m    Month as a decimal number (01-12). Ex: 08
  //!         %M    Minute (00-59). Ex: 55
  //!         %n    New-line character ('\n').
  //!         %t    Horizontal-tab character ('\t').
  //!         %p    AM or PM designation. Ex: PM
  //!         %r    12-hour clock time. Ex: 02:55:02 pm
  //!         %R    24-hour HH:MM time, equivalent to %H:%M. Ex: 14:55
  //!         %S    Second (00-61). Ex: 02
  //!         %w    Weekday as a decimal number with Sunday as 0 (0-6). Ex: 4
  //!         %y    Year, last two digits (00-99). Ex: 01
  //!         %Y    Year. Ex: 2001
  //!         %z    An RFC-822/ISO 8601 standard timezone specification. Ex: Z (GMT), +08 (+8 hours), -0630 (-6hrs & 30 minutes)
  //!         %F    ISO 8601 date format (YYYY-MM-DD), equivalent to %Y-%m-%d. Ex: 2016-08-03
  //!         %T    ISO 8601 time format (HH:MM:SS), equivalent to %H:%M:%S. Ex: 05:55:10
  //!         %%    A % sign
  //!         ISO8601 or NULL or empty string, ISO8601 date time format, equivalent to: %FT%T%z & %Y-%m-%dT%H:%M:%S%z
  //! \return This time object.
  virtual iTime* __stdcall ParseString(const achar* szString, const achar* aszFormat) = 0;

  //! Format time.
  //! \remark Format syntax:
  //!         YYYY    year          2011
  //!         YY      year          99
  //!         M       month         1 2 ... 30 31
  //!         MM      month         01 02 ... 30 31
  //!         MMM     month         Mar
  //!         MMMM    month         March
  //!         D       day of month  1 2 ... 30 31
  //!         Do      day of month  1st 2nd ... 30th 31st
  //!         DD      day of month  01 02 ... 30 31
  //!         d       day of week   0 1 ... 5 6
  //!         do      day of week   0th 1st ... 5th 6th
  //!         dd      day of week   Su
  //!         ddd     day of week   Sun
  //!         dddd    day of week   Sunday
  //!         e       day of week   0 1 ... 5 6
  //!         E       day of week   1 2 ... 6 7
  //!         a       am/pm marker  am/pm
  //!         A       AM/PM marker  AM/PM
  //!         H       hour(0-23)    16
  //!         HH      hour(0-23)    06
  //!         h       hour(1-12)    4
  //!         hh      hour(1-12)    04
  //!         m       minute        6
  //!         mm      minute        06
  //!         s       second        6
  //!         ss      second        06
  //!         Z       timezone      -07:00 -06:00 ... +06:00 +07:00
  //!         ZZ      timezone      -0700 -0600 ... +0600 +0700
  //!         RFC1123 RFC1123       Wed, 16 Feb 2011 03:34:56 Z
  //!         RFC850  RFC850        Wednesday, 16-Feb-11 03:34:56 Z
  //!         ASCTIME ASCTIME       Wed Feb 16 03:34:56 2011
  //!         ISO8601 or NULL or empty string, ISO8601, 2014-09-08T08:02:17-05:00
  //!         [text to include verbatim]
  virtual cString __stdcall Format(const achar* aszFormat) const = 0;

  //! Add seconds to this time.
  virtual iTime* __stdcall AddSeconds(tI64 anSeconds) = 0;
};

niExportFunc(ni::iTime*) CreateTimeZero();
niExportFunc(ni::iTime*) GetCurrentTime();
// apTimeT must be a pointer to time_t variable.
niExportFunc(ni::tBool) SetTimeFromTimeT(ni::iTime* apTime, const void* apTimeT);

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}; // End of ni
#endif // __TIME_20124180_H__
