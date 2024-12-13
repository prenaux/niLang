#pragma once
#ifndef __NIVULKAN_H_C0346F2C_60B4_EF11_98EE_3D62D2536177__
#define __NIVULKAN_H_C0346F2C_60B4_EF11_98EE_3D62D2536177__
//
// Include this header instead of <vulkan/vulkan.h> since this will include
// volk.h on platforms that need it.
//
#include <niLang/Types.h>

#if !defined niVulkan_Implement && !defined niVulkan_Include
#error "E/niVulkan_Implement or niVulkan_Include must be defined before including niVulkan.h"
#endif

//===========================================================================
// Statically/directly linked Vulkan libraries
//===========================================================================
#if defined niOSX
#define niVulkan_Linked

#include <vulkan/vulkan.h>

//===========================================================================
// Dynamically loaded Vulkan libraries
//===========================================================================
#elif defined niLinux || defined niWindows
#define niVulkan_Volk

#if defined niLinuxDesktop
#define VK_USE_PLATFORM_XLIB_KHR
#define niVulkan_UseSurfaceKHR
#endif

#if defined niWindows
#define VK_USE_PLATFORM_WIN32_KHR
#define niVulkan_UseSurfaceKHR
#endif

#ifdef niVulkan_Implement
#define VOLK_IMPLEMENTATION
#endif

#include "volk/volk.h"

//===========================================================================
// Unknown platform
//===========================================================================
#else
#error "Unknown Vulkan platform."
#endif

//===========================================================================
// Other utilities
//===========================================================================

#ifdef _MSC_VER
// warning C4324: 'VmaPoolAllocator<VmaBlockMetadata_TLSF::Block>::Item': structure was padded due to alignment specifier
#pragma warning(disable: 4324)
#endif

#endif // __NIVULKAN_H_C0346F2C_60B4_EF11_98EE_3D62D2536177__
