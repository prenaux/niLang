// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/Utils/ObjectInterfaceCast.h"
#include "API/niLang/Utils/SmartPtr.h"
#include "API/niLang/Utils/UTFImpl.h"
#include "API/niLang/Utils/Path.h"
#include "API/niLang/STL/stack.h"
#include "API/niLang/IFile.h"
#include "API/niLang/Var.h"
#include "API/niLang/ILang.h"
#include "API/niLang/IZip.h"
#include "Lang.h"
#include "DataTable.h"

using namespace ni;

static const achar* kXMLCDataProperty = "_data";
static const achar* kXMLInnerTextProperty = "_inner";
static const ni::tU32 _knZipCompressionMode = 6;

//--------------------------------------------------------------------------------------------
//
//  Binary serialization
//
//--------------------------------------------------------------------------------------------

typedef tSize (*tpfnReadDataTable)(iFile*, iDataTable*);
typedef tSize (*tpfnWriteDataTable)(iFile*, iDataTable*);

///////////////////////////////////////////////
static tSize _WriteDataTableRaw(iFile* apFile, iDataTable* apTable)
{
  tI64 pos = apFile->Tell();
  apFile->WriteLE32(knDataTableSerializeVersionRaw);
  apFile->WriteStringZ(apTable->GetName());
  // was the DataTable's metadata, kept for compatibility
  apFile->WriteStringZ(AZEROSTR);
  apFile->WriteLE32(apTable->GetNumProperties());
  tU32 i;
  for (i = 0; i < apTable->GetNumProperties(); ++i) {
    const achar* aszName = apTable->GetPropertyName(i);
    apFile->WriteStringZ(aszName);
    if (!VarSerializeWriteRaw(apFile,apTable->GetVarFromIndex(i))) {
      niError(niFmt(_A("Can't write property '%s'."),aszName));
      return eInvalidHandle;
    }
    apFile->WriteStringZ(HStringGetStringEmpty(apTable->GetMetadataFromIndex(i)));
  }
  apFile->WriteLE32(apTable->GetNumChildren());

  for (i = 0; i < apTable->GetNumChildren(); ++i) {
    if (_WriteDataTableRaw(apFile,apTable->GetChildFromIndex(i)) == eInvalidHandle) {
      niError(niFmt(_A("Can't write child data table '%s' (%d)."),apTable->GetChildFromIndex(i)->GetName(),i));
      return eInvalidHandle;
    }
  }

  return (tSize)(apFile->Tell()-pos);
}

///////////////////////////////////////////////
static tSize _ReadDataTableRaw(iFile* apFile, iDataTable* apTable)
{
  tI64 nPos = apFile->Tell();
  tVersion ver = apFile->ReadLE32();
  if (ver != knDataTableSerializeVersionRaw) {
    niError(niFmt(_A("Invalid binary data table version '%s', expected '%s'."),niFmtVersion(ver),niFmtVersion(knDataTableSerializeVersionRaw)));
    return eInvalidHandle;
  }

  apTable->SetName(apFile->ReadString().Chars());
  // was the DataTable's metadata, kept for compatibility
  apFile->ReadString();
  tU32 nNumProps = apFile->ReadLE32();
  tU32 i;
  for (i = 0; i < nNumProps; ++i) {
    cString strName = apFile->ReadString();
    Var v;
    if (!VarSerializeReadRaw(apFile,v)) {
      niError(niFmt(_A("Can't read property '%s'."),strName.Chars()));
      return eInvalidHandle;
    }
    tU32 nIndex = apTable->SetVar(strName.Chars(),v);
    apTable->SetMetadataFromIndex(nIndex,_H(apFile->ReadString().Chars()));
  }

  tU32 nNumChildrenTables = apFile->ReadLE32();
  for (i = 0; i < nNumChildrenTables; ++i) {
    Ptr<iDataTable> ptrDT = ni::CreateDataTable();
    if (_ReadDataTableRaw(apFile,ptrDT) == eInvalidHandle) {
      niError(niFmt(_A("Can't read child data table '%s' (%d)."),ptrDT->GetName(),i));
      return eInvalidHandle;
    }
    apTable->AddChild(ptrDT);
  }

  return (tSize)(apFile->Tell()-nPos);
}

