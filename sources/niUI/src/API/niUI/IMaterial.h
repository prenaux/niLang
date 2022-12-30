#ifndef __IMATERIAL_38682273_H__
#define __IMATERIAL_38682273_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "niLang/Types.h"
#include <niLang/IExpression.h>

namespace ni {

struct iMaterial;
struct iRasterizerStates;
struct iDepthStencilStates;
struct iSamplerStates;
struct iDrawOperation;
struct iShaderConstants;

/** \addtogroup niUI
 * @{
 */

//! Material flags.
enum eMaterialFlags {
  //! Material is double sided.
  //! \remark This is independant of the culling mode, the graphics driver is
  //! responsible
  //!         for making sure that double sided materials are drawn both sides
  //!         when this flag is set.
  eMaterialFlags_DoubleSided = niBit(0),
  //! Material is translucent.
  eMaterialFlags_Translucent = niBit(1),
  //! Material is transparent.
  //! \remark A transparent material is alpha tested, the alpha test key
  //!         value is in the alpha channel of the Base Channel color.
  eMaterialFlags_Transparent = niBit(2),
  //! Multiply the diffuse channel texture by the diffuse channel's color.
  eMaterialFlags_DiffuseModulate = niBit(3),
  //! The material is vertex lit.
  eMaterialFlags_Vertex = niBit(4),
  //! The material has a height map.
  eMaterialFlags_HeightMap = niBit(5),
  //! The material uses a normal map.
  //! Otherwise it's considered to be a grayscale bump.
  eMaterialFlags_NormalMap = niBit(6),
  //! The material is refracting.
  eMaterialFlags_Refraction = niBit(7),
  //! The material is used as a decal.
  eMaterialFlags_Decal = niBit(8),
  //! The material is a tilemap.
  eMaterialFlags_TileMap = niBit(9),
  //! The material should only be drawn in the depth buffer. (no color write)
  eMaterialFlags_DepthOnly = niBit(10),
  //! The material doesn't cast shadow.
  eMaterialFlags_NoShadow = niBit(11),
  //! The material shouldn't be lit.
  eMaterialFlags_NoLighting = niBit(12),
  //! The material isnt drawn.
  eMaterialFlags_NoDraw = niBit(13),
  //! The material's opacity channel contains a distance field.
  eMaterialFlags_DistanceField = niBit(14),
  //! Enable transparent (alpha-tested) anti-aliasing.
  eMaterialFlags_TransparentAA = niBit(15),
  //! No collision mesh should be associated with this material.
  eMaterialFlags_NoCollision = niBit(16),
  //! Using polygon offset in this material
  eMaterialFlags_PolygonOffset = niBit(17),
  //! Reverses current culling
  eMaterialFlags_ReverseCulling = niBit(18),
  //! \internal
  eMaterialFlags_ForceDWORD = 0xFFFFFFFF
};

//! Material flags type.
typedef tU32  tMaterialFlags;

//! Material channel
enum eMaterialChannel
{
  //! Base texture map.
  //! RGB : Diffuse/albedo color
  //! Alpha : Opacity
  //! Default Blend mode : NoBlending
  eMaterialChannel_Base = 0,
  //! Opacity/Detail texture map.
  //! RGB : Detail
  //! Alpha : Opacity/Detail
  //! Default Blend mode : NoBlending
  eMaterialChannel_Opacity = 1,
  //! Bump map.
  //! RGB : Normal map.
  //! Alpha : Height map.
  //! Default Blend mode : NoBlending
  eMaterialChannel_Bump = 2,
  //! Specular map.
  //! RGB : Specular color
  //! Grayscale : Gloss.
  //! Specular color alpha contains the specular power.
  //! Default Blend mode : NoBlending
  eMaterialChannel_Specular = 3,
  //! Environment map.
  //! RGB : Color
  //! Default Blend mode : Modulate
  eMaterialChannel_Environment = 4,
  //! Light/ambient map.
  //! RGB : Light color
  //! Alpha : Light intensity
  //! Default Blend mode : Modulate
  eMaterialChannel_Ambient = 5,
  //! Emissive map.
  //! RGB : Color
  //! Alpha : Directional coefficient.
  //! Default Blend mode : Additive
  eMaterialChannel_Emissive = 6,
  //! Refraction direction map.
  //! RGB : Refraction direction
  //! Default Blend mode : NoBlending
  eMaterialChannel_Refraction = 7,
  //! User 0 map.
  //! Default Blend mode : NoBlending
  eMaterialChannel_T0 = 8,
  //! User 1 map.
  //! Default Blend mode : NoBlending
  eMaterialChannel_T1 = 9,
  //! User 2 map.
  //! Default Blend mode : NoBlending
  eMaterialChannel_T2 = 10,
  //! User 3 map.
  //! Default Blend mode : NoBlending
  eMaterialChannel_T3 = 11,
  //! User 4 map.
  //! Default Blend mode : NoBlending
  eMaterialChannel_T4 = 12,
  //! User 5 map.
  //! Default Blend mode : NoBlending
  eMaterialChannel_T5 = 13,
  //! User 6 map.
  //! Default Blend mode : NoBlending
  eMaterialChannel_T6 = 14,
  //! User 7 map.
  //! Default Blend mode : NoBlending
  eMaterialChannel_T7 = 15,
  //! \internal
  //! Default Blend mode : NoBlending
  eMaterialChannel_Last = 16,
  //! \internal
  eMaterialChannel_ForceDWORD = 0xFFFFFFFF
};

//! Material expressions
enum eMaterialExpression {
  //! Texture channel 0 matrix expression. Shader uniform: float4 fpsExprTex0.
  //! \remark Works with the fixed pipeline.
  eMaterialExpression_Tex0 = 0,
  //! Texture channel 1 matrix expression. Shader uniform: float4 fpsExprTex1.
  //! \remark Works with the fixed pipeline.
  eMaterialExpression_Tex1 = 1,
  //! Generic matrix expression. Shader uniform: float4x4 fpsExprMatrix0.
  //! \remark The usage depends on the shader / render path actually used.
  eMaterialExpression_Matrix0 = 2,
  //! Generic matrix expression. Shader uniform: float4x4 fpsExprMatrix1.
  //! \remark The usage depends on the shader / render path actually used.
  eMaterialExpression_Matrix1 = 3,
  //! \internal
  eMaterialExpression_Last = 4
};

//! Material channel
struct sMaterialChannel {
  Ptr<iTexture> mTexture;
  sColor4f      mColor;
  tIntPtr       mhSS;
};

//! Material description structure.
struct sMaterialDesc {
  tHStringPtr           mhspName;
  tHStringPtr           mhspClass;
  tFVF                  mFVF;
  tMaterialFlags        mFlags;
  eBlendMode            mBlendMode;
  tIntPtr               mhRS;
  tIntPtr               mhDS;
  sMaterialChannel      mChannels[eMaterialChannel_Last];
  Ptr<iShader>          mShaders[eShaderUnit_Last];
  Ptr<iShaderConstants> mptrConstants;
  Ptr<iExpression>      mptrExpressions[eMaterialExpression_Last];
  sVec2f                mvPolygonOffset;
};

//! Material interface.
struct iMaterial : public iUnknown
{
  niDeclareInterfaceUUID(iMaterial,0x3d0f9569,0x2d97,0x4efd,0xab,0xe2,0x85,0x3f,0xbf,0xaf,0xbc,0x88)

