// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"

#ifdef GDRV_VULKAN

#include "API/niUI_ModuleDef.h"
#include <niLang/Math/MathMatrix.h>
#include <niLang_ModuleDef.h>
#include <niUI/GraphicsEnum.h>
#include <niUI/FVF.h>
#include <niLang/STL/set.h>

#include <niUI/IVertexArray.h>
#include <niUI/IIndexArray.h>
#include <niUI/IGraphics.h>
#include <niUI/IGpu.h>
#include <niLang/Utils/IDGenerator.h>

#define niVulkan_Implement
#include "../../thirdparty/VulkanUtils/niVulkan.h"
#define niVulkanMemoryAllocator_Implement
#include "../../thirdparty/VulkanUtils/niVulkanMemoryAllocator.h"
#include "../../thirdparty/VulkanUtils/niVulkanEnumToString.h"

#if defined niOSX
#include "../../thirdparty/VulkanUtils/niVulkanOSXMetal.h"
#elif defined niLinux
#include <niLang/Platforms/Linux/linuxgl.h>
#endif

#include "GDRV_Gpu.h"
#include "GDRV_Utils.h"
#include "../../../data/test/gpufunc/TestGpuFuncs.hpp"

namespace ni {

_HDecl(__vktex_white__);
_HDecl(__vkbuff_dummy__);

niLetK knVulkanMaxFramesInFlight = 1_u32;
niLetK kfVulkanSamplerFilterAnisotropy = 8.0_f32;
static const char* const _vkRequiredDeviceExtensions[] = {
  VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
  VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
#ifdef niLinux
  VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#endif
};
niLetK knVkRequiredDeviceExtensionsCount = (tU32)niCountOf(_vkRequiredDeviceExtensions);
niLetK knVulkanMaxDescriptorSets = 10000_u32;

#define VULKAN_TRACE(aFmt) //niDebugFmt(aFmt)

#define NISH_VULKAN_TARGET spv_vk12

_HDecl(NISH_VULKAN_TARGET);
static niInline iHString* _GetVulkanGpuFunctionTarget() {
  return _HC(NISH_VULKAN_TARGET);
}

static VkFormat _GetVulkanPixelFormat(eGpuPixelFormat aFormat) {
  switch (aFormat) {
    case eGpuPixelFormat_None:
      return VK_FORMAT_UNDEFINED;
    case eGpuPixelFormat_BGRA8:
      return VK_FORMAT_B8G8R8A8_UNORM;
    case eGpuPixelFormat_RGBA8:
      return VK_FORMAT_R8G8B8A8_UNORM;
    case eGpuPixelFormat_RGBA16F:
      return VK_FORMAT_R16G16B16A16_SFLOAT;
    case eGpuPixelFormat_R16F:
      return VK_FORMAT_R16_SFLOAT;
    case eGpuPixelFormat_R32F:
      return VK_FORMAT_R32_SFLOAT;
    case eGpuPixelFormat_D32:
      return VK_FORMAT_D32_SFLOAT;
    case eGpuPixelFormat_D16:
      return VK_FORMAT_D16_UNORM;
    case eGpuPixelFormat_D24S8:
      return VK_FORMAT_D24_UNORM_S8_UINT;
  }
  return VK_FORMAT_UNDEFINED;
}

static const VkBlendFactor _ToVkBlendFactor[] = {
  VK_BLEND_FACTOR_ZERO,                      // eGpuBlendFactor_Zero
  VK_BLEND_FACTOR_ONE,                       // eGpuBlendFactor_One
  VK_BLEND_FACTOR_SRC_COLOR,                 // eGpuBlendFactor_SrcColor
  VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,       // eGpuBlendFactor_InvSrcColor
  VK_BLEND_FACTOR_SRC_ALPHA,                 // eGpuBlendFactor_SrcAlpha
  VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,       // eGpuBlendFactor_InvSrcAlpha
  VK_BLEND_FACTOR_DST_ALPHA,                 // eGpuBlendFactor_DstAlpha
  VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,       // eGpuBlendFactor_InvDstAlpha
  VK_BLEND_FACTOR_DST_COLOR,                 // eGpuBlendFactor_DstColor
  VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,       // eGpuBlendFactor_InvDstColor
  VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,        // eGpuBlendFactor_SrcAlphaSat
  VK_BLEND_FACTOR_CONSTANT_COLOR,            // eGpuBlendFactor_BlendColorConstant
  VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,  // eGpuBlendFactor_InvBlendColorConstant
};
niCAssert(niCountOf(_ToVkBlendFactor) == eGpuBlendFactor_Last);

static const VkBlendOp _ToVkBlendOp[] = {
  VK_BLEND_OP_ADD,              // eGpuBlendOp_Add
  VK_BLEND_OP_SUBTRACT,         // eGpuBlendOp_Subtract
  VK_BLEND_OP_REVERSE_SUBTRACT, // eGpuBlendOp_ReverseSubtract
  VK_BLEND_OP_MIN,             // eGpuBlendOp_Min
  VK_BLEND_OP_MAX,             // eGpuBlendOp_Max
};
niCAssert(niCountOf(_ToVkBlendOp) == eGpuBlendOp_Last);

static const VkIndexType _ToVkIndexType[] = {
  VK_INDEX_TYPE_UINT16,
  VK_INDEX_TYPE_UINT32,
};
niCAssert(niCountOf(_ToVkIndexType) == eGpuIndexType_Last);

static const VkPrimitiveTopology _ToVkPrimitiveTopology[] = {
  VK_PRIMITIVE_TOPOLOGY_POINT_LIST,       // eGraphicsPrimitiveType_PointList
  VK_PRIMITIVE_TOPOLOGY_LINE_LIST,        // eGraphicsPrimitiveType_LineList
  VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,       // eGraphicsPrimitiveType_LineStrip
  VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,    // eGraphicsPrimitiveType_TriangleList
  VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,   // eGraphicsPrimitiveType_TriangleStrip
};
niCAssert(niCountOf(_ToVkPrimitiveTopology) == eGraphicsPrimitiveType_Last);

static void _toVkSamplerFilter(VkSamplerCreateInfo& desc, eSamplerFilter aFilter, const VkPhysicalDeviceFeatures& aFeatures) {
  switch(aFilter) {
    case eSamplerFilter_Smooth: {
      desc.minFilter = VK_FILTER_LINEAR;
      desc.magFilter = VK_FILTER_LINEAR;
      desc.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
      if (aFeatures.samplerAnisotropy) {
        desc.maxAnisotropy = kfVulkanSamplerFilterAnisotropy;
        desc.anisotropyEnable = VK_TRUE;
      }
      else {
        desc.maxAnisotropy = 1.0f;
        desc.anisotropyEnable = VK_FALSE;
      }
      break;
    }
    case eSamplerFilter_Point: {
      desc.minFilter = VK_FILTER_NEAREST;
      desc.magFilter = VK_FILTER_NEAREST;
      desc.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
      desc.maxAnisotropy = 1.0f;
      desc.anisotropyEnable = VK_FALSE;
      break;
    }
  }
}

static const VkSamplerAddressMode _toVkSamplerAddress[] = {
  VK_SAMPLER_ADDRESS_MODE_REPEAT,
  VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
  VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
  VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
};
niCAssert(niCountOf(_toVkSamplerAddress) == eSamplerWrap_Last);

static VkCompareOp _ToVkCompareOp(eGraphicsCompare aCompare) {
  niCheck(aCompare < eGraphicsCompare_Last, VK_COMPARE_OP_ALWAYS);
  static const VkCompareOp _toVkCompareOp[] = {
    VK_COMPARE_OP_NEVER,
    VK_COMPARE_OP_EQUAL,
    VK_COMPARE_OP_NOT_EQUAL,
    VK_COMPARE_OP_LESS,
    VK_COMPARE_OP_LESS_OR_EQUAL,
    VK_COMPARE_OP_GREATER,
    VK_COMPARE_OP_GREATER_OR_EQUAL,
    VK_COMPARE_OP_ALWAYS,
  };
  niCAssert(niCountOf(_toVkCompareOp) == eGraphicsCompare_Last);
  return _toVkCompareOp[aCompare];
}

static VkStencilOp _ToVkStencilOp(eStencilOp aOp) {
  niCheck(aOp < eStencilOp_Last, VK_STENCIL_OP_KEEP);
  static const VkStencilOp _toVkStencilOp[] = {
    VK_STENCIL_OP_KEEP,
    VK_STENCIL_OP_ZERO,
    VK_STENCIL_OP_REPLACE,
    VK_STENCIL_OP_INCREMENT_AND_WRAP,
    VK_STENCIL_OP_DECREMENT_AND_WRAP,
    VK_STENCIL_OP_INCREMENT_AND_CLAMP,
    VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    VK_STENCIL_OP_INVERT,
  };
  niCAssert(niCountOf(_toVkStencilOp) == eStencilOp_Last);
  return _toVkStencilOp[aOp];
}

niLetK _vkFrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

static VkCullModeFlags _ToVkCullMode(eCullingMode aCullMode) {
  niCheck(aCullMode < eCullingMode_Last, VK_CULL_MODE_NONE);
  static const VkCullModeFlags _toVkCullMode[] = {
    VK_CULL_MODE_NONE,              // eCullingMode_None
    VK_CULL_MODE_BACK_BIT,          // eCullingMode_CW
    VK_CULL_MODE_FRONT_BIT,         // eCullingMode_CCW
  };
  niCAssert(niCountOf(_toVkCullMode) == eCullingMode_Last);
  return _toVkCullMode[aCullMode];
}

static VkColorComponentFlags _ToVkColorWriteMask(eColorWriteMask aMask) {
  switch (aMask) {
    case eColorWriteMask_None:
      return 0;
    case eColorWriteMask_Alpha:
      return VK_COLOR_COMPONENT_A_BIT;
    case eColorWriteMask_Red:
      return VK_COLOR_COMPONENT_R_BIT;
    case eColorWriteMask_Green:
      return VK_COLOR_COMPONENT_G_BIT;
    case eColorWriteMask_Blue:
      return VK_COLOR_COMPONENT_B_BIT;
    case eColorWriteMask_RGB:
      return VK_COLOR_COMPONENT_R_BIT |
          VK_COLOR_COMPONENT_G_BIT |
          VK_COLOR_COMPONENT_B_BIT;
    case eColorWriteMask_All:
      return VK_COLOR_COMPONENT_R_BIT |
          VK_COLOR_COMPONENT_G_BIT |
          VK_COLOR_COMPONENT_B_BIT |
          VK_COLOR_COMPONENT_A_BIT;
  }
  return 0;
}

static astl::vector<VkVertexInputAttributeDescription> Vulkan_CreateVertexInputDesc(tFVF aFVF) {
  astl::vector<VkVertexInputAttributeDescription> attrs;
  cFVFDescription fvfDesc(aFVF);

  if (fvfDesc.HasPosition()) {
    attrs.push_back({
        .location = eGLSLVulkanVertexInputLayout_Position,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = (uint32_t)fvfDesc.GetPositionOffset()
      });

    if (fvfDesc.HasWeights4()) {
      attrs.push_back({
          .location = eGLSLVulkanVertexInputLayout_Weights,
          .binding = 0,
          .format = VK_FORMAT_R32G32B32A32_SFLOAT,
          .offset = (uint32_t)fvfDesc.GetWeightsOffset()
        });
    }
  }

  if (fvfDesc.HasIndices()) {
    attrs.push_back({
        .location = eGLSLVulkanVertexInputLayout_Indices,
        .binding = 0,
        .format = VK_FORMAT_B8G8R8A8_UNORM,
        .offset = (uint32_t)fvfDesc.GetIndicesOffset()
      });
  }

  if (fvfDesc.HasNormal()) {
    attrs.push_back({
        .location = eGLSLVulkanVertexInputLayout_Normal,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = (uint32_t)fvfDesc.GetNormalOffset()
      });
  }

  if (fvfDesc.HasColorA()) {
    attrs.push_back({
        .location = eGLSLVulkanVertexInputLayout_ColorA,
        .binding = 0,
        .format = VK_FORMAT_B8G8R8A8_UNORM,
        .offset = (uint32_t)fvfDesc.GetColorAOffset()
      });
  }

  for (tU32 i = 0; i < fvfDesc.GetNumTexCoos(); ++i) {
    const tU32 dim = fvfDesc.GetTexCooDim(i);
    VkFormat format;
    switch (dim) {
      case 1:
        format = VK_FORMAT_R32_SFLOAT;
        break;
      case 2:
        format = VK_FORMAT_R32G32_SFLOAT;
        break;
      case 3:
        format = VK_FORMAT_R32G32B32_SFLOAT;
        break;
      case 4:
        format = VK_FORMAT_R32G32B32A32_SFLOAT;
        break;
      default:
        continue;
    }
    attrs.push_back({
        .location = eGLSLVulkanVertexInputLayout_Tex0 + i,
        .binding = 0,
        .format = format,
        .offset = fvfDesc.GetTexCooOffset(i)
      });
  }
  return attrs;
}

static tBool _VulkanTransitionImageLayout(
  VkCommandBuffer aCmdBuffer, VkImage aImage,
  VkImageLayout aOldLayout, VkImageLayout aNewLayout,
  tU32 aBaseMipLevel = 0,
  tU32 aBaseArrayLayer = 0)
{
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = aOldLayout;
  barrier.newLayout = aNewLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = aImage;
  barrier.subresourceRange = {
    .baseMipLevel = aBaseMipLevel,
    .levelCount = 1,
    .baseArrayLayer = aBaseArrayLayer,
    .layerCount = 1
  };

  if (aOldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
      aNewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
  {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  }
  else {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  switch (aOldLayout) {
    case VK_IMAGE_LAYOUT_UNDEFINED:
      barrier.srcAccessMask = 0;
      sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      break;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      break;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      break;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
      barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
      break;
    default:
      niError(niFmt("Unsupported old layout transition '%s'.",ni_vulkan::VkImageLayoutToString(aOldLayout)));
      return eFalse;
  }

  switch (aNewLayout) {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      break;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      break;
      break;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
      barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
      break;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      break;
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
      barrier.dstAccessMask = 0;
      destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
      break;
    default:
      niError(niFmt("Unsupported new layout transition '%s'.",ni_vulkan::VkImageLayoutToString(aNewLayout)));
      return eFalse;
  }

  if ((aOldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
       aNewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) ||
      (aOldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
       (aNewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ||
        aNewLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ||
        aNewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)) ||
      (aOldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
       (aNewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ||
        aNewLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)) ||
      (aOldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL &&
       (aNewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
  ) {
    vkCmdPipelineBarrier(
      aCmdBuffer,
      sourceStage, destinationStage,
      0,
      0, nullptr,
      0, nullptr,
      1, &barrier
    );
    return eTrue;
  }
  else {
    niError(niFmt("Unsupported layout transition '%s' -> '%s'.",
                  ni_vulkan::VkImageLayoutToString(aOldLayout),
                  ni_vulkan::VkImageLayoutToString(aNewLayout)));
    return eFalse;
  }
}

struct sVulkanBuffer;

struct sVulkanDriver : public ImplRC<iGraphicsDriver,eImplFlags_Default,iGraphicsDriverGpu> {
  nn<iGraphics> _graphics;
  VkDevice _device = VK_NULL_HANDLE;
  VmaAllocator _allocator = nullptr;
  VkQueue _graphicsQueue = VK_NULL_HANDLE;
  VkCommandPool _commandPool = VK_NULL_HANDLE;

  VkInstance _instance = VK_NULL_HANDLE;
  VkPhysicalDeviceFeatures _physicalDeviceFeatures = {};
  // VkPhysicalDevice will be implicitly destroyed when the VkInstance is destroyed.
  VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
  typedef astl::map<cString,tU32> tVkExtensionsMap;
  tVkExtensionsMap _extensions;
  tBool _enableValidationLayers = eTrue;
  typedef astl::set<cString> tVkInstanceLayersSet;
  tVkInstanceLayersSet _instanceLayers;
  tU32 _queueFamilyIndex = 0;

  LocalIDGenerator _idGenerator;
  VkSampler _ssCompiled[(eCompiledStates_SS_SmoothWhiteBorder-eCompiledStates_SS_PointRepeat)+1];
  Ptr<sVulkanBuffer> _dummyUniformBuffer;

  Ptr<iGraphicsDrawOpCapture> _drawOpCapture;
  Ptr<iFixedGpuPipelines> _fixedPipelines;

  sVulkanDriver(ain<nn<iGraphics>> aGraphics)
      : _graphics(aGraphics)
  {
  }

  tBool __stdcall _CreateVulkanDriver(const achar* aAppName) {
    niCheck(_CreateInstance(aAppName),eFalse);
    niCheck(_InitPhysicalDevice(),eFalse);
    niCheck(_FindQueueFamily(_queueFamilyIndex), eFalse);
    niLog(Info,niFmt("Vulkan using Queue Family: %d",_queueFamilyIndex));
    niCheck(_CreateLogicalDevice(), eFalse);
    niCheck(_CreateCommandPool(), eFalse);
    niCheck(_CreateAllocator(), eFalse);
    niCheck(_CreateVulkanDriverResources(), eFalse) ;
    return eTrue;
  }

  virtual ~sVulkanDriver() {
    _fixedPipelines = nullptr;

    _dummyUniformBuffer = nullptr;
    niLoop(i,niCountOf(_ssCompiled)) {
      if (_ssCompiled[i]) {
        vkDestroySampler(_device, _ssCompiled[i],
                         nullptr);
        _ssCompiled[i] = VK_NULL_HANDLE;
      }
    }

    if (_commandPool) {
      vkDestroyCommandPool(_device, _commandPool, nullptr);
      _commandPool = VK_NULL_HANDLE;
    }

    if (_allocator != nullptr) {
      vmaDestroyAllocator(_allocator);
      _allocator = nullptr;
    }

    if (_device) {
      vkDestroyDevice(_device, nullptr);
      _device = VK_NULL_HANDLE;
    }
    if (_instance) {
      vkDestroyInstance(_instance, nullptr);
      _instance = VK_NULL_HANDLE;
    }
  }

  virtual tBool __stdcall IsOK() const niImpl {
    return _device != VK_NULL_HANDLE;
  }
  virtual void __stdcall Invalidate() niImpl {
  }

  static VKAPI_ATTR VkBool32 VKAPI_CALL _DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* /*pUserData*/) {

    switch (severity) {
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        niLog(Error,niFmt("Vulkan: %s", pCallbackData->pMessage));
        break;
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        niLog(Warning,niFmt("Vulkan: %s", pCallbackData->pMessage));
        break;
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        niLog(Info,niFmt("Vulkan: %s", pCallbackData->pMessage));
        break;
      default:
        niLog(Debug,niFmt("Vulkan: %s", pCallbackData->pMessage));
        break;
    }
    return VK_FALSE;
  }

  tBool _CreateInstance(const achar* aAppName) {
#ifdef niVulkan_Volk
    VK_CHECK(volkInitialize(),eFalse);
#endif

    {
      tU32 layerCount;
      vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
      astl::vector<VkLayerProperties> availableLayers(layerCount);
      vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
      for (const auto& layer : availableLayers) {
        _instanceLayers.insert(layer.layerName);
      }
      {
        cString o;
        niLoopit(tVkInstanceLayersSet::const_iterator,it,_instanceLayers) {
          if (it != _instanceLayers.begin())
            o << ", ";
          o << *it;
        }
        niLog(Info,niFmt("Vulkan instance layers[%d]: %s", _instanceLayers.size(), o));
      }
    }

    astl::vector<const char*> extensions = {
      VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    };

#if defined niLinux
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif

    astl::vector<const char*> layers;
    if (_enableValidationLayers) {
      // NOTE: Statically linking directly MoltenVK we cannot use the
      // validation layer.
      const achar* validationLayerName = "VK_LAYER_KHRONOS_validation";
      if (!astl::contains(_instanceLayers,validationLayerName)) {
        _enableValidationLayers = eFalse;
        niWarning(niFmt("Vulkan validation layer '%s' not found, disabling it.",
                        validationLayerName));
      }
      else {
        layers.push_back(validationLayerName);
      }
    }

    VkApplicationInfo appInfo = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = aAppName,
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "niVlk",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_API_VERSION_1_2
    };

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .messageSeverity = (VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT|
                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT|
                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT),
      .messageType = (VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT|
                      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT|
                      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT),
      .pfnUserCallback = _DebugCallback
    };

    VkInstanceCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = nullptr,
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = (tU32)layers.size(),
      .ppEnabledLayerNames = layers.data(),
      .enabledExtensionCount = (tU32)extensions.size(),
      .ppEnabledExtensionNames = extensions.data()
    };
    createInfo.pNext = _enableValidationLayers ? &debugCreateInfo : nullptr;

    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &_instance),eFalse);
#ifdef niVulkan_Volk
    volkLoadInstance(_instance);
#endif
    return eTrue;
  }

  void _InitDeviceFeatures2(VkPhysicalDevice physicalDevice) {
    // Initialize structures to query ray tracing and mesh shader features
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures = {};
    rayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;

    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures = {};
    accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    rayTracingPipelineFeatures.pNext = &accelerationStructureFeatures;

    VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures = {};
    meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
    accelerationStructureFeatures.pNext = &meshShaderFeatures;

    VkPhysicalDeviceRobustness2FeaturesEXT robustness2Features = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
    };
    meshShaderFeatures.pNext = &robustness2Features;

    VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures2.pNext = &rayTracingPipelineFeatures;

    // Query the physical device features
    vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures2);

    // Determine support for ray tracing and mesh shaders
    bool isRayTracingSupported = rayTracingPipelineFeatures.rayTracingPipeline && accelerationStructureFeatures.accelerationStructure;
    if (isRayTracingSupported) {
      VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingProps = {};
      rayTracingProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

      VkPhysicalDeviceProperties2 deviceProps2 = {};
      deviceProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
      deviceProps2.pNext = &rayTracingProps;

      vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProps2);

      niLog(Info, niFmt(
        "Vulkan Ray Tracing Properties:\n"
        // Size in bytes of the shader group handle.
        "  shaderGroupHandleSize: %u\n"
        // Maximum number of levels of ray recursion allowed in a trace command.
        "  maxRayRecursionDepth: %u\n"
        // Maximum stride in bytes allowed between shader groups in the shader binding table.
        "  maxShaderGroupStride: %u\n"
        // Required alignment in bytes for the base of the shader binding table.
        "  shaderGroupBaseAlignment: %u\n"
        // Size in bytes of the shader group handle for capture and replay.
        "  shaderGroupHandleCaptureReplaySize: %u",
        rayTracingProps.shaderGroupHandleSize,
        rayTracingProps.maxRayRecursionDepth,
        rayTracingProps.maxShaderGroupStride,
        rayTracingProps.shaderGroupBaseAlignment,
        rayTracingProps.shaderGroupHandleCaptureReplaySize));
    }
    else {
      niLog(Info, "Vulkan Ray Tracing not supported.");
    }

    if (meshShaderFeatures.meshShader) {
      VkPhysicalDeviceMeshShaderPropertiesEXT meshShaderProps = {};
      meshShaderProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT;

      VkPhysicalDeviceProperties2 deviceProps2 = {};
      deviceProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
      deviceProps2.pNext = &meshShaderProps;

      vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProps2);

      niLog(Info, niFmt(
        "Vulkan Mesh Shader Properties:\n"
        // Maximum total count of task work groups.
        "  maxTaskWorkGroupTotalCount: %d\n"
        // Maximum count of task work groups in each dimension.
        "  maxTaskWorkGroupCount: %s\n"
        // Maximum number of task shader invocations in a single work group.
        "  maxTaskWorkGroupInvocations: %d\n"
        // Maximum size of task work group in each dimension.
        "  maxTaskWorkGroupSize: %s\n"
        // Maximum size in bytes of the task payload.
        "  maxTaskPayloadSize: %d\n"
        // Maximum size in bytes of task shared memory.
        "  maxTaskSharedMemorySize: %d\n"
        // Maximum total count of mesh work groups.
        "  maxMeshWorkGroupTotalCount: %d\n"
        // Maximum count of mesh work groups in each dimension.
        "  maxMeshWorkGroupCount: %s\n"
        // Maximum number of mesh shader invocations in a single work group.
        "  maxMeshWorkGroupInvocations: %d\n"
        // Maximum size of mesh work group in each dimension.
        "  maxMeshWorkGroupSize: %s\n"
        // Maximum size in bytes of mesh shared memory.
        "  maxMeshSharedMemorySize: %d\n"
        // Maximum number of mesh output vertices.
        "  maxMeshOutputVertices: %d\n"
        // Maximum number of mesh output primitives.
        "  maxMeshOutputPrimitives: %d\n"
        // Maximum number of mesh output layers.
        "  maxMeshOutputLayers: %d\n"
        // Maximum number of mesh multiview views.
        "  maxMeshMultiviewViewCount: %d\n"
        // Granularity of mesh output per vertex.
        "  meshOutputPerVertexGranularity: %d\n"
        // Granularity of mesh output per primitive.
        "  meshOutputPerPrimitiveGranularity: %d",
        meshShaderProps.maxTaskWorkGroupTotalCount,
        Vec3i(meshShaderProps.maxTaskWorkGroupCount[0], meshShaderProps.maxTaskWorkGroupCount[1], meshShaderProps.maxTaskWorkGroupCount[2]),
        meshShaderProps.maxTaskWorkGroupInvocations,
        Vec3i(meshShaderProps.maxTaskWorkGroupSize[0], meshShaderProps.maxTaskWorkGroupSize[1], meshShaderProps.maxTaskWorkGroupSize[2]),
        meshShaderProps.maxTaskPayloadSize,
        meshShaderProps.maxTaskSharedMemorySize,
        meshShaderProps.maxMeshWorkGroupTotalCount,
        Vec3i(meshShaderProps.maxMeshWorkGroupCount[0], meshShaderProps.maxMeshWorkGroupCount[1], meshShaderProps.maxMeshWorkGroupCount[2]),
        meshShaderProps.maxMeshWorkGroupInvocations,
        Vec3i(meshShaderProps.maxMeshWorkGroupSize[0], meshShaderProps.maxMeshWorkGroupSize[1], meshShaderProps.maxMeshWorkGroupSize[2]),
        meshShaderProps.maxMeshSharedMemorySize,
        meshShaderProps.maxMeshOutputVertices,
        meshShaderProps.maxMeshOutputPrimitives,
        meshShaderProps.maxMeshOutputLayers,
        meshShaderProps.maxMeshMultiviewViewCount,
        meshShaderProps.meshOutputPerVertexGranularity,
        meshShaderProps.meshOutputPerPrimitiveGranularity));
    }
    else {
      niLog(Info, "Vulkan Mesh shader not supported.");
    }

    if (meshShaderFeatures.taskShader) {
      niLog(Info, "Vulkan Task shader supported.");
    }
    else {
      niLog(Info, "Vulkan Task shader not supported.");
    }

    niLog(Info, niFmt(
      "Vulkan Robustness2 Features:\n"
      "  robustBufferAccess2: %y\n"
      "  robustImageAccess2: %y\n"
      "  nullDescriptor: %y",
      (tBool)!!robustness2Features.robustBufferAccess2,
      (tBool)!!robustness2Features.robustImageAccess2,
      (tBool)!!robustness2Features.nullDescriptor));
  }

  tBool _InitPhysicalDevice() {
    tU32 deviceCount = 0;
    vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
    niCheck(deviceCount > 0, eFalse);

    vkEnumeratePhysicalDevices(_instance, &deviceCount, &_physicalDevice);

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(_physicalDevice, &props);
    niLog(Info,niFmt("Vulkan Device: %s", props.deviceName));
    niLog(Info,niFmt("Vulkan Driver Version: %d.%d.%d",
                     VK_VERSION_MAJOR(props.driverVersion),
                     VK_VERSION_MINOR(props.driverVersion),
                     VK_VERSION_PATCH(props.driverVersion)));
    niLog(Info,niFmt("Vulkan API: %d.%d.%d",
                     VK_VERSION_MAJOR(props.apiVersion),
                     VK_VERSION_MINOR(props.apiVersion),
                     VK_VERSION_PATCH(props.apiVersion)));

    // Query and log device features
    vkGetPhysicalDeviceFeatures(_physicalDevice, &_physicalDeviceFeatures);
    niLog(Info,niFmt(
      "Vulkan Physical Device Features:\n"
      // Geometry features
      "  geometryShader: %y\n"
      "  tessellationShader: %y\n"
      // Rendering features
      "  depthBiasClamp: %y\n"
      "  fillModeNonSolid: %y\n"
      "  depthClamp: %y\n"
      "  depthBounds: %y\n"
      "  wideLines: %y\n"
      "  largePoints: %y\n"
      // Texture features
      "  samplerAnisotropy: %y\n"
      "  textureCompressionBC: %y\n"
      "  textureCompressionETC2: %y\n"
      "  textureCompressionASTC_LDR: %y\n",
      // Geometry features
      (tBool)!!_physicalDeviceFeatures.geometryShader,
      (tBool)!!_physicalDeviceFeatures.tessellationShader,
      // Rendering features
      (tBool)!!_physicalDeviceFeatures.depthBiasClamp,
      (tBool)!!_physicalDeviceFeatures.fillModeNonSolid,
      (tBool)!!_physicalDeviceFeatures.depthClamp,
      (tBool)!!_physicalDeviceFeatures.depthBounds,
      (tBool)!!_physicalDeviceFeatures.wideLines,
      (tBool)!!_physicalDeviceFeatures.largePoints,
      // Texture features
      (tBool)!!_physicalDeviceFeatures.samplerAnisotropy,
      (tBool)!!_physicalDeviceFeatures.textureCompressionBC,
      (tBool)!!_physicalDeviceFeatures.textureCompressionETC2,
      (tBool)!!_physicalDeviceFeatures.textureCompressionASTC_LDR));

    niLog(Info,niFmt(
      "Vulkan Buffer Alignment Properties:\n"
      "  minUniformBufferOffsetAlignment: %llu\n"
      "  minStorageBufferOffsetAlignment: %llu\n"
      "  minTexelBufferOffsetAlignment: %llu\n"
      "  optimalBufferCopyOffsetAlignment: %llu\n"
      "  optimalBufferCopyRowPitchAlignment: %llu\n"
      "  nonCoherentAtomSize: %llu",
      props.limits.minUniformBufferOffsetAlignment,
      props.limits.minStorageBufferOffsetAlignment,
      props.limits.minTexelBufferOffsetAlignment,
      props.limits.optimalBufferCopyOffsetAlignment,
      props.limits.optimalBufferCopyRowPitchAlignment,
      props.limits.nonCoherentAtomSize));

    // Descriptor Set Limits
    niLog(Info,niFmt(
      "Vulkan Descriptor Set Limits:\n"
      // The maximum number of descriptor sets that can be bound at one time.
      // Useful to know to optimize descriptor set usage and avoid exceeding device limits.
      "  maxBoundDescriptorSets: %d\n"
      // The maximum number of samplers in a descriptor set.
      // Important for determining how many textures can be accessed in a single shader.
      "  maxDescriptorSetSamplers: %d\n"
      // The maximum number of uniform buffers in a descriptor set.
      // Determines how many uniform buffers can be used for sharing data with shaders.
      "  maxDescriptorSetUniformBuffers: %d\n"
      // The maximum number of storage buffers in a descriptor set.
      // Impacts how much storage data you can access in shaders for operations like compute.
      "  maxDescriptorSetStorageBuffers: %d\n"
      // The maximum number of sampled images that can be included in a descriptor set.
      // Important for managing large sets of textures in rendering.
      "  maxDescriptorSetSampledImages: %d\n"
      // The maximum number of storage images in a descriptor set.
      // Affects image writing in compute and fragment shaders.
      "  maxDescriptorSetStorageImages: %d\n"
      // The maximum number of samplers per stage (e.g., vertex, fragment).
      // Helps avoid exceeding the per-stage sampler limit, which can impact shader resource layout.
      "  maxPerStageDescriptorSamplers: %d",
      props.limits.maxBoundDescriptorSets,
      props.limits.maxDescriptorSetSamplers,
      props.limits.maxDescriptorSetUniformBuffers,
      props.limits.maxDescriptorSetStorageBuffers,
      props.limits.maxDescriptorSetSampledImages,
      props.limits.maxDescriptorSetStorageImages,
      props.limits.maxPerStageDescriptorSamplers));

    // Vertex Input Limits
    niLog(Info,niFmt(
      "Vulkan Vertex Input Limits:\n"
      // The maximum number of vertex input attributes.
      // Important for defining complex vertex formats, such as those involving multiple components.
      "  maxVertexInputAttributes: %d\n"
      // The maximum number of vertex input bindings.
      // Limits the number of vertex buffers that can be used in a single pipeline.
      "  maxVertexInputBindings: %d",
      props.limits.maxVertexInputAttributes,
      props.limits.maxVertexInputBindings));

    // Push Constants and Buffers
    niLog(Info,niFmt(
      "Vulkan Push Constants and Buffers:\n"
      // The maximum size (in bytes) of push constants.
      // Push constants are a fast way to provide data to shaders, so this determines how much data can be passed in this way.
      "  maxPushConstantsSize: %d\n"
      // The maximum size of a uniform buffer.
      // Impacts how much uniform data (e.g., transformation matrices, material properties) can be shared with shaders.
      "  maxUniformBufferRange: %d\n"
      // The maximum size of a storage buffer.
      // Useful for determining the size of data that can be read and written in compute shaders.
      "  maxStorageBufferRange: %d",
      props.limits.maxPushConstantsSize,
      props.limits.maxUniformBufferRange,
      props.limits.maxStorageBufferRange));

    // Image and Memory Alignment
    niLog(Info,niFmt(
      "Vulkan Image and Memory Alignment:\n"
      // The alignment required between buffers and images in memory.
      // Affects how resources are allocated to ensure correct alignment for performance.
      "  bufferImageGranularity: %llu\n"
      // The maximum dimensions of a 2D image (width and height).
      // Important for determining the largest texture resolution supported by the device.
      "  maxImageDimension2D: %d\n"
      // The maximum number of layers in an image array.
      // Important for applications that need array textures, such as cube maps or layered rendering.
      "  maxImageArrayLayers: %d",
      props.limits.bufferImageGranularity,
      props.limits.maxImageDimension2D,
      props.limits.maxImageArrayLayers));

    // Framebuffer Limits
    niLog(Info,niFmt(
      "Vulkan Framebuffer Limits:\n"
      // The maximum width of a framebuffer.
      // Important for determining the resolution limits for rendering targets.
      "  maxFramebufferWidth: %d\n"
      // The maximum height of a framebuffer.
      // Helps in deciding the render target resolution.
      "  maxFramebufferHeight: %d\n"
      // The maximum number of color attachments in a framebuffer.
      // Limits the number of color outputs possible, affecting advanced rendering techniques like MRT (multiple render targets).
      "  maxColorAttachments: %d",
      props.limits.maxFramebufferWidth,
      props.limits.maxFramebufferHeight,
      props.limits.maxColorAttachments));

    // Compute Shader Limits
    niLog(Info,niFmt(
      "Vulkan Compute Shader Limits:\n"
      // The maximum number of workgroups in each dimension for compute shaders.
      // Important for understanding how to best structure large compute workloads.
      "  maxComputeWorkGroupCount: %s\n"
      // The maximum number of workgroup invocations in compute shaders.
      // Limits the number of compute shader threads that can run concurrently within a workgroup.
      "  maxComputeWorkGroupInvocations: %d",
      Vec3i(props.limits.maxComputeWorkGroupCount[0],
            props.limits.maxComputeWorkGroupCount[1],
            props.limits.maxComputeWorkGroupCount[2]),
      props.limits.maxComputeWorkGroupInvocations));

    // Sampler Limits
    niLog(Info,niFmt(
      "Vulkan Sampler Limits:\n"
      // The maximum anisotropy value for samplers.
      // Useful for determining the level of texture quality possible when using anisotropic filtering.
      "  maxSamplerAnisotropy: %g",
      props.limits.maxSamplerAnisotropy));

    // Ray tracing & mesh shaders detection
    _InitDeviceFeatures2(_physicalDevice);

    // Get extensions
    {
      tU32 extensionCount = 0;
      vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extensionCount, nullptr);
      astl::vector<VkExtensionProperties> extensions(extensionCount);
      vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extensionCount, extensions.data());
      for (const auto& extension : extensions) {
        astl::upsert(_extensions,extension.extensionName,extension.specVersion);
      }
      {
        cString o;
        niLoopit(tVkExtensionsMap::const_iterator,it,_extensions) {
          if (it != _extensions.begin())
            o << ", ";
          o << it->first << "=" << it->second;
        }
        niLog(Info,niFmt("Vulkan extensions[%d]: %s", _extensions.size(), o));
      }
    }

    // Check requirements
    niCheck(props.limits.maxBoundDescriptorSets >= eGLSLVulkanDescriptorSet_Last,eFalse);
    niCheck(props.limits.maxVertexInputBindings >= eGLSLVulkanVertexInputLayout_Last,eFalse);

    // Check extensions
    for (tU32 i = 0; i < knVkRequiredDeviceExtensionsCount; ++i) {
      if (_extensions.find(_vkRequiredDeviceExtensions[i]) == _extensions.end()) {
        niError(niFmt("Required device extension '%s' not found.", _vkRequiredDeviceExtensions[i]));
        return eFalse;
      }
    }

    return eTrue;
  }

  tBool _FindQueueFamily(tU32& aQueueFamilyIndex) {
    tU32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, nullptr);
    astl::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, queueFamilies.data());

    niLog(Info,"Vulkan Queue Families:");
    for (tU32 i = 0; i < queueFamilyCount; i++) {
      niLog(Info,niFmt("  Family %d:", i));
      niLog(Info,niFmt("    Queue Count: %d", queueFamilies[i].queueCount));
      niLog(Info,niFmt("    Graphics: %s", (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) ? "Yes" : "No"));
      niLog(Info,niFmt("    Compute: %s", (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) ? "Yes" : "No"));
      niLog(Info,niFmt("    Transfer: %s", (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) ? "Yes" : "No"));
      niLog(Info,niFmt("    Sparse: %s", (queueFamilies[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) ? "Yes" : "No"));
    }

    for (tU32 i = 0; i < queueFamilyCount; i++) {
      if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        aQueueFamilyIndex = i;
        return eTrue;
      }
    }

    return eFalse;
  }

  tBool _CreateLogicalDevice() {
    tF32 queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = _queueFamilyIndex,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority
    };

    VkPhysicalDeviceFeatures2 features2 = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2
    };
    vkGetPhysicalDeviceFeatures2(_physicalDevice, &features2);

    // Enable base features
    features2.features.samplerAnisotropy = _physicalDeviceFeatures.samplerAnisotropy;

    // Enable dynamic rendering
    VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
      .dynamicRendering = VK_TRUE
    };
    features2.pNext = &dynamicRenderingFeatures;

    // Enable dynamic states
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extDynamicStateFeatures = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
      .extendedDynamicState = VK_TRUE
    };
    dynamicRenderingFeatures.pNext = &extDynamicStateFeatures;

    // Create the device
    VkDeviceCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queueCreateInfo,
      .enabledExtensionCount = knVkRequiredDeviceExtensionsCount,
      .ppEnabledExtensionNames = _vkRequiredDeviceExtensions,
      .enabledLayerCount = 0,
      .pEnabledFeatures = nullptr,
      .pNext = &features2,
    };
    VK_CHECK(vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device), eFalse);
    vkGetDeviceQueue(_device, _queueFamilyIndex, 0, &_graphicsQueue);
    return eTrue;
  }

  tBool _CreateCommandPool() {
    VkCommandPoolCreateInfo poolInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .queueFamilyIndex = _queueFamilyIndex,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
    };

    VK_CHECK(vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool), eFalse);
    return eTrue;
  }

  tBool _CreateAllocator() {
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = _physicalDevice;
    allocatorInfo.device = _device;
    allocatorInfo.instance = _instance;
#ifdef niVulkan_Volk
    VmaVulkanFunctions vmaVulkanFuncs {
      .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
      .vkGetDeviceProcAddr = vkGetDeviceProcAddr
    };
    allocatorInfo.pVulkanFunctions = &vmaVulkanFuncs;
#endif
    VK_CHECK(vmaCreateAllocator(&allocatorInfo, &_allocator), eFalse);
    return eTrue;
  }

  tBool _CreateVulkanDriverResources();

  inline VkSampler _GetVkSamplerState(tIntPtr ahSS) const {
    if (ahSS >= eCompiledStates_SS_PointRepeat &&
        ahSS <= eCompiledStates_SS_SmoothWhiteBorder)
    {
      return _ssCompiled[ahSS-eCompiledStates_SS_PointRepeat];
    }
    niPanicAssert(niFmt("Unknown sampler states '%d'.", ahSS));
    return _ssCompiled[0];
  }

  VkCommandBuffer BeginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandPool = _commandPool,
      .commandBufferCount = 1
    };

    VkCommandBuffer cmdBuf;
    vkAllocateCommandBuffers(_device, &allocInfo, &cmdBuf);

    VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    vkBeginCommandBuffer(cmdBuf, &beginInfo);
    return cmdBuf;
  }

  void EndSingleTimeCommands(VkCommandBuffer cmdBuf) {
    vkEndCommandBuffer(cmdBuf);

    VkSubmitInfo submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .commandBufferCount = 1,
      .pCommandBuffers = &cmdBuf
    };

    vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_graphicsQueue);

    vkFreeCommandBuffers(_device, _commandPool, 1, &cmdBuf);
  }

  //// iGraphicsDriver ///////////////////////////////
  virtual iGraphics* __stdcall GetGraphics() const niImpl { return _graphics; }
  virtual const achar* __stdcall GetName() const niImpl { return _A("Vulkan"); }
  virtual const achar* __stdcall GetDesc() const niImpl { return _A("Vulkan Graphics Driver"); }
  virtual const achar* __stdcall GetDeviceName() const niImpl { return _A("Default"); }

  virtual tInt __stdcall GetCaps(eGraphicsCaps aCaps) const niImpl {
    switch (aCaps) {
      case eGraphicsCaps_Resize:
      case eGraphicsCaps_MultiContext:
      case eGraphicsCaps_ScissorTest:
      case eGraphicsCaps_OverlayTexture:
      case eGraphicsCaps_NumTextureUnits:
      case eGraphicsCaps_NumRenderTargetTextures:
        return 1;
      case eGraphicsCaps_Texture2DMaxSize:
        return 0xFFFF;
      case eGraphicsCaps_TextureCubeMaxSize:
      case eGraphicsCaps_Texture3DMaxSize:
        return 0;
      case eGraphicsCaps_MaxVertexIndex:
        return 0xFFFFFFFF;
      case eGraphicsCaps_DepthStencilTexture:
      case eGraphicsCaps_StencilTwoSideded:
      case eGraphicsCaps_StencilWrap:
      case eGraphicsCaps_OcclusionQueries:
      case eGraphicsCaps_MaxPointSize:
      case eGraphicsCaps_HardwareInstancing:
      case eGraphicsCaps_OrthoProjectionOffset:
      case eGraphicsCaps_BlitBackBuffer:
        return 0;
      case eGraphicsCaps_Wireframe:
        return 1;
      case eGraphicsCaps_IGpu:
        return 1;
    }
  }

  virtual tBool __stdcall ResetAllCaches() niImpl {
    return eTrue;
  }

  virtual tGraphicsDriverImplFlags __stdcall GetGraphicsDriverImplFlags() const niImpl {
    return eGraphicsDriverImplFlags_IndexArrayObject|
        eGraphicsDriverImplFlags_VertexArrayObject;
  }

  virtual tBool __stdcall CheckTextureFormat(iBitmapFormat* apFormat, tTextureFlags aFlags) niImpl {
    niCheckSilent(niIsOK(apFormat),eFalse);

    niLet gpufmt = _GetClosestGpuPixelFormatForTexture(apFormat->GetPixelFormat()->GetFormat(),aFlags);

    // TODO: For now all eGpuPixelFormat are supported by Vulkan but that
    // might not always be the case. Eventually we should validate this.
    NN<iPixelFormat> bmpFormat = niCheckNN(bmpFormat,_GetIPixelFormat(_graphics,gpufmt),eFalse);
    if (bmpFormat->IsSamePixelFormat(apFormat->GetPixelFormat())) {
      return eTrue;
    }
    else {
      apFormat->SetPixelFormat(bmpFormat);
      return eFalse;
    }
  }

  virtual iTexture* __stdcall CreateTexture(iHString* ahspName, eBitmapType aType, const achar* aaszFormat, tU32 anNumMipMaps, tU32 anWidth, tU32 anHeight, tU32 anDepth, tTextureFlags aFlags) niImpl;

  virtual tBool __stdcall BlitBitmapToTexture(iBitmap2D* apSrc, iTexture* apDest, tU32 anDestLevel, const sRecti& aSrcRect, const sRecti& aDestRect, eTextureBlitFlags aFlags);

  virtual tBool __stdcall BlitTextureToBitmap(iTexture* apSrc, tU32 anSrcLevel, iBitmap2D* apDest, const sRecti& aSrcRect, const sRecti& aDestRect, eTextureBlitFlags aFlags)  {
    return eFalse;
  }
  virtual tBool __stdcall BlitTextureToTexture(iTexture* apSrc, tU32 anSrcLevel, iTexture* apDest, tU32 anDestLevel, const sRecti& aSrcRect, const sRecti& aDestRect, eTextureBlitFlags aFlags)  {
    return eFalse;
  }
  virtual tBool __stdcall BlitBitmap3DToTexture(iBitmap3D* apSrc, iTexture* apDest, tU32 anDestLevel, const sVec3i& aSrcMin, const sVec3i& aDestMin, const sVec3i& avSize, eTextureBlitFlags aFlags)  {
    return eFalse;
  }
  virtual tBool __stdcall BlitTextureToBitmap3D(iTexture* apSrc, tU32 anSrcLevel, iBitmap3D* apDest, const sVec3i& aSrcMin, const sVec3i& aDestMin, const sVec3i& avSize, eTextureBlitFlags aFlags)  {
    return eFalse;
  }

  /////////////////////////////////////////////
  virtual tU32 __stdcall GetNumShaderProfile(eShaderUnit aUnit) const niImpl {
    return 0;
  }
  virtual iHString* __stdcall GetShaderProfile(eShaderUnit aUnit, tU32 anIndex) const niImpl {
    return nullptr;
  }
  virtual iShader* __stdcall CreateShader(iHString* ahspName, iFile* apFile) niImpl {
    return nullptr;
  }
  virtual iOcclusionQuery* __stdcall CreateOcclusionQuery() niImpl {
    return nullptr;
  }

  /////////////////////////////////////////////
  virtual void __stdcall SetDrawOpCapture(iGraphicsDrawOpCapture* apCapture) {
    _drawOpCapture = niGetIfOK(apCapture);
  }
  virtual iGraphicsDrawOpCapture* __stdcall GetDrawOpCapture() const {
    return _drawOpCapture;
  }

  /////////////////////////////////////////////
  virtual iVertexArray* __stdcall CreateVertexArray(tU32 anNumVertices, tFVF aFVF, eArrayUsage aUsage) {
    return CreateFixedGpuVertexArray(this,anNumVertices,aFVF,aUsage);
  }
  virtual iIndexArray* __stdcall CreateIndexArray(eGraphicsPrimitiveType aPrimitiveType, tU32 anNumIndices, tU32 anMaxVertexIndex, eArrayUsage aUsage) {
    return CreateFixedGpuIndexArray(this,aPrimitiveType,anNumIndices,anMaxVertexIndex,aUsage);
  }

  /////////////////////////////////////////////
  virtual iGraphicsContext* __stdcall CreateContextForWindow(iOSWindow* apWindow, const achar* aaszBBFormat, const achar* aaszDSFormat, tU32 anSwapInterval, tTextureFlags aBackBufferFlags) niImpl;
  virtual iGraphicsContextRT* __stdcall CreateContextForRenderTargets(iTexture* apRT0, iTexture* apRT1, iTexture* apRT2, iTexture* apRT3, iTexture* apDS) niImpl;
  //// iGraphicsDriver ///////////////////////////////

  //// iGraphicsDriverGpu ///////////////////////////////
  virtual Ptr<iGpuBuffer> __stdcall CreateGpuBuffer(iHString* ahspName, tU32 anSize, eGpuBufferMemoryMode aMemMode, tGpuBufferUsageFlags aUsage) niImpl;
  virtual Ptr<iGpuBuffer> __stdcall CreateGpuBufferFromData(iHString* ahspName, iFile* apFile, tU32 anSize, eGpuBufferMemoryMode aMemMode, tGpuBufferUsageFlags aUsage) niImpl;
  virtual Ptr<iGpuBuffer> __stdcall CreateGpuBufferFromDataRaw(iHString* ahspName, tPtr apData, tU32 anSize, eGpuBufferMemoryMode aMemMode, tGpuBufferUsageFlags aUsage) niImpl;
  virtual iHString* __stdcall GetGpuFunctionTarget() const niImpl;
  virtual Ptr<iGpuFunction> __stdcall CreateGpuFunction(eGpuFunctionType aType, iHString* ahspPath) niImpl;
  virtual Ptr<iGpuPipelineDesc> __stdcall CreateGpuPipelineDesc() niImpl;
  virtual Ptr<iGpuBlendMode> __stdcall CreateGpuBlendMode() niImpl;
  virtual Ptr<iGpuPipeline> __stdcall CreateGpuPipeline(iHString* ahspName, const iGpuPipelineDesc* apDesc) niImpl;
  virtual tBool __stdcall BlitManagedGpuBufferToSystemMemory(iGpuBuffer* apBuffer) niImpl;
  //// iGraphicsDriverGpu ///////////////////////////////
};

