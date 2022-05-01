// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/Types.h"

#if niMinFeatures(20)

#include "API/niLang/Utils/ObjectInterfaceCast.h"
#include "API/niLang/Utils/SmartPtr.h"
#include "API/niLang/Utils/UTFImpl.h"
#include "API/niLang/Utils/Path.h"
#include "API/niLang/STL/stack.h"
#include "API/niLang/IXml.h"
#include "API/niLang/IFile.h"

using namespace ni;

// #define XMLPARSER_TRACE

//======================================================================
//
//   Xml Parser
//
//======================================================================
class cXMLParser
{
 public:
  Ptr<iXmlParserSink> _sink;
  const achar* P;                // current point in text to parse
  const achar* TextBegin;        // start of text to parse
  tSize TextSize;                // size of text to parse in characters, not bytes
  tI32  mElementDepth;

  cXMLParser(iXmlParserSink* apSink, const ni::achar* aaszFile, const tSize aTextSize) {
    _sink = apSink;
    niAssert(_sink.IsOK());
    this->setData(aaszFile,aTextSize);
    mElementDepth = 0;
  }

  virtual void setData(const ni::achar* aaszFile, const tSize aTextSize) {
    // set pointer to text begin
    TextBegin = aaszFile;
    TextSize = aTextSize;
    P = TextBegin;
  }

  //! Reads forward to the next xml node.
  //! \return Returns false, if there was no further node.
  virtual tBool read() {
    // if not end reached, parse the node
    if (P && (unsigned int)(P - TextBegin) < TextSize - 1 && *P != 0) {
      return parseCurrentNode();
    }
    return eFalse;
  }

  tBool notifyElementBegin(const achar* b, const achar* e) {
    ++mElementDepth;
    cString elName(b,e);
    if (!_sink->OnXmlParserSink_Node(eXmlParserNodeType_ElementBegin,elName.Chars()))
      return eFalse;
    return eTrue;
  }

  tBool notifyElementEnd(const achar* b, const achar* e) {
    --mElementDepth;
    cString elName(b,e);
    if (!_sink->OnXmlParserSink_Node(eXmlParserNodeType_ElementEnd,elName.Chars()))
      return eFalse;
    return eTrue;
  }


  // Reads the current xml node
  tBool parseCurrentNode()
  {
    const achar* start = P;

    // more forward until '<' found
    while (*P && *P != '<') {
      ++P;
    }
    if (!*P)
      return eFalse;

    if ((P - start) > 0 && mElementDepth > 0) {
      // we found some text, store it
      if (!setText(start, P))
        return eFalse;
    }
    ++P;
    if (!*P)
      return eFalse;

    // based on current token, parse and report next element
    switch (*P) {
      case '/':
        if (!parseClosingXMLElement())
          return eFalse;
        break;
      case '?':
        if (!ignoreDefinition())
          return eFalse;
        break;
      case '!':
        if (*(P+1) == '[') {
          if (!parseCDATA())
            return eFalse;
        }
        else {
          if (!parseComment())
            return eFalse;
        }
        break;
      default:
        if (!parseOpeningXMLElement())
          return eFalse;
        break;
    }

    return eTrue;
  }


  //! sets the state that text was found. Returns true if set should be set
  tBool setText(const achar* start, const achar* end)
  {
    // discard text that is composed only of whitespaces and new lines
    {
      tBool hasChar = eFalse;
      const achar* p = start;
      for(; *p && p != end; ++p) {
        if (!StrIsSpace(*p)) {
          hasChar = eTrue;
          break;
        }
      }
      if (!hasChar) {
        // is empty text
        cString nodeText(start,end);
        return _sink->OnXmlParserSink_Node(eXmlParserNodeType_EmptyText,nodeText.Chars());
      }
    }

    // Do the check here, so the test above should discard end of
    // file spaces and new lines for use
    niAssert(mElementDepth > 0);
    if (mElementDepth <= 0)
      return eFalse;

    // current XML node type is text
    cString nodeText(start,end);
    return _sink->OnXmlParserSink_Node(eXmlParserNodeType_Text,nodeText.Chars());
  }



  //! ignores an xml definition like <?xml something />
  tBool ignoreDefinition()
  {
    // move until end marked with '>' reached
    while (*P && *P != '>') {
      ++P;
    }
    if (!*P)
      return eFalse;
    ++P;
    return eTrue;
  }


  //! parses a comment
  tBool parseComment()
  {
    if (!*P)
      return eFalse;

    P += 1;

    const achar *pCommentBegin = P;
    int count = 1;

    // move until end of comment reached
    while (*P && count) {
      if (*P == '>') {
        --count;
      }
      else if (*P == '<') {
        ++count;
      }
      ++P;
    }
    if (!*P)
      return eFalse;

    P -= 3;
    cString nodeComment(pCommentBegin+2,P-2);
    tBool r = _sink->OnXmlParserSink_Node(eXmlParserNodeType_Comment,nodeComment.Chars());
    P += 3;
    return r;
  }


