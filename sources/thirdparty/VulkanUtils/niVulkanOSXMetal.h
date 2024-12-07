#pragma once
#ifndef __NIVULKANOSXMETAL_H_107EB1C6_87D2_4D40_81D7_6FCC02168685__
#define __NIVULKANOSXMETAL_H_107EB1C6_87D2_4D40_81D7_6FCC02168685__

#include "niVulkan.h"
#include <niLang/Platforms/OSX/osxMetal.h>

#ifdef niVulkanOSXMetal_Implement
#if !defined(__OBJC__)
#error "niVulkanOSXMetal.h: niVulkanOSXMetal_Implement must be compiled as Objective-C++, include and define niVulkanOSXMetal_Implement before it in a mm file."
#endif
#include <vulkan/vulkan_metal.h>
#include "niVulkanEnumToString.h"
#endif

namespace ni {

niExportFunc(tBool) osxVkCreateImageForMetalAPI(
  iOSXMetalAPI* apMetalAPI,
  VkDevice avkDevice,
  const VkAllocationCallbacks* pAllocator,
  VkImage* pImage)
#ifdef niVulkanOSXMetal_Implement
{
  MTKView* mtkView = (__bridge MTKView*)apMetalAPI->GetMTKView();
  niCheck(mtkView != nullptr, eFalse);

  MTLRenderPassDescriptor* passDesc = mtkView.currentRenderPassDescriptor;
  niCheck(passDesc != nullptr, eFalse);

  id<MTLTexture> mtlTexture = passDesc.colorAttachments[0].texture;
  niCheck(mtlTexture != nullptr, eFalse);

  VkImportMetalTextureInfoEXT importInfo = {
    .sType = VK_STRUCTURE_TYPE_IMPORT_METAL_TEXTURE_INFO_EXT,
    .plane = VK_IMAGE_ASPECT_COLOR_BIT,
    .mtlTexture = mtlTexture
  };

  VkImageCreateInfo imageInfo = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .pNext = &importInfo,
    .imageType = VK_IMAGE_TYPE_2D,
    .format = VK_FORMAT_B8G8R8A8_UNORM,
    .extent = {(tU32)mtlTexture.width, (tU32)mtlTexture.height, 1},
    .mipLevels = 1,
    .arrayLayers = 1,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
  };

  VK_CHECK(vkCreateImage(
    avkDevice,
    &imageInfo,
    pAllocator,
    pImage), eFalse);

  return eTrue;
}
#endif
;

}
#endif // __NIVULKANOSXMETAL_H_107EB1C6_87D2_4D40_81D7_6FCC02168685__
