#include "../API/niLang/ICrypto.h"
#ifndef IDLC_BEGIN_NAMESPACE
#error "IDLC_BEGIN_NAMESPACE not defined !"
#endif
#ifndef IDLC_END_NAMESPACE
#error "IDLC_END_NAMESPACE not defined !"
#endif
IDLC_BEGIN_NAMESPACE()
/** NAMESPACE : ni **/
/** interface : iCryptoRand **/
/** iCryptoRand -> MIN FEATURES '15' **/
#if niMinFeatures(15)
IDLC_BEGIN_INTF(ni,iCryptoRand)
/** ni -> iCryptoRand::SetEntropyLength/1 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCryptoRand::SetEntropyLength/1 **/
IDLC_METH_BEGIN(ni,iCryptoRand,SetEntropyLength,1)
	IDLC_DECL_VAR(tSize,anEntropyLen)
	IDLC_BUF_TO_BASE(ni::eType_Size,anEntropyLen)
	IDLC_METH_CALL_VOID(ni,iCryptoRand,SetEntropyLength,1,(anEntropyLen))
IDLC_METH_END(ni,iCryptoRand,SetEntropyLength,1)
#endif // niMinFeatures(15)

/** ni -> iCryptoRand::SetReseedInterval/1 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCryptoRand::SetReseedInterval/1 **/
IDLC_METH_BEGIN(ni,iCryptoRand,SetReseedInterval,1)
	IDLC_DECL_VAR(tInt,anInterval)
	IDLC_BUF_TO_BASE(ni::eType_Int,anInterval)
	IDLC_METH_CALL_VOID(ni,iCryptoRand,SetReseedInterval,1,(anInterval))
IDLC_METH_END(ni,iCryptoRand,SetReseedInterval,1)
#endif // niMinFeatures(15)

/** ni -> iCryptoRand::SetPredictionResistance/1 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCryptoRand::SetPredictionResistance/1 **/
IDLC_METH_BEGIN(ni,iCryptoRand,SetPredictionResistance,1)
	IDLC_DECL_VAR(tBool,abPR)
	IDLC_BUF_TO_BASE(ni::eType_I8,abPR)
	IDLC_METH_CALL_VOID(ni,iCryptoRand,SetPredictionResistance,1,(abPR))
IDLC_METH_END(ni,iCryptoRand,SetPredictionResistance,1)
#endif // niMinFeatures(15)

/** ni -> iCryptoRand::Reseed/2 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCryptoRand::Reseed/2 **/
IDLC_METH_BEGIN(ni,iCryptoRand,Reseed,2)
	IDLC_DECL_VAR(iFile*,apFile)
	IDLC_BUF_TO_INTF(iFile,apFile)
	IDLC_DECL_VAR(tSize,anAddSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,anAddSize)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCryptoRand,Reseed,2,(apFile,anAddSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iCryptoRand,Reseed,2)
#endif // niMinFeatures(15)

/** ni -> iCryptoRand::ReseedRaw/2 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCryptoRand::ReseedRaw/2 **/
IDLC_METH_BEGIN(ni,iCryptoRand,ReseedRaw,2)
	IDLC_DECL_VAR(tPtr,apData)
	IDLC_BUF_TO_PTR(ni::eTypeFlags_Constant|ni::eType_Ptr,apData)
	IDLC_DECL_VAR(tSize,anAddSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,anAddSize)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCryptoRand,ReseedRaw,2,(apData,anAddSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iCryptoRand,ReseedRaw,2)
#endif // niMinFeatures(15)

/** ni -> iCryptoRand::Update/2 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCryptoRand::Update/2 **/
IDLC_METH_BEGIN(ni,iCryptoRand,Update,2)
	IDLC_DECL_VAR(iFile*,apFile)
	IDLC_BUF_TO_INTF(iFile,apFile)
	IDLC_DECL_VAR(tSize,anAddSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,anAddSize)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCryptoRand,Update,2,(apFile,anAddSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iCryptoRand,Update,2)
