#ifndef __IGRAPHICSSTATES_33130606_H__
#define __IGRAPHICSSTATES_33130606_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */

//! Sampler states description structure.
struct sSamplerStatesDesc
{
  eSamplerFilter  mFilter;
  eSamplerWrap  mWrapS;
  eSamplerWrap  mWrapT;
  eSamplerWrap  mWrapR;
  sColor4f    mcolBorder;
  bool operator == (const sSamplerStatesDesc& aR) const {
    return
        mFilter == aR.mFilter &&
        mWrapS == aR.mWrapS &&
        mWrapT == aR.mWrapT &&
        mWrapR == aR.mWrapR &&
        mcolBorder == aR.mcolBorder;
  }
  bool operator != (const sSamplerStatesDesc& aR) const {
    return !(*this == aR);
  }
};

//! Sampler states interface.
//! {Serialize}
struct iSamplerStates : public iUnknown {

  niDeclareInterfaceUUID(iSamplerStates,0x86963478,0x914d,0x4e43,0x84,0x59,0xa1,0xdc,0x87,0xf5,0x83,0xf9)

  //! Copy another sampler states.
  virtual tBool __stdcall Copy(const iSamplerStates* apStates) = 0;
  //! Clone this sampler states.
  virtual iSamplerStates* __stdcall Clone() const = 0;
  //! Return whether the sampler states are compiled (read-only)
  //! {Property}
  virtual tBool __stdcall GetIsCompiled() const = 0;
  //! Set the filtering mode. (default eSamplerFilter_Point)
  //! {Property}{Serialize}
  virtual tBool __stdcall SetFilter(eSamplerFilter aFilter) = 0;
  //! Get the filtering mode.
  //! {Property}{Serialize}
  virtual eSamplerFilter __stdcall GetFilter() const = 0;
  //! Set the S-axis wrapping mode. (default eSamplerWrap_Clamp)
  //! {Property}{Serialize}
  virtual tBool __stdcall SetWrapS(eSamplerWrap aWrap) = 0;
  //! Get the S-axis wrapping mode.
  //! {Property}{Serialize}
  virtual eSamplerWrap __stdcall GetWrapS() const = 0;
  //! Set the T-axis wrapping mode. (default eSamplerWrap_Clamp)
  //! {Property}{Serialize}
  virtual tBool __stdcall SetWrapT(eSamplerWrap aWrap) = 0;
  //! Get the T-axis wrapping mode.
  //! {Property}{Serialize}
  virtual eSamplerWrap __stdcall GetWrapT() const = 0;
  //! Set the R-axis wrapping mode. (default eSamplerWrap_Clamp)
  //! {Property}{Serialize}
  virtual tBool __stdcall SetWrapR(eSamplerWrap aWrap) = 0;
  //! Get the R-axis wrapping mode.
  //! {Property}{Serialize}
  virtual eSamplerWrap __stdcall GetWrapR() const = 0;
  //! Set the border color, for border sampler wrap mode.
  //! {Property}{Serialize}
  virtual tBool __stdcall SetBorderColor(const sColor4f& avColor) = 0;
  //! Get the border color.
  //! {Property}{Serialize}
  virtual const sColor4f& __stdcall GetBorderColor() const = 0;

  //! Get the states description structure pointer.
  //! {Property}
  virtual tPtr __stdcall GetDescStructPtr() const = 0;

  //! Serialize the states.
  virtual ni::tBool __stdcall SerializeDataTable(ni::iDataTable* apDT, tSerializeFlags aFlags) = 0;
};

//! Depth stencil states description structure.
struct sDepthStencilStatesDesc
{
  tBool            mbDepthTest;
  tBool            mbDepthTestWrite;
  eGraphicsCompare mDepthTestCompare;
  eStencilMode     mStencilMode;
  tI32             mnStencilRef;
  tU32             mnStencilMask;
  eGraphicsCompare mStencilFrontCompare;
  eStencilOp       mStencilFrontFail;
  eStencilOp       mStencilFrontPassDepthFail;
  eStencilOp       mStencilFrontPassDepthPass;
  eGraphicsCompare mStencilBackCompare;
  eStencilOp       mStencilBackFail;
  eStencilOp       mStencilBackPassDepthFail;
  eStencilOp       mStencilBackPassDepthPass;

