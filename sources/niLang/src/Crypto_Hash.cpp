// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/Types.h"
#if niMinFeatures(15)

#include "API/niLang/Utils/UnknownImpl.h"
#include "API/niLang/ICrypto.h"
#include "API/niLang/IFile.h"
#include "API/niLang/Var.h"
#include "API/niLang/StringDef.h"

#include "zlib/zlib.h"
#include "mbedtls/md5.h"
#include "mbedtls/ripemd160.h"
#include "mbedtls/sha1.h"
#include "mbedtls/sha256.h"
#include "mbedtls/sha512.h"

using namespace ni;

//===========================================================================
//
// Our secure hashes implementation
//
//===========================================================================
struct iCryptoHashDef {
  virtual int __stdcall GetSize() const = 0;
  virtual const achar* __stdcall GetName() const = 0;
};

struct MD5 {
  static struct Def : public iCryptoHashDef {
    virtual int __stdcall GetSize() const { return DIGEST_SIZE; }
    virtual const achar* __stdcall GetName() const { return "SHA-1"; }
  } _Def;

  enum { DIGEST_SIZE = 16 };

  mbedtls_md5_context _ctx;

  MD5() {
    mbedtls_md5_init(&_ctx);
    Restart();
  }
  ~MD5() {
    mbedtls_md5_free(&_ctx);
  }
  void Restart() {
    mbedtls_md5_starts(&_ctx);
  }
  void Update(void* data, size_t size) {
    mbedtls_md5_update(&_ctx, (unsigned char*)data, size);
  }
  void Final(void* data) {
    mbedtls_md5_finish(&_ctx, (unsigned char*)data);
    Restart();
  }
};
MD5::Def MD5::_Def;

struct SHA1 {
  static struct Def : public iCryptoHashDef {
    virtual int __stdcall GetSize() const { return DIGEST_SIZE; }
    virtual const achar* __stdcall GetName() const { return "SHA-1"; }
  } _Def;

  enum { DIGEST_SIZE = 20 };

  mbedtls_sha1_context _ctx;

  SHA1() {
    mbedtls_sha1_init(&_ctx);
    Restart();
  }
  ~SHA1() {
    mbedtls_sha1_free(&_ctx);
  }
  void Restart() {
    mbedtls_sha1_starts(&_ctx);
  }
  void Update(void* data, size_t size) {
    mbedtls_sha1_update(&_ctx, (unsigned char*)data, size);
  }
  void Final(void* data) {
    mbedtls_sha1_finish(&_ctx, (unsigned char*)data);
    Restart();
  }
};
SHA1::Def SHA1::_Def;

struct RIPEMD160 {
  static struct Def : public iCryptoHashDef {
    virtual int __stdcall GetSize() const { return DIGEST_SIZE; }
    virtual const achar* __stdcall GetName() const { return "SHA-1"; }
  } _Def;

  enum { DIGEST_SIZE = 20 };

  mbedtls_ripemd160_context _ctx;

  RIPEMD160() {
    mbedtls_ripemd160_init(&_ctx);
    Restart();
  }
  ~RIPEMD160() {
    mbedtls_ripemd160_free(&_ctx);
  }
  void Restart() {
    mbedtls_ripemd160_starts(&_ctx);
  }
  void Update(void* data, size_t size) {
    mbedtls_ripemd160_update(&_ctx, (unsigned char*)data, size);
  }
  void Final(void* data) {
    mbedtls_ripemd160_finish(&_ctx, (unsigned char*)data);
    Restart();
  }
};
RIPEMD160::Def RIPEMD160::_Def;

struct SHA256 {
  static struct Def : public iCryptoHashDef {
    virtual int __stdcall GetSize() const { return DIGEST_SIZE; }
    virtual const achar* __stdcall GetName() const { return "SHA256"; }
  } _Def;

  enum { DIGEST_SIZE = 32 };

  mbedtls_sha256_context _ctx;

  SHA256() {
    mbedtls_sha256_init(&_ctx);
    Restart();
  }
  ~SHA256() {
    mbedtls_sha256_free(&_ctx);
  }
  void Restart() {
    mbedtls_sha256_starts(&_ctx, 0);
  }
  void Update(void* data, size_t size) {
    mbedtls_sha256_update(&_ctx, (unsigned char*)data, size);
  }
  void Final(void* data) {
    mbedtls_sha256_finish(&_ctx, (unsigned char*)data);
    Restart();
  }
};
SHA256::Def SHA256::_Def;

