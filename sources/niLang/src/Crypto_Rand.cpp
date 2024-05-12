#include "API/niLang/Types.h"
#include "API/niLang/IFile.h"
#include "API/niLang/ICrypto.h"
#include "API/niLang/Utils/UnknownImpl.h"

#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"

namespace ni {

static const tSize _kMaxChunkSize = MBEDTLS_CTR_DRBG_MAX_REQUEST;

tInt _RandMakeChunk(mbedtls_ctr_drbg_context* ctx, unsigned char* buffer, tSize length) {
  // length is smaller than
  if (length <= _kMaxChunkSize) {
    return mbedtls_ctr_drbg_random(ctx, buffer, length);
  }

  // needs to make in chunks
  for (tSize i = 0; (i + _kMaxChunkSize) <= length; i += _kMaxChunkSize) {
    tInt ret = mbedtls_ctr_drbg_random(ctx, buffer + i, _kMaxChunkSize);
    if (ret != 0)
      return ret;
  }

  // last chunk
  tSize residue = length % _kMaxChunkSize;
  if (residue) {
    tInt ret =
        mbedtls_ctr_drbg_random(ctx, buffer + length - residue, residue);
    if (ret != 0)
      return ret;
  }

  return 0; // success
}

struct cCryptoRand : public ImplRC<iCryptoRand> {
  mbedtls_entropy_context  _entropy;
  mbedtls_ctr_drbg_context _ctx;

  cCryptoRand() {
    mbedtls_entropy_init(&_entropy);
    mbedtls_ctr_drbg_init(&_ctx);
    mbedtls_ctr_drbg_seed(&_ctx, mbedtls_entropy_func, &_entropy, NULL, 0);
    mbedtls_ctr_drbg_set_prediction_resistance(&_ctx, MBEDTLS_CTR_DRBG_PR_OFF);
  }
  ~cCryptoRand() {
    mbedtls_entropy_free(&_entropy);
    mbedtls_ctr_drbg_free(&_ctx);
  }

  void __stdcall SetEntropyLength(tSize len) niImpl {
    mbedtls_ctr_drbg_set_entropy_len(&_ctx, len);
  }
  void __stdcall SetReseedInterval(tInt interval) niImpl {
    mbedtls_ctr_drbg_set_reseed_interval(&_ctx, interval);
  }
  void __stdcall SetPredictionResistance(tBool p) niImpl {
    mbedtls_ctr_drbg_set_prediction_resistance(
        &_ctx, p ? MBEDTLS_CTR_DRBG_PR_ON : MBEDTLS_CTR_DRBG_PR_OFF);
  }

  tBool __stdcall ReseedRaw(const tPtr custom, tSize length) niImpl {
    return (mbedtls_ctr_drbg_reseed(&_ctx, custom, length) == 0) ? eTrue : eFalse;
  }
  tBool __stdcall Reseed(iFile* apFile, tSize length) niImpl {
    if (!apFile && length == 0) {
      return (mbedtls_ctr_drbg_reseed(&_ctx, NULL, 0) == 0) ? eTrue : eFalse;
    }
    else {
      niCheckIsOK(apFile,eFalse);
      sReadBufferFromFile buffer(apFile, length);
      niCheck(buffer.readAll(),eFalse);
      return this->ReseedRaw(buffer.begin(),buffer.size());
    }
  }

  tBool __stdcall UpdateRaw(const tPtr additional, tSize length) niImpl {
    mbedtls_ctr_drbg_update(&_ctx, additional, length);
    return eTrue;
  }
  tBool __stdcall Update(iFile* apFile, tSize length) niImpl {
    niCheckIsOK(apFile,eFalse);
    sReadBufferFromFile buffer(apFile, length);
    niCheck(buffer.readAll(),eFalse);
    return UpdateRaw(buffer.begin(), buffer.size());
  }

  tSize __stdcall RandRaw(unsigned char* buffer, tSize olen) niImpl {
    if (_RandMakeChunk(&_ctx, buffer, olen) != 0) {
      return 0;
    }
    return olen;
  }
  tSize __stdcall RandFile(iFile* apFile, tSize olen) niImpl {
    niCheckIsOK(apFile,eFalse);
    sWriteBufferToFile buffer(apFile, olen);
    return RandRaw(buffer.begin(),buffer.size());
  }
  tI64  __stdcall RandInt() {
    tI64 r;
    RandRaw((tPtr)&r, sizeof(r));
    return r;
  }
  tF64  __stdcall RandFloat() {
    tF64 r;
    RandRaw((tPtr)&r, sizeof(r));
    return r;
  }
  tUUID __stdcall RandUUID() {
    tUUID r;
    RandRaw((tPtr)&r, sizeof(r));
    return r;
  }
};

niExportFunc(ni::iUnknown*) New_niLang_CryptoRand(const ni::Var&,const ni::Var&) {
  return niNew cCryptoRand();
}

niExportFunc(iCryptoRand*) GetCryptoRand() {
  static Ptr<iCryptoRand> _cryptoRand = niNew cCryptoRand();
  return _cryptoRand;
}

}