  bool operator == (const sDepthStencilStatesDesc& aR) const {
    if (!(mbDepthTest == aR.mbDepthTest))
      return false;
    if (mbDepthTest) {
      if (!(mbDepthTestWrite == aR.mbDepthTestWrite &&
            mDepthTestCompare != aR.mDepthTestCompare))
        return false;
    }
    if (mStencilMode != aR.mStencilMode)
      return false;
    if (mStencilMode == eStencilMode_None) {
      // go no further in stencil mode compare...
    }
    else {
      if (!(mnStencilRef == aR.mnStencilRef &&
            mnStencilMask == aR.mnStencilMask))
        return false;
      if (!(mStencilFrontCompare == aR.mStencilFrontCompare &&
            mStencilFrontFail == aR.mStencilFrontFail &&
            mStencilFrontPassDepthFail == aR.mStencilFrontPassDepthFail &&
            mStencilFrontPassDepthPass == aR.mStencilFrontPassDepthPass))
        return false;
      if (mStencilMode == eStencilMode_TwoSided) {
        if (!(mStencilBackCompare == aR.mStencilBackCompare &&
              mStencilBackFail == aR.mStencilBackFail &&
              mStencilBackPassDepthFail == aR.mStencilBackPassDepthFail &&
              mStencilBackPassDepthPass == aR.mStencilBackPassDepthPass))
          return false;
      }
    }
    return true;
  }
  bool operator != (const sDepthStencilStatesDesc& aR) const {
    return !(*this == aR);
  }
};

//! DepthStencil states
//! {Serialize}
struct iDepthStencilStates : public iUnknown {
  niDeclareInterfaceUUID(iDepthStencilStates,0xfdb9705c,0x32d0,0x41fb,0xa0,0x41,0x54,0x9d,0xc2,0x3c,0x7b,0xcd)

  //########################################################################################
  //! \name General
  //########################################################################################
  //! @{

  //! Copy another depth-stencil states.
  virtual tBool __stdcall Copy(const iDepthStencilStates* apStates)  = 0;
  //! Clone this depth-stencil states.
  virtual iDepthStencilStates* __stdcall Clone() const = 0;
  //! Return whether the depth-stencil states are compiled (read-only)
  //! {Property}
  virtual tBool __stdcall GetIsCompiled() const = 0;
  //! @}

  //########################################################################################
  //! \name Depth test
  //########################################################################################
  //! @{

  //! Set whether the depth test is enabled. (default false)
  //! {Property}{Serialize}
  virtual tBool __stdcall SetDepthTest(tBool aVal) = 0;
  //! Get whether the depth test is enabled.
  //! {Property}{Serialize}
  virtual tBool __stdcall GetDepthTest() const = 0;
  //! Set whether the depth test write is enabled. (default true)
  //! {Property}{Serialize}
  virtual tBool __stdcall SetDepthTestWrite(tBool aVal) = 0;
  //! Get whether the depth test write is enabled.
  //! {Property}{Serialize}
  virtual tBool __stdcall GetDepthTestWrite() const = 0;
  //! Set depth test compare function. (default eGraphicsCompare_LessEqual)
  //! {Property}{Serialize}
  virtual tBool __stdcall SetDepthTestCompare(eGraphicsCompare aVal) = 0;
  //! Get depth test compare function.
  //! {Property}{Serialize}
  virtual eGraphicsCompare __stdcall GetDepthTestCompare() const = 0;
  //! @}

  //########################################################################################
  //! \name Stencil
  //########################################################################################
  //! @{

  //! Set the stencil mode. (default eStencilMode_None)
  //! {Property}{Serialize}
  virtual tBool __stdcall SetStencilMode(eStencilMode aVal) = 0;
  //! Get the stencil mode.
  //! {Property}{Serialize}
  virtual eStencilMode __stdcall GetStencilMode() const = 0;
  //! Set the stencil reference value. (default 0)
  //! {Property}{Serialize}
  virtual tBool __stdcall SetStencilRef(tI32 aVal) = 0;
  //! Get the stencil reference value.
  //! {Property}{Serialize}
  virtual tI32 __stdcall GetStencilRef() const = 0;
  //! Set the stencil mask. (default 0xFFFFFFFF)
  //! {Property}{Serialize}
  virtual tBool __stdcall SetStencilMask(tU32 aVal) = 0;
  //! Get the stencil mask.
  //! {Property}{Serialize}
  virtual tU32 __stdcall GetStencilMask() const = 0;

