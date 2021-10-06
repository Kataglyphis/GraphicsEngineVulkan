#include "VulkanRenderer.h"

VulkanRenderer::VulkanRenderer()
{
}

int VulkanRenderer::init(std::shared_ptr<MyWindow> window)
{
	this->window = window;
	
	try {

		create_instance();
		create_surface();
		get_physical_device();
		create_logical_device();
		create_swap_chain();
		create_render_pass();
		create_graphics_pipeline();

	}
	catch (const std::runtime_error &e) {

		printf("ERROR: %s\n", e.what());
		return EXIT_FAILURE;

	}

	return 0;
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
	app_info.apiVersion = VK_API_VERSION_1_1;									// the vulkan version

	// creation info for a VkInstance
	VkInstanceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;

	//add validation layers IF enabled to the creeate info struct
	if (enableValidationLayers) {

		create_info.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		create_info.ppEnabledLayerNames = validationLayers.data();

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

	// vector for queue creation information and set for family indices
	std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
	std::set<int> queue_family_indices = {indices.graphics_family, indices.presentation_family};

	// Queue the logical device needs to create and info to do so (only 1 for now, will add more later!)
	for (int queue_family_index : queue_family_indices) {

		VkDeviceQueueCreateInfo queue_create_info{};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = queue_family_index;															// the index of the family to create a queue from
		queue_create_info.queueCount = 1;																											// number of queues to create
		float priority = 1.0f;
		queue_create_info.pQueuePriorities = &priority;																						//Vulkan needs to know how to handle multiple queues, so decide priority (1 = highest)

		queue_create_infos.push_back(queue_create_info);

	}

	// information to create logical device (sometimes called "device") 
	VkDeviceCreateInfo device_create_info{};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());		// number of queue create infos
	device_create_info.pQueueCreateInfos = queue_create_infos.data();														// list of queue create infos so device can create required queues
	device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());		// number of enabled logical device extensions
	device_create_info.ppEnabledExtensionNames = device_extensions.data();;																			// list of enabled logical device extensions 
	
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
	vkGetDeviceQueue(MainDevice.logical_device, indices.presentation_family, 0, &presentation_queue);
}

void VulkanRenderer::create_surface()
{
	// create surface (creates a surface create info struct, runs the create surface function, returns result)
	VkResult result = glfwCreateWindowSurface(instance, window->get_window(), nullptr, &surface);

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create a surface!");

}

