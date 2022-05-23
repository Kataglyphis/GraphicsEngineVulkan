#pragma once
#include <vulkan/vulkan.h>

#include <string>
#include <sstream>
#include <iostream>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::string prefix("");

	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
		prefix = "VERBOSE: ";
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
		prefix = "INFO: ";
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		prefix = "WARNING: ";
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		prefix = "ERROR: ";
	}


	// Display message to default output (console/logcat)
	std::stringstream debugMessage;
	debugMessage << prefix << "[" << pCallbackData->messageIdNumber << "][" << pCallbackData->pMessageIdName << "] : " << pCallbackData->pMessage;
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		std::cerr << debugMessage.str() << "\n";
	}
	else {
		std::cout << debugMessage.str() << "\n";
	}
	fflush(stdout);
	return VK_FALSE;

}

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

class VulkanDebug
{
public:

	VulkanDebug();

	void						init(VkInstance* vulkanInstance);

	~VulkanDebug();

private:

	VkInstance*					vulkanInstance;
	VkDebugUtilsMessengerEXT	debug_messenger;

};

