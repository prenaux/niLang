#ifndef __GRAPHICSENUM_456789789_H__
#define __GRAPHICSENUM_456789789_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */

//! Graphics capabilites.
enum eGraphicsCaps
{
  //! Number of texture units.
  eGraphicsCaps_NumTextureUnits         = 0,
  //! Return eTrue if the renderer support run-time window resize.
  eGraphicsCaps_Resize                  = 1,
  //! Return eTrue if the renderer support the creation of more than one
  //! rendering context.
  eGraphicsCaps_MultiContext            = 2,
  //! Maximum 2d texture size supported. Return 0 if textures are not
  //! supported.
  eGraphicsCaps_Texture2DMaxSize        = 3,
  //! Maximum cube texture size supported. Return 0 if cube maps are not
  //! supported.
  eGraphicsCaps_TextureCubeMaxSize      = 4,
  //! Maximum 3d texture size supported. . Return 0 if 3d textures are not
  //! supported.
  eGraphicsCaps_Texture3DMaxSize        = 5,
  //! Return the number of simultaneous render target textures supported.
  //! \remark Return 0 if render target textures are not supported.
  eGraphicsCaps_NumRenderTargetTextures = 6,
  //! Return eTrue if the renderer supports depth stencil as texture.
  eGraphicsCaps_DepthStencilTexture     = 7,
  //! Return eTrue if the renderer supports two sided stencil.
  eGraphicsCaps_StencilTwoSideded       = 8,
  //! Return eTrue if the renderer supports Inc/Decr wrap stencil operation.
  eGraphicsCaps_StencilWrap             = 9,
  //! Return eTrue if the renderer supports occlusion queries.
  eGraphicsCaps_OcclusionQueries        = 10,
  //! Return the maximum point size. (tF32)
  eGraphicsCaps_MaxPointSize            = 11,
  //! Return wheter or not the hardware supports instancing.
  eGraphicsCaps_HardwareInstancing      = 12,
  //! Return wheter or not the hardware scissor test is supported.
  eGraphicsCaps_ScissorTest             = 13,
  //! The maximum vertex index that can be used in an index array.
  eGraphicsCaps_MaxVertexIndex          = 14,
  //! Overlay textures supported.
  eGraphicsCaps_OverlayTexture          = 15,
  //! Get the orthogonal screen projection offset in screen pixel
  //! unit. (float)
  //! \remark Used to build a proper otho projection matrix that matches the
  //! screen pixels exactly.
  eGraphicsCaps_OrthoProjectionOffset   = 16,
  //! Whether we can blit the backbuffer to a texture with BlitTextureToTexture.
  eGraphicsCaps_BlitBackBuffer          = 17,
  //! \internal
  eGraphicsCaps_Last                    = 18,
  //! \internal
  eGraphicsCaps_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Buffer clearing flags.
enum eClearBuffersFlags
{
  //! Clear color buffer.
  eClearBuffersFlags_Color = 0x1,
  //! Clear depth buffer.
  eClearBuffersFlags_Depth = 0x2,
  //! Clear stencil buffer.
  eClearBuffersFlags_Stencil = 0x4,
  //! Clear the color and depth buffers.
  eClearBuffersFlags_ColorDepth = eClearBuffersFlags_Color|eClearBuffersFlags_Depth,
  //! Clear the color and stencil buffers.
  eClearBuffersFlags_ColorStencil = eClearBuffersFlags_Color|eClearBuffersFlags_Stencil,
  //! Clear the depth and stencil buffers.
  eClearBuffersFlags_DepthStencil = eClearBuffersFlags_Depth|eClearBuffersFlags_Depth,
  //! Clear the color, depth and stencil buffers.
  eClearBuffersFlags_ColorDepthStencil = eClearBuffersFlags_Color|eClearBuffersFlags_Depth|eClearBuffersFlags_Stencil,
  //! \internal
  eClearBuffersFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Buffer clearing flags type.
typedef tU32 tClearBuffersFlags;

//! Array usage.
enum eArrayUsage
{
  //! Dynamic array, write only.
  eArrayUsage_Dynamic = 0,
  //! Dynamic array, Read/Write.
  eArrayUsage_DynamicReadWrite = 1,
  //! Static array, write only.
  eArrayUsage_Static = 2,
  //! System memory array.
  eArrayUsage_SystemMemory = 3,
  //! \internal
  eArrayUsage_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Lock.
enum eLock
{
  //! Normal behavior.
  eLock_Normal = 0,
  //! The application overwrites (with a write-only operation) every location within the
  //! region being locked. This is a valid option when using dynamic textures, dynamic
  //! vertex arrays, and dynamic index arrays.
  //!
  //! For vertex and index arrays, the application discards the entire buffer. A pointer to
  //! a new memory area is returned so that the dynamic memory access (DMA) and rendering
  //! from the previous area do not stall.
  //!
  //! For textures, the application overwrites (with a write-only operation) every location
  //! within the region being locked.
  eLock_Discard = niBit(0),
  //! The application promises not to overwrite any data in the vertex and index arrays.
  //! Specifying this flag allows the driver to return immediately and continue rendering,
  //! using this array. If this flag is not used, the driver must finish rendering before
  //! returning from locking.
  eLock_NoOverwrite = niBit(1),
  //! The application will not write to the buffer.
  eLock_ReadOnly = niBit(2),
  //! The application will read only the positions in the buffer.
  //! \remark This is meaningfull only for vertex arrays.
  eLock_ReadOnlyPosition = eLock_ReadOnly|niBit(3),
  //! \internal
  eLock_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Lock flags type.
typedef tU32 tLockFlags;

#if niMinFeatures(20)
//! Shader compiler flags.
enum eShaderCompilerFlags
{
  //! Insert debug filename, line numbers, and type and symbol information during shader compile.
  eShaderCompilerFlags_Debug = niBit(0),
  //! Do not validate the generated code against known capabilities and constraints. This
  //! option is recommended only when compiling shaders that are known to work (that is,
  //! shaders that have compiled before without this option). Shaders are always validated
  //! by the runtime before they are set to the device.
  eShaderCompilerFlags_SkipValidation = niBit(1),
  //! Instruct the compiler to skip optimization steps during code generation. Unless you
  //! are trying to isolate a problem in your code and you suspect the compiler, using this
  //! option is not recommended.
  eShaderCompilerFlags_SkipOptimization = niBit(2),
  //! Unless explicitly specified, matrices will be packed in row-major order (each
  //! vector will be in a single row) when passed to or from the shader.
  eShaderCompilerFlags_PackMatrixRowMajor = niBit(3),
  //! Unless explicitly specified, matrices will be packed in column-major order (each
  //! vector will be in a single column) when passed to and from the shader. This is
  //! generally more efficient because it allows vector-matrix multiplication to be
  //! performed using a series of dot products.
  eShaderCompilerFlags_PackMatrixColumnMajor = niBit(4),
  //! Force all computations in the resulting shader to occur at partial precision.
  //! This may result in faster evaluation of shaders on some hardware.
  eShaderCompilerFlags_PartialPrecision = niBit(5),
  //! This is a hint to the compiler to avoid using flow-control instructions.
  eShaderCompilerFlags_AvoidFlowControl = niBit(6),
  //! This is a hint to the compiler to prefer using flow-control instructions.
  eShaderCompilerFlags_PreferFlowControl = niBit(7),
  //! This will instruct the compiler to keep the data necessary to serialize the
  //! shader to a file.
  eShaderCompilerFlags_Save = niBit(8),
  //! This will instruct the driver to keep a copy of the source code used to compile the shader.
  //! That can be retrieved with ni::iShader::GetCode().
  eShaderCompilerFlags_KeepCode = niBit(9),
  //! This will instruct the driver to log the macros and other debugging informations when
  //! compiling.
  eShaderCompilerFlags_DebugLog = niBit(10),
  //! \internal
  eShaderCompilerFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Shader compiler flags type.
typedef tU32 tShaderCompilerFlags;
#endif // niMinFeatures

//! Graphics primitive types
enum eGraphicsPrimitiveType
{
  //! List of isolated points.
  eGraphicsPrimitiveType_PointList = 0,
  //! List of isolated straight line segments.
  eGraphicsPrimitiveType_LineList = 1,
  //! List of vertices as a single polyline.
  eGraphicsPrimitiveType_LineStrip = 2,
  //! List of vertices as a sequence of isolated triangles. Each group of
  //! three vertices defines a separate triangle. Culling is affected by the current
  //! culling rendering parameter.
  eGraphicsPrimitiveType_TriangleList = 3,
  //! List of vertices as a triangle strip. The culling flag is automatically flipped on
  //! even-numbered triangles.
  eGraphicsPrimitiveType_TriangleStrip = 4,
  //! Line list with adjacency information.
  eGraphicsPrimitiveType_LineListAdjacency = 5,
  //! Line strip with adjacency information.
  eGraphicsPrimitiveType_LineStripAdjacency = 6,
  //! Triangle list with adjacency information.
  eGraphicsPrimitiveType_TriangleListAdjacency = 7,
  //! Triangle strip with adjacency information.
  eGraphicsPrimitiveType_TriangleStripAdjacency = 8,
  //! List of screen aligned rectangle blitting.
  //! \remark The vertex format used is Position|Tex1(3)|ColorA.
  //!     Two vertices define one rectangle blitting (topLeft/bottomRight),
  //!     the z coordinate of the texture define the slice/texture to use
  //!     for the blitting.
  //! \remark Blit list is a special type of screen aligned only blitting,
  //!     it's mean to be used to render GUI elements, a 3D texture can
  //!     be used to pack several pages of textures.
  eGraphicsPrimitiveType_BlitList = 9,
  //! \internal
  eGraphicsPrimitiveType_Last = 10,
  //! \internal
  eGraphicsPrimitiveType_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Graphics driver implementation details.
enum eGraphicsDriverImplFlags {
  //! Vertex array.
  eGraphicsDriverImplFlags_VertexArrayObject = niBit(0),
  //! Index array.
  eGraphicsDriverImplFlags_IndexArrayObject = niBit(1),
  //! Shader constants.
  eGraphicsDriverImplFlags_ShaderConstantsObject = niBit(2),
  //! Can render using system memory textures.
  eGraphicsDriverImplFlags_SystemMemoryTexture = niBit(3),
  //! Compile sampler states.
  eGraphicsDriverImplFlags_CompileSamplerStates = niBit(4),
  //! Compile DepthStencil states.
  eGraphicsDriverImplFlags_CompileDepthStencilStates = niBit(5),
  //! Compile rasterizer states.
  eGraphicsDriverImplFlags_CompileRasterizerStates = niBit(6),
  //! \internal
  eGraphicsDriverImplFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Graphics driver implementation details.
//! \sa eGraphicsDriverObject
typedef tU32 tGraphicsDriverImplFlags;

//! Sampler filter.
enum eSamplerFilter {
  //! Default, smooth filtering.
  eSamplerFilter_Smooth = 0,
  //! Sharper filtering. Anisotropic filtering and/or a negative lod bias.
  eSamplerFilter_Sharp = 1,
  //! Point filtering.
  eSamplerFilter_Point = 2,
  //! Sharper point filtering. Anisotropic filtering and/or a negative lod bias.
  eSamplerFilter_SharpPoint = 3,
  //! \internal
  eSamplerFilter_Last = 4,
  //! \internal
  eSamplerFilter_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Texture wrapping modes.
enum eSamplerWrap
{
  //! The texture is repeated along the axis. default.
  eSamplerWrap_Repeat = 0,
  //! The texture is clamped at the end and the rest of the polygon
  //! is filled with the content of the texture edge.
  eSamplerWrap_Clamp  = 1,
  //! Similar to eSamplerWrap_Repeat, except that the texture is flipped at every
  //! integer junction. For u values between 0 and 1, for example, the texture is
  //! addressed normally; between 1 and 2, the texture is flipped (mirrored);
  //! between 2 and 3, the texture is normal again, and so on.
  eSamplerWrap_Mirror = 2,
  //! Border mode, the border color is used when outside of 0,1 range.
  eSamplerWrap_Border = 3,
  //! \internal
  eSamplerWrap_Last = 4,
  //! \internal
  eSamplerWrap_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Graphics compare functions.
enum eGraphicsCompare
{
  //! Always fail the test.
  eGraphicsCompare_Never = 0,
  //! Accept the new pixel if its value equals the value of the current pixel.
  eGraphicsCompare_Equal = 1,
  //! Accept the new pixel if its value does not equal the value of the current pixel.
  eGraphicsCompare_NotEqual = 2,
  //! Accept the new pixel if its value is less than the value of the current pixel.
  eGraphicsCompare_Less = 3,
  //! Accept the new pixel if its value is less than or equal to the value of the current pixel.
  eGraphicsCompare_LessEqual = 4,
  //! Accept the new pixel if its value is greater than the value of the current pixel.
  eGraphicsCompare_Greater = 5,
  //! Accept the new pixel if its value is greater than or equal to the value of the current pixel.
  eGraphicsCompare_GreaterEqual = 6,
  //! Always pass the test.
  eGraphicsCompare_Always = 7,
  //! \internal
  eGraphicsCompare_Last = 8,
  //! \internal
  eGraphicsCompare_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Stencil mode.
enum eStencilMode
{
  //! Disable the stencil.
  eStencilMode_None = 0,
  //! One sided stencil.
  eStencilMode_OneSided = 1,
  //! Two sided stencil.
  //! \remark Will fail if the renderer doesn't support two sided stencil. \see eRendererCaps
  eStencilMode_TwoSided = 2,
  //! \internal
  eStencilMode_Last = 3,
  //! \internal
  eStencilMode_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Stencil operations.
enum eStencilOp
{
  //! Do not update the entry in the stencil buffer. This is the default value.
  eStencilOp_Keep = 0,
  //! Set the stencil-buffer entry to 0.
  eStencilOp_Zero = 1,
  //! Replace the stencil-buffer entry with a reference value.
  eStencilOp_Replace = 2,
  //! Increment the stencil-buffer entry, wrapping to zero if the new value exceeds the maximum value.
  eStencilOp_IncrWrap = 3,
  //! Decrement the stencil-buffer entry, wrapping to the maximum value if the new value is less than zero.
  eStencilOp_DecrWrap = 4,
  //! Increment the stencil-buffer entry, clamping to the maximum value.
  eStencilOp_IncrSat = 5,
  //! Increment the stencil-buffer entry, clamping to zero.
  eStencilOp_DecrSat = 6,
  //! Invert the bits in the stencil-buffer entry.
  eStencilOp_Invert = 7,
  //! \internal
  eStencilOp_Last = 8,
  //! \internal
  eStencilOp_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Blending mode
//! A*SrcColor + B*DestColor
//! \remark For material channel zero : SrcColor/Alpha is the channel 0 color, DestColor/Alpha is the color A
//! \remark For material channel > zero : SrcColor/Alpha is the channel n color, DestColor/Alpha is the channel n-1 color
enum eBlendMode
{
  //! Solid, no blending
  eBlendMode_NoBlending     = 0,
  //! Means that the current alpha will be replaced by the current stage's alpha.
  //! \remark For first stage only Final = Src*0 + Dest*SrcAlpha.
  eBlendMode_ReplaceAlpha     = 1,
  //! Additive blending
  //! \remark Final = Src*1 + Dest*1
  eBlendMode_Additive       = 2,
  //! Modulate
  //! \remark Final = Src*Dest + Dest*0
  eBlendMode_Modulate       = 3,
  //! Modulate 2x
  //! \remark Final = (Src*Dest + Dest*Src)
  eBlendMode_Modulate2x     = 4,
  //! Translucent blending
  //! \remark Final = (Src*SrcAlpha + Dest*(1-SrcAlpha))
  eBlendMode_Translucent      = 5,
  //! Translucent blending with inverted alpha
  //! \remark Final = (Src*(1-SrcAlpha) + Dest*SrcAlpha)
  eBlendMode_TranslucentInvAlpha  = 6,
  //! Tinted blending
  //! \remark Final = (Src*Src + Dest*(1-SrcColor))
  eBlendMode_TintedGlass      = 7,
  //! Premultiplied alpha light blending.
  //! \remark Final = (Src*1 + Dest*(1-SrcAlpha))
  eBlendMode_PreMulAlpha          = 8,
  //! Modulate RGB, replace alpha
  //! \remark Final.RGB = Src*Dest + Dest*0, Final.Alpha = Channel.Alpha
  eBlendMode_ModulateReplaceAlpha = 9,
  //! Modulate RGB 2x, replace alpha
  //! \remark Final.RGB = (Src*Dest + Dest*Src), Final.Alpha = Channel.Alpha
  eBlendMode_Modulate2xReplaceAlpha = 10,
  //! \internal
  eBlendMode_Last = 11,
  //! \internal
  eBlendMode_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Culling mode
enum eCullingMode
{
  //! Do not cull back faces.
  eCullingMode_None = 0,
  //! Cull back faces with clockwise vertices.
  eCullingMode_CW = 1,
  //! Cull back faces with counter clockwise vertices (default).
  eCullingMode_CCW = 2,
  //! \internal
  eCullingMode_Last = 3,
  //! \internal
  eCullingMode_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Color write mask.
enum eColorWriteMask
{
  //! Disable writting in the color buffer.
  eColorWriteMask_None = 0x0,
  //! Write only in the alpha buffer.
  eColorWriteMask_Alpha = 0x1,
  //! Write only in the red channel of the color buffer.
  eColorWriteMask_Red = 0x2,
  //! Write only in the green channel of the color buffer.
  eColorWriteMask_Green = 0x4,
  //! Write only in the blue channel of the color buffer.
  eColorWriteMask_Blue = 0x8,
  //! Write only in the rgb buffer.
  eColorWriteMask_RGB = eColorWriteMask_Red|eColorWriteMask_Green|eColorWriteMask_Blue,
  //! Normal, write in all channels.
  eColorWriteMask_All = eColorWriteMask_RGB|eColorWriteMask_Alpha,
  //! \internal
  eColorWriteMask_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Graphics display flags
enum eGraphicsDisplayFlags
{
  //! Flush all rendering but do not show the back buffer.
  eGraphicsDisplayFlags_Skip = niBit(0),
  eGraphicsDisplayFlags_DoNotWait = niBit(1),
  eGraphicsDisplayFlags_LinearContent = niBit(2),
  eGraphicsDisplayFlags_Transparent = niBit(3),
  eGraphicsDisplayFlags_Translucent = niBit(4),
  //! \internal
  eGraphicsDisplayFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Graphics display flags type. \see ni::eGraphicsDisplayFlags
typedef tU32 tGraphicsDisplayFlags;

//! Graphics built-in compiled states.
//! \see GetCompiledStates
enum eCompiledStates {
  eCompiledStates_Invalid = 0,