static VkBufferUsageFlags _ToVkBufferUsageFlags(tGpuBufferUsageFlags aUsage) {
  VkBufferUsageFlags vkUsage = 0;
  if (niFlagIs(aUsage,eGpuBufferUsageFlags_Vertex)) vkUsage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  if (niFlagIs(aUsage,eGpuBufferUsageFlags_Index)) vkUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  if (niFlagIs(aUsage,eGpuBufferUsageFlags_Uniform)) vkUsage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  if (niFlagIs(aUsage,eGpuBufferUsageFlags_Storage)) vkUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  if (niFlagIs(aUsage,eGpuBufferUsageFlags_Indirect)) vkUsage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
  if (niFlagIs(aUsage,eGpuBufferUsageFlags_TransferSrc)) vkUsage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  if (niFlagIs(aUsage,eGpuBufferUsageFlags_TransferDst)) vkUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  return vkUsage;
}

struct sVulkanBuffer : public ImplRC<iGpuBuffer,eImplFlags_DontInherit1,iDeviceResource> {
  nn<sVulkanDriver> _driver;
  tHStringPtr _name;
  VkBuffer _vkBuffer = VK_NULL_HANDLE;
  VmaAllocation _vmaAllocation = nullptr;
  eGpuBufferMemoryMode _memMode;
  tGpuBufferUsageFlags _usage;
  tU32 _lockOffset = 0, _lockSize = 0;
  tU32 _modifiedOffset = 0, _modifiedSize = 0;
  eLock _lockMode;
  tBool _boundModifiedBuffer = eFalse;

