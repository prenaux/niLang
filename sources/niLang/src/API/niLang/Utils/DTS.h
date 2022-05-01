#ifndef __DTS_H_3565D2D1_F5F5_46A6_A0FD_A1160D832A2F__
#define __DTS_H_3565D2D1_F5F5_46A6_A0FD_A1160D832A2F__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#define _DTS_INIT(DT,FLAGS,RO)                                          \
  ni::iDataTable* pSerializeDT = const_cast<ni::iDataTable*>(DT);       \
  niCheckIsOK(pSerializeDT,ni::eFalse);                                 \
  const ni::tBool bSerializeRO = RO;                                    \
  niUnused(bSerializeRO);                                               \
  const ni::tBool bSerializeWrite = niFlagIs(aFlags,eSerializeFlags_Write); \
  niUnused(bSerializeWrite);                                            \
  const ni::tBool bSerializeMT = niFlagIs(aFlags,eSerializeFlags_TypeInfoMetadata); \
  niUnused(bSerializeMT);

#define _DTS_WRITE_RAW_(NAME,VAL,TYPE)          \
  pSerializeDT->Set##TYPE(NAME,VAL);
#define _DTS_WRITE_MT_(NAME,VAL,TYPE,MT)        \
  pSerializeDT->Set##TYPE(NAME,VAL);           \
  if (niIsStringOK(MT) && bSerializeMT) {       \
    pSerializeDT->SetMetadata(NAME,_H(MT));    \
  }