///////////////////////////////////////////////
static tSize _WriteDataTableRawZip(iFile* apFile, iDataTable* apTable) {
  tI64 pos = apFile->Tell();
  apFile->WriteLE32(knDataTableSerializeVersionRawZip);
  Ptr<iFile> fpZip = ni::GetZip()->CreateFileZipBufferEncoder(apFile->GetFileBase(),_knZipCompressionMode);
  if (_WriteDataTableRaw(fpZip,apTable) == eInvalidHandle) {
    niError(_A("Can't write datatable into zip buffer."));
    return eInvalidHandle;
  }
  return (tSize)(apFile->Tell()-pos);
}

///////////////////////////////////////////////
static tSize _ReadDataTableRawZip(iFile* apFile, iDataTable* apTable)
{
  tI64 nPos = apFile->Tell();
  tVersion ver = apFile->ReadLE32();
  if (ver != knDataTableSerializeVersionRawZip) {
    niError(niFmt(_A("Invalid binary data table version '%s', expected '%s'."),niFmtVersion(ver),niFmtVersion(knDataTableSerializeVersionRawZip)));
    return eInvalidHandle;
  }
  Ptr<iFile> fpZip = ni::GetZip()->CreateFileZipBufferDecoder(apFile->GetFileBase(),2048);
  if (_ReadDataTableRaw(fpZip,apTable) == eInvalidHandle) {
    niError(_A("Can't read datatable from zip buffer."));
    return eInvalidHandle;
  }
  return (tSize)(apFile->Tell()-nPos);
}

//--------------------------------------------------------------------------------------------
//
//
//
//--------------------------------------------------------------------------------------------

struct sDataTableSerializeVersion {
  const achar* const  ext;
  const tU32      ver;
  tpfnWriteDataTable  write;
  tpfnReadDataTable read;
};
static const sDataTableSerializeVersion _Vers[] = {
  { _A("dtb"),  knDataTableSerializeVersionRaw,     _WriteDataTableRaw,     _ReadDataTableRaw     },
  { _A("dtz"),  knDataTableSerializeVersionRawZip,  _WriteDataTableRawZip,  _ReadDataTableRawZip  },
};

///////////////////////////////////////////////
ni::tU32 DataTableSerialize_GetExtVer(const ni::achar* aaszExt) {
  niLoop(i,niCountOf(_Vers)) {
    if (ni::StrEq(aaszExt,_Vers[i].ext))
      return _Vers[i].ver;
  }
  return 0;
}
const ni::achar* DataTableSerialize_GetVerExt(const ni::tU32 anVer) {
  niLoop(i,niCountOf(_Vers)) {
    if (anVer == _Vers[i].ver)
      return _Vers[i].ext;
  }
  return NULL;
}
static const sDataTableSerializeVersion* DataTableSerialize_GetVer(const ni::tU32 anVer) {
  niLoop(i,niCountOf(_Vers)) {
    if (anVer == _Vers[i].ver)
      return &_Vers[i];
  }
  return NULL;
}

///////////////////////////////////////////////
tSize DataTableSerialize_Write(iFile* apFile, iDataTable* apTable, const tU32 anVersion) {
  tSize ret = eInvalidHandle;

  const sDataTableSerializeVersion* v = DataTableSerialize_GetVer(anVersion);
  if (!v) {
    niError(niFmt(_A("Invalid binary data table version '%s'."),niFmtVersion(anVersion)));
    return eInvalidHandle;
  }
  apFile->WriteLE32(anVersion); // write the header once, this allows us to read
  // without needing to seek back at the header
  ret = v->write(apFile,apTable);
  if (ret == eInvalidHandle) {
    niError(niFmt(_A("Can't write datatable with version '%s'."),niFmtVersion(anVersion)));
    return eInvalidHandle;
  }
  return ret;
}

