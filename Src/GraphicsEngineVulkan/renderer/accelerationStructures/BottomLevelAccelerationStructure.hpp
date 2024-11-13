#pragma once
#include <vulkan/vulkan.h>

#include "VulkanBuffer.hpp"

struct BottomLevelAccelerationStructure
{
    VkAccelerationStructureKHR vulkanAS;
    VulkanBuffer vulkanBuffer;
};