#define _DTS_READ_(NAME,VAL,TYPE)                                       \
  if (!bSerializeRO) { (VAL) = pSerializeDT->Get##TYPE##Default(NAME,VAL); }
#define _DTS_RAW_(NAME,VAL,TYPE)                            \
  if (bSerializeWrite) { _DTS_WRITE_RAW_(NAME,VAL,TYPE); }  \
  else { _DTS_READ_(NAME,VAL,TYPE); }
#define _DTS_MT_(NAME,VAL,TYPE,MT)                            \
  if (bSerializeWrite) { _DTS_WRITE_MT_(NAME,VAL,TYPE,MT); }  \
  else { _DTS_READ_(NAME,VAL,TYPE); }                         \

#define _DTS_WRITE_E(NAME,VAL,TYPE)                                    \
  pSerializeDT->SetEnum(NAME,niEnumExpr(TYPE),VAL);               \
  if (bSerializeMT) {                                                   \
    pSerializeDT->SetMetadata(NAME,_H(niFmt(_A("enum[%s]"),niEnumName(TYPE)))); \
  }
#define _DTS_READ_E(NAME,VAL,TYPE)                                     \
  if (!bSerializeRO) { (VAL) = (TYPE)pSerializeDT->GetEnumDefault(NAME,niEnumExpr(TYPE),VAL); }
#define _DTS_E(NAME,VAL,TYPE)                             \
  if (bSerializeWrite) { _DTS_WRITE_E(NAME,VAL,TYPE); }  \
  else { _DTS_READ_E(NAME,VAL,TYPE); }

#define _DTS_WRITE_L(NAME,VAL,TYPE)                                    \
  pSerializeDT->SetEnum(NAME,niFlagsExpr(TYPE),VAL);              \
  if (bSerializeMT) {                                                   \
    pSerializeDT->SetMetadata(NAME,_H(niFmt(_A("flags[%s]"),niEnumName(TYPE)))); \
  }
#define _DTS_READ_L(NAME,VAL,TYPE)                                     \
  if (!bSerializeRO) { (VAL) = (TYPE)pSerializeDT->GetEnumDefault(NAME,niFlagsExpr(TYPE),VAL); }
#define _DTS_L(NAME,VAL,TYPE)                             \
  if (bSerializeWrite) { _DTS_WRITE_L(NAME,VAL,TYPE); }  \
  else { _DTS_READ_L(NAME,VAL,TYPE); }

#define _DTS_WRITE_S(NAME,VAL) _DTS_WRITE_RAW_(NAME,VAL,String)
#define _DTS_READ_S(NAME,VAL) _DTS_READ_(NAME,VAL,String)
#define _DTS_S(NAME,VAL)                            \
  if (bSerializeWrite) { _DTS_WRITE_S(NAME,VAL); }  \
  else { _DTS_READ_H(NAME,VAL); }

#define _DTS_WRITE_H(NAME,VAL) _DTS_WRITE_RAW_(NAME,VAL,HString)
#define _DTS_READ_H(NAME,VAL) _DTS_READ_(NAME,VAL,HString)
#define _DTS_H(NAME,VAL)                            \
  if (bSerializeWrite) { _DTS_WRITE_H(NAME,VAL); }  \
  else { _DTS_READ_H(NAME,VAL); }

#define _DTS_WRITE_I(NAME,VAL) _DTS_WRITE_RAW_(NAME,VAL,Int)
#define _DTS_READ_I(NAME,VAL) _DTS_READ_(NAME,VAL,Int)
#define _DTS_I(NAME,VAL)                            \
  if (bSerializeWrite) { _DTS_WRITE_I(NAME,VAL); }  \
  else { _DTS_READ_I(NAME,VAL); }

#define _DTS_WRITE_F(NAME,VAL) _DTS_WRITE_RAW_(NAME,VAL,Float)
#define _DTS_READ_F(NAME,VAL) _DTS_READ_(NAME,VAL,Float)
#define _DTS_F(NAME,VAL)                            \
  if (bSerializeWrite) { _DTS_WRITE_F(NAME,VAL); }  \
  else { _DTS_READ_F(NAME,VAL); }

#define _DTS_WRITE_F_RAD(NAME,VAL) _DTS_WRITE_MT_(NAME,VAL,Float,"radians")
#define _DTS_READ_F_RAD(NAME,VAL) _DTS_READ_(NAME,VAL,Float)
#define _DTS_F_RAD(NAME,VAL)                            \
  if (bSerializeWrite) { _DTS_WRITE_F_RAD(NAME,VAL); }  \
  else { _DTS_READ_F_RAD(NAME,VAL); }

#define _DTS_WRITE_B(NAME,VAL) _DTS_WRITE_MT_(NAME,VAL,Bool,"bool")
#define _DTS_READ_B(NAME,VAL) _DTS_READ_(NAME,VAL,Bool)

#define _DTS_B(NAME,VAL)                            \
  if (bSerializeWrite) { _DTS_WRITE_B(NAME,VAL); }  \
  else { _DTS_READ_B(NAME,VAL); }

#define _DTS_WRITE_XY(NAME,VAL) _DTS_WRITE_MT_(NAME,VAL,Vec2,"xy")
#define _DTS_READ_XY(NAME,VAL) _DTS_READ_(NAME,VAL,Vec2)
#define _DTS_XY(NAME,VAL)                           \
  if (bSerializeWrite) { _DTS_WRITE_XY(NAME,VAL); } \
  else { _DTS_READ_XY(NAME,VAL); }

#define _DTS_WRITE_XYZ(NAME,VAL) _DTS_WRITE_MT_(NAME,VAL,Vec3,"xyz")
#define _DTS_READ_XYZ(NAME,VAL) _DTS_READ_(NAME,VAL,Vec3)
#define _DTS_XYZ(NAME,VAL)                            \
  if (bSerializeWrite) { _DTS_WRITE_XYZ(NAME,VAL); }  \
  else { _DTS_READ_XYZ(NAME,VAL); }

#define _DTS_WRITE_XYZW(NAME,VAL) _DTS_WRITE_MT_(NAME,VAL,Vec4,"xyzw")
#define _DTS_READ_XYZW(NAME,VAL) _DTS_READ_(NAME,VAL,Vec4)
#define _DTS_XYZW(NAME,VAL)                           \
  if (bSerializeWrite) { _DTS_WRITE_XYZW(NAME,VAL); } \
  else {  _DTS_READ_XYZW(NAME,VAL); }

#define _DTS_WRITE_PLANE(NAME,VAL) _DTS_WRITE_MT_(NAME,VAL,Vec4,"plane")
#define _DTS_READ_PLANE(NAME,VAL) _DTS_READ_(NAME,VAL,Vec4)
#define _DTS_PLANE(NAME,VAL)                           \
  if (bSerializeWrite) { _DTS_WRITE_PLANE(NAME,VAL); } \
  else {  _DTS_READ_PLANE(NAME,VAL); }

#define _DTS_WRITE_RECT(NAME,VAL) _DTS_WRITE_MT_(NAME,VAL,Vec4,"rect")
#define _DTS_READ_RECT(NAME,VAL) _DTS_READ_(NAME,VAL,Vec4)
#define _DTS_RECT(NAME,VAL)                            \
  if (bSerializeWrite) { _DTS_WRITE_RECT(NAME,VAL); }  \
  else {  _DTS_READ_RECT(NAME,VAL); }

#define _DTS_WRITE_XY_RAD(NAME,VAL) _DTS_WRITE_MT_(NAME,VAL,Vec2,"xy[radians]")
#define _DTS_READ_XY_RAD(NAME,VAL) _DTS_READ_(NAME,VAL,Vec2)
#define _DTS_XY_RAD(NAME,VAL)                           \
  if (bSerializeWrite) { _DTS_WRITE_XY_RAD(NAME,VAL); } \
  else { _DTS_READ_XY_RAD(NAME,VAL); }

#define _DTS_WRITE_XYZ_RAD(NAME,VAL) _DTS_WRITE_MT_(NAME,VAL,Vec3,"xyz[radians]")
#define _DTS_READ_XYZ_RAD(NAME,VAL) _DTS_READ_(NAME,VAL,Vec3)
#define _DTS_XYZ_RAD(NAME,VAL)                            \
  if (bSerializeWrite) { _DTS_WRITE_XYZ_RAD(NAME,VAL); }  \
  else { _DTS_READ_XYZ_RAD(NAME,VAL); }

#define _DTS_WRITE_XYZW_RAD(NAME,VAL) _DTS_WRITE_MT_(NAME,VAL,Vec4,"xyzw[radians]")
#define _DTS_READ_XYZW_RAD(NAME,VAL) _DTS_READ_(NAME,VAL,Vec4)
#define _DTS_XYZW_RAD(NAME,VAL)                           \
  if (bSerializeWrite) { _DTS_WRITE_XYZW_RAD(NAME,VAL); } \
  else { _DTS_READ_XYZW_RAD(NAME,VAL); }

#define _DTS_WRITE_RGB(NAME,VAL) _DTS_WRITE_MT_(NAME,VAL,Vec3,"color")
#define _DTS_READ_RGB(NAME,VAL) _DTS_READ_(NAME,VAL,Vec3)
#define _DTS_RGB(NAME,VAL)                            \
  if (bSerializeWrite) { _DTS_WRITE_RGB(NAME,VAL); }  \
  else { _DTS_READ_RGB(NAME,VAL); }

#define _DTS_WRITE_RGBA(NAME,VAL) _DTS_WRITE_MT_(NAME,VAL,Vec4,"color")
#define _DTS_READ_RGBA(NAME,VAL) _DTS_READ_(NAME,VAL,Vec4)
#define _DTS_RGBA(NAME,VAL)                           \
  if (bSerializeWrite) { _DTS_WRITE_RGBA(NAME,VAL); } \
  else { _DTS_READ_RGBA(NAME,VAL); }

#define _DTS_WRITE_MATRIX(NAME,VAL) _DTS_WRITE_RAW_(NAME,VAL,Matrix)
#define _DTS_READ_MATRIX(NAME,VAL) _DTS_READ_(NAME,VAL,Matrix)
#define _DTS_MATRIX(NAME,VAL)                            \
  if (bSerializeWrite) { _DTS_WRITE_MATRIX(NAME,VAL); }  \
  else { _DTS_READ_MATRIX(NAME,VAL); }

#endif // __DTS_H_3565D2D1_F5F5_46A6_A0FD_A1160D832A2F__