#endif // niMinFeatures(15)

/** ni -> iCryptoRand::UpdateRaw/2 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCryptoRand::UpdateRaw/2 **/
IDLC_METH_BEGIN(ni,iCryptoRand,UpdateRaw,2)
	IDLC_DECL_VAR(tPtr,apAdd)
	IDLC_BUF_TO_PTR(ni::eTypeFlags_Constant|ni::eType_Ptr,apAdd)
	IDLC_DECL_VAR(tSize,anAddSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,anAddSize)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCryptoRand,UpdateRaw,2,(apAdd,anAddSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iCryptoRand,UpdateRaw,2)
#endif // niMinFeatures(15)

/** ni -> iCryptoRand::RandFile/2 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCryptoRand::RandFile/2 **/
IDLC_METH_BEGIN(ni,iCryptoRand,RandFile,2)
	IDLC_DECL_VAR(iFile*,apFile)
	IDLC_BUF_TO_INTF(iFile,apFile)
	IDLC_DECL_VAR(tSize,anOutLen)
	IDLC_BUF_TO_BASE(ni::eType_Size,anOutLen)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCryptoRand,RandFile,2,(apFile,anOutLen))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iCryptoRand,RandFile,2)
#endif // niMinFeatures(15)

/** ni -> iCryptoRand::RandRaw/2 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCryptoRand::RandRaw/2 **/
IDLC_METH_BEGIN(ni,iCryptoRand,RandRaw,2)
	IDLC_DECL_VAR(tPtr,apOut)
	IDLC_BUF_TO_PTR(ni::eType_Ptr,apOut)
	IDLC_DECL_VAR(tSize,anOutLen)
	IDLC_BUF_TO_BASE(ni::eType_Size,anOutLen)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCryptoRand,RandRaw,2,(apOut,anOutLen))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iCryptoRand,RandRaw,2)
#endif // niMinFeatures(15)

/** ni -> iCryptoRand::RandInt/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCryptoRand::RandInt/0 **/
IDLC_METH_BEGIN(ni,iCryptoRand,RandInt,0)
	IDLC_DECL_RETVAR(tI64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCryptoRand,RandInt,0,())
	IDLC_RET_FROM_BASE(ni::eType_I64,_Ret)
IDLC_METH_END(ni,iCryptoRand,RandInt,0)
#endif // niMinFeatures(15)

/** ni -> iCryptoRand::RandFloat/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCryptoRand::RandFloat/0 **/
IDLC_METH_BEGIN(ni,iCryptoRand,RandFloat,0)
	IDLC_DECL_RETVAR(tF64,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCryptoRand,RandFloat,0,())
	IDLC_RET_FROM_BASE(ni::eType_F64,_Ret)
IDLC_METH_END(ni,iCryptoRand,RandFloat,0)
#endif // niMinFeatures(15)

/** ni -> iCryptoRand::RandUUID/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCryptoRand::RandUUID/0 **/
IDLC_METH_BEGIN(ni,iCryptoRand,RandUUID,0)
	IDLC_DECL_RETVAR(tUUID,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCryptoRand,RandUUID,0,())
	IDLC_RET_FROM_BASE(ni::eType_UUID,_Ret)
IDLC_METH_END(ni,iCryptoRand,RandUUID,0)
#endif // niMinFeatures(15)

IDLC_END_INTF(ni,iCryptoRand)

