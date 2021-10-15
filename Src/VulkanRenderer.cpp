#include "VulkanRenderer.h"

VulkanRenderer::VulkanRenderer()
{
}

int VulkanRenderer::init(std::shared_ptr<MyWindow> window, glm::vec3 eye, float near_plane, float far_plane)
{

	this->window = window;
	
	try {

		create_instance();
		create_surface();
		get_physical_device();
		create_logical_device();
		create_swap_chain();
		create_render_pass();
		create_descriptor_set_layout();
		create_push_constant_range();
		create_graphics_pipeline();
		create_depthbuffer_image();
		create_framebuffers();
		create_command_pool();
		create_command_buffers();
		//allocate_dynamic_buffer_transfer_space();
		create_texture_sampler();
		create_uniform_buffers();
		create_descriptor_pool();
		create_gui_context();
		create_fonts_and_upload();
		create_descriptor_sets();
		// no longer initial record needed for we record them every single frame
		//record_commands();
		create_synchronization();

		ubo_view_projection.projection = glm::perspective(glm::radians(45.0f), (float) swap_chain_extent.width / (float) swap_chain_extent.height, 
																		near_plane, far_plane);

		ubo_view_projection.view = glm::lookAt(eye, glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		// for reasons that vulkan needs the y-coordinate in an inverse manner
		ubo_view_projection.projection[1][1] *= -1;

		// create meshes 
		// vertex data
		//std::vector<Vertex> mesh_vertices = {
		//				{{-0.8f, 1.f, 0.0f}, {1.0f, 1.0f}},
		//				{{-0.8f, -1.f, 0.0f}, {1.0f, 0.0f}},
		//				{{0.8f, -1.f, 0.0f}, {0.0f, 0.0f}},
		//				{{0.8f, 1.f, 0.0f}, {0.0f, 1.0f}}
		//};

		//std::vector<Vertex> mesh_vertices2 = {
		//				{{-0.75f, 1.f, 0.0f}, {1.0f, 1.0f}},
		//				{{-0.75f, -1.f, 0.0f}, {1.0f, 0.0f}},
		//				{{0.75f, -1.f, 0.0f}, {0.0f, 0.0f}},
		//				{{0.75f, 1.f, 0.0f}, {0.0f, 1.0f}}
		//};

		//// index data
		//std::vector<uint32_t> mesh_indices = {

		//	0, 1, 2,
		//	2, 3, 0

		//};

		//// graphics queues are actually transfer queues
		//Mesh first_mesh = Mesh(MainDevice.physical_device, MainDevice.logical_device, graphics_queue, graphics_command_pool,
		//								&mesh_vertices, &mesh_indices, create_texture("first_texture.jpg"));

		//Mesh second_mesh = Mesh(MainDevice.physical_device, MainDevice.logical_device, graphics_queue, graphics_command_pool,
		//	&mesh_vertices2, &mesh_indices, create_texture("second_texture.jpg"));

		//meshes.push_back(first_mesh);
		//meshes.push_back(second_mesh);

		// create our default no texture texture
		create_texture("plain.png");

	}
	catch (const std::runtime_error &e) {

		printf("ERROR: %s\n", e.what());
		return EXIT_FAILURE;

	}

	return 0;
}

void VulkanRenderer::update_model(int model_id, glm::mat4 new_model)
{

	if (model_id >= static_cast<int32_t>(model_list.size()) || model_id < 0) {

		throw std::runtime_error("Wrong model id value!");

	}

	model_list[model_id].set_model(new_model);

}

void VulkanRenderer::update_view(glm::mat4 view)
{

	ubo_view_projection.view = view;

}

void VulkanRenderer::update_gui_draw_data(ImDrawData* gui_draw_data)
{
	this->gui_draw_data = gui_draw_data;

}

void VulkanRenderer::draw()
{

	// -- GET NEXT IMAGE --
	VkResult result = vkWaitForFences(MainDevice.logical_device, 1, &draw_fences[current_frame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to wait for fences!");
	}

	result = vkResetFences(MainDevice.logical_device, 1, &draw_fences[current_frame]);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to reset fences!");
	}

	// 1. Get next available image to draw to and set something to signal when we're finished with the image  (a semaphore)
	// wait for given fence to signal (open) from last draw before continuing
	uint32_t image_index;
	result = vkAcquireNextImageKHR(MainDevice.logical_device, swapchain, std::numeric_limits<uint64_t>::max(), image_available[current_frame], VK_NULL_HANDLE, &image_index);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to acquire next image!");

	}

	// only update soecific command buffer not in use 
	record_commands(image_index);

	update_uniform_buffers(image_index);

	// 2. Submit command buffer to queue for execution, making sure it waits for the image to be signalled as available before drawing
	// and signals when it has finished rendering 
	// -- SUBMIT COMMAND BUFFER TO RENDER --
	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.waitSemaphoreCount = 1;																			// number of semaphores to wait on
	submit_info.pWaitSemaphores = &image_available[current_frame];						// list of semaphores to wait on
	
	VkPipelineStageFlags wait_stages[] = {
	
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT

	};

	submit_info.pWaitDstStageMask = wait_stages;															// stages to check semaphores at
	submit_info.commandBufferCount = 1;																			// number of command buffers to submit
	submit_info.pCommandBuffers = &command_buffers[image_index];						// command buffer to submit
	submit_info.signalSemaphoreCount = 1;																			// number of semaphores to signal
	submit_info.pSignalSemaphores = &render_finished[current_frame];						// semaphores to signal when command buffer finishes 

	// submit command buffer to queue
	result = vkQueueSubmit(graphics_queue, 1, &submit_info, draw_fences[current_frame]);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to submit command buffer to queue!");

	}

	// 3. Present image to screen when it has signalled finished rendering 
	// -- PRESENT RENDERED IMAGE TO SCREEN --
	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;																			// number of semaphores to wait on
	present_info.pWaitSemaphores = &render_finished[current_frame];						// semaphores to wait on
	present_info.swapchainCount = 1;																					// number of swapchains to present to
	present_info.pSwapchains = &swapchain;																		// swapchains to present images to 
	present_info.pImageIndices = &image_index;																// index of images in swapchain to present

	result = vkQueuePresentKHR(presentation_queue, &present_info);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to present image!");

	}

	current_frame = (current_frame + 1) % MAX_FRAME_DRAWS;
	 
}

