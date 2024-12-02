#include "stdafx.h"

#if defined niOSX

#include "../src/API/niLang/Math/MathMatrix.h"
#include "../src/API/niLang_ModuleDef.h"
#include "../../niUI/src/API/niUI/GraphicsEnum.h"
#include "../../niUI/src/API/niUI/FVF.h"
#include <niLang/STL/set.h>

#include <vulkan/vulkan.h>
#include "../../niUI/src/API/niUI/IVertexArray.h"
#include "../../niUI/src/API/niUI/IIndexArray.h"
#define niVulkanMemoryAllocator_Implement
#include "../../thirdparty/VulkanUtils/niVulkanMemoryAllocator.h"
#include "../../thirdparty/VulkanUtils/niVulkanOSXMetal.h"
#include "../../thirdparty/VulkanUtils/niVulkanEnumToString.h"
#include "../../niUI/tsrc/data/A.jpg.hxx"
#include "../../niUI/src/API/niUI/IGraphics.h"
#include "../../../data/test/gpufunc/triangle_vs_gpufunc_spv_vk12.h"
#include "../../../data/test/gpufunc/triangle_ps_gpufunc_spv_vk12.h"
#include "../../../data/test/gpufunc/texture_vs_gpufunc_spv_vk12.h"
#include "../../../data/test/gpufunc/texture_ps_gpufunc_spv_vk12.h"
#include "../../niUI/src/API/niUI/IGpu.h"
#include <niLang/Utils/IDGenerator.h>

#include "../../niUI/src/GDRV_Gpu.h"

// #define TRACE_MOUSE_MOVE