void VulkanRenderer::create_swap_chain()
{

	// get swap chain details so we can pick the best settings
	SwapChainDetails swap_chain_details = get_swapchain_details(MainDevice.physical_device);

	// 1. choose best surface format
	// 2. choose best presentation mode 
	// 3. choose swap chain image resolution

	VkSurfaceFormatKHR surface_format = choose_best_surface_format(swap_chain_details.formats);
	VkPresentModeKHR present_mode = choose_best_presentation_mode(swap_chain_details.presentation_mode);
	VkExtent2D extent = choose_swap_extent(swap_chain_details.surface_capabilities);

	// how many images are in the swap chain; get 1 more than the minimum to allow tiple buffering
	uint32_t image_count = swap_chain_details.surface_capabilities.minImageCount + 1;

	// if maxImageCount == 0, then limitless
	if (swap_chain_details.surface_capabilities.maxImageCount > 0 && 
		swap_chain_details.surface_capabilities.maxImageCount < image_count) {

		image_count = swap_chain_details.surface_capabilities.maxImageCount;

	}

	VkSwapchainCreateInfoKHR swap_chain_create_info{};
	swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swap_chain_create_info.surface = surface;																											// swapchain surface
	swap_chain_create_info.imageFormat = surface_format.format;																		// swapchain format
	swap_chain_create_info.imageColorSpace = surface_format.colorSpace;														// swapchain color space
	swap_chain_create_info.presentMode = present_mode;																						// swapchain presentation mode 
	swap_chain_create_info.imageExtent = extent;																									// swapchain image extents
	swap_chain_create_info.minImageCount = image_count;																					// minimum images in swapchain
	swap_chain_create_info.imageArrayLayers = 1;																									// number of layers for each image in chain
	swap_chain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;								// what attachment images will be used as 
	swap_chain_create_info.preTransform = swap_chain_details.surface_capabilities.currentTransform;	// transform to perform on swap chain images
	swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;								// dont do blending; everything opaque
	swap_chain_create_info.clipped = VK_TRUE;																											// of course activate clipping ! :) 
	
	// get queue family indices
	QueueFamilyIndices indices = get_queue_families(MainDevice.physical_device);
	
	// if graphics and presentation families are different then swapchain must let images be shared between families
	if (indices.graphics_family != indices.presentation_family) {

		uint32_t queue_family_indices[] = {
						(uint32_t)indices.graphics_family,
						(uint32_t)indices.presentation_family,
		};

		swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;									// image share handling
		swap_chain_create_info.queueFamilyIndexCount = 2;																					// number of queues to share images between
		swap_chain_create_info.pQueueFamilyIndices = queue_family_indices;														// array of queues to share between 

	}
	else {

		swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swap_chain_create_info.queueFamilyIndexCount = 0;
		swap_chain_create_info.pQueueFamilyIndices = nullptr;

	}

	// if old swap chain been destroyed and this one replaces it then link old one to quickly hand over responsibilities
	swap_chain_create_info.oldSwapchain = VK_NULL_HANDLE;

	// create swap chain 
	VkResult result = vkCreateSwapchainKHR(MainDevice.logical_device, &swap_chain_create_info, nullptr, &swapchain);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed create swapchain!");

	}

	// store for later reference
	swap_chain_image_format = surface_format.format;
	swap_chain_extent = extent;

	// get swapchain images (first count, then values)
	uint32_t swapchain_image_count;
	vkGetSwapchainImagesKHR(MainDevice.logical_device, swapchain, &swapchain_image_count, nullptr);
	std::vector<VkImage> images(swapchain_image_count);
	vkGetSwapchainImagesKHR(MainDevice.logical_device, swapchain, &swapchain_image_count, images.data());

	for (VkImage image : images) {

		// store image handle
		SwapChainImage swap_chain_image{};
		swap_chain_image.image = image;
		swap_chain_image.image_view = create_image_view(image, swap_chain_image_format, VK_IMAGE_ASPECT_COLOR_BIT);
		
		// add to swapchain image list 
		swap_chain_images.push_back(swap_chain_image);
		
	}

}

void VulkanRenderer::create_render_pass()
{

	// Color attachment of render pass
	VkAttachmentDescription color_attachment{};
	color_attachment.format = swap_chain_image_format;													// format to use for attachment
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;												// number of samples to write for multisampling
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;									// describes what to do with attachment before rendering
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;								// describes what to do with attachment after rendering 
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;			// describes what to do with stencil before rendering
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;		// describes what to do with stencil after rendering

	// framebuffer data will be stored as an image, but images can be given different layouts 
	// to give optimal use for certain operations
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;								// image data layout before render pass starts
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;					// image data layout after render pass (to change to)

	// attachment reference uses an attachment index that refers to index in the attachment list passed to renderPassCreateInfo
	VkAttachmentReference color_attachment_reference{};
	color_attachment_reference.attachment = 0;
	color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// information about a particular subpass the render pass is using
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;							// pipeline type subpass is to be bound to
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_reference;

	// need to determine when layout transitions occur using subpass dependencies
	std::array<VkSubpassDependency, 2> subpass_dependencies;

	// conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	// transition must happen after ....
	subpass_dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;											// subpass index (VK_SUBPASS_EXTERNAL = Special value meaning outside of renderpass)
	subpass_dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;	// pipeline stage 
	subpass_dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;						// stage access mask (memory access)

	// but must happen before ...
	subpass_dependencies[0].dstSubpass = 0;
	subpass_dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpass_dependencies[0].dependencyFlags = 0;

	// conversion from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	// transition must happen after ....
	subpass_dependencies[1].srcSubpass = 0;																																												// subpass index 
	subpass_dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;																		// pipeline stage 
	subpass_dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;	// stage access mask (memory access)

	// but must happen before ...
	subpass_dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpass_dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpass_dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpass_dependencies[1].dependencyFlags = 0;

	// create info for render pass 
	VkRenderPassCreateInfo render_pass_create_info{};
	render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_create_info.attachmentCount = 1;
	render_pass_create_info.pAttachments = &color_attachment;
	render_pass_create_info.subpassCount = 1;
	render_pass_create_info.pSubpasses = &subpass;
	render_pass_create_info.dependencyCount = static_cast<uint32_t>(subpass_dependencies.size());
	render_pass_create_info.pDependencies = subpass_dependencies.data();

	VkResult result = vkCreateRenderPass(MainDevice.logical_device, &render_pass_create_info, nullptr, &render_pass);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create render pass!");

	}
}

