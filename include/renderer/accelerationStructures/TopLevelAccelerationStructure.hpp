#pragma once

#include <vulkan/vulkan.h>

#include "VulkanBuffer.hpp"

struct TopLevelAccelerationStructure
{
    VkAccelerationStructureKHR vulkanAS;
    VulkanBuffer vulkanBuffer;
};