  //! Set the stencil front test compare function. (eGraphicsCompare_Never)
  //! {Property}{Serialize}
  virtual tBool __stdcall SetStencilFrontCompare(eGraphicsCompare aVal) = 0;
  //! Get the stencil front test compare function.
  //! {Property}{Serialize}
  virtual eGraphicsCompare __stdcall GetStencilFrontCompare() const = 0;
  //! Set the stencil operation when the stencil front test fail. (eStencilOp_Keep)
  //! {Property}{Serialize}
  virtual tBool __stdcall SetStencilFrontFail(eStencilOp aVal) = 0;
  //! Get the stencil operation when the stencil front test fail.
  //! {Property}{Serialize}
  virtual eStencilOp __stdcall GetStencilFrontFail() const = 0;
  //! Set the stencil operation when the stencil front test pass and the depth test fail. (eStencilOp_Keep)
  //! {Property}{Serialize}
  virtual tBool __stdcall SetStencilFrontPassDepthFail(eStencilOp aVal) = 0;
  //! Get the stencil operation when the stencil front test pass and the depth test fail. (eStencilOp_Keep)
  //! {Property}{Serialize}
  virtual eStencilOp __stdcall GetStencilFrontPassDepthFail() const = 0;
  //! Set the stencil operation when the stencil front test pass and the depth test pass. (eStencilOp_Keep)
  //! {Property}{Serialize}
  virtual tBool __stdcall SetStencilFrontPassDepthPass(eStencilOp aVal) = 0;
  //! Get the stencil operation when the stencil front test pass and the depth test pass.
  //! {Property}{Serialize}
  virtual eStencilOp __stdcall GetStencilFrontPassDepthPass() const = 0;

  //! Set the stencil back test compare function. (eGraphicsCompare_Never)
  //! {Property}{Serialize}
  virtual tBool __stdcall SetStencilBackCompare(eGraphicsCompare aVal) = 0;
  //! Get the stencil back test compare function.
  //! {Property}{Serialize}
  virtual eGraphicsCompare __stdcall GetStencilBackCompare() const = 0;
  //! Set the stencil operation when the stencil back test fail. (eStencilOp_Keep)
  //! {Property}{Serialize}
  virtual tBool __stdcall SetStencilBackFail(eStencilOp aVal) = 0;
  //! Get the stencil operation when the stencil back test fail.
  //! {Property}{Serialize}
  virtual eStencilOp __stdcall GetStencilBackFail() const = 0;
  //! Set the stencil operation when the stencil back test pass and the depth test fail. (eStencilOp_Keep)
  //! {Property}{Serialize}
  virtual tBool __stdcall SetStencilBackPassDepthFail(eStencilOp aVal) = 0;
  //! Get the stencil operation when the stencil back test pass and the depth test fail.
  //! {Property}{Serialize}
  virtual eStencilOp __stdcall GetStencilBackPassDepthFail() const = 0;
  //! Set the stencil operation when the stencil back test pass and the depth test pass. (eStencilOp_Keep)
  //! {Property}{Serialize}
  virtual tBool __stdcall SetStencilBackPassDepthPass(eStencilOp aVal) = 0;
  //! Get the stencil operation when the stencil back test pass and the depth test pass.
  //! {Property}{Serialize}
  virtual eStencilOp __stdcall GetStencilBackPassDepthPass() const = 0;
  //! @}

  //! Get the states description structure pointer.
  //! {Property}
  virtual tPtr __stdcall GetDescStructPtr() const = 0;

  //! Serialize the states.
  virtual ni::tBool __stdcall SerializeDataTable(ni::iDataTable* apDT, tSerializeFlags aFlags) = 0;
};

//! Rasterizer states description structure.
struct sRasterizerStatesDesc
{
  tBool     mbWireframe;
  eCullingMode  mCullingMode;
  eColorWriteMask mColorWriteMask;
  tBool     mbScissorTest;
  tF32      mfDepthBiasFactor;
  tF32      mfDepthBiasUnitScale;
  bool operator == (const sRasterizerStatesDesc& aR) const {
    return
        mbWireframe == aR.mbWireframe &&
        mCullingMode == aR.mCullingMode &&
        mColorWriteMask == aR.mColorWriteMask &&
        mbScissorTest == aR.mbScissorTest &&
        mfDepthBiasFactor == aR.mfDepthBiasFactor &&
        mfDepthBiasUnitScale == aR.mfDepthBiasUnitScale;
  }
  bool operator != (const sRasterizerStatesDesc& aR) const {
    return !(*this == aR);
  }
};

//! Rasterizer states
//! {Serialize}
struct iRasterizerStates : public iUnknown {

  niDeclareInterfaceUUID(iRasterizerStates,0x8712bd8c,0x0924,0x481d,0x92,0x22,0xee,0xfa,0xc1,0x73,0xc9,0x41)

  //########################################################################################
  //! \name General
  //########################################################################################
  //! @{