void VulkanRenderer::create_instance()
{

	// take care we 
	if (enableValidationLayers && !check_validation_layer_support()) {
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
	device_features.samplerAnisotropy = VK_TRUE;

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

	// depth attachment of render pass
	VkAttachmentDescription depth_attachment{};
	depth_attachment.format = choose_supported_format({ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT,  VK_FORMAT_D24_UNORM_S8_UINT },
																								VK_IMAGE_TILING_OPTIMAL,
																								VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// attachment reference uses an attachment index that refers to index in the attachment list passed to renderPassCreateInfo
	VkAttachmentReference color_attachment_reference{};
	color_attachment_reference.attachment = 0;
	color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// attachment reference 
	VkAttachmentReference depth_attachment_reference{};
	depth_attachment_reference.attachment = 1;
	depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// information about a particular subpass the render pass is using
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;							// pipeline type subpass is to be bound to
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_reference;
	subpass.pDepthStencilAttachment = &depth_attachment_reference ;

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

	std::array<VkAttachmentDescription, 2> render_pass_attachments = {color_attachment, depth_attachment};

	// create info for render pass 
	VkRenderPassCreateInfo render_pass_create_info{};
	render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_create_info.attachmentCount = static_cast<uint32_t>(render_pass_attachments.size());
	render_pass_create_info.pAttachments = render_pass_attachments.data();
	render_pass_create_info.subpassCount = 1;
	render_pass_create_info.pSubpasses = &subpass;
	render_pass_create_info.dependencyCount = static_cast<uint32_t>(subpass_dependencies.size());
	render_pass_create_info.pDependencies = subpass_dependencies.data();

	VkResult result = vkCreateRenderPass(MainDevice.logical_device, &render_pass_create_info, nullptr, &render_pass);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create render pass!");

	}

}

void VulkanRenderer::create_fonts_and_upload()
{

	VkCommandBuffer command_buffer = beginSingleTimeCommands(MainDevice.logical_device, graphics_command_pool);
	ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
	endSingleTimeCommands(command_buffer, graphics_queue, MainDevice.logical_device, graphics_command_pool);

	//clear font textures from cpu data
	ImGui_ImplVulkan_DestroyFontUploadObjects();


}

void VulkanRenderer::create_descriptor_set_layout()
{
	// UNIFORM VALUES DESCRIPTOR SET LAYOUT

	//ubo_view_projection Binding info
	VkDescriptorSetLayoutBinding ubo_view_projection_layout_binding{};
	ubo_view_projection_layout_binding.binding = 0;																												// binding point in shader (designated by binding number in shader)
	ubo_view_projection_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;						// type of descriptor (uniform, dynamic uniform, image sampler, etc)
	ubo_view_projection_layout_binding.descriptorCount = 1;																								// number of descriptors for binding
	ubo_view_projection_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;												// we need to say at which shader we bind this uniform to
	ubo_view_projection_layout_binding.pImmutableSamplers = nullptr;																			// for texture: can make sampler data unchangeable (immutable) by specifying in layout

	// our model matrix which updates every frame for each object
	/*VkDescriptorSetLayoutBinding model_layout_binding{};
	model_layout_binding.binding = 1;
	model_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	model_layout_binding.descriptorCount = 1;
	model_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	model_layout_binding.pImmutableSamplers = nullptr;*/

	std::vector<VkDescriptorSetLayoutBinding> layout_bindings = { ubo_view_projection_layout_binding
																														/*,model_layout_binding*/};

	// create descriptor set layout with given bindings
	VkDescriptorSetLayoutCreateInfo layout_create_info{};
	layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_create_info.bindingCount = static_cast<uint32_t>(layout_bindings.size());														// only have 1 for the ubo_view_projection
	layout_create_info.pBindings = layout_bindings.data();																										// array of binding infos 

	// create descriptor set layout
	VkResult result = vkCreateDescriptorSetLayout(MainDevice.logical_device, &layout_create_info, nullptr, &descriptor_set_layout);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create descriptor set layout!");

	}

	// CREATE TEXTURE SAMPLER DESCRIPTOR SET LAYOUT
	// texture binding info
	VkDescriptorSetLayoutBinding sampler_layout_binding{};
	sampler_layout_binding.binding = 0;
	sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sampler_layout_binding.descriptorCount = 1;
	sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	sampler_layout_binding.pImmutableSamplers = nullptr;

	// create a descriptor set layout with given bindings for texture 
	VkDescriptorSetLayoutCreateInfo texture_layout_create_info{};
	texture_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	texture_layout_create_info.bindingCount = 1;
	texture_layout_create_info.pBindings = &sampler_layout_binding;

	// create descriptor set layout
	result = vkCreateDescriptorSetLayout(MainDevice.logical_device, &texture_layout_create_info, nullptr, &sampler_set_layout);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create sampler set layout!");

	}

}