  sVulkanBuffer(
    ain_nn<sVulkanDriver> aDriver,
    eGpuBufferMemoryMode aMemMode,
    tGpuBufferUsageFlags aUsage)
      : _driver(aDriver)
      , _memMode(aMemMode)
      , _usage(aUsage)
  {}

  ~sVulkanBuffer() {
    if (_vkBuffer) {
      vmaDestroyBuffer(_driver->_allocator, _vkBuffer, _vmaAllocation);
      _vkBuffer = VK_NULL_HANDLE;
      _vmaAllocation = nullptr;
    }
  }

  tBool Create(tU32 anSize) {
    VkBufferCreateInfo bufferInfo = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = anSize,
      .usage = _ToVkBufferUsageFlags(_usage)
    };

    VmaAllocationCreateInfo allocInfo = {};
    switch (_memMode) {
      case eGpuBufferMemoryMode_Shared:
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        break;
      case eGpuBufferMemoryMode_Private:
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        break;
      case eGpuBufferMemoryMode_Managed:
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        break;
    }

    VK_CHECK(vmaCreateBuffer(
      _driver->_allocator, &bufferInfo, &allocInfo,
      &_vkBuffer, &_vmaAllocation, nullptr), eFalse);

    return eTrue;
  }

  void _Untrack() {
    // niDebugFmt(("... Unbind: %p: [ms:%d,me:%d].", (tIntPtr)this, _modifiedStart,_modifiedEnd));
    _modifiedOffset = _modifiedSize = 0;
    _boundModifiedBuffer = eFalse;
  }

  virtual tBool __stdcall IsOK() const niImpl {
    return _vkBuffer != VK_NULL_HANDLE;
  }

  virtual iHString* __stdcall GetDeviceResourceName() const niImpl {
    return _name;
  }
  virtual tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) niImpl {
    return eFalse;
  }
  virtual tBool __stdcall ResetDeviceResource() niImpl {
    return eTrue;
  }
  virtual iDeviceResource* __stdcall Bind(iUnknown* apDevice) niImpl {
    return this;
  }

  virtual tU32 __stdcall GetSize() const niImpl {
    VmaAllocationInfo info;
    vmaGetAllocationInfo(_driver->_allocator, _vmaAllocation, &info);
    return (tU32)info.size;
  }

  virtual eGpuBufferMemoryMode __stdcall GetMemoryMode() const niImpl {
    return _memMode;
  }

  virtual tGpuBufferUsageFlags __stdcall GetUsageFlags() const niImpl {
    return _usage;
  }

  virtual tPtr __stdcall Lock(tU32 anOffset, tU32 anSize, eLock aLock) niImpl {
    niCheck(_memMode != eGpuBufferMemoryMode_Private, nullptr);
    niCheck(!GetIsLocked(),nullptr);

    _lockMode = aLock;
    _lockOffset = anOffset;
    _lockSize = anSize ? anSize : (GetSize()-anOffset);

    if (_modifiedSize == 0) {
      _modifiedOffset = _lockOffset;
      _modifiedSize = _lockSize;
    }
    else if ((_lockOffset < (_modifiedOffset+_modifiedSize)) &&
             (_lockOffset+_lockSize) > _modifiedOffset)
    {
      if (_boundModifiedBuffer) {
        // TODO: The lock should fail in this case and return nullptr? We
        // should not allow submitted buffers to be modified?
        niWarning(niFmt(
          "Lock(%d,%d,%d): %p: [lo:%d,ls:%d] [mo:%d,ms:%d] Locked inflight overlapping area.",
          anOffset,anSize,aLock,
          (tIntPtr)this,
          _lockOffset,_lockSize,
          _modifiedOffset,_modifiedSize));
      }
      const tU32 newStart = ni::Min(_modifiedOffset,_lockOffset);
      const tU32 newEnd = ni::Max(_modifiedOffset+_modifiedSize,
                                  _lockOffset+_lockSize);
      _modifiedOffset = newStart;
      _modifiedSize = newEnd - newStart;
    }

    void* data;
    VK_CHECK(vmaMapMemory(_driver->_allocator, _vmaAllocation, &data), nullptr);
    return ((tPtr)data) + _lockOffset;
  }

  virtual tBool __stdcall Unlock() niImpl {
    if (!GetIsLocked())
      return eFalse;
    vmaUnmapMemory(_driver->_allocator, _vmaAllocation);
    _lockOffset = _lockSize = 0;
    return eTrue;
  }

  virtual tBool __stdcall GetIsLocked() const niImpl {
    return _lockSize != 0;
  }
};