///////////////////////////////////////////////
tSize DataTableSerialize_Read(iFile* apFile, iDataTable* apTable)
{
  tVersion ver = apFile->ReadLE32();
  const sDataTableSerializeVersion* v = DataTableSerialize_GetVer(ver);
  if (!v) {
    niError(niFmt(_A("Invalid binary data table version '%s'."),niFmtVersion(ver)));
    return eInvalidHandle;
  }
  tSize ret = v->read(apFile,apTable);
  if (ret == eInvalidHandle) {
    niError(niFmt(_A("Can't read datatable with version '%s'."),niFmtVersion(ver)));
    return eInvalidHandle;
  }
  return ret;
}

//--------------------------------------------------------------------------------------------
//
//  Read/Write XML
//
//--------------------------------------------------------------------------------------------
#if niMinFeatures(20)

static const achar* const _kaszXMLIndent = _A("\t");
// static const achar* const _kaszXMLIndent = _A(" ");

struct sXMLSpecialChar {
  const achar theChar;
  const achar* theStr;
  tSize len;
};
static sXMLSpecialChar _kXMLSpecialChars[] = {
  {'&', _A("amp;"),4},
  {'<', _A("lt;"),3},
  {'>', _A("gt;"),3},
  {'"', _A("quot;"),5},
  {'\'',_A("apos;"),5}
};

///////////////////////////////////////////////
static tSize _XMLWriteString(iFile* apFile, const achar* p)
{
  tSize w = 0;
  for (;;) {
    tU32 c = ni::utf8::raw_next(p);
    if (!c) break;
    switch (c) {
      case '<':
        w += apFile->WriteString(_A("&lt;"));
        break;
      case '>':
        w += apFile->WriteString(_A("&gt;"));
        break;
      case '&':
        w += apFile->WriteString(_A("&amp;"));
        break;
      case '"':
        w += apFile->WriteString(_A("&quot;"));
        break;
      case '\'':
        w += apFile->WriteString(_A("&apos;"));
        break;
      case '\r':
      case '\n':
        w += apFile->WriteString(niFmt(_A("&#x%X;"),c));
        break;
      default:
        w += apFile->WriteChar(c);
        break;
    }
  }
  return w;
}

static tSize __forceinline _XMLWriteIndent(iFile* apFile, int indent) {
  tSize w = 0;
  if (indent >= 0) {
    w += apFile->WriteString(_A("\n"));
    niLoop(i,indent) {
      w += apFile->WriteString(_kaszXMLIndent);
    }
  }
  return w;
}

