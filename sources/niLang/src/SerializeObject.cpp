// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/Types.h"
#include "API/niLang/ObjModel.h"
#include "API/niLang/IFile.h"
#include "API/niLang/ILang.h"
#include "API/niLang/Var.h"
#include "API/niLang/StringDef.h"
#include "Lang.h"

using namespace ni;

///////////////////////////////////////////////
struct sStringIDSerializer : public ImplLocal<iSerializable>
{
  sStringIDSerializer(cString& aStringID) : mStringID(aStringID) {}

  const achar* __stdcall GetSerializeObjectTypeID() const { return NULL; }

  //! Serialize the object.
  tSize __stdcall Serialize(iFile* apFile, eSerializeMode aMode) {
    switch(aMode) {
      case eSerializeMode_Read:
      case eSerializeMode_ReadRaw: {
        tI64 pos = apFile->Tell();
        mStringID = apFile->ReadStringEx(eTextEncodingFormat_BitStream);
        return (tSize)(pos-apFile->Tell());
      }
      case eSerializeMode_Write:
      case eSerializeMode_WriteRaw: {
        return apFile->WriteStringZEx(eTextEncodingFormat_BitStream,mStringID.Chars());
      }
      default:
        break;
    }
    return eInvalidHandle;
  }

  cString& mStringID;
};

static iUnknown* _CreateObjectInstance(const achar* aOID, const Var& aVarA, const Var& aVarB) {
  return ni::GetLang()->CreateInstance(aOID,aVarA,aVarB);
}

///////////////////////////////////////////////
tBool __stdcall cLang::IsSerializedObject(iFile* apFile, const achar* aID, tI64* apObjSize)
{
  niCheck(niIsOK(apFile),eFalse);

  tBool bRes = eFalse;
  tI64 nPos = apFile->Tell();
  if (apFile->ReadLE32() == kfccSerializedObject) {
    apFile->BeginReadBits();
    if (apFile->ReadBit()) {
      cString id = AZEROSTR;
      if (sStringIDSerializer(id).Serialize(apFile,eSerializeMode_ReadRaw) != eInvalidHandle) {
        if (id.IEq(aID))
          bRes = eTrue;
      }
    }
    tI64 nSize = apFile->ReadBitsPackedU32();
    if (apObjSize) *apObjSize = nSize;
    apFile->EndReadBits();
  }

  apFile->SeekSet(nPos);
  return bRes;
}

///////////////////////////////////////////////
tI64 __stdcall cLang::ReadSerializationHeader(iFile* apFile, cString* apID, tI64* apObjSize)
{
  niCheck(niIsOK(apFile),eInvalidHandle);

  cString id;
  if (apID == NULL)
    apID = &id;

  if (apFile->ReadLE32() == kfccSerializedObject)
  {
    tI64 nStartPos = apFile->Tell();
    apFile->BeginReadBits();
    if (apFile->ReadBit()) {
      if (sStringIDSerializer(*apID).Serialize(apFile,eSerializeMode_ReadRaw) == eInvalidHandle) {
        niWarning(_A("Can't read serialized object type ID."));
        return eInvalidHandle;
      }
    }
    tI64 nSize = apFile->ReadBitsPackedU32();
    if (apObjSize) *apObjSize = nSize;
    apFile->EndReadBits();
    return (tI64)(apFile->Tell()-nStartPos);
  }
  else
  {
    return eInvalidHandle;
  }
}

///////////////////////////////////////////////
//! Write serialization header.
tI64 __stdcall cLang::WriteSerializationHeader(iFile* apFile, const achar* apID, tI64 anSize)
{
  niCheck(niIsOK(apFile),eInvalidHandle);

  tI64 nSize = 0;

  nSize += apFile->WriteLE32(kfccSerializedObject);
  apFile->BeginWriteBits();
  if (apID && niIsStringOK(apID)) {
    apFile->WriteBit(eTrue);
    cString objID = apID;
    if (sStringIDSerializer(objID).Serialize(apFile,eSerializeMode_WriteRaw) == eInvalidHandle) {
      niWarning(_A("Can't serialize the object type ID."));
      return eInvalidHandle;
    }
  }
  else {
    apFile->WriteBit(eFalse);
  }
  apFile->WriteBitsPackedU32(anSize);
  nSize += apFile->EndWriteBits();

  return nSize;
}

