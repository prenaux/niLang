#pragma once
#ifndef __IXML_H_6C5A488C_B1AB_4EE5_B0C3_3E8C3E04D61C__
#define __IXML_H_6C5A488C_B1AB_4EE5_B0C3_3E8C3E04D61C__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#if niMinFeatures(20)
namespace ni {
/** \addtogroup niLang
 * @{
 */

//! Xml parser node types.
enum eXmlParserNodeType {
  //! A xml element, like \<foo>
  eXmlParserNodeType_ElementBegin = 0,
  //! End of an xml element, like \</foo>
  eXmlParserNodeType_ElementEnd = 1,
  //! Text within a xml element: \<foo> this is the text. \</foo>
  eXmlParserNodeType_Text = 2,
  //! Empty Text within a xml element: \<foo> \</foo>
  //! \remark Empty is text composed only of non-printable characters.
  eXmlParserNodeType_EmptyText = 3,
  //! An xml comment like &lt;!-- I am a comment --&gt; or a DTD definition.
  eXmlParserNodeType_Comment = 4,
  //! An xml cdata section like &lt;![CDATA[ this is some CDATA ]]&gt;
  eXmlParserNodeType_CDATA = 5,
  //! \internal
  eXmlParserNodeType_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Xml parser sink interface.
struct iXmlParserSink : public iUnknown {
  niDeclareInterfaceUUID(iXmlParserSink,0x061d5946,0xbb2f,0x4f8c,0x9b,0x67,0xa5,0xc3,0x42,0x63,0x59,0x71);
  virtual tBool __stdcall OnXmlParserSink_Node(
      eXmlParserNodeType aType, const ni::achar* aNameOrData) = 0;
  virtual tBool __stdcall OnXmlParserSink_Attribute(
      const ni::achar* aName,
      const ni::achar* aValue) = 0;
};

niExportFunc(ni::tBool) XmlParseFile(ni::iFile* apFile, ni::iXmlParserSink* apSink);
niExportFunc(ni::tBool) XmlParseString(const ni::cString& aString, ni::iXmlParserSink* apSink);

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // niMinFeatures
#endif // __IXML_H_6C5A488C_B1AB_4EE5_B0C3_3E8C3E04D61C__