void VulkanRenderer::create_push_constant_range()
{

	// define push constant values (no 'create' needed)
	push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;																// shader stage push constant will go to 
	push_constant_range.offset = 0;																															// offset into given data to pass tp push constant
	push_constant_range.size = sizeof(Model);																											// size of data being passed

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


	// how the data for a single vertex (including info such as position, color, texture coords, normals, etc) is as a whole 
	VkVertexInputBindingDescription binding_description{};
	binding_description.binding = 0;																																				// can bind multiple streams of data, this defines which one 
	binding_description.stride = sizeof(Vertex);																															// size of a single vertex object
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;																					// how to move between data after each vertex.
																																																	// VK_VERTEX_INPUT_RATE_VERTEX : Move on to the next vertex
																																																	// VK_VERTEX_INPUT_RATE_INSTANCE : Move on to the next instance
	
	// how the data for an attribute is defined within a vertex
	std::array<VkVertexInputAttributeDescription, 2> attribute_describtions;

	// Position attribute
	attribute_describtions[0].binding = 0;																																// which binding the data is at (should be same as above)
	attribute_describtions[0].location = 0;																																// location in shader where data will be read from
	attribute_describtions[0].format = VK_FORMAT_R32G32B32_SFLOAT;																			// format data will take (also helps define size of data)
	attribute_describtions[0].offset = offsetof(Vertex, pos);																									// where this attribute is defined in the data for a single vertex

	// texture coord attribute
	attribute_describtions[1].binding = 0;																																// which binding the data is at (should be same as above)
	attribute_describtions[1].location = 1;																																// location in shader where data will be read from
	attribute_describtions[1].format = VK_FORMAT_R32G32_SFLOAT;																					// format data will take (also helps define size of data)
	attribute_describtions[1].offset = offsetof(Vertex, texture_coords);																				// where this attribute is defined in the data for a single vertex

	// Color attribute TODO

	// CREATE PIPELINE
	// 1.) Vertex input (TODO: Put in vertex descriptions when resources created)
	VkPipelineVertexInputStateCreateInfo vertex_input_create_info{};
	vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_create_info.vertexBindingDescriptionCount = 1;
	vertex_input_create_info.pVertexBindingDescriptions = &binding_description;																	// list of vertex binding descriptions(data spacing/ stride information)
	vertex_input_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_describtions.size());
	vertex_input_create_info.pVertexAttributeDescriptions = attribute_describtions.data();													// list of vertex attribute descriptions (data format and where to bind to/from)

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
	rasterizer_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;														// winding to determine which side is front; y-coordinate is inverted in comparison to OpenGL
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
	
	// -- PIPELINE LAYOUT --

	std::array<VkDescriptorSetLayout, 2> descriptor_set_layouts = { descriptor_set_layout, sampler_set_layout };

	VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
	pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_create_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
	pipeline_layout_create_info.pSetLayouts = descriptor_set_layouts.data();
	pipeline_layout_create_info.pushConstantRangeCount = 1;
	pipeline_layout_create_info.pPushConstantRanges = &push_constant_range;

	// create pipeline layout
	VkResult result = vkCreatePipelineLayout(MainDevice.logical_device, &pipeline_layout_create_info, nullptr, &pipeline_layout);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create pipeline layout!");

	}

	// -- DEPTH STENCIL TESTING --
	VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info{};
	depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_create_info.depthTestEnable = VK_TRUE;																							// enable checking depth to determine fragment write
	depth_stencil_create_info.depthWriteEnable = VK_TRUE;																						// enable writing to depth buffer for replacing old values
	depth_stencil_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
	depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;																			// depth bounds test: does the depth value exist between 2 bounds
	depth_stencil_create_info.stencilTestEnable = VK_FALSE;

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
	graphics_pipeline_create_info.pDepthStencilState = &depth_stencil_create_info;
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

void VulkanRenderer::create_depthbuffer_image()
{

	// get supported format for depth buffer
	depth_format = choose_supported_format({VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT,  VK_FORMAT_D24_UNORM_S8_UINT }, 
																			VK_IMAGE_TILING_OPTIMAL,
																			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	// create depth buffer image
	depth_buffer_image = create_image(swap_chain_extent.width, swap_chain_extent.height, depth_format, VK_IMAGE_TILING_OPTIMAL,
																	VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
																	VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &depth_buffer_image_memory);

	// depth buffer image view 
	depth_buffer_image_view = create_image_view(depth_buffer_image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);

}

void VulkanRenderer::create_framebuffers()
{

	// resize framebuffer size to equal swap chain image count
	swap_chain_framebuffers.resize(swap_chain_images.size());

	for (size_t i = 0; i < swap_chain_framebuffers.size(); i++) {

		std::array<VkImageView, 2> attachments = {
							swap_chain_images[i].image_view,
							depth_buffer_image_view
		};

		VkFramebufferCreateInfo frame_buffer_create_info{};
		frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frame_buffer_create_info.renderPass = render_pass;																				// render pass layout the framebuffer will be used with
		frame_buffer_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		frame_buffer_create_info.pAttachments = attachments.data();															// list of attachments (1:1 with render pass)
		frame_buffer_create_info.width = swap_chain_extent.width;																	// framebuffer width
		frame_buffer_create_info.height = swap_chain_extent.height;																// framebuffer height
		frame_buffer_create_info.layers = 1;																											// framebuffer layer 

		VkResult result = vkCreateFramebuffer(MainDevice.logical_device, &frame_buffer_create_info, nullptr, &swap_chain_framebuffers[i]);

		if (result != VK_SUCCESS) {

			throw std::runtime_error("Failed to create framebuffer!");

		}

	}
}

void VulkanRenderer::create_command_pool()
{

	// get indices of queue familes from device
	QueueFamilyIndices queue_family_indices = get_queue_families(MainDevice.physical_device);

	VkCommandPoolCreateInfo pool_info{};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;									// we are ready now to re-record our command buffers
	pool_info.queueFamilyIndex = queue_family_indices.graphics_family;													// queue family type that buffers from this command pool will use 

	// create a graphics queue family command pool
	VkResult result = vkCreateCommandPool(MainDevice.logical_device, &pool_info, nullptr, &graphics_command_pool);

	if(result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create command pool!");

	}
}

void VulkanRenderer::create_command_buffers()
{

	// resize command buffer count to have one for each framebuffer
	command_buffers.resize(swap_chain_framebuffers.size());

	VkCommandBufferAllocateInfo command_buffer_alloc_info{};
	command_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_alloc_info.commandPool = graphics_command_pool;
	command_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;											// VK_COMMAND_BUFFER_LEVEL_PRIMARY       : Buffer you submit directly to queue. Cant be called by  other buffers.
																																														// VK_COMMAND_BUFFER_LEVEL_SECONDARY : Buffer can't be called directly. Can be called from other buffers via 
																																														// "vkCmdExecuteCommands" when recording commands in primary buffer
	command_buffer_alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

	VkResult result = vkAllocateCommandBuffers(MainDevice.logical_device, &command_buffer_alloc_info, command_buffers.data());

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to allocate command buffers!");

	}



}