  eCompiledStates_RS_ColorWriteNone = 1,
  eCompiledStates_RS_Filled = 2,
  eCompiledStates_RS_Wireframe = 3,
  eCompiledStates_RS_FilledScissor = 4,
  eCompiledStates_RS_WireframeScissor = 5,
  eCompiledStates_RS_NoCullingColorWriteNone = 6,
  eCompiledStates_RS_NoCullingFilled = 7,
  eCompiledStates_RS_NoCullingWireframe = 8,
  eCompiledStates_RS_NoCullingFilledScissor = 9,
  eCompiledStates_RS_NoCullingWireframeScissor = 10,
  eCompiledStates_RS_CWCullingColorWriteNone = 11,
  eCompiledStates_RS_CWCullingFilled = 12,
  eCompiledStates_RS_CWCullingWireframe = 13,
  eCompiledStates_RS_CWCullingFilledScissor = 14,
  eCompiledStates_RS_CWCullingWireframeScissor = 15,

  eCompiledStates_DS_NoDepthTest = 101,
  eCompiledStates_DS_DepthTestAndWrite = 102,
  eCompiledStates_DS_DepthTestOnly = 103,

  eCompiledStates_SS_PointRepeat = 201,
  eCompiledStates_SS_PointClamp = 202,
  eCompiledStates_SS_PointMirror = 203,
  eCompiledStates_SS_SmoothRepeat = 204,
  eCompiledStates_SS_SmoothClamp = 205,
  eCompiledStates_SS_SmoothMirror = 206,
  eCompiledStates_SS_SharpRepeat = 207,
  eCompiledStates_SS_SharpClamp = 208,
  eCompiledStates_SS_SharpMirror = 209,
  eCompiledStates_SS_SharpPointRepeat = 210,
  eCompiledStates_SS_SharpPointClamp = 211,
  eCompiledStates_SS_SharpPointMirror = 212,

  //! Driver compiled
  eCompiledStates_Driver = 1000,
  //! \internal
  eCompiledStates_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __GRAPHICSENUM_456789789_H__