namespace ni {

_HDecl(__vktex_white__);
_HDecl(__vkbuff_dummy__);

struct FOSWindowVulkan {
};

static const tU32 knMaxFramesInFlight = 2;

#define VULKAN_TRACE(aFmt) niDebugFmt(aFmt)

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

static tF32 kfVulkanSamplerFilterAnisotropy = 8.0f;

void _toVkSamplerFilter(VkSamplerCreateInfo& desc, eSamplerFilter aFilter) {
  switch(aFilter) {
    case eSamplerFilter_Smooth: {
      desc.minFilter = VK_FILTER_LINEAR;
      desc.magFilter = VK_FILTER_LINEAR;
      desc.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
      desc.maxAnisotropy = kfVulkanSamplerFilterAnisotropy;
      desc.anisotropyEnable = VK_TRUE;
      break;
    }
    case eSamplerFilter_Point: {
      desc.minFilter = VK_FILTER_NEAREST;
      desc.magFilter = VK_FILTER_NEAREST;
      desc.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
      desc.maxAnisotropy = kfVulkanSamplerFilterAnisotropy;
      desc.anisotropyEnable = VK_TRUE;
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
  tU32 location = 0;
  tU32 offset = 0;

  if (eFVF_HasPosition(aFVF)) {
    attrs.push_back({
        .location = location++,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offset
      });
    offset += sizeof(sVec3f);

    if (eFVF_HasWeights(aFVF)) {
      const tU32 numWeights = eFVF_NumWeights(aFVF);
      switch (numWeights) {
        case 1:
          attrs.push_back({
              .location = location++,
              .binding = 0,
              .format = VK_FORMAT_R32_SFLOAT,
              .offset = offset
            });
          offset += sizeof(tF32);
          break;
        case 2:
          attrs.push_back({
              .location = location++,
              .binding = 0,
              .format = VK_FORMAT_R32G32_SFLOAT,
              .offset = offset
            });
          offset += sizeof(tF32) * 2;
          break;
        case 3:
          attrs.push_back({
              .location = location++,
              .binding = 0,
              .format = VK_FORMAT_R32G32B32_SFLOAT,
              .offset = offset
            });
          offset += sizeof(tF32) * 3;
          break;
        case 4:
          attrs.push_back({
              .location = location++,
              .binding = 0,
              .format = VK_FORMAT_R32G32B32A32_SFLOAT,
              .offset = offset
            });
          offset += sizeof(tF32) * 4;
          break;
        default:
          niPanicUnreachable(niFmt("Unexpected number of vertex weights '%d'.",numWeights));
          break;
      }
    }
  }

  if (niFlagIs(aFVF,eFVF_Indices)) {
    attrs.push_back({
        .location = location++,
        .binding = 0,
        .format = VK_FORMAT_B8G8R8A8_UNORM,
        .offset = offset
      });
    offset += sizeof(tU32);
  }

  if (niFlagIs(aFVF,eFVF_Normal)) {
    attrs.push_back({
        .location = location++,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offset
      });
    offset += sizeof(sVec3f);
  }

  if (niFlagIs(aFVF,eFVF_ColorA)) {
    attrs.push_back({
        .location = location++,
        .binding = 0,
        .format = VK_FORMAT_B8G8R8A8_UNORM,
        .offset = offset
      });
    offset += sizeof(tU32);
  }

  for (tU32 i = 0; i < eFVF_TexNumCoo(aFVF); ++i) {
    attrs.push_back({
        .location = location++,
        .binding = 0,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = offset
      });
    offset += sizeof(sVec2f);
  }

  return attrs;
}

struct sVulkanTexture;
struct sVulkanBuffer;

struct sVulkanDriver : public ImplRC<iUnknown> {
  NN<iGraphics> _graphics;
  VkDevice _device = VK_NULL_HANDLE;
  VmaAllocator _allocator = nullptr;
  VkQueue _graphicsQueue = VK_NULL_HANDLE;
  VkCommandPool _commandPool = VK_NULL_HANDLE;

  VkInstance _instance = VK_NULL_HANDLE;
  VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE; // This object will be implicitly destroyed when the VkInstance is destroyed.
  typedef astl::map<cString,tU32> tVkExtensionsMap;
  tVkExtensionsMap _extensions;
  tBool _enableValidationLayers = eTrue;
  typedef astl::set<cString> tVkInstanceLayersSet;
  tVkInstanceLayersSet _instanceLayers;
  tU32 _queueFamilyIndex = 0;

  LocalIDGenerator _idGenerator;
  VkSampler _ssCompiled[(eCompiledStates_SS_SmoothWhiteBorder-eCompiledStates_SS_PointRepeat)+1];
  Ptr<sVulkanTexture> _whiteTexture;
  Ptr<sVulkanBuffer> _dummyBuffer;

  sVulkanDriver()
      : _graphics(as_NN(QueryInterface<iGraphics>(niCreateInstance(niUI,Graphics,niVarNull,niVarNull))))
  {
  }

  virtual tBool __stdcall Init(const achar* aAppName) {
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
    _dummyBuffer = nullptr;
    _whiteTexture = nullptr;
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
    if (_device) {
      vkDestroyDevice(_device, nullptr);
      _device = VK_NULL_HANDLE;
    }
    if (_instance) {
      vkDestroyInstance(_instance, nullptr);
      _instance = VK_NULL_HANDLE;
    }

    if (_allocator != nullptr) {
      vmaDestroyAllocator(_allocator);
      _allocator = nullptr;
    }
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
      .apiVersion = VK_API_VERSION_1_0
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

    niCheck(_extensions.find("VK_KHR_push_descriptor") != _extensions.end(), eFalse);
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

    VkDeviceCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queueCreateInfo,
      .enabledExtensionCount = 0,
      .enabledLayerCount = 0
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
};

struct sVulkanFramebuffer {
  VkImage _image = VK_NULL_HANDLE;
  VkImageView _view = VK_NULL_HANDLE;
  tU32 _width = 0;
  tU32 _height = 0;

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
  tBool _locked = eFalse;
  eGpuBufferMemoryMode _memMode;
  tGpuBufferUsageFlags _usage;

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
    if (_memMode == eGpuBufferMemoryMode_Private)
      return nullptr;
    if (_locked)
      return nullptr;

    void* data;
    VK_CHECK(vmaMapMemory(_driver->_allocator, _vmaAllocation, &data), nullptr);
    _locked = eTrue;
    return ((tPtr)data) + anOffset;
  }

  virtual tBool __stdcall Unlock() niImpl {
    if (!_locked)
      return eFalse;
    vmaUnmapMemory(_driver->_allocator, _vmaAllocation);
    _locked = eFalse;
    return eTrue;
  }

  virtual tBool __stdcall GetIsLocked() const niImpl {
    return _locked;
  }
};

struct sVulkanFunction : public ImplRC<iGpuFunction,eImplFlags_DontInherit1,iDeviceResource> {
  nn<sVulkanDriver> _driver;
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

  tBool _Compile(VkDevice aDevice, iHString* ahspName, const tPtr apData, tSize aSize) {
    _hspName = ahspName;

    VkShaderModuleCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = aSize,
      .pCode = (tU32*)apData
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
    return nullptr;
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
  iHString* ahspName,
  eGpuFunctionType aType,
  const tPtr apData,
  tSize aSize)
{
  niLet newId = aDriver->_idGenerator.AllocID();
  NN<sVulkanFunction> func = MakeNN<sVulkanFunction>(aDriver,aType,newId);
  if (!func->_Compile(aDriver->_device, ahspName, apData, aSize)) {
    aDriver->_idGenerator.FreeID(newId);
    niError(niFmt(
      "Can't create gpu function '%s': Compilation failed.",
      ahspName));
    return nullptr;
  }
  if (func->GetFunctionType() != aType) {
    aDriver->_idGenerator.FreeID(newId);
    niError(niFmt(
      "Can't create gpu function '%s': Expected function type '%s' but got '%s'.",
      ahspName,
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
      VK_CHECK(vkCreateDescriptorSetLayout(vkDevice, &bufferLayoutInfo, nullptr, &_vkDescSetLayouts[eGLSLVulkanLayoutSets_Buffer]), eFalse);
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
      VK_CHECK(vkCreateDescriptorSetLayout(vkDevice, &textureLayoutInfo, nullptr, &_vkDescSetLayouts[eGLSLVulkanLayoutSets_Texture2D]), eFalse);
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
      VK_CHECK(vkCreateDescriptorSetLayout(vkDevice, &samplerLayoutInfo, nullptr, &_vkDescSetLayouts[eGLSLVulkanLayoutSets_Sampler]), eFalse);
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

    // Pipeline layout
    {
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
    }

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

static Ptr<sVulkanPipeline> _CreateVulkanPipelineTriangle(ain_nn<sVulkanDriver> aDriver) {
  NN<sVulkanFunction> vs = niCheckNN(vs,CreateVulkanGpuFunction(
    aDriver,
    _H("triangle_vs_gpufunc_spv_vk12"),
    eGpuFunctionType_Vertex,
    triangle_vs_gpufunc_spv_vk12_DATA,
    triangle_vs_gpufunc_spv_vk12_DATA_SIZE), nullptr);

  NN<sVulkanFunction> ps = niCheckNN(ps,CreateVulkanGpuFunction(
    aDriver,
    _H("triangle_ps_gpufunc_spv_vk12"),
    eGpuFunctionType_Pixel,
    triangle_ps_gpufunc_spv_vk12_DATA,
    triangle_ps_gpufunc_spv_vk12_DATA_SIZE), nullptr);

  niLetMut pipelineDesc = _CreateGpuPipelineDesc();
  niCheckIsOK(pipelineDesc, nullptr);

  pipelineDesc->SetFVF(sVertexPA::eFVF);
  pipelineDesc->SetColorFormat(0, eGpuPixelFormat_BGRA8);
  pipelineDesc->SetFunction(eGpuFunctionType_Vertex, vs);
  pipelineDesc->SetFunction(eGpuFunctionType_Pixel, ps);

  return CreateVulkanGpuPipeline(aDriver,_H("triangle_pipeline"),pipelineDesc);
}

static Ptr<sVulkanPipeline> _CreateVulkanPipelineTexture(
  ain_nn<sVulkanDriver> aDriver)
{
  NN<sVulkanFunction> vs = niCheckNN(vs,CreateVulkanGpuFunction(
    aDriver,
    _H("texture_vs_gpufunc_spv_vk12"),
    eGpuFunctionType_Vertex,
    texture_vs_gpufunc_spv_vk12_DATA,
    texture_vs_gpufunc_spv_vk12_DATA_SIZE), nullptr);

  NN<sVulkanFunction> ps = niCheckNN(ps,CreateVulkanGpuFunction(
    aDriver,
    _H("texture_ps_gpufunc_spv_vk12"),
    eGpuFunctionType_Pixel,
    texture_ps_gpufunc_spv_vk12_DATA,
    texture_ps_gpufunc_spv_vk12_DATA_SIZE), nullptr);

  niLetMut pipelineDesc = _CreateGpuPipelineDesc();
  niCheckIsOK(pipelineDesc, nullptr);

  pipelineDesc->SetFVF(sVertexPAT1::eFVF);
  pipelineDesc->SetColorFormat(0, eGpuPixelFormat_BGRA8);
  pipelineDesc->SetFunction(eGpuFunctionType_Vertex, vs);
  pipelineDesc->SetFunction(eGpuFunctionType_Pixel, ps);

  return CreateVulkanGpuPipeline(aDriver,_H("texture_pipeline"),pipelineDesc);
}

struct sVulkanCommandEncoder : public ImplRC<iGpuCommandEncoder> {
  nn<sVulkanDriver> _driver; // TODO: weakptr
  VkCommandBuffer _cmdBuffer = VK_NULL_HANDLE;
  struct sCache {
    sMaterialDesc _lastMaterial;
    Ptr<sVulkanPipeline> _lastPipeline = nullptr;
    Ptr<sVulkanBuffer> _lastBuffer = nullptr;
  } _cache;
  VkSemaphore _encoderFinishedSemaphore = VK_NULL_HANDLE;
  VkFence _encoderInFlightFence = VK_NULL_HANDLE;
  tBool _beganFrame = eFalse;

  sVulkanCommandEncoder(ain<nn<sVulkanDriver>> aDriver)
      : _driver(aDriver)
  {}

  ~sVulkanCommandEncoder() {
    if (_encoderInFlightFence) {
      vkDestroyFence(_driver->_device, _encoderInFlightFence, nullptr);
      _encoderInFlightFence = VK_NULL_HANDLE;
    }
    if (_encoderFinishedSemaphore) {
      vkDestroySemaphore(_driver->_device, _encoderFinishedSemaphore, nullptr);
      _encoderFinishedSemaphore = VK_NULL_HANDLE;
    }

    if (_cmdBuffer) {
      vkFreeCommandBuffers(_driver->_device, _driver->_commandPool, 1, &_cmdBuffer);
      _cmdBuffer = VK_NULL_HANDLE;
    }
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

    VkSemaphoreCreateInfo semaphoreInfo = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    VkFenceCreateInfo fenceInfo = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT  // Start signaled so first frame doesn't wait
    };

    VK_CHECK(vkCreateSemaphore(_driver->_device, &semaphoreInfo, nullptr, &_encoderFinishedSemaphore), eFalse);
    VK_CHECK(vkCreateFence(_driver->_device, &fenceInfo, nullptr, &_encoderInFlightFence), eFalse);

    return eTrue;
  }

  tBool _BeginFrame(
    const sVulkanFramebuffer& aFB,
    ain<sVec4f> aClearColor = sVec4f::Zero(),
    ain<tF32> aClearDepth = 1.0f,
    ain<tU32> aClearStencil = 0)
  {
    niCheck(_beganFrame == eFalse, eFalse);
    _beganFrame = eTrue;
    _cache = sCache {};

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
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &_encoderFinishedSemaphore,
      .commandBufferCount = 1,
      .pCommandBuffers = &_cmdBuffer
    };

    VK_CHECK(vkQueueSubmit(_driver->_graphicsQueue, 1, &submitInfo, _encoderInFlightFence), eFalse);

    return eTrue;
  }

  tBool WaitForInFlightFence() {
    VK_CHECK(vkWaitForFences(_driver->_device, 1, &_encoderInFlightFence, VK_TRUE, UINT64_MAX), eFalse);
    VK_CHECK(vkResetFences(_driver->_device, 1, &_encoderInFlightFence), eFalse);
    return eTrue;
  }

  virtual void __stdcall SetPipeline(iGpuPipeline* apPipeline) niImpl {
    niCheck(apPipeline != nullptr,;);
    if ((tIntPtr)_cache._lastPipeline.raw_ptr() == (tIntPtr)apPipeline)
      return;
    sVulkanPipeline* pipeline = (sVulkanPipeline*)apPipeline;
    vkCmdBindPipeline(_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->_vkPipeline);
    _cache._lastPipeline = (sVulkanPipeline*)apPipeline;
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
    niPanicUnreachable("NOT IMPLEMENTED");
    return eFalse;
  }

  virtual tBool __stdcall StreamIndexBuffer(const tPtr apData, tU32 anSize, eGpuIndexType aIndexType) niImpl {
    niPanicUnreachable("NOT IMPLEMENTED");
    return eFalse;
  }

  virtual tBool __stdcall StreamUniformBuffer(const tPtr apData, tU32 anSize, tU32 anBinding) niImpl {
    niPanicUnreachable("NOT IMPLEMENTED");
    return eFalse;
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
      .y = (float)aRect.y,
      .width = (float)aRect.GetWidth(),
      .height = (float)aRect.GetHeight(),
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
  NN<sVulkanCommandEncoder> cmdEncoder = MakeNN<sVulkanCommandEncoder>(aDriver);
  niCheck(cmdEncoder->_CreateCommandBuffer(), nullptr);
  return cmdEncoder;
}

struct sVulkanTexture : public ImplRC<iTexture> {
  nn<sVulkanDriver> _driver;
  VkImage _vkImage = VK_NULL_HANDLE;
  VmaAllocation _vmaAllocation;
  VkImageView _vkView = VK_NULL_HANDLE;
  VkSampler _vkSampler = VK_NULL_HANDLE;
  tU32 _width = 0, _height = 0;
  const tHStringPtr _name;
  const tTextureFlags _flags = eTextureFlags_Default;
  eGpuPixelFormat _pixelFormat = eGpuPixelFormat_None;

  sVulkanTexture(ain_nn<sVulkanDriver> aDriver, iHString* ahspName)
      : _driver(aDriver)
      , _name(ahspName)
  {
  }

  ~sVulkanTexture() {
    if (_vkSampler) {
      vkDestroySampler(_driver->_device, _vkSampler, nullptr);
      _vkSampler = VK_NULL_HANDLE;
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

  tBool _CreateTextureRGBA8(tU32 aWidth, tU32 aHeight, const tPtr apData) {
    _pixelFormat = eGpuPixelFormat_RGBA8;
    _width = aWidth;
    _height = aHeight;

    // Create image
    VkImageCreateInfo imageInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = VK_FORMAT_R8G8B8A8_UNORM,
      .extent = {_width, _height, 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VmaAllocationCreateInfo allocInfo = {
      .usage = VMA_MEMORY_USAGE_GPU_ONLY
    };

    VK_CHECK(vmaCreateImage(
      _driver->_allocator, &imageInfo, &allocInfo,
      &_vkImage, &_vmaAllocation, nullptr), eFalse);

    // Staging buffer
    VkBuffer vkStagingBuffer;
    VmaAllocation vmaStagingAllocation;

    VkBufferCreateInfo bufferInfo = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = _width * _height * sizeof(tU32),
      .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
    };

    VmaAllocationCreateInfo stagingAllocInfo = {
      .usage = VMA_MEMORY_USAGE_CPU_TO_GPU
    };

    VK_CHECK(vmaCreateBuffer(
      _driver->_allocator, &bufferInfo, &stagingAllocInfo,
      &vkStagingBuffer, &vmaStagingAllocation, nullptr), eFalse);

    // Copy data to staging
    void* data;
    VK_CHECK(vmaMapMemory(_driver->_allocator, vmaStagingAllocation, &data), eFalse);
    memcpy(data, apData, _width * _height * sizeof(tU32));
    vmaUnmapMemory(_driver->_allocator, vmaStagingAllocation);

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
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
      },
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT
    };

    vkCmdPipelineBarrier(cmdBuf,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);

    // Copy buffer to image
    VkBufferImageCopy region = {
      .bufferOffset = 0,
      .bufferRowLength = 0,
      .bufferImageHeight = 0,
      .imageSubresource = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevel = 0,
        .baseArrayLayer = 0,
        .layerCount = 1
      },
      .imageOffset = {0, 0, 0},
      .imageExtent = {_width, _height, 1}
    };

    vkCmdCopyBufferToImage(cmdBuf, vkStagingBuffer, _vkImage,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    // Transition to shader read
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmdBuf,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);

    _driver->EndSingleTimeCommands(cmdBuf);

    // Cleanup staging
    vmaDestroyBuffer(_driver->_allocator, vkStagingBuffer, vmaStagingAllocation);

    // Create image view
    VkImageViewCreateInfo viewInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = _vkImage,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = VK_FORMAT_R8G8B8A8_UNORM,
      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
      }
    };

    VK_CHECK(vkCreateImageView(_driver->_device, &viewInfo, nullptr, &_vkView), eFalse);

    // Create sampler
    VkSamplerCreateInfo samplerInfo = {
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .magFilter = VK_FILTER_LINEAR,
      .minFilter = VK_FILTER_LINEAR,
      .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
      .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .mipLodBias = 0.0f,
      .anisotropyEnable = VK_FALSE,
      .maxAnisotropy = 1.0f,
      .compareEnable = VK_FALSE,
      .compareOp = VK_COMPARE_OP_ALWAYS,
      .minLod = 0.0f,
      .maxLod = 0.0f,
      .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
      .unnormalizedCoordinates = VK_FALSE
    };

    VK_CHECK(vkCreateSampler(_driver->_device, &samplerInfo, nullptr, &_vkSampler), eFalse);

    return eTrue;
  }
};

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
    _toVkSamplerFilter(desc, eSamplerFilter_##FILTER);                  \
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

  _whiteTexture = as_NN(niNew sVulkanTexture(as_nn(this),_HC(__vktex_white__)));
  const tU32 pixel[1] = {0xFFFFFFFF};
  niCheck(_whiteTexture->_CreateTextureRGBA8(1,1,(tPtr)pixel),eFalse);

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
      eGLSLVulkanLayoutSets_Buffer,
      writes.size(),writes.data());
  }

  {
    sVulkanTexture* texture = (sVulkanTexture*)_cache._lastMaterial.mChannels[0].mTexture.raw_ptr();
    if (!texture) {
      texture = _driver->_whiteTexture.raw_ptr();
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
      eGLSLVulkanLayoutSets_Texture2D,
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
      eGLSLVulkanLayoutSets_Sampler,
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

struct sVulkanWindowSink : public ImplRC<iMessageHandler> {
  const tU32 _threadId;
  iOSWindow* _w;
  Ptr<iOSXMetalAPI> _metalAPI;
  Ptr<sVulkanDriver> _driver;
  sVulkanFramebuffer _currentFb;

  sVulkanWindowSink(iOSWindow* aWindow)
      : _threadId(ni::ThreadGetCurrentThreadID())
      , _w(aWindow)
      , _driver(niNew sVulkanDriver())
  {
  }

  ~sVulkanWindowSink() {
    this->Cleanup();
    _currentFb.Destroy(_driver->_device);
    _driver = nullptr;
  }

  tU64 __stdcall GetThreadID() const {
    return _threadId;
  }

  virtual tBool __stdcall Init(const achar* aAppName) {
    osxMetalSetDefaultDevice();
    _metalAPI = osxMetalCreateAPIForWindow(osxMetalGetDevice(),_w);
    niCheck(_metalAPI.IsOK(),eFalse);
    niCheck(_driver->Init(aAppName),eFalse);
    niCheck(_currentFb.CreateFromMetalAPI(_metalAPI, _driver->_device), eFalse);
    return eTrue;
  }

  virtual void Cleanup() {
  }

  tBool PresentAndCommit() {
    _metalAPI->DrawablePresent();
    return eTrue;
  }

  tBool BeginFrame() {
    _currentFb.Destroy(_driver->_device);
    niCheck(_currentFb.CreateFromMetalAPI(_metalAPI, _driver->_device), eFalse);
    return eTrue;
  }
  virtual void Draw() = 0;

  virtual void __stdcall HandleMessage(const tU32 anMsg, const Var& a, const Var& b) {
    switch (anMsg) {
      case eOSWindowMessage_Close:
        niDebugFmt((_A("eOSWindowMessage_Close: \n")));
        _w->Invalidate();
        break;
      case eOSWindowMessage_SwitchIn:
        niDebugFmt((_A("eOSWindowMessage_SwitchIn: \n")));
        break;
      case eOSWindowMessage_SwitchOut:
        niDebugFmt((_A("eOSWindowMessage_SwitchOut: \n")));
        break;
      case eOSWindowMessage_Size:
        niDebugFmt((_A("eOSWindowMessage_Size: %s\n"),_ASZ(_w->GetSize())));
        break;
      case eOSWindowMessage_Move:
        niDebugFmt((_A("eOSWindowMessage_Move: %s\n"),_ASZ(_w->GetPosition())));
        break;
      case eOSWindowMessage_KeyDown:
        niDebugFmt((_A("eOSWindowMessage_KeyDown: %d (%s)\n"),a.mU32,niEnumToChars(eKey,a.mU32)));
        switch (a.mU32) {
          case eKey_F:
            _w->SetFullScreen(_w->GetFullScreen() == eInvalidHandle ? 0 : eInvalidHandle);
            break;
          case eKey_Z:
            _w->SetCursorPosition(sVec2i::Zero());
            break;
          case eKey_X:
            _w->SetCursorPosition(_w->GetClientSize()/2);
            break;
          case eKey_C:
            _w->SetCursorPosition(_w->GetClientSize());
            break;
          case eKey_Q:
            _w->SetCursor(eOSCursor_None);
            break;
          case eKey_W:
            _w->SetCursor(eOSCursor_Arrow);
            break;
          case eKey_E:
            _w->SetCursor(eOSCursor_Text);
            break;
          case eKey_K:
            _w->SetCursorCapture(!_w->GetCursorCapture());
            break;
        }
        break;
      case eOSWindowMessage_KeyUp:
        niDebugFmt((_A("eOSWindowMessage_KeyUp: %d (%s)\n"),a.mU32,niEnumToChars(eKey,a.mU32)));
        break;
      case eOSWindowMessage_KeyChar:
        niDebugFmt((_A("eOSWindowMessage_KeyChar: %c (%d) \n"),a.mU32,a.mU32));
        break;
      case eOSWindowMessage_MouseButtonDown:
        niDebugFmt((_A("eOSWindowMessage_MouseButtonDown: %d\n"),a.mU32));
        break;
      case eOSWindowMessage_MouseButtonUp:
        niDebugFmt((_A("eOSWindowMessage_MouseButtonUp: %d\n"),a.mU32));
        break;
      case eOSWindowMessage_MouseButtonDoubleClick:
        niDebugFmt((_A("eOSWindowMessage_MouseButtonDoubleClick: %d\n"),a.mU32));
        if (a.mU32 == 9 /*ePointerButton_DoubleClickRight*/) {
          //                     _w->SetFullScreen(!_w->GetFullScreen());
        }
        else if (a.mU32 == 8 /*ePointerButton_DoubleClickLeft*/) {
          //                     _w->SetFullScreen(!_w->GetFullScreen());
        }
        break;
      case eOSWindowMessage_MouseWheel:
        niDebugFmt((_A("eOSWindowMessage_MouseWheel: %g, %g\n"),a.GetFloatValue(),b.GetFloatValue()));
        break;
      case eOSWindowMessage_LostFocus: {
        niDebugFmt((_A("eOSWindowMessage_LostFocus: \n")));
        break;
      }
      case eOSWindowMessage_SetFocus:
        niDebugFmt((_A("eOSWindowMessage_SetFocus: \n")));
        break;
      case eOSWindowMessage_Drop:
        niDebugFmt((_A("eOSWindowMessage_Drop: \n")));
        break;

      case eOSWindowMessage_RelativeMouseMove:
#ifdef TRACE_MOUSE_MOVE
        niDebugFmt((_A("eOSWindowMessage_RelativeMouseMove: [%d,%d]\n"),
                    a.mV2L[0],a.mV2L[1]));
#endif
        break;

      case eOSWindowMessage_MouseMove:
#ifdef TRACE_MOUSE_MOVE
        niDebugFmt((_A("eOSWindowMessage_MouseMove: [%d,%d] [%d,%d] (contentsScale: %g)\n"),
                    a.mV2L[0],a.mV2L[1],
                    b.mV2L[0],b.mV2L[1],
                    _w->GetContentsScale()));
#endif
        break;
      case eOSWindowMessage_Paint: {
        this->Draw();
        break;
      };
    }
  }
};

TEST_FIXTURE(FOSWindowVulkan,Clear) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  AUTO_WARNING_MODE_IF(UnitTest::IsRunningInCI());

  Ptr<iOSWindow> wnd = ni::GetLang()->CreateWindow(
    NULL,
    "HelloWindow",
    sRecti(50,50,400,300),
    0,
    eOSWindowStyleFlags_Regular);
  CHECK(wnd.IsOK());

  struct sVulkanClear_VulkanWindowSink : public sVulkanWindowSink {
    sVulkanClear_VulkanWindowSink(iOSWindow* w) : sVulkanWindowSink(w) {}

    sVec4f _clearColor = { 0.0f, 1.0f, 1.0f, 1.0f };
    tF64 _clearTimer = 0.0f;
    NN<sVulkanCommandEncoder> _cmdEncoder = niDeferredInit(NN<sVulkanCommandEncoder>);

    tBool Init(const achar* aAppName) niImpl {
      niCheck(sVulkanWindowSink::Init(aAppName), eFalse);
      _cmdEncoder = niCheckNN(_cmdEncoder, _CreateVulkanCommandEncoder(as_nn(_driver)), eFalse);
      return eTrue;
    }

    void Cleanup() niImpl {
      // _cmdEncoder = nullptr;
    }

    void Draw() niImpl {
      niCheck(_cmdEncoder->WaitForInFlightFence(), ;);
      niCheck(BeginFrame(),;);

      if ((ni::TimerInSeconds()-_clearTimer) > 0.5f) {
        _clearColor = Vec4f(
          (tF32)RandFloat(), // r
          (tF32)RandFloat(), // g
          (tF32)RandFloat(), // b
          1.0f               // a
        );
        _clearTimer = ni::TimerInSeconds();
      }

      niCheck(_cmdEncoder->_BeginFrame(_currentFb,_clearColor),;);
      {
        // Nothing, we're just clearing the buffer
      }
      niCheck(_cmdEncoder->_EndFrame(),;);

      this->PresentAndCommit();
    };
  };

  Ptr<sVulkanWindowSink> metalSink = niNew sVulkanClear_VulkanWindowSink(wnd);
  CHECK_RETURN_IF_FAILED(metalSink->Init(m_testName));
  wnd->GetMessageHandlers()->AddSink(metalSink);

  if (isInteractive) {
    wnd->CenterWindow();
    wnd->SetShow(eOSWindowShowFlags_Show);
    wnd->ActivateWindow();
    while (!wnd->GetRequestedClose()) {
      wnd->UpdateWindow(eTrue);
    }
  }
}

TEST_FIXTURE(FOSWindowVulkan,Triangle) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  AUTO_WARNING_MODE_IF(UnitTest::IsRunningInCI());

  Ptr<iOSWindow> wnd = ni::GetLang()->CreateWindow(
    NULL,
    "HelloWindow",
    sRecti(50,50,400,300),
    0,
    eOSWindowStyleFlags_Regular);
  CHECK(wnd.IsOK());

  struct sVulkanTriangle_VulkanWindowSink : public sVulkanWindowSink {
    sVulkanTriangle_VulkanWindowSink(iOSWindow* w) : sVulkanWindowSink(w) {}

    sVec4f _clearColor = { 0.0f, 1.0f, 1.0f, 1.0f };
    tF64 _clearTimer = 0.0f;
    NN<sVulkanCommandEncoder> _cmdEncoder = niDeferredInit(NN<sVulkanCommandEncoder>);
    Ptr<sVulkanBuffer> _vaBuffer;
    Ptr<sVulkanPipeline> _pipeline;

    tBool _CreateArrays() {
      cFVFDescription fvfDesc(sVertexPA::eFVF);
      _vaBuffer = niNew sVulkanBuffer(as_nn(_driver), eGpuBufferMemoryMode_Shared, eGpuBufferUsageFlags_Vertex);
      niCheck(
        _vaBuffer->Create(3 * fvfDesc.GetStride()),
        eFalse);
      {
        sVertexPA* verts = (sVertexPA*)_vaBuffer->Lock(0, 3, eLock_Discard);
        niCheck(verts != nullptr, eFalse);
        verts[0] = {{0.0f, -0.5f, 0.0f}, 0xFF0000FF}; // Red
        verts[1] = {{0.5f, 0.5f, 0.0f}, 0xFF00FF00};  // Green
        verts[2] = {{-0.5f, 0.5f, 0.0f}, 0xFFFF0000}; // Blue
        _vaBuffer->Unlock();
      }
      return eTrue;
    }

    tBool Init(const achar* aAppName) override {
      niCheck(sVulkanWindowSink::Init(aAppName), eFalse);
      _cmdEncoder = niCheckNN(_cmdEncoder, _CreateVulkanCommandEncoder(as_nn(_driver)), eFalse);
      niCheck(_CreateArrays(), eFalse);
      _pipeline = _CreateVulkanPipelineTriangle(as_nn(_driver));
      return eTrue;
    }

    void Cleanup() override {
      _vaBuffer = nullptr;
      _pipeline = nullptr;
      sVulkanWindowSink::Cleanup();
    }

    void Draw() override {
      niCheck(_cmdEncoder->WaitForInFlightFence(), ;);
      niCheck(BeginFrame(),;);

      if ((ni::TimerInSeconds()-_clearTimer) > 0.5f) {
        _clearColor = Vec4f(
          (tF32)RandFloat(), // r
          (tF32)RandFloat(), // g
          (tF32)RandFloat(), // b
          1.0f               // a
        );
        _clearTimer = ni::TimerInSeconds();
      }

      niCheck(_cmdEncoder->_BeginFrame(_currentFb,_clearColor),;);
      {
        niLet vp = Recti(0,0,_currentFb._width,_currentFb._height);
        _cmdEncoder->SetViewport(vp);
        _cmdEncoder->SetScissorRect(vp);
        _cmdEncoder->SetPipeline(_pipeline);
        _cmdEncoder->SetVertexBuffer(_vaBuffer,0,0);
        _cmdEncoder->Draw(eGraphicsPrimitiveType_TriangleList,3,0);
      }
      niCheck(_cmdEncoder->_EndFrame(),;);
      this->PresentAndCommit();
    };
  };

  Ptr<sVulkanWindowSink> metalSink = niNew sVulkanTriangle_VulkanWindowSink(wnd);
  CHECK_RETURN_IF_FAILED(metalSink->Init(m_testName));
  wnd->GetMessageHandlers()->AddSink(metalSink);

  if (isInteractive) {
    wnd->CenterWindow();
    wnd->SetShow(eOSWindowShowFlags_Show);
    wnd->ActivateWindow();
    while (!wnd->GetRequestedClose()) {
      wnd->UpdateWindow(eTrue);
    }
  }
}

TEST_FIXTURE(FOSWindowVulkan,Square) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  AUTO_WARNING_MODE_IF(UnitTest::IsRunningInCI());

  Ptr<iOSWindow> wnd = ni::GetLang()->CreateWindow(
    NULL,
    "HelloWindow",
    sRecti(50,50,400,300),
    0,
    eOSWindowStyleFlags_Regular);
  CHECK(wnd.IsOK());

  struct sVulkanSquare_VulkanWindowSink : public sVulkanWindowSink {
    sVulkanSquare_VulkanWindowSink(iOSWindow* w) : sVulkanWindowSink(w) {}

    sVec4f _clearColor = { 0.0f, 1.0f, 1.0f, 1.0f };
    tF64 _clearTimer = 0.0f;
    NN<sVulkanCommandEncoder> _cmdEncoder = niDeferredInit(NN<sVulkanCommandEncoder>);
    Ptr<sVulkanBuffer> _vaBuffer;
    Ptr<sVulkanBuffer> _iaBuffer;
    Ptr<sVulkanPipeline> _pipeline;

    tBool _CreateArrays() {
      {
        cFVFDescription fvfDesc(sVertexPA::eFVF);
        _vaBuffer = niNew sVulkanBuffer(as_nn(_driver), eGpuBufferMemoryMode_Shared, eGpuBufferUsageFlags_Vertex);
        niCheck(_vaBuffer->Create(4 * fvfDesc.GetStride()), eFalse);
        sVertexPA* pVerts = (sVertexPA*)_vaBuffer->Lock(0, 4, eLock_Discard);
        pVerts[0] = {{-0.5f, -0.5f, 0.0f}, 0xFF0000FF}; // Bottom Left - Red
        pVerts[1] = {{ 0.5f, -0.5f, 0.0f}, 0xFF00FF00}; // Bottom Right - Green
        pVerts[2] = {{ 0.5f,  0.5f, 0.0f}, 0xFFFF0000}; // Top Right - Blue
        pVerts[3] = {{-0.5f,  0.5f, 0.0f}, 0xFFFFFFFF}; // Top Left - White
        _vaBuffer->Unlock();
      }
      {
        _iaBuffer = niNew sVulkanBuffer(as_nn(_driver), eGpuBufferMemoryMode_Shared, eGpuBufferUsageFlags_Index);
        niCheck(_iaBuffer->Create(6 * sizeof(tU32)), eFalse);
        tU32* pIndices = (tU32*)_iaBuffer->Lock(0, 6, eLock_Discard);
        pIndices[0] = 0; pIndices[1] = 1; pIndices[2] = 2; // First triangle
        pIndices[3] = 2; pIndices[4] = 3; pIndices[5] = 0; // Second triangle
        _iaBuffer->Unlock();
      }
      return eTrue;
    }

    tBool Init(const achar* aAppName) override {
      niCheck(sVulkanWindowSink::Init(aAppName), eFalse);
      _cmdEncoder = niCheckNN(_cmdEncoder, _CreateVulkanCommandEncoder(as_nn(_driver)), eFalse);
      _pipeline = _CreateVulkanPipelineTriangle(as_nn(_driver));
      niCheckIsOK(_pipeline, eFalse);
      niCheck(_CreateArrays(), eFalse);
      return eTrue;
    }

    void Cleanup() override {
      _vaBuffer = nullptr;
      _iaBuffer = nullptr;
      sVulkanWindowSink::Cleanup();
    }

    void Draw() override {
      niCheck(_cmdEncoder->WaitForInFlightFence(), ;);
      niCheck(BeginFrame(),;);

      if ((ni::TimerInSeconds()-_clearTimer) > 0.5f) {
        _clearColor = Vec4f(
          (tF32)RandFloat(), // r
          (tF32)RandFloat(), // g
          (tF32)RandFloat(), // b
          1.0f               // a
        );
        _clearTimer = ni::TimerInSeconds();
      }

      niCheck(_cmdEncoder->_BeginFrame(_currentFb,_clearColor),;);
      {
        niLet vp = Recti(0,0,_currentFb._width,_currentFb._height);
        _cmdEncoder->SetViewport(vp);
        _cmdEncoder->SetScissorRect(vp);
        _cmdEncoder->SetPipeline(_pipeline);
        _cmdEncoder->SetVertexBuffer(_vaBuffer,0,0);
        _cmdEncoder->SetIndexBuffer(_iaBuffer,0,eGpuIndexType_U32);
        _cmdEncoder->DrawIndexed(eGraphicsPrimitiveType_TriangleList,6,0);
      }
      niCheck(_cmdEncoder->_EndFrame(),;);
      this->PresentAndCommit();
    };
  };

  Ptr<sVulkanWindowSink> metalSink = niNew sVulkanSquare_VulkanWindowSink(wnd);
  CHECK_RETURN_IF_FAILED(metalSink->Init(m_testName));
  wnd->GetMessageHandlers()->AddSink(metalSink);

  if (isInteractive) {
    wnd->CenterWindow();
    wnd->SetShow(eOSWindowShowFlags_Show);
    wnd->ActivateWindow();
    while (!wnd->GetRequestedClose()) {
      wnd->UpdateWindow(eTrue);
    }
  }
}

TEST_FIXTURE(FOSWindowVulkan,Texture) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  AUTO_WARNING_MODE_IF(UnitTest::IsRunningInCI());

  Ptr<iOSWindow> wnd = ni::GetLang()->CreateWindow(
    NULL,
    "HelloWindow",
    sRecti(50,50,400,300),
    0,
    eOSWindowStyleFlags_Regular);
  CHECK(wnd.IsOK());

  struct sVulkanTexture_VulkanWindowSink : public sVulkanWindowSink {
    sVulkanTexture_VulkanWindowSink(iOSWindow* w) : sVulkanWindowSink(w) {}

    sVec4f _clearColor = { 0.0f, 1.0f, 1.0f, 1.0f };
    tF64 _clearTimer = 0.0f;
    NN<sVulkanCommandEncoder> _cmdEncoder = niDeferredInit(NN<sVulkanCommandEncoder>);
    Ptr<sVulkanBuffer> _vaBuffer;
    Ptr<sVulkanBuffer> _iaBuffer;
    Ptr<sVulkanPipeline> _pipeline;
    Ptr<sVulkanTexture> _texture;

    tBool _CreateArrays() {
      {
        cFVFDescription fvfDesc(sVertexPAT1::eFVF);
        _vaBuffer = niNew sVulkanBuffer(as_nn(_driver), eGpuBufferMemoryMode_Shared, eGpuBufferUsageFlags_Vertex);
        niCheck(_vaBuffer->Create(4 * fvfDesc.GetStride()), eFalse);
        sVertexPAT1* pVerts = (sVertexPAT1*)_vaBuffer->Lock(0, 4, eLock_Discard);
        pVerts[0] = {{-0.5f, -0.5f, 0.0f}, 0xFFFFFFFF, {0.0f, 0.0f}}; // TL (White)
        pVerts[1] = {{ 0.5f, -0.5f, 0.0f}, 0xFFFF0000, {1.0f, 0.0f}}; // TR (Red)
        pVerts[2] = {{ 0.5f,  0.5f, 0.0f}, 0xFF00FF00, {1.0f, 1.0f}}; // BR (Green)
        pVerts[3] = {{-0.5f,  0.5f, 0.0f}, 0xFF0000FF, {0.0f, 1.0f}}; // BL (Blue)
        _vaBuffer->Unlock();
      }
      {
        _iaBuffer = niNew sVulkanBuffer(as_nn(_driver), eGpuBufferMemoryMode_Shared, eGpuBufferUsageFlags_Index);
        niCheck(_iaBuffer->Create(6 * sizeof(tU32)), eFalse);
        tU32* pIndices = (tU32*)_iaBuffer->Lock(0, 6, eLock_Discard);
        pIndices[0] = 0; pIndices[1] = 1; pIndices[2] = 2; // First triangle
        pIndices[3] = 2; pIndices[4] = 3; pIndices[5] = 0; // Second triangle
        _iaBuffer->Unlock();
      }
      return eTrue;
    }

    tBool _CreateTextures() {
      Ptr<iFile> fp = niFileOpenBin2H(A_jpg);
      niCheck(fp.IsOK(),eFalse);
      QPtr<iBitmap2D> bmp = _driver->_graphics->LoadBitmap(fp);
      niCheck(bmp.IsOK(),eFalse);
      // Make sure its RGBA
      {
        Ptr<iPixelFormat> pxf = _driver->_graphics->CreatePixelFormat("R8G8B8A8");
        bmp = bmp->CreateConvertedFormat(pxf);
        niCheck(bmp.IsOK(),eFalse);
      }
      niDebugFmt(("... Loaded bitmap '%s' %dx%d %s",
                  fp->GetSourcePath(),
                  bmp->GetWidth(),
                  bmp->GetHeight(),
                  bmp->GetPixelFormat()->GetFormat()));

      _texture = niNew sVulkanTexture(as_nn(_driver), _H(fp->GetSourcePath()));
      niCheck(_texture->_CreateTextureRGBA8(
        bmp->GetWidth(), bmp->GetHeight(), bmp->GetData()), eFalse);

      return eTrue;
    }

    tBool Init(const achar* aAppName) override {
      niCheck(sVulkanWindowSink::Init(aAppName), eFalse);
      _cmdEncoder = niCheckNN(_cmdEncoder, _CreateVulkanCommandEncoder(as_nn(_driver)), eFalse);
      _pipeline = _CreateVulkanPipelineTexture(as_nn(_driver));
      niCheckIsOK(_pipeline, eFalse);
      niCheck(_CreateArrays(), eFalse);
      niCheck(_CreateTextures(), eFalse);
      return eTrue;
    }

    void Cleanup() override {
      _texture = nullptr;
      _vaBuffer = nullptr;
      _iaBuffer = nullptr;
      _pipeline = nullptr;
      sVulkanWindowSink::Cleanup();
    }

    void Draw() override {
      niCheck(_cmdEncoder->WaitForInFlightFence(), ;);
      niCheck(BeginFrame(),;);

      if ((ni::TimerInSeconds()-_clearTimer) > 0.5f) {
        _clearColor = Vec4f(
          (tF32)RandFloat(), // r
          (tF32)RandFloat(), // g
          (tF32)RandFloat(), // b
          1.0f               // a
        );
        _clearTimer = ni::TimerInSeconds();
      }

      niCheck(_cmdEncoder->_BeginFrame(_currentFb,_clearColor),;);
      {
        niLetMut cmdBuffer = _cmdEncoder->_cmdBuffer;
        sRecti vp = Recti(0,0,_currentFb._width,_currentFb._height);
        _cmdEncoder->SetViewport(vp);
        _cmdEncoder->SetScissorRect(vp);
        _cmdEncoder->SetPipeline(_pipeline);
        _cmdEncoder->SetVertexBuffer(_vaBuffer,0,0);
        _cmdEncoder->SetIndexBuffer(_iaBuffer,0,eGpuIndexType_U32);
        _cmdEncoder->SetTexture(_texture,0);
        _cmdEncoder->SetSamplerState(eCompiledStates_SS_PointRepeat,0);
        _cmdEncoder->DrawIndexed(eGraphicsPrimitiveType_TriangleList,6,0);
      }
      niCheck(_cmdEncoder->_EndFrame(),;);
      this->PresentAndCommit();
    };
  };

  Ptr<sVulkanWindowSink> vulkanSink = niNew sVulkanTexture_VulkanWindowSink(wnd);
  CHECK_RETURN_IF_FAILED(vulkanSink->Init(m_testName));
  wnd->GetMessageHandlers()->AddSink(vulkanSink);

  if (isInteractive) {
    wnd->CenterWindow();
    wnd->SetShow(eOSWindowShowFlags_Show);
    wnd->ActivateWindow();
    while (!wnd->GetRequestedClose()) {
      wnd->UpdateWindow(eTrue);
    }
  }
}

} // end of namespace ni


#include "../../niUI/src/GDRV_Gpu.cpp"
#endif