  //########################################################################################
  //! \name General
  //########################################################################################
  //! @{

  //! Get the parent graphics object.
  //! {Property}
  virtual iGraphics* __stdcall GetGraphics() const = 0;
  //! Set the material's name.
  //! {Property}
  virtual void __stdcall SetName(iHString* ahspName) = 0;
  //! Get the material's name.
  //! {Property}
  virtual iHString* __stdcall GetName() const = 0;
  //! Set the material's class.
  //! {Property}
  virtual void __stdcall SetClass(iHString* ahspName) = 0;
  //! Get the material's class.
  //! {Property}
  virtual iHString* __stdcall GetClass() const = 0;
  //! Get the texture width.
  //! {Property}
  virtual tU32 __stdcall GetWidth() const = 0;
  //! Get the texture height.
  //! {Property}
  virtual tU32 __stdcall GetHeight() const = 0;
  //! Get the texture depth.
  //! {Property}
  virtual tU32 __stdcall GetDepth() const = 0;
  //! Set the material's flags.
  //! {Property}
  virtual void __stdcall SetFlags(tMaterialFlags aFlags) = 0;
  //! Get the material's flags.
  //! {Property}
  virtual tMaterialFlags __stdcall GetFlags() const = 0;
  //! Set the material's blend mode.
  //! {Property}
  //! \remark Shortcut to base sampler's blend mode.
  virtual tBool __stdcall SetBlendMode(eBlendMode aBlendMode) = 0;
  //! Get the material's blend mode.
  //! {Property}
  //! \remark Shortcut to base sampler's blend mode.
  virtual eBlendMode  __stdcall GetBlendMode() const = 0;
  //! @}

  //########################################################################################
  //! \name Rendering States
  //########################################################################################
  //! @{

  //! Set the material's rasterizer states.
  //! {Property}
  virtual tBool __stdcall SetRasterizerStates(tIntPtr aHandle) = 0;
  //! Get the material's rasterizer states.
  //! {Property}
  virtual tIntPtr __stdcall GetRasterizerStates() const = 0;
  //! Set the material's depth stencil states.
  //! {Property}
  virtual tBool __stdcall SetDepthStencilStates(tIntPtr aHandle) = 0;
  //! Get the material's depth stencil states.
  //! {Property}
  virtual tIntPtr __stdcall GetDepthStencilStates() const = 0;
  //! Set the material's polygon offset.
  //! {Property}
  virtual void __stdcall SetPolygonOffset(const ni::sVec2f& avOffset) = 0;
  //! Get the material's polygon offset.
  //! {Property}
  virtual ni::sVec2f __stdcall GetPolygonOffset() const = 0;
  //! Get whether any shader is set.
  //! {Property}
  virtual tBool __stdcall GetHasShader() const = 0;
  //! Set the specified shader.
  //! {Property}
  //! \remark Not cloned with the material.
  virtual tBool __stdcall SetShader(eShaderUnit aUnit, iShader* apProgram) = 0;
  //! Get the specified shader.
  //! {Property}
  virtual iShader* __stdcall GetShader(eShaderUnit aUnit) const = 0;
  //! Set the material constant buffer.
  //! {Property}
  virtual tBool __stdcall SetShaderConstants(iShaderConstants* apBuffer) = 0;
  //! Get the material constant buffer.
  //! {Property}
  virtual iShaderConstants* __stdcall GetShaderConstants() const = 0;
  //! @}