struct sVulkanTexture : public ImplRC<iTexture> {
  nn<sVulkanDriver> _driver;
  VkImage _vkImage = VK_NULL_HANDLE;
  VmaAllocation _vmaAllocation;
  VkImageView _vkView = VK_NULL_HANDLE;
  const tU32 _width = 0;
  const tU32 _height = 0;
  const tU32 _numMipMaps = 0;
  const tHStringPtr _name;
  const tTextureFlags _flags = eTextureFlags_Default;
  eGpuPixelFormat _pixelFormat = eGpuPixelFormat_None;
  eBitmapType _type;
  astl::vector<Ptr<sVulkanTexture>> _subTexs;
  tU32 _subTexId = 0;

  sVulkanTexture(
    ain_nn<sVulkanDriver> aDriver, iHString* ahspName,
    eBitmapType aType,
    tU32 anWidth, tU32 anHeight, tU32 anNumMipMaps,
    eGpuPixelFormat aGpuPixelFormat,
    tTextureFlags aFlags)
      : _driver(aDriver)
      , _name(ahspName)
      , _type(aType)
      , _width(anWidth)
      , _height(anHeight)
      , _numMipMaps(anNumMipMaps)
      , _flags(aFlags)
      , _pixelFormat(aGpuPixelFormat)
  {
    if (niFlagIsNot(_flags,eTextureFlags_SubTexture)) {
      _driver->_graphics->GetTextureDeviceResourceManager()->Register(this);
    }
  }

  ~sVulkanTexture() {
    this->Invalidate();
  }

  virtual void __stdcall Invalidate() override {
    _subTexs.clear();
    if (_vkView) {
      vkDestroyImageView(_driver->_device, _vkView, nullptr);
      _vkView = VK_NULL_HANDLE;
    }
    if (niFlagIsNot(_flags,eTextureFlags_SubTexture)) {
      if (_driver->_graphics->GetTextureDeviceResourceManager()) {
        _driver->_graphics->GetTextureDeviceResourceManager()->Unregister(this);
      }
      if (_vkImage) {
        vmaDestroyImage(_driver->_allocator, _vkImage, _vmaAllocation);
        _vkImage = VK_NULL_HANDLE;
      }
    }
  }

  virtual iHString* __stdcall GetDeviceResourceName() const override {
    return _name;
  }

  virtual tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) override {
    return eFalse;
  }

  virtual tBool __stdcall ResetDeviceResource() override {
    return eTrue;
  }

  virtual iDeviceResource* __stdcall Bind(iUnknown* apDevice) override {
    return this;
  }

  virtual eBitmapType __stdcall GetType() const override {
    return _type;
  }

  virtual tU32 __stdcall GetWidth() const override {
    return _width;
  }

  virtual tU32 __stdcall GetHeight() const override {
    return _height;
  }

  virtual tU32 __stdcall GetDepth() const override {
    return 1;
  }

  virtual iPixelFormat* __stdcall GetPixelFormat() const override {
    return _GetIPixelFormat(_driver->_graphics,_pixelFormat);
  }

  virtual tU32 __stdcall GetNumMipMaps() const override {
    return _numMipMaps;
  }

  virtual tTextureFlags __stdcall GetFlags() const override {
    return _flags;
  }

  virtual iTexture* __stdcall GetSubTexture(tU32 anIndex) const override {
    if (anIndex >= _subTexs.size())
      return nullptr;
    return _subTexs[anIndex];
  }

  tBool _CreateVulkanTexture() {
    niPanicAssert(_vkImage == VK_NULL_HANDLE && _vkView == VK_NULL_HANDLE);

    VkImageCreateInfo imageInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = _GetVulkanPixelFormat(_pixelFormat),
      .extent = {_width, _height, 1},
      .mipLevels = 1+_numMipMaps,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    if (niFlagIs(_flags,eTextureFlags_RenderTarget)) {
      imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    else if (niFlagIs(_flags,eTextureFlags_DepthStencil)) {
      imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }

    switch (_type) {
      case eBitmapType_2D: {
        break;
      }
      case eBitmapType_Cube: {
        imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        imageInfo.arrayLayers = 6;
        break;
      }
      default: {
        niError(niFmt("Unsupported bitmap type '%d' for image creation.",_type));
        return eFalse;
      }
    }

    VmaAllocationCreateInfo allocInfo = {
      .usage = VMA_MEMORY_USAGE_GPU_ONLY
    };
    VK_CHECK(vmaCreateImage(
      _driver->_allocator, &imageInfo, &allocInfo,
      &_vkImage, &_vmaAllocation, nullptr), eFalse);

    if (_type == eBitmapType_Cube) {
      _subTexs.resize(6);
      niLoop(i,6) {
        _subTexs[i] = niNew sVulkanTexture(
          _driver,
          _name,
          eBitmapType_2D,
          _width,_height,GetNumMipMaps(),
          _pixelFormat,
          eTextureFlags_SubTexture|eTextureFlags_Surface);
        _subTexs[i]->_vkImage = _vkImage;
        _subTexs[i]->_vmaAllocation = _vmaAllocation;
        _subTexs[i]->_subTexId = i;
      }
    }

    // Create image view
    VkImageViewCreateInfo viewInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = _vkImage,
      .format = imageInfo.format,
      .components = {
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY
      },
      .subresourceRange = {
        .aspectMask = niFlagIs(_flags,eTextureFlags_DepthStencil) ?
        VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1+_numMipMaps,
        .baseArrayLayer = 0
      }
    };
    switch (_type) {
      case eBitmapType_2D: {
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.subresourceRange.layerCount = 1;
        break;
      }
      case eBitmapType_Cube: {
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        viewInfo.subresourceRange.layerCount = 6;
        break;
      }
      default: {
        niError(niFmt("Unsupported bitmap type '%d' for view creation.",_type));
        return eFalse;
      }
    }

    VK_CHECK(vkCreateImageView(_driver->_device, &viewInfo, nullptr, &_vkView), eFalse);
    return eTrue;
  }

  tBool _UploadTexture(const iBitmap2D* apBmpLevel,
                       const tU32 anLevel,
                       const sRecti& aDestRect)
  {
    const tU32 bpp = apBmpLevel->GetPixelFormat()->GetBytesPerPixel();
    const tU32 bpr = apBmpLevel->GetPitch();
    const tU32 startOffset = (aDestRect.y * bpr) + (aDestRect.x * bpp);
    const tPtr bytes = (tPtr)(apBmpLevel->GetData()+startOffset);

    // Create staging buffer
    VkBuffer stagingBuffer;
    VmaAllocation stagingAlloc;

    VkBufferCreateInfo bufferInfo = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = aDestRect.GetWidth() * aDestRect.GetHeight() * bpp,
      .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
    };

    VmaAllocationCreateInfo stagingAllocInfo = {
      .usage = VMA_MEMORY_USAGE_CPU_TO_GPU
    };

    VK_CHECK(vmaCreateBuffer(
      _driver->_allocator, &bufferInfo, &stagingAllocInfo,
      &stagingBuffer, &stagingAlloc, nullptr), eFalse);
    niDefer {
      vmaDestroyBuffer(_driver->_allocator, stagingBuffer, stagingAlloc);
    };

    // Copy data to staging
    void* data;
    VK_CHECK(vmaMapMemory(_driver->_allocator, stagingAlloc, &data), eFalse);
    for (tU32 y = 0; y < aDestRect.GetHeight(); ++y) {
      memcpy(
        (tU8*)data + (y * aDestRect.GetWidth() * bpp),
        (tU8*)bytes + (y * bpr),
        aDestRect.GetWidth() * bpp);
    }
    vmaUnmapMemory(_driver->_allocator, stagingAlloc);

    // Transition image layout for copy
    VkCommandBuffer cmdBuf = _driver->BeginSingleTimeCommands();
    niCheck(_VulkanTransitionImageLayout(
      cmdBuf,_vkImage,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      anLevel,_subTexId),eFalse);

    // Copy buffer to image
    VkBufferImageCopy region = {
      .bufferOffset = 0,
      .bufferRowLength = 0,
      .bufferImageHeight = 0,
      .imageSubresource = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevel = anLevel,
        .baseArrayLayer = _subTexId,
        .layerCount = 1
      },
      .imageOffset = {
        aDestRect.x,
        aDestRect.y,
        0
      },
      .imageExtent = {
        (tU32)aDestRect.GetWidth(),
        (tU32)aDestRect.GetHeight(),
        1
      }
    };

    vkCmdCopyBufferToImage(
      cmdBuf,
      stagingBuffer,
      _vkImage,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1,
      &region);

    // Transition to shader read
    niCheck(_VulkanTransitionImageLayout(
      cmdBuf,_vkImage,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      anLevel,_subTexId),eFalse);

    _driver->EndSingleTimeCommands(cmdBuf);
    return eTrue;
  }

};

struct sVulkanFunction : public ImplRC<iGpuFunction,eImplFlags_DontInherit1,iDeviceResource> {
  nn<sVulkanDriver> _driver;
  NN<iDataTable> _datatable = niDeferredInit(NN<iDataTable>);
  const eGpuFunctionType _functionType;
  const tU32 _id;
  tHStringPtr _hspName;
  VkShaderModule _vkShaderModule = VK_NULL_HANDLE;
  eGpuFunctionBindType _bindType;

  sVulkanFunction(
    ain_nn<sVulkanDriver> aDriver,
    ain<eGpuFunctionType> aFuncType,
    ain<tU32> anID)
      : _driver(aDriver)
      , _functionType(aFuncType)
      , _id(anID)
  {}

  ~sVulkanFunction() {
    if (_vkShaderModule) {
      vkDestroyShaderModule(_driver->_device, _vkShaderModule, nullptr);
      _vkShaderModule = VK_NULL_HANDLE;
    }
  }

  tBool _Compile(VkDevice aDevice, iHString* ahspPath) {
    _hspName = ahspPath;
    _datatable = niCheckNN(_datatable,GpuFunctionDT_Load(niHStr(ahspPath),_GetVulkanGpuFunctionTarget(),&_bindType),eFalse);
    NN<iFile> spvData = niCheckNN_(
      spvData,GpuFunctionDT_GetSourceData(_datatable),
      niFmt("Can't get gpufunc data for target '%s' in '%s'.",_GetVulkanGpuFunctionTarget(),ahspPath),
      eFalse);

    spvData->SeekSet(0);
    astl::vector<tU8> data;
    data.resize(spvData->GetSize());
    if (spvData->ReadRaw((tPtr)data.data(),data.size()) != data.size()) {
      niError(niFmt("Can't read gpufunc data for target '%s' in '%s'.",_GetVulkanGpuFunctionTarget(),ahspPath));
      return eFalse;
    }

    VkShaderModuleCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = data.size(),
      .pCode = (tU32*)data.data()
    };

    VK_CHECK(vkCreateShaderModule(aDevice, &createInfo, nullptr, &_vkShaderModule), eFalse);
    return eTrue;
  }

  virtual tU32 __stdcall GetFunctionId() const niImpl {
    return _id;
  }

  virtual eGpuFunctionType __stdcall GetFunctionType() const niImpl {
    return _functionType;
  }

  virtual eGpuFunctionBindType __stdcall GetFunctionBindType() const niImpl {
    return _bindType;
  }

  virtual iDataTable* __stdcall GetDataTable() const niImpl {
    return _datatable;
  }

  virtual iHString* __stdcall GetDeviceResourceName() const niImpl {
    return _hspName;
  }
  virtual tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) niImpl {
    return eFalse;
  }
  virtual tBool __stdcall ResetDeviceResource() niImpl {
    return eTrue;
  }
  virtual iDeviceResource* __stdcall Bind(iUnknown* apDevice) niImpl {
    return this;
  }
};

static Ptr<sVulkanFunction> __stdcall CreateVulkanGpuFunction(
  ain_nn<sVulkanDriver> aDriver,
  eGpuFunctionType aType,
  iHString* ahspPath)
{
  niLet newId = aDriver->_idGenerator.AllocID();
  NN<sVulkanFunction> func = MakeNN<sVulkanFunction>(aDriver,aType,newId);
  if (!func->_Compile(aDriver->_device,ahspPath)) {
    aDriver->_idGenerator.FreeID(newId);
    niError(niFmt(
      "Can't create gpu function '%s': Compilation failed.",
      ahspPath));
    return nullptr;
  }
  if (func->GetFunctionType() != aType) {
    aDriver->_idGenerator.FreeID(newId);
    niError(niFmt(
      "Can't create gpu function '%s': Expected function type '%s' but got '%s'.",
      ahspPath,
      (tU32)aType, (tU32)func->GetFunctionType()
      // niEnumToChars(eGpuFunctionType,aType),
      // niEnumToChars(eGpuFunctionType,func->GetFunctionType())
    ));
    return nullptr;
  }
  return func;
}

struct sVulkanPipeline : public ImplRC<iGpuPipeline,eImplFlags_DontInherit1,iDeviceResource> {
  nn<sVulkanDriver> _driver; // TODO: Should be a weakptr
  tHStringPtr _hspName;
  NN<iGpuPipelineDesc> _desc = niDeferredInit(NN<iGpuPipelineDesc>);
  VkPipelineLayout _vkPipelineLayout = VK_NULL_HANDLE;
  VkPipeline _vkPipeline = VK_NULL_HANDLE;
  astl::vector<VkDescriptorSetLayout> _vkDescSetLayouts;
  VkDescriptorSetLayout _vkDescSetEmptyLayout;
  eGpuFunctionBindType _gpufuncBindType = eGpuFunctionBindType_None;