void VulkanRenderer::create_graphics_pipeline()
{

	auto vertex_shader_code = read_file("../Resources/Shader/vert.spv");
	auto fragment_shader_code = read_file("../Resources/Shader/frag.spv");

	// build shader modules to link to graphics pipeline
	VkShaderModule vertex_shader_module = create_shader_module(vertex_shader_code);
	VkShaderModule fragment_shader_module = create_shader_module(fragment_shader_code);

	// shader stage creation information
	// vertex stage creation information
	VkPipelineShaderStageCreateInfo vertex_shader_create_info{};
	vertex_shader_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; 
	vertex_shader_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertex_shader_create_info.module = vertex_shader_module;
	vertex_shader_create_info.pName = "main";																													// entry point into shader

	// fragment stage creation information
	VkPipelineShaderStageCreateInfo fragment_shader_create_info{};
	fragment_shader_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragment_shader_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragment_shader_create_info.module = fragment_shader_module;
	fragment_shader_create_info.pName = "main";																											// entry point into shader

	VkPipelineShaderStageCreateInfo shader_stages[] = {vertex_shader_create_info, 
																									fragment_shader_create_info};


	// CREATE PIPELINE
	// 1.) Vertex input (TODO: Put in vertex descriptions when resources created)
	VkPipelineVertexInputStateCreateInfo vertex_input_create_info{};
	vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_create_info.vertexBindingDescriptionCount = 0;
	vertex_input_create_info.pVertexBindingDescriptions = nullptr;																				// list of vertex binding descriptions(data spacing/ stride information)
	vertex_input_create_info.vertexAttributeDescriptionCount = 0;
	vertex_input_create_info.pVertexAttributeDescriptions = nullptr;																			// list of vertex attribute descriptions (data format and where to bind to/from)

	// input assembly 
	VkPipelineInputAssemblyStateCreateInfo input_assembly{};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;																	// primitive type to assemble vertices as 
	input_assembly.primitiveRestartEnable = VK_FALSE;																									// allow overwritting of "strip" topology to start new primitives

	// viewport & scissor
	// create a viewport info struct
	VkViewport viewport{};
	viewport.x = 0.0f;																																									// x start coordinate
	viewport.y = 0.0f;																																									// y start coordinate
	viewport.width = (float) swap_chain_extent.width;																										// width of viewport 
	viewport.height = (float)swap_chain_extent.height;																									// height of viewport
	viewport.minDepth = 0.0f;																																					// min framebuffer depth
	viewport.maxDepth = 1.0f;																																					// max framebuffer depth

	// create a scissor info struct
	VkRect2D scissor{};
	scissor.offset = {0,0};																																							// offset to use region from 
	scissor.extent = swap_chain_extent;																																// extent to describe region to use, starting at offset

	VkPipelineViewportStateCreateInfo viewport_state_create_info{};
	viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state_create_info.viewportCount = 1;
	viewport_state_create_info.pViewports = &viewport;
	viewport_state_create_info.scissorCount = 1;
	viewport_state_create_info.pScissors = &scissor;

	//// dynamic states to enable
	//std::vector<VkDynamicState> dynamic_state_enables;
	//dynamic_state_enables.push_back(VK_DYNAMIC_STATE_VIEWPORT);																	// dynamic viewport : can resize in command buffer with vkCmdSetViewport (commandbuffer, 0, 1, &viewport)
	//dynamic_state_enables.push_back(VK_DYNAMIC_STATE_SCISSOR);																		// dynamic scissor :    can resize in command buffer with vkCmdSetScissor (commandbuffer, 0, 1, &scissor)
	//
	//// dynamic state creation info
	//VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
	//dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	//dynamic_state_create_info.dynamicStateCount = static_cast<uint32_t>(dynamic_state_enables.size());
	//dynamic_state_create_info.pDynamicStates = dynamic_state_enables.data();

	// RASTERIZER
	VkPipelineRasterizationStateCreateInfo rasterizer_create_info{};
	rasterizer_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer_create_info.depthClampEnable = VK_FALSE;																								// change if fragments beyond near/far plane are clipped (default) or clamped to plane
	rasterizer_create_info.rasterizerDiscardEnable = VK_FALSE;																						// you don't output anything to a framebuffer but just output data ! 
	rasterizer_create_info.polygonMode = VK_POLYGON_MODE_FILL;																			// how to handle filling points between vertices 
	rasterizer_create_info.lineWidth = 1.0f;																															// how thic lines should be when drawn
	rasterizer_create_info.cullMode = VK_CULL_MODE_BACK_BIT;																					// backface culling as standard
	rasterizer_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;																			// winding to determine which side is front 
	rasterizer_create_info.depthBiasClamp = VK_FALSE;																									// for preventing shadow acne

	// -- MULTISAMPLING --
	VkPipelineMultisampleStateCreateInfo multisample_create_info{};
	multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;		
	multisample_create_info.sampleShadingEnable = VK_FALSE;																					// enable multisampling shading or not 
	multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;														// number of samples to use per fragment

	// -- BLENDING --
	// blend attachment state 
	VkPipelineColorBlendAttachmentState color_state{};
	color_state.colorWriteMask =   VK_COLOR_COMPONENT_R_BIT | 
															VK_COLOR_COMPONENT_G_BIT | 
															VK_COLOR_COMPONENT_B_BIT | 
															VK_COLOR_COMPONENT_A_BIT;																				// apply to all channels 
	
	color_state.blendEnable = VK_TRUE;																																// enable BLENDING
	// blending uses equation: (srcColorBlendFactor * new_color) color_blend_op (dstColorBlendFactor * old_color)
	color_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_state.colorBlendOp = VK_BLEND_OP_ADD;
	color_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_state.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo color_blending_create_info{};
	color_blending_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending_create_info.logicOpEnable = VK_FALSE;																								// alternative to calculations is to use logical operations
	color_blending_create_info.attachmentCount = 1;
	color_blending_create_info.pAttachments = &color_state;
	
	// -- PIPELINE LAYOUT (TODO: Apply future descriptor set layouts) --
	VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
	pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_create_info.setLayoutCount = 0;
	pipeline_layout_create_info.pSetLayouts = nullptr;
	pipeline_layout_create_info.pushConstantRangeCount = 0;
	pipeline_layout_create_info.pPushConstantRanges = nullptr;

	// create pipeline layout
	VkResult result = vkCreatePipelineLayout(MainDevice.logical_device, &pipeline_layout_create_info, nullptr, &pipeline_layout);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create pipeline layout!");

	}

	// -- DEPTH STENCIL TESTING --
	// TODO: set up depth stencil testing

	// -- GRAPHICS PIPELINE CREATION --
	VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
	graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphics_pipeline_create_info.stageCount = 2;
	graphics_pipeline_create_info.pStages = shader_stages;
	graphics_pipeline_create_info.pVertexInputState = &vertex_input_create_info;
	graphics_pipeline_create_info.pInputAssemblyState = &input_assembly;
	graphics_pipeline_create_info.pViewportState = &viewport_state_create_info;
	graphics_pipeline_create_info.pDynamicState = nullptr;
	graphics_pipeline_create_info.pRasterizationState = &rasterizer_create_info;
	graphics_pipeline_create_info.pMultisampleState = &multisample_create_info;
	graphics_pipeline_create_info.pColorBlendState = &color_blending_create_info;
	graphics_pipeline_create_info.pDepthStencilState = nullptr;
	graphics_pipeline_create_info.layout = pipeline_layout;																// pipeline layout pipeline should use 
	graphics_pipeline_create_info.renderPass = render_pass;															// renderpass description the pipeline is compatible with
	graphics_pipeline_create_info.subpass = 0;																					// subpass of renderpass to use with pipeline
	
	// pipeline derivatives : can create multiple pipelines that derive from one another for optimization
	graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;								// existing pipeline to derive from ...
	graphics_pipeline_create_info.basePipelineIndex = -1;																// or index of pipeline being created to derive from (in case creating multiple at once)

	// create graphics pipeline 
	result = vkCreateGraphicsPipelines(MainDevice.logical_device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &graphics_pipeline);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create a graphics pipeline!");

	}

	// Destroy shader modules, no longer needed after pipeline created
	vkDestroyShaderModule(MainDevice.logical_device, vertex_shader_module, nullptr);
	vkDestroyShaderModule(MainDevice.logical_device, fragment_shader_module, nullptr);

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

	/*std::cout << "available extensions:\n";

	for (const auto& extension : extensions) {
		std::cout << '\t' << extension.extensionName << '\n';
	}*/

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

