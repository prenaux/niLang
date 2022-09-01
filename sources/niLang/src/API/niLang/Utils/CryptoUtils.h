#pragma once
#ifndef __CRYPTOUTILS_H_692D3D45_2A68_4E35_BC73_8BB77F69C4AE__
#define __CRYPTOUTILS_H_692D3D45_2A68_4E35_BC73_8BB77F69C4AE__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "../../niLang_ModuleDef.h"
#include "../StringDef.h"
#include "../IDataTable.h"
#include "../ICrypto.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

inline Ptr<iCryptoHash> CreateHash(const achar* aHashType) {
  return (iCryptoHash*)niCreateInstance(niLang,CryptoHash,aHashType,niVarNull);
}

inline Ptr<iCryptoHash> HashFile(iCryptoHash* apHash, iFile* apFile) {
  niAssert(niIsOK(apHash));
  niCheckIsOK(apFile,NULL);
  apFile->SeekSet(0);
  apHash->Update(apFile,apFile->GetSize());
  return apHash;
}

inline Ptr<iCryptoHash> HashString(iCryptoHash* apHash, const achar* aString, const tI32 aStrLen = 0) {
  niAssert(niIsOK(apHash));
  apHash->UpdateRaw((tPtr)aString, aStrLen > 0 ? aStrLen : StrSize(aString));
  return apHash;
}

inline Ptr<iCryptoHash> HashDataTable(iCryptoHash* apHash, iDataTable* apDT) {
  niAssert(niIsOK(apHash));
  Nonnull<iFile> file(GetLang()->CreateFileDynamicMemory(1024, ""));
  if (!GetLang()->SerializeDataTable("dtb",eSerializeMode_Write,apDT,file)) {
    niError("Can't write data table to temporary hash file.");
    return NULL;
  }
  file->SeekSet(0);
  apHash->Update(file, file->GetSize());
  return apHash;
}

inline cString HashToString(iCryptoHash* apHash, eRawToStringEncoding aEncoding = eRawToStringEncoding_Hex) {
  return apHash->FinalString(aEncoding);
}

inline sVec4<tI64> HashToInt256(iCryptoHash* apHash) {
  sVec4<tI64> buffer;
  niCAssert(sizeof(buffer) == 256/8);
  if (apHash->FinalRaw((tPtr)buffer.ptr(),sizeof(buffer)) != sizeof(buffer))
    return sVec4<tI64>::Zero();
  return buffer;
}

inline sVec4i HashToInt128(iCryptoHash* apHash) {
  sVec4i buffer;
  niCAssert(sizeof(buffer) == 128/8);
  if (apHash->FinalRaw((tPtr)buffer.ptr(),sizeof(buffer)) != sizeof(buffer))
    return sVec4i::Zero();
  return buffer;
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __CRYPTOUTILS_H_692D3D45_2A68_4E35_BC73_8BB77F69C4AE__