///////////////////////////////////////////////
// indent == -1 to disable indentation
static tSize DataTableSerialize_WriteXML(iFile* apFile, iDataTable* apTable, const int indent)
{
  const int nextIndent = (indent>=0)?(indent+1):(-1);
  tSize w = 0;
  if (apTable->GetParent()) {
    w += _XMLWriteIndent(apFile,indent);
  }
  w += apFile->WriteString(_A("<"));
  w += apFile->WriteString(apTable->GetName());
  tU32 innerIndex = eInvalidHandle;
  tU32 cdataIndex = eInvalidHandle;
  const tBool hasChildren = (apTable->GetNumChildren() > 0);
  const tU32 numProps = apTable->GetNumProperties();
  const tBool propNL = (indent>=0) && (numProps >= 10);
  niLoop(i,numProps) {
    const achar* propName = apTable->GetPropertyName(i);
    if (!niStringIsOK(propName))
      continue;
    if (ni::StrEq(propName,kXMLInnerTextProperty)) {
      innerIndex = i;
      continue;
    }
    else if (ni::StrEq(propName,kXMLCDataProperty)) {
      cdataIndex = i;
      continue;
    }
    else {
      if (propNL) {
        _XMLWriteIndent(apFile,nextIndent);
      }
      else {
        w += apFile->WriteString(_A(" "));
      }
      w += apFile->WriteString(apTable->GetPropertyName(i));
      w += apFile->WriteString(_A("="));
      w += apFile->WriteString(_A("\""));
      w += _XMLWriteString(apFile,apTable->GetStringFromIndex(i).Chars());
      w += apFile->WriteString(_A("\""));
      iHString* hspPropMD = apTable->GetMetadataFromIndex(i);
      if (HStringIsNotEmpty(hspPropMD)) {
        w += apFile->WriteString(_A(" "));
        w += apFile->WriteString(apTable->GetPropertyName(i));
        w += apFile->WriteString(_A("__"));
        w += apFile->WriteString(_A("="));
        w += apFile->WriteString(_A("\""));
        w += _XMLWriteString(apFile,niHStr(hspPropMD));
        w += apFile->WriteString(_A("\""));
      }
    }
  }
  if ((apTable->GetNumChildren() == 0) &&
      cdataIndex == eInvalidHandle &&
      innerIndex == eInvalidHandle)
  {
    w += apFile->WriteString(_A("/>"));
  }
  else
  {
    w += apFile->WriteString(_A(">"));
    niLoop(i,apTable->GetNumChildren()) {
      w += DataTableSerialize_WriteXML(apFile,apTable->GetChildFromIndex(i),nextIndent);
    }
    if (cdataIndex != eInvalidHandle) {
      w += _XMLWriteIndent(apFile,nextIndent);
      w += apFile->WriteString(_A("<![CDATA["));
      w += apFile->WriteString(apTable->GetStringFromIndex(cdataIndex).Chars());
      w += apFile->WriteString(_A("]]>"));
      w += _XMLWriteIndent(apFile,indent);
    }
    else if (innerIndex != eInvalidHandle) {
      if (hasChildren || propNL)
        w += _XMLWriteIndent(apFile,indent);
      w += _XMLWriteString(apFile,apTable->GetStringFromIndex(innerIndex).Chars());
      if (hasChildren || propNL)
        w += _XMLWriteIndent(apFile,indent);
    }
    else if (hasChildren || !apTable->GetParent()) {
      w += _XMLWriteIndent(apFile,indent);
    }
    w += apFile->WriteString(_A("</"));
    w += apFile->WriteString(apTable->GetName());
    w += apFile->WriteString(_A(">"));
  }
  return w;
}

struct  sXMLParserSinkDT : public cIUnknownImpl<iXmlParserSink>
{
  Ptr<iDataTable> _rootDT;
  astl::stack<Ptr<iDataTable> > _dt;
  virtual tBool __stdcall OnXmlParserSink_Node(eXmlParserNodeType type, const ni::achar* aNameOrData) niOverride {
    switch (type) {
      case eXmlParserNodeType_ElementBegin: {
#ifdef XMLPARSER_TRACE
        niDebugFmt((_A("ELEMENT(%d): %s\n"),_dt.size(),aName));
#endif
        if (_dt.empty()) {
          _rootDT->SetName(aNameOrData);
          _dt.push(_rootDT);
        }
        else {
          Ptr<iDataTable> parent = _dt.top();
          Ptr<iDataTable> newDT = ni::GetLang()->CreateDataTable(aNameOrData);
          _dt.push(newDT);
          parent->AddChild(newDT);
        }
        break;
      }
      case eXmlParserNodeType_ElementEnd: {
#ifdef XMLPARSER_TRACE
        cString name(b,e);
        niDebugFmt((_A("ELEMENT-END: %s\n"),aNameOrData));
#endif
        if (_dt.size() > 1) {
          _dt.pop();
        }
        break;
      }
      case eXmlParserNodeType_CDATA: {
#ifdef XMLPARSER_TRACE
        niDebugFmt((_A("CDATA: %s\n"),aName));
#endif
        _dt.top()->SetString(kXMLCDataProperty,aNameOrData);
        break;
      }
      case eXmlParserNodeType_Text: {
        cString txt;
#ifdef XMLPARSER_TRACE
        txt.Set(b,e);
        niDebugFmt((_A("TEXT: %s\n"),txt.Chars()));
        txt.clear();
#endif
        StringDecodeXml(txt,aNameOrData);
        _dt.top()->SetString(kXMLInnerTextProperty,txt.Chars());
        break;
      }
    }
    return eTrue;
  }
  virtual tBool __stdcall OnXmlParserSink_Attribute(const achar* aName, const achar* aValue) niOverride
  {
    cString val;
    StringDecodeXml(val,aValue);
#ifdef XMLPARSER_TRACE
    {
      cString name(nameB,nameE);
      niDebugFmt((_A("ATTR: %s = %s\n"),name.Chars(),val.Chars()));
    }
#endif
    const char* nameEnd = aName + StrSize(aName);
    if ((nameEnd-aName) >= 2 && StrNCmp(nameEnd-2,_A("__"),2) == 0) {
      cString name(aName,nameEnd-2);
      _dt.top()->SetMetadata(
          name.Chars(),
          _H(val.Chars()));
    }
    else {
      _dt.top()->SetString(aName,val.Chars());
    }
    return eTrue;
  }
};