  //! parses an opening xml element and reads attributes
  tBool parseOpeningXMLElement()
  {
    tBool directClose = eFalse;

    // find name
    const achar* startName = P;

    // find end of element
    while (*P && *P != '>' && !isWhiteSpace(*P) && *P != '/') {
      ++P;
    }

    const achar* endName = P;

    if (!notifyElementBegin(startName,endName))
      return eFalse;

    // find Attributes
    while (*P && *P != '>')
    {
      if (isWhiteSpace(*P)) {
        ++P;
      }
      else {
        if (*P != '/') {
          // we've got an attribute

          // read the attribute names
          const achar* attributeNameBegin = P;
          while (*P &&
                 // PIERRE: THIS IS COMMENTED ON PURPOSE. We allow spaces in
                 // property names, which isn't standard XML, but since JSON
                 // supports it makes our lives much easier in practise when
                 // converting JSON to a DataTable that is later serialized in
                 // XML. Usually this happens when the JSON is read from an
                 // HTTP API.
                 // !isWhiteSpace(*P) &&
                 *P != '=' &&
                 *P != '>') {
            ++P;
          }
          if (!*P) // malformatted xml file
            return eFalse;

          const achar* attributeNameEnd = P;
          // Remove trailing whitespaces. Whitespaces in the middle of an
          // attribute name is fine, but not trailing the property name,
          // that's just confusing...
          while(isWhiteSpace(*(attributeNameEnd-1)) &&
                ((attributeNameEnd-1) > attributeNameBegin)) {
            --attributeNameEnd;
          }

          if (*P == '>') {
            // Don't skip '>' since it also markes the end of the element
            // declaration
            cString attrName(attributeNameBegin,attributeNameEnd);
            if (!_sink->OnXmlParserSink_Attribute(attrName.Chars(),attrName.Chars()))
              return eFalse;
          }
          else /*if (*P == '=')*/ {
            ++P; // skip '='
            // read the attribute value
            // check for quotes and single quotes, thx to murphy
            while (*P && (*P != '\"') && (*P != '\'')) {
              ++P;
            }
            if (!*P) // malformatted xml file
              return eFalse;

            const achar attributeQuoteChar = *P;
            ++P;

            const achar* attributeValueBegin = P;
            while (*P != attributeQuoteChar && *P) {
              ++P;
            }
            if (!*P) // malformatted xml file
              return eFalse;

            const achar* attributeValueEnd = P;
            ++P;

            cString attrName(attributeNameBegin,attributeNameEnd);
            cString attrValue(attributeValueBegin,attributeValueEnd);
            if (!_sink->OnXmlParserSink_Attribute(attrName.Chars(),attrValue.Chars()))
              return eFalse;
          }
        }
        else
        {
          // tag is closed directly
          ++P;
          directClose = eTrue;
          break;
        }
      }
    }

    // check if this tag is closing directly
    if (endName > startName && *(endName-1) == '/') {
      // directly closing tag
      endName--;
      directClose = eTrue;
    }
    if (directClose) {
      if (!notifyElementEnd(startName,endName))
        return eFalse;
    }

    ++P;
    return eTrue;
  }


  //! parses an closing xml tag
  tBool parseClosingXMLElement()
  {
    if (!*P) // malformatted xml file
      return eFalse;

    ++P;
    const achar* pBeginClose = P;
    while (*P && *P != '>') {
      ++P;
    }
    if (!*P)
      return eFalse;
    if (!notifyElementEnd(pBeginClose,P))
      return eFalse;
    ++P;
    return eTrue;
  }

  //! parses a possible CDATA section, returns false if begin was not a CDATA section
  tBool parseCDATA()
  {
    niAssert(mElementDepth > 0);
    if (mElementDepth <= 0)
      return eFalse;

    // skip '<![CDATA['
    int count=0;
    while (*P && count < 8) {
      ++P;
      ++count;
    }
    if (!*P)
      return eFalse;

    const achar *cDataBegin = P;
    const achar *cDataEnd = 0;

    // find end of CDATA
    while (*P && !cDataEnd) {
      if (*P == '>' &&
          (*(P-1) == ']') &&
          (*(P-2) == ']'))
      {
        cDataEnd = P - 2;
      }
      ++P;
    }

    if (cDataEnd) {
      cString nodeCDATA(cDataBegin,cDataEnd);
      _sink->OnXmlParserSink_Node(eXmlParserNodeType_CDATA,nodeCDATA.Chars());
    }
    else {
      _sink->OnXmlParserSink_Node(eXmlParserNodeType_CDATA,AZEROSTR);
    }

    return true;
  }

  //! returns true if a character is whitespace
  inline bool isWhiteSpace(achar c) {
    return (c==' ' || c=='\t' || c=='\n' || c=='\r');
  }
};

//======================================================================
//
//   Xml API
//
//======================================================================
namespace ni {

niExportFunc(ni::tBool) XmlParseString(const ni::cString& aString, ni::iXmlParserSink* apSink) {
  niCheckIsOK(apSink,ni::eFalse);
  ni::Ptr<ni::iXmlParserSink> sink = apSink;
  cXMLParser parser(sink,aString.Chars(),aString.size());
  while (parser.read()) {
  }
  return ni::eTrue;
}

niExportFunc(ni::tBool) XmlParseFile(ni::iFile* apFile, ni::iXmlParserSink* apSink) {
  niCheckIsOK(apFile,ni::eFalse);
  niCheckIsOK(apSink,ni::eFalse);
  ni::Ptr<ni::iXmlParserSink> sink = apSink;
  return XmlParseString(apFile->ReadString(),apSink);
}

}
#endif
