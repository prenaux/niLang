#ifndef __GPUENUM_456789789_H__
#define __GPUENUM_456789789_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */

//! GPU Index type
enum eGpuIndexType
{
  //! 16-bit unsigned integer
  eGpuIndexType_U16 = 0,
  //! 32-bit unsigned integer
  eGpuIndexType_U32 = 1,
  //! \internal
  eGpuIndexType_Last niMaybeUnused = 2,
  //! \internal
  eGpuIndexType_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! GPU Buffer memory mode
enum eGpuBufferMemoryMode
{
  //! Default shared memory between CPU & GPU, best for dynamic resources
  //! that are updated frequently by the CPU.
  eGpuBufferMemoryMode_Shared = 0,
  //! Memory only accessible by GPU, optimal for static resources
  //! that won't change after initialization.
  //! \remark Lock() with private buffers always fails and return nullptr. You should use CreateGpuBufferFromData to initialize them.
  eGpuBufferMemoryMode_Private = 1,
  //! Defines a synchronized memory pair for a resource, with one copy in
  //! system memory and another in video memory. Managed resources benefit
  //! from fast CPU and GPU access to each copy of the resource, with minimal
  //! API calls needed to synchronize these copies.
  //! \remark Lock() returns a pointer to the system memory, Unlock synchronizes it with the GPU.
  //! \remark To copy the GPU version of the buffer to the system memory you must use iGraphicsDriverGpu::BlitGpuBufferToSystemMemory().
  eGpuBufferMemoryMode_Managed = 2,

  //! \internal
  eGpuBufferMemoryMode_Last niMaybeUnused = 3,
  //! \internal
  eGpuBufferMemoryMode_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! GPU Buffer usage flags
enum eGpuBufferUsageFlags
{
  //! Can be used as vertex buffer
  eGpuBufferUsageFlags_Vertex = niBit(0),
  //! Can be used as index buffer
  eGpuBufferUsageFlags_Index = niBit(1),
  //! Can be used as uniform buffer
  eGpuBufferUsageFlags_Uniform = niBit(2),
  //! Can be used as storage buffer
  eGpuBufferUsageFlags_Storage = niBit(3),
  //! Can be used as indirect buffer
  eGpuBufferUsageFlags_Indirect = niBit(4),
  //! Source for transfer operations
  eGpuBufferUsageFlags_TransferSrc = niBit(5),
  //! Destination for transfer operations
  eGpuBufferUsageFlags_TransferDst = niBit(6),

  //! \internal
  eGpuBufferUsageFlags_Last niMaybeUnused = 8,
  //! \internal
  eGpuBufferUsageFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! GPU Function type
enum eGpuFunctionType
{
  //! Vertex function
  eGpuFunctionType_Vertex = 0,
  //! Pixel/Fragment function
  eGpuFunctionType_Pixel = 1,
  //! Compute function
  eGpuFunctionType_Compute = 2,
  //! Mesh function
  eGpuFunctionType_Mesh = 3,
  //! Task/Amplification function
  eGpuFunctionType_Task = 4,
  //! Ray generation function
  eGpuFunctionType_RayGeneration = 5,
  //! Ray intersection function
  eGpuFunctionType_RayIntersection = 6,
  //! Ray any hit function
  eGpuFunctionType_RayAnyHit = 7,
  //! Ray closest hit function
  eGpuFunctionType_RayClosestHit = 8,
  //! Ray miss function
  eGpuFunctionType_RayMiss = 9,

  //! \internal
  eGpuFunctionType_Last niMaybeUnused = 10,
  //! \internal
  eGpuFunctionType_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! GPU Function bind type
enum eGpuFunctionBindType {
  //! No resource bindings
  eGpuFunctionBindType_None = 0,
  //! Basic fixed-function pipeline, single base channel
  eGpuFunctionBindType_Fixed = 1,
  //! Full iMaterial pipeline with channels
  eGpuFunctionBindType_Material = 2,
  //! Global resource arrays
  eGpuFunctionBindType_Bindless = 3,
  //! \internal
  eGpuFunctionBindType_Last = 4,
};

//! GPU Pixel format
enum eGpuPixelFormat
{
  eGpuPixelFormat_None = 0,
  eGpuPixelFormat_BGRA8 = 1,
  eGpuPixelFormat_RGBA8 = 2,
  eGpuPixelFormat_RGBA16F = 3,
  eGpuPixelFormat_R16F = 4,
  eGpuPixelFormat_R32F = 5,
  eGpuPixelFormat_D32 = 6,
  eGpuPixelFormat_D16 = 7,
  eGpuPixelFormat_D24S8 = 8,