  //########################################################################################
  //! \name Textures and Colors
  //########################################################################################
  //! @{

  //! Check if the material has the specified channel texture.
  virtual tBool __stdcall HasChannelTexture(eMaterialChannel aChannel) const = 0;
  //! Set a material channel texture.
  //! {Property}
  //! \remark Not cloned with the material.
  virtual tBool __stdcall SetChannelTexture(eMaterialChannel aChannel, iTexture* apTexture) = 0;
  //! Get a material channel texture.
  //! {Property}
  virtual iTexture* __stdcall GetChannelTexture(eMaterialChannel aChannel) const = 0;
  //! Set a material channel color.
  //! {Property}
  virtual tBool __stdcall SetChannelColor(eMaterialChannel aChannel, const sColor4f& aColor) = 0;
  //! Get a material channel color.
  //! {Property}
  virtual const sColor4f&  __stdcall GetChannelColor(eMaterialChannel aChannel) const = 0;
  //! Set the sampler states of the specified texture channel.
  //! \remark If the sampler states are not specified the default states are used.
  //! {Property}
  virtual tBool __stdcall SetChannelSamplerStates(eMaterialChannel aChannel, tIntPtr aHandle) = 0;
  //! Get the sampler states of the specified texture channel.
  //! {Property}
  virtual tIntPtr __stdcall GetChannelSamplerStates(eMaterialChannel aChannel) const = 0;
  //! Copy the channel properties from another material.
  virtual tBool __stdcall CopyChannel(eMaterialChannel aDestChannel, const iMaterial* apSource, eMaterialChannel aSrcChannel) = 0;
  //! @}

  //########################################################################################
  //! \name Copy
  //########################################################################################
  //! @{

  //! Copy the source material.
  virtual tBool __stdcall Copy(const iMaterial* apMat) = 0;
  //! Clone this material.
  virtual iMaterial* __stdcall Clone() const = 0;
  //! Serialize the material in a data table.
  virtual tBool __stdcall Serialize(eSerializeMode aMode, iDataTable* apDT, iHString* ahspBasePath) = 0;
  //! @}

  //########################################################################################
  //! \name Low level
  //########################################################################################
  //! @{

  //! Get the material description structure pointer.
  //! {Property}
  virtual tPtr __stdcall GetDescStructPtr() const = 0;
  //! @}

  //########################################################################################
  //! \name Expression
  //########################################################################################
  //! @{

  //! Set an expression of the material.
  //! {Property}
  virtual void __stdcall SetExpression(eMaterialExpression aExpr, iHString* ahspExpr) = 0;
  //! Get the expression of the material.
  //! {Property}
  virtual iHString* __stdcall GetExpression(eMaterialExpression aExpr) const = 0;
  //! Get the matrix result of the specified material expression.
  //! {Property}
  //! \remark Returns identity if no valid expression is set.
  virtual sMatrixf __stdcall GetExpressionValueMatrix(eMaterialExpression aExpr) const = 0;
  //! Get the vector4 result of the specified material expression.
  //! {Property}
  //! \remark Returns zero if no valid expression is set.
  virtual sVec4f __stdcall GetExpressionValueVector(eMaterialExpression aExpr) const = 0;

  //! Set an expression object on the material.
  virtual void __stdcall SetExpressionObject(eMaterialExpression aExpr, iHString* ahspExpr, iExpression* apExpr) = 0;
  //! Get the expression object.
  virtual iExpression* __stdcall GetExpressionObject(eMaterialExpression aExpr) = 0;
  //! @}


  //########################################################################################
  //! \name Userdata
  //########################################################################################
  //! @{

  //! Set a userdata.
  //! {Property}
  virtual tBool __stdcall SetUserdata(iHString* ahspID, iUnknown* apUserdata) = 0;
  //! Get a userdata.
  //! {Property}
  virtual iUnknown* __stdcall GetUserdata(iHString* ahspID) const = 0;
  //! Get the number of userdata.
  //! {Property}
  virtual tSize __stdcall GetNumUserdata() const = 0;
  //! Get the name of the user data at the specified index.
  //! {Property}
  virtual iHString* __stdcall GetUserdataName(tU32 anIndex) const = 0;
  //! Get the userdata at the specified index.
  //! {Property}
  virtual iUnknown* __stdcall GetUserdataFromIndex(tU32 anIndex) const = 0;
  //! @}
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IMATERIAL_38682273_H__
