#include "VulkanDebug.h"
#include "Utilities.h"

VulkanDebug::VulkanDebug()
{
}

void VulkanDebug::init(VkInstance* vulkanInstance)
{

	this->vulkanInstance = vulkanInstance;

	VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};

	if (ENABLE_VALIDATION_LAYERS) {

		messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

		messengerCreateInfo.messageSeverity =	VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
												VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
												VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

		messengerCreateInfo.messageType =	VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
											VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
											VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

		messengerCreateInfo.pfnUserCallback = debugCallback;

		PFN_vkCreateDebugUtilsMessengerEXT pvkCreateDebugUtilsMessengerEXT =
			(PFN_vkCreateDebugUtilsMessengerEXT)
				vkGetInstanceProcAddr(*vulkanInstance, "vkCreateDebugUtilsMessengerEXT");


		ASSERT_VULKAN(pvkCreateDebugUtilsMessengerEXT(	*vulkanInstance,
														&messengerCreateInfo, NULL, &debug_messenger),
														"Failed to create debug messenger.\n");

	}

}

void VulkanDebug::cleanUp()
{
	if (ENABLE_VALIDATION_LAYERS) {

		PFN_vkDestroyDebugUtilsMessengerEXT pvkDestroyDebugUtilsMessengerEXT =
			(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(	*vulkanInstance,
																		"vkDestroyDebugUtilsMessengerEXT");
		pvkDestroyDebugUtilsMessengerEXT(*vulkanInstance, debug_messenger, NULL);

	}
}

VulkanDebug::~VulkanDebug()
{

}