  virtual iHString* __stdcall GetDeviceResourceName() const niImpl {
    return _hspName;
  }
  virtual tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) niImpl {
    return eFalse;
  }
  virtual tBool __stdcall ResetDeviceResource() niImpl {
    return eTrue;
  }
  virtual iDeviceResource* __stdcall Bind(iUnknown* apDevice) niImpl {
    return this;
  }

  sVulkanPipeline(ain<nn<sVulkanDriver>> aDriver)
      : _driver(aDriver)
  {}

  ~sVulkanPipeline() {
    if (!_vkDescSetLayouts.empty()) {
      niLoop(i,_vkDescSetLayouts.size()) {
        if (_vkDescSetLayouts[i] != _vkDescSetEmptyLayout &&
            _vkDescSetLayouts[i] != VK_NULL_HANDLE)
        {
          vkDestroyDescriptorSetLayout(
            _driver->_device,
            _vkDescSetLayouts[i],
            nullptr);
        }
      }
      _vkDescSetLayouts.clear();
    }
    if (_vkDescSetEmptyLayout) {
      vkDestroyDescriptorSetLayout(
        _driver->_device,
        _vkDescSetEmptyLayout,
        nullptr);
      _vkDescSetEmptyLayout = VK_NULL_HANDLE;
    }
    if (_vkPipeline) {
      vkDestroyPipeline(_driver->_device, _vkPipeline, nullptr);
      _vkPipeline = VK_NULL_HANDLE;
    }
    if (_vkPipelineLayout) {
      vkDestroyPipelineLayout(_driver->_device, _vkPipelineLayout, nullptr);
      _vkPipelineLayout = VK_NULL_HANDLE;
    }
  }

  tBool _CreateNoneDescSetLayout() {
    niLet vkDevice = _driver->_device;
    niPanicAssert(_vkDescSetLayouts.empty());
    VkPipelineLayoutCreateInfo layoutInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 0,
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = nullptr
    };
    if (!_vkDescSetLayouts.empty()) {
      layoutInfo.setLayoutCount = _vkDescSetLayouts.size();
      layoutInfo.pSetLayouts = _vkDescSetLayouts.data();
    }
    VK_CHECK(vkCreatePipelineLayout(vkDevice, &layoutInfo, nullptr, &_vkPipelineLayout), eFalse);
    return eTrue;
  }

  tBool _CreateFixedDescSetLayout() {
    niLet vkDevice = _driver->_device;
    niPanicAssert(_vkDescSetLayouts.empty());
    _vkDescSetLayouts.resize(6); // Space for sets 0-5

    niLet stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    niLet layoutFlags = 0;

    // Initialize all with empty layouts
    {
      VkDescriptorSetLayoutCreateInfo emptyLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 0,
        .pBindings = nullptr
      };
      VK_CHECK(vkCreateDescriptorSetLayout(vkDevice, &emptyLayoutInfo, nullptr, &_vkDescSetEmptyLayout), eFalse);
      niLoop(i,_vkDescSetLayouts.size()) {
        _vkDescSetLayouts[i] = _vkDescSetEmptyLayout;
      }
    }

    // Buffer layout
    {
      VkDescriptorSetLayoutBinding bufferBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = stageFlags,
        .pImmutableSamplers = nullptr
      };
      VkDescriptorSetLayoutCreateInfo bufferLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .flags = layoutFlags,
        .bindingCount = 1,
        .pBindings = &bufferBinding
      };
      VK_CHECK(vkCreateDescriptorSetLayout(vkDevice, &bufferLayoutInfo, nullptr, &_vkDescSetLayouts[eGLSLVulkanDescriptorSet_Buffer]), eFalse);
    }

    // Texture2D layout
    {
      VkDescriptorSetLayoutBinding textureBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .descriptorCount = 1,
        .stageFlags = stageFlags,
        .pImmutableSamplers = nullptr
      };
      VkDescriptorSetLayoutCreateInfo textureLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .flags = layoutFlags,
        .bindingCount = 1,
        .pBindings = &textureBinding
      };
      VK_CHECK(vkCreateDescriptorSetLayout(vkDevice, &textureLayoutInfo, nullptr, &_vkDescSetLayouts[eGLSLVulkanDescriptorSet_Texture2D]), eFalse);
    }

    // TextureCube layout
    {
      VkDescriptorSetLayoutBinding textureBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .descriptorCount = 1,
        .stageFlags = stageFlags,
        .pImmutableSamplers = nullptr
      };
      VkDescriptorSetLayoutCreateInfo textureLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .flags = layoutFlags,
        .bindingCount = 1,
        .pBindings = &textureBinding
      };
      VK_CHECK(vkCreateDescriptorSetLayout(vkDevice, &textureLayoutInfo, nullptr, &_vkDescSetLayouts[eGLSLVulkanDescriptorSet_TextureCube]), eFalse);
    }

    // Sampler layout
    {
      VkDescriptorSetLayoutBinding samplerBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = stageFlags,
        .pImmutableSamplers = nullptr
      };
      VkDescriptorSetLayoutCreateInfo samplerLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .flags = layoutFlags,
        .bindingCount = 1,
        .pBindings = &samplerBinding
      };
      VK_CHECK(vkCreateDescriptorSetLayout(vkDevice, &samplerLayoutInfo, nullptr, &_vkDescSetLayouts[eGLSLVulkanDescriptorSet_Sampler]), eFalse);
    }

    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = (tU32)_vkDescSetLayouts.size(),
      .pSetLayouts = _vkDescSetLayouts.data(),
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = nullptr
    };

    VK_CHECK(vkCreatePipelineLayout(vkDevice, &pipelineLayoutInfo, nullptr, &_vkPipelineLayout), eFalse);
    return eTrue;
  }

  tBool _CreateVulkanPipeline(
    iHString* ahspName,
    const iGpuPipelineDesc* apDesc)
  {
    niCheckIsOK(apDesc,eFalse);
    _hspName = ahspName;
    _desc = niCheckNN(_desc,apDesc->Clone(),eFalse);

    niLet vkDevice = _driver->_device;
    niLet graphics = as_nn(_driver->GetGraphics());

    // Shaders
    VkPipelineShaderStageCreateInfo shaderStages[2] = {};
    sVulkanFunction* vs = (sVulkanFunction*)_desc->GetFunction(eGpuFunctionType_Vertex);
    niCheck(vs, eFalse);
    sVulkanFunction* ps = (sVulkanFunction*)_desc->GetFunction(eGpuFunctionType_Pixel);
    niCheck(ps, eFalse);

    // Check that pipeline gpu functions use compatible bind types
    {
      niLet vsFuncBindType = vs->GetFunctionBindType();
      niLet psFuncBindType = ps->GetFunctionBindType();
      if (vsFuncBindType != psFuncBindType &&
          vsFuncBindType != eGpuFunctionBindType_None &&
          psFuncBindType != eGpuFunctionBindType_None)
      {
        niError(niFmt(
          "Incompatible gpu function bind types: vertex='%d', pixel='%d'",
          vsFuncBindType, psFuncBindType));
        return eFalse;
      }
      _gpufuncBindType = ni::Max(vsFuncBindType,psFuncBindType);
    }

    shaderStages[0] = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = vs->_vkShaderModule,
      .pName = "main"
    };
    shaderStages[1] = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = ps->_vkShaderModule,
      .pName = "main"
    };

    switch (_gpufuncBindType) {
      case eGpuFunctionBindType_None: {
        niCheck(_CreateNoneDescSetLayout(),eFalse);
        break;
      }
      case eGpuFunctionBindType_Fixed: {
        niCheck(_CreateFixedDescSetLayout(),eFalse);
        break;
      }
      default: {
        niError(niFmt("Unknown bind type '%d'.", _gpufuncBindType));
        return eFalse;
      }
    }
    niPanicAssert(_vkPipelineLayout != nullptr);

    // Vertex input
    const cFVFDescription fvfDesc(_desc->GetFVF());
    niLet vertexAttrs = Vulkan_CreateVertexInputDesc(fvfDesc.GetFVF());
    VkVertexInputBindingDescription bindingDesc = {
      .binding = 0,
      .stride = fvfDesc.GetStride(),
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &bindingDesc,
      .vertexAttributeDescriptionCount = (tU32)vertexAttrs.size(),
      .pVertexAttributeDescriptions = vertexAttrs.data()
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE
    };

    // Dynamic states
    VkDynamicState dynamicStates[] = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
      VK_DYNAMIC_STATE_BLEND_CONSTANTS,
      VK_DYNAMIC_STATE_STENCIL_REFERENCE,
      VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
    };
    VkPipelineDynamicStateCreateInfo dynamicState = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = niCountOf(dynamicStates),
      .pDynamicStates = dynamicStates
    };

    // Viewport & Scissor
    VkPipelineViewportStateCreateInfo viewportState = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .scissorCount = 1,
    };

    // Rasterization
    niLet rs = GetGpuRasterizerDesc(graphics,_desc->GetRasterizerStates());
    VkPipelineRasterizationStateCreateInfo rasterizer = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = (rs->mbWireframe ?
                      VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL),
      .cullMode = _ToVkCullMode(rs->mCullingMode),
      .frontFace = _vkFrontFace,
      .depthBiasEnable = VK_FALSE,
      .lineWidth = 1.0f
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
      .blendEnable = VK_FALSE,
      .colorWriteMask = _ToVkColorWriteMask(rs->mColorWriteMask)
    };

    // Depth stencil
    VkPipelineDepthStencilStateCreateInfo depthStencil = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO
    };
    {
      niLet ds = GetGpuDepthStencilDesc(graphics,_desc->GetDepthStencilStates());
      // Depth
      depthStencil.depthTestEnable = ds->mbDepthTest;
      depthStencil.depthWriteEnable = ds->mbDepthTestWrite;
      depthStencil.depthCompareOp = _ToVkCompareOp(ds->mDepthTestCompare);
      // Stencil
      depthStencil.stencilTestEnable = (ds->mStencilMode != eStencilMode_None);
      if (depthStencil.stencilTestEnable) {
        depthStencil.front = {
          .failOp = _ToVkStencilOp(ds->mStencilFrontFail),
          .passOp = _ToVkStencilOp(ds->mStencilFrontPassDepthPass),
          .depthFailOp = _ToVkStencilOp(ds->mStencilFrontPassDepthFail),
          .compareOp = _ToVkCompareOp(ds->mStencilFrontCompare),
          .compareMask = ds->mnStencilMask,
          .writeMask = ds->mnStencilMask,
          .reference = (tU32)ds->mnStencilRef
        };
        if (ds->mStencilMode == eStencilMode_TwoSided) {
          depthStencil.back = {
            .failOp = _ToVkStencilOp(ds->mStencilBackFail),
            .passOp = _ToVkStencilOp(ds->mStencilBackPassDepthPass),
            .depthFailOp = _ToVkStencilOp(ds->mStencilBackPassDepthFail),
            .compareOp = _ToVkCompareOp(ds->mStencilBackCompare),
            .compareMask = ds->mnStencilMask,
            .writeMask = ds->mnStencilMask,
            .reference = (tU32)ds->mnStencilRef
          };
        }
        else {
          depthStencil.back = depthStencil.front;
        }
      }
    }

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE,
    };

    // Setup blend mode if specified
    if (_desc->GetBlendMode()) {
      const sGpuBlendModeDesc* bm = (const sGpuBlendModeDesc*)_desc->GetBlendMode()->GetDescStructPtr();
      colorBlendAttachment.blendEnable = VK_TRUE;
      colorBlendAttachment.srcColorBlendFactor = _ToVkBlendFactor[bm->mSrcRGB];
      colorBlendAttachment.dstColorBlendFactor = _ToVkBlendFactor[bm->mDstRGB];
      colorBlendAttachment.colorBlendOp = _ToVkBlendOp[bm->mOp];
      colorBlendAttachment.srcAlphaBlendFactor = _ToVkBlendFactor[bm->mSrcAlpha];
      colorBlendAttachment.dstAlphaBlendFactor = _ToVkBlendFactor[bm->mDstAlpha];
      colorBlendAttachment.alphaBlendOp = _ToVkBlendOp[bm->mOp];
    }

    VkPipelineColorBlendStateCreateInfo colorBlending = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment,
    };

    VkFormat colorFormat = _GetVulkanPixelFormat(_desc->GetColorFormat(0));
    VkFormat depthFormat = _GetVulkanPixelFormat(_desc->GetDepthFormat());
    VkPipelineRenderingCreateInfo renderingInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &colorFormat,
      .depthAttachmentFormat = depthFormat,
      // TODO: Stencil format
      .stencilAttachmentFormat = VK_FORMAT_UNDEFINED
    };

    // Create the pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = 2,
      .pStages = shaderStages,
      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &inputAssembly,
      .pViewportState = &viewportState,
      .pDynamicState = &dynamicState,
      .pRasterizationState = &rasterizer,
      .pMultisampleState = &multisampling,
      .pDepthStencilState = &depthStencil,
      .pColorBlendState = &colorBlending,
      .layout = _vkPipelineLayout,
      .renderPass = VK_NULL_HANDLE,
      .subpass = 0,
      .pNext = &renderingInfo
    };

    VK_CHECK(vkCreateGraphicsPipelines(
      vkDevice, VK_NULL_HANDLE, 1,
      &pipelineInfo, nullptr, &_vkPipeline), eFalse);

    return eTrue;
  }

  virtual const iGpuPipelineDesc* __stdcall GetDesc() const niImpl {
    return _desc;
  }
};

static Ptr<sVulkanPipeline> __stdcall CreateVulkanGpuPipeline(
  ain_nn<sVulkanDriver> aDriver,
  iHString* ahspName,
  const iGpuPipelineDesc* apDesc)
{
  niCheckIsOK(apDesc,nullptr);
  NN<sVulkanPipeline> pipeline = MakeNN<sVulkanPipeline>(aDriver);
  niCheck(
    pipeline->_CreateVulkanPipeline(ahspName,apDesc),
    nullptr);
  return pipeline;
}

struct sVulkanDescriptorPool {
  VkDescriptorPool _descPool = VK_NULL_HANDLE;
  tU32 _numAllocated = 0;

  tBool _CreateDescriptorPool(VkDevice aDevice) {
    VkDescriptorPoolSize poolSizes[] = {
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, knVulkanMaxDescriptorSets},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, knVulkanMaxDescriptorSets},
      {VK_DESCRIPTOR_TYPE_SAMPLER, knVulkanMaxDescriptorSets}
    };

    VkDescriptorPoolCreateInfo poolInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .flags = 0, // No FREE_DESCRIPTOR_SET_BIT = linear allocation
      .maxSets = knVulkanMaxDescriptorSets,
      .poolSizeCount = niCountOf(poolSizes),
      .pPoolSizes = poolSizes
    };

    VK_CHECK(vkCreateDescriptorPool(aDevice, &poolInfo, nullptr, &_descPool), eFalse);
    return eTrue;
  }

  void Destroy(VkDevice aDevice) {
    if (_descPool) {
      vkDestroyDescriptorPool(aDevice, _descPool, nullptr);
      _descPool = VK_NULL_HANDLE;
    }
  }

  VkDescriptorSet AllocateDescriptorSet(VkDevice aDevice, VkDescriptorSetLayout aLayout) {
    if (!_descPool) {
      niCheck(_CreateDescriptorPool(aDevice),VK_NULL_HANDLE);
    }

    VkDescriptorSetAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = _descPool,
      .descriptorSetCount = 1,
      .pSetLayouts = &aLayout
    };

    VkDescriptorSet descSet;
    VK_CHECK(vkAllocateDescriptorSets(
      aDevice, &allocInfo, &descSet), VK_NULL_HANDLE);
    ++_numAllocated;
    return descSet;
  }

  void ResetDescriptorPool(VkDevice aDevice) {
    if (_descPool) {
      vkResetDescriptorPool(aDevice, _descPool, 0);
    }
    _numAllocated = 0;
  }

  tBool PushDescriptorUniformBuffer(
    VkDevice aDevice,
    VkCommandBuffer aCmdBuffer,
    ain<nn<sVulkanPipeline>> apPipeline,
    tU32 aSetIndex,
    VkBuffer aBuffer,
    VkDeviceSize aOffset,
    VkDeviceSize aRange = VK_WHOLE_SIZE)
  {
    niLet descSet = AllocateDescriptorSet(aDevice,apPipeline->_vkDescSetLayouts[aSetIndex]);
    niCheck(descSet != VK_NULL_HANDLE,eFalse);

    VkDescriptorBufferInfo bufferInfo = {
      .buffer = aBuffer,
      .offset = aOffset,
      .range = aRange
    };

    VkWriteDescriptorSet writes[] = {{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descSet,
        .dstBinding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .pBufferInfo = &bufferInfo
      }};

    vkUpdateDescriptorSets(aDevice,niCountOf(writes),writes,0,nullptr);
    vkCmdBindDescriptorSets(
      aCmdBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,
      apPipeline->_vkPipelineLayout,aSetIndex,1,&descSet,0,nullptr);
    return eTrue;
  }

  tBool PushDescriptorImage(
    VkDevice aDevice,
    VkCommandBuffer aCmdBuffer,
    ain<nn<sVulkanPipeline>> apPipeline,
    tU32 aSetIndex,
    VkImageView aImageView,
    VkImageLayout aImageLayout)
  {
    niLet descSet = AllocateDescriptorSet(aDevice,apPipeline->_vkDescSetLayouts[aSetIndex]);
    niCheck(descSet != VK_NULL_HANDLE,eFalse);

    VkDescriptorImageInfo imageInfo = {
      .imageView = aImageView,
      .imageLayout = aImageLayout
    };

    VkWriteDescriptorSet write = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = descSet,
      .dstBinding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
      .descriptorCount = 1,
      .pImageInfo = &imageInfo
    };
    vkUpdateDescriptorSets(aDevice,1,&write,0,nullptr);
    vkCmdBindDescriptorSets(
      aCmdBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,
      apPipeline->_vkPipelineLayout,aSetIndex,1,&descSet,0,nullptr);
    return eTrue;
  }

  tBool PushDescriptorSampler(
    VkDevice aDevice,
    VkCommandBuffer aCmdBuffer,
    ain<nn<sVulkanPipeline>> apPipeline,
    tU32 aSetIndex,
    VkSampler aSampler)
  {
    niLet descSet = AllocateDescriptorSet(aDevice,apPipeline->_vkDescSetLayouts[aSetIndex]);
    niCheck(descSet != VK_NULL_HANDLE,eFalse);

    VkDescriptorImageInfo samplerInfo = {
      .sampler = aSampler
    };

    VkWriteDescriptorSet writes[] = {{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descSet,
        .dstBinding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
        .descriptorCount = 1,
        .pImageInfo = &samplerInfo
      }};
    vkUpdateDescriptorSets(aDevice,niCountOf(writes),writes,0,nullptr);
    vkCmdBindDescriptorSets(
      aCmdBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,
      apPipeline->_vkPipelineLayout,aSetIndex,1,&descSet,0,nullptr);
    return eTrue;
  }
};

struct sVulkanEncoderFrameData : public ImplRC<iUnknown> {
  ThreadEvent _eventFrameCompleted = ThreadEvent(eFalse);
  astl::vector<Ptr<sVulkanBuffer>> _trackedBuffers;
  astl::vector<Ptr<sVulkanTexture>> _trackedTextures;
  astl::vector<Ptr<sVulkanPipeline>> _trackedPipelines;
  Ptr<iGpuStream> _stream;
  sVulkanDescriptorPool _descriptorPool;
  tBool _inFrame = eFalse;;

  sVulkanEncoderFrameData(ain<nn<sVulkanDriver>> aDriver) {
    _stream = CreateGpuStream(
      aDriver,
      eGpuBufferUsageFlags_Vertex|
      eGpuBufferUsageFlags_Index|
      eGpuBufferUsageFlags_Uniform);
  }

  void Destroy(VkDevice aDevice) {
    _descriptorPool.Destroy(aDevice);
  }

  niInline sVulkanBuffer* BindBuffer(iGpuBuffer* apBuffer) {
    niLet buffer = (sVulkanBuffer*)apBuffer; // Note: Bind() is a noop so we dont call it
    if (!buffer->_boundModifiedBuffer && buffer->_modifiedSize) {
      buffer->_boundModifiedBuffer = eTrue;
    }
    _trackedBuffers.push_back(buffer);
    return buffer;
  }

  niInline sVulkanTexture* BindTexture(iTexture* apTexture) {
    sVulkanTexture* texture = (sVulkanTexture*)apTexture; // Note: Bind() is a noop so we dont call it
    _trackedTextures.push_back(texture);
    return texture;
  }

  niInline sVulkanPipeline* BindPipeline(iGpuPipeline* apPipeline) {
    sVulkanPipeline* pipeline = (sVulkanPipeline*)apPipeline; // Note: Bind() is a noop so we dont call it
    _trackedPipelines.push_back(pipeline);
    return pipeline;
  }

  void OnBeginFrame(VkDevice aDevice) {
    niUnused(aDevice);
    niPanicAssert(_inFrame == eFalse);
    _inFrame = eTrue;
  }

  void OnEndFrame(VkDevice aDevice) {
    niLet& lastBlock = _stream->GetLastBlock();

    // niDebugFmt((
    //   "... OnFrameCompleted: sVulkanEncoderFrameData{_trackedBuffers=%d,_trackedTextures=%d,_stream._numBlocks=%d,_stream.mOffset=%d,_stream.mSize=%d,_descriptorPool._numAllocated=%d}",
    //   _trackedBuffers.size(),_trackedTextures.size(),_stream->GetNumBlocks(),
    //   lastBlock.mOffset,lastBlock.mSize,
    //   _descriptorPool._numAllocated));

    niLoop(i,_trackedBuffers.size()) {
      _trackedBuffers[i]->_Untrack();
    }
    _trackedBuffers.clear();
    _trackedTextures.clear();
    _trackedPipelines.clear();
    _stream->Reset();
    _descriptorPool.ResetDescriptorPool(aDevice);
    _inFrame = eFalse;
    _eventFrameCompleted.Signal();
  }

  void WaitFrameCompleted() {
    if (_inFrame) {
      _eventFrameCompleted.InfiniteWait();
    }
  }
};

struct sVulkanCommandEncoder : public ImplRC<iGpuCommandEncoder> {
  nn<sVulkanDriver> _driver;
  VkCommandBuffer _cmdBuffer = VK_NULL_HANDLE;
  struct sCache {
    sMaterialDesc _lastMaterial;
    Ptr<sVulkanPipeline> _lastPipeline = nullptr;
    Ptr<sVulkanBuffer> _lastBuffer = nullptr;
    tU32 _lastBufferOffset = 0;
    tFixedGpuPipelineId _lastFixedPipeline = 0;
  } _cache;
  VkFence _encoderInFlightFence = VK_NULL_HANDLE;
  tBool _beganCmdBuffer = eFalse;
  astl::vector<NN<sVulkanEncoderFrameData>> _frames;
  tU32 _currentFrame = 0;

  sVulkanCommandEncoder(ain<nn<sVulkanDriver>> aDriver, ain<tU32> aFrameMaxInFlight)
      : _driver(aDriver)
  {
    _frames.reserve(aFrameMaxInFlight);
    niLoop(i,aFrameMaxInFlight) {
      _frames.emplace_back(niNew sVulkanEncoderFrameData(aDriver));
    }
  }

  ~sVulkanCommandEncoder() {
    niLoop(i,_frames.size()) {
      _frames[i]->WaitFrameCompleted();
      _frames[i]->Destroy(_driver->_device);
    }
    if (_encoderInFlightFence) {
      vkDestroyFence(_driver->_device, _encoderInFlightFence, nullptr);
      _encoderInFlightFence = VK_NULL_HANDLE;
    }
    if (_cmdBuffer) {
      vkFreeCommandBuffers(_driver->_device, _driver->_commandPool, 1, &_cmdBuffer);
      _cmdBuffer = VK_NULL_HANDLE;
    }
  }