bool VulkanRenderer::check_device_extension_support(VkPhysicalDevice device)
{

	uint32_t extension_count = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

	if (extension_count == 0) {
		return false;
	}

	// populate list of extensions 
	std::vector<VkExtensionProperties> extensions(extension_count);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, extensions.data());

	for (const auto& device_extension : device_extensions) {

		bool has_extension = false;

		for (const auto& extension : extensions) {

			if (strcmp(device_extension, extension.extensionName) == 0) {
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

	bool extensions_supported = check_device_extension_support(device);

	bool swap_chain_valid = false;
	
	if (extensions_supported) {

		SwapChainDetails swap_chain_details = get_swapchain_details(device);
		swap_chain_valid = !swap_chain_details.presentation_mode.empty() && !swap_chain_details.formats.empty();

	} 


	return indices.is_valid() && extensions_supported && swap_chain_valid;
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

		// check if queue family suppports presentation
		VkBool32 presentation_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface, &presentation_support);
		// check if queue is presentation type (can be both graphics and presentation)
		if (queue_family.queueCount > 0 && presentation_support) {
			indices.presentation_family = index;
		}

		// check if queue family indices are in a valid state
		if (indices.is_valid()) {
			break;
		}

		index++;

	}

	return indices;
}

SwapChainDetails VulkanRenderer::get_swapchain_details(VkPhysicalDevice device)
{

	SwapChainDetails swapchain_details{};
	//get the surface capabilities for the given surface on the given physical device
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapchain_details.surface_capabilities);

	uint32_t format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

	// if formats returned, get list of formats
	if (format_count != 0) {

		swapchain_details.formats.resize(format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, swapchain_details.formats.data());

	}

	uint32_t presentation_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentation_count, nullptr);

	// if presentation modes returned, get list of presentation modes
	if (presentation_count > 0) {

		swapchain_details.presentation_mode.resize(presentation_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentation_count, swapchain_details.presentation_mode.data());

	}

	return swapchain_details;
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

