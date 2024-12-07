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
#endif

#include "GDRV_Gpu.h"
#include "GDRV_Utils.h"
#include "../../../data/test/gpufunc/TestGpuFuncs.hpp"

namespace ni {

_HDecl(__vktex_white__);
_HDecl(__vkbuff_dummy__);

static const tU32 knVulkanMaxFramesInFlight = 1;
static tF32 kfVulkanSamplerFilterAnisotropy = 8.0f;
static const char* const _vkRequiredDeviceExtensions[] = {
  VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
  VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
  VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
};
static const tU32 knVkRequiredDeviceExtensionsCount = niCountOf(_vkRequiredDeviceExtensions);

#define VULKAN_TRACE(aFmt) //niDebugFmt(aFmt)

#define NISH_VULKAN_TARGET spv_vk12

_HDecl(NISH_VULKAN_TARGET);
static __forceinline iHString* _GetVulkanGpuFunctionTarget() {
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
struct sVulkanTexture;
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
  Ptr<sVulkanBuffer> _dummyBuffer;

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

    _dummyBuffer = nullptr;
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

    astl::vector<const char*> extensions = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
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

    eGpuPixelFormat gpufmt;
    if (niFlagIs(aFlags,eTextureFlags_DepthStencil)) {
      gpufmt = _GetClosestGpuPixelFormatForDS(apFormat->GetPixelFormat()->GetFormat());
    }
    else if (niFlagIs(aFlags,eTextureFlags_RenderTarget)) {
      gpufmt = _GetClosestGpuPixelFormatForRT(apFormat->GetPixelFormat()->GetFormat());
    }
    else {
      gpufmt = _GetClosestGpuPixelFormatForTexture(apFormat->GetPixelFormat()->GetFormat());
    }

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
  virtual iGraphicsContextRT* __stdcall CreateContextForRenderTargets(iTexture* apRT0, iTexture* apRT1, iTexture* apRT2, iTexture* apRT3, iTexture* apDS) niImpl {
    return nullptr;
  }
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

struct sVulkanFramebuffer {
  VkImage _image = VK_NULL_HANDLE;
  VkImageView _view = VK_NULL_HANDLE;
  tU32 _width = 0;
  tU32 _height = 0;

#if defined niOSX || defined niIOS
  tBool CreateFromMetalAPI(iOSXMetalAPI* apMetalAPI, VkDevice aDevice) {
    niCheck(osxVkCreateImageForMetalAPI(apMetalAPI,aDevice,nullptr,&_image),eFalse);
    niLet viewSize = apMetalAPI->GetViewSize();
    _width = viewSize.x;
    _height = viewSize.y;

    VkImageViewCreateInfo viewInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = _image,
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

    VK_CHECK(vkCreateImageView(aDevice, &viewInfo, nullptr, &_view), eFalse);
    return eTrue;
  }
#endif

  void Destroy(VkDevice device) {
    if (_view) {
      vkDestroyImageView(device, _view, nullptr);
      _view = VK_NULL_HANDLE;
    }
    if (_image) {
      vkDestroyImage(device, _image, nullptr);
      _image = VK_NULL_HANDLE;
    }
  }
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
  tBool _tracked = eFalse;

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
    _tracked = eFalse;
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
      if (_tracked) {
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

struct sVulkanFunction : public ImplRC<iGpuFunction,eImplFlags_DontInherit1,iDeviceResource> {
  nn<sVulkanDriver> _driver;
  NN<iDataTable> _datatable = niDeferredInit(NN<iDataTable>);
  const eGpuFunctionType _functionType;
  const tU32 _id;
  tHStringPtr _hspName;
  VkShaderModule _vkShaderModule = VK_NULL_HANDLE;

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
    _datatable = niCheckNN(_datatable,GpuFunctionDT_Load(niHStr(ahspPath),_GetVulkanGpuFunctionTarget()),eFalse);
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
    // TODO: Actually detect the bind type correctly.
    return eGpuFunctionBindType_Fixed;
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
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR,
        .bindingCount = 1,
        .pBindings = &bufferBinding
      };
      VK_CHECK(vkCreateDescriptorSetLayout(vkDevice, &bufferLayoutInfo, nullptr, &_vkDescSetLayouts[eGLSLVulkanDescriptorSet_Buffer]), eFalse);
    }

    // Texture layout
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
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR,
        .bindingCount = 1,
        .pBindings = &textureBinding
      };
      VK_CHECK(vkCreateDescriptorSetLayout(vkDevice, &textureLayoutInfo, nullptr, &_vkDescSetLayouts[eGLSLVulkanDescriptorSet_Texture2D]), eFalse);
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
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR,
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
    // TODO: Apply from rasterizer states
    VkPipelineRasterizationStateCreateInfo rasterizer = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_NONE,
      .frontFace = VK_FRONT_FACE_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
      .lineWidth = 1.0f
    };

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE,
    };

    // Color blend attachment
    // TODO: Apply from rasterizer states
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
      .blendEnable = VK_FALSE,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
      VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT |
      VK_COLOR_COMPONENT_A_BIT
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

    // TODO: Setup from RT0/DS
    VkFormat swapChainFormat = VK_FORMAT_B8G8R8A8_UNORM;
    VkPipelineRenderingCreateInfo renderingInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &swapChainFormat
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
      .pDepthStencilState = nullptr,
      .pColorBlendState = &colorBlending,
      .layout = _vkPipelineLayout,
      .renderPass = VK_NULL_HANDLE,
      .subpass = 0,
      .pNext = &renderingInfo
    };

    // TODO: Apply iRasterizerStates / sRasterizerStatesDesc
    // TODO: Apply iDepthStencilStates / sDepthStencilStatesDesc
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

