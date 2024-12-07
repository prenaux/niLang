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
#elif defined niLinux
#define niVulkan_Volk

#if defined niLinuxDesktop
#define VK_USE_PLATFORM_XLIB_KHR
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

// Why is this is a thing? it completely defeats the purposes of named
// structure initializers.
EA_DISABLE_CLANG_WARNING(-Wreorder-init-list);

#endif // __NIVULKAN_H_C0346F2C_60B4_EF11_98EE_3D62D2536177__