  //! Copy another rasterizer states.
  virtual tBool __stdcall Copy(const iRasterizerStates* apStates)  = 0;
  //! Clone this rasterizer states.
  virtual iRasterizerStates* __stdcall Clone() const = 0;
  //! Return whether the rasterizer states are compiled (read-only)
  //! {Property}
  virtual tBool __stdcall GetIsCompiled() const = 0;
  //! @}

  //########################################################################################
  //! \name Rasterizer
  //########################################################################################
  //! @{

  //! Set wireframe rendering. (default false)
  //! {Property}{Serialize}
  virtual tBool __stdcall SetWireframe(tBool abWireframe) = 0;
  //! Get wireframe rendering.
  //! {Property}{Serialize}
  virtual tBool __stdcall GetWireframe() const = 0;

  //! Set the culling mode. (default eCullingMode_None)
  //! {Property}{Serialize}
  virtual tBool __stdcall SetCullingMode(eCullingMode aMode) = 0;
  //! Get the culling mode.
  //! {Property}{Serialize}
  virtual eCullingMode __stdcall GetCullingMode() const = 0;

  //! Set the color write mask. (default eColorWriteMask_All)
  //! {Property}{Serialize}
  virtual tBool __stdcall SetColorWriteMask(eColorWriteMask aMask) = 0;
  //! Get the color write mask.
  //! {Property}{Serialize}
  virtual eColorWriteMask __stdcall GetColorWriteMask() const = 0;
  //! @}

  //########################################################################################
  //! \name Scissor
  //########################################################################################
  //! @{

  //! Set whether the scissor test is enabled. (default false)
  //! {Property}{Serialize}
  virtual tBool __stdcall SetScissorTest(tBool abTest) = 0;
  //! Get whether the scissor test is enabled.
  //! {Property}{Serialize}
  virtual tBool __stdcall GetScissorTest() const = 0;
  //! @}

  //########################################################################################
  //! \name Depth bias
  //########################################################################################
  //! @{

  //! Set the depth bias factor. (default 0)
  //! {Property}{Serialize}
  virtual tBool __stdcall SetDepthBiasFactor(tF32 aVal) = 0;
  //! Get the depth bias factor.
  //! {Property}{Serialize}
  virtual tF32 __stdcall GetDepthBiasFactor() const = 0;

  //! Set the depth bias unit scale factor. (default 0)
  //! {Property}{Serialize}
  virtual tBool __stdcall SetDepthBiasUnitScale(tF32 aVal) = 0;
  //! Get the depth bias unit scale factor.
  //! {Property}{Serialize}
  virtual tF32 __stdcall GetDepthBiasUnitScale() const = 0;
  //! @}

  //! Get the states description structure pointer.
  //! {Property}
  virtual tPtr __stdcall GetDescStructPtr() const = 0;

  //! Serialize the states.
  virtual ni::tBool __stdcall SerializeDataTable(ni::iDataTable* apDT, tSerializeFlags aFlags) = 0;
};

//! Graphics fixed pipeline states description structure.
struct sFixedStatesDesc
{
  sMatrixf mCameraViewMatrix;
  sMatrixf mCameraProjectionMatrix;
  sMatrixf mViewMatrix;
  sMatrixf mProjectionMatrix;
};

//! Graphics fixed pipeline states interface.
//! {Serialize}
struct iFixedStates : public iUnknown {
  niDeclareInterfaceUUID(iFixedStates,0xe5ed8d23,0xc0b6,0x443b,0x9b,0x90,0xc5,0xf4,0x49,0x5d,0x62,0x0f)

  //########################################################################################
  //! \name General
  //########################################################################################
  //! @{

  //! Copy another fixed pipeline states.
  virtual tBool __stdcall Copy(const iFixedStates* apStates) = 0;
  //! Clone this fixed pipeline states.
  virtual iFixedStates* __stdcall Clone() const = 0;
  //! @}

  //########################################################################################
  //! \name Matrices
  //########################################################################################
  //! @{