struct sVulkanEncoderFrameData : public ImplRC<iUnknown> {
  ThreadEvent _event = ThreadEvent(eFalse);
  astl::vector<Ptr<sVulkanBuffer>> _trackedBuffers;
  Ptr<iGpuStream> _stream;
  tU32 _frameNumber = 0;
  tU32 _frameInFlight = 0;

  sVulkanEncoderFrameData(ain<nn<sVulkanDriver>> aDriver) {
    _stream = CreateGpuStream(
      aDriver,
      eGpuBufferUsageFlags_Vertex|
      eGpuBufferUsageFlags_Index|
      eGpuBufferUsageFlags_Uniform);
  }

  void StartFrame(const tU32 anFrameNumber, const tU32 anCurrentFrame) {
    niPanicAssert(!IsInUse());
    _frameNumber = anFrameNumber;
    _frameInFlight = anCurrentFrame;
  }

  tBool IsInUse() {
    return _frameNumber != 0;
  }

  void _TrackBuffer(iGpuBuffer* apBuffer) {
    sVulkanBuffer* buffer = (sVulkanBuffer*)apBuffer;
    if (!buffer->_tracked && buffer->_modifiedSize) {
      _trackedBuffers.push_back(buffer);
      buffer->_tracked = eTrue;
    }
  }

  void _WaitCompletion() {
    _event.InfiniteWait();
  }

  void OnFrameCompleted() {
    niLet& lastBlock = _stream->GetLastBlock();
    // niDebugFmt((
    //   "... OnFrameCompleted: sVulkanEncoderFrameData{_frameNumber=%d,_frameInFlight=%d,_trackedBuffers=%d,_stream._numBlocks=%d,_stream.mOffset=%d,_stream.mSize=%d}",
    //   _frameNumber,_frameInFlight,
    //   _trackedBuffers.size(),_stream->GetNumBlocks(),
    //   lastBlock.mOffset,lastBlock.mSize));
    niLoop(i,_trackedBuffers.size()) {
      _trackedBuffers[i]->_Untrack();
    }
    _trackedBuffers.clear();
    _stream->Reset();
    _event.Signal();
    _frameNumber = 0;
  }
};

struct sVulkanCommandEncoder : public ImplRC<iGpuCommandEncoder> {
  nn<sVulkanDriver> _driver;
  VkCommandBuffer _cmdBuffer = VK_NULL_HANDLE;
  struct sCache {
    sMaterialDesc _lastMaterial;
    Ptr<sVulkanPipeline> _lastPipeline = nullptr;
    Ptr<sVulkanBuffer> _lastBuffer = nullptr;
    tFixedGpuPipelineId _lastFixedPipeline = 0;
  } _cache;
  VkFence _encoderInFlightFence = VK_NULL_HANDLE;
  tBool _beganFrame = eFalse;
  astl::vector<NN<sVulkanEncoderFrameData>> _frames;
  tU32 _currentFrame = 0;
  tU32 _currentWidth = 1, _currentHeight = 1;

