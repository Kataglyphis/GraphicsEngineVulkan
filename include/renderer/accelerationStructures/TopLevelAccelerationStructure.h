#pragma once

#include <vulkan/vulkan.h>
#include "VulkanBuffer.h"

struct TopLevelAccelerationStructure {

  VkAccelerationStructureKHR vulkanAS;
  VulkanBuffer vulkanBuffer;
};
