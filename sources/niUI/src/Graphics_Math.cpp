// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Graphics.h"
#include "NUSpline.h"
#include "API/niUI/Utils/DampedSpring.h"
#include "API/niUI/Utils/UDPoints.h"

///////////////////////////////////////////////
iNUSpline* __stdcall cGraphics::CreateNUSpline(eNUSplineType aType) const
{
  return niNew cNUSpline(aType);
}

///////////////////////////////////////////////
tF32 __stdcall cGraphics::ColorLuminance(const sVec3f& aColor) {
  return ni::ColorLuminance(aColor);
}

///////////////////////////////////////////////
tF32 __stdcall cGraphics::ColorLuminanceEx(const sVec3f& aColor, const sVec3f& avLuminanceDistribution) {
  return ni::ColorLuminance(aColor,avLuminanceDistribution);
}

///////////////////////////////////////////////
sVec3f __stdcall cGraphics::ColorGammaCorrect(const sVec3f& aC, tF32 afFactor)
{
  sColor3f ret;
  return ni::ColorGammaCorrect(ret,afFactor,(sColor3f*)&aC);
}

///////////////////////////////////////////////
sVec3f __stdcall cGraphics::ColorAdjustContrast(const sVec3f& C, tF32 c)
{
  sColor3f ret;
  return ni::ColorAdjustContrast(ret,(sColor3f&)C,c);
}

///////////////////////////////////////////////
sVec3f __stdcall cGraphics::ColorAdjustSaturation(const sVec3f& C, tF32 s)
{
  sColor3f ret;
  return ni::ColorAdjustSaturation(ret,(sColor3f&)C,s);
}

///////////////////////////////////////////////
sVec3f __stdcall cGraphics::ColorNegative(const sVec3f& C)
{
  return sVec3f::One() - C;
}

///////////////////////////////////////////////
sVec3f __stdcall cGraphics::ColorGrey(const sVec3f& C)
{
  sColor3f ret;
  return ni::ColorGrey(ret,(sColor3f&)C);
}

///////////////////////////////////////////////
sVec3f __stdcall cGraphics::ColorBlackWhite(const sVec3f& C, tF32 s)
{
  sColor3f ret;
  return ni::ColorBlackWhite(ret,(sColor3f&)C,s);
}

///////////////////////////////////////////////
sVec4f __stdcall cGraphics::ColorConvert(eColorSpace aSource, eColorSpace aDest, const sVec4f& aColor)
{
  return ColorConvertEx(aSource,aDest,aColor,kcolXYZRef2D65);
}

///////////////////////////////////////////////
sVec4f __stdcall cGraphics::ColorConvertEx(eColorSpace aSource, eColorSpace aDest, const sVec4f& aColor, const sVec4<tF32>& aXYZRef)
{
  return ni::ColorConvert(aSource,aDest,aColor,aXYZRef);
}

static const sEnumDef* __stdcall _GetColorEnumDef() {
  static const sEnumDef* _pColorEnumDef = NULL;
  if (!_pColorEnumDef) {
    _pColorEnumDef = ni::GetLang()->GetEnumDef("eColor");
  }
  niAssert(_pColorEnumDef != NULL);
  return _pColorEnumDef;
}

///////////////////////////////////////////////
tU32 __stdcall cGraphics::GetNumNamedColors() const {
  return _GetColorEnumDef()->mnNumValues;
}

///////////////////////////////////////////////
const achar* __stdcall cGraphics::GetColorName(tU32 anIndex) const {
  const sEnumDef* pColorEnumDef = _GetColorEnumDef();
  if (anIndex >= pColorEnumDef->mnNumValues)
    return NULL;
  return pColorEnumDef->mpValues[anIndex].maszName;
}

///////////////////////////////////////////////
const achar* __stdcall cGraphics::FindColorName(const sColor4f& aColor) const {
  const sEnumDef* pColorEnumDef = _GetColorEnumDef();
  tU32 nColor = ULColorBuild(aColor);
  tU32 nMinDiff = 0xFFFFFFFF;
  tU32 nMinIndex = pColorEnumDef->mnNumValues-1;
  for (tU32 i = 0; i < pColorEnumDef->mnNumValues; ++i) {
    const tU32 v = pColorEnumDef->mpValues[i].mnValue;
    tU32 diff = (v>nColor) ? v-nColor : nColor-v; // avoid overflow... probably not necessary, but for portability reason we make sure that their's no ambiguity
    if (diff <= nMinDiff) {
      nMinDiff = diff;
      nMinIndex = i;
      if (nMinDiff == 0)
        break;
    }
  }
  return GetColorName(nMinIndex);
}

