#ifndef __FOURCC_56393748_H__
#define __FOURCC_56393748_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../StringDef.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
class FourCC
{
 public:
  //! Initialize the FourCC with a tU32 value.
  inline FourCC(tU32 anFourCC = 0);
  //! Initialize the FourCC with four tU8.
  inline FourCC(tU8 anA, tU8 anB, tU8 anC, tU8 anD);
  //! Initialize the FourCC with a string.
  inline FourCC(const cchar* acszFourCC);

  //! Cast to a tU32.
  inline operator tU32 () const;

  //! Set the ABCD components of the FourCC.
  inline void Set(tU8 anA, tU8 anB, tU8 anC, tU8 anD);
  //! Set the A component of the FourCC.
  inline void SetA(tU8 anV);
  //! Set teh B component of the FourCC.
  inline void SetB(tU8 anV);
  //! Set the C component of the FourCC.
  inline void SetC(tU8 anV);
  //! Set the D component of the FourCC.
  inline void SetD(tU8 anV);
  //! Get the A component of the FourCC.
  inline tU8 GetA() const;
  //! Get teh B component of the FourCC.
  inline tU8 GetB() const;
  //! Get the C component of the FourCC.
  inline tU8 GetC() const;
  //! Get the D component of the FourCC.
  inline tU8 GetD() const;
  //! Get the string translation of the FourCC.
  inline cString GetString() const;

  //! Check if this FourCC is equal to another FourCC.
  inline tBool operator == (const FourCC& aFourCC) const;
  //! Check if this FourCC is not equal to another FourCC.
  inline tBool operator != (const FourCC& aFourCC) const;

 private:
  union {
    tU32 mnFourCC;
    struct {
      tU8 mnA,mnB,mnC,mnD;
    };
  };
};

///////////////////////////////////////////////
//! Initialize the FourCC with a tU32 value.
inline FourCC::FourCC(tU32 anFourCC)
    : mnFourCC(anFourCC)
{
}

///////////////////////////////////////////////
//! Initialize the FourCC with four tU8.
inline FourCC::FourCC(tU8 anA, tU8 anB, tU8 anC, tU8 anD)
    : mnA(anA), mnB(anB), mnC(anC), mnD(anD)
{
  // fourcc = (((d)<<24)|((c)<<16)|((b)<<8)|(a))
}

///////////////////////////////////////////////
//! Initialize the FourCC with a string.
inline FourCC::FourCC(const cchar* acszFourCC)
{
  tSize nStrLen = strlen(acszFourCC);
  if (nStrLen == 0)
  {
    mnFourCC = 0;
  }
  else
  {
    mnA = acszFourCC[0];
    mnB = (nStrLen >= 2) ? acszFourCC[1] : 0;
    mnC = (nStrLen >= 3) ? acszFourCC[2] : 0;
    mnD = (nStrLen >= 4) ? acszFourCC[3] : 0;
  }
}

///////////////////////////////////////////////
//! Cast to a tU32.
inline FourCC::operator tU32 () const
{
  return mnFourCC;
}

///////////////////////////////////////////////
//! Set the ABCD components of the FourCC.
inline void FourCC::Set(tU8 anA, tU8 anB, tU8 anC, tU8 anD)
{
  mnA = anA; mnB = anB; mnC = anC; mnD = anD;
}

///////////////////////////////////////////////
//! Set the A component of the FourCC.
inline void FourCC::SetA(tU8 anV)
{
  mnA = anV;
}

///////////////////////////////////////////////
//! Set teh B component of the FourCC.
inline void FourCC::SetB(tU8 anV)
{
  mnB = anV;
}

///////////////////////////////////////////////
//! Set the C component of the FourCC.
inline void FourCC::SetC(tU8 anV)
{
  mnC = anV;
}

///////////////////////////////////////////////
//! Set the D component of the FourCC.
inline void FourCC::SetD(tU8 anV)
{
  mnD = anV;
}

///////////////////////////////////////////////
//! Get the A component of the FourCC.
inline tU8 FourCC::GetA() const
{
  return mnA;
}

///////////////////////////////////////////////
//! Get teh B component of the FourCC.
inline tU8 FourCC::GetB() const
{
  return mnB;
}

///////////////////////////////////////////////
//! Get the C component of the FourCC.
inline tU8 FourCC::GetC() const
{
  return mnC;
}

///////////////////////////////////////////////
//! Get the D component of the FourCC.
inline tU8 FourCC::GetD() const
{
  return mnD;
}

///////////////////////////////////////////////
//! Get the string translation of the FourCC.
inline cString FourCC::GetString() const
{
  cString str;
  str.reserve(4);
  str.appendChar(mnA);
  str.appendChar(mnB);
  str.appendChar(mnC);
  str.appendChar(mnD);
  return str;
}

///////////////////////////////////////////////
//! Check if this FourCC is equal to another FourCC.
inline tBool FourCC::operator == (const FourCC& aFourCC) const
{
  return mnFourCC == aFourCC.mnFourCC;
}

///////////////////////////////////////////////
//! Check if this FourCC is not equal to another FourCC.
inline tBool FourCC::operator != (const FourCC& aFourCC) const
{
  return mnFourCC != aFourCC.mnFourCC;
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __FOURCC_56393748_H__