static tSize DataTableSerialize_ReadXML(iFile* apFile, iDataTable* apTable)
{
  Ptr<sXMLParserSinkDT> sink = niNew sXMLParserSinkDT();
  sink->_rootDT = apTable;
  tI64 pos = apFile->Tell();
  const cString fileContent = apFile->ReadString();
  if (!ni::GetLang()->XmlParseString(fileContent,sink)) {
    niError("Invalid XML.");
    return eInvalidHandle;
  }
  return (tSize)(pos-apFile->Tell());
}
#endif

///////////////////////////////////////////////
tBool __stdcall cLang::SerializeDataTable(const achar* aaszType, eSerializeMode aMode, iDataTable* apTable, iFile* apFile)
{
  niCheckIsOK(apTable,eFalse);
  niCheckIsOK(apFile,eFalse);

  cString strFileType = aaszType;
  if (strFileType.IsEmpty()) {
    strFileType = apFile->GetSourcePath();
    strFileType = strFileType.RAfter(".");
  }

  if (strFileType.IEq(_A("binary")) || strFileType.IEq(_A("bin"))) {
    if (!ni::GetLang()->SerializeObject(apFile,apTable,aMode,NULL)) {
      niError(_A("Can't serialize the data table in binary."));
      return eFalse;
    }
    return eTrue;
  }
  else if (DataTableSerialize_GetExtVer(strFileType.Chars()) != 0) {
    const ni::tU32 nVersion = DataTableSerialize_GetExtVer(strFileType.Chars());
    if (niFlagIs(aMode,eSerializeFlags_Read)) {
      if (DataTableSerialize_Read(apFile,apTable) == eInvalidHandle) {
        niError(_A("Can't read the data table."));
        return eFalse;
      }
    }
    else if (niFlagIs(aMode,eSerializeFlags_Write)) {
      if (DataTableSerialize_Write(apFile,apTable,nVersion) == eInvalidHandle) {
        niError(_A("Can't write the data table."));
        return eFalse;
      }
    }
    return eTrue;
  }
  else
  {
    // Try the default xml reader/writer
    tBool isXML = eFalse;
    if (strFileType.IEq(_A("xml")) ||
        strFileType.IEq(_A("xml-raw")) ||
        strFileType.IEq(_A("xml-stream")))
    {
      isXML = eTrue;
    }
    else {
      const tI64 pos = apFile->Tell();
      isXML = apFile->Read8() == '<';
      apFile->SeekSet(pos);
    }

    if (isXML)
    {
      if (niFlagIs(aMode,eSerializeFlags_Read)) {
        if (DataTableSerialize_ReadXML(apFile,apTable) == eInvalidHandle) {
          niError(_A("Can't read the XML data table."));
          return eFalse;
        }
      }
      else if (niFlagIs(aMode,eSerializeFlags_Write)) {
        if (DataTableSerialize_WriteXML(apFile,apTable,strFileType.IEq(_A("xml-stream"))?-1:0)
            == eInvalidHandle)
        {
          niError(_A("Can't write the XML data table."));
          return eFalse;
        }
      }
      return eTrue;
    }

    niError(niFmt(_A("Unknown datatable file type '%s'."),strFileType.Chars()));
    return eFalse;
  }
}
