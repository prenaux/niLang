#ifndef __MATHCOLORS_22202854_H__
#define __MATHCOLORS_22202854_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Colors.h"

namespace ni {
/** \addtogroup niUI
 * @{
 */
/** \addtogroup niUI_Utils
 * @{
 */

//--------------------------------------------------------------------------------------------
//
//  General
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
template <typename T>
sVec3<T>& ColorScaleToUnit(sVec3<T>& aOut, const sVec3<T>& aCol) {
  T maxC = ni::Max(aCol.x,aCol.y,aCol.z);
  if (maxC > 1.0) {
    aOut.x = aCol.x/maxC;
    aOut.y = aCol.y/maxC;
    aOut.z = aCol.z/maxC;
  }
  else {
    aOut.x = aCol.x;
    aOut.y = aCol.y;
    aOut.z = aCol.z;
  }
  return VecMaximize(aOut,aOut,sVec3<T>::Zero());
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& ColorAdjustContrast(sVec3<T>& Out,
                                 const sVec3<T>& C,
                                 T c)
{
  Out.x = 0.5f + c * (C.x - 0.5f);
  Out.y = 0.5f + c * (C.y - 0.5f);
  Out.z = 0.5f + c * (C.z - 0.5f);
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& ColorAdjustSaturation(sVec3<T>& Out,
                                   const sVec3<T>& C,
                                   T s)
{
  // Approximate values for each component's contribution to luminance.
  // Based upon the NTSC standard described in ITU-R Recommendation BT.709.
  T grey = C.x * 0.2125f + C.y * 0.7154f + C.z * 0.0721f;
  Out.x = grey + s * (C.x - grey);
  Out.y = grey + s * (C.y - grey);
  Out.z = grey + s * (C.z - grey);
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& ColorGrey(sVec3<T>& Out, const sVec3<T>& C)
{
  T m = (C.x + C.y + C.z) / T(3.0);
  Out.x = m;
  Out.y = m;
  Out.z = m;
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& ColorBlackWhite(sVec3<T>& Out, const sVec3<T>& C, T s)
{
  T add = C.x + C.y + C.z;
  if(add <= s)
  {
    Out.x = 0.0;
    Out.y = 0.0;
    Out.z = 0.0;
  }
  else
  {
    Out.x = 1.0;
    Out.y = 1.0;
    Out.z = 1.0;
  }
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& ColorGammaCorrect(sVec3<T>& Out, tF32 afFactor, const sVec3<T>* pSrc = NULL)
{
  if (!pSrc)
    pSrc = &Out;

  T fInvFac = 1.0f / afFactor;
  if (sizeof(Out.x) == 1)
  {
    tU32 lr = tU32(powf(T(pSrc->x),fInvFac));
    tU32 lg = tU32(powf(T(pSrc->y),fInvFac));
    tU32 lb = tU32(powf(T(pSrc->z),fInvFac));
    Out.x = T((lr > 255) ? 255 : lr);
    Out.y = T((lg > 255) ? 255 : lg);
    Out.z = T((lb > 255) ? 255 : lb);
  }
  else if (sizeof(Out.x) == 2)
  {
    tU32 lr = tU32(powf(T(pSrc->x),fInvFac));
    tU32 lg = tU32(powf(T(pSrc->y),fInvFac));
    tU32 lb = tU32(powf(T(pSrc->z),fInvFac));
    Out.x = T((lr > 65535) ? 65535 : lr);
    Out.y = T((lg > 65535) ? 65535 : lg);
    Out.z = T((lb > 65535) ? 65535 : lb);
  }
  else
  {
    Out.x = (T)powf(T(pSrc->x),fInvFac);
    Out.y = (T)powf(T(pSrc->y),fInvFac);
    Out.z = (T)powf(T(pSrc->z),fInvFac);
  }

  return Out;
}

///////////////////////////////////////////////
template <typename T>
T ColorLuminance(const sVec3<T>& aColor, const sVec3f& avLuminanceDistribution = kvColorLuminanceR21G71B07)
{
  return (aColor.x*avLuminanceDistribution.x+
          aColor.y*avLuminanceDistribution.y+
          aColor.z*avLuminanceDistribution.z);
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& ColorAdjustContrast(sVec4<T>& Out,
                                 const sVec4<T>& C,
                                 T c)
{
  Out.x = 0.5f + c * (C.x - 0.5f);
  Out.y = 0.5f + c * (C.y - 0.5f);
  Out.z = 0.5f + c * (C.z - 0.5f);
  Out.w = 0.5f + c * (C.w - 0.5f);
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& ColorAdjustSaturation(sVec4<T>& Out,
                                   const sVec4<T>& C,
                                   T s)
{
  // Approximate values for each component's contribution to luminance.
  // Based upon the NTSC standard described in ITU-R Recommendation BT.709.
  T grey = C.x * 0.2125f + C.y * 0.7154f + C.z * 0.0721f;
  Out.x = grey + s * (C.x - grey);
  Out.y = grey + s * (C.y - grey);
  Out.z = grey + s * (C.z - grey);
  Out.w = grey + s * (C.w - grey);
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& ColorGrey(sVec4<T>& Out, const sVec4<T>& C)
{
  T m = (C.x + C.y + C.z) / T(3.0);
  Out.x = Out.y = Out.z = m;
  Out.w = C.w;
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& ColorBlackWhite(sVec4<T>& Out, const sVec4<T>& C, T s)
{
  T add = C.x + C.y + C.z;
  if(add <= s)
  {
    Out.x = 0.0;
    Out.y = 0.0;
    Out.z = 0.0;
  }
  else
  {
    Out.x = 1.0;
    Out.y = 1.0;
    Out.z = 1.0;
  }
  Out.w = C.w;
  return Out;
}

///////////////////////////////////////////////
template <typename T>
T ColorLuminance(const sVec4<T>& aColor, const sVec3f& avLuminanceDistribution = kvColorLuminanceR21G71B07)
{
  return ColorLuminance((const sVec3<T>&)aColor);
}


//--------------------------------------------------------------------------------------------
//
//  Color conversion
//
//--------------------------------------------------------------------------------------------

// YUV is generaly 422, or 844.
// y (luminance) = 0.299*red + 0.587*green + 0.114*blue
// u = blue - luminance
// v = red - luminance

///////////////////////////////////////////////////////////////////////////////
//
// The algorithms for these routines were taken from:
//     http://www.neuro.sfc.keio.ac.jp/~aly/polygon/info/color-space-faq.html
//
// RGB --> CMYK                              CMYK --> RGB
// ---------------------------------------   --------------------------------------------
// Black   = minimum(1-Red,1-Green,1-Blue)   Red   = 1-minimum(1,Cyan*(1-Black)+Black)
// Cyan    = (1-Red-Black)/(1-Black)         Green = 1-minimum(1,Magenta*(1-Black)+Black)
// Magenta = (1-Green-Black)/(1-Black)       Blue  = 1-minimum(1,Yellow*(1-Black)+Black)
// Yellow  = (1-Blue-Black)/(1-Black)
//

/*

  XYZ (Tristimulus) Reference values of a perfect reflecting diffuser

  Observer        2 (CIE 1931)        10 (CIE 1964)
  Illuminant      X2    Y2    Z2      X10     Y10   Z10

  A (Incandescent)  109.850 100 35.585      111.144   100   35.200
  C         98.074  100 118.232     97.285    100   116.145
  D50         96.422  100 82.521      96.720    100   81.427
  D55         95.682  100 92.149      95.799    100   90.926
  D65 (Daylight)    95.047  100 108.883     94.811    100   107.304
  D75         94.972  100 122.638     94.416    100   120.641
  F2 (Fluorescent)  99.187  100 67.395      103.280   100   69.026
  F7          95.044  100 108.755     95.792    100   107.687
  F11         100.966 100 64.370      103.866   100   65.627

*/

/*
  XYZ -> RGB
  //Observer = 2, Illuminant = D65
  var_X = X / 100        //Where X = 0 -  95.047
  var_Y = Y / 100        //Where Y = 0 - 100.000
  var_Z = Z / 100        //Where Z = 0 - 108.883

  var_R = var_X *  3.2406 + var_Y * -1.5372 + var_Z * -0.4986
  var_G = var_X * -0.9689 + var_Y *  1.8758 + var_Z *  0.0415
  var_B = var_X *  0.0557 + var_Y * -0.2040 + var_Z *  1.0570

  if ( var_R > 0.0031308 ) var_R = 1.055 * ( var_R ^ ( 1 / 2.4 ) ) - 0.055
  else                     var_R = 12.92 * var_R
  if ( var_G > 0.0031308 ) var_G = 1.055 * ( var_G ^ ( 1 / 2.4 ) ) - 0.055
  else                     var_G = 12.92 * var_G
  if ( var_B > 0.0031308 ) var_B = 1.055 * ( var_B ^ ( 1 / 2.4 ) ) - 0.055
  else                     var_B = 12.92 * var_B

  R = var_R * 255
  G = var_G * 255
  B = var_B * 255
*/

//! Convert an XYZ to a RGB color
template <typename T>
inline sVec4<T> ColorConvert_XYZ_RGB(const sVec4<T>& aXYZ, const sVec4<T>& aXYZRef) {
  T X = aXYZ.x / aXYZRef.x;
  T Y = aXYZ.y / aXYZRef.y;
  T Z = aXYZ.z / aXYZRef.z;
  T R = T(X *  T(3.2406) + Y * -T(1.5372) + Z * -T(0.4986));
  T G = T(X * -T(0.9689) + Y *  T(1.8758) + Z *  T(0.0415));
  T B = T(X *  T(0.0557) + Y * -T(0.2040) + Z *  T(1.0570));

  if ( R > T(0.0031308) ) {
    R = T(1.055) * ni::Pow<T>(R, T(1)/T(2.4)) - T(0.055);
  }
  else {
    R = T(12.92) * R;
  }
  if ( G > T(0.0031308) ) {
    G = T(1.055) * ni::Pow<T>(G, T(1)/T(2.4)) - T(0.055);
  }
  else {
    G = T(12.92) * G;
  }
  if ( B > T(0.0031308) ) {
    B = T(1.055) * ni::Pow<T>(B, T(1)/T(2.4)) - T(0.055);
  }
  else  {
    B = T(12.92) * B;
  }

  return Vec4<T>(R,G,B,T(0));
}

/*
  RGB -> XYZ

  var_R = ( R / 255 )        //Where R = 0 - 255
  var_G = ( G / 255 )        //Where G = 0 - 255
  var_B = ( B / 255 )        //Where B = 0 - 255

  if ( var_R > 0.04045 ) var_R = ( ( var_R + 0.055 ) / 1.055 ) ^ 2.4
  else                   var_R = var_R / 12.92
  if ( var_G > 0.04045 ) var_G = ( ( var_G + 0.055 ) / 1.055 ) ^ 2.4
  else                   var_G = var_G / 12.92
  if ( var_B > 0.04045 ) var_B = ( ( var_B + 0.055 ) / 1.055 ) ^ 2.4
  else                   var_B = var_B / 12.92

  var_R = var_R * 100
  var_G = var_G * 100
  var_B = var_B * 100

  //Observer. = 2, Illuminant = D65
  X = var_R * 0.4124 + var_G * 0.3576 + var_B * 0.1805
  Y = var_R * 0.2126 + var_G * 0.7152 + var_B * 0.0722
  Z = var_R * 0.0193 + var_G * 0.1192 + var_B * 0.9505
*/

//! Convert a RGB color to a XYZ color
template <typename T>
inline sVec4<T> ColorConvert_RGB_XYZ(const sVec4<T>& aRGB, const sVec4<T>& aXYZRef) {
  T R = aRGB.x;
  T G = aRGB.y;
  T B = aRGB.z;

  if ( R > T(0.04045) ) {
    R = ni::Pow(((R + T(0.055)) / T(1.055)), T(2.4));
  }
  else {
    R = R / T(12.92);
  }
  if ( G > T(0.04045) ) {
    G = ni::Pow(((G + T(0.055)) / T(1.055)), T(2.4));
  }
  else {
    G = G / T(12.92);
  }
  if ( B > T(0.04045) ) {
    B = ni::Pow(((B + T(0.055)) / T(1.055)), T(2.4));
  }
  else {
    B = B / T(12.92);
  }

  R *= aXYZRef.x;
  G *= aXYZRef.y;
  B *= aXYZRef.z;

  //Observer. = 2, Illuminant = D65
  T X = T(R * T(0.4124) + G * T(0.3576) + B * T(0.1805));
  T Y = T(R * T(0.2126) + G * T(0.7152) + B * T(0.0722));
  T Z = T(R * T(0.0193) + G * T(0.1192) + B * T(0.9505));
  return Vec4<T>(X,Y,Z,T(0));
}

/*

  XYZ -> Yxy

  //Where X = 0 -  95.047       Observer. = 2, Illuminant = D65
  //Where Y = 0 - 100.000
  //Where Z = 0 - 108.883

  Y = Y
  x = X / ( X + Y + Z )
  y = Y / ( X + Y + Z )

*/

//! Convert a normalize XYZ to Yxy
template <typename T>
inline sVec4<T> ColorConvert_XYZ_Yxy(const sVec4<T>& aXYZ) {
  T X = aXYZ.x;
  T Y = aXYZ.y;
  T Z = aXYZ.z;
  return Vec4<T>(
      Y,
      X / (X + Y + Z),
      Y / (X + Y + Z),
      T(0)
                    );
}

/*

  Yxy -> XYZ

  //Where Y = 0 - 100
  //Where x = 0 - 1
  //Where y = 0 - 1

  X = x * ( Y / y )
  Y = Y
  Z = ( 1 - x - y ) * ( Y / y )

*/

//! Convert a Yxy to XYZ
template <typename T>
inline sVec4<T> ColorConvert_Yxy_XYZ(const sVec4<T>& aYxy) {
  T Y = aYxy.x;
  T x = aYxy.y;
  T y = aYxy.z;
  return Vec4<T>(
      x * (Y / y),
      Y,
      (1 - x - y) * (Y / y),
      T(0)
                    );
}

/*

  XYZ -> Hunter-Lab

  (H)L = 10 * sqrt( Y )
  (H)a = 17.5 * ( ( ( 1.02 * X ) - Y ) / sqrt( Y ) )
  (H)b = 7 * ( ( Y - ( 0.847 * Z ) ) / sqrt( Y ) )

*/
template <typename T>
inline sVec4<T> ColorConvert_XYZ_HunterLab(const sVec4<T>& aXYZ) {
  T X = aXYZ.x;
  T Y = aXYZ.y;
  T Z = aXYZ.z;
  T sqrtY = ni::Sqrt(Y);
  return Vec4<T>(
      T(T(10.0) * sqrtY),           // L
      T(T(17.7) * (((T(1.02)*X)-Y) / sqrtY)), // a
      T(T(10.0) * ((Y-(T(0.847) * Z)) / sqrtY)),  // b
      T(0)
                    );
}

/*

  Hunter-Lab -> XYZ

  var_Y = (H)L / 10
  var_X = (H)a / 17.5 * (H)L / 10
  var_Z = (H)b / 7 * (H)L / 10

  Y = var_Y ^ 2
  X = ( var_X + Y ) / 1.02
  Z = -( var_Z - Y ) / 0.847

*/
template <typename T>
inline sVec4<T> ColorConvert_HunterLab_XYZ(const sVec4<T>& aHLab) {
  T L = aHLab.x;
  T a = aHLab.y;
  T b = aHLab.z;
  T Y = T(L / T(10.0));
  T X = T(a / T(17.5) * L / T(10.0));
  T Z = T(b /  T(7.0) * L / T(10.0));

  Y = ni::Pow<T>(Y,T(2.0));
  return Vec4<T>(
      Y,
      (X + Y) / T(1.02),
      -(Z - Y) / T(0.847),
      T(0)
                    );
}

/*

  XYZ -> CIE-L*ab

  var_X = X / ref_X          //ref_X =  95.047  Observer= 2, Illuminant= D65
  var_Y = Y / ref_Y          //ref_Y = 100.000
  var_Z = Z / ref_Z          //ref_Z = 108.883

  if ( var_X > 0.008856 ) var_X = var_X ^ ( 1/3 )
  else                    var_X = ( 7.787 * var_X ) + ( 16 / 116 )
  if ( var_Y > 0.008856 ) var_Y = var_Y ^ ( 1/3 )
  else                    var_Y = ( 7.787 * var_Y ) + ( 16 / 116 )
  if ( var_Z > 0.008856 ) var_Z = var_Z ^ ( 1/3 )
  else                    var_Z = ( 7.787 * var_Z ) + ( 16 / 116 )

  CIE-L* = ( 116 * var_Y ) - 16
  CIE-a* = 500 * ( var_X - var_Y )
  CIE-b* = 200 * ( var_Y - var_Z )

*/
template <typename T>
inline sVec4<T> ColorConvert_XYZ_CIELab(const sVec4<T>& aXYZ, const sVec4<T>& aXYZRef) {
  T X = aXYZ.x / aXYZRef.x;
  T Y = aXYZ.y / aXYZRef.y;
  T Z = aXYZ.z / aXYZRef.z;

  if ( X > T(0.008856) ) {
    X = T(ni::Pow<T>(X, T(1)/T(3)));
  }
  else {
    X = T(( T(7.787) * X ) + ( T(16) / T(116) ));
  }
  if ( Y > 0.008856 ) {
    Y = T(ni::Pow<T>(Y, T(1)/T(3)));
  }
  else {
    Y = T(( T(7.787) * Y ) + ( T(16) / T(116) ));
  }
  if ( Z > 0.008856 ) {
    Z = T(ni::Pow<T>(Z, T(1)/T(3)));
  }
  else {
    Z = T(( T(7.787) * Z ) + ( T(16) / T(116) ));
  }

  T CIE_L = ( T(116) * Y ) - T(16);
  T CIE_a = T(500) * ( X - Y );
  T CIE_b = T(200) * ( Y - Z );

  return Vec4<T>(CIE_L, CIE_a, CIE_b, T(0));
}

/*

  CIE-L*ab -> XYZ

  var_Y = ( CIE-L* + 16 ) / 116
  var_X = CIE-a* / 500 + var_Y
  var_Z = var_Y - CIE-b* / 200

  if ( var_Y^3 > 0.008856 ) var_Y = var_Y^3
  else                      var_Y = ( var_Y - 16 / 116 ) / 7.787
  if ( var_X^3 > 0.008856 ) var_X = var_X^3
  else                      var_X = ( var_X - 16 / 116 ) / 7.787
  if ( var_Z^3 > 0.008856 ) var_Z = var_Z^3
  else                      var_Z = ( var_Z - 16 / 116 ) / 7.787

  X = ref_X * var_X     //ref_X =  95.047  Observer= 2, Illuminant= D65
  Y = ref_Y * var_Y     //ref_Y = 100.000
  Z = ref_Z * var_Z     //ref_Z = 108.883

*/
template <typename T>
inline sVec4<T> ColorConvert_CIELab_XYZ(const sVec4<T>& aCIELab, const sVec4<T>& aXYZRef) {
  T CIE_L = aCIELab.x;
  T CIE_a = aCIELab.y;
  T CIE_b = aCIELab.z;

  T Y = (CIE_L + T(16)) / T(116);
  T X = CIE_a / T(500) + Y;
  T Z = Y - CIE_b / T(200);

  T pow3;

  pow3 = ni::Pow<T>(Y,T(3));
  if ( pow3 > 0.008856 ) Y = pow3;
  else                  Y = T(( Y - T(16) / T(116) ) / T(7.787));

  pow3 = ni::Pow<T>(X,T(3));
  if ( pow3 > 0.008856 ) X = pow3;
  else                  X = T(( X - T(16) / T(116) ) / T(7.787));

  pow3 = ni::Pow<T>(Z,T(3));
  if ( pow3 > 0.008856 ) Z = pow3;
  else                  Z = T(( Z - T(16) / T(116) ) / T(7.787));

  X = aXYZRef.x * X;     //ref_X =  95.047  Observer= 2, Illuminant= D65
  Y = aXYZRef.y * Y;    //ref_Y = 100.000
  Z = aXYZRef.z * Z;     //ref_Z = 108.883

  return Vec4<T>(X,Y,Z,T(0));
}

/*

  CIE-L*ab -> CIE-L*CH

  var_H = arc_tangent( CIE-b*, CIE-a* )  //Quadrant by signs

  if ( var_H > 0 ) var_H = ( var_H / PI ) * 180
  else             var_H = 360 - ( abs( var_H ) / PI ) * 180

  CIE-L* = CIE-L*
  CIE-C* = sqrt( CIE-a* ^ 2 + CIE-b* ^ 2 )
  CIE-H = var_H

*/
template <typename T>
inline sVec4<T> ColorConvert_CIELab_CIELCH(const sVec4<T>& aCIELab) {
  T CIE_L = aCIELab.x;
  T CIE_a = aCIELab.y;
  T CIE_b = aCIELab.z;

  T H = ni::ATan2<T>(CIE_b, CIE_a); //Quadrant by signs
  if (H > 0)  H = (H / ni::Pi<T>()) * T(180);
  else    H = T(360) - (ni::Abs(H) / ni::Pi<T>()) * T(180);
  T CIELCH_L = CIE_L;
  T CIELCH_C = ni::Sqrt<T>(ni::Pow<T>(CIE_a,2) + ni::Pow<T>(CIE_b,2));
  T CIELCH_H = H;
  return Vec4<T>(CIELCH_L, CIELCH_C, CIELCH_H, T(0));
}

/*

  CIE-L*CH -> CIE-L*ab

  //Where CIE-H = 0 - 360

  CIE-L* = CIE-L*
  CIE-a* = cos( degree_2_radian( CIE-H ) ) * CIE-C*
  CIE-b* = sin( degree_2_radian( CIE-H ) ) * CIE-C*

*/
template <typename T>
inline sVec4<T> ColorConvert_CIELCH_CIELab(const sVec4<T>& aCIELCH) {
  T CIELCH_L = aCIELCH.x;
  T CIELCH_C = aCIELCH.y;
  T CIELCH_H = aCIELCH.z;

  return Vec4<T>(
      CIELCH_L,
      ni::Cos<T>(ni::Rad<T>(CIELCH_H)) * CIELCH_C,
      ni::Sin<T>(ni::Rad<T>(CIELCH_H)) * CIELCH_C,
      T(0)
                    );
}

/*

  XYZ -> CIE-L*uv

  var_U = ( 4 * X ) / ( X + ( 15 * Y ) + ( 3 * Z ) )
  var_V = ( 9 * Y ) / ( X + ( 15 * Y ) + ( 3 * Z ) )

  var_Y = Y / 100
  if ( var_Y > 0.008856 ) var_Y = var_Y ^ ( 1/3 )
  else                    var_Y = ( 7.787 * var_Y ) + ( 16 / 116 )

  ref_X =  95.047        //Observer= 2, Illuminant= D65
  ref_Y = 100.000
  ref_Z = 108.883

  ref_U = ( 4 * ref_X ) / ( ref_X + ( 15 * ref_Y ) + ( 3 * ref_Z ) )
  ref_V = ( 9 * ref_Y ) / ( ref_X + ( 15 * ref_Y ) + ( 3 * ref_Z ) )

  CIE-L* = ( 116 * var_Y ) - 16
  CIE-u* = 13 * CIE-L* * ( var_U - ref_U )
  CIE-v* = 13 * CIE-L* * ( var_V - ref_V )

*/
template <typename T>
inline sVec4<T> ColorConvert_XYZ_CIELuv(const sVec4<T>& aXYZ, const sVec4<T>& aXYZRef) {
  T U = (T(4) * aXYZ.x) / (aXYZ.x + (T(15) * aXYZ.y) + (T(3) * aXYZ.z));
  T V = (T(9) * aXYZ.y) / (aXYZ.x + (T(15) * aXYZ.y) + (T(3) * aXYZ.z));

  T Y = aXYZ.y / T(100);
  if ( Y > 0.008856 ) Y = ni::Pow<T>(Y, ( T(1)/T(3) ));
  else                Y = T(( 7.787 * Y ) + ( T(16) / T(116) ));

  T ref_U = ( T(4) * aXYZRef.x ) / ( aXYZRef.x + ( T(15) * aXYZRef.y ) + ( T(3) * aXYZRef.z ) );
  T ref_V = ( T(9) * aXYZRef.y ) / ( aXYZRef.x + ( T(15) * aXYZRef.y ) + ( T(3) * aXYZRef.z ) );

  T CIE_L = ( T(116) * Y ) - T(16);
  T CIE_u = T(13) * CIE_L * ( U - ref_U );
  T CIE_v = T(13) * CIE_L * ( V - ref_V );

  return Vec4<T>(CIE_L, CIE_u, CIE_v, T(0));
}

/*

  CIE-L*uv -> XYZ

  var_Y = ( CIE-L* + 16 ) / 116
  if ( var_Y^3 > 0.008856 ) var_Y = var_Y^3
  else                      var_Y = ( var_Y - 16 / 116 ) / 7.787

  ref_X =  95.047      //Observer= 2, Illuminant= D65
  ref_Y = 100.000
  ref_Z = 108.883

  ref_U = ( 4 * ref_X ) / ( ref_X + ( 15 * ref_Y ) + ( 3 * ref_Z ) )
  ref_V = ( 9 * ref_Y ) / ( ref_X + ( 15 * ref_Y ) + ( 3 * ref_Z ) )

  var_U = CIE-u* / ( 13 * CIE-L* ) + ref_U
  var_V = CIE-v* / ( 13 * CIE-L* ) + ref_V

  Y = var_Y * 100
  X =  - ( 9 * Y * var_U ) / ( ( var_U - 4 ) * var_V  - var_U * var_V )
  Z = ( 9 * Y - ( 15 * var_V * Y ) - ( var_V * X ) ) / ( 3 * var_V )

*/
template <typename T>
inline sVec4<T> ColorConvert_CIELuv_XYZ(const sVec4<T>& aCIELuv, const sVec4<T>& aXYZRef) {
  T CIE_L = aCIELuv.x;
  T CIE_u = aCIELuv.y;
  T CIE_v = aCIELuv.z;

  T Y = ( CIE_L + T(16) ) / T(116);
  T pow3 = ni::Pow<T>(Y,T(3));
  if ( pow3 > T(0.008856) ) {
    Y = pow3;
  }
  else {
    Y = T(( Y - T(16) / T(116) ) / T(7.787));
  }

  T ref_U = ( T(4) * aXYZRef.x ) / ( aXYZRef.x + ( T(15) * aXYZRef.y ) + ( T(3) * aXYZRef.z ) );
  T ref_V = ( T(9) * aXYZRef.y ) / ( aXYZRef.x + ( T(15) * aXYZRef.y ) + ( T(3) * aXYZRef.z ) );

  T U = CIE_u / ( T(13) * CIE_L ) + ref_U;
  T V = CIE_v / ( T(13) * CIE_L ) + ref_V;

  Y = Y * T(100);
  T X =  - ( T(9) * Y * U ) / ( ( U - T(4) ) * V  - U * V );
  T Z = ( T(9) * Y - ( T(15) * V * Y ) - ( V * X ) ) / ( T(3) * V );

  return Vec4<T>(X,Y,Z,T(0));
}

/*

  RGB -> HSL

  var_R = ( R / 255 )                     //Where RGB values = 0 - 255
  var_G = ( G / 255 )
  var_B = ( B / 255 )

  var_Min = min( var_R, var_G, var_B )    //Min. value of RGB
  var_Max = max( var_R, var_G, var_B )    //Max. value of RGB
  del_Max = var_Max - var_Min             //Delta RGB value

  L = ( var_Max + var_Min ) / 2

  if ( del_Max == 0 )                     //This is a gray, no chroma...
  {
  H = 0                                //HSL results = 0 - 1
  S = 0
  }
  else                                    //Chromatic data...
  {
  if ( L < 0.5 ) S = del_Max / ( var_Max + var_Min )
  else           S = del_Max / ( 2 - var_Max - var_Min )

  del_R = ( ( ( var_Max - var_R ) / 6 ) + ( del_Max / 2 ) ) / del_Max
  del_G = ( ( ( var_Max - var_G ) / 6 ) + ( del_Max / 2 ) ) / del_Max
  del_B = ( ( ( var_Max - var_B ) / 6 ) + ( del_Max / 2 ) ) / del_Max

  if      ( var_R == var_Max ) H = del_B - del_G
  else if ( var_G == var_Max ) H = ( 1 / 3 ) + del_R - del_B
  else if ( var_B == var_Max ) H = ( 2 / 3 ) + del_G - del_R

  if ( H < 0 ) ; H += 1
  if ( H > 1 ) ; H -= 1
  }
*/
template <typename T>
inline sVec4<T> ColorConvert_RGB_HSL(const sVec4<T>& aRGB) {
  T R = aRGB.x;
  T G = aRGB.y;
  T B = aRGB.z;

  T Min = ni::Min( R, G, B );    //Min. value of RGB
  T Max = ni::Max( R, G, B );    //Max. value of RGB
  T del_Max = Max - Min;          //Delta RGB value

  T H = 0, S = 0, L = 0;

  L = ( Max + Min ) / T(2);

  if ( del_Max == 0 )                     //This is a gray, no chroma...
  {
    H = 0;                                //HSL results = 0 - 1
    S = 0;
  }
  else                                    //Chromatic data...
  {
    if ( L < T(0.5) )
      S = del_Max / ( Max + Min );
    else
      S = del_Max / ( T(2) - Max - Min );

    T del_R = ( ( ( Max - R ) / T(6) ) + ( del_Max / T(2) ) ) / del_Max;
    T del_G = ( ( ( Max - G ) / T(6) ) + ( del_Max / T(2) ) ) / del_Max;
    T del_B = ( ( ( Max - B ) / T(6) ) + ( del_Max / T(2) ) ) / del_Max;

    if      ( R == Max ) H = del_B - del_G;
    else if ( G == Max ) H = ( T(1) / T(3) ) + del_R - del_B;
    else if ( B == Max ) H = ( T(2) / T(3) ) + del_G - del_R;

    if ( H < 0 ) H += T(1);
    if ( H > 1 ) H -= T(1);
  }

  return Vec4<T>(H,S,L,T(0));
}

/*

  HSL -> RGB

  if ( S == 0 )                       //HSL values = 0 - 1
  {
  R = L * 255                      //RGB results = 0 - 255
  G = L * 255
  B = L * 255
  }
  else
  {
  if ( L < 0.5 ) var_2 = L * ( 1 + S )
  else           var_2 = ( L + S ) - ( S * L )

  var_1 = 2 * L - var_2

  R = 255 * Hue_2_RGB( var_1, var_2, H + ( 1 / 3 ) )
  G = 255 * Hue_2_RGB( var_1, var_2, H )
  B = 255 * Hue_2_RGB( var_1, var_2, H - ( 1 / 3 ) )
  }

  Hue_2_RGB( v1, v2, vH )             //Function Hue_2_RGB
  {
  if ( vH < 0 ) vH += 1
  if ( vH > 1 ) vH -= 1
  if ( ( 6 * vH ) < 1 ) return ( v1 + ( v2 - v1 ) * 6 * vH )
  if ( ( 2 * vH ) < 1 ) return ( v2 )
  if ( ( 3 * vH ) < 2 ) return ( v1 + ( v2 - v1 ) * ( ( 2 / 3 ) - vH ) * 6 )
  return ( v1 )
  }

*/
template <typename T>
inline T Hue_2_RGB(T v1, T v2, T vH ) {
  if ( vH < T(0) ) vH += T(1);
  if ( vH > T(1) ) vH -= T(1);
  if ( ( T(6) * vH ) < T(1) )
    return ( v1 + ( v2 - v1 ) * T(6) * vH );
  if ( ( T(2) * vH ) < T(1) )
    return ( v2 );
  if ( ( T(3) * vH ) < T(2) )
    return ( v1 + ( v2 - v1 ) * ( ( T(2) / T(3) ) - vH ) * T(6) );
  return ( v1 );
}

template <typename T>
inline sVec4<T> ColorConvert_HSL_RGB(const sVec4<T>& aHSL) {
  T R,G,B;
  T H = aHSL.x;
  T S = aHSL.y;
  T L = aHSL.z;
  if ( S == 0 )                       //HSL values = 0 - 1
  {
    R = L;                      //RGB results = 0 - 255
    G = L;
    B = L;
  }
  else
  {
    T var_1, var_2;

    if ( L < T(0.5) )
      var_2 = L * ( T(1) + S );
    else
      var_2 = ( L + S ) - ( S * L );

    var_1 = T(2) * L - var_2;

    R = Hue_2_RGB<T>( var_1, var_2, H + ( T(1) / T(3) ) );
    G = Hue_2_RGB<T>( var_1, var_2, H );
    B = Hue_2_RGB<T>( var_1, var_2, H - ( T(1) / T(3) ) );
  }

  return Vec4<T>(R,G,B,T(0));
}

/*

  RGB ->  HSV

  var_R = ( R / 255 )                     //RGB values = 0 - 255
  var_G = ( G / 255 )
  var_B = ( B / 255 )

  var_Min = min( var_R, var_G, var_B )    //Min. value of RGB
  var_Max = max( var_R, var_G, var_B )    //Max. value of RGB
  del_Max = var_Max - var_Min             //Delta RGB value

  V = var_Max

  if ( del_Max == 0 )                     //This is a gray, no chroma...
  {
  H = 0                                //HSV results = 0 - 1
  S = 0
  }
  else                                    //Chromatic data...
  {
  S = del_Max / var_Max

  del_R = ( ( ( var_Max - var_R ) / 6 ) + ( del_Max / 2 ) ) / del_Max
  del_G = ( ( ( var_Max - var_G ) / 6 ) + ( del_Max / 2 ) ) / del_Max
  del_B = ( ( ( var_Max - var_B ) / 6 ) + ( del_Max / 2 ) ) / del_Max

  if      ( var_R == var_Max ) H = del_B - del_G
  else if ( var_G == var_Max ) H = ( 1 / 3 ) + del_R - del_B
  else if ( var_B == var_Max ) H = ( 2 / 3 ) + del_G - del_R

  if ( H < 0 ) ; H += 1
  if ( H > 1 ) ; H -= 1
  }

*/
template <typename T>
inline sVec4<T> ColorConvert_RGB_HSV(const sVec4<T>& aRGB) {
  T R = aRGB.x;
  T G = aRGB.y;
  T B = aRGB.z;

  T Min = ni::Min( R, G, B );    //Min. value of RGB
  T Max = ni::Max( R, G, B );    //Max. value of RGB
  T del_Max = Max - Min;             //Delta RGB value

  T H = 0, S = 0, V = 0;

  V = Max;

  if (del_Max == T(0) )                     //This is a gray, no chroma...
  {
    H = 0;                                //HSV results = 0 - 1
    S = 0;
  }
  else                                    //Chromatic data...
  {
    S = del_Max / Max;

    T del_R = ( ( ( Max - R ) / T(6) ) + ( del_Max / T(2) ) ) / del_Max;
    T del_G = ( ( ( Max - G ) / T(6) ) + ( del_Max / T(2) ) ) / del_Max;
    T del_B = ( ( ( Max - B ) / T(6) ) + ( del_Max / T(2) ) ) / del_Max;

    if      ( R == Max ) H = del_B - del_G;
    else if ( G == Max ) H = ( T(1) / T(3) ) + del_R - del_B;
    else if ( B == Max ) H = ( T(2) / T(3) ) + del_G - del_R;

    if ( H < T(0) ) H += T(1);
    if ( H > T(1) ) H -= T(1);
  }

  return Vec4<T>(H,S,V,T(0));
}

/*

  HSV -> RGB

  if ( S == 0 )                       //HSV values = 0 - 1
  {
  R = V * 255
  G = V * 255
  B = V * 255
  }
  else
  {
  var_h = H * 6
  if ( var_h == 6 ) var_h = 0      //H must be < 1
  var_i = int( var_h )             //Or ... var_i = floor( var_h )
  var_1 = V * ( 1 - S )
  var_2 = V * ( 1 - S * ( var_h - var_i ) )
  var_3 = V * ( 1 - S * ( 1 - ( var_h - var_i ) ) )

  if      ( var_i == 0 ) { var_r = V     ; var_g = var_3 ; var_b = var_1 }
  else if ( var_i == 1 ) { var_r = var_2 ; var_g = V     ; var_b = var_1 }
  else if ( var_i == 2 ) { var_r = var_1 ; var_g = V     ; var_b = var_3 }
  else if ( var_i == 3 ) { var_r = var_1 ; var_g = var_2 ; var_b = V     }
  else if ( var_i == 4 ) { var_r = var_3 ; var_g = var_1 ; var_b = V     }
  else                   { var_r = V     ; var_g = var_1 ; var_b = var_2 }

  R = var_r * 255                  //RGB results = 0 - 255
  G = var_g * 255
  B = var_b * 255
  }
  }

*/
template <typename T>
inline sVec4<T> ColorConvert_HSV_RGB(const sVec4<T>& aHSV) {
  T H = aHSV.x;
  T S = aHSV.y;
  T V = aHSV.z;
  T R, G, B;

  /*if ( S == 0 )                       //HSV values = 0 - 1
    {
    R = V;
    G = V;
    B = V;
    }
    else*/
  {
    T var_h = H * T(6);
    if ( var_h == T(6) )
      var_h = T(0);      //H must be < 1
    T var_i = ni::Floor<T>( var_h );             //Or ... var_i = floor( var_h )
    T var_1 = V * ( T(1) - S );
    T var_2 = V * ( T(1) - S * ( var_h - var_i ) );
    T var_3 = V * ( T(1) - S * ( T(1) - ( var_h - var_i ) ) );

    T var_r, var_g, var_b;
    if      ( var_i == 0 ) { var_r = V     ; var_g = var_3 ; var_b = var_1; }
    else if ( var_i == 1 ) { var_r = var_2 ; var_g = V     ; var_b = var_1; }
    else if ( var_i == 2 ) { var_r = var_1 ; var_g = V     ; var_b = var_3; }
    else if ( var_i == 3 ) { var_r = var_1 ; var_g = var_2 ; var_b = V;     }
    else if ( var_i == 4 ) { var_r = var_3 ; var_g = var_1 ; var_b = V;     }
    else                   { var_r = V     ; var_g = var_1 ; var_b = var_2; }

    R = var_r;
    G = var_g;
    B = var_b;
  }

  return Vec4<T>(R,G,B,T(0));
}

/*

  Range of HSL, HSB and HSV in popular applications

  Applications      Space   H Range     S Range       L/V/B Range

  Paint Shop Pro      HSL     0 - 255     0 - 255     L   0 - 255
  Gimp          HSV     0 - 360   0 - 100     V   0 - 100
  Photoshop       HSV     0 - 360   0 - 100%    B   0 - 100%
  Windows         HSL     0 - 240     0 - 240     L   0 - 240
  Linux / KDE       HSV     0 - 360   0 - 255     V   0 - 255
  GTK           HSV     0 - 360   0 - 1.0     V   0 - 1.0
  Java (awt.Color)    HSV     0 - 1.0     0 - 1.0     B   0 - 1.0
  Apple         HSV     0 - 360   0 - 100%    L   0 - 100%

*/

/*

  RGB -> CMY

  //RGB values = 0 - 255
  //CMY values = 0 - 1

  C = 1 - ( R / 255 )
  M = 1 - ( G / 255 )
  Y = 1 - ( B / 255 )

*/
template <typename T>
inline sVec4<T> ColorConvert_RGB_CMY(const sVec4<T>& aRGB) {
  return Vec4<T>(
      T(1) - aRGB.x,
      T(1) - aRGB.y,
      T(1) - aRGB.z,
      T(0));
}

/*

  CMY -> RGB

  //CMY values = 0 - 1
  //RGB values = 0 - 255

  R = ( 1 - C ) * 255
  G = ( 1 - M ) * 255
  B = ( 1 - Y ) * 255

*/
template <typename T>
inline sVec4<T> ColorConvert_CMY_RGB(const sVec4<T>& aCMY) {
  return Vec4<T>(
      T(1) - aCMY.x,
      T(1) - aCMY.y,
      T(1) - aCMY.z,
      T(0));
}

/*

  CMY -> CMYK

  //Where CMYK and CMY values = 0 - 1

  var_K = 1

  if ( C < var_K )   var_K = C
  if ( M < var_K )   var_K = M
  if ( Y < var_K )   var_K = Y
  if ( var_K == 1 ) { //Black
  C = 0
  M = 0
  Y = 0
  }
  else {
  C = ( C - var_K ) / ( 1 - var_K )
  M = ( M - var_K ) / ( 1 - var_K )
  Y = ( Y - var_K ) / ( 1 - var_K )
  }
  K = var_K

*/
template <typename T>
inline sVec4<T> ColorConvert_CMY_CMYK(const sVec4<T>& aCMY) {
  T C = aCMY.x;
  T M = aCMY.y;
  T Y = aCMY.z;
  T K = T(1);

  if ( C < K )
    K = C;
  if ( M < K )
    K = M;
  if ( Y < K )
    K = Y;

  if ( K == 1 ) { //Black
    C = 0;
    M = 0;
    Y = 0;
  }
  else {
    C = ( C - K ) / ( T(1) - K );
    M = ( M - K ) / ( T(1) - K );
    Y = ( Y - K ) / ( T(1) - K );
  }
  return Vec4<T>(C,M,Y,K);
}

/*

  CMYK -> CMY

  //Where CMYK and CMY values = 0 - 1
  C = ( C * ( 1 - K ) + K )
  M = ( M * ( 1 - K ) + K )
  Y = ( Y * ( 1 - K ) + K )

*/
template <typename T>
inline sVec4<T> ColorConvert_CMYK_CMY(const sVec4<T>& aCMYK) {
  return Vec4<T>(
      aCMYK.x * ( T(1) - aCMYK.w ) + aCMYK.w,
      aCMYK.y * ( T(1) - aCMYK.w ) + aCMYK.w,
      aCMYK.z * ( T(1) - aCMYK.w ) + aCMYK.w,
      T(0)
                    );
}

///////////////////////////////////////////////
template <typename T>
inline sVec4<T> ColorConvert_RGB_YIQ(const sVec4<T>& aRGB) {
  T Red = aRGB.x;
  T Green = aRGB.y;
  T Blue = aRGB.z;
  return Vec4<T>(
      T(T(0.299)*Red+T(0.587)*Green+T(0.114)*Blue),
      T(T(0.596)*Red-T(0.274)*Green+T(0.322)*Blue),
      T(T(0.212)*Red-T(0.523)*Green-T(0.311)*Blue),
      T(0)
                    );
}

///////////////////////////////////////////////
template <typename T>
inline sVec4<T> ColorConvert_YIQ_RGB(const sVec4<T>& aYIQ) {
  T Y = aYIQ.x;
  T I = aYIQ.y;
  T Q = aYIQ.z;
  return Vec4<T>(
      T(Y+T(0.956)*I+T(0.621)*Q),
      T(Y-T(0.272)*I-T(0.647)*Q),
      T(Y-T(1.105)*I+T(1.702)*Q),
      T(0)
                    );
}

///////////////////////////////////////////////
template <typename T>
inline sVec4<T> ColorConvert_RGB_YUV(const sVec4<T>& aRGB) {
  T Red = aRGB.x;
  T Green = aRGB.y;
  T Blue = aRGB.z;
  return Vec4<T>(
      T(T(0.299)*Red+T(0.587)*Green+T(0.114)*Blue),
      T(-T(0.147)*Red-T(0.289)*Green+T(0.436)*Blue),
      T(T(0.615)*Red-T(0.515)*Green-T(0.100)*Blue),
      T(0));
}

///////////////////////////////////////////////
template <typename T>
inline sVec4<T> ColorConvert_YUV_RGB(const sVec4<T>& aYUV) {
  T Y = aYUV.x;
  T U = aYUV.y;
  T V = aYUV.z;
  return Vec4<T>(
      T(Y+T(0.000)*U+T(1.140)*V),
      T(Y-T(0.396)*U-T(0.581)*V),
      T(Y+T(2.029)*U+T(0.000)*V),
      T(0));
}



///////////////////////////////////////////////
template <typename T>
inline sVec4<T> ColorConvert_YIQ_YUV(const sVec4<T>& aYIQ) {
  T Y = aYIQ.x;
  T I = aYIQ.y;
  T Q = aYIQ.z;
  return Vec4<T>(
      Y,
      T(-T(1.1270)*I+T(1.8050)*Q),
      T(T(0.9489)*I+T(0.6561)*Q),
      T(0));
}

///////////////////////////////////////////////
template <typename T>
inline sVec4<T> ColorConvert_YUV_YIQ(const sVec4<T>& aYUV) {
  T Y = aYUV.x;
  T U = aYUV.y;
  T V = aYUV.z;
  return Vec4<T>(
      Y,
      T(-T(0.2676)*U+T(0.7361)*V),
      T(T(0.3869)*U+T(0.4596)*V),
      T(0));
}

///////////////////////////////////////////////
template <typename T>
inline sVec4<T> ColorConvert_RGB_YCbCr(const sVec4<T>& aRGB) {
  T Red = aRGB.x;
  T Green = aRGB.y;
  T Blue = aRGB.z;
  return Vec4<T>(
      T(T(0.2989)*Red+T(0.5866)*Green+T(0.1145)*Blue),
      T(-T(0.1687)*Red-T(0.3312)*Green+T(0.5000)*Blue),
      T(T(0.5000)*Red-T(0.4183)*Green-T(0.0816)*Blue),
      T(0));
}

///////////////////////////////////////////////
template <typename T>
inline sVec4<T> ColorConvert_YCbCr_RGB(const sVec4<T>& aYCbCr) {
  T Y = aYCbCr.x;
  T Cb = aYCbCr.y;
  T Cr = aYCbCr.z;
  return Vec4<T>(
      T(Y+T(0.0000)*Cb+T(1.4022)*Cr),
      T(Y-T(0.3456)*Cb-T(0.7145)*Cr),
      T(Y+T(1.7710)*Cb+T(0.0000)*Cr),
      T(0));
}

///////////////////////////////////////////////
template <typename T>
inline sVec4<T> ColorConvert_RGB_YPbPr(const sVec4<T>& aRGB) {
  T Red = aRGB.x;
  T Green = aRGB.y;
  T Blue = aRGB.z;

  // Gamma correction
  Red = ni::Pow<T>(ni::Abs<T>(Red),T(0.45));
  Green = ni::Pow<T>(ni::Abs<T>(Green),T(0.45));
  Blue = ni::Pow<T>(ni::Abs<T>(Blue),T(0.45));

  // Linear transformations:
  T Y  = T( T(0.2122)*Red+T(0.7013)*Green+T(0.0865)*Blue);
  T Pb = T( -T(0.1162)*Red-T(0.3838)*Green+T(0.5000)*Blue);
  T Pr = T( T(0.5000)*Red-T(0.4451)*Green-T(0.0549)*Blue);

  return Vec4<T>(Y,Pb,Pr,T(0));
}

///////////////////////////////////////////////
template <typename T>
inline sVec4<T> ColorConvert_YPbPr_RGB(const sVec4<T>& aYPbPr) {
  T Y = aYPbPr.x;
  T Pb = aYPbPr.y;
  T Pr = aYPbPr.z;

  T Red = T(1*Y+T(0.0000)*Pb+T(1.5756)*Pr);
  T Green = T(1*Y-T(0.2253)*Pb+T(0.5000)*Pr);
  T Blue  = T(1*Y+T(1.8270)*Pb+T(0.0000)*Pr);

  // Gamma correction
  Red = ni::Pow<T>(ni::Abs<T>(Red),T(T(1)/T(0.45)));
  Green = ni::Pow<T>(ni::Abs<T>(Green),T(T(1)/T(0.45)));
  Blue = ni::Pow<T>(ni::Abs<T>(Blue),T(T(1)/T(0.45)));
  return Vec4<T>(Red,Green,Blue,T(0));
}

///////////////////////////////////////////////
template <typename T>
inline sVec4<T> ColorConvert_RGB_YCoCg(const sVec4<T>& aRGB) {
  const T Red = aRGB.x;
  const T Green = aRGB.y;
  const T Blue = aRGB.z;

  // Linear transformations:
  T Y  = T( Red/T(4)+Green/T(2)+Blue/T(4));
  T Co = T( Red/T(2)-Blue/T(2));
  T Cg = T( -Red/T(4)+Green/T(2)-Blue/T(4));

  return Vec4<T>(Y,Co,Cg,T(0));
}

///////////////////////////////////////////////
template <typename T>
inline sVec4<T> ColorConvert_YCoCg_RGB(const sVec4<T>& aYCoCg) {
  const T Y = aYCoCg.x;
  const T Co = aYCoCg.y;
  const T Cg = aYCoCg.z;

  const T t = Y - Cg;
  T Red = t + Co;
  T Green = Y + Cg;
  T Blue  = t - Co;

  return Vec4<T>(Red,Green,Blue,T(0));
}

///////////////////////////////////////////////
template <typename T>
sVec4<T> __stdcall ColorConvert(eColorSpace aSource, eColorSpace aDest, const sVec4<T>& aColor, const sVec4<T>& aXYZRef) {
  if (aSource == aDest)
    return aColor;

  sVec4<T> xyzColor = sVec4<T>::Zero();

  switch (aSource) {
    case eColorSpace_RGB:
      {
        switch (aDest) {
          case eColorSpace_CMY:
            return ColorConvert_RGB_CMY(aColor);
          case eColorSpace_CMYK:
            return ColorConvert_CMY_CMYK(ColorConvert_RGB_CMY(aColor));
          case eColorSpace_HSL:
            return ColorConvert_RGB_HSL(aColor);
          case eColorSpace_HSV:
            return ColorConvert_RGB_HSV(aColor);
          case eColorSpace_YIQ:
            return ColorConvert_RGB_YIQ(aColor);
          case eColorSpace_YUV:
            return ColorConvert_RGB_YUV(aColor);
          case eColorSpace_YCbCr:
            return ColorConvert_RGB_YCbCr(aColor);
          case eColorSpace_YPbPr:
            return ColorConvert_RGB_YPbPr(aColor);
          case eColorSpace_YCoCg:
            return ColorConvert_RGB_YCoCg(aColor);
          default:
            xyzColor = ColorConvert_RGB_XYZ(aColor,aXYZRef);
            break;
        }
      }
    case eColorSpace_CMY:
      {
        switch (aDest) {
          case eColorSpace_RGB:
            return ColorConvert_CMY_RGB(aColor);
          case eColorSpace_CMYK:
            return ColorConvert_CMY_CMYK(aColor);
          case eColorSpace_HSL:
            return ColorConvert_RGB_HSL(ColorConvert_CMY_RGB(aColor));
          case eColorSpace_HSV:
            return ColorConvert_RGB_HSV(ColorConvert_CMY_RGB(aColor));
          case eColorSpace_YIQ:
            return ColorConvert_RGB_YIQ(ColorConvert_CMY_RGB(aColor));
          case eColorSpace_YUV:
            return ColorConvert_RGB_YUV(ColorConvert_CMY_RGB(aColor));
          case eColorSpace_YCbCr:
            return ColorConvert_RGB_YCbCr(ColorConvert_CMY_RGB(aColor));
          case eColorSpace_YPbPr:
            return ColorConvert_RGB_YPbPr(ColorConvert_CMY_RGB(aColor));
          case eColorSpace_YCoCg:
            return ColorConvert_RGB_YCoCg(ColorConvert_CMY_RGB(aColor));
          default:
            xyzColor = ColorConvert_RGB_XYZ(ColorConvert_CMY_RGB(aColor),aXYZRef);
            break;
        }
        break;
      }
    case eColorSpace_CMYK:
      {
        switch (aDest) {
          case eColorSpace_RGB:
            return ColorConvert_CMY_RGB(ColorConvert_CMYK_CMY(aColor));
          case eColorSpace_CMY:
            return ColorConvert_CMYK_CMY(aColor);
          case eColorSpace_HSL:
            return ColorConvert_RGB_HSL(ColorConvert_CMY_RGB(ColorConvert_CMYK_CMY(aColor)));
          case eColorSpace_HSV:
            return ColorConvert_RGB_HSV(ColorConvert_CMY_RGB(ColorConvert_CMYK_CMY(aColor)));
          case eColorSpace_YIQ:
            return ColorConvert_RGB_YIQ(ColorConvert_CMY_RGB(ColorConvert_CMYK_CMY(aColor)));
          case eColorSpace_YUV:
            return ColorConvert_RGB_YUV(ColorConvert_CMY_RGB(ColorConvert_CMYK_CMY(aColor)));
          case eColorSpace_YCbCr:
            return ColorConvert_RGB_YCbCr(ColorConvert_CMY_RGB(ColorConvert_CMYK_CMY(aColor)));
          case eColorSpace_YPbPr:
            return ColorConvert_RGB_YPbPr(ColorConvert_CMY_RGB(ColorConvert_CMYK_CMY(aColor)));
          case eColorSpace_YCoCg:
            return ColorConvert_RGB_YCoCg(ColorConvert_CMY_RGB(ColorConvert_CMYK_CMY(aColor)));
          default:
            xyzColor = ColorConvert_RGB_XYZ(ColorConvert_CMY_RGB(ColorConvert_CMYK_CMY(aColor)),aXYZRef);
            break;
        }
        break;
      }
    case eColorSpace_HSV:
      {
        switch (aDest) {
          case eColorSpace_RGB:
            return ColorConvert_HSV_RGB(aColor);
          case eColorSpace_CMY:
            return ColorConvert_RGB_CMY(ColorConvert_HSV_RGB(aColor));
          case eColorSpace_CMYK:
            return ColorConvert_CMY_CMYK(ColorConvert_RGB_CMY(ColorConvert_HSV_RGB(aColor)));
          case eColorSpace_HSL:
            return ColorConvert_RGB_HSL(ColorConvert_HSV_RGB(aColor));
          case eColorSpace_YIQ:
            return ColorConvert_RGB_YIQ(ColorConvert_HSV_RGB(aColor));
          case eColorSpace_YUV:
            return ColorConvert_RGB_YUV(ColorConvert_HSV_RGB(aColor));
          case eColorSpace_YCbCr:
            return ColorConvert_RGB_YCbCr(ColorConvert_HSV_RGB(aColor));
          case eColorSpace_YPbPr:
            return ColorConvert_RGB_YPbPr(ColorConvert_HSV_RGB(aColor));
          case eColorSpace_YCoCg:
            return ColorConvert_RGB_YCoCg(ColorConvert_HSV_RGB(aColor));
          default:
            xyzColor = ColorConvert_RGB_XYZ(ColorConvert_HSV_RGB(aColor),aXYZRef);
            break;
        }
        break;
      }
    case eColorSpace_HSL:
      {
        switch (aDest) {
          case eColorSpace_RGB:
            return ColorConvert_HSL_RGB(aColor);
          case eColorSpace_CMY:
            return ColorConvert_RGB_CMY(ColorConvert_HSL_RGB(aColor));
          case eColorSpace_CMYK:
            return ColorConvert_CMY_CMYK(ColorConvert_RGB_CMY(ColorConvert_HSL_RGB(aColor)));
          case eColorSpace_HSV:
            return ColorConvert_RGB_HSV(ColorConvert_HSL_RGB(aColor));
          case eColorSpace_YIQ:
            return ColorConvert_RGB_YIQ(ColorConvert_HSL_RGB(aColor));
          case eColorSpace_YUV:
            return ColorConvert_RGB_YUV(ColorConvert_HSL_RGB(aColor));
          case eColorSpace_YCbCr:
            return ColorConvert_RGB_YCbCr(ColorConvert_HSL_RGB(aColor));
          case eColorSpace_YPbPr:
            return ColorConvert_RGB_YPbPr(ColorConvert_HSL_RGB(aColor));
          case eColorSpace_YCoCg:
            return ColorConvert_RGB_YCoCg(ColorConvert_HSL_RGB(aColor));
          default:
            xyzColor = ColorConvert_RGB_XYZ(ColorConvert_HSL_RGB(aColor),aXYZRef);
            break;
        }
        break;
      }
    case eColorSpace_XYZ:
      {
        // in XYZ space already
        break;
      }
    case eColorSpace_Yxy:
      {
        xyzColor = ColorConvert_Yxy_XYZ(aColor);
        break;
      }
    case eColorSpace_HunterLab:
      {
        xyzColor = ColorConvert_HunterLab_XYZ(aColor);
        break;
      }
    case eColorSpace_CIELab:
      {
        if (aDest == eColorSpace_CIELCH) {
          return ColorConvert_CIELab_CIELCH(aColor);
        }
        else {
          xyzColor = ColorConvert_CIELab_XYZ(aColor,aXYZRef);
        }
        break;
      }
    case eColorSpace_CIELCH:
      {
        if (aDest == eColorSpace_CIELab) {
          return ColorConvert_CIELCH_CIELab(aColor);
        }
        else {
          xyzColor = ColorConvert_CIELab_XYZ(ColorConvert_CIELCH_CIELab(aColor),aXYZRef);
        }
        break;
      }
    case eColorSpace_CIELuv:
      {
        xyzColor = ColorConvert_CIELuv_XYZ(aColor,aXYZRef);
        break;
      }
    case eColorSpace_YIQ:
      {
        switch (aDest) {
          case eColorSpace_RGB:
            return ColorConvert_YIQ_RGB(aColor);
          case eColorSpace_CMY:
            return ColorConvert_RGB_CMY(ColorConvert_YIQ_RGB(aColor));
          case eColorSpace_CMYK:
            return ColorConvert_CMY_CMYK(ColorConvert_RGB_CMY(ColorConvert_YIQ_RGB(aColor)));
          case eColorSpace_HSV:
            return ColorConvert_RGB_HSV(ColorConvert_YIQ_RGB(aColor));
          case eColorSpace_HSL:
            return ColorConvert_RGB_HSL(ColorConvert_YIQ_RGB(aColor));
          case eColorSpace_YUV:
            return ColorConvert_YIQ_YUV(aColor);
          case eColorSpace_YCbCr:
            return ColorConvert_RGB_YCbCr(ColorConvert_YIQ_RGB(aColor));
          case eColorSpace_YPbPr:
            return ColorConvert_RGB_YPbPr(ColorConvert_YIQ_RGB(aColor));
          case eColorSpace_YCoCg:
            return ColorConvert_RGB_YCoCg(ColorConvert_YIQ_RGB(aColor));
          default:
            xyzColor = ColorConvert_RGB_XYZ(ColorConvert_YIQ_RGB(aColor),aXYZRef);
            break;
        }
        break;
      }
    case eColorSpace_YUV:
      {
        switch (aDest) {
          case eColorSpace_RGB:
            return ColorConvert_YUV_RGB(aColor);
          case eColorSpace_CMY:
            return ColorConvert_RGB_CMY(ColorConvert_YUV_RGB(aColor));
          case eColorSpace_CMYK:
            return ColorConvert_CMY_CMYK(ColorConvert_RGB_CMY(ColorConvert_YUV_RGB(aColor)));
          case eColorSpace_HSV:
            return ColorConvert_RGB_HSV(ColorConvert_YUV_RGB(aColor));
          case eColorSpace_HSL:
            return ColorConvert_RGB_HSL(ColorConvert_YUV_RGB(aColor));
          case eColorSpace_YIQ:
            return ColorConvert_YUV_YIQ(aColor);
          case eColorSpace_YCbCr:
            return ColorConvert_RGB_YCbCr(ColorConvert_YUV_RGB(aColor));
          case eColorSpace_YPbPr:
            return ColorConvert_RGB_YPbPr(ColorConvert_YUV_RGB(aColor));
          case eColorSpace_YCoCg:
            return ColorConvert_RGB_YCoCg(ColorConvert_YUV_RGB(aColor));
          default:
            xyzColor = ColorConvert_RGB_XYZ(ColorConvert_YUV_RGB(aColor),aXYZRef);
            break;
        }
        break;
      }
    case eColorSpace_YCbCr:
      {
        switch (aDest) {
          case eColorSpace_RGB:
            return ColorConvert_YCbCr_RGB(aColor);
          case eColorSpace_CMY:
            return ColorConvert_RGB_CMY(ColorConvert_YCbCr_RGB(aColor));
          case eColorSpace_CMYK:
            return ColorConvert_CMY_CMYK(ColorConvert_RGB_CMY(ColorConvert_YCbCr_RGB(aColor)));
          case eColorSpace_HSV:
            return ColorConvert_RGB_HSV(ColorConvert_YCbCr_RGB(aColor));
          case eColorSpace_HSL:
            return ColorConvert_RGB_HSL(ColorConvert_YCbCr_RGB(aColor));
          case eColorSpace_YIQ:
            return ColorConvert_RGB_YIQ(ColorConvert_YCbCr_RGB(aColor));
          case eColorSpace_YUV:
            return ColorConvert_RGB_YUV(ColorConvert_YCbCr_RGB(aColor));
          case eColorSpace_YPbPr:
            return ColorConvert_RGB_YPbPr(ColorConvert_YCbCr_RGB(aColor));
          case eColorSpace_YCoCg:
            return ColorConvert_RGB_YCoCg(ColorConvert_YCbCr_RGB(aColor));
          default:
            xyzColor = ColorConvert_RGB_XYZ(ColorConvert_YCbCr_RGB(aColor),aXYZRef);
            break;
        }
        break;
      }
    case eColorSpace_YPbPr:
      {
        switch (aDest) {
          case eColorSpace_RGB:
            return ColorConvert_YPbPr_RGB(aColor);
          case eColorSpace_CMY:
            return ColorConvert_RGB_CMY(ColorConvert_YPbPr_RGB(aColor));
          case eColorSpace_CMYK:
            return ColorConvert_CMY_CMYK(ColorConvert_RGB_CMY(ColorConvert_YPbPr_RGB(aColor)));
          case eColorSpace_HSV:
            return ColorConvert_RGB_HSV(ColorConvert_YPbPr_RGB(aColor));
          case eColorSpace_HSL:
            return ColorConvert_RGB_HSL(ColorConvert_YPbPr_RGB(aColor));
          case eColorSpace_YIQ:
            return ColorConvert_RGB_YIQ(ColorConvert_YPbPr_RGB(aColor));
          case eColorSpace_YUV:
            return ColorConvert_RGB_YUV(ColorConvert_YPbPr_RGB(aColor));
          case eColorSpace_YCbCr:
            return ColorConvert_RGB_YCbCr(ColorConvert_YPbPr_RGB(aColor));
          case eColorSpace_YCoCg:
            return ColorConvert_RGB_YCbCr(ColorConvert_YPbPr_RGB(aColor));
          default:
            xyzColor = ColorConvert_RGB_XYZ(ColorConvert_YPbPr_RGB(aColor),aXYZRef);
            break;
        }
        break;
      }
    case eColorSpace_YCoCg:
      {
        switch (aDest) {
          case eColorSpace_RGB:
            return ColorConvert_YCoCg_RGB(aColor);
          case eColorSpace_CMY:
            return ColorConvert_RGB_CMY(ColorConvert_YCoCg_RGB(aColor));
          case eColorSpace_CMYK:
            return ColorConvert_CMY_CMYK(ColorConvert_RGB_CMY(ColorConvert_YCoCg_RGB(aColor)));
          case eColorSpace_HSV:
            return ColorConvert_RGB_HSV(ColorConvert_YCoCg_RGB(aColor));
          case eColorSpace_HSL:
            return ColorConvert_RGB_HSL(ColorConvert_YCoCg_RGB(aColor));
          case eColorSpace_YIQ:
            return ColorConvert_RGB_YIQ(ColorConvert_YCoCg_RGB(aColor));
          case eColorSpace_YUV:
            return ColorConvert_RGB_YUV(ColorConvert_YCoCg_RGB(aColor));
          case eColorSpace_YCbCr:
            return ColorConvert_RGB_YCbCr(ColorConvert_YCoCg_RGB(aColor));
          case eColorSpace_YPbPr:
            return ColorConvert_RGB_YPbPr(ColorConvert_YCoCg_RGB(aColor));
          default:
            xyzColor = ColorConvert_RGB_XYZ(ColorConvert_YCoCg_RGB(aColor),aXYZRef);
            break;
        }
        break;
      }
    default:
      /* do nothing */
      break;
  }

  /// XYZ to other color formats
  switch (aDest) {
    case eColorSpace_RGB:
      {
        return ColorConvert_XYZ_RGB(xyzColor,aXYZRef);
      }
    case eColorSpace_CMY:
      {
        return ColorConvert_RGB_CMY(ColorConvert_XYZ_RGB(xyzColor,aXYZRef));
      }
    case eColorSpace_CMYK:
      {
        return ColorConvert_CMY_CMYK(ColorConvert_RGB_CMY(ColorConvert_XYZ_RGB(xyzColor,aXYZRef)));
      }
    case eColorSpace_HSV:
      {
        return ColorConvert_RGB_HSV(ColorConvert_XYZ_RGB(xyzColor,aXYZRef));
      }
    case eColorSpace_HSL:
      {
        return ColorConvert_RGB_HSL(ColorConvert_XYZ_RGB(xyzColor,aXYZRef));
      }
    case eColorSpace_XYZ:
      { // already in XYZ format
        return xyzColor;
      }
    case eColorSpace_Yxy:
      {
        return ColorConvert_XYZ_Yxy(xyzColor);
      }
    case eColorSpace_HunterLab:
      {
        return ColorConvert_XYZ_HunterLab(xyzColor);
      }
    case eColorSpace_CIELab:
      {
        return ColorConvert_XYZ_CIELab(xyzColor,aXYZRef);
      }
    case eColorSpace_CIELCH:
      {
        return ColorConvert_CIELab_CIELCH(ColorConvert_XYZ_CIELab(xyzColor,aXYZRef));
      }
    case eColorSpace_CIELuv:
      {
        return ColorConvert_XYZ_CIELuv(xyzColor,aXYZRef);
      }
    case eColorSpace_YIQ:
      {
        return ColorConvert_RGB_YIQ(ColorConvert_XYZ_RGB(xyzColor,aXYZRef));
      }
    case eColorSpace_YUV:
      {
        return ColorConvert_RGB_YUV(ColorConvert_XYZ_RGB(xyzColor,aXYZRef));
      }
    case eColorSpace_YCbCr:
      {
        return ColorConvert_RGB_YCbCr(ColorConvert_XYZ_RGB(xyzColor,aXYZRef));
      }
    case eColorSpace_YPbPr:
      {
        return ColorConvert_RGB_YPbPr(ColorConvert_XYZ_RGB(xyzColor,aXYZRef));
      }
    case eColorSpace_YCoCg:
      {
        return ColorConvert_RGB_YCoCg(ColorConvert_XYZ_RGB(xyzColor,aXYZRef));
      }
    default:
      /* do nothing */
      break;
  }

  return xyzColor;
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////}
/**@}*/
/**@}*/
}
#endif // __MATHCOLORS_22202854_H__
