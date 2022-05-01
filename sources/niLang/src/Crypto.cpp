#include "API/niLang/Types.h"
#include "API/niLang/IFile.h"
#include "API/niLang/STL/vector.h"
#include "API/niLang/Utils/UnknownImpl.h"
#include "API/niLang/Utils/QPtr.h"
#include "API/niLang/ICrypto.h"

namespace ni {

struct cCrypto : public iCrypto
{
  niBeginClass(cCrypto);
  niIUnknownImpl_Local(iCrypto);

  //! Compute the hash of the specified data.
  virtual cString __stdcall DigestRaw(const tPtr apData, tSize anSize, const achar* aType, eRawToStringEncoding aEncoding) {
    QPtr<iCryptoHash> hash = New_niLang_CryptoHash(aType,niVarNull);
    niCheckIsOK(hash,AZEROSTR);
    const tU32 digestSize = hash->GetDigestSize();
    hash->UpdateRaw(apData,anSize);
    Ptr<iFile> buffer = ni::CreateFileMemoryAlloc(digestSize, "");
    hash->FinalRaw(buffer->GetBase(), digestSize);
    return buffer->ReadRawToString(aEncoding, digestSize);
  }
  //! Compute the hash of the specified data read from the specified file.
  virtual cString __stdcall DigestFile(iFile* apFile, tSize anSize, const achar* aType, eRawToStringEncoding aEncoding) {
    niCheckIsOK(apFile, AZEROSTR);
    sReadBufferFromFile buffer(apFile,anSize);
    return DigestRaw(buffer.begin(), buffer.size(), aType, aEncoding);
  }
  //! Compute the hash of the specified string. Does not include the end zero.
  virtual cString __stdcall Digest(const achar* aaszData, const achar* aType, eRawToStringEncoding aEncoding) {
    const tSize dataLen = ni::StrSize(aaszData);
    return DigestRaw((tPtr)aaszData, dataLen, aType, aEncoding);
  }

  //! Create a new random number generator.
  virtual iCryptoRand* __stdcall CreateRand() {
    return (iCryptoRand*)New_niLang_CryptoRand(niVarNull,niVarNull);
  }
  //! Get the default crypto random number generator.
  //! {Property}
  virtual iCryptoRand* __stdcall GetRand() {
    return GetCryptoRand();
  }

  //! Generate a salt for KDFCrypt. \see CryptoKDFGenSaltBlowfish
  virtual cString __stdcall KDFGenSaltBlowfish(iCryptoRand* apRand, tU32 aRounds) {
    char output[knCryptoKDFSaltMaxLen];
    if (!apRand) {
      apRand = this->GetRand();
    }
    return CryptoKDFGenSaltBlowfish(apRand, aRounds, output);
  }
  //! Calculates a crypt(3)-style hash of password. \see CryptoKDFCrypt
  virtual cString __stdcall KDFCrypt(const achar* aKey, const achar* aSalt) {
    char output[knCryptoKDFCryptMaxLen];
    return CryptoKDFCrypt(aKey, aSalt, output);
  }

  virtual tBool __stdcall SigVerify(const achar* signatureHex, const achar* publicKeyPEM, const achar* payload) {
    return CryptoSigVerify(signatureHex, publicKeyPEM, payload);
  }

  virtual ni::iFile* __stdcall HmacSignature(
    const char* aAlgo, ni::iFile* apOutput,
    ni::iFile* apPayload, ni::tSize aPayloadSize,
    const char* aSecret, ni::eRawToStringEncoding aSecretFormat) niImpl
  {
    return CryptoHmacSignature(aAlgo, apOutput, apPayload, aPayloadSize, aSecret, aSecretFormat);
  }
};

niExportFunc(iCrypto*) GetCrypto() {
  static cCrypto _crypto;
  return &_crypto;
}
niExportFunc(ni::iUnknown*) New_niLang_Crypto(const ni::Var&,const ni::Var&) {
  return GetCrypto();
}

}
