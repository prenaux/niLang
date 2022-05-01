#ifndef __UDPOINTS_2795146_H__
#define __UDPOINTS_2795146_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */
/** \addtogroup niUI_Utils
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
// Sphere

///////////////////////////////////////////////
//! Generates random points on a unit sphere.
template <typename T>
inline void UDPointsSphereRandom(sVec3<T>* result, tU32 n)
{
  if (n == 0) return;

  T theta, angle2;
  tI32 pos;
  tU32 i;

  for (i = 0, pos = 0 ; i < n ; ++i, pos+=3, ++result)
  {
    theta = (T)acos(1.0 - 2.0*ni::RandFloat());
    angle2 = T(2.0*niPi*ni::RandFloat());
    result->x = T(sin(theta)*cos(angle2));
    result->y = T(sin(theta)*sin(angle2));
    result->z = T(cos(theta));
  }
}

///////////////////////////////////////////////
//! Generates hammersley base2 points on a unit sphere.
template <typename T>
inline void UDPointsSphereHammersley(sVec3<T>* result, tU32 n)
{
  if (n == 0) return;

  T p, t, st, phi, phirad;
  tI32 kk, pos;
  tU32 k;

  for (k=0, pos=0 ; k<n ; ++k, ++result)
  {
    t = T(0);
    for (p=0.5, kk=k ; kk ; p*=0.5, kk>>=1)
      if (kk & 1)                           // kk mod 2 == 1
        t += p;
    t = T(2.0 * t - 1.0);                     // map from [0,1] to [-1,1]

    phi = T((k + 0.5) / n);                   // a slight shift
    phirad =  T(phi * 2.0 * niPi);           // map to [0, 2 pi)

    st = T(sqrt(1.0-t*t));
    result->x = T(st * cos(phirad));
    result->y = T(st * sin(phirad));
    result->z = T(t);
  }
}

///////////////////////////////////////////////
//! Generates hammersley base p1 points on a unit sphere.
template <typename T>
inline void UDPointsSphereHammersleyEx(sVec3<T>* result, tU32 n, tI32 p1)
{
  if (n == 0) return;

  T a, p, ip, t, st, phi, phirad;
  tI32 kk, pos;
  tU32 k;

  for (k=0, pos=0 ; k<n ; ++k, ++result)
  {
    t = 0;
    ip = T(1.0/p1);                           // recipical of p1
    for (p=ip, kk=k ; kk ; p*=ip, kk/=p1)  // kk = (tI32)(kk/p1)
      if ((a = T(kk % p1)))
        t += a * p;
    t = T(2.0 * t  - 1.0);                    // map from [0,1] to [-1,1]

    phi = T((k + 0.5) / n);
    phirad =  T(phi * 2.0 * niPi);            // map to [0, 2 pi)

    st = T(sqrt(1.0-t*t));
    result->x = T(st * cos(phirad));
    result->y = T(st * sin(phirad));
    result->z = T(t);
  }
}

///////////////////////////////////////////////
//! Generates halton p1=2,p2  points on a unit sphere.
template <typename T>
inline void UDPointsSphereHalton(sVec3<T>* result, tU32 n, tI32 p2 = 3)
{
  if (n == 0) return;

  niAssert(p2 > 2);
  if (p2 <= 2) return;

  T p, t, st, phi, phirad, ip;
  tI32 kk, pos, a;
  tU32 k;

  for (k=0, pos=0 ; k<n ; ++k, ++result)
  {
    t = 0;
    for (p=0.5, kk=k ; kk ; p*=0.5, kk>>=1)
      if (kk & 1)                          // kk mod 2 == 1
        t += p;
    t = T(2.0 * t - 1.0);                     // map from [0,1] to [-1,1]
    st = T(sqrt(1.0-t*t));

    phi = 0;
    ip = T(1.0/p2);                        // recipical of p2
    for (p=ip, kk=k ; kk ; p*=ip, kk/=p2) {  // kk = (tI32)(kk/p2)
      a = kk % p2;
      if (a) {
        phi += a * p;
      }
    }
    phirad = T(phi * 2.0 * niPi);            // map from [0,0.5] to [0, 2 pi)

    result->x = T(st * cos(phirad));
    result->y = T(st * sin(phirad));
    result->z = T(t);
  }
}

///////////////////////////////////////////////
//! Generates halton p1,p2  points on a unit sphere.
template <typename T>
inline void UDPointsSphereHaltonEx(sVec3<T>* result, tU32 n, tI32 p1, tI32 p2)
{
  if (n == 0) return;

  niAssert(p2 > p1);
  if (p2 <= p1) return;

  T p, t, st, phi, phirad, ip;
  tI32 kk, pos, a;
  tU32 k;

  for (k=0, pos=0 ; k<n ; ++k, ++result)
  {
    t = 0;
    ip = T(1.0/p1);                        // recipical of p1
    for (p=ip, kk=k ; kk ; p*=ip, kk/=p1) {  // kk = (tI32)(kk/p1)
      a = kk % p1;
      if (a) {
        t += a * p;
      }
    }
    t = T(2.0 * t - 1.0);                    // map from [0,1] to [-1,1]
    st = T(sqrt(1.0-t*t));

    phi = 0;
    ip = T(1.0/p2);                        // recipical of p2
    for (p=ip, kk=k ; kk ; p*=ip, kk/=p2) {  // kk = (tI32)(kk/p2)
      a = kk % p2;
      if (a) {
        phi += a * p;
      }
    }
    phirad =  T(phi * 2.0 * niPi);            // map from [0,0.5] to [0, 2 pi)

    result->x = T(st * cos(phirad));
    result->y = T(st * sin(phirad));
    result->z = T(t);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Hemisphere (upper (Y+ dir) hemisphere)

///////////////////////////////////////////////
//! Generates random points on a unit hemisphere.
template <typename T>
inline void UDPointsHemisphereRandom(sVec3<T>* result, tU32 n)
{
  if (n == 0) return;

  T theta, angle2;

  for (tU32 i = 0, pos = 0 ; i < n ; ++i, pos+=3, ++result)
  {
    theta = (T)acos(1.0 - 2.0*ni::RandFloat());
    angle2 = T(niPi*ni::RandFloat());
    result->x = T(sin(theta)*cos(angle2));
    result->y = T(sin(theta)*sin(angle2));
    result->z = T(cos(theta));
  }
}

///////////////////////////////////////////////
//! Generates hammersley base2 points on a unit hemisphere.
template <typename T>
inline void UDPointsHemisphereHammersley(sVec3<T>* result, tU32 n)
{
  if (n == 0) return;

  T p, t, st, phi, phirad;
  tI32 kk, pos;
  tU32 k;

  for (k=0, pos=0 ; k<n ; ++k, ++result)
  {
    t = T(0);
    for (p=0.5, kk=k ; kk ; p*=0.5, kk>>=1)
      if (kk & 1)                           // kk mod 2 == 1
        t += p;
    t = T(2.0 * t - 1.0);                     // map from [0,1] to [-1,1]

    phi = T((k + 0.5) / n);                   // a slight shift
    phirad =  T(phi * niPi);           // map to [0, 2 pi)

    st = T(sqrt(1.0-t*t));
    result->x = T(st * cos(phirad));
    result->y = T(st * sin(phirad));
    result->z = T(t);
  }
}

///////////////////////////////////////////////
//! Generates hammersley base p1 points on a unit hemisphere.
template <typename T>
inline void UDPointsHemisphereHammersleyEx(sVec3<T>* result, tU32 n, tI32 p1)
{
  if (n == 0) return;

  T a, p, ip, t, st, phi, phirad;
  tI32 kk, pos;
  tU32 k;

  for (k=0, pos=0 ; k<n ; ++k, ++result)
  {
    t = 0;
    ip = T(1.0/p1);                           // recipical of p1
    for (p=ip, kk=k ; kk ; p*=ip, kk/=p1)  // kk = (tI32)(kk/p1)
      if ((a = T(kk % p1)))
        t += a * p;
    t = T(2.0 * t  - 1.0);                    // map from [0,1] to [-1,1]

    phi = T((k + 0.5) / n);
    phirad =  T(phi * niPi);            // map to [0, 2 pi)

    st = T(sqrt(1.0-t*t));
    result->x = T(st * cos(phirad));
    result->y = T(st * sin(phirad));
    result->z = T(t);
  }
}

///////////////////////////////////////////////
//! Generates halton p1=2,p2  points on a unit hemisphere.
template <typename T>
inline void UDPointsHemisphereHalton(sVec3<T>* result, tU32 n, tI32 p2 = 3)
{
  if (n == 0) return;

  niAssert(p2 > 2);
  if (p2 <= 2) return;

  T p, t, st, phi, phirad, ip;
  tI32 kk, pos, a;
  tU32 k;

  for (k=0, pos=0 ; k<n ; ++k, ++result)
  {
    t = 0;
    for (p=0.5, kk=k ; kk ; p*=0.5, kk>>=1)
      if (kk & 1)                          // kk mod 2 == 1
        t += p;
    t = T(2.0 * t - 1.0);                     // map from [0,1] to [-1,1]
    st = T(sqrt(1.0-t*t));

    phi = 0;
    ip = T(1.0/p2);                        // recipical of p2
    for (p=ip, kk=k ; kk ; p*=ip, kk/=p2) {  // kk = (tI32)(kk/p2)
      a = kk % p2;
      if (a) {
        phi += a * p;
      }
    }
    phirad = T(phi * niPi);            // map from [0,0.5] to [0, 2 pi)

    result->x = T(st * cos(phirad));
    result->y = T(st * sin(phirad));
    result->z = T(t);
  }
}

///////////////////////////////////////////////
//! Generates halton p1,p2  points on a unit hemisphere.
template <typename T>
inline void UDPointsHemisphereHaltonEx(sVec3<T>* result, tU32 n, tI32 p1, tI32 p2)
{
  if (n == 0) return;

  niAssert(p2 > p1);
  if (p2 <= p1) return;

  T p, t, st, phi, phirad, ip;
  tI32 kk, pos, a;
  tU32 k;

  for (k=0, pos=0 ; k<n ; ++k, ++result)
  {
    t = 0;
    ip = T(1.0/p1);                        // recipical of p1
    for (p=ip, kk=k ; kk ; p*=ip, kk/=p1) { // kk = (tI32)(kk/p1)
      a = kk % p1;
      if (a) {
        t += a * p;
      }
    }
    t = T(2.0 * t - 1.0);                    // map from [0,1] to [-1,1]
    st = T(sqrt(1.0-t*t));

    phi = 0;
    ip = T(1.0/p2);                        // recipical of p2
    for (p=ip, kk=k ; kk ; p*=ip, kk/=p2) { // kk = (tI32)(kk/p2)
      a = kk % p2;
      if (a) {
        phi += a * p;
      }
    }
    phirad =  T(phi * niPi);            // map from [0,0.5] to [0, 2 pi)

    result->x = T(st * cos(phirad));
    result->y = T(st * sin(phirad));
    result->z = T(t);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Plane

////////////////////////////////////////////////
//! Generate random points on a unit plane.
template <typename T>
void UDPointsPlaneRandom(sVec2<T>* result, tU32 n)
{
  if (n == 0) return;
  for (tU32 i=0; i<n ; ++i, ++result)
  {
    result->x = ni::RandFloat();
    result->y = ni::RandFloat();
  }
}

////////////////////////////////////////////////
//! Generate hammersley base2 points on a unit plane.
template <typename T>
void UDPointsPlaneHammersley(sVec2<T>* result, tU32 n)
{
  if (n == 0) return;

  T p, u, v;
  tI32 kk, pos;
  tU32 k;

  for (k=0, pos=0 ; k<n ; ++k, ++result)
  {
    u = 0;
    for (p=0.5, kk=k ; kk ; p*=0.5, kk>>=1) {
      if (kk & 1)                           // kk mod 2 == 1
        u += p;
    }

    v = T((k + 0.5) / n);

    result->x = u;
    result->y = v;
  }
}

////////////////////////////////////////////////
//! Generate hammersley base p1 points on a unit plane.
template <typename T>
void UDPointsPlaneHammersleyEx(sVec2<T>* result, tU32 n, tI32 p1)
{
  if (n == 0) return;

  T a, p, ip, u, v;
  tI32 kk, pos;
  tU32 k;

  for (k=0, pos=0 ; k<n ; ++k, ++result)
  {
    u = 0;
    ip = T(1.0/p1);                        // recipical of p1
    for (p=ip, kk=k ; kk ; p*=ip, kk/=p1) {  // kk = (tI32)(kk/p1)
      a = T(kk % p1);
      if (a) {
        u += a * p;
      }
    }

    v = T((k + 0.5) / n);

    result->x = u;
    result->y = v;
  }
}


////////////////////////////////////////////////
//! Generate halton p1=2,p2 points on a unit plane.
template <typename T>
void UDPointsPlaneHalton(sVec2<T>* result, tU32 n, tI32 p2 = 3)
{
  if (n == 0) return;

  niAssert(p2 > 2);
  if (p2 <= 2) return;

  T p, u, v, ip;
  tI32 kk, pos, a;
  tU32 k;

  for (k=0, pos=0 ; k<n ; ++k, ++result)
  {
    u = 0;
    for (p=0.5, kk=k ; kk ; p*=0.5, kk>>=1) {
      if (kk & 1)                          // kk mod 2 == 1
        u += p;
    }

    v = 0;
    ip = T(1.0/p2);                        // recipical of p2
    for (p=ip, kk=k ; kk ; p*=ip, kk/=p2) {  // kk = (tI32)(kk/p2)
      a = kk % p2;
      if (a) {
        v += a * p;
      }
    }

    result->x = u;
    result->y = v;
  }
}


////////////////////////////////////////////////
//! Generate halton p1,p2 points on a unit plane.
template <typename T>
void UDPointsPlaneHaltonEx(sVec2<T>* result, tU32 n, tI32 p1, tI32 p2)
{
  if (n == 0) return;

  niAssert(p2 > p1);
  if (p2 <= p1) return;

  T p, u, v, ip;
  tI32 kk, pos, a;
  tU32 k;

  for (k=0, pos=0 ; k<n ; ++k, ++result)
  {
    u = 0;
    ip = T(1.0/p1);                        // recipical of p1
    for (p=ip, kk=k ; kk ; p*=ip, kk/=p1) {  // kk = (tI32)(kk/p1)
      a = kk % p1;
      if (a) {
        u += a * p;
      }
    }

    v = 0;
    ip = T(1.0/p2);                        // recipical of p2
    for (p=ip, kk=k ; kk ; p*=ip, kk/=p2) {  // kk = (tI32)(kk/p2)
      a = kk % p2;
      if (a)
        v += a * p;
    }

    result->x = u;
    result->y = v;
  }
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __UDPOINTS_2795146_H__