///////////////////////////////////////////////
//! Serialize an object.
iUnknown* __stdcall cLang::SerializeObject(iFile* apFile, iUnknown* apObject,
                                             eSerializeMode aMode,
                                             tI64* apSize)
{
  if (!niIsOK(apFile)) {
    niError(_A("Invalid File instance."));
    return NULL;
  }

  switch (aMode) {
    case eSerializeMode_Write:
    case eSerializeMode_WriteRaw: {
      if (apObject == NULL) {
        niError(_A("Invalid object passed."));
        return NULL;
      }

      iSerializable* pSerializable = ni::QueryInterface<iSerializable>(apObject);
      if (pSerializable == NULL) {
        niError(_A("Object doesnt implement iSerializable."));
        return NULL;
      }

      tI64 nStartPos = apFile->Tell();

      if (aMode != eSerializeMode_WriteRaw)
      {
        Ptr<iFile> fpDummy = ni::GetLang()->CreateFileWriteDummy();
        tU32 nObjSize = pSerializable->Serialize(fpDummy,aMode);
        if (nObjSize == eInvalidHandle)
        {
          niError(_A("Can't get the size of the serializable object."));
          return NULL;
        }

        if (WriteSerializationHeader(apFile, pSerializable->GetSerializeObjectTypeID(), nObjSize) == eInvalidHandle)
        {
          niError(_A("Can't write the serialization header."));
          return NULL;
        }
      }

      if (pSerializable->Serialize(apFile,aMode) == eInvalidHandle)
      {
        niError(_A("Can't serialize write the object."));
        return NULL;
      }

      if (apSize)
        *apSize = (tI64)(apFile->Tell()-nStartPos);
      break;
    }
    case eSerializeMode_Read:
    case eSerializeMode_ReadRaw: {
      tI64 nStartPos = apFile->Tell();
      cString objID;
      tI64 nObjSize = 0;
      tI64 nDestPos = nStartPos;

      if (aMode != eSerializeMode_ReadRaw) {
        if (ReadSerializationHeader(apFile,&objID,&nObjSize) == eInvalidHandle) {
          niError(_A("Can't read the serialization header."));
          return NULL;
        }
        nDestPos = apFile->Tell()+nObjSize;
      }

      iSerializable* pSerializable = NULL;
      if (objID.IsEmpty()) {
        if (!apObject) {
          niError(_A("Invalid object passed and no object ID serialized."));
          apFile->SeekSet(nDestPos);
          return NULL;
        }
        pSerializable = ni::QueryInterface<iSerializable>(apObject);
        if (pSerializable == NULL)
        {
          niError(_A("Object doesnt implement iSerializable and no object ID serialized."));
          apFile->SeekSet(nDestPos);
          return NULL;
        }
      }
      else
      {
        if (!apObject) {
          apObject = _CreateObjectInstance(objID.Chars(), niVarNull, niVarNull);
          if (!niIsOK(apObject)) {
            niError(niFmt(_A("Can't create an instance of the object '%s'."), objID));
            apFile->SeekSet(nDestPos);
            return NULL;
          }
        }

        pSerializable = ni::QueryInterface<iSerializable>(apObject);
        if (pSerializable == NULL)
        {
          niError(_A("Object doesnt implement iSerializable."));
          apFile->SeekSet(nDestPos);
          return NULL;
        }
      }

      if (pSerializable->Serialize(apFile,aMode) == eInvalidHandle)
      {
        niError(_A("Can't serialize read the object."));
        apFile->SeekSet(nDestPos);
        return NULL;
      }

      if (apSize)
        *apSize = (tI64)(apFile->Tell()-nStartPos);
      break;
    }
    default:
      break;
  }

  return apObject;
}