///////////////////////////////////////////////
tU32 __stdcall cGraphics::GetColorIndex(iHString* ahspName) const {
  const sEnumDef* pColorEnumDef = _GetColorEnumDef();
  cString strName = niHStr(ahspName);
  strName.ToLower();
  for (tU32 i = 0; i < pColorEnumDef->mnNumValues; ++i) {
    if (ni::StrEq(strName.Chars(),pColorEnumDef->mpValues[i].maszName)) {
      return i;
    }
  }
  return eInvalidHandle;
}

///////////////////////////////////////////////
tU32 __stdcall cGraphics::GetColorValue(tU32 anIndex) const {
  const sEnumDef* pColorEnumDef = _GetColorEnumDef();
  return pColorEnumDef->mpValues[anIndex].mnValue;
}

///////////////////////////////////////////////
sVec4f __stdcall cGraphics::GetCssColor4FromName(iHString* ahspName) const {
  tF32 r = 0.0f;
  tF32 g = 0.0f;
  tF32 b = 0.0f;
  tF32 a = 1.0f;

  if (ni::HStringIsNotEmpty(ahspName)) {
    const achar* p = niHStr(ahspName);
    if (*p == '#') {
      cString str;

      ++p; // skip the '#'
      int len = ni::StrLen(p);
      if (len == 3) {
        // expand the string
        str.appendChar(p[0]); str.appendChar(p[0]);
        str.appendChar(p[1]); str.appendChar(p[1]);
        str.appendChar(p[2]); str.appendChar(p[2]);
        p = str.Chars();
        len = 6;
      }
      else if (len == 4) {
        // expand the string
        str.appendChar(p[0]); str.appendChar(p[0]);
        str.appendChar(p[1]); str.appendChar(p[1]);
        str.appendChar(p[2]); str.appendChar(p[2]);
        str.appendChar(p[3]); str.appendChar(p[3]);
        p = str.Chars();
        len = 8;
      }

      if (len <= 7) { // 7 shouldn't happen.. but this is the 'best' fallback
        tU32 c = (tU32)ni::StrToUL(p,NULL,16);
        r = ULColorGetRf(c);
        g = ULColorGetGf(c);
        b = ULColorGetBf(c);
      }
      else {
        achar final[9] = {0,0,0,0,0,0,0,0,0};
        // get into the AARRGGBB format which the
        // native representation (0xAARRGGBB), however
        // the spec for this format is #RGBA
        final[0] = p[6]; final[1] = p[7]; // AA
        final[2] = p[0]; final[3] = p[1]; // RR
        final[4] = p[2]; final[5] = p[3]; // GG
        final[6] = p[4]; final[7] = p[5]; // BB
        // len > 6
        tU32 c = (tU32)ni::StrToUL(final,NULL,16);
        r = ULColorGetRf(c);
        g = ULColorGetGf(c);
        b = ULColorGetBf(c);
        a = ULColorGetAf(c);
      }
    }
    else {
      tBool isRGBFloat = (*p == ':' ||
                          StrNICmp(p,"Vec",3) == 0 ||
                          StrNCmp(p,"RGB",3) == 0);
      tBool isRGBByte = ni::eFalse;
      if (!isRGBFloat) {
        isRGBByte = (StrNICmp(p,"rgb",3) == 0);
      }
      if (isRGBFloat || isRGBByte) {
#define SKIP_SPACESANDCOMMA                                       \
        while (*p && (ni::StrIsSpace(*p) || *p == ',')) { ++p; }

#define NORMALIZE_VALUE(p,v)                    \
        if (*p == '%') { v /= 100.0f; ++p; }    \
        else if (*p == 'f') { ++p; }            \
        else if (isRGBByte) { v /= 255.0f; }

        const achar* pEnd;

        // goto the (
        while (*p && *p != '(') { ++p; }

        if (*p == '(') {
          ++p; // skip the (
          // parse r
          SKIP_SPACESANDCOMMA;
          if (*p && *p != ')') {
            r = (tF32)StrToD(p,&pEnd);
            p = pEnd;
            NORMALIZE_VALUE(p,r);
          }
          // parse g
          SKIP_SPACESANDCOMMA;
          if (*p && *p != ')') {
            SKIP_SPACESANDCOMMA;
            g = (tF32)StrToD(p,&pEnd);
            p = pEnd;
            NORMALIZE_VALUE(p,g);
          }
          // parse b
          SKIP_SPACESANDCOMMA;
          if (*p && *p != ')') {
            SKIP_SPACESANDCOMMA;
            b = (tF32)StrToD(p,&pEnd);
            p = pEnd;
            NORMALIZE_VALUE(p,b);
          }
          // parse a
          SKIP_SPACESANDCOMMA;
          if (*p && *p != ')') {
            SKIP_SPACESANDCOMMA;
            a = (tF32)StrToD(p,&pEnd);
            p = pEnd;
            NORMALIZE_VALUE(p,a);
          }
        }
      }
      else {
        cString str;
        str = p;
        str.ToLower();
        tU32 v = 0;

        const sEnumDef* pColorEnumDef = _GetColorEnumDef();
        for (tU32 i = 0; i < pColorEnumDef->mnNumValues; ++i) {
          if (ni::StrEq(str.Chars(),pColorEnumDef->mpValues[i].maszName)) {
            v = pColorEnumDef->mpValues[i].mnValue;
            break;
          }
        }
        r = ULColorGetRf(v);
        g = ULColorGetGf(v);
        b = ULColorGetBf(v);
        a = ULColorGetAf(v);
      }
    }
  }

  return Vec4(r,g,b,a);
}

