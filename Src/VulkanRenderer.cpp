#include "VulkanRenderer.h"

VulkanRenderer::VulkanRenderer()
{
}

int VulkanRenderer::init(std::shared_ptr<MyWindow> window)
{
	this->window = window;
	
	try {

		create_instance();
		create_debug_messenger();
		get_physical_device();
		create_logical_device();

	}
	catch (const std::runtime_error &e) {

		printf("ERROR: %s\n", e.what());
		return EXIT_FAILURE;

	}

	return 0;
}

void VulkanRenderer::clean_up()
{

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	//vkDestroyDevice(MainDevice.logical_device, nullptr);
	vkDestroyInstance(instance, nullptr);

}

void VulkanRenderer::create_instance()
{

	// take care we 
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("Validation layers requested, but not available!");
	}

	// info about app
	// most data doesn't affect program; is for developer convenience
	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Epic Graphics";								// custom name of app
	app_info.applicationVersion = VK_MAKE_VERSION(1,0,0);			// custom version of app
	app_info.pEngineName = "No Engine";											// custom engine name
	app_info.engineVersion = VK_MAKE_VERSION(1,0,0);					// custom engine version 
	app_info.apiVersion = VK_API_VERSION_1_2;									// the vulkan version

	// creation info for a VkInstance
	VkInstanceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	//add validation layers IF enabled to the creeate info struct
	if (enableValidationLayers) {

		create_info.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		create_info.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

	}
	else {

		create_info.enabledLayerCount = 0;
		create_info.pNext = nullptr;

	}

	// create list to hold instance extensions
	std::vector<const char*> instance_extensions = std::vector<const char*>();

	//Setup extensions the instance will use 
	uint32_t glfw_extensions_count = 0;											// GLFW may require multiple extensions
	const char** glfw_extensions;														// Extensions passed as array of cstrings, so need pointer(array) to pointer (the cstring)

	//set GLFW extensions
	glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);

	// Add GLFW extensions to list of extensions
	for (size_t i = 0; i < glfw_extensions_count; i++) {

		instance_extensions.push_back(glfw_extensions[i]);

	}

	if (enableValidationLayers) {
		instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	
	// check instance extensions supported
	if (!check_instance_extension_support(&instance_extensions)) {

		throw std::runtime_error("VkInstance does not support required extensions!");

	}

	create_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size());
	create_info.ppEnabledExtensionNames = instance_extensions.data();

	// create instance 
	VkResult result = vkCreateInstance(&create_info, nullptr, &instance);

	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Vulkan instance!");
	}

}

void VulkanRenderer::create_logical_device()
{

	// get the queue family indices for the chosen physical device
	QueueFamilyIndices indices = get_queue_families(MainDevice.physical_device);

	// Queue the logical device needs to create and info to do so (only 1 for now, will add more later!)
	VkDeviceQueueCreateInfo queue_create_info{};
	queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info.queueFamilyIndex = indices.graphics_family;									// the index of the family to create a queue from
	queue_create_info.queueCount = 1;																					// number of queues to create
	float priority = 1.0f;
	queue_create_info.pQueuePriorities = &priority;																//Vulkan needs to know how to handle multiple queues, so decide priority (1 = highest)

	// information to create logical device (sometimes called "device") 
	VkDeviceCreateInfo device_create_info{};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount = 1;																// number of queue create infos
	device_create_info.pQueueCreateInfos = &queue_create_info;									// list of queue create infos so device can create required queues
	device_create_info.enabledExtensionCount = 0;																// number of enabled logical device extensions
	device_create_info.ppEnabledExtensionNames = nullptr;												// list of enabled logical device extensions 
	
	// physical device features the logical device will be using 
	VkPhysicalDeviceFeatures device_features{};

	device_create_info.pEnabledFeatures = &device_features;

	// create logical device for the given physical device
	VkResult result = vkCreateDevice(MainDevice.physical_device, &device_create_info, nullptr, &MainDevice.logical_device);

	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a logical device!");
	}

	//  Queues are created at the same time as the device...
	// So we want handle to queues
	// From given logical device of given queue family, of given queue index (0 since only one queue), place reference in given VkQueue
	vkGetDeviceQueue(MainDevice.logical_device, indices.graphics_family, 0, &graphics_queue);

}

void VulkanRenderer::create_debug_messenger()
{

	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT create_info;
	populateDebugMessengerCreateInfo(create_info);

	if (CreateDebugUtilsMessengerEXT(instance, &create_info, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}

}

void VulkanRenderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{

	VkDebugUtilsMessengerCreateInfoEXT create_info{};

	create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

	create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	create_info.pfnUserCallback = debugCallback;
	create_info.pUserData = nullptr; // Optional

}

void VulkanRenderer::get_physical_device()
{
	// Enumerate physical devices the vkInstance can access
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

	// if no devices available, then none support of Vulkan
	if (device_count == 0) {
		throw std::runtime_error("Can not find GPU's that support Vulkan Instance!");
	}

	//Get list of physical devices 
	std::vector<VkPhysicalDevice> device_list(device_count);
	vkEnumeratePhysicalDevices(instance, &device_count, device_list.data());

	for (const auto& device : device_list) {

		if (check_device_suitable(device)) {

			MainDevice.physical_device = device;
			break;

		}

	}
}

bool VulkanRenderer::check_instance_extension_support(std::vector<const char*>* check_extensions)
{

	//Need to get number of extensions to create array of correct size to hold extensions
	uint32_t extension_count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

	// create a list of VkExtensionProperties using count
	std::vector<VkExtensionProperties> extensions(extension_count);
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

	std::cout << "available extensions:\n";

	for (const auto& extension : extensions) {
		std::cout << '\t' << extension.extensionName << '\n';
	}

	// check if given extensions are in list of available extensions 
	for (const auto& check_extension  : *check_extensions) {

		bool has_extension = false;
		
		for (const auto& extension : extensions) {

			if (strcmp(check_extension, extension.extensionName)) {
				has_extension = true;
				break;
			}

		}

		if (!has_extension) {

			return false;

		}

	}

	return true;

}

bool VulkanRenderer::check_device_suitable(VkPhysicalDevice device)
{

	//Information about device itself (ID, name, type, vendor, etc)
	VkPhysicalDeviceProperties device_properties;
	vkGetPhysicalDeviceProperties(device, &device_properties);

	VkPhysicalDeviceFeatures device_features;
	vkGetPhysicalDeviceFeatures(device, &device_features);

	QueueFamilyIndices indices = get_queue_families(device);

	return indices.is_valid();
}

QueueFamilyIndices VulkanRenderer::get_queue_families(VkPhysicalDevice device)
{

	QueueFamilyIndices indices{};

	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

	std::vector<VkQueueFamilyProperties> queue_family_list(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_family_list.data());

	// Go through each queue family and check if it has at least 1 of required types
	// we need to keep track th eindex by our own
	int index = 0;
	for (const auto& queue_family : queue_family_list) {

		// first check if queue family has at least 1 queue in that family 
		// Queue can be multiple types defined through bitfield. Need to bitwise AND with VK_QUE_*_BIT to check if has required  type 
		if (queue_family.queueCount > 0 && queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {

			indices.graphics_family = index; // if queue family valid, than get index

		}

		// check if queue family indices are in a valid state
		if (indices.is_valid()) {
			break;
		}

		index++;

	}

	return indices;
}

bool VulkanRenderer::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

VulkanRenderer::~VulkanRenderer()
{
}