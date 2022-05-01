#ifndef __ISHADER_34445783_H__
#define __ISHADER_34445783_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/IHString.h>

namespace ni {
struct iShader;
struct iShaderConstants;

/** \addtogroup niUI
 * @{
 */

static const tU32 kfccD3DShader = niFourCC('A','D','3','D');

//! Shader Units.
enum eShaderUnit
{
  //! Vertex shader.
  eShaderUnit_Vertex = 0,
  //! Pixel shader.
  eShaderUnit_Pixel = 1,
  //! Native driver dependent shader.
  eShaderUnit_Native = 2,
  //! \internal.
  eShaderUnit_Last = 3,
  //! \internal
  eShaderUnit_ForceDWORD = 0xFFFFFFFF
};

//! Shader input register type.
enum eShaderRegisterType
{
  //! Input register.
  eShaderRegisterType_Input = 0,
  //! Output position register.
  eShaderRegisterType_Output = 1,
  //! Temporary register.
  eShaderRegisterType_Temporary = 2,
  //! Constant float register.
  eShaderRegisterType_ConstFloat = 3,
  //! Constant int register.
  eShaderRegisterType_ConstInt = 4,
  //! Constant bool register.
  eShaderRegisterType_ConstBool = 5,
  //! Sampler register.
  eShaderRegisterType_Sampler = 6,
  //! Address register.
  eShaderRegisterType_Address = 7,
  //! Loop register.
  eShaderRegisterType_Loop = 8,
  //! Texture register.
  eShaderRegisterType_Texture = 9,
  //! Predicate register.
  eShaderRegisterType_Predicate = 10,
  //! \internal
  eShaderRegisterType_Last = 11,
};

//! Shader constants buffer description structure.
struct sShaderConstantsDesc
{
  struct sConstant {
    //! The constant's name, as extracted from the shader.
    tHStringPtr         hspName;
    //! The constant's metadata
    tHStringPtr         hspMetadata;
    //! The constant's size, each register has four components.
    tU32                nSize;
    //! Index in the default data, float used mvFloatRegisters,
    //! int and bools uses mvIntRegisters.
    tU32                nDataIndex;
    //! Index of the constant to be set on the device.
    tU32                nHwIndex;
    //! The constant's type.
    tU32                Type;

    explicit sConstant()
        : nSize(0)
        , nDataIndex(0)
        , nHwIndex(0)
        , Type(0)
    {
    }
    explicit sConstant(iHString* ahspName, eShaderRegisterType aType, tU32 anSize, tU32 anDataIndex)
        : hspName(ahspName)
        , nSize(anSize)
        , nDataIndex(anDataIndex)
        , nHwIndex(eInvalidHandle)
        , Type((tU16)aType)
    {
    }
    sConstant(const sConstant& c)
        : hspName(c.hspName)
        , nSize(c.nSize)
        , nDataIndex(c.nDataIndex)
        , nHwIndex(c.nHwIndex)
        , Type((tU16)c.Type)
    {
    }
  };
  typedef astl::hstring_hash_map<tU32>    tConstMap;

  tU32              mnMaxRegisters;
  astl::vector<sVec4f> mvFloatRegisters;
  astl::vector<sVec4i> mvIntRegisters;
  astl::vector<sConstant> mvConstants;
  tConstMap       mmapConstants;
};

niCAssert(sizeof(sShaderConstantsDesc::sConstant) == ((2*sizeof(tPtr)) + 16));

//! Shader constants buffer.
//! \remark All sizes and offsets are expressed in register (sVec4f/l) unit.
struct iShaderConstants : public iSerializable {
  niDeclareInterfaceUUID(iShaderConstants,0x4450dc53,0xe4b1,0x4bc3,0x8c,0xd3,0xec,0x39,0x6b,0x36,0xc2,0xd7)

  //! Clone this constant buffer.
  virtual iShaderConstants* __stdcall Clone() const = 0;

  //! Get the constants buffer maximum number of registers.
  //! {Property}
  virtual tU32 __stdcall GetMaxNumRegisters() const = 0;

  //! Add a new constant.
  //! \return The index of the added constant, eInvalidHandle if error.
  virtual tU32 __stdcall AddConstant(iHString* ahspName, eShaderRegisterType aType, tU32 anSize) = 0;

  //! Get the number of constants in the buffer.
  //! {Property}
  virtual tU32 __stdcall GetNumConstants() const = 0;
  //! Get the index of the specified constant.
  //! {Property}
  virtual tU32 __stdcall GetConstantIndex(iHString* ahspName) const = 0;
  //! Get the name of the specified constant.
  //! {Property}
  virtual iHString* __stdcall GetConstantName(tU32 anConstIndex) const = 0;
  //! Get the size of the constant at the specified index.
  //! {Property}
  virtual tU32 __stdcall GetConstantSize(tU32 anConstIndex) const = 0;
  //! Get the type of the specified constant.
  //! {Property}
  virtual eShaderRegisterType __stdcall GetConstantType(tU32 anConstIndex) const = 0;

  //! Set the hardware register index of the specified constant.
  //! \remark a eInvalidHandle value means that the register index is not specified and
  //!     has to be determined from the context.
  //! {Property}
  virtual tBool __stdcall SetHwIndex(tU32 anConstIndex, tU32 anRegisterIndex) = 0;
  //! Get the hardware register index of the specified constant.
  //! {Property}
  virtual tU32 __stdcall GetHwIndex(tU32 anConstIndex) const = 0;