///////////////////////////////////////////////
sVec3f __stdcall cGraphics::GetCssColor3FromName(iHString* ahspName) const
{
  return Vec3(GetCssColor4FromName(ahspName));
}

///////////////////////////////////////////////
sVec4f __stdcall cGraphics::GetCssColor4FromNameA(iHString* ahspName, tF32 afAlpha) const
{
  sVec4f v = GetCssColor4FromName(ahspName);
  v.w = afAlpha;
  return v;
}

///////////////////////////////////////////////
sVec3f __stdcall cGraphics::GetColor3FromName(iHString* ahspName) const {
  return Vec3(GetColor4FromName(ahspName));
}

///////////////////////////////////////////////
sVec4f __stdcall cGraphics::GetColor4FromName(iHString* ahspName) const {
  if (ni::HStringIsNotEmpty(ahspName)) {
    return GetCssColor4FromName(ahspName);
  }

  return sVec4f::Black();
}

///////////////////////////////////////////////
sVec4f __stdcall cGraphics::GetColor4FromNameA(iHString* ahspName, tF32 afAlpha) const {
  sVec4f v = GetColor4FromName(ahspName);
  v.w = afAlpha;
  return v;
}

///////////////////////////////////////////////
tF32 __stdcall cGraphics::DampedSpringGetDampingRatio(const tF32 Ks, const tF32 Kd) const {
  return ni::DampedSpringGetDampingRatio(Ks,Kd);
}

///////////////////////////////////////////////
tF32 __stdcall cGraphics::DampedSpringComputeKdFromDampingRatio(const tF32 Ks, const tF32 E) const {
  return ni::DampedSpringComputeKdFromDampingRatio(Ks,E);
}

///////////////////////////////////////////////
tF32 __stdcall cGraphics::DampedSpringAcceleration1(tF32 D, tF32 V, const tF32 Ks, const tF32 Kd) const {
  return DampedSpringAcceleration(D,V,Ks,Kd);
}

///////////////////////////////////////////////
sVec2f __stdcall cGraphics::DampedSpringAcceleration2(const sVec2f& D, const sVec2f& V, const tF32 Ks, const tF32 Kd) const {
  return DampedSpringAcceleration(D,V,Ks,Kd);
}

///////////////////////////////////////////////
sVec3f __stdcall cGraphics::DampedSpringAcceleration3(const sVec3f& D, const sVec3f& V, const tF32 Ks, const tF32 Kd) const {
  return DampedSpringAcceleration(D,V,Ks,Kd);
}

