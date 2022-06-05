#pragma once
#include <vulkan/vulkan.h>

#include "VulkanBuffer.h"

struct BottomLevelAccelerationStructure {

  VkAccelerationStructureKHR vulkanAS;
  VulkanBuffer vulkanBuffer;
};
