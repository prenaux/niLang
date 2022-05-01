#pragma once
#ifndef __DATATABLEUTILS_H_0CC136B9_4819_4DE4_86F1_40C26D7E5831__
#define __DATATABLEUTILS_H_0CC136B9_4819_4DE4_86F1_40C26D7E5831__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../IDataTable.h"
#include "../IZip.h"

namespace ni {

/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

///////////////////////////////////////////////
inline Ptr<iDataTable> LoadDataTable(const achar* aURL, const achar* aaszSerFormat = "xml") {
  Ptr<iFile> fp = ni::GetLang()->URLOpen(aURL);
  niCheckIsOK(fp,NULL);
  Ptr<iDataTable> dt = ni::GetLang()->CreateDataTable("");
  if (!ni::GetLang()->SerializeDataTable(aaszSerFormat,eSerializeMode_Read,dt,fp)) {
    niError(niFmt("Can't read datatable from '%s'.", aURL));
    return NULL;
  }
  return dt;
}

///////////////////////////////////////////////
inline tBool WriteDataTable(const iDataTable* apDT, const achar* aPath, const achar* aaszSerFormat = "xml") {
  niCheckIsOK(apDT,eFalse);
  Ptr<iFile> fp = ni::GetRootFS()->FileOpen(aPath,eFileOpenMode_Write);
  if (!fp.IsOK()) {
    niError(niFmt("Can't open file '%s' to write datatable.", aPath));
    return eFalse;
  }
  if (!ni::GetLang()->SerializeDataTable(aaszSerFormat,eSerializeMode_Write,(iDataTable*)apDT,fp)) {
    niError(niFmt("Can't write datatable to '%s'.", aPath));
    return eFalse;
  }
  return eTrue;
}

///////////////////////////////////////////////
inline Ptr<iDataTable> CreateDataTableFromXML(const achar* aaszText, tI32 anSize = -1) {
  if (anSize < 0) {
    anSize = ni::StrSizeZ(aaszText);
  }
  Ptr<iFile> fp = ni::GetLang()->CreateFileMemory((tPtr)aaszText,anSize,eFalse,"::CreateDataTableFromXML");
  Ptr<iDataTable> dt = ni::GetLang()->CreateDataTable("");
  if (!ni::GetLang()->SerializeDataTable("xml",eSerializeMode_Read,dt,fp))
    return NULL;
  return dt;
}

///////////////////////////////////////////////
inline cString DataTableToXML(iDataTable* apDT, tBool abStream = eTrue) {
  niCheckIsOK(apDT,"");
  Ptr<iFile> fp = ni::GetLang()->CreateFileDynamicMemory();
  if (!ni::GetLang()->SerializeDataTable(abStream ? "xml-stream" : "xml",eSerializeMode_Write,apDT,fp))
    return AZEROSTR;
  fp->SeekSet(0);
  return fp->ReadString();
}

///////////////////////////////////////////////
static const char* _kDataTableCompressedStringMode = "xml-stream";

///////////////////////////////////////////////
inline cString DataTableToCompressedString(iDataTable* apDT) {
  niCheckIsOK(apDT,"");
  Ptr<iFile> fp = ni::GetLang()->CreateFileDynamicMemory();

  {
    Ptr<iFile> fpXml = ni::GetLang()->CreateFileDynamicMemory();
    if (!ni::GetLang()->SerializeDataTable(_kDataTableCompressedStringMode,eSerializeMode_Write,apDT,fpXml))
      return AZEROSTR;
    fpXml->SeekSet(0);
    fp->WriteLE32(0);
    if (!ni::GetZip()->ZipCompressFile(fp,fpXml,(tU32)fpXml->GetSize(),9))
      return AZEROSTR;
    *((tU32*)fp->GetBase()) = (tU32)fpXml->GetSize();
    fp->SeekSet(0);
  }

  return fp->ReadRawToString(eRawToStringEncoding_Base64, (tInt)fp->GetSize());
}

///////////////////////////////////////////////
inline Ptr<iDataTable> CreateDataTableFromCompressedString(const achar* aaszText) {
  Ptr<iFile> fpDecodedBytes = ni::GetLang()->CreateFileDynamicMemory();
  fpDecodedBytes->WriteRawFromString(eRawToStringEncoding_Base64, aaszText);

  Ptr<iFile> fp = ni::GetLang()->CreateFileDynamicMemory();
  {
    tPtr output = fpDecodedBytes->GetBase();
    tU32 decompressedSize = *(tU32*)output;
    if (!ni::GetZip()->ZipUncompressBufferInFile(fp,decompressedSize,output+4,(tU32)fpDecodedBytes->GetSize()-4))
      return NULL;
    fp->SeekSet(0);
  }

  Ptr<iDataTable> dt = ni::GetLang()->CreateDataTable("");
  if (!ni::GetLang()->SerializeDataTable(_kDataTableCompressedStringMode,eSerializeMode_Read,dt,fp))
    return NULL;

  return dt;
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __DATATABLEUTILS_H_0CC136B9_4819_4DE4_86F1_40C26D7E5831__
