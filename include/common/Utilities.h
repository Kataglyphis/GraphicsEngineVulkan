#pragma once

#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream
#include <vector>
#include <vulkan/vulkan.h>

#include "host_device_shared_vars.h"

// Error checking on vulkan function calls
#define ASSERT_VULKAN(val,error_string)\
            if(val!=VK_SUCCESS) {\
               throw std::runtime_error(error_string); \
            }

#define NOT_YET_IMPLEMENTED throw std::runtime_error("Not yet implemented!");

#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYERS = false;
#else
const bool ENABLE_VALIDATION_LAYERS = true;
#endif

static VkFormat choose_supported_format(VkPhysicalDevice physical_device,
										const std::vector<VkFormat>&formats, 
										VkImageTiling tiling, 
										VkFormatFeatureFlags feature_flags)
{

	// loop through options and find compatible one
	for (VkFormat format : formats) {

		// get properties for give format on this device
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(physical_device, format, &properties);

		// depending on tiling choice, need to check for different bit flag
		if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & feature_flags) == feature_flags) {

			return format;

		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & feature_flags) == feature_flags) {

			return format;

		}

	}

	throw std::runtime_error("Failed to find supported format!");

}