  sVulkanCommandEncoder(ain<nn<sVulkanDriver>> aDriver, ain<tU32> aFrameMaxInFlight)
      : _driver(aDriver)
  {
    _frames.reserve(aFrameMaxInFlight);
    niLoop(i,aFrameMaxInFlight) {
      _frames.emplace_back(niNew sVulkanEncoderFrameData(aDriver));
    }
  }

  ~sVulkanCommandEncoder() {
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

  tBool _BeginFrame(
    const sVulkanFramebuffer& aFB,
    ain<sRecti> aViewport,
    ain<sRecti> aScissor,
    ain<sVec4f> aClearColor = Vec4f(1,0,1,0),
    ain<tF32> aClearDepth = 1.0f,
    ain<tU32> aClearStencil = 0)
  {
    niCheck(_beganFrame == eFalse, eFalse);
    _beganFrame = eTrue;
    _cache = sCache {};
    _currentWidth = aFB._width;
    _currentHeight = aFB._height;

    vkResetCommandBuffer(_cmdBuffer, 0);

    VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    VK_CHECK(vkBeginCommandBuffer(_cmdBuffer, &beginInfo), eFalse);

    VkClearValue clearColorValue = {};
    static_assert(sizeof(clearColorValue) == sizeof(sVec4f));
    ((sVec4f&)clearColorValue.color) = aClearColor;

    VkRenderingAttachmentInfo colorAttachment{
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = aFB._view,
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue = clearColorValue
    };

    VkRenderingInfo renderingInfo{
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea = {{0, 0}, {aFB._width, aFB._height}},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachment
    };

    vkCmdBeginRendering(_cmdBuffer, &renderingInfo);
    this->SetViewport(aViewport);
    this->SetScissorRect(aScissor);
    return eTrue;
  }

  tBool _EndFrame() {
    niCheck(_beganFrame == eTrue, eFalse);
    niDefer {
      _beganFrame = eFalse;
    };

    vkCmdEndRendering(_cmdBuffer);

    VK_CHECK(vkEndCommandBuffer(_cmdBuffer), eFalse);

    VkSubmitInfo submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = 0,
      .signalSemaphoreCount = 0,
      .commandBufferCount = 1,
      .pCommandBuffers = &_cmdBuffer
    };

    // TODO: The usage of the fence here is suboptimal since we just wait for
    // the queue to be finished before continuing which waiting tons of CPU
    // cycles. But we want to get all the other bits right first.
    VK_CHECK(vkResetFences(_driver->_device, 1, &_encoderInFlightFence), eFalse);
    VK_CHECK(vkQueueSubmit(_driver->_graphicsQueue, 1, &submitInfo, _encoderInFlightFence), eFalse);
    VK_CHECK(vkWaitForFences(_driver->_device, 1, &_encoderInFlightFence, VK_TRUE, UINT64_MAX), eFalse);
    return eTrue;
  }

  void _DoBindPipeline(iGpuPipeline* apPipeline, tFixedGpuPipelineId aFixedPipelineId) {
    sVulkanPipeline* pipeline = (sVulkanPipeline*)apPipeline;
    vkCmdBindPipeline(_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->_vkPipeline);
    _cache._lastPipeline = (sVulkanPipeline*)apPipeline;
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
    sVulkanBuffer* buffer = (sVulkanBuffer*)apBuffer;
    VkBuffer vertexBuffers[] = {buffer->_vkBuffer};
    VkDeviceSize offsets[] = {anOffset};
    vkCmdBindVertexBuffers(_cmdBuffer, anBinding, 1, vertexBuffers, offsets);
  }

  virtual void __stdcall SetIndexBuffer(iGpuBuffer* apBuffer, tU32 anOffset, eGpuIndexType aIndexType) niImpl {
    niCheck(apBuffer != nullptr, ;);
    sVulkanBuffer* indexBuffer = (sVulkanBuffer*)apBuffer;
    vkCmdBindIndexBuffer(_cmdBuffer, indexBuffer->_vkBuffer, anOffset, _ToVkIndexType[aIndexType]);
  }

  virtual void __stdcall SetUniformBuffer(iGpuBuffer* apBuffer, tU32 anOffset, tU32 anBinding) niImpl {
    niCheck(anBinding == 0, ;);
    _cache._lastBuffer = (sVulkanBuffer*)apBuffer;
  }