void VulkanRenderer::create_synchronization()
{

	image_available.resize(MAX_FRAME_DRAWS);
	render_finished.resize(MAX_FRAME_DRAWS);
	draw_fences.resize(MAX_FRAME_DRAWS);

	// semaphore creation information
	VkSemaphoreCreateInfo semaphore_create_info{};
	semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	// fence creation information
	VkFenceCreateInfo fence_create_info{};
	fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (int i = 0; i < MAX_FRAME_DRAWS; i++) {

		if ((vkCreateSemaphore(MainDevice.logical_device, &semaphore_create_info, nullptr, &image_available[i]) != VK_SUCCESS) ||
			(vkCreateSemaphore(MainDevice.logical_device, &semaphore_create_info, nullptr, &render_finished[i]) != VK_SUCCESS) || 
			vkCreateFence(MainDevice.logical_device, &fence_create_info, nullptr, &draw_fences[i])							!= VK_SUCCESS) {

			throw std::runtime_error("Failed to create a semaphore and/or fence!");

		}

	}

}

void VulkanRenderer::create_texture_sampler()
{

	// sampler create info
	VkSamplerCreateInfo sampler_create_info{};
	sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_create_info.magFilter = VK_FILTER_LINEAR;
	sampler_create_info.minFilter = VK_FILTER_LINEAR;
	sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_create_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	sampler_create_info.unnormalizedCoordinates = VK_FALSE;
	sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler_create_info.mipLodBias = 0.0f;
	sampler_create_info.minLod = 0.0f;
	sampler_create_info.maxLod = 0.0f;
	sampler_create_info.anisotropyEnable = VK_TRUE;
	sampler_create_info.maxAnisotropy = 16;																							// max anisotropy sample level

	VkResult result = vkCreateSampler(MainDevice.logical_device, &sampler_create_info, nullptr, &texture_sampler);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create a texture sampler!");

	}

}

void VulkanRenderer::create_uniform_buffers()
{

	// buffer size will be size of all two variables (will offset to access) 
	VkDeviceSize vp_buffer_size = sizeof(ubo_view_projection);
	// buffer size will be size of model buffer (will offset to access)
	// VkDeviceSize model_buffer_size = model_uniform_alignment * MAX_OBJECTS;

	// one uniform buffer for each image (and by extension, command buffer)
	vp_uniform_buffer.resize(swap_chain_images.size());
	vp_uniform_buffer_memory.resize(swap_chain_images.size());
	//model_dynamic_uniform_buffer.resize(swap_chain_images.size());
	//model_dynamic_uniform_buffer_memory.resize(swap_chain_images.size());

	// create uniform buffers 
	for (size_t i = 0; i < swap_chain_images.size(); i++) {

		create_buffer(MainDevice.physical_device, MainDevice.logical_device, vp_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
																																							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
																																							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
																																							&vp_uniform_buffer[i], &vp_uniform_buffer_memory[i]);

		/*create_buffer(MainDevice.physical_device, MainDevice.logical_device, model_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
																																							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
																																							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
																																							&model_dynamic_uniform_buffer[i], &model_dynamic_uniform_buffer_memory[i]);*/

	}
}

void VulkanRenderer::create_descriptor_pool()
{

	// CREATE UNIFORM DESCRIPTOR POOL


	// type of descriptors + how many descriptors, not descriptor sets (combined makes the pool size)
	// ViewProjection Pool 
	VkDescriptorPoolSize vp_pool_size{};
	vp_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vp_pool_size.descriptorCount = static_cast<uint32_t>(vp_uniform_buffer.size());

	// Model pool
	/*VkDescriptorPoolSize model_pool_size{};
	model_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	model_pool_size.descriptorCount = static_cast<uint32_t>(model_dynamic_uniform_buffer.size());*/

	// list of pool sizes 
	std::vector<VkDescriptorPoolSize> descriptor_pool_sizes = { vp_pool_size };

	VkDescriptorPoolCreateInfo pool_create_info{};
	pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_create_info.maxSets = static_cast<uint32_t>(swap_chain_images.size());												// maximum number of descriptor sets that can be created from pool
	pool_create_info.poolSizeCount = static_cast<uint32_t>(descriptor_pool_sizes.size());									// amount of pool sizes being passed
	pool_create_info.pPoolSizes = descriptor_pool_sizes.data();																			// pool sizes to create pool with

	// create descriptor pool
	VkResult result = vkCreateDescriptorPool(MainDevice.logical_device, &pool_create_info, nullptr, &descriptor_pool);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create a descriptor pool!");
		  
	}

	// CREATE SAMPLER DESCRIPTOR POOL
	// TEXTURE SAMPLER POOL
	VkDescriptorPoolSize sampler_pool_size{};
	sampler_pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sampler_pool_size.descriptorCount = MAX_OBJECTS;
	
	VkDescriptorPoolCreateInfo sampler_pool_create_info{};
	sampler_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	sampler_pool_create_info.maxSets = MAX_OBJECTS;
	sampler_pool_create_info.poolSizeCount = 1;
	sampler_pool_create_info.pPoolSizes = &sampler_pool_size;

	// create descriptor pool
	result = vkCreateDescriptorPool(MainDevice.logical_device, &sampler_pool_create_info, nullptr, &sampler_descriptor_pool);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create a sampler descriptor pool!");

	}



}

