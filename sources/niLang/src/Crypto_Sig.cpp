#include "API/niLang/Types.h"
#include "API/niLang/Utils/CryptoUtils.h"
#include "API/niLang/IToString.h"
#include "API/niLang/STL/scope_guard.h"
#include "API/niLang/StringDef.h"
#include "API/niLang/STL/vector.h"
#include "mbedtls/rsa.h"
#include "mbedtls/pk.h"
#include "mbedtls/md.h"
#include "mbedtls/sha256.h"

niExportFunc(ni::tBool) CryptoSigVerify(const char* signatureHex, const char* publicKeyPEM, const char* payload) {
  int r = -1;

  unsigned char hashBuffer[32];
  ni::Ptr<ni::iCryptoHash> hash = HashString(ni::CreateHash("SHA256"), payload, ni::StrLen(payload));
  hash->FinalRaw(hashBuffer, sizeof(hashBuffer));

  mbedtls_pk_context ctx;
  mbedtls_pk_init(&ctx);
  niDefer { mbedtls_pk_free(&ctx); };

  r = mbedtls_pk_parse_public_key(&ctx, (unsigned char*) publicKeyPEM, ni::StrSizeZ(publicKeyPEM));
  if (r != 0) {
    niError(niFmt("Couldn't parse the public key: %X.", ni::Abs(r)));
    return ni::eFalse;
  }

  mbedtls_rsa_context* rsa_ctx = mbedtls_pk_rsa(ctx);

  astl::vector<ni::tU8> sigData;
  sigData.resize(ni::HexDecodeOutputSize(signatureHex, 0));
  if (sigData.size() != 256) {
    niError("Invalid signature size, should be 256 bytes.");
    return ni::eFalse;
  }
  ni::HexDecode(sigData.data(), sigData.size(), signatureHex, 0);

  r = mbedtls_rsa_pkcs1_verify(rsa_ctx,
                               MBEDTLS_MD_SHA256,
                               sizeof(hashBuffer), hashBuffer,
                               sigData.data());
  if (r != 0) {
    niError(niFmt("Couldn't verify the signature: %X.", ni::Abs(r)));
    return ni::eFalse;
  }

  return ni::eTrue;
}

niExportFunc(ni::iFile*) CryptoHmacSignature(
  const char* aAlgo, ni::iFile* apOutput,
  ni::iFile* apPayload, ni::tSize aPayloadSize,
  const char* aSecret, ni::eRawToStringEncoding aSecretFormat)
{
  niCheckIsOK(apOutput, NULL);
  niCheckIsOK(apPayload, NULL);
  niCheck(aPayloadSize > 0, NULL);

  astl::vector<ni::tU8> decodedSecret;
  const ni::tSize slen = ni::StrLen(aSecret);
  switch (aSecretFormat) {
    case ni::eRawToStringEncoding_Hex: {
      const ni::tInt decodedSecretSize = ni::HexDecodeOutputSize(aSecret,slen);
      niCheck(decodedSecretSize >= 0, NULL);
      decodedSecret.resize(decodedSecretSize);
      ni::HexDecode((ni::tPtr)decodedSecret.data(),(ni::tInt)decodedSecret.size(),aSecret,slen);
      break;
    }
    case ni::eRawToStringEncoding_Base64: {
      const ni::tInt decodedSecretSize = ni::Base64DecodeOutputSize(aSecret,slen);
      niCheck(decodedSecretSize >= 0, NULL);
      decodedSecret.resize(decodedSecretSize);
      ni::Base64Decode((ni::tPtr)decodedSecret.data(),(ni::tInt)decodedSecret.size(),aSecret,slen);
      break;
    }
    case ni::eRawToStringEncoding_Base32: {
      const ni::tInt decodedSecretSize = ni::Base32DecodeOutputSize(aSecret,slen);
      niCheck(decodedSecretSize >= 0, NULL);
      decodedSecret.resize(decodedSecretSize);
      ni::Base32Decode((ni::tPtr)decodedSecret.data(),(ni::tInt)decodedSecret.size(),aSecret,slen);
      break;
    }
    default: {
      niError(niFmt("Unsupported secret format: '%d'.", (ni::tU32)aSecretFormat));
      return NULL;
    }
  }

  astl::vector<ni::tU8> payload;
  payload.resize(aPayloadSize);
  if (apPayload->ReadRaw(payload.data(), aPayloadSize) != aPayloadSize) {
    niError("Couldn't read the payload!");
    return NULL;
  }

  unsigned char hmac_hash[64] = {'\0'};
  mbedtls_md_context_t ctx;

  mbedtls_md_type_t md_type;
  if (ni::StrIEq(aAlgo, "MD5")) {
    md_type = MBEDTLS_MD_MD5;
  }
  else if (ni::StrIEq(aAlgo, "SHA1")) {
    md_type = MBEDTLS_MD_SHA1;
  }
  else if (ni::StrIEq(aAlgo, "SHA224")) {
    md_type = MBEDTLS_MD_SHA224;
  }
  else if (ni::StrIEq(aAlgo, "SHA256")) {
    md_type = MBEDTLS_MD_SHA256;
  }
  else if (ni::StrIEq(aAlgo, "SHA384")) {
    md_type = MBEDTLS_MD_SHA384;
  }
  else if (ni::StrIEq(aAlgo, "SHA512")) {
    md_type = MBEDTLS_MD_SHA512;
  }
  else if (ni::StrIEq(aAlgo, "RIPEMD160")) {
    md_type = MBEDTLS_MD_RIPEMD160;
  }
  else {
    niError(niFmt("Unknown signing algo '%s'.", aAlgo));
    return NULL;
  }

  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type) , 1); //use hmac
  mbedtls_md_hmac_starts(&ctx, (const unsigned char *)decodedSecret.data(), decodedSecret.size());
  mbedtls_md_hmac_update(&ctx, (const unsigned char *)payload.data(), payload.size());
  mbedtls_md_hmac_finish(&ctx, hmac_hash);
  mbedtls_md_free(&ctx);

  apOutput->WriteRaw((ni::tPtr)hmac_hash, sizeof(hmac_hash));
  return apOutput;
}