  virtual void __stdcall SetTexture(iTexture* apTexture, tU32 anBinding) niImpl {
    niCheck(anBinding < eMaterialChannel_Last, ;);
    _cache._lastMaterial.mChannels[anBinding].mTexture = apTexture;
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
    vkCmdSetPrimitiveTopology(_cmdBuffer, _ToVkPrimitiveTopology[aPrimType]);
    vkCmdDrawIndexed(_cmdBuffer, anNumIndices, 1, anFirstIndex, 0, 0);
    return eTrue;
  }

  virtual tBool __stdcall Draw(eGraphicsPrimitiveType aPrimType, tU32 anVertexCount, tU32 anFirstVertex) niImpl {
    niCheck(aPrimType <= eGraphicsPrimitiveType_Last,eFalse);
    niCheck(_BindGpuFunction(),eFalse);
    vkCmdSetPrimitiveTopology(_cmdBuffer, _ToVkPrimitiveTopology[aPrimType]);
    vkCmdDraw(_cmdBuffer, anVertexCount, 1, anFirstVertex, 0);
    return eTrue;
  }
};

static Ptr<sVulkanCommandEncoder> _CreateVulkanCommandEncoder(ain_nn<sVulkanDriver> aDriver) {
  NN<sVulkanCommandEncoder> cmdEncoder = MakeNN<sVulkanCommandEncoder>(aDriver,knVulkanMaxFramesInFlight);
  niCheck(cmdEncoder->_CreateCommandBuffer(), nullptr);
  return cmdEncoder;
}

struct sVulkanTexture : public ImplRC<iTexture> {
  nn<sVulkanDriver> _driver;
  VkImage _vkImage = VK_NULL_HANDLE;
  VmaAllocation _vmaAllocation;
  VkImageView _vkView = VK_NULL_HANDLE;
  tU32 _width = 0, _height = 0;
  const tHStringPtr _name;
  const tTextureFlags _flags = eTextureFlags_Default;
  eGpuPixelFormat _pixelFormat = eGpuPixelFormat_None;

  sVulkanTexture(
    ain_nn<sVulkanDriver> aDriver, iHString* ahspName,
    tU32 anWidth, tU32 anHeight, eGpuPixelFormat aGpuPixelFormat,
    tTextureFlags aFlags)
      : _driver(aDriver)
      , _name(ahspName)
      , _width(anWidth)
      , _height(anHeight)
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
    if (_driver->_graphics->GetTextureDeviceResourceManager()) {
      _driver->_graphics->GetTextureDeviceResourceManager()->Unregister(this);
    }
    if (_vkView) {
      vkDestroyImageView(_driver->_device, _vkView, nullptr);
      _vkView = VK_NULL_HANDLE;
    }
    if (_vkImage) {
      vmaDestroyImage(_driver->_allocator, _vkImage, _vmaAllocation);
      _vkImage = VK_NULL_HANDLE;
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
    return eBitmapType_2D;
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
    return 1;
  }

  virtual tTextureFlags __stdcall GetFlags() const override {
    return _flags;
  }

  virtual iTexture* __stdcall GetSubTexture(tU32 anIndex) const override {
    return nullptr;
  }