void VulkanRenderer::create_gui_context()
{

	// UI
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	// Create Descriptor Pool
	VkDescriptorPoolSize gui_pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo gui_pool_info = {};
	gui_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	gui_pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	gui_pool_info.maxSets = 1000 * IM_ARRAYSIZE(gui_pool_sizes);
	gui_pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(gui_pool_sizes);
	gui_pool_info.pPoolSizes = gui_pool_sizes;

	VkResult result = vkCreateDescriptorPool(MainDevice.logical_device, &gui_pool_info, nullptr, &gui_descriptor_pool);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create a gui descriptor pool!");

	}

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForVulkan(window->get_window(), true);

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = instance;
	init_info.PhysicalDevice = MainDevice.physical_device;
	init_info.Device = MainDevice.logical_device;
	init_info.QueueFamily = get_queue_families(MainDevice.physical_device).graphics_family;
	init_info.Queue = graphics_queue;
	init_info.DescriptorPool = gui_descriptor_pool;
	init_info.MinImageCount = MAX_FRAME_DRAWS;
	init_info.PipelineCache = VK_NULL_HANDLE;																					// we do not need those 
	init_info.ImageCount = MAX_FRAME_DRAWS;
	init_info.Allocator = VK_NULL_HANDLE;
	init_info.CheckVkResultFn = VK_NULL_HANDLE;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&init_info, render_pass);

}

void VulkanRenderer::create_descriptor_sets()
{

	// resize descriptor set list so one for every buffer
	descriptor_sets.resize(swap_chain_images.size());

	std::vector<VkDescriptorSetLayout> set_layouts(swap_chain_images.size(), descriptor_set_layout);

	// descriptor set allocation info
	VkDescriptorSetAllocateInfo set_alloc_info{};
	set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	set_alloc_info.descriptorPool = descriptor_pool;																										// pool to allocate descriptor set from
	set_alloc_info.descriptorSetCount = static_cast<uint32_t>(swap_chain_images.size());									// number of sets to allocate
	set_alloc_info.pSetLayouts = set_layouts.data();																										// layouts to use to allocate sets (1:1 relationship)
	
	// allocate descriptor sets (multiple)
	VkResult result = vkAllocateDescriptorSets(MainDevice.logical_device, &set_alloc_info, descriptor_sets.data());

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create descriptor sets!");

	}

	// update all of descriptor set buffer bindings
	for (size_t i = 0; i < swap_chain_images.size(); i++) {

		// VIEW PROJECTION DESCRIPTOR
		// buffer info and data offset info
		VkDescriptorBufferInfo ubo_view_projection_buffer_info{};
		ubo_view_projection_buffer_info.buffer = vp_uniform_buffer[i];																					// buffer to get data from 
		ubo_view_projection_buffer_info.offset = 0;																													// position of start of data
		ubo_view_projection_buffer_info.range = sizeof(ubo_view_projection);																	// size of data

		// data about connection between binding and buffer
		VkWriteDescriptorSet ubo_view_projection_set_write{};
		ubo_view_projection_set_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		ubo_view_projection_set_write.dstSet = descriptor_sets[i];																						// descriptor set to update 
		ubo_view_projection_set_write.dstBinding = 0;																												// binding to update (matches with binding on layout/shader)
		ubo_view_projection_set_write.dstArrayElement = 0;																									// index in array to update
		ubo_view_projection_set_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;							// type of descriptor
		ubo_view_projection_set_write.descriptorCount = 1;																									// amount to update
		ubo_view_projection_set_write.pBufferInfo = &ubo_view_projection_buffer_info;													// information about buffer data to bind

		// MODEL DESCRIPTOR 
		// model buffer binding info
		//VkDescriptorBufferInfo ubo_model_buffer_info{};
		//ubo_model_buffer_info.buffer = model_dynamic_uniform_buffer[i];																			// buffer to get data from 
		//ubo_model_buffer_info.offset = 0;																																		// position of start of data
		//ubo_model_buffer_info.range = model_uniform_alignment;																							// size of data

		//// data about connection between binding and buffer
		//VkWriteDescriptorSet ubo_model_set_write{};
		//ubo_model_set_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//ubo_model_set_write.dstSet = descriptor_sets[i];																											// descriptor set to update 
		//ubo_model_set_write.dstBinding = 1;																																// binding to update (matches with binding on layout/shader)
		//ubo_model_set_write.dstArrayElement = 0;																													// index in array to update
		//ubo_model_set_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;							// type of descriptor
		//ubo_model_set_write.descriptorCount = 1;																													// amount to update
		//ubo_model_set_write.pBufferInfo = &ubo_model_buffer_info;																					// information about buffer data to bind

		std::vector<VkWriteDescriptorSet> write_descriptor_sets = { ubo_view_projection_set_write 
																													/*, ubo_model_set_write*/};

		// update the descriptor sets with new buffer/binding info
		vkUpdateDescriptorSets(MainDevice.logical_device, static_cast<uint32_t>(write_descriptor_sets.size()),
													write_descriptor_sets.data(), 0, nullptr);
	}

}