#endif // if niMinFeatures(15)
/** interface : iCrypto **/
/** iCrypto -> MIN FEATURES '15' **/
#if niMinFeatures(15)
IDLC_BEGIN_INTF(ni,iCrypto)
/** ni -> iCrypto::Digest/3 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCrypto::Digest/3 **/
IDLC_METH_BEGIN(ni,iCrypto,Digest,3)
	IDLC_DECL_VAR(achar*,aaszData)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aaszData)
	IDLC_DECL_VAR(achar*,aType)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aType)
	IDLC_DECL_VAR(eRawToStringEncoding,aEncoding)
	IDLC_BUF_TO_ENUM(eRawToStringEncoding,aEncoding)
	IDLC_DECL_RETVAR(cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCrypto,Digest,3,(aaszData,aType,aEncoding))
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iCrypto,Digest,3)
#endif // niMinFeatures(15)

/** ni -> iCrypto::DigestRaw/4 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCrypto::DigestRaw/4 **/
IDLC_METH_BEGIN(ni,iCrypto,DigestRaw,4)
	IDLC_DECL_VAR(tPtr,apData)
	IDLC_BUF_TO_PTR(ni::eTypeFlags_Constant|ni::eType_Ptr,apData)
	IDLC_DECL_VAR(tSize,anSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,anSize)
	IDLC_DECL_VAR(achar*,aType)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aType)
	IDLC_DECL_VAR(eRawToStringEncoding,aEncoding)
	IDLC_BUF_TO_ENUM(eRawToStringEncoding,aEncoding)
	IDLC_DECL_RETVAR(cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCrypto,DigestRaw,4,(apData,anSize,aType,aEncoding))
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iCrypto,DigestRaw,4)
#endif // niMinFeatures(15)

/** ni -> iCrypto::DigestFile/4 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCrypto::DigestFile/4 **/
IDLC_METH_BEGIN(ni,iCrypto,DigestFile,4)
	IDLC_DECL_VAR(iFile*,apFile)
	IDLC_BUF_TO_INTF(iFile,apFile)
	IDLC_DECL_VAR(tSize,anSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,anSize)
	IDLC_DECL_VAR(achar*,aType)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aType)
	IDLC_DECL_VAR(eRawToStringEncoding,aEncoding)
	IDLC_BUF_TO_ENUM(eRawToStringEncoding,aEncoding)
	IDLC_DECL_RETVAR(cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCrypto,DigestFile,4,(apFile,anSize,aType,aEncoding))
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iCrypto,DigestFile,4)
#endif // niMinFeatures(15)

/** ni -> iCrypto::CreateRand/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCrypto::CreateRand/0 **/
IDLC_METH_BEGIN(ni,iCrypto,CreateRand,0)
	IDLC_DECL_RETVAR(iCryptoRand*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCrypto,CreateRand,0,())
	IDLC_RET_FROM_INTF(iCryptoRand,_Ret)
IDLC_METH_END(ni,iCrypto,CreateRand,0)
#endif // niMinFeatures(15)

/** ni -> iCrypto::GetRand/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCrypto::GetRand/0 **/
IDLC_METH_BEGIN(ni,iCrypto,GetRand,0)
	IDLC_DECL_RETVAR(iCryptoRand*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCrypto,GetRand,0,())
	IDLC_RET_FROM_INTF(iCryptoRand,_Ret)
IDLC_METH_END(ni,iCrypto,GetRand,0)
#endif // niMinFeatures(15)

/** ni -> iCrypto::KDFGenSaltBlowfish/2 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCrypto::KDFGenSaltBlowfish/2 **/
IDLC_METH_BEGIN(ni,iCrypto,KDFGenSaltBlowfish,2)
	IDLC_DECL_VAR(iCryptoRand*,apRand)
	IDLC_BUF_TO_INTF(iCryptoRand,apRand)
	IDLC_DECL_VAR(tU32,aRounds)
	IDLC_BUF_TO_BASE(ni::eType_U32,aRounds)
	IDLC_DECL_RETVAR(cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCrypto,KDFGenSaltBlowfish,2,(apRand,aRounds))
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iCrypto,KDFGenSaltBlowfish,2)
#endif // niMinFeatures(15)