VkSurfaceFormatKHR VulkanRenderer::choose_best_surface_format(const std::vector<VkSurfaceFormatKHR>& formats)
{
	//best format is subjective, but I go with:
	// Format:           VK_FORMAT_R8G8B8A8_UNORM (backup-format: VK_FORMAT_B8G8R8A8_UNORM)
	// color_space:  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
	// the condition in if means all formats are available (no restrictions)
	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {

		return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

	}

	// if restricted, search  for optimal format
	for (const auto& format : formats) {

		if ((format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM) && 
			format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {

			return format;

		}

	}

	//in case just return first one--- but really shouldn't be the case ....
	return formats[0];

}

VkPresentModeKHR VulkanRenderer::choose_best_presentation_mode(const std::vector<VkPresentModeKHR>& presentation_modes)
{

	// look for mailbox presentation mode 
	for (const auto& presentation_mode : presentation_modes) {

		if (presentation_mode == VK_PRESENT_MODE_MAILBOX_KHR) {

			return presentation_mode;

		}

	}

	// if can't find, use FIFO as Vulkan spec says it must be present
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::choose_swap_extent(const VkSurfaceCapabilitiesKHR& surface_capabilities)
{
	// if current extent is at numeric limits, than extent can vary. Otherwise it is size of window
	if (surface_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {

		return surface_capabilities.currentExtent;

	}
	else {

		int width, height;
		glfwGetFramebufferSize(window->get_window(), &width, &height);

		// create new extent using window size
		VkExtent2D new_extent{};
		new_extent.width = static_cast<uint32_t>(width);
		new_extent.height = static_cast<uint32_t>(height);

		// surface also defines max and min, so make sure within boundaries bly clamping value
		new_extent.width = std::max(surface_capabilities.minImageExtent.width, std::min(surface_capabilities.maxImageExtent.width, new_extent.width));
		new_extent.height = std::max(surface_capabilities.minImageExtent.height, std::min(surface_capabilities.maxImageExtent.height, new_extent.height));

		return new_extent;

	}
	
}

VkImageView VulkanRenderer::create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags)
{

	VkImageViewCreateInfo view_create_info{};
	view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_create_info.image = image;																											// image to create view for 
	view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;																// typ of image
	view_create_info.format = format;
	view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;									// allows remapping of rgba components to other rgba values 
	view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	// subresources allow the view to view only a part of an image 
	view_create_info.subresourceRange.aspectMask = aspect_flags;												// which aspect of an image to view (e.g. color bit for viewing color)
	view_create_info.subresourceRange.baseMipLevel = 0;																	// start mipmap level to view from
	view_create_info.subresourceRange.levelCount = 1;																		// number of mipmap levels to view 
	view_create_info.subresourceRange.baseArrayLayer = 0;																// start array level to view from 
	view_create_info.subresourceRange.layerCount = 1;																		// number of array levels to view 

	// create image view 
	VkImageView image_view;
	VkResult result = vkCreateImageView(MainDevice.logical_device, &view_create_info, nullptr, &image_view);
	
	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create an image view!");

	}

	return image_view;
}

VkShaderModule VulkanRenderer::create_shader_module(const std::vector<char>& code)
{
	// shader module create info
	VkShaderModuleCreateInfo shader_module_create_info{};
	shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_module_create_info.codeSize = code.size();																											// size of code
	shader_module_create_info.pCode = reinterpret_cast<const uint32_t *>(code.data());											// pointer to code 

	VkShaderModule shader_module;
	VkResult result = vkCreateShaderModule(MainDevice.logical_device, &shader_module_create_info, nullptr, &shader_module);
	
	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create a shader module!");

	}

	return shader_module;

}

void VulkanRenderer::clean_up()
{

	vkDestroyPipeline(MainDevice.logical_device, graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(MainDevice.logical_device, pipeline_layout, nullptr);
	vkDestroyRenderPass(MainDevice.logical_device, render_pass, nullptr);

	for (auto image : swap_chain_images) {

		vkDestroyImageView(MainDevice.logical_device, image.image_view, nullptr);

	}
	vkDestroySwapchainKHR(MainDevice.logical_device, swapchain, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyDevice(MainDevice.logical_device, nullptr);
	vkDestroyInstance(instance, nullptr);

}

VulkanRenderer::~VulkanRenderer()
{
}