void VulkanRenderer::update_uniform_buffers(uint32_t image_index)
{
	// copy VP data 
	void* data;
	vkMapMemory(MainDevice.logical_device, vp_uniform_buffer_memory[image_index], 0, sizeof(UboViewProjection), 0, &data);
	memcpy(data, &ubo_view_projection, sizeof(UboViewProjection));
	vkUnmapMemory(MainDevice.logical_device, vp_uniform_buffer_memory[image_index]);

	// copy Model data
	//for (size_t i = 0; i < meshes.size(); i++) {

	//	Model* thisModel = (Model*)((uint64_t)model_transfer_space + (i * model_uniform_alignment));
	//	*thisModel = meshes[i].get_model();

	//}

	//// map the list of model data
	//vkMapMemory(MainDevice.logical_device, model_dynamic_uniform_buffer_memory[image_index], 0,
	//							model_uniform_alignment * meshes.size(), 0, &data);
	//memcpy(data, model_transfer_space, model_uniform_alignment * meshes.size());
	//vkUnmapMemory(MainDevice.logical_device, model_dynamic_uniform_buffer_memory[image_index]);

}

void VulkanRenderer::record_commands(uint32_t current_image)
{

	// information about how to begin each command buffer
	VkCommandBufferBeginInfo buffer_begin_info{};
	buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	// this falg is not longer needed because we synchronize with fences and semaphores
	// buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;					// buffer can be resubmitted when it has already been submitted and is awaiting execution
	
	// information about how to begin a render pass (only needed for graphical applications)
	VkRenderPassBeginInfo render_pass_begin_info{};
	render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.renderPass = render_pass;																				// render pass to begin 
	render_pass_begin_info.renderArea.offset = {0,0};																				// start point of render pass in pixels 
	render_pass_begin_info.renderArea.extent = swap_chain_extent;													// size of region to run render pass on (starting at offset)

	// make sure the order you put the values into the array matches with the attchment order you have defined previous
	std::array<VkClearValue, 2> clear_values = {};
	clear_values[0].color = {0.6f, 0.65f,0.4f, 1.0f};
	clear_values[1].depthStencil.depth = 1.0f;

	render_pass_begin_info.pClearValues = clear_values.data();
	render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());


	render_pass_begin_info.framebuffer = swap_chain_framebuffers[current_image];												// used framebuffer depends on the swap chain and therefore is changing for each command buffer

	// start recording commands to command buffer
	VkResult result = vkBeginCommandBuffer(command_buffers[current_image], &buffer_begin_info);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to start recording a command buffer!");

	}

	// begin render pass
	vkCmdBeginRenderPass(command_buffers[current_image], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

	// bind pipeline to be used in render pass
	vkCmdBindPipeline(command_buffers[current_image], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

	for (size_t m = 0; m < model_list.size(); m++) {

		// for GCC doen't allow references on rvalues go like that ... 
		glm::mat4 temp_model = model_list[m].get_model();
		// just "Push" constants to given shader stage directly (no buffer)
		vkCmdPushConstants(command_buffers[current_image],
											pipeline_layout,
											VK_SHADER_STAGE_VERTEX_BIT,				// stage to push constants to 
											0,																	// offset to push constants to update
											sizeof(Model),												// size of data being pushed 
											&temp_model);							// using model of current mesh (can be array)

		for (size_t k = 0; k < model_list[m].get_mesh_count(); k++) {

			// list of vertex buffers we want to draw 
			VkBuffer vertex_buffers[] = { model_list[m].get_mesh(k)->get_vertex_buffer()};																						// buffers to bind 
			VkDeviceSize offsets[] = { 0 };																																				// offsets into buffers being bound
			vkCmdBindVertexBuffers(command_buffers[current_image], 0, 1, vertex_buffers, offsets);															// command to bind vertex buffer before drawing with them
		
			// bind mesh index buffer with 0 offset and using the uint32 type
			vkCmdBindIndexBuffer(command_buffers[current_image], model_list[m].get_mesh(k)->get_index_buffer(), 0, VK_INDEX_TYPE_UINT32);			// command to bind index buffer before drawing with them

			// danamic offset amount
			// uint32_t dynamic_offset = static_cast<uint32_t>(model_uniform_alignment) * static_cast<uint32_t>(m);

			std::array<VkDescriptorSet, 2> descriptor_set_group = {descriptor_sets[current_image], 
																										sampler_descriptor_sets[model_list[m].get_mesh(k)->get_texture_id()]/*,
																										gui_descriptor_sets[current_image] */};

			// bind descriptor sets 
			vkCmdBindDescriptorSets(command_buffers[current_image], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 
																									0, static_cast<uint32_t>(descriptor_set_group.size()), 
																									descriptor_set_group.data(), 0, nullptr);

			// execute pipeline
			vkCmdDrawIndexed(command_buffers[current_image], static_cast<uint32_t>(model_list[m].get_mesh(k)->get_index_count()), 1, 0, 0, 0);

		}

	}

	// Record dear imgui primitives into command buffer
	ImGui_ImplVulkan_RenderDrawData(gui_draw_data, command_buffers[current_image]);										// record data for drawing the GUI 

	// end render pass 
	vkCmdEndRenderPass(command_buffers[current_image]);

	// stop recording to command buffer
	result = vkEndCommandBuffer(command_buffers[current_image]);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to stop recording a command buffer!");

	}

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

	// get properties of our new device
	VkPhysicalDeviceProperties device_properties;
	vkGetPhysicalDeviceProperties(MainDevice.physical_device, &device_properties);

	// min_uniform_buffer_offset = device_properties.limits.minUniformBufferOffsetAlignment;

}

