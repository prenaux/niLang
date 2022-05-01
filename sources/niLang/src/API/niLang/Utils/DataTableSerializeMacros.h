#ifndef __DATATABLESERIALIZEMACROS_50546964_H__
#define __DATATABLESERIALIZEMACROS_50546964_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

#define niDTS_SetProp_(NAME,VAL,TYPE,MT)                                \
  apDT->Set##TYPE(NAME,VAL);                                            \
  if (niIsStringOK(MT) && niFlagIs(aFlags,ni::eSerializeFlags_TypeInfoMetadata)) { \
    apDT->SetMetadata(NAME,_H(MT));                                     \
  }

#define niDTS_SetPropS(NAME,VAL,TYPE,MT)                                \
  apDT->Set##TYPE(NAME,(VAL).Chars());                                  \
  if (niIsStringOK(MT) && niFlagIs(aFlags,ni::eSerializeFlags_TypeInfoMetadata)) { \
    apDT->SetMetadata(NAME,_H(MT));                                     \
  }

#define niDTS_SetProp(NAME,TYPE,MT)                                     \
  apDT->Set##TYPE(_A(#NAME),m##NAME);                                   \
  if (niIsStringOK(MT) && niFlagIs(aFlags,ni::eSerializeFlags_TypeInfoMetadata)) { \
    apDT->SetMetadata(_A(#NAME),_H(MT));                                \
  }

#define niDTS_SetPropE(NAME,TYPE)                                       \
  apDT->SetEnum(_A(#NAME),niEnumExpr(TYPE),m##NAME);               \
  if (niFlagIs(aFlags,ni::eSerializeFlags_TypeInfoMetadata)) {          \
    apDT->SetMetadata(_A(#NAME),_H(niFmt(_A("enum[%s]"),niEnumName(TYPE)))); \
  }

#define niDTS_SetPropE_(NAME,VAL,TYPE)                                  \
  apDT->SetEnum(NAME,niEnumExpr(TYPE),VAL);                        \
  if (niFlagIs(aFlags,ni::eSerializeFlags_TypeInfoMetadata)) {          \
    apDT->SetMetadata(NAME,_H(niFmt(_A("enum[%s]"),niEnumName(TYPE)))); \
  }

#define niDTS_SetPropF(NAME,TYPE)                                       \
  apDT->SetEnum(_A(#NAME),niFlagsExpr(TYPE),m##NAME);                   \
  if (niFlagIs(aFlags,ni::eSerializeFlags_TypeInfoMetadata)) {          \
    apDT->SetMetadata(_A(#NAME),_H(niFmt(_A("flags[%s]"),niEnumName(TYPE)))); \
  }

#define niDTS_SetPropF_(NAME,VAL,TYPE)                                  \
  apDT->SetEnum(NAME,niFlagsExpr(TYPE),VAL);                       \
  if (niFlagIs(aFlags,ni::eSerializeFlags_TypeInfoMetadata)) {          \
    apDT->SetMetadata(NAME,_H(niFmt(_A("flags[%s]"),niEnumName(TYPE)))); \
  }

#define niDTS_GetPropE(NAME,TYPE)                                       \
  m##NAME = (TYPE)apDT->GetEnumDefault(_A(#NAME),niEnumExpr(TYPE),m##NAME);

#define niDTS_GetPropE_(NAME,VAL,TYPE)                              \
  VAL = (TYPE)apDT->GetEnumDefault(NAME,niEnumExpr(TYPE),VAL);

#define niDTS_GetPropF(NAME,TYPE)                                       \
  m##NAME = (TYPE)apDT->GetEnumDefault(_A(#NAME),niFlagsExpr(TYPE),m##NAME);

#define niDTS_GetPropF_(NAME,VAL,TYPE)                                \
  VAL = (TYPE)apDT->GetEnumDefault(NAME,niFlagsExpr(TYPE),VAL);

#define niDTS_GetProp(NAME,TYPE)                          \
  m##NAME = apDT->Get##TYPE##Default(_A(#NAME),m##NAME);

#define niDTS_GetProp_(NAME,VAL,TYPE)           \
  VAL = apDT->Get##TYPE##Default(NAME,VAL);

#define niDTS_GetPropS(NAME,VAL,TYPE)                 \
  VAL = apDT->Get##TYPE##Default(NAME,(VAL).Chars());

#define niDTS_GetPropFSet(NAME,TYPE)                                    \
  Set##VAL((TYPE)apDT->GetEnumDefault(_A(#NAME),niFlagsExpr(TYPE),Get##VAL()));

#define niDTS_GetPropESet(NAME,VAL,TYPE)                                \
  Set##VAL((TYPE)apDT->GetEnumDefault(NAME,niEnumExpr(TYPE),Get##VAL()));

#define niDTS_GetPropSet(NAME,VAL,TYPE)                 \
  Set##VAL(apDT->Get##TYPE##Default(NAME,Get##VAL()));

#define niDTS_GetPropFSetI(NAME,I,TYPE)                                 \
  Set##VAL(I,(TYPE)apDT->GetEnumDefault(_A(#NAME),niFlagsExpr(TYPE),Get##VAL(I)));

#define niDTS_GetPropESetI(NAME,I,VAL,TYPE)                             \
  Set##VAL(I,(TYPE)apDT->GetEnumDefault(NAME,niEnumExpr(TYPE),Get##VAL(I)));

#define niDTS_GetPropSetI(NAME,I,VAL,TYPE)                \
  Set##VAL(I,apDT->Get##TYPE##Default(NAME,Get##VAL(I)));

#define niDTS_SetPropVec2i(NAME,VAL,MT)                                 \
  apDT->SetVec2(NAME,ni::Vec2f(VAL));                                   \
  if (niIsStringOK(MT) && niFlagIs(aFlags,ni::eSerializeFlags_TypeInfoMetadata)) { \
    apDT->SetMetadata(NAME,_H(MT));                                     \
  }

#define niDTS_SetPropVec3i(NAME,VAL,MT)                                 \
  apDT->SetVec3(NAME,ni::Vec3f(VAL));                                   \
  if (niIsStringOK(MT) && niFlagIs(aFlags,ni::eSerializeFlags_TypeInfoMetadata)) { \
    apDT->SetMetadata(NAME,_H(MT));                                     \
  }

#define niDTS_SetPropVec4i(NAME,VAL,MT)                                 \
  apDT->SetVec4(NAME,ni::Vec4f(VAL));                                   \
  if (niIsStringOK(MT) && niFlagIs(aFlags,ni::eSerializeFlags_TypeInfoMetadata)) { \
    apDT->SetMetadata(NAME,_H(MT));                                     \
  }

#define niDTS_GetPropVec2i(NAME,VAL)                          \
  VAL = ni::Vec2i(apDT->GetVec2Default(NAME,ni::Vec2f(VAL)));

#define niDTS_GetPropVec3i(NAME,VAL)                          \
  VAL = ni::Vec3i(apDT->GetVec3Default(NAME,ni::Vec3f(VAL)));

#define niDTS_GetPropVec4i(NAME,VAL)                          \
  VAL = ni::Vec4i(apDT->GetVec4Default(NAME,ni::Vec4f(VAL)));

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
#endif // __DATATABLESERIALIZEMACROS_50546964_H__