  //! Set a float constant from 4D vectors.
  virtual tBool __stdcall SetFloatArray(tU32 anConstIndex, const tVec4fCVec* apV, tU32 anSize = 1) = 0;
  //! Set a float constant from 4D vectors.
  //! {NoAutomation}
  virtual tBool __stdcall SetFloatPointer(tU32 anConstIndex, const sVec4f* apV, tU32 anSize = 1) = 0;
  //! Set a float constant from 4D vectors.
  virtual tBool __stdcall SetFloat(tU32 anConstIndex, const sVec4f& aV) = 0;
  //! Set four float constants from matrices.
  virtual tBool __stdcall SetFloatMatrixArray(tU32 anConstIndex, const tMatrixfCVec* apV, tU32 anSize = 1) = 0;
  //! Set four float constants from matrices.
  virtual tBool __stdcall SetFloatMatrix(tU32 anConstIndex, const sMatrixf& aV) = 0;
  //! Set an integer constant from 4D vectors.
  virtual tBool __stdcall SetIntArray(tU32 anConstIndex, const tVec4iCVec* apV, tU32 anSize = 1) = 0;
  //! Set an integer constant from 4D vectors.
  //! {NoAutomation}
  virtual tBool __stdcall SetIntPointer(tU32 anConstIndex, const sVec4i* apV, tU32 anSize = 1) = 0;
  //! Set an integer constant from 4D vectors.
  virtual tBool __stdcall SetInt(tU32 anConstIndex, const sVec4i& aV) = 0;

  //! Get the float constant at the specified index.
  virtual sVec4f __stdcall GetFloat(tU32 anConstIndex, tU32 anOffset) const = 0;
  //! Get the float constant matrix at the specified index.
  virtual sMatrixf __stdcall GetFloatMatrix(tU32 anConstIndex) const = 0;
  //! Get the float constant pointer at the specified index.
  //! {NoAutomation}
  virtual sVec4f* __stdcall GetFloatPointer(tU32 anConstIndex) const = 0;
  //! Get the float constant at the specified index.
  virtual sVec4i __stdcall GetInt(tU32 anConstIndex, tU32 anOffset) const = 0;
  //! Get the float constant pointer at the specified index.
  //! {NoAutomation}
  virtual sVec4i* __stdcall GetIntPointer(tU32 anConstIndex) const = 0;

  //! Get the shader constants buffer description structure pointer.
  //! {Property}
  virtual tPtr __stdcall GetDescStructPtr() const = 0;

  //! Set the metadata of the specified constant.
  //! {Property}
  virtual void __stdcall SetConstantMetadata(tU32 anConstIndex, iHString* ahspMetadata) = 0;
  //! Get the metadata of the specified constant.
  //! {Property}
  virtual iHString* __stdcall GetConstantMetadata(tU32 anConstIndex) const = 0;
};

//! Shader description structure.
struct sShaderDesc
{
  tHStringPtr            mhspName;
  Ptr<iHString>          mhspProfile;
  Ptr<iShaderConstants>  mptrConstants;
};

//! Shader interface.
struct iShader : public iDeviceResource
{
  niDeclareInterfaceUUID(iShader,0xd33a0c21,0xad8f,0x4a61,0xa1,0x32,0xc0,0xcd,0x95,0xf9,0x86,0x4d)

  //! Return the Shader unit on which it runs.
  //! {Property}
  virtual eShaderUnit __stdcall GetUnit() const = 0;
  //! Return the profile of the Shader.
  //! {Property}
  virtual iHString* __stdcall GetProfile() const = 0;
  //! Get the program's constants.
  //! {Property}
  virtual const iShaderConstants* __stdcall GetConstants() const = 0;
  //! Get the code of the shader.
  //! {Property}
  virtual iHString* __stdcall GetCode() const = 0;
  //! Get the shader description structure pointer.
  //! {Property}
  virtual tPtr __stdcall GetDescStructPtr() const = 0;
};

//===========================================================================
//
//  OpenGL interfaces
//
//===========================================================================

//!
//! The OpenGL shader that is linked and can give use the bindings and pushes
//! the uniforms. That's the interface the native shader's Bind() function
//! must return.
//!
//!
//! The calling order is:
//! - iGLShader::Bind(iDrawOperation) -> iGLShader
//! - iGLShader::GetVertexAttributeLocationArray() before binding the VA.
//! - glBindBuffer(VA), glDrawElements/glDrawArrays
//! - iGLShader::BeforeDraw(), its expected to push shader uniforms.
//! - glBindBuffer(IA), glDrawElements/glDrawArrays
//! - iGLShader::AfterDraw()
//!
struct iGLShader : public iUnknown {
  niDeclareInterfaceUUID(iGLShader,0xdd4046fa,0x5076,0x1641,0xad,0x1d,0x70,0xde,0xc5,0x05,0x4e,0xdf);
  //! {NoAutomation}
  virtual const tI32* __stdcall GetVertexAttributeLocationArray() const = 0;
  virtual tBool __stdcall BeforeDraw() = 0;
  virtual void __stdcall AfterDraw() = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __ISHADER_34445783_H__