///////////////////////////////////////////////
sVec4f __stdcall cGraphics::DampedSpringAcceleration4(const sVec4f& D, const sVec4f& V, const tF32 Ks, const tF32 Kd) const {
  return DampedSpringAcceleration(D,V,Ks,Kd);
}

///////////////////////////////////////////////
ni::iDampedSpring1* __stdcall cGraphics::CreateDampedSpring1(const tF32 Ks, const tF32 Kd) const {
  return niNew sDampedSpring1fImpl();
}

///////////////////////////////////////////////
ni::iDampedSpring2* __stdcall cGraphics::CreateDampedSpring2(const tF32 Ks, const tF32 Kd) const {
  return niNew sDampedSpring2fImpl();
}

///////////////////////////////////////////////
ni::iDampedSpring3* __stdcall cGraphics::CreateDampedSpring3(const tF32 Ks, const tF32 Kd) const {
  return niNew sDampedSpring3fImpl();
}

///////////////////////////////////////////////
ni::iDampedSpring4* __stdcall cGraphics::CreateDampedSpring4(const tF32 Ks, const tF32 Kd) const {
  return niNew sDampedSpring4fImpl();
}

///////////////////////////////////////////////
ni::iDampedSpringPosition1* __stdcall cGraphics::CreateDampedSpringPosition1(const tF32 Ks, const tF32 Kd) const {
  return niNew sDampedSpringPosition1fImpl();
}

///////////////////////////////////////////////
ni::iDampedSpringPosition2* __stdcall cGraphics::CreateDampedSpringPosition2(const tF32 Ks, const tF32 Kd) const {
  return niNew sDampedSpringPosition2fImpl();
}

///////////////////////////////////////////////
ni::iDampedSpringPosition3* __stdcall cGraphics::CreateDampedSpringPosition3(const tF32 Ks, const tF32 Kd) const {
  return niNew sDampedSpringPosition3fImpl();
}

///////////////////////////////////////////////
ni::iDampedSpringPosition4* __stdcall cGraphics::CreateDampedSpringPosition4(const tF32 Ks, const tF32 Kd) const {
  return niNew sDampedSpringPosition4fImpl();
}

///////////////////////////////////////////////
void __stdcall cGraphics::UDPointsSphereRandom(tVec3fCVec* apResults) const
{
  niAssert(tVec3fCVec::IsSameType(apResults));
  niCheckSilent(tVec3fCVec::IsSameType(apResults),;);
  ni::UDPointsSphereRandom(apResults->_Data(),(tU32)apResults->size());
}

///////////////////////////////////////////////
void __stdcall cGraphics::UDPointsSphereHammersley(tVec3fCVec* apResults) const
{
  niAssert(tVec3fCVec::IsSameType(apResults));
  niCheckSilent(tVec3fCVec::IsSameType(apResults),;);
  ni::UDPointsSphereHammersley(apResults->_Data(),(tU32)apResults->size());
}

///////////////////////////////////////////////
void __stdcall cGraphics::UDPointsSphereHammersleyEx(tVec3fCVec* apResults, tI32 p1) const
{
  niAssert(tVec3fCVec::IsSameType(apResults));
  niCheckSilent(tVec3fCVec::IsSameType(apResults),;);
  ni::UDPointsSphereHammersleyEx(apResults->_Data(),(tU32)apResults->size(),p1);
}

///////////////////////////////////////////////
void __stdcall cGraphics::UDPointsSphereHalton(tVec3fCVec* apResults, tI32 p2) const
{
  niAssert(tVec3fCVec::IsSameType(apResults));
  niCheckSilent(tVec3fCVec::IsSameType(apResults),;);
  ni::UDPointsSphereHalton(apResults->_Data(),(tU32)apResults->size(),p2);
}

///////////////////////////////////////////////
void __stdcall cGraphics::UDPointsSphereHaltonEx(tVec3fCVec* apResults, tI32 p1, tI32 p2) const
{
  niAssert(tVec3fCVec::IsSameType(apResults));
  niCheckSilent(tVec3fCVec::IsSameType(apResults),;);
  ni::UDPointsSphereHaltonEx(apResults->_Data(),(tU32)apResults->size(),p1,p2);
}

