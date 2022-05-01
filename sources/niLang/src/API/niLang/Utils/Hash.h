#ifndef __HASHIMPL_6497858_H__
#define __HASHIMPL_6497858_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"
#include "../STL/stl_alloc.h"

/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

namespace ni {

///////////////////////////////////////////////
//! A simple hash function from Robert Sedgwicks Algorithms in C book.
inline tU32 Hash_RS(tU8* str, tU32 len)
{
  tU32 b    = 378551;
  tU32 a    = 63689;
  tU32 hash = 0;
  tU32 i    = 0;

  for(i = 0; i < len; str++, i++)
  {
    hash = hash * a + (*str);
    a    = a * b;
  }

  return (hash & 0x7FFFFFFF);
}

///////////////////////////////////////////////
//! A bitwise hash function written by Justin Sobel.
inline tU32 Hash_JS(tU8* str, tU32 len)
{
  tU32 hash = 1315423911;
  tU32 i    = 0;

  for(i = 0; i < len; str++, i++)
  {
    hash ^= ((hash << 5) + (*str) + (hash >> 2));
  }

  return (hash & 0x7FFFFFFF);
}

///////////////////////////////////////////////
//! This hash algorithm is based on work by Peter J. Weinberger of AT&T Bell Labs.
//! The book Compilers (Principles, Techniques and Tools) by Aho, Sethi and Ulman, recommends the use
//! of hash functions that employ the hashing methodology found in this particular algorithm.
inline tU32 Hash_PJW(tU8* str, tU32 len)
{
  tU32 BitsInUnsignedInt = (tU32)(sizeof(tU32) * 8);
  tU32 ThreeQuarters     = (tU32)((BitsInUnsignedInt  * 3) / 4);
  tU32 OneEighth         = (tU32)(BitsInUnsignedInt / 8);
  tU32 HighBits          = (tU32)(0xFFFFFFFF) << (BitsInUnsignedInt - OneEighth);
  tU32 hash              = 0;
  tU32 test              = 0;
  tU32 i                 = 0;

  for(i = 0; i < len; str++, i++)
  {
    hash = (hash << OneEighth) + (*str);

    if((test = hash & HighBits)  != 0)
    {
      hash = (( hash ^ (test >> ThreeQuarters)) & (~HighBits));
    }
  }

  return (hash & 0x7FFFFFFF);
}

///////////////////////////////////////////////
//! Similar to the PJW Hash function, but tweaked for 32-bit processors. Its the hash function widely
//! used on most UNIX systems.
inline tU32 Hash_ELF(tU8* str, tU32 len)
{
  tU32 hash = 0;
  tU32 x    = 0;
  tU32 i    = 0;

  for(i = 0; i < len; str++, i++)
  {
    hash = (hash << 4) + (*str);
    if((x = hash & 0xF0000000L) != 0)
    {
      hash ^= (x >> 24);
      hash &= ~x;
    }
  }

  return (hash & 0x7FFFFFFF);
}

///////////////////////////////////////////////
//! This hash function comes from Brian Kernighan and Dennis Ritchie's book "The C Programming Language".
//! It is a simple hash function using a strange set of possible seeds which all constitute a pattern
//! of 31....31...31 etc, it seems to be very similar to the DJB hash function.
inline tU32 Hash_BKDR(tU8* str, tU32 len)
{
  tU32 seed = 131; // 31 131 1313 13131 131313 etc..
  tU32 hash = 0;
  tU32 i    = 0;

  for(i = 0; i < len; str++, i++)
  {
    hash = (hash * seed) + (*str);
  }

  return (hash & 0x7FFFFFFF);
}

///////////////////////////////////////////////
//! This is the algorithm of choice which is used in the open source SDBM project.
//! The hash function seems to have a good over-all distribution for many different data sets.
//! It seems to work well in situations where there is a high variance in the MSBs of the
//! elements in a data set.
inline tU32 Hash_SDBM(tU8* str, tU32 len)
{
  tU32 hash = 0;
  tU32 i    = 0;

  for(i = 0; i < len; str++, i++)
  {
    hash = (*str) + (hash << 6) + (hash << 16) - hash;
  }

  return (hash & 0x7FFFFFFF);
}

///////////////////////////////////////////////
//! An algorithm produced by Professor Daniel J. Bernstein and shown first to the world on
//! the usenet newsgroup comp.lang.c. It is one of the most efficient hash functions ever
//! published.
inline tU32 Hash_DJB(tU8* str, tU32 len)
{
  tU32 hash = 5381;
  tU32 i    = 0;

  for(i = 0; i < len; str++, i++)
  {
    hash = ((hash << 5) + hash) + (*str);
  }

  return (hash & 0x7FFFFFFF);
}

///////////////////////////////////////////////
//! An algorithm proposed by Donald E. Knuth in The Art Of Computer Programming Volume 3,
//! under the topic of sorting and search chapter 6.4.
inline tU32 Hash_DEK(tU8* str, tU32 len)
{
  tU32 hash = len;
  tU32 i    = 0;

  for(i = 0; i < len; str++, i++)
  {
    hash = ((hash << 5) ^ (hash >> 27)) ^ (*str);
  }
  return (hash & 0x7FFFFFFF);
}

///////////////////////////////////////////////
//! An algorithm produced by Arash Partow. He took ideas from all of the above hash functions making
//! a hybrid rotative and additive hash function algorithm based around four primes 3,5,7 and 11.
//! There isn't any real mathematical analysis explaining why one should use this hash function
//! instead of the others described above other than the fact that I tired to resemble the design
//! as close as possible to a simple LFSR. An empirical result which demonstrated the distributive
//! abilities of the hash algorithm was obtained using a hash-table with 100003 buckets, hashing
//! The Project Gutenberg Etext of Webster's Unabridged Dictionary, the longest encountered chain
//! length was 7, the average chain length was 2, the number of empty buckets was 4579.
inline tU32 Hash_AP(tU8* str, tU32 len)
{
  tU32 hash = 0;
  tU32 i    = 0;

  for(i = 0; i < len; str++, i++)
  {
    hash ^= ((i & 1) == 0) ? (  (hash <<  7) ^ (*str) ^ (hash >> 3)) :
        (~((hash << 11) ^ (*str) ^ (hash >> 5)));
  }

  return (hash & 0x7FFFFFFF);
}

///////////////////////////////////////////////
inline tU32 HashVec2(const tF32 v[2]) {
  const unsigned int * h = (const unsigned int *)(&v[0]);
  // the multipliers are prime numbers
  unsigned int f = (h[0] + h[1]*11)&0x7fffffff;     // avoid problems with +-0
  return (f>>22)^(f>>12)^(f);
}

///////////////////////////////////////////////
inline tU32 HashVec3(const tF32 v[3]) {
  const unsigned int * h = (const unsigned int *)(&v[0]);
  // the multipliers are prime numbers
  unsigned int f = (h[0] + h[1]*11 - (h[2]*17))&0x7fffffff;     // avoid problems with +-0
  return (f>>22)^(f>>12)^(f);
}

///////////////////////////////////////////////
inline tU32 HashVec4(const tF32 v[4]) {
  const unsigned int * h = (const unsigned int *)(&v[0]);
  // the multipliers are prime numbers
  unsigned int f = h[0]*1 + h[1]*11 + h[2]*17 + h[3]*19;
  return (f>>22)^(f>>12)^(f);
}

///////////////////////////////////////////////
inline tU32 HashMatrix(const tF32 v[16]) {
  tU32 rows[4] = {
    HashVec4(&v[0]),
    HashVec4(&v[4]),
    HashVec4(&v[8]),
    HashVec4(&v[12])
  };
  return Hash_AP((tU8*)&rows[0],sizeof(rows));
}

//! Compute a 16-bit hash for a GUID.
static inline ni::tU16 HashUUID(const tUUID& aUUID) {
  ni::tU16 c0, c1;
  ni::tU8* next_uuid;

  // initialize counters
  c0 = c1 = 0;
  next_uuid = (ni::tU8*)&aUUID.nData1;

  /*
   * For speed lets unroll the following loop:
   *
   *   for (i = 0; i < UUID_K_LENGTH; i++)
   *   {
   *       c0 = c0 + *next_uuid++;
   *       c1 = c1 + c0;
   *   }
   */
  c0 = c0 + *next_uuid++;
  c1 = c1 + c0;
  c0 = c0 + *next_uuid++;
  c1 = c1 + c0;
  c0 = c0 + *next_uuid++;
  c1 = c1 + c0;
  c0 = c0 + *next_uuid++;
  c1 = c1 + c0;

  c0 = c0 + *next_uuid++;
  c1 = c1 + c0;
  c0 = c0 + *next_uuid++;
  c1 = c1 + c0;
  c0 = c0 + *next_uuid++;
  c1 = c1 + c0;
  c0 = c0 + *next_uuid++;
  c1 = c1 + c0;

  c0 = c0 + *next_uuid++;
  c1 = c1 + c0;
  c0 = c0 + *next_uuid++;
  c1 = c1 + c0;
  c0 = c0 + *next_uuid++;
  c1 = c1 + c0;
  c0 = c0 + *next_uuid++;
  c1 = c1 + c0;

  c0 = c0 + *next_uuid++;
  c1 = c1 + c0;
  c0 = c0 + *next_uuid++;
  c1 = c1 + c0;
  c0 = c0 + *next_uuid++;
  c1 = c1 + c0;
  c0 = c0 + *next_uuid++;
  c1 = c1 + c0;

  // Calculate the value for "First octet" of the hash
  int x = -c1 % 255;
  if (x < 0) {
    x = x + 255;
  }

  // Calculate the value for "second octet" of the hash
  int y = (c1 - c0) % 255;
  if (y < 0) {
    y = y + 255;
  }

  return (ni::tU16)((y * 256) + x);
}

}

namespace eastl {
ASTL_TEMPLATE_NULL
struct hash<ni::tUUID> {
  __forceinline size_t operator()(const ni::tUUID& __x) const {
    return ni::HashUUID(__x);
  }
};
}

/**@}*/
/**@}*/
/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __HASHIMPL_6497858_H__