/** ni -> iCrypto::KDFCrypt/2 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCrypto::KDFCrypt/2 **/
IDLC_METH_BEGIN(ni,iCrypto,KDFCrypt,2)
	IDLC_DECL_VAR(achar*,aKey)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aKey)
	IDLC_DECL_VAR(achar*,aSalt)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aSalt)
	IDLC_DECL_RETVAR(cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCrypto,KDFCrypt,2,(aKey,aSalt))
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iCrypto,KDFCrypt,2)
#endif // niMinFeatures(15)

/** ni -> iCrypto::SigVerify/3 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCrypto::SigVerify/3 **/
IDLC_METH_BEGIN(ni,iCrypto,SigVerify,3)
	IDLC_DECL_VAR(achar*,signatureHex)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,signatureHex)
	IDLC_DECL_VAR(achar*,publicKeyPEM)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,publicKeyPEM)
	IDLC_DECL_VAR(achar*,payload)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,payload)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCrypto,SigVerify,3,(signatureHex,publicKeyPEM,payload))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iCrypto,SigVerify,3)
#endif // niMinFeatures(15)

/** ni -> iCrypto::HmacSignature/6 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCrypto::HmacSignature/6 **/
IDLC_METH_BEGIN(ni,iCrypto,HmacSignature,6)
	IDLC_DECL_VAR(achar*,aAlgo)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aAlgo)
	IDLC_DECL_VAR(ni::iFile*,apOutput)
	IDLC_BUF_TO_INTF(ni::iFile,apOutput)
	IDLC_DECL_VAR(ni::iFile*,apPayload)
	IDLC_BUF_TO_INTF(ni::iFile,apPayload)
	IDLC_DECL_VAR(ni::tSize,aPayloadSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,aPayloadSize)
	IDLC_DECL_VAR(achar*,aSecret)
	IDLC_BUF_TO_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,aSecret)
	IDLC_DECL_VAR(ni::eRawToStringEncoding,aSecretFormat)
	IDLC_BUF_TO_ENUM(ni::eRawToStringEncoding,aSecretFormat)
	IDLC_DECL_RETVAR(ni::iFile*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCrypto,HmacSignature,6,(aAlgo,apOutput,apPayload,aPayloadSize,aSecret,aSecretFormat))
	IDLC_RET_FROM_INTF(ni::iFile,_Ret)
IDLC_METH_END(ni,iCrypto,HmacSignature,6)
#endif // niMinFeatures(15)

IDLC_END_INTF(ni,iCrypto)

#endif // if niMinFeatures(15)
/** interface : iCryptoHash **/
/** iCryptoHash -> MIN FEATURES '15' **/
#if niMinFeatures(15)
IDLC_BEGIN_INTF(ni,iCryptoHash)
/** ni -> iCryptoHash::GetHashType/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCryptoHash::GetHashType/0 **/
IDLC_METH_BEGIN(ni,iCryptoHash,GetHashType,0)
	IDLC_DECL_RETVAR(const achar*,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCryptoHash,GetHashType,0,())
	IDLC_RET_FROM_BASE(ni::eTypeFlags_Constant|ni::eType_AChar|ni::eTypeFlags_Pointer,_Ret)
IDLC_METH_END(ni,iCryptoHash,GetHashType,0)
#endif // niMinFeatures(15)

/** ni -> iCryptoHash::GetDigestSize/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCryptoHash::GetDigestSize/0 **/
IDLC_METH_BEGIN(ni,iCryptoHash,GetDigestSize,0)
	IDLC_DECL_RETVAR(tU32,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCryptoHash,GetDigestSize,0,())
	IDLC_RET_FROM_BASE(ni::eType_U32,_Ret)
IDLC_METH_END(ni,iCryptoHash,GetDigestSize,0)
#endif // niMinFeatures(15)

/** ni -> iCryptoHash::Restart/0 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCryptoHash::Restart/0 **/
IDLC_METH_BEGIN(ni,iCryptoHash,Restart,0)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCryptoHash,Restart,0,())
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iCryptoHash,Restart,0)
#endif // niMinFeatures(15)