void VulkanRenderer::allocate_dynamic_buffer_transfer_space()
{

	// calculate alignment of model data
	//model_uniform_alignment = (sizeof(Model) + min_uniform_buffer_offset - 1) & ~(min_uniform_buffer_offset - 1);

	//// create space in memory to hold dynamic buffer that is aligned to our required alignment and holds MAX_OBJECTS
	//// it was impossible for MSVC to implement std::aligned_alloc; use MSVC specific function instead!
	//#if defined (_WIN32) 
	//	model_transfer_space = (Model*)_aligned_malloc(model_uniform_alignment * MAX_OBJECTS, model_uniform_alignment);
	//#elif defined (__linux__)
	//	model_transfer_space = (Model*) std::aligned_alloc(model_uniform_alignment, model_uniform_alignment * MAX_OBJECTS);
	//#endif	

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


	return indices.is_valid() && extensions_supported && swap_chain_valid && device_features.samplerAnisotropy;
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

bool VulkanRenderer::check_validation_layer_support()
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

VkFormat VulkanRenderer::choose_supported_format(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags feature_flags)
{

	// loop through options and find compatible one
	for (VkFormat format : formats) {

		// get properties for give format on this device
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(MainDevice.physical_device, format, &properties);

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


VkImage VulkanRenderer::create_image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags use_flags, VkMemoryPropertyFlags prop_flags, VkDeviceMemory* image_memory)
{

	// CREATE image
	// image creation info
	VkImageCreateInfo image_create_info{};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;													// type of image (1D, 2D, 3D)
	image_create_info.extent.width = width;																		// width if image extent
	image_create_info.extent.height = height;																	// height if image extent
	image_create_info.extent.depth = 1;																				// height if image extent
	image_create_info.mipLevels = 1;																					// number of mipmap levels 
	image_create_info.arrayLayers = 1;																				// number of levels in image array
	image_create_info.format = format;																				// format type of image 
	image_create_info.tiling = tiling;																					// tiling of image ("arranged" for optimal reading)
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;							// layout of image data on creation 
	image_create_info.usage = use_flags;																			// bit flags defining what image will be used for
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;											// number of samples for multisampling 
	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;							// whether image can be shared between queues

	VkImage image;
	VkResult result = vkCreateImage(MainDevice.logical_device, &image_create_info, nullptr, &image);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create an image!");

	}

	// CREATE memory for image
	// get memory requirements for a type of image
	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(MainDevice.logical_device, image, &memory_requirements);

	// allocate memory using image requirements and user defined properties
	VkMemoryAllocateInfo memory_alloc_info{};
	memory_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_alloc_info.allocationSize = memory_requirements.size;
	memory_alloc_info.memoryTypeIndex = find_memory_type_index(MainDevice.physical_device, memory_requirements.memoryTypeBits, 
																		prop_flags);
	
	result = vkAllocateMemory(MainDevice.logical_device, &memory_alloc_info, nullptr, image_memory);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to allocate memory!");

	}

	// connect memory to image
	vkBindImageMemory(MainDevice.logical_device, image, *image_memory, 0);

	return image;
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

int VulkanRenderer::create_texture_image(std::string filename)
{

	int width, height;
	VkDeviceSize image_size;
	stbi_uc* image_data = load_texture_file(filename, &width, &height, &image_size);

	// create staging buffer to hold loaded data, ready to copy to device
	VkBuffer image_staging_buffer;
	VkDeviceMemory image_staging_buffer_memory;
	create_buffer(MainDevice.physical_device, MainDevice.logical_device, image_size, 
							VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
							&image_staging_buffer, &image_staging_buffer_memory);

	// copy image data to staging buffer 
	void* data;
	vkMapMemory(MainDevice.logical_device, image_staging_buffer_memory, 0, image_size, 0, &data);
	memcpy(data, image_data, static_cast<size_t>(image_size));
	vkUnmapMemory(MainDevice.logical_device, image_staging_buffer_memory);

	// free original image data
	stbi_image_free(image_data);

	// create image to hold final texture
	VkImage texture_image;
	VkDeviceMemory texture_image_memory;
	texture_image = create_image(width, height, VK_FORMAT_R8G8B8A8_UNORM,
																				VK_IMAGE_TILING_OPTIMAL,
																				VK_IMAGE_USAGE_TRANSFER_DST_BIT |
																				VK_IMAGE_USAGE_SAMPLED_BIT, 
																				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
																				&texture_image_memory);

	// copy data to image
	// transition image to be DST for copy operation
	transition_image_layout(MainDevice.logical_device, graphics_queue, graphics_command_pool, texture_image, VK_IMAGE_LAYOUT_UNDEFINED,
												VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// copy data to image
	copy_image_buffer(MainDevice.logical_device, graphics_queue, graphics_command_pool, image_staging_buffer, 
									texture_image, width, height);

	// transition image to be shader readable for shader stage

	transition_image_layout(MainDevice.logical_device, graphics_queue, graphics_command_pool, texture_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
												VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	// add texture data to vector for reference
	texture_images.push_back(texture_image);
	texture_images_memory.push_back(texture_image_memory);

	// destroy staging buffers
	vkDestroyBuffer(MainDevice.logical_device, image_staging_buffer, nullptr);
	vkFreeMemory(MainDevice.logical_device, image_staging_buffer_memory, nullptr);

	// return index to element in vector
	return texture_images.size() - 1;
}

int VulkanRenderer::create_texture(std::string filename)
{
	// create texture image and get its location in array
	int texture_image_location = create_texture_image(filename);

	// create image view and add to list
	VkImageView image_view = create_image_view(texture_images[texture_image_location], VK_FORMAT_R8G8B8A8_UNORM, 
																					VK_IMAGE_ASPECT_COLOR_BIT);
	texture_image_views.push_back(image_view);

	// create texture descriptor set here
	int descriptor_location = create_texture_descriptor(image_view);

	return descriptor_location;
}

int VulkanRenderer::create_texture_descriptor(VkImageView texture_image)
{

	VkDescriptorSet descriptor_set;

	// descriptor set allocation info 
	VkDescriptorSetAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = sampler_descriptor_pool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = &sampler_set_layout;

	// allocte descriptor sets
	VkResult result = vkAllocateDescriptorSets(MainDevice.logical_device, &alloc_info, &descriptor_set);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to allocate texture descriptor sets!");

	}

	// texture image info
	VkDescriptorImageInfo image_info{};
	image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	image_info.imageView = texture_image;
	image_info.sampler = texture_sampler;

	// descriptor write info
	VkWriteDescriptorSet descriptor_write{};
	descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_write.dstSet = descriptor_set;
	descriptor_write.dstBinding = 0;
	descriptor_write.dstArrayElement = 0;
	descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptor_write.descriptorCount = 1;
	descriptor_write.pImageInfo = &image_info;

	// update new descriptor set
	vkUpdateDescriptorSets(MainDevice.logical_device, 1, &descriptor_write, 0, nullptr);

	// add descriptor set to list
	sampler_descriptor_sets.push_back(descriptor_set);

	// return descriptor set location
	return sampler_descriptor_sets.size() - 1;

}

int VulkanRenderer::create_mesh_model(std::string model_file)
{

	// import model "scene"
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(model_file, aiProcess_Triangulate |
																							aiProcess_FlipUVs | 
																							aiProcess_JoinIdenticalVertices);

	if (!scene) {

		throw std::runtime_error("Failed to load model! (" + model_file + ")");

	}
	
	// get vector of all materials with 1:1 ID placement
	std::vector<std::string> texture_names = MeshModel::load_materials(scene);

	// conversion from the materials list IDs to our descriptor array IDs
	std::vector<int> mat_to_tex(texture_names.size());

	// loop over texture_names and create textures for them
	for (size_t i = 0; i < texture_names.size(); i++) {

		// if material no texture set 0 to indicate no texture, texture 0 will be reserved for a default texture
		if (texture_names[i].empty()) {

			mat_to_tex[i] = 0;

		}
		else {

			// otherwise, create texture and set value to index of new texture
			mat_to_tex[i] = create_texture(texture_names[i]);

		}

	}

	// load in all our meshes
	std::vector<Mesh> model_meshes = MeshModel::load_node(MainDevice.physical_device, 
															MainDevice.logical_device, 
															graphics_queue, 
															graphics_command_pool, 
															scene->mRootNode, 
															scene, mat_to_tex);

	// create mesh model and add to list
	MeshModel mesh_model = MeshModel(model_meshes);
	model_list.push_back(mesh_model);

	return model_list.size() - 1;

}

stbi_uc* VulkanRenderer::load_texture_file(std::string file_name, int* width, int* height, VkDeviceSize* image_size)
{

	// number of channels image uses
	int channels;
	// load pixel data for image
	std::string file_loc = "../Resources/Textures/" + file_name;
	stbi_uc* image = stbi_load(file_loc.c_str(), width, height, &channels, STBI_rgb_alpha);

	if (!image) {

		throw std::runtime_error("Failed to load a texture file! (" + file_name + ")");

	}

	// calculate image size using given and known data
	*image_size = *width * *height * 4;

	return image;
}

void VulkanRenderer::clean_up()
{

	// wait until no actions being run on device before destroying
	vkDeviceWaitIdle(MainDevice.logical_device);
	
	for (size_t i = 0; i < model_list.size(); i++) {

		model_list[i].destroy_mesh_model();

	}

	vkDestroyDescriptorPool(MainDevice.logical_device, sampler_descriptor_pool, nullptr);
	vkDestroyDescriptorSetLayout(MainDevice.logical_device, sampler_set_layout, nullptr);

	vkDestroySampler(MainDevice.logical_device, texture_sampler, nullptr);

	for (size_t i = 0; i < texture_images.size(); i++) {

		vkDestroyImageView(MainDevice.logical_device, texture_image_views[i], nullptr);
		vkDestroyImage(MainDevice.logical_device, texture_images[i], nullptr);
		vkFreeMemory(MainDevice.logical_device, texture_images_memory[i], nullptr);

	}

	vkDestroyImageView(MainDevice.logical_device, depth_buffer_image_view, nullptr);
	vkDestroyImage(MainDevice.logical_device, depth_buffer_image, nullptr);
	vkFreeMemory(MainDevice.logical_device, depth_buffer_image_memory, nullptr);

	// free all space we allocated for all model matrices for each object in our scene
	/*#if defined (_WIN32) 
		_aligned_free(model_transfer_space);
	#elif defined (__linux__)
		std::free(model_transfer_space);
	#endif	*/

	vkDestroyDescriptorPool(MainDevice.logical_device, descriptor_pool, nullptr);
	vkDestroyDescriptorSetLayout(MainDevice.logical_device, descriptor_set_layout, nullptr);
	
	for (size_t i = 0; i < swap_chain_images.size(); i++) {

		vkDestroyBuffer(MainDevice.logical_device, vp_uniform_buffer[i], nullptr);
		vkFreeMemory(MainDevice.logical_device, vp_uniform_buffer_memory[i], nullptr);

		/*vkDestroyBuffer(MainDevice.logical_device, model_dynamic_uniform_buffer[i], nullptr);
		vkFreeMemory(MainDevice.logical_device, model_dynamic_uniform_buffer_memory[i], nullptr);*/

	}

	for (int i = 0; i < MAX_FRAME_DRAWS; i++) {

		vkDestroySemaphore(MainDevice.logical_device, render_finished[i], nullptr);
		vkDestroySemaphore(MainDevice.logical_device, image_available[i], nullptr);
		vkDestroyFence(MainDevice.logical_device, draw_fences[i], nullptr);

	}

	vkDestroyCommandPool(MainDevice.logical_device, graphics_command_pool, nullptr);

	for (auto framebuffer : swap_chain_framebuffers) {

		vkDestroyFramebuffer(MainDevice.logical_device, framebuffer, nullptr);

	}

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