  //! \internal
  eGpuPixelFormat_Last niMaybeUnused = 9,
  //! \internal
  eGpuPixelFormat_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! GPU Blend factor
enum eGpuBlendFactor
{
  //! (0, 0, 0, 0)
  eGpuBlendFactor_Zero = 0,
  //! (1, 1, 1, 1)
  eGpuBlendFactor_One = 1,
  //! (Rs, Gs, Bs, As)
  eGpuBlendFactor_SrcColor = 2,
  //! (1-Rs, 1-Gs, 1-Bs, 1-As)
  eGpuBlendFactor_InvSrcColor = 3,
  //! (As, As, As, As)
  eGpuBlendFactor_SrcAlpha = 4,
  //! (1-As, 1-As, 1-As, 1-As)
  eGpuBlendFactor_InvSrcAlpha = 5,
  //! (Ad, Ad, Ad, Ad)
  eGpuBlendFactor_DstAlpha = 6,
  //! (1-Ad, 1-Ad, 1-Ad, 1-Ad)
  eGpuBlendFactor_InvDstAlpha = 7,
  //! (Rd, Gd, Bd, Ad)
  eGpuBlendFactor_DstColor = 8,
  //! (1-Rd, 1-Gd, 1-Bd, 1-Ad)
  eGpuBlendFactor_InvDstColor = 9,
  //! (f, f, f, 1) where f = min(As, 1-Ad)
  eGpuBlendFactor_SrcAlphaSat = 10,
  //! Constant blend color
  eGpuBlendFactor_BlendColorConstant = 11,
  //! 1 - Constant blend color
  eGpuBlendFactor_InvBlendColorConstant = 12,

  //! \internal
  eGpuBlendFactor_Last niMaybeUnused = 13,
  //! \internal
  eGpuBlendFactor_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! GPU Blend operation
enum eGpuBlendOp {
  eGpuBlendOp_Add = 0,
  eGpuBlendOp_Subtract = 1,
  eGpuBlendOp_ReverseSubtract = 2,
  eGpuBlendOp_Min = 3,
  eGpuBlendOp_Max = 4,

  //! \internal
  eGpuBlendOp_Last niMaybeUnused = 5,
  //! \internal
  eGpuBlendOp_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Ray function group type for hit functions
enum eRayGpuFunctionGroupType {
  //! Hit group for built-in triangle intersection
  eRayGpuFunctionGroupType_Triangles = 0,
  //! Hit group with custom intersection function
  eRayGpuFunctionGroupType_Procedural = 1,
  //! \internal
  eRayGpuFunctionGroupType_Last niMaybeUnused = 2,
  //! \internal
  eRayGpuFunctionGroupType_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//===========================================================================
//
//  Vulkan interfaces
//
//===========================================================================
enum eGLSLVulkanDescriptorSet {
  eGLSLVulkanDescriptorSet_Buffer = 0,
  eGLSLVulkanDescriptorSet_Texture2D = 1,
  eGLSLVulkanDescriptorSet_TextureCube = 2,
  eGLSLVulkanDescriptorSet_Texture3D = 3,
  eGLSLVulkanDescriptorSet_TextureShadow = 4,
  eGLSLVulkanDescriptorSet_Sampler = 5,
  eGLSLVulkanDescriptorSet_SamplerShadow = 6,
  eGLSLVulkanDescriptorSet_AccelerationStructure = 7,
  eGLSLVulkanDescriptorSet_Image2D = 8,
  //! \internal
  eGLSLVulkanDescriptorSet_Last niMaybeUnused = 9,
};

enum eGLSLVulkanVertexInputLayout {
  eGLSLVulkanVertexInputLayout_Position = 0,
  eGLSLVulkanVertexInputLayout_Weights = 1,
  eGLSLVulkanVertexInputLayout_Indices = 2,
  eGLSLVulkanVertexInputLayout_Normal = 3,
  eGLSLVulkanVertexInputLayout_ColorA = 4,
  eGLSLVulkanVertexInputLayout_Tex0 = 5,
  eGLSLVulkanVertexInputLayout_Tex1 = 6,
  eGLSLVulkanVertexInputLayout_Tex2 = 7,
  eGLSLVulkanVertexInputLayout_Tex3 = 8,
  eGLSLVulkanVertexInputLayout_Tex4 = 9,
  eGLSLVulkanVertexInputLayout_Tex5 = 10,
  eGLSLVulkanVertexInputLayout_Tex6 = 11,
  eGLSLVulkanVertexInputLayout_Tex7 = 12,
  //! \internal
  eGLSLVulkanVertexInputLayout_Last niMaybeUnused = 13
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __GRAPHICSENUM_456789789_H__