  tBool _CreateVulkanTexture() {
    niPanicAssert(_vkImage == VK_NULL_HANDLE && _vkView == VK_NULL_HANDLE);

    VkImageCreateInfo imageInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = _GetVulkanPixelFormat(_pixelFormat),
      .extent = {_width, _height, 1},
      .mipLevels = 1,
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

    VmaAllocationCreateInfo allocInfo = {
      .usage = VMA_MEMORY_USAGE_GPU_ONLY
    };

    VK_CHECK(vmaCreateImage(
      _driver->_allocator, &imageInfo, &allocInfo,
      &_vkImage, &_vmaAllocation, nullptr), eFalse);

    // Create image view
    VkImageViewCreateInfo viewInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = _vkImage,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
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
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
      }
    };

    VK_CHECK(vkCreateImageView(_driver->_device, &viewInfo, nullptr, &_vkView), eFalse);
    return eTrue;
  }

  void UploadTexture(const iBitmap2D* apBmpLevel,
                     const tU32 anLevel,
                     const sRecti& aDestRect)
  {
    if (anLevel != 0) // TODO: Implement mipmaps
      return;

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
      &stagingBuffer, &stagingAlloc, nullptr), ;);
    niDefer {
      vmaDestroyBuffer(_driver->_allocator, stagingBuffer, stagingAlloc);
    };

    // Copy data to staging
    void* data;
    VK_CHECK(vmaMapMemory(_driver->_allocator, stagingAlloc, &data), ;);
    for (tU32 y = 0; y < aDestRect.GetHeight(); ++y) {
      memcpy(
        (tU8*)data + (y * aDestRect.GetWidth() * bpp),
        (tU8*)bytes + (y * bpr),
        aDestRect.GetWidth() * bpp);
    }
    vmaUnmapMemory(_driver->_allocator, stagingAlloc);

    // Transition image layout for copy
    VkCommandBuffer cmdBuf = _driver->BeginSingleTimeCommands();

    VkImageMemoryBarrier barrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = _vkImage,
      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = anLevel,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
      },
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT
    };

    vkCmdPipelineBarrier(
      cmdBuf,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_TRANSFER_BIT,
      0,
      0, nullptr,
      0, nullptr,
      1, &barrier);

    // Copy buffer to image
    VkBufferImageCopy region = {
      .bufferOffset = 0,
      .bufferRowLength = 0,
      .bufferImageHeight = 0,
      .imageSubresource = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevel = anLevel,
        .baseArrayLayer = 0,
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
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
      cmdBuf,
      VK_PIPELINE_STAGE_TRANSFER_BIT,
      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
      0,
      0, nullptr,
      0, nullptr,
      1, &barrier);

    _driver->EndSingleTimeCommands(cmdBuf);
  }
};