  sVulkanEncoderFrameData* _GetCurrentFrame() {
    return _frames[_currentFrame];
  }

  tBool _CreateCommandBuffer() {
    niCheck(_driver->_commandPool != VK_NULL_HANDLE, eFalse);

    VkCommandBufferAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = _driver->_commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1
    };

    VK_CHECK(vkAllocateCommandBuffers(_driver->_device, &allocInfo, &_cmdBuffer),eFalse);

    VkFenceCreateInfo fenceInfo = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };
    VK_CHECK(vkCreateFence(_driver->_device, &fenceInfo, nullptr, &_encoderInFlightFence), eFalse);
    return eTrue;
  }

  tBool _BeginCmdBuffer()
  {
    niCheck(_beganCmdBuffer == eFalse, eFalse);
    niDebugAssert(_cmdBuffer != VK_NULL_HANDLE);

    _beganCmdBuffer = eTrue;
    _cache = sCache {};

    vkResetCommandBuffer(_cmdBuffer, 0);

    VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    VK_CHECK(vkBeginCommandBuffer(_cmdBuffer, &beginInfo), eFalse);
    return eTrue;
  }

  tBool _BeginRendering(
    ain<VkImage> aColorImage,
    ain<VkImageView> aColorImageView,
    ain<VkImage> aDepthImage,
    ain<VkImageView> aDepthImageView,
    ain<tU32> anWidth,
    ain<tU32> anHeight,
    ain<sRecti> aViewport,
    ain<sRecti> aScissor,
    ain<sVec4f> aClearColor = Vec4f(1,0,1,0),
    ain<tF32> aClearDepth = 1.0f,
    ain<tU32> aClearStencil = 0)
  {
    niDebugAssert(_beganCmdBuffer);

    VkRenderingInfoKHR renderingInfo {
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea = {{0, 0}, {anWidth, anHeight}},
      .layerCount = 1,
      .colorAttachmentCount = 0,
      .pDepthAttachment = nullptr
    };

    VkRenderingAttachmentInfo colorAttachment {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    };
    if (aColorImageView != VK_NULL_HANDLE) {
      VkClearValue clearColorValue = {};
      static_assert(sizeof(clearColorValue) == sizeof(sVec4f));
      ((sVec4f&)clearColorValue.color) = aClearColor;
      colorAttachment.imageView = aColorImageView;
      colorAttachment.clearValue = clearColorValue;
      renderingInfo.pColorAttachments = &colorAttachment;
      renderingInfo.colorAttachmentCount = 1;
    }

    VkRenderingAttachmentInfo depthAttachment = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE
    };
    if (aDepthImageView != VK_NULL_HANDLE) {
      VkClearValue clearDepthValue = {};
      clearDepthValue.depthStencil = {aClearDepth, aClearStencil};
      depthAttachment.imageView = aDepthImageView;
      depthAttachment.clearValue = clearDepthValue;
      renderingInfo.pDepthAttachment = &depthAttachment;
    }
    vkCmdBeginRenderingKHR(_cmdBuffer, &renderingInfo);
    _GetCurrentFrame()->OnBeginFrame(_driver->_device);
    this->SetViewport(aViewport);
    this->SetScissorRect(aScissor);
    return eTrue;
  }

  void _EndRendering() {
    vkCmdEndRenderingKHR(_cmdBuffer);
  }

  tBool _EndCmdBufferAndSubmit(
    VkSemaphore aImageAvailableSemaphore,
    VkSemaphore aRendererFinishedSemaphore)
  {
    niCheck(_beganCmdBuffer == eTrue, eFalse);
    niDefer {
      _beganCmdBuffer = eFalse;
    };

    VK_CHECK(vkEndCommandBuffer(_cmdBuffer), eFalse);

    VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    VkSubmitInfo submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .pWaitDstStageMask = waitStages,
      .commandBufferCount = 1,
      .pCommandBuffers = &_cmdBuffer
    };
    if (aImageAvailableSemaphore) {
      submitInfo.waitSemaphoreCount = 1;
      submitInfo.pWaitSemaphores = &aImageAvailableSemaphore;
    }
    else {
      submitInfo.waitSemaphoreCount = 0;
    }
    if (aRendererFinishedSemaphore) {
      submitInfo.signalSemaphoreCount = 1;
      submitInfo.pSignalSemaphores = &aRendererFinishedSemaphore;
    }
    else {
      submitInfo.signalSemaphoreCount = 0;
    }

    // TODO: The usage of the fence here is suboptimal since we just wait for
    // the queue to be finished before continuing which waiting tons of CPU
    // cycles. But we want to get all the other bits right first.
    VK_CHECK(vkResetFences(_driver->_device, 1, &_encoderInFlightFence), eFalse);
    VK_CHECK(vkQueueSubmit(_driver->_graphicsQueue, 1, &submitInfo, _encoderInFlightFence), eFalse);
    VK_CHECK(vkWaitForFences(_driver->_device, 1, &_encoderInFlightFence, VK_TRUE, UINT64_MAX), eFalse);
    _GetCurrentFrame()->OnEndFrame(_driver->_device);
    return eTrue;
  }

  void _DoBindPipeline(iGpuPipeline* apPipeline, tFixedGpuPipelineId aFixedPipelineId) {
    sVulkanPipeline* pipeline = (sVulkanPipeline*)apPipeline;
    vkCmdBindPipeline(_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->_vkPipeline);
    _cache._lastPipeline = _GetCurrentFrame()->BindPipeline(apPipeline);
    _cache._lastFixedPipeline = aFixedPipelineId;
  }

  virtual void __stdcall SetPipeline(iGpuPipeline* apPipeline) niImpl {
    niCheck(apPipeline != nullptr,;);
    if ((tIntPtr)_cache._lastPipeline.raw_ptr() == (tIntPtr)apPipeline)
      return;
    _DoBindPipeline(apPipeline,0);
  }

  void __stdcall _SetFixedPipeline(iGpuPipeline* apPipeline, tFixedGpuPipelineId aFixedPipelineId) {
    _DoBindPipeline(apPipeline,aFixedPipelineId);
  }

  virtual void __stdcall SetVertexBuffer(iGpuBuffer* apBuffer, tU32 anOffset, tU32 anBinding) niImpl {
    niCheck(apBuffer != nullptr, ;);
    sVulkanBuffer* buffer = _GetCurrentFrame()->BindBuffer(apBuffer);
    VkBuffer vertexBuffers[] = {buffer->_vkBuffer};
    VkDeviceSize offsets[] = {anOffset};
    vkCmdBindVertexBuffers(_cmdBuffer, anBinding, 1, vertexBuffers, offsets);
  }

  virtual void __stdcall SetIndexBuffer(iGpuBuffer* apBuffer, tU32 anOffset, eGpuIndexType aIndexType) niImpl {
    niCheck(apBuffer != nullptr, ;);
    sVulkanBuffer* indexBuffer = _GetCurrentFrame()->BindBuffer(apBuffer);
    vkCmdBindIndexBuffer(_cmdBuffer, indexBuffer->_vkBuffer, anOffset, _ToVkIndexType[aIndexType]);
  }

  virtual void __stdcall SetUniformBuffer(iGpuBuffer* apBuffer, tU32 anOffset, tU32 anBinding) niImpl {
    niCheck(anBinding == 0, ;);
    _cache._lastBuffer = _GetCurrentFrame()->BindBuffer(apBuffer);
    _cache._lastBufferOffset = anOffset;
  }

  virtual void __stdcall SetTexture(iTexture* apTexture, tU32 anBinding) niImpl {
    niCheck(anBinding < eMaterialChannel_Last, ;);
    _cache._lastMaterial.mChannels[anBinding].mTexture = _GetCurrentFrame()->BindTexture(apTexture);
  }

  virtual void __stdcall SetSamplerState(tIntPtr ahSS, tU32 anBinding) niImpl {
    niCheck(anBinding < eMaterialChannel_Last, ;);
    _cache._lastMaterial.mChannels[anBinding].mhSS = ahSS;
  }

  virtual tBool __stdcall StreamVertexBuffer(const tPtr apData, tU32 anSize, tU32 anBinding) niImpl {
    return UpdateGpuStreamToVertexBuffer(_GetCurrentFrame()->_stream,this,apData,anSize,anBinding);
  }

  virtual tBool __stdcall StreamIndexBuffer(const tPtr apData, tU32 anSize, eGpuIndexType aIndexType) niImpl {
    return UpdateGpuStreamToIndexBuffer(_GetCurrentFrame()->_stream,this,apData,anSize,aIndexType);
  }

  virtual tBool __stdcall StreamUniformBuffer(const tPtr apData, tU32 anSize, tU32 anBinding) niImpl {
    return UpdateGpuStreamToUniformBuffer(_GetCurrentFrame()->_stream,this,apData,anSize,anBinding);
  }

  virtual void __stdcall SetPolygonOffset(const sVec2f& avOffset) niImpl {
    vkCmdSetDepthBias(_cmdBuffer, avOffset.x, 0.0f, avOffset.y);
  }

  virtual void __stdcall SetScissorRect(const sRecti& aRect) niImpl {
    VkRect2D scissor = {
      .offset = {aRect.x, aRect.y},
      .extent = {(tU32)aRect.GetWidth(), (tU32)aRect.GetHeight()}
    };
    vkCmdSetScissor(_cmdBuffer, 0, 1, &scissor);
  }

  virtual void __stdcall SetViewport(const sRecti& aRect) niImpl {
    VkViewport viewport = {
      .x = (float)aRect.x,
      .y = (float)(aRect.y + aRect.GetHeight()),
      .width = (float)aRect.GetWidth(),
      .height = (float)-aRect.GetHeight(),
      .minDepth = 0.0f,
      .maxDepth = 1.0f
    };
    vkCmdSetViewport(_cmdBuffer, 0, 1, &viewport);
  }

  virtual void __stdcall SetStencilReference(tI32 aRef) niImpl {
    vkCmdSetStencilReference(_cmdBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, aRef);
  }

  virtual void __stdcall SetStencilMask(tU32 aMask) niImpl {
    vkCmdSetStencilWriteMask(_cmdBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, aMask);
    vkCmdSetStencilCompareMask(_cmdBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, aMask);
  }

  virtual void __stdcall SetBlendColorConstant(const sColor4f& aColor) niImpl {
    vkCmdSetBlendConstants(_cmdBuffer, &aColor.x);
  }

  tBool _DoBindFixedDescLayout();
  tBool _BindGpuFunction();

  virtual tBool __stdcall DrawIndexed(eGraphicsPrimitiveType aPrimType, tU32 anNumIndices, tU32 anFirstIndex) niImpl {
    niCheck(aPrimType <= eGraphicsPrimitiveType_Last,eFalse);
    niCheck(_BindGpuFunction(),eFalse);
    vkCmdSetPrimitiveTopologyEXT(_cmdBuffer, _ToVkPrimitiveTopology[aPrimType]);
    vkCmdDrawIndexed(_cmdBuffer, anNumIndices, 1, anFirstIndex, 0, 0);
    return eTrue;
  }

  virtual tBool __stdcall Draw(eGraphicsPrimitiveType aPrimType, tU32 anVertexCount, tU32 anFirstVertex) niImpl {
    niCheck(aPrimType <= eGraphicsPrimitiveType_Last,eFalse);
    niCheck(_BindGpuFunction(),eFalse);
    vkCmdSetPrimitiveTopologyEXT(_cmdBuffer, _ToVkPrimitiveTopology[aPrimType]);
    vkCmdDraw(_cmdBuffer, anVertexCount, 1, anFirstVertex, 0);
    return eTrue;
  }
};

static Ptr<sVulkanCommandEncoder> _CreateVulkanCommandEncoder(ain_nn<sVulkanDriver> aDriver) {
  NN<sVulkanCommandEncoder> cmdEncoder = MakeNN<sVulkanCommandEncoder>(aDriver,knVulkanMaxFramesInFlight);
  niCheck(cmdEncoder->_CreateCommandBuffer(), nullptr);
  return cmdEncoder;
}

iTexture* __stdcall sVulkanDriver::CreateTexture(iHString* ahspName, eBitmapType aType, const achar* aaszFormat, tU32 anNumMipMaps, tU32 anWidth, tU32 anHeight, tU32 anDepth, tTextureFlags aFlags) {
  VULKAN_TRACE(("sVulkanDriver::CreateTexture: %s, %s, %dx%dx%d, mips:%d, flags:%d",
                ahspName,aaszFormat,anWidth,anHeight,anDepth,anNumMipMaps,aFlags));
  Ptr<iPixelFormat> pxf = _graphics->CreatePixelFormat(aaszFormat);
  niCheck(pxf.IsOK(),nullptr);
  niCheck(aType == eBitmapType_2D || aType == eBitmapType_Cube,nullptr);

  Ptr<sVulkanTexture> tex { niNew sVulkanTexture(
    as_nn(this),
    ahspName,
    aType,anWidth,anHeight,anNumMipMaps,
    _GetClosestGpuPixelFormatForTexture(pxf->GetFormat(),aFlags),
    aFlags) };
  niCheck(tex->_CreateVulkanTexture(),nullptr);
  return tex.GetRawAndSetNull();
}

tBool __stdcall sVulkanDriver::BlitBitmapToTexture(
  iBitmap2D* apSrc,
  iTexture* apDest,
  tU32 anDestLevel,
  const sRecti& aSrcRect,
  const sRecti& aDestRect,
  eTextureBlitFlags aFlags)
{
  niCheckSilent(niIsOK(apSrc),eFalse);
  niCheckSilent(niIsOK(apDest),eFalse);
  niCheckSilent(apDest->GetType() == eBitmapType_2D, eFalse);

  sVulkanTexture* tex = (sVulkanTexture*)apDest;
  Ptr<iBitmap2D> srcBmp = apSrc;

  // Check if formats are compatible, convert if needed
  if (!srcBmp->GetPixelFormat()->IsSamePixelFormat(tex->GetPixelFormat())) {
    niWarning(niFmt(
      "BlitBitmapToTexture: Converting from '%s' to '%s' - performance warning.",
      srcBmp->GetPixelFormat()->GetFormat(),
      tex->GetPixelFormat()->GetFormat()));
    srcBmp = _graphics->CreateBitmap2DEx(
      srcBmp->GetWidth(),
      srcBmp->GetHeight(),
      tex->GetPixelFormat());
    srcBmp->BlitStretch(apSrc,0,0,0,0,
                        apSrc->GetWidth(),apSrc->GetHeight(),
                        srcBmp->GetWidth(),srcBmp->GetHeight());
  }

  if (tex->_vkImage) {
    tex->_UploadTexture(srcBmp, anDestLevel, aDestRect);
  }

  VULKAN_TRACE(("BlitBitmapToTexture %s %s %s",
                aSrcRect, aDestRect, apDest->GetDeviceResourceName()));
  return eTrue;
}