/** ni -> iCryptoHash::Update/2 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCryptoHash::Update/2 **/
IDLC_METH_BEGIN(ni,iCryptoHash,Update,2)
	IDLC_DECL_VAR(iFile*,apFile)
	IDLC_BUF_TO_INTF(iFile,apFile)
	IDLC_DECL_VAR(tI64,aSize)
	IDLC_BUF_TO_BASE(ni::eType_I64,aSize)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCryptoHash,Update,2,(apFile,aSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iCryptoHash,Update,2)
#endif // niMinFeatures(15)

/** ni -> iCryptoHash::UpdateBlock/3 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCryptoHash::UpdateBlock/3 **/
IDLC_METH_BEGIN(ni,iCryptoHash,UpdateBlock,3)
	IDLC_DECL_VAR(iFile*,apFile)
	IDLC_BUF_TO_INTF(iFile,apFile)
	IDLC_DECL_VAR(tI64,aStart)
	IDLC_BUF_TO_BASE(ni::eType_I64,aStart)
	IDLC_DECL_VAR(tI64,aSize)
	IDLC_BUF_TO_BASE(ni::eType_I64,aSize)
	IDLC_DECL_RETVAR(tBool,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCryptoHash,UpdateBlock,3,(apFile,aStart,aSize))
	IDLC_RET_FROM_BASE(ni::eType_I8,_Ret)
IDLC_METH_END(ni,iCryptoHash,UpdateBlock,3)
#endif // niMinFeatures(15)

/** ni -> iCryptoHash::UpdateRaw/2 -> NO AUTOMATION **/
/** ni -> iCryptoHash::FinalString/1 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCryptoHash::FinalString/1 **/
IDLC_METH_BEGIN(ni,iCryptoHash,FinalString,1)
	IDLC_DECL_VAR(eRawToStringEncoding,aEncoding)
	IDLC_BUF_TO_ENUM(eRawToStringEncoding,aEncoding)
	IDLC_DECL_RETVAR(cString,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCryptoHash,FinalString,1,(aEncoding))
	IDLC_RET_FROM_BASE(ni::eType_String,_Ret)
IDLC_METH_END(ni,iCryptoHash,FinalString,1)
#endif // niMinFeatures(15)

/** ni -> iCryptoHash::FinalFile/1 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCryptoHash::FinalFile/1 **/
IDLC_METH_BEGIN(ni,iCryptoHash,FinalFile,1)
	IDLC_DECL_VAR(iFile*,apOutput)
	IDLC_BUF_TO_INTF(iFile,apOutput)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCryptoHash,FinalFile,1,(apOutput))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iCryptoHash,FinalFile,1)
#endif // niMinFeatures(15)

/** ni -> iCryptoHash::FinalRaw/2 -> MIN FEATURES '15' **/
#if niMinFeatures(15)
/** ni -> iCryptoHash::FinalRaw/2 **/
IDLC_METH_BEGIN(ni,iCryptoHash,FinalRaw,2)
	IDLC_DECL_VAR(tPtr,apData)
	IDLC_BUF_TO_PTR(ni::eType_Ptr,apData)
	IDLC_DECL_VAR(tSize,anSize)
	IDLC_BUF_TO_BASE(ni::eType_Size,anSize)
	IDLC_DECL_RETVAR(tSize,_Ret)
	IDLC_METH_CALL(_Ret,ni,iCryptoHash,FinalRaw,2,(apData,anSize))
	IDLC_RET_FROM_PTR(ni::eType_Size,_Ret)
IDLC_METH_END(ni,iCryptoHash,FinalRaw,2)
#endif // niMinFeatures(15)

IDLC_END_INTF(ni,iCryptoHash)

#endif // if niMinFeatures(15)
IDLC_END_NAMESPACE()
// EOF //