struct SHA384 {
  static struct Def : public iCryptoHashDef {
    virtual int __stdcall GetSize() const { return DIGEST_SIZE; }
    virtual const achar* __stdcall GetName() const { return "SHA384"; }
  } _Def;

  enum { DIGEST_SIZE = 48 };

  mbedtls_sha512_context _ctx;

  SHA384() {
    mbedtls_sha512_init(&_ctx);
    Restart();
  }
  ~SHA384() {
    mbedtls_sha512_free(&_ctx);
  }
  void Restart() {
    mbedtls_sha512_starts(&_ctx, 1);
  }
  void Update(void* data, size_t size) {
    mbedtls_sha512_update(&_ctx, (unsigned char*)data, size);
  }
  void Final(void* data) {
    mbedtls_sha512_finish(&_ctx, (unsigned char*)data);
    Restart();
  }
};
SHA384::Def SHA384::_Def;

enum eSHA512_DigestSize {
  eSHA512_DigestSize_128 = 16,
  eSHA512_DigestSize_160 = 20,
  eSHA512_DigestSize_256 = 32,
  eSHA512_DigestSize_512 = 64,
};

#define SHA512_VARIANT(NAME,NUM_BITS)                                   \
  struct NAME {                                                         \
    static struct Def : public iCryptoHashDef {                         \
      virtual int __stdcall GetSize() const { return eSHA512_DigestSize_##NUM_BITS; } \
      virtual const achar* __stdcall GetName() const { return "NAME"; } \
    } _Def;                                                             \
                                                                        \
    enum { DIGEST_SIZE = eSHA512_DigestSize_##NUM_BITS };               \
                                                                        \
    mbedtls_sha512_context _ctx;                                        \
                                                                        \
    NAME() {                                                            \
      mbedtls_sha512_init(&_ctx);                                       \
      Restart();                                                        \
    }                                                                   \
    ~NAME() {                                                           \
      mbedtls_sha512_free(&_ctx);                                       \
    }                                                                   \
    void Restart() {                                                    \
      mbedtls_sha512_starts(&_ctx, 0);                                  \
    }                                                                   \
    void Update(void* data, size_t size) {                              \
      mbedtls_sha512_update(&_ctx, (tU8*)data, size);                   \
    }                                                                   \
    void Final(void* data) {                                            \
      if (DIGEST_SIZE == 64) {                                          \
        mbedtls_sha512_finish(&_ctx, (tU8*)data);                       \
      }                                                                 \
      else {                                                            \
        tU8 fullDigest[64];                                             \
        mbedtls_sha512_finish(&_ctx, fullDigest);                       \
        memcpy(data,fullDigest,DIGEST_SIZE);                            \
      }                                                                 \
      Restart();                                                        \
    }                                                                   \
  };                                                                    \
  NAME::Def NAME::_Def;

SHA512_VARIANT(SHA512_128, 128);
SHA512_VARIANT(SHA512_160, 160);
SHA512_VARIANT(SHA512_256, 256);
SHA512_VARIANT(SHA512, 512);

//===========================================================================
//
// Hash
//
//===========================================================================
template <typename tHashModule>
class cCryptoHash : public cIUnknownImpl<iCryptoHash>
{
  static const tU32 BLOCKSIZE = 4096;

 public:
  cCryptoHash() {
    ZeroMembers();
  }
  ~cCryptoHash() {
  }

  tBool __stdcall IsOK() const {
    return eTrue;
  }

  void __stdcall ZeroMembers() {
  }

  const achar* __stdcall GetHashType() const {
    return tHashModule::_Def.GetName();
  }
  tU32 __stdcall GetDigestSize() const {
    return tHashModule::DIGEST_SIZE;
  }

  tBool __stdcall UpdateBlock(iFile* apFile, tI64 aStart, tI64 aSize) {
    niCheckSilent(niIsOK(apFile),eFalse);
    apFile->SeekSet(aStart);
    return Update(apFile,aSize);
  }
  tBool __stdcall Update(iFile* apFile, tI64 aSize) {
    niCheckSilent(niIsOK(apFile),eFalse);
    tU8 buff[BLOCKSIZE+1];
    tI64 sz = aSize;
    if (sz) {
      // process blocks
      while (sz > BLOCKSIZE) {
        tI64 read = apFile->ReadRaw(buff,BLOCKSIZE);
        if (read != BLOCKSIZE) return eFalse;
        _hash.Update(buff,BLOCKSIZE);
        sz -= BLOCKSIZE;
      }
      // process last block
      if (sz > 0) {
        tI64 read = apFile->ReadRaw(buff,(tSize)sz);
        if (read != sz) return eFalse;
        _hash.Update(buff,(tSize)sz);
        sz = 0;
      }
    }
    else {
      // no size specified read until partial read or zero bytes read
      do {
        tI64 read = apFile->ReadRaw(buff,BLOCKSIZE);
        if (read == 0) break;
        _hash.Update(buff,(tSize)read);
        if (apFile->GetPartialRead()) break;
      } while (1);
    }
    return eTrue;
  }
  tBool __stdcall UpdateRaw(tPtr apData, tSize aSize) {
    niCheckSilent(apData!=NULL,eFalse);
    _hash.Update(apData,aSize);
    return eTrue;
  }

  tSize __stdcall FinalRaw(tPtr apData, tSize anMaxSize) {
    niCheckSilent(apData != NULL,eFalse);
    niCheckSilent(anMaxSize >= tHashModule::DIGEST_SIZE,eFalse);
    _hash.Final(apData);
    return tHashModule::DIGEST_SIZE;
  }
  tSize __stdcall FinalFile(iFile* apDigest) {
    niCheckSilent(niIsOK(apDigest),eFalse);
    sWriteBufferToFile buffer(apDigest,tHashModule::DIGEST_SIZE);
    _hash.Final(buffer.begin());
    return buffer.commit(-1);
  }
  cString __stdcall FinalString(eRawToStringEncoding aEncoding) {
    tU8 buffer[tHashModule::DIGEST_SIZE];
    _hash.Final(buffer);
    switch (aEncoding) {
      case eRawToStringEncoding_Hex: {
        return HexEncodeToString(buffer, tHashModule::DIGEST_SIZE, eFalse);
      }
      case eRawToStringEncoding_Base64: {
        return Base64EncodeToString(buffer, tHashModule::DIGEST_SIZE);
      }
      case eRawToStringEncoding_Base32: {
        return Base32EncodeToString(buffer, tHashModule::DIGEST_SIZE, eFalse);
      }
    }
    return AZEROSTR;
  }

  tBool __stdcall Restart() {
    _hash.Restart();
    return eTrue;
  }

 private:
  tHashModule _hash;
};

//////////////////////////////////////////////////////////////////////////////////////////////
niExportFunc(ni::iUnknown*) New_niLang_CryptoHash(const Var& avarA, const Var& avarB) {
  Ptr<iCryptoHash> hash;
  cString type = VarGetString(avarA);
  if (type.IEq("MD5")) {
    hash = niNew cCryptoHash<MD5>();
  }
  else if (type.IEq("RIPEMD160")) {
    hash = niNew cCryptoHash<RIPEMD160>();
  }
  else if (type.IEq("SHA1") || type.IEq("SHA-1")) {
    hash = niNew cCryptoHash<SHA1>();
  }
  else if (type.IEq("SHA256")) {
    hash = niNew cCryptoHash<SHA256>();
  }
  else if (type.IEq("SHA384")) {
    hash = niNew cCryptoHash<SHA384>();
  }
  else if (type.IEq("SHA512_128")) {
    hash = niNew cCryptoHash<SHA512_128>();
  }
  else if (type.IEq("SHA512_160")) {
    hash = niNew cCryptoHash<SHA512_160>();
  }
  else if (type.IEq("SHA512_256")) {
    hash = niNew cCryptoHash<SHA512_256>();
  }
  else if (type.IEq("SHA512")) {
    hash = niNew cCryptoHash<SHA512>();
  }
  else {
    niError(niFmt("Message Hash algorithm '%s' not supported.",type));
    return NULL;
  }
  return hash.GetRawAndSetNull();
}
#endif