tBool sVulkanDriver::_CreateVulkanDriverResources() {
  VkSamplerCreateInfo desc = {
    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    .compareEnable = VK_FALSE,
    .minLod = 0.0f,
    .maxLod = FLT_MAX,
    .unnormalizedCoordinates = VK_FALSE,
  };

#define INIT_COMPILED_SAMPLER_STATES(STATE,FILTER,WRAP) {               \
    desc.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;              \
    _toVkSamplerFilter(desc, eSamplerFilter_##FILTER, _physicalDeviceFeatures); \
    desc.addressModeU = desc.addressModeV = desc.addressModeW =         \
        _toVkSamplerAddress[eSamplerWrap_##WRAP];                       \
    VK_CHECK(vkCreateSampler(_device, &desc, nullptr,                   \
                             &_ssCompiled[eCompiledStates_##STATE - eCompiledStates_SS_PointRepeat]), eFalse); \
  }

  INIT_COMPILED_SAMPLER_STATES(SS_PointRepeat, Point, Repeat);
  INIT_COMPILED_SAMPLER_STATES(SS_PointClamp,  Point, Clamp);
  INIT_COMPILED_SAMPLER_STATES(SS_PointMirror, Point, Mirror);
  INIT_COMPILED_SAMPLER_STATES(SS_PointWhiteBorder, Point, Border);

  INIT_COMPILED_SAMPLER_STATES(SS_SmoothRepeat, Smooth, Repeat);
  INIT_COMPILED_SAMPLER_STATES(SS_SmoothClamp,  Smooth, Clamp);
  INIT_COMPILED_SAMPLER_STATES(SS_SmoothMirror, Smooth, Mirror);
  INIT_COMPILED_SAMPLER_STATES(SS_SmoothWhiteBorder, Smooth, Border);
#undef INIT_COMPILED_SAMPLER_STATES

  _dummyUniformBuffer = niNew sVulkanBuffer(as_nn(this), eGpuBufferMemoryMode_Shared, eGpuBufferUsageFlags_Uniform);
  niCheck(_dummyUniformBuffer->Create(1024),eFalse);
  {
    tPtr data = _dummyUniformBuffer->Lock(0,1024,eLock_Discard);
    ni::MemZero(data,1024);
    _dummyUniformBuffer->Unlock();
  }

  return eTrue;
}

tBool sVulkanCommandEncoder::_DoBindFixedDescLayout() {
  niLet& mat = _cache._lastMaterial;
  niLet pipeline = as_nn(_cache._lastPipeline);
  niLet device = _driver->_device;
  niLetMut& descPool = _GetCurrentFrame()->_descriptorPool;

  {
    sVulkanBuffer* buffer = _cache._lastBuffer.raw_ptr();
    tU32 bufferOffset = 0;
    if (buffer) {
      bufferOffset = _cache._lastBufferOffset;
    }
    else {
      buffer = _driver->_dummyUniformBuffer.raw_ptr();
    }
    niCheck(descPool.PushDescriptorUniformBuffer(
      device,_cmdBuffer,
      pipeline,eGLSLVulkanDescriptorSet_Buffer,
      buffer->_vkBuffer,bufferOffset),eFalse);
  }

  {
    sVulkanTexture* texture = (sVulkanTexture*)_cache._lastMaterial.mChannels[0].mTexture.raw_ptr();
    if (!texture) {
      texture = (sVulkanTexture*)_driver->_fixedPipelines->GetWhiteTexture().raw_ptr();
    }
    switch (texture->GetType()) {
      case eBitmapType_Cube: {
        niCheck(descPool.PushDescriptorImage(
          device,_cmdBuffer,
          pipeline,eGLSLVulkanDescriptorSet_TextureCube,
          texture->_vkView,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),eFalse);
        break;
      }
      case eBitmapType_2D: {
        niCheck(descPool.PushDescriptorImage(
          device,_cmdBuffer,
          pipeline,eGLSLVulkanDescriptorSet_Texture2D,
          texture->_vkView,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),eFalse);
        break;
      }
      default: {
        niError(niFmt(
          "Can't bind texture '%s' (type:%d), invalid type.",
          texture->GetDeviceResourceName(),
          texture->GetType()));
        return eFalse;
      }
    }
  }

  {
    tIntPtr hSS = _cache._lastMaterial.mChannels[0].mhSS;
    niCheck(descPool.PushDescriptorSampler(
      device,_cmdBuffer,
      pipeline,eGLSLVulkanDescriptorSet_Sampler,
      _driver->_GetVkSamplerState(hSS)),eFalse);
  }

  return eTrue;
}

tBool sVulkanCommandEncoder::_BindGpuFunction() {
  niLet pipeline = _cache._lastPipeline;
  if (pipeline->_gpufuncBindType == eGpuFunctionBindType_Fixed) {
    niCheck(_DoBindFixedDescLayout(),eFalse);
    return eTrue;
  }
  return eTrue;
}

struct sVulkanSurface {
  VkImage _image = VK_NULL_HANDLE;
  VkImageView _imageView = VK_NULL_HANDLE;

  tBool __stdcall IsOK() const {
    return _imageView != VK_NULL_HANDLE;
  }
};

struct sVulkanContextBase :
    public sGraphicsContext<1,ni::ImplRC<
                             iGraphicsContextRT,
                             eImplFlags_DontInherit1,
                             iGraphicsContext,
                             iGraphicsContextGpu> >
{
  const tU32 _frameMaxInFlight;
  nn<sVulkanDriver> _driver;
  NN<sVulkanCommandEncoder> _cmdEncoder = niDeferredInit(NN<sVulkanCommandEncoder>);
  eGpuPixelFormat _rt0Format = eGpuPixelFormat_None;
  eGpuPixelFormat _dsFormat = eGpuPixelFormat_None;
  tBool _beganFrame = eFalse;
  tU64 _frameCounter = 0;

  sVulkanContextBase(ain<nn<sVulkanDriver>> aDriver, const tU32 aFrameMaxInFlight)
      : tGraphicsContextBase(aDriver->_graphics)
      , _driver(aDriver)
      , _frameMaxInFlight(aFrameMaxInFlight)
      , _cmdEncoder(as_NN(_CreateVulkanCommandEncoder(_driver)))
  {
  }

  virtual ~sVulkanContextBase() {
    this->Invalidate();
  }

  virtual tBool __stdcall IsOK() const {
    return mptrRT[0].IsOK();
  }

  virtual iGraphics* __stdcall GetGraphics() const {
    return _driver->_graphics;
  }
  virtual iGraphicsDriver* __stdcall GetDriver() const {
    return _driver;
  }

  virtual iGpuCommandEncoder* __stdcall GetCommandEncoder() niImpl {
    if (!_beganFrame) {
      niCheck(_BeginFrame(),nullptr);
    }
    return _cmdEncoder;
  }

  tBool __stdcall _ResizeContextRTDS(const achar* aKind, ain<tU32> w, ain<tU32> h) {
    iGraphics* g = _driver->_graphics;
    ni::SafeInvalidate(mptrRT[0].ptr());
    mptrRT[0] = niNew sVulkanTexture(
      _driver,HFmt("Vulkan_MainRT_%s_%p",aKind,(tIntPtr)this),
      eBitmapType_2D,w,h,0,eGpuPixelFormat_RGBA8,
      eTextureFlags_RenderTarget|eTextureFlags_Surface);
    _rt0Format = _GetClosestGpuPixelFormatForRT(
      mptrRT[0]->GetPixelFormat()->GetFormat());

    ni::SafeInvalidate(mptrDS.ptr());
    mptrDS = niNew sVulkanTexture(
      _driver,HFmt("Vulkan_MainDS_%s_%p",aKind,(tIntPtr)this),
      eBitmapType_2D,w,h,0,eGpuPixelFormat_D32,
      eTextureFlags_DepthStencil|eTextureFlags_Surface);
    _dsFormat = _GetClosestGpuPixelFormatForDS(
      mptrDS->GetPixelFormat()->GetFormat());

    SetViewport(sRecti(0,0,w,h));
    SetScissorRect(sRecti(0,0,w,h));

    niLog(Info, niFmt(
      "Vulkan Context Resized: %s (%p), %dx%d, BB: %s, DS: %s, VP: %s, SC: %s",
      aKind,
      (tIntPtr)this,w,h,
      mptrRT[0]->GetPixelFormat()->GetFormat(),
      mptrDS->GetPixelFormat()->GetFormat(),
      GetViewport(),
      GetScissorRect()));
    return eTrue;
  }

  virtual tBool _BeginFrame() = 0;

  virtual void __stdcall ClearBuffers(tClearBuffersFlags clearBuffer, tU32 anColor, tF32 afDepth, tI32 anStencil) {
    niUnused(anStencil);
    this->ClearBuffersRect(
      clearBuffer,
      Rectf(0,0,(tF32)this->GetWidth(),(tF32)this->GetHeight()),
      anColor, afDepth);
  }

  virtual tBool __stdcall ClearBuffersRect(tClearBuffersFlags aFlags, const sRectf& aRect, tU32 anColor, tF32 afZ) {
    if (!_beganFrame) {
      niCheck(_BeginFrame(),eFalse);
    }
    niLet pixelSize = Vec2f(
      2.0f / (tF32)this->GetWidth(),
      2.0f / (tF32)this->GetHeight()
    );
    return _driver->_fixedPipelines->ClearRect(_cmdEncoder,pixelSize,aFlags,aRect,anColor,afZ);
  }

  virtual tBool __stdcall DrawOperation(iDrawOperation* apDrawOp) {
    niCheckSilent(niIsOK(apDrawOp), eFalse);

    if (!_beganFrame) {
      niCheck(_BeginFrame(),eFalse);
    }

    niLet doCapture = _driver->_drawOpCapture.IsOK();
    niDefer {
      if (doCapture) {
        _driver->_drawOpCapture->EndCaptureDrawOp(this,apDrawOp,sVec4i::Zero());
      }
    };
    if (doCapture) {
      if (!_driver->_drawOpCapture->BeginCaptureDrawOp(this,apDrawOp,sVec4i::Zero()))
        return eTrue;
    }

    iVertexArray* va = apDrawOp->GetVertexArray();
    if (!va) {
      return eFalse;
    }

    VULKAN_TRACE(("DrawOperation BEGIN %s:%s",this->GetWidth(),this->GetHeight()));
    niLet fvf = va->GetFVF();
    niLet pDOMatDesc = (const sMaterialDesc*)apDrawOp->GetMaterial()->GetDescStructPtr();

    iGpuFunction* funcVertex = _driver->_fixedPipelines->GetFixedGpuFuncVertex(fvf);
    iGpuFunction* funcPixel = _driver->_fixedPipelines->GetFixedGpuFuncPixel(*pDOMatDesc);
    const tFixedGpuPipelineId rpId = GetFixedGpuPipelineId(
      _rt0Format, _dsFormat,
      fvf,
      _GetBlendMode(pDOMatDesc),
      (eCompiledStates)_GetRS(pDOMatDesc),
      (eCompiledStates)_GetDS(pDOMatDesc),
      funcVertex, funcPixel);
    niCheck(rpId != 0, eFalse);

    niLetMut& cmdStateCache = _cmdEncoder->_cache;
    if (rpId != cmdStateCache._lastFixedPipeline) {
      iGpuPipeline* pipeline = _driver->_fixedPipelines->GetRenderPipeline(
        _driver, rpId,
        funcVertex, funcPixel).raw_ptr();
      if (!pipeline) {
        niError("Can't get the pipeline.");
        return eFalse;
      }
      _cmdEncoder->_SetFixedPipeline(pipeline,rpId);
    }

    _cmdEncoder->SetViewport(mrectViewport);
    _cmdEncoder->SetScissorRect(mrectScissor);

    TestGpuFuncs_TestUniforms fixedUniforms;
    {
      const sMaterialChannel& chBase = _GetChannel(pDOMatDesc, eMaterialChannel_Base);
      const sMaterialChannel& chOpacity = _GetChannel(pDOMatDesc, eMaterialChannel_Opacity);
      _cmdEncoder->SetTexture(chBase.mTexture, 0);
      _cmdEncoder->SetSamplerState(chBase.mhSS, 0);

      if (pDOMatDesc->mFlags & eMaterialFlags_DiffuseModulate ||
          !chBase.mTexture.raw_ptr())
      {
        fixedUniforms.materialColor = chBase.mColor;
      }
      else {
        fixedUniforms.materialColor = sColor4f::White();
      }
      fixedUniforms.alphaRef = chOpacity.mColor.w;
    }

    {
      sMatrixf mtxVP = this->GetFixedStates()->GetViewProjectionMatrix();
      fixedUniforms.mtxWVP = apDrawOp->GetMatrix() * mtxVP;
      _cmdEncoder->StreamUniformBuffer((tPtr)&fixedUniforms,sizeof(fixedUniforms),0);
    }

    return DrawOperationSubmitGpuDrawCall(_cmdEncoder,apDrawOp);
  }

  /////////////////////////////////////////////
  virtual iBitmap2D* __stdcall CaptureFrontBuffer() const {
    return nullptr;
  }
};

#if defined niOSX
struct sVulkanContextWindowMetal : public sVulkanContextBase {
  Ptr<iOSWindow> _window;
  Ptr<iOSXMetalAPI> _metalAPI;
  VkSemaphore _renderFinishedSemaphore = VK_NULL_HANDLE;
  struct {
    VkImage _colorImage = VK_NULL_HANDLE;
    VkImageView _colorImageView = VK_NULL_HANDLE;
    VkImage _depthImage = VK_NULL_HANDLE;
    VkImageView _depthImageView = VK_NULL_HANDLE;
    sVec2i _size = Vec2i(1,1);
  } _metalSurface;

  sVulkanContextWindowMetal(
    ain<nn<sVulkanDriver>> aDriver,
    const tU32 aFrameMaxInFlight,
    iOSWindow* apWindow)
      : sVulkanContextBase(aDriver,aFrameMaxInFlight)
  {
    _window = apWindow;
  }

  virtual ~sVulkanContextWindowMetal() {
    this->Invalidate();
  }

  tBool _ResizeContextRTDS(ain<sVec2i> aNewSize) {
    tBool resizedContextRTDS = static_cast<sVulkanContextBase*>(this)->_ResizeContextRTDS(
      "WindowMetal",aNewSize.x,aNewSize.y);
    niCheck(resizedContextRTDS,eFalse);
    return eTrue;
  }

  tBool _CreateContextWindowMetal() {
    niCheckIsOK(_window,eFalse);
    osxMetalSetDefaultDevice();
    _metalAPI = osxMetalCreateAPIForWindow(osxMetalGetDevice(),_window);
    if (!_metalAPI.IsOK()) {
      niError("Can't get metal api for iOSWindow.");
      return eFalse;
    }

    VkSemaphoreCreateInfo semaphoreInfo = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    VK_CHECK(vkCreateSemaphore(_driver->_device, &semaphoreInfo, nullptr, &_renderFinishedSemaphore), eFalse);
    niCheck(_UpdateSurfaceFromMetalAPI(_driver->_device),eFalse);
    return eTrue;
  }

  void __stdcall Invalidate() niImpl {
    // Wait for the device to finish all operations before destroying objects.
    vkDeviceWaitIdle(_driver->_device);
    _DestroySurface(_driver->_device);
    if (_renderFinishedSemaphore) {
      vkDestroySemaphore(_driver->_device, _renderFinishedSemaphore, nullptr);
      _renderFinishedSemaphore = VK_NULL_HANDLE;
    }
    _metalAPI = nullptr;
    _window = nullptr;
  }

  tBool _BeginFrame() niImpl {
    niPanicAssert(_beganFrame == eFalse);
    _beganFrame = eTrue;

    // Get the next view
    niCheck(_UpdateSurfaceFromMetalAPI(_driver->_device),eFalse);

    // Begin buffer and rendering
    niCheck(_cmdEncoder->_BeginCmdBuffer(),eFalse);
    niCheck(_cmdEncoder->_BeginRendering(
      _metalSurface._colorImage,_metalSurface._colorImageView,
      _metalSurface._depthImage,_metalSurface._depthImageView,
      this->GetWidth(),this->GetHeight(),
      mrectViewport,mrectScissor),eFalse);

    return eTrue;
  }

  tBool __stdcall Display(tGraphicsDisplayFlags aFlags, const sRecti& aRect) niImpl {
    niCheckIsOK(_window,eFalse);
    niCheck(_beganFrame,eFalse);
    _beganFrame = eFalse;
    _cmdEncoder->_EndRendering();
    niCheck(_cmdEncoder->_EndCmdBufferAndSubmit(
      VK_NULL_HANDLE,
      _renderFinishedSemaphore),eFalse);
    _metalAPI->DrawablePresent();
    return eTrue;
  }

  tBool _UpdateSurfaceFromMetalAPI(VkDevice aDevice) {
    _DestroySurface(aDevice);

    niCheck(osxVkCreateImageForMetalAPI(
      _metalAPI,aDevice,nullptr,60,
      &_metalSurface._colorImage,
      &_metalSurface._depthImage),eFalse);
    niLet viewSize = _metalAPI->GetViewSize();
    _metalSurface._size = Vec2i(viewSize.x,viewSize.y);

    // niDebugFmt((
    //   "... _UpdateSurfaceFromMetalAPI: color: %s, depth: %s, size: %s",
    //   (tIntPtr)_metalSurface._colorImage,
    //   (tIntPtr)_metalSurface._depthImage,
    //   _metalSurface._size));

    if (_metalSurface._colorImage) {
      VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = _metalSurface._colorImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_B8G8R8A8_UNORM,
        .components = {
          VK_COMPONENT_SWIZZLE_IDENTITY,
          VK_COMPONENT_SWIZZLE_IDENTITY,
          VK_COMPONENT_SWIZZLE_IDENTITY,
          VK_COMPONENT_SWIZZLE_IDENTITY
        },
        .subresourceRange = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .baseMipLevel = 0,
          .levelCount = 1,
          .baseArrayLayer = 0,
          .layerCount = 1
        }
      };
      VK_CHECK(vkCreateImageView(aDevice, &viewInfo, nullptr, &_metalSurface._colorImageView), eFalse);
    }

    if (_metalSurface._depthImage) {
      VkImageViewCreateInfo depthViewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = _metalSurface._depthImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_D32_SFLOAT,
        .components = {
          VK_COMPONENT_SWIZZLE_IDENTITY,
          VK_COMPONENT_SWIZZLE_IDENTITY,
          VK_COMPONENT_SWIZZLE_IDENTITY,
          VK_COMPONENT_SWIZZLE_IDENTITY
        },
        .subresourceRange = {
          .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
          .baseMipLevel = 0,
          .levelCount = 1,
          .baseArrayLayer = 0,
          .layerCount = 1
        }
      };
      VK_CHECK(vkCreateImageView(aDevice, &depthViewInfo, nullptr, &_metalSurface._depthImageView), eFalse);
    }

    if ((!mptrRT[0].raw_ptr()) ||
        (_metalSurface._size.x != this->GetWidth()) ||
        (_metalSurface._size.y != this->GetHeight()))
    {
      niCheck(_ResizeContextRTDS(_metalSurface._size),eFalse);
    }
    return eTrue;
  }

  void _DestroySurface(VkDevice device) {
    if (_metalSurface._depthImageView) {
      vkDestroyImageView(device, _metalSurface._depthImageView, nullptr);
      _metalSurface._depthImageView = VK_NULL_HANDLE;
    }
    if (_metalSurface._depthImage) {
      vkDestroyImage(device, _metalSurface._depthImage, nullptr);
      _metalSurface._depthImage = VK_NULL_HANDLE;
    }
    if (_metalSurface._colorImageView) {
      vkDestroyImageView(device, _metalSurface._colorImageView, nullptr);
      _metalSurface._colorImageView = VK_NULL_HANDLE;
    }
    if (_metalSurface._colorImage) {
      vkDestroyImage(device, _metalSurface._colorImage, nullptr);
      _metalSurface._colorImage = VK_NULL_HANDLE;
    }
  }
};
#endif

#if defined niLinuxDesktop
struct sVulkanContextWindowX11 : public sVulkanContextBase {
  Ptr<iOSWindow> _window;
  VkSurfaceKHR _surface = VK_NULL_HANDLE;
  VkSwapchainKHR _swapchain = VK_NULL_HANDLE;

  VkSemaphore _imageAvailableSemaphore = VK_NULL_HANDLE;
  VkSemaphore _renderFinishedSemaphore = VK_NULL_HANDLE;

  uint32_t _currentImageIndex = 0;

  // Store swapchain images, image views, and format
  astl::vector<VkImage> _swapchainImages;
  astl::vector<VkImageView> _swapchainImageViews;
  VkFormat _swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;
  VkExtent2D _swapchainExtent = {0,0};
  sVec2i _swapchainWindowSize = sVec2i::Zero();

  sVulkanContextWindowX11(
    ain<nn<sVulkanDriver>> aDriver,
    const tU32 aFrameMaxInFlight,
    iOSWindow* apWindow)
      : sVulkanContextBase(aDriver,aFrameMaxInFlight)
  {
    _window = apWindow;
  }

  ~sVulkanContextWindowX11() {
    this->Invalidate();
  }

  tBool _ResizeContextRTDS() {
    niLet resizedContextRTDS = static_cast<sVulkanContextBase*>(this)->_ResizeContextRTDS(
      "WindowX11",_swapchainExtent.width,_swapchainExtent.height);
    niCheck(resizedContextRTDS,eFalse);
    niLet dsTex = (sVulkanTexture*)mptrDS.raw_ptr();
    if (dsTex) {
      niCheck(dsTex->_CreateVulkanTexture(),eFalse);
    }
    return eTrue;
  }

  tBool _CreateContextWindowX11() {
    VkSemaphoreCreateInfo semaphoreInfo = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    VK_CHECK(vkCreateSemaphore(_driver->_device, &semaphoreInfo, nullptr, &_renderFinishedSemaphore), eFalse);
    niCheck(_CreateSurface(), eFalse);
    niCheck(_CreateSwapChain(), eFalse);
    niCheck(_ResizeContextRTDS(), eFalse);
    return eTrue;
  }

  void __stdcall Invalidate() niImpl {
    // Wait for the device to finish all operations before destroying objects.
    vkDeviceWaitIdle(_driver->_device);
    _DestroySwapChainResources();
    if (_renderFinishedSemaphore) {
      vkDestroySemaphore(_driver->_device, _renderFinishedSemaphore, nullptr);
      _renderFinishedSemaphore = VK_NULL_HANDLE;
    }
    if (_surface != VK_NULL_HANDLE) {
      vkDestroySurfaceKHR(_driver->_instance, _surface, nullptr);
      _surface = VK_NULL_HANDLE;
    }
    _window = nullptr;
  }

  tBool _CreateSurface() {
    sOSWindowXWinHandles xwinHandles = {};
    niCheck(linuxGetOSWindowXWinHandles(_window,xwinHandles),eFalse);

    VkXlibSurfaceCreateInfoKHR createInfo = {
      .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
      .dpy = xwinHandles._display,
      .window = xwinHandles._window
    };

    VK_CHECK(vkCreateXlibSurfaceKHR(
      _driver->_instance,
      &createInfo,
      nullptr,
      &_surface),
      eFalse);

    return eTrue;
  }

  tBool _CreateSwapChain() {
    VkSemaphoreCreateInfo semaphoreInfo = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    VK_CHECK(vkCreateSemaphore(_driver->_device, &semaphoreInfo, nullptr, &_imageAvailableSemaphore), eFalse);

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      _driver->_physicalDevice,
      _surface,
      &capabilities);

    // We assume we can use our chosen format and present mode directly
    _swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    VkSwapchainCreateInfoKHR createInfo = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = _surface,
      .minImageCount = capabilities.minImageCount,
      .imageFormat = _swapchainImageFormat,
      .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
      .imageExtent = capabilities.currentExtent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .preTransform = capabilities.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = VK_PRESENT_MODE_FIFO_KHR,
      .clipped = VK_TRUE
    };

    VK_CHECK(vkCreateSwapchainKHR(_driver->_device, &createInfo, nullptr, &_swapchain), eFalse);
    _swapchainExtent = capabilities.currentExtent;

    // Retrieve swapchain images
    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(_driver->_device, _swapchain, &imageCount, nullptr);
    _swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(_driver->_device, _swapchain, &imageCount, _swapchainImages.data());

    // Create image views for each swapchain image
    _swapchainImageViews.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; i++) {
      VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = _swapchainImages[i],
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = _swapchainImageFormat,
        .components = {
          VK_COMPONENT_SWIZZLE_IDENTITY,
          VK_COMPONENT_SWIZZLE_IDENTITY,
          VK_COMPONENT_SWIZZLE_IDENTITY,
          VK_COMPONENT_SWIZZLE_IDENTITY
        },
        .subresourceRange = {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .baseMipLevel = 0,
          .levelCount = 1,
          .baseArrayLayer = 0,
          .layerCount = 1
        }
      };
      VK_CHECK(vkCreateImageView(_driver->_device, &viewInfo, nullptr, &_swapchainImageViews[i]), eFalse);
    }

    _swapchainWindowSize = _window->GetSize();
    return eTrue;
  }

  void _DestroySwapChainResources() {
    if (_imageAvailableSemaphore) {
      vkDestroySemaphore(_driver->_device, _imageAvailableSemaphore, nullptr);
      _imageAvailableSemaphore = VK_NULL_HANDLE;
    }

    // Destroy all swapchain image views
    for (auto iv : _swapchainImageViews) {
      if (iv != VK_NULL_HANDLE) {
        vkDestroyImageView(_driver->_device, iv, nullptr);
      }
    }
    _swapchainImageViews.clear();

    if (_swapchain != VK_NULL_HANDLE) {
      vkDestroySwapchainKHR(_driver->_device, _swapchain, nullptr);
      _swapchain = VK_NULL_HANDLE;
    }

    _swapchainImages.clear();
    _swapchainWindowSize = sVec2i::Zero();
  }

  tBool _RecreateSwapchain(const achar* aaszReason) {
    vkDeviceWaitIdle(_driver->_device);
    niInfo(niFmt("Recreating vulkan swapchain: %s.", aaszReason));

    _DestroySwapChainResources();
    if (mptrDS.IsOK()) {
      mptrDS->Invalidate();
    }

    if (!_CreateSwapChain()) {
      niError("Failed to recreate swapchain.");
      return eFalse;
    }

    if (!_ResizeContextRTDS()) {
      niError("Failed to resize context after recreating swapchain.");
      return eFalse;
    }

    return eTrue;
  }

  tBool _BeginFrame() niImpl {
    niPanicAssert(_beganFrame == eFalse);
    _beganFrame = eTrue;

    // Note: Detect window size changes as VK_ERROR_OUT_OF_DATE_KHR &
    // VK_SUBOPTIMAL_KHR are not reliabily returned by all platforms and
    // drivers. The size of the window might be different from the surface's
    // size depending on the window manager, contents scaling, etc, so we only
    // compare it against itself and not against the surface size.
    if (_swapchainWindowSize != _window->GetSize()) {
      if (!_RecreateSwapchain("swapchain window size changed")) {
        niError("Failed to recreate swapchain after swapchain window size changed.");
        return eFalse;
      }
    }

    VkResult acquireRes = vkAcquireNextImageKHR(
      _driver->_device,
      _swapchain,
      UINT64_MAX,
      _imageAvailableSemaphore,
      VK_NULL_HANDLE,
      &_currentImageIndex);
    if (acquireRes != VK_SUCCESS) {
      niLoop(i,5) {
        if (acquireRes == VK_ERROR_OUT_OF_DATE_KHR || acquireRes == VK_SUBOPTIMAL_KHR) {
          if (!_RecreateSwapchain(niFmt("vkAcquireNextImageKHR(try:%s) -> %s",i,ni_vulkan::VkResultToString(acquireRes)))) {
            niError("Failed to recreate swapchain after AcquireNextImage out-of-date.");
            return eFalse;
          }
          acquireRes = vkAcquireNextImageKHR(
            _driver->_device,
            _swapchain,
            UINT64_MAX,
            _imageAvailableSemaphore,
            VK_NULL_HANDLE,
            &_currentImageIndex);
          if (acquireRes == VK_SUCCESS)
            break;
        }
        else {
          VK_CHECK(acquireRes, eFalse);
        }
      }
    }

    niLet currentImage = _swapchainImages[_currentImageIndex];
    niLet currentImageView = _swapchainImageViews[_currentImageIndex];

    niCheck(_cmdEncoder->_BeginCmdBuffer(),eFalse);

    // Transition the image from UNDEFINED to COLOR_ATTACHMENT_OPTIMAL (or
    // another suitable layout) here using vkCmdPipelineBarrier.  This must
    // happen before any rendering commands.
    niCheck(_VulkanTransitionImageLayout(
      _cmdEncoder->_cmdBuffer,currentImage,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),eFalse);

    sVulkanTexture* ds = (sVulkanTexture*)mptrDS.ptr();
    if (ds && ds->_vkImage) {
      niCheck(_VulkanTransitionImageLayout(
        _cmdEncoder->_cmdBuffer,ds->_vkImage,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL),eFalse);
    }

    niCheck(_cmdEncoder->_BeginRendering(
      currentImage,currentImageView,
      ds ? ds->_vkImage : VK_NULL_HANDLE,
      ds ? ds->_vkView : VK_NULL_HANDLE,
      this->GetWidth(),this->GetHeight(),
      mrectViewport,mrectScissor),eFalse);
    return eTrue;
  }

  virtual tBool __stdcall Display(tGraphicsDisplayFlags aFlags, const sRecti& aRect) niImpl {
    niCheckIsOK(_window,eFalse);
    niCheck(_beganFrame,eFalse);
    _beganFrame = eFalse;

    _cmdEncoder->_EndRendering();
    niCheck(_VulkanTransitionImageLayout(
      _cmdEncoder->_cmdBuffer,_swapchainImages[_currentImageIndex],
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR),eFalse);

    sVulkanTexture* ds = (sVulkanTexture*)mptrDS.ptr();
    if (ds && ds->_vkImage) {
      niCheck(_VulkanTransitionImageLayout(
        _cmdEncoder->_cmdBuffer,ds->_vkImage,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),eFalse);
    }

    niCheck(_cmdEncoder->_EndCmdBufferAndSubmit(
      _imageAvailableSemaphore,
      _renderFinishedSemaphore),eFalse);

    VkPresentInfoKHR presentInfo = {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &_renderFinishedSemaphore,
      .swapchainCount = 1,
      .pSwapchains = &_swapchain,
      .pImageIndices = &_currentImageIndex,
    };

    VkResult presentRes = vkQueuePresentKHR(_driver->_graphicsQueue, &presentInfo);
    if (presentRes == VK_ERROR_OUT_OF_DATE_KHR || presentRes == VK_SUBOPTIMAL_KHR) {
      if (!_RecreateSwapchain(niFmt("vkQueuePresentKHR -> %s",ni_vulkan::VkResultToString(presentRes)))) {
        niError("Failed to recreate swapchain after Present out-of-date.");
        return eFalse;
      }
    }
    else {
      VK_CHECK(presentRes, eFalse);
    }

    ++_frameCounter;
    return eTrue;
  }
};
#endif

