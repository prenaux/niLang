#!/bin/bash
export HAM_TOOLSET=vulkan_sdk
export HAM_TOOLSET_NAME=vulkan_sdk
export HAM_TOOLSET_DIR="${HAM_HOME}/toolsets/vulkan_sdk"

case "$HAM_BIN_LOA" in
  osx-x64 | osx-arm64)
    ham-brew-install vulkan-headers "include/vulkan/vulkan.h"
    ham-brew-install glslang "bin/glslang"
    ham-brew-install spirv-tools "bin/spirv-opt"
    ham-brew-install molten-vk "bin/MoltenVKShaderConverter"
    ham-brew-install vulkan-tools "bin/vulkaninfo"
    ham-brew-install vulkan-profiles "share/vulkan/explicit_layer.d/VkLayer_khronos_profiles.json"
    export VULKAN_SDK_INCDIR="$(ham-brew-installdir vulkan-headers include)"
    export VULKAN_SDK_MOLTENVK_INCDIR="$(ham-brew-installdir molten-vk include)"
    export VULKAN_SDK_MOLTENVK_LIBDIR="$(ham-brew-installdir molten-vk lib)"
    export VK_ICD_FILENAMES="$(ham-brew-installdir molten-vk)/share/vulkan/icd.d/MoltenVK_icd.json"
    export VK_LAYER_PATH="$(ham-brew-installdir vulkan-profiles)/share/vulkan/explicit_layer.d"
    ;;
  *)
    complain vulkan_sdk "Unsupported arch '$HAM_BIN_LOA'."
    return 1
    ;;
esac
pathenv_add "$HAM_TOOLSET_DIR"

VER="--- vulkan_sdk ------------------------"
if [ "$HAM_NO_VER_CHECK" != "1" ]; then
  if [ ! -e "$VK_LAYER_PATH/VkLayer_khronos_profiles.json" ]; then
    complain vulkan_sdk "Cant find VK_LAYER_PATH '$VK_LAYER_PATH/VkLayer_khronos_profiles.json'." && return 1
  fi
  if [ ! -e "$VK_ICD_FILENAMES" ]; then
    complain vulkan_sdk "Cant find VK_ICD_FILENAMES '$VK_ICD_FILENAMES'." && return 1
  fi

  if ! VER="$VER
glslang: $(glslang --version | grep Glslang)"; then
    echo "E/Can't get version."
    return 1
  fi
  if ! VER="$VER
spirv-opt: $(spirv-opt --version 2>&1)"; then
    echo "E/Can't get version."
    return 1
  fi

  VER="$VER
VULKAN_SDK_INCDIR: $VULKAN_SDK_INCDIR
VULKAN_SDK_MOLTENVK_INCDIR: $VULKAN_SDK_MOLTENVK_INCDIR
VULKAN_SDK_MOLTENVK_LIBDIR: $VULKAN_SDK_MOLTENVK_LIBDIR
VK_ICD_FILENAMES: $VK_ICD_FILENAMES
VK_LAYER_PATH: $VK_LAYER_PATH"

fi

export HAM_TOOLSET_VERSIONS="$HAM_TOOLSET_VERSIONS
$VER"