///////////////////////////////////////////////
void __stdcall cGraphics::UDPointsHemisphereRandom(tVec3fCVec* apResults) const
{
  niAssert(tVec3fCVec::IsSameType(apResults));
  niCheckSilent(tVec3fCVec::IsSameType(apResults),;);
  ni::UDPointsHemisphereRandom(apResults->_Data(),(tU32)apResults->size());
}

///////////////////////////////////////////////
void __stdcall cGraphics::UDPointsHemisphereHammersley(tVec3fCVec* apResults) const
{
  niAssert(tVec3fCVec::IsSameType(apResults));
  niCheckSilent(tVec3fCVec::IsSameType(apResults),;);
  ni::UDPointsHemisphereHammersley(apResults->_Data(),(tU32)apResults->size());
}

///////////////////////////////////////////////
void __stdcall cGraphics::UDPointsHemisphereHammersleyEx(tVec3fCVec* apResults, tI32 p1) const
{
  niAssert(tVec3fCVec::IsSameType(apResults));
  niCheckSilent(tVec3fCVec::IsSameType(apResults),;);
  ni::UDPointsHemisphereHammersleyEx(apResults->_Data(),(tU32)apResults->size(),p1);
}

///////////////////////////////////////////////
void __stdcall cGraphics::UDPointsHemisphereHalton(tVec3fCVec* apResults, tI32 p2) const
{
  niAssert(tVec3fCVec::IsSameType(apResults));
  niCheckSilent(tVec3fCVec::IsSameType(apResults),;);
  ni::UDPointsHemisphereHalton(apResults->_Data(),(tU32)apResults->size(),p2);
}

///////////////////////////////////////////////
void __stdcall cGraphics::UDPointsHemisphereHaltonEx(tVec3fCVec* apResults, tI32 p1, tI32 p2) const
{
  niAssert(tVec3fCVec::IsSameType(apResults));
  niCheckSilent(tVec3fCVec::IsSameType(apResults),;);
  ni::UDPointsHemisphereHaltonEx(apResults->_Data(),(tU32)apResults->size(),p1,p2);
}

///////////////////////////////////////////////
void __stdcall cGraphics::UDPointsPlaneRandom(tVec2fCVec* apResults) const
{
  niAssert(tVec2fCVec::IsSameType(apResults));
  niCheckSilent(tVec2fCVec::IsSameType(apResults),;);
  ni::UDPointsPlaneRandom(apResults->_Data(),(tU32)apResults->size());
}

///////////////////////////////////////////////
void __stdcall cGraphics::UDPointsPlaneHammersley(tVec2fCVec* apResults) const
{
  niAssert(tVec2fCVec::IsSameType(apResults));
  niCheckSilent(tVec2fCVec::IsSameType(apResults),;);
  ni::UDPointsPlaneHammersley(apResults->_Data(),(tU32)apResults->size());
}

///////////////////////////////////////////////
void __stdcall cGraphics::UDPointsPlaneHammersleyEx(tVec2fCVec* apResults, tI32 p1) const
{
  niAssert(tVec2fCVec::IsSameType(apResults));
  niCheckSilent(tVec2fCVec::IsSameType(apResults),;);
  ni::UDPointsPlaneHammersleyEx(apResults->_Data(),(tU32)apResults->size(),p1);
}

///////////////////////////////////////////////
void __stdcall cGraphics::UDPointsPlaneHalton(tVec2fCVec* apResults, tI32 p2) const
{
  niAssert(tVec2fCVec::IsSameType(apResults));
  niCheckSilent(tVec2fCVec::IsSameType(apResults),;);
  ni::UDPointsPlaneHalton(apResults->_Data(),(tU32)apResults->size(),p2);
}

///////////////////////////////////////////////
void __stdcall cGraphics::UDPointsPlaneHaltonEx(tVec2fCVec* apResults, tI32 p1, tI32 p2) const
{
  niAssert(tVec2fCVec::IsSameType(apResults));
  niCheckSilent(tVec2fCVec::IsSameType(apResults),;);
  ni::UDPointsPlaneHaltonEx(apResults->_Data(),(tU32)apResults->size(),p1,p2);
}