iGraphicsContext* sVulkanDriver::CreateContextForWindow(
  iOSWindow* apWindow,
  const achar* aaszBBFormat,
  const achar* aaszDSFormat,
  tU32 anSwapInterval,
  tTextureFlags aBackBufferFlags)
{
  niCheckIsOK(apWindow,nullptr);
  niUnused(anSwapInterval);
  niUnused(aBackBufferFlags);
  niUnused(aaszBBFormat);
  niUnused(aaszDSFormat);
  if (!_fixedPipelines.IsOK()) {
    _fixedPipelines = niCheckNN(_fixedPipelines, CreateFixedGpuPipelines(this), nullptr);
  }

#if defined niOSX
  Ptr<sVulkanContextWindowMetal> gc = niCheckNN(
    gc, niNew sVulkanContextWindowMetal(
      as_nn(this),knVulkanMaxFramesInFlight,apWindow), nullptr);
  niCheck(gc->_CreateContextWindowMetal(),nullptr);
#elif defined niLinuxDesktop
  Ptr<sVulkanContextWindowX11> gc = niCheckNN(gc, niNew sVulkanContextWindowX11(
    as_nn(this),knVulkanMaxFramesInFlight,apWindow), nullptr);
  niCheck(gc->_CreateContextWindowX11(),nullptr);
#else
#error "sVulkanDriver::CreateContextForWindow: Unsupported platform!"
#endif

  return gc.GetRawAndSetNull();
}

struct sVulkanContextRT : public sVulkanContextBase {
  sVulkanContextRT(
    ain<nn<sVulkanDriver>> aDriver,
    const tU32 aFrameMaxInFlight,
    iTexture* apRT0,
    iTexture* apDS)
      : sVulkanContextBase(aDriver,aFrameMaxInFlight)
  {
  }

  virtual ~sVulkanContextRT() {
    this->Invalidate();
  }

  tBool _CreateContextRT(iTexture* apRT0, iTexture* apDS) {
    niCheckIsOK(apRT0,eFalse);
    mptrRT[0] = apRT0;
    _rt0Format = _GetClosestGpuPixelFormatForRT(
      apRT0->GetPixelFormat()->GetFormat());

    if (apDS) {
      mptrDS = apDS;
      _dsFormat = _GetClosestGpuPixelFormatForDS(
        apDS->GetPixelFormat()->GetFormat());
    }

    // niDebugFmt((
    //   "... sVulkanContextRT::_CreateContextRT: rt0: %p, ds: %p",
    //   (tIntPtr)apRT0,(tIntPtr)apDS));

    SetViewport(sRecti(0,0,apRT0->GetWidth(),apRT0->GetHeight()));
    SetScissorRect(sRecti(0,0,apRT0->GetWidth(),apRT0->GetHeight()));
    return eTrue;
  }

  tBool _BeginFrame() niImpl {
    niPanicAssert(_beganFrame == eFalse);
    _beganFrame = eTrue;

    // niDebugFmt((
    //   "... sVulkanContextRT::_BeginFrame: rt0: %p, ds: %p",
    //   (tIntPtr)mptrRT[0].ptr(),(tIntPtr)mptrDS.ptr()));

    niCheck(_cmdEncoder->_BeginCmdBuffer(),eFalse);
    sVulkanTexture* rt0 = (sVulkanTexture*)mptrRT[0].ptr();
    if (rt0) {
      niCheck(_VulkanTransitionImageLayout(
        _cmdEncoder->_cmdBuffer,rt0->_vkImage,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),eFalse);
    }

    sVulkanTexture* ds = (sVulkanTexture*)mptrDS.ptr();
    if (ds) {
      niCheck(_VulkanTransitionImageLayout(
        _cmdEncoder->_cmdBuffer,ds->_vkImage,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL),eFalse);
    }

    niCheck(_cmdEncoder->_BeginRendering(
      rt0?rt0->_vkImage:VK_NULL_HANDLE,
      rt0?rt0->_vkView:VK_NULL_HANDLE,
      ds?ds->_vkImage:VK_NULL_HANDLE,
      ds?ds->_vkView:VK_NULL_HANDLE,
      this->GetWidth(),this->GetHeight(),
      mrectViewport,mrectScissor),eFalse);
    return eTrue;
  }

  virtual tBool __stdcall Display(tGraphicsDisplayFlags aFlags, const sRecti& aRect) niImpl {
    niCheck(_beganFrame,eFalse);
    _beganFrame = eFalse;
    _cmdEncoder->_EndRendering();

    // Add transition to shader read
    sVulkanTexture* rt0 = (sVulkanTexture*)mptrRT[0].ptr();
    if (rt0) {
      niCheck(_VulkanTransitionImageLayout(
        _cmdEncoder->_cmdBuffer,rt0->_vkImage,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),eFalse);
    }

    sVulkanTexture* ds = (sVulkanTexture*)mptrDS.ptr();
    if (ds) {
      niCheck(_VulkanTransitionImageLayout(
        _cmdEncoder->_cmdBuffer,ds->_vkImage,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),eFalse);
    }

    niCheck(_cmdEncoder->_EndCmdBufferAndSubmit(
      VK_NULL_HANDLE,VK_NULL_HANDLE),eFalse);
    return eTrue;
  }
};

iGraphicsContextRT* sVulkanDriver::CreateContextForRenderTargets(
  iTexture* apRT0, iTexture* apRT1, iTexture* apRT2, iTexture* apRT3, iTexture* apDS)
{
  if (!_fixedPipelines.IsOK()) {
    _fixedPipelines = niCheckNN(_fixedPipelines, CreateFixedGpuPipelines(this), nullptr);
  }

  Ptr<sVulkanContextRT> gc = niNew sVulkanContextRT(
    as_nn(this),knVulkanMaxFramesInFlight,apRT0,apDS);
  niCheck(gc->_CreateContextRT(apRT0,apDS),nullptr);
  return gc.GetRawAndSetNull();
}

Ptr<iGpuBuffer> sVulkanDriver::CreateGpuBuffer(iHString* ahspName, tU32 anSize, eGpuBufferMemoryMode aMemMode, tGpuBufferUsageFlags aUsage) {
  niLet buffer = MakeNN<sVulkanBuffer>(as_nn(this),aMemMode,aUsage);
  niCheck(buffer->Create(anSize),nullptr);
  return buffer;
}

Ptr<iGpuBuffer> sVulkanDriver::CreateGpuBufferFromData(iHString* ahspName, iFile* apFile, tU32 anSize, eGpuBufferMemoryMode aMemMode, tGpuBufferUsageFlags aUsage) {
  niCheckIsOK(apFile,nullptr);
  astl::vector<tU8> data;
  data.resize(anSize);
  if (apFile->ReadRaw(data.data(),anSize) != anSize) {
    return nullptr;
  }
  return this->CreateGpuBufferFromDataRaw(ahspName,data.data(),anSize,aMemMode,aUsage);
}

Ptr<iGpuBuffer> sVulkanDriver::CreateGpuBufferFromDataRaw(iHString* ahspName, tPtr apData, tU32 anSize, eGpuBufferMemoryMode aMemMode, tGpuBufferUsageFlags aUsage) {
  niCheck(apData != nullptr, nullptr);
  niLet buffer = MakeNN<sVulkanBuffer>(as_nn(this),aMemMode,aUsage);
  niCheck(buffer->Create(anSize),nullptr);
  {
    niLet data = buffer->Lock(0,anSize,eLock_Discard);
    niCheck(data != nullptr,nullptr);
    memcpy(data,apData,anSize);
    buffer->Unlock();
  }
  return buffer;
}

iHString* sVulkanDriver::GetGpuFunctionTarget() const {
  return _GetVulkanGpuFunctionTarget();
}

Ptr<iGpuFunction> sVulkanDriver::CreateGpuFunction(eGpuFunctionType aType, iHString* ahspPath) {
  niLet newId = _idGenerator.AllocID();
  NN<sVulkanFunction> func = MakeNN<sVulkanFunction>(as_nn(this),aType,newId);
  if (!func->_Compile(_device,ahspPath)) {
    _idGenerator.FreeID(newId);
    niError(niFmt(
      "Can't create gpu function '%s': Compilation failed.",
      ahspPath));
    return nullptr;
  }
  return func;
}

Ptr<iGpuPipelineDesc> sVulkanDriver::CreateGpuPipelineDesc() {
  return ni::_CreateGpuPipelineDesc();
}

Ptr<iGpuBlendMode> sVulkanDriver::CreateGpuBlendMode() {
  return ni::_CreateGpuBlendMode();
}

Ptr<iGpuPipeline> sVulkanDriver::CreateGpuPipeline(iHString* ahspName, const iGpuPipelineDesc* apDesc) {
  niCheckIsOK(apDesc,nullptr);
  return CreateVulkanGpuPipeline(as_nn(this),ahspName,apDesc);
}

tBool sVulkanDriver::BlitManagedGpuBufferToSystemMemory(iGpuBuffer* apBuffer) {
  niPanicUnreachable("Unimplemented");
  return eFalse;
}

niExportFunc(iUnknown*) New_GraphicsDriver_Vulkan(const Var& avarA, const Var& avarB) {
  QPtr<iGraphics> ptrGraphics = avarA;
  niCheckIsOK(ptrGraphics,nullptr);

  cString appName = ni::GetLang()->GetProperty("ni.app.name");
  if (appName.empty()) {
    appName = "niApp";
  }

  Ptr<sVulkanDriver> driver = niNew sVulkanDriver(ptrGraphics.non_null());
  niCheck(driver->_CreateVulkanDriver(appName.c_str()), nullptr);

  return static_cast<iGraphicsDriver*>(driver.GetRawAndSetNull());
}

} // end of namespace ni

#endif // GDRV_VULKAN