iTexture* __stdcall sVulkanDriver::CreateTexture(iHString* ahspName, eBitmapType aType, const achar* aaszFormat, tU32 anNumMipMaps, tU32 anWidth, tU32 anHeight, tU32 anDepth, tTextureFlags aFlags) {
  VULKAN_TRACE(("sVulkanDriver::CreateTexture: %s, %s, %dx%dx%d, mips:%d, flags:%d",
                ahspName,aaszFormat,anWidth,anHeight,anDepth,anNumMipMaps,aFlags));
  Ptr<iPixelFormat> pxf = _graphics->CreatePixelFormat(aaszFormat);
  niCheck(pxf.IsOK(),nullptr);

  Ptr<sVulkanTexture> tex { niNew sVulkanTexture(
    as_nn(this),
    ahspName,
    anWidth,anHeight,
    _GetClosestGpuPixelFormatForTexture(pxf->GetFormat()),
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
    tex->UploadTexture(srcBmp, anDestLevel, aDestRect);
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

  _dummyBuffer = niNew sVulkanBuffer(as_nn(this), eGpuBufferMemoryMode_Shared, eGpuBufferUsageFlags_Vertex);
  niCheck(_dummyBuffer->Create(1024),eFalse);
  {
    tPtr data = _dummyBuffer->Lock(0,1024,eLock_Discard);
    ni::MemZero(data,1024);
    _dummyBuffer->Unlock();
  }

  return eTrue;
}

tBool sVulkanCommandEncoder::_DoBindFixedDescLayout() {
  niLet& mat = _cache._lastMaterial;
  niLet pipeline = _cache._lastPipeline;

  {
    sVulkanBuffer* buffer = _cache._lastBuffer.raw_ptr();
    if (!buffer) {
      buffer = _driver->_dummyBuffer.raw_ptr();
    }
    VkDescriptorBufferInfo bufferInfo = {
      .buffer = buffer->_vkBuffer,
      .offset = 0,
      .range = VK_WHOLE_SIZE
    };
    astl::vector<VkWriteDescriptorSet> writes;
    writes.emplace_back(VkWriteDescriptorSet {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = VK_NULL_HANDLE, // Push descriptor
        .dstBinding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .pBufferInfo = &bufferInfo
      });
    vkCmdPushDescriptorSetKHR(
      _cmdBuffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      pipeline->_vkPipelineLayout,
      eGLSLVulkanDescriptorSet_Buffer,
      writes.size(),writes.data());
  }

  {
    sVulkanTexture* texture = (sVulkanTexture*)_cache._lastMaterial.mChannels[0].mTexture.raw_ptr();
    if (!texture) {
      texture = (sVulkanTexture*)_driver->_fixedPipelines->GetWhiteTexture().raw_ptr();
    }
    VkDescriptorImageInfo imageInfo = {
      .imageView = texture->_vkView,
      .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };
    astl::vector<VkWriteDescriptorSet> writes;
    writes.emplace_back(VkWriteDescriptorSet {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = VK_NULL_HANDLE, // Push descriptor
        .dstBinding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .descriptorCount = 1,
        .pImageInfo = &imageInfo
      });
    vkCmdPushDescriptorSetKHR(
      _cmdBuffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      pipeline->_vkPipelineLayout,
      eGLSLVulkanDescriptorSet_Texture2D,
      writes.size(),writes.data());
  }

  {
    tIntPtr hSS = _cache._lastMaterial.mChannels[0].mhSS;
    VkDescriptorImageInfo samplerInfo = {
      .sampler = _driver->_GetVkSamplerState(hSS)
    };
    astl::vector<VkWriteDescriptorSet> writes;
    writes.emplace_back(VkWriteDescriptorSet {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = VK_NULL_HANDLE, // Push descriptor
        .dstBinding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
        .descriptorCount = 1,
        .pImageInfo = &samplerInfo
      });
    vkCmdPushDescriptorSetKHR(
      _cmdBuffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      pipeline->_vkPipelineLayout,
      eGLSLVulkanDescriptorSet_Sampler,
      writes.size(),writes.data());
  }

  return eTrue;
}

tBool sVulkanCommandEncoder::_BindGpuFunction() {
  niLet pipeline = _cache._lastPipeline;
  if (pipeline->_gpufuncBindType == eGpuFunctionBindType_Fixed) {
    return _DoBindFixedDescLayout();
  }
  return eTrue;
}

struct sVulkanContextBase :
    public sGraphicsContext<1,ni::ImplRC<
                             iGraphicsContextRT,
                             eImplFlags_DontInherit1,
                             iGraphicsContext,
                             iGraphicsContextGpu> >
{
  nn<sVulkanDriver> _driver;
  NN<sVulkanCommandEncoder> _cmdEncoder = niDeferredInit(NN<sVulkanCommandEncoder>);
  eGpuPixelFormat _rt0Format;
  eGpuPixelFormat _dsFormat;
  tBool _beganFrame = eFalse;
  tU32 _currentFrame = 0;
  const tU32 _frameMaxInFlight;
  sVulkanFramebuffer _currentFb;

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

  tBool __stdcall _CreateContextBase() {
    return eTrue;
  }

  virtual tBool __stdcall IsOK() const {
    return mptrRT[0].IsOK();
  }

  virtual void __stdcall Invalidate() {
  }

  virtual iGraphics* __stdcall GetGraphics() const {
    return _driver->_graphics;
  }
  virtual iGraphicsDriver* __stdcall GetDriver() const {
    return _driver;
  }

  virtual iGpuCommandEncoder* __stdcall GetCommandEncoder() niImpl {
    if (!_beganFrame) {
      _BeginFrame();
    }
    return _cmdEncoder;
  }

  virtual tBool _UpdateFrameBuffer() = 0;

  virtual tBool _BeginFrame() {
    niCheck(_UpdateFrameBuffer(),eFalse);

    niPanicAssert(_beganFrame == eFalse);
    _beganFrame = eTrue;

    _rt0Format = _GetClosestGpuPixelFormatForRT(mptrRT[0]->GetPixelFormat()->GetFormat());
    if (mptrDS.IsOK()) {
      _dsFormat = _GetClosestGpuPixelFormatForDS(mptrDS->GetPixelFormat()->GetFormat());
    }
    else {
      _dsFormat = eGpuPixelFormat_None;
    }

    niCheck(_cmdEncoder->_BeginFrame(_currentFb,mrectViewport,mrectScissor),eFalse);
    return eTrue;
  }

  void _EndFrame() {
    if (!_beganFrame)
      return;
    _cmdEncoder->_EndFrame();
    _beganFrame = eFalse;
  }

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

    // niAssert(mbBeganFrame);
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

  virtual tBool __stdcall Display(tGraphicsDisplayFlags aFlags, const sRecti& aRect) {
    _EndFrame();
    _currentFrame = (_currentFrame + 1) % _frameMaxInFlight;
    return eTrue;
  }

  /////////////////////////////////////////////
  virtual iBitmap2D* __stdcall CaptureFrontBuffer() const {
    return NULL;
  }
};

struct sVulkanContextWindow : public sVulkanContextBase {
  Ptr<iOSWindow> _window;

#if defined niOSX
  Ptr<iOSXMetalAPI> _metalAPI;
  tBool _PlatformInit() {
    osxMetalSetDefaultDevice();
    _metalAPI = osxMetalCreateAPIForWindow(osxMetalGetDevice(),_window);
    if (!_metalAPI.IsOK()) {
      niError("Can't get metal api for iOSWindow.");
      return eFalse;
    }
    return eTrue;
  }
  void _PlatformInvalidate() {
    _metalAPI = nullptr;
  }
  tBool _PlatformPresent() {
    _metalAPI->DrawablePresent();
    return eTrue;
  }
  tBool _PlatformUpdateFrameBuffer() {
    return _currentFb.CreateFromMetalAPI(_metalAPI, _driver->_device);
  }
//#elif defined niLinux
#else
  //#error "Unsupported Vulkan platform."
  tBool _PlatformInit() { return eFalse; }
  void _PlatformInvalidate() {}
  tBool _PlatformPresent() { return eFalse; }
  tBool _PlatformUpdateFrameBuffer() { return eFalse; }
#endif

  sVulkanContextWindow(
    ain<nn<sVulkanDriver>> aDriver,
    const tU32 aFrameMaxInFlight,
    iOSWindow* apWindow)
      : sVulkanContextBase(aDriver,aFrameMaxInFlight)
  {
    _window = apWindow;

    if (!_PlatformInit()) {
      niError("Platform initialization failed.");
      this->Invalidate();
      return;
    }

    if (!_DoResizeContext()) {
      niError("Can't do the initial context initialization.");
      this->Invalidate();
      return;
    }
  }

  virtual ~sVulkanContextWindow() {
    this->Invalidate();
  }

  void __stdcall Invalidate() niImpl {
    _currentFb.Destroy(_driver->_device);
    _PlatformInvalidate();
    _window = nullptr;
  }

  virtual tBool __stdcall Display(tGraphicsDisplayFlags aFlags, const sRecti& aRect) niImpl {
    if (!_window.IsOK()) {
      return eFalse;
    }
    niCheck(sVulkanContextBase::Display(aFlags,aRect),eFalse);
    niCheck(_PlatformPresent(),eFalse);
    return eTrue;
  }

  tBool __stdcall _DoResizeContext() {
    iGraphics* g = _driver->_graphics;
    const tU32 w = _window->GetClientSize().x;
    const tU32 h = _window->GetClientSize().y;

    ni::SafeInvalidate(mptrRT[0].ptr());
    mptrRT[0] = niNew sVulkanTexture(
      _driver,_H("Vulkan_MainRT"),
      w,h,eGpuPixelFormat_RGBA8,
      eTextureFlags_RenderTarget|eTextureFlags_Surface);

    ni::SafeInvalidate(mptrDS.ptr());
    mptrDS = niNew sVulkanTexture(
      _driver,_H("Vulkan_MainDS"),
      w,h,eGpuPixelFormat_D32,
      eTextureFlags_RenderTarget|eTextureFlags_Surface);

    SetViewport(sRecti(0,0,w,h));
    SetScissorRect(sRecti(0,0,w,h));

    niLog(Info, niFmt(
      "Vulkan Context - Resized [%p]: %dx%d, BB: %s, DS: %s, VP: %s, SC: %s",
      (tIntPtr)this,
      w,h,
      mptrRT[0]->GetPixelFormat()->GetFormat(),
      mptrDS->GetPixelFormat()->GetFormat(),
      GetViewport(),
      GetScissorRect()));
    return eTrue;
  }

  virtual tBool __stdcall _UpdateFrameBuffer() niImpl {
    if (!_window.IsOK()) {
      return eFalse;
    }
    _currentFb.Destroy(_driver->_device);
    const sVec2i newSize = _window->GetClientSize();
    if ((newSize.x != this->GetWidth()) || (newSize.y != this->GetHeight())) {
      _DoResizeContext();
    }
    niCheck(_PlatformUpdateFrameBuffer(), eFalse);
    return eTrue;
  }
};

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
  return niNew sVulkanContextWindow(
    as_nn(this),knVulkanMaxFramesInFlight,apWindow);
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