  //! Set the fixed pipeline camera view matrix. (default identity)
  //! {Property}{Serialize}
  //! \remark This function will set the view matrix aswell
  virtual void __stdcall SetCameraViewMatrix(const sMatrixf& aVal) = 0;
  //! Set only the fixed pipeline camera view matrix. (default identity)
  //! {Property}
  //! \remark This function wont modify the view matrix
  virtual void __stdcall SetOnlyCameraViewMatrix(const sMatrixf& aVal) = 0;
  //! Get the fixed pipeline camera view matrix.
  //! {Property}{Serialize}
  virtual sMatrixf __stdcall GetCameraViewMatrix() const = 0;
  //! Get the inverse camera view matrix.
  //! {Property}
  virtual sMatrixf __stdcall GetCameraInvViewMatrix() const = 0;
  //! Set the fixed pipeline camera projection matrix. (default identity)
  //! {Property}{Serialize}
  //! \remark This function will set the projection matrix aswell
  virtual void __stdcall SetCameraProjectionMatrix(const sMatrixf& aVal) = 0;
  //! Set only the fixed pipeline camera projection matrix. (default identity)
  //! {Property}
  //! \remark This function wont modify the projection matrix
  virtual void __stdcall SetOnlyCameraProjectionMatrix(const sMatrixf& aVal) = 0;
  //! Get the fixed pipeline camera projection matrix.
  //! {Property}{Serialize}
  virtual sMatrixf __stdcall GetCameraProjectionMatrix() const = 0;
  //! Get the inverse camera projection matrix.
  //! {Property}
  virtual sMatrixf __stdcall GetCameraInvProjectionMatrix() const = 0;
  //! Get the camera view projection matrix.
  //! {Property}
  virtual sMatrixf __stdcall GetCameraViewProjectionMatrix() const = 0;
  //! Get the camera inverse view projection matrix.
  //! {Property}
  virtual sMatrixf __stdcall GetCameraInvViewProjectionMatrix() const = 0;
  //! Set the fixed pipeline view matrix. (default identity)
  //! {Property}{Serialize}
  virtual void __stdcall SetViewMatrix(const sMatrixf& aVal) = 0;
  //! Get the fixed pipeline view matrix.
  //! {Property}{Serialize}
  virtual sMatrixf __stdcall GetViewMatrix() const = 0;
  //! Set the fixed pipeline projection matrix. (default identity)
  //! {Property}{Serialize}
  virtual void __stdcall SetProjectionMatrix(const sMatrixf& aVal) = 0;
  //! Get the fixed pipeline projection matrix.
  //! {Property}{Serialize}
  virtual sMatrixf __stdcall GetProjectionMatrix() const = 0;

  //! Get the inverse view matrix.
  //! {Property}
  virtual sMatrixf __stdcall GetInvViewMatrix() const = 0;
  //! Get the view projection matrix.
  //! {Property}
  virtual sMatrixf __stdcall GetViewProjectionMatrix() const = 0;
  //! Get the inverse view projection matrix.
  //! {Property}
  virtual sMatrixf __stdcall GetInvViewProjectionMatrix() const = 0;
  //! Get the inverse projection matrix.
  //! {Property}
  virtual sMatrixf __stdcall GetInvProjectionMatrix() const = 0;

  //! Set a look at matrix and perspective projection in the view and projection matrices.
  //! \param abSetCameraMatrices if true the camera and render matrices will be set, if false only the render matrices will be set.
  //! \param avEye is the position of the camera
  //! \param avAt is the target of the camera
  //! \param avUp is the up direction of the camera
  //! \param afFovY is the vertical field of view of the camera, in radians
  //! \param afAspect is the aspect ratio of the projection (width/height)
  //! \param afNear is the distance of the near clip plane
  //! \param afFar is the distance of the far clip plane
  virtual void __stdcall SetLookAtMatrices(tBool abSetCameraMatrices,
                                           const sVec3f& avEye, const sVec3f& avAt, const sVec3f& avUp,
                                           tF32 afFovY, tF32 afAspect, tF32 afNear, tF32 afFar) = 0;
  //! Set an orthographic projection in the view and projection matrices.
  //! \param abSetCameraMatrices if true the camera and render matrices will be set, if false only the render matrices will be set.
  //! \param arectViewport defines the orthographic projection area.
  //! \param afNear is the distance of the near clip plane
  //! \param afFar is the distance of the far clip plane
  //! \remark Takes in account ni::eGraphicsCaps_OrthoProjectionOffset
  //! \remark This is meant to be used to do ortho projections at the screen's resolution. To do generic ortho projection
  //!     generate your ortho projection matrix using MatrixOrthoOffCenterLH (for example) and set the view
  //!     matrix to identity.
  virtual void __stdcall SetOrthoMatrices(tBool abSetCameraMatrices, const sRectf& arectViewport, tF32 afNear, tF32 afFar) = 0;
  //! @}

  //! Get the states description structure pointer.
  //! {Property}
  virtual tPtr __stdcall GetDescStructPtr() const = 0;

  //! Serialize the states.
  virtual ni::tBool __stdcall SerializeDataTable(ni::iDataTable* apDT, tSerializeFlags aFlags) = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IGRAPHICSSTATES_33130606_H__
