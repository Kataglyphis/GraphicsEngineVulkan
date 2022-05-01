#include "VulkanRenderer.hpp"

#ifndef VMA_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#endif // !VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

VulkanRenderer::VulkanRenderer() : max_levels(std::numeric_limits<int>::max()), 
																	current_frame(0),
																	framebuffer_resized(false),
																	raytracing(false)
{
}

int VulkanRenderer::init(std::shared_ptr<MyWindow> window, std::shared_ptr<Scene> scene, glm::vec3 eye, float near_plane, float far_plane,
											float fov,
											glm::vec3 view_dir, bool raytracing) 

{

	this->window = window;

	this->near_plane = near_plane;
	this->far_plane = far_plane;
	this->fov = fov;

	ubo_view_projection = UboViewProjection{};
	ubo_directions = UboDirections{};
	pc_raster = PushConstantRaster{};

	ubo_view_projection.projection = glm::perspective(glm::radians(this->fov), (float) window->get_width() / (float)window->get_height(),
																	near_plane, far_plane);

	ubo_view_projection.view = glm::lookAt(eye, glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	ubo_directions.light_dir = glm::vec4(	directional_light_direction[0], 
											directional_light_direction[1], 
											directional_light_direction[2],
											1.0f);

	ubo_directions.view_dir = glm::vec4(view_dir,1.0f);
	ubo_directions.cam_pos = glm::vec4(eye,1.0f);

	// ----- Update scene we are working on 
	this->scene = scene;

	// -- RAYTRACING STUFF
	this->raytracing = raytracing;

	try {

		create_instance();
		create_surface();
		get_physical_device();
		create_logical_device();
		create_vma_allocator();
		create_command_pool();
		create_swap_chain();
		create_uniform_buffers();

		init_rasterizer();

		// init the offscreen render pass 
		create_offscreen_textures();
		create_offscreen_render_pass();
		create_offscreen_framebuffers();
		create_offscreen_graphics_pipeline();

		// all post
		create_post_renderpass();
		create_post_descriptor();
		update_post_descriptor_set();
		create_post_pipeline();
		create_framebuffers();
		// this texture is used if the wanted texture could not be loaded :))
		create_texture("../Resources/Textures/plain.png");

		//std::string modelFile = "../Resources/Model/crytek-sponza/sponza_triag.obj";
		//std::string modelFile = "../Resources/Model/Dinosaurs/dinosaurs.obj";
		std::string modelFile = "../Resources/Model/Pillum/PilumPainting_export.obj";
		//std::string modelFile = "../Resources/Model/sibenik/sibenik.obj";
		//std::string modelFile = "../Resources/Model/sportsCar/sportsCar.obj";
		//std::string modelFile = "../Resources/Model/StanfordDragon/dragon.obj";
		//std::string modelFile = "../Resources/Model/CornellBox/CornellBox-Sphere.obj";
		//std::string modelFile = "../Resources/Model/bunny/bunny.obj";
		//std::string modelFile = "../Resources/Model/buddha/buddha.obj";
		//std::string modelFile = "../Resources/Model/bmw/bmw.obj";
		//std::string modelFile = "../Resources/Model/testScene.obj";
		//std::string modelFile = "../Resources/Model/San_Miguel/san-miguel-low-poly.obj";
		create_model(modelFile);

		glm::mat4 dragon_model(1.0f);
		//dragon_model = glm::translate(dragon_model, glm::vec3(0.0f, -40.0f, -50.0f));
		dragon_model = glm::scale(dragon_model, glm::vec3(1000.0f, 1000.0f, 1000.0f));
		/*dragon_model = glm::rotate(dragon_model, glm::radians(-90.f), glm::vec3(1.0f, 0.0f, 0.0f));
		dragon_model = glm::rotate(dragon_model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));*/
		update_model(0, dragon_model);

		init_raytracing();

		create_gui();
		create_synchronization();

	}
	catch (const std::runtime_error& e) {

		printf("ERROR: %s\n", e.what());
		return EXIT_FAILURE;

	}

	return EXIT_SUCCESS;
}

void VulkanRenderer::init_rasterizer()
{
		create_render_pass();
		create_descriptor_set_layouts();
		create_push_constant_range();
		create_rasterizer_graphics_pipeline();
		create_depthbuffer_image();
		
		create_command_buffers();
		create_texture_sampler();
		create_descriptor_pool_uniforms();
		create_descriptor_pool_sampler();
		create_descriptor_sets();
}

void VulkanRenderer::init_raytracing() {

	create_raytracing_descriptor_pool();
	//create_descriptor_pool_object_description();

	create_object_description_buffer();

	create_BLAS();
	create_TLAS();

	create_raytracing_descriptor_set_layouts();
	create_raytracing_descriptor_sets();

	create_raytracing_pipeline();
	create_shader_binding_table();

}

void VulkanRenderer::update_model(int model_id, glm::mat4 new_model)
{

	if (model_id >= static_cast<int32_t>(this->scene->get_model_count()) || model_id < 0) {

		throw std::runtime_error("Wrong model id value!");

	}

	scene->update_model_matrix(new_model, model_id);

}

void VulkanRenderer::update_view(glm::mat4 view)
{

	ubo_view_projection.view = view;

}

void VulkanRenderer::update_light_direction(glm::vec3 light_dir)
{

	ubo_directions.light_dir = glm::vec4(light_dir, 1.0f);


}

void VulkanRenderer::update_raytracing(bool raytracing_on)
{

	this->raytracing = raytracing_on;

}

void VulkanRenderer::update_view_direction(glm::vec3 view_dir)
{
	ubo_directions.view_dir = glm::vec4(view_dir,1.0f);

}

void VulkanRenderer::update_cam_pos(glm::vec3 cam_pos)
{
	ubo_directions.cam_pos = glm::vec4(cam_pos,1.0f);
}

void VulkanRenderer::hot_reload_all_shader()
{

	// wait until no actions being run on device before destroying
	vkDeviceWaitIdle(MainDevice.logical_device);

	vkDestroyPipeline(MainDevice.logical_device, graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(MainDevice.logical_device, pipeline_layout, nullptr);
	create_rasterizer_graphics_pipeline();

	vkDestroyPipeline(MainDevice.logical_device, offscreen_graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(MainDevice.logical_device, offscreen_pipeline_layout, nullptr);
	create_offscreen_graphics_pipeline();

	vkDestroyPipeline(MainDevice.logical_device, post_graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(MainDevice.logical_device, post_pipeline_layout, nullptr);
	create_post_pipeline();

	vkDestroyPipeline(MainDevice.logical_device, raytracing_pipeline, nullptr);
	vkDestroyPipelineLayout(MainDevice.logical_device, raytracing_pipeline_layout, nullptr);
	create_raytracing_pipeline();

}

void VulkanRenderer::drawFrame()
{
	check_changed_framebuffer_size();

	 /*1. Get next available image to draw to and set something to signal when we're finished with the image  (a semaphore)
	 wait for given fence to signal (open) from last draw before continuing*/
	VkResult result = vkWaitForFences(MainDevice.logical_device, 1, &in_flight_fences[current_frame], VK_TRUE, std::numeric_limits<uint64_t>::max());

	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to wait for fences!");
	}

	// -- GET NEXT IMAGE --
	uint32_t image_index;
	result = vkAcquireNextImageKHR(MainDevice.logical_device, swapchain, std::numeric_limits<uint64_t>::max(), image_available[current_frame], VK_NULL_HANDLE, &image_index);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {

		recreate_swap_chain();
		return;

	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {

		throw std::runtime_error("Failed to acquire next image!");

	}

	//// check if previous frame is using this image (i.e. there is its fence to wait on)
	if (images_in_flight_fences[image_index] != VK_NULL_HANDLE) {
		vkWaitForFences(MainDevice.logical_device, 1, &images_in_flight_fences[image_index], VK_TRUE, UINT64_MAX);
	}

	 //mark the image as now being in use by this frame
	images_in_flight_fences[image_index] = in_flight_fences[current_frame];

	render_gui();

	//vkResetCommandBuffer(command_buffers[image_index], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

	VkCommandBufferBeginInfo buffer_begin_info{};
	buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	// start recording commands to command buffer
	result = vkBeginCommandBuffer(command_buffers[image_index], &buffer_begin_info);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to start recording a command buffer!");

	}

	update_uniform_buffers(image_index);

	if(raytracing) update_raytracing_descriptor_set(image_index);

	record_commands(image_index);

	// stop recording to command buffer
	result = vkEndCommandBuffer(command_buffers[image_index]);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to stop recording a command buffer!");

	}
	// 2. Submit command buffer to queue for execution, making sure it waits for the image to be signalled as available before drawing
	// and signals when it has finished rendering 
	// -- SUBMIT COMMAND BUFFER TO RENDER --
	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.waitSemaphoreCount = 1;													// number of semaphores to wait on
	submit_info.pWaitSemaphores = &image_available[current_frame];						// list of semaphores to wait on
	
	VkPipelineStageFlags wait_stages = {
	
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT /*|
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT |
		VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR*/

	};

	submit_info.pWaitDstStageMask = &wait_stages;										// stages to check semaphores at
	
	submit_info.commandBufferCount = 1;													// number of command buffers to submit
	submit_info.pCommandBuffers = &command_buffers[image_index];						// command buffer to submit
	submit_info.signalSemaphoreCount = 1;												// number of semaphores to signal
	submit_info.pSignalSemaphores = &render_finished[current_frame];					// semaphores to signal when command buffer finishes 

	result = vkResetFences(MainDevice.logical_device, 1, &in_flight_fences[current_frame]);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to reset fences!");
	}

	// submit command buffer to queue
	result = vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fences[current_frame]); 

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to submit command buffer to queue!");

	}

	// 3. Present image to screen when it has signalled finished rendering 
	// -- PRESENT RENDERED IMAGE TO SCREEN --
  	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;												// number of semaphores to wait on
	present_info.pWaitSemaphores = &render_finished[current_frame];						// semaphores to wait on
	present_info.swapchainCount = 1;													// number of swapchains to present to
	present_info.pSwapchains = &swapchain;												// swapchains to present images to 
	present_info.pImageIndices = &image_index;											// index of images in swapchain to present

	result = vkQueuePresentKHR(presentation_queue, &present_info);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {

		recreate_swap_chain();
		return;

	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {

		throw std::runtime_error("Failed to acquire next image!");

	}

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to submit to present queue!");

	}

	//vkFreeCommandBuffers(MainDevice.logical_device, graphics_command_pool, 1, &command_buffers[image_index]);

	current_frame = (current_frame + 1) % MAX_FRAME_DRAWS;
	 
	/*vkQueueWaitIdle(presentation_queue);
	vkQueueWaitIdle(graphics_queue);
	vkDeviceWaitIdle(MainDevice.logical_device);*/

}

void VulkanRenderer::create_instance()
{

	if (ENABLE_VALIDATION_LAYERS && !check_validation_layer_support()) {
		throw std::runtime_error("Validation layers requested, but not available!");
	}

	// info about app
	// most data doesn't affect program; is for developer convenience
	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "\__/ Epic Graphics from hell \__/";	// custom name of app
	app_info.applicationVersion = VK_MAKE_VERSION(1,0,0);				// custom version of app
	app_info.pEngineName = "Cataglyphis Renderer";						// custom engine name
	app_info.engineVersion = VK_MAKE_VERSION(1,0,3);					// custom engine version 
	app_info.apiVersion = VK_API_VERSION_1_3;							// the vulkan version

	// creation info for a VkInstance
	VkInstanceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;

	VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
	//add validation layers IF enabled to the creeate info struct
	if (ENABLE_VALIDATION_LAYERS) {

		uint32_t layerCount = 1;
		const char** layerNames = (const char**)malloc(sizeof(const char*) * layerCount);
		//layerNames[0] = "VK_LAYER_KHRONOS_validation";

		messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

		messengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
											VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
											VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

		messengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |	
										VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
										VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

		messengerCreateInfo.pfnUserCallback = debugCallback;

		//create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&messengerCreateInfo;

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
	uint32_t glfw_extensions_count = 0;										// GLFW may require multiple extensions
	const char** glfw_extensions;											// Extensions passed as array of cstrings, so need pointer(array) to pointer
																			// (the cstring)

	//set GLFW extensions
	glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);

	// Add GLFW extensions to list of extensions
	for (size_t i = 0; i < glfw_extensions_count; i++) {

		instance_extensions.push_back(glfw_extensions[i]);

	}

	if (ENABLE_VALIDATION_LAYERS) {

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

	if (ENABLE_VALIDATION_LAYERS) {

		PFN_vkCreateDebugUtilsMessengerEXT pvkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)
																																						vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		if (pvkCreateDebugUtilsMessengerEXT(instance, &messengerCreateInfo, NULL, &debug_messenger) == VK_SUCCESS) {
			printf("created debug messenger\n");
		}

	}


}

void VulkanRenderer::create_logical_device()
{

	// get the queue family indices for the chosen physical device
	QueueFamilyIndices indices = get_queue_families(MainDevice.physical_device);

	// vector for queue creation information and set for family indices
	std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
	std::set<int> queue_family_indices = {indices.graphics_family, indices.presentation_family, indices.compute_family};

	// Queue the logical device needs to create and info to do so (only 1 for now, will add more later!)
	for (int queue_family_index : queue_family_indices) {

		VkDeviceQueueCreateInfo queue_create_info{};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = queue_family_index;											// the index of the family to create a queue from
		queue_create_info.queueCount = 1;																	// number of queues to create
		float priority = 1.0f;
		queue_create_info.pQueuePriorities = &priority;														//Vulkan needs to know how to handle multiple queues, so decide priority (1 = highest)

		queue_create_infos.push_back(queue_create_info);

	}

	// -- ALL EXTENSION WE NEED

	VkPhysicalDeviceDescriptorIndexingFeatures indexing_features{};
	indexing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
	indexing_features.runtimeDescriptorArray = VK_TRUE;
	indexing_features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;

	VkPhysicalDeviceFeatures2 features2{};
	features2.pNext =&indexing_features;
	features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	features2.features.samplerAnisotropy = VK_TRUE;
	features2.features.shaderInt64 = VK_TRUE;
	features2.features.geometryShader = VK_TRUE;

	// -- NEEDED FOR QUERING THE DEVICE ADDRESS WHEN CREATING ACCELERATION STRUCTURES
	VkPhysicalDeviceBufferDeviceAddressFeaturesEXT buffer_device_address_features{};
	buffer_device_address_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT;
	buffer_device_address_features.pNext = &features2;
	buffer_device_address_features.bufferDeviceAddress = VK_TRUE;
	buffer_device_address_features.bufferDeviceAddressCaptureReplay = VK_FALSE;
	buffer_device_address_features.bufferDeviceAddressMultiDevice = VK_FALSE;

	// --ENABLE RAY TRACING PIPELINE
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_pipeline_features{};
	ray_tracing_pipeline_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
	ray_tracing_pipeline_features.pNext = &buffer_device_address_features;
	ray_tracing_pipeline_features.rayTracingPipeline = VK_TRUE;

	// -- ENABLE ACCELERATION STRUCTURES
	VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration_structure_features{};
	acceleration_structure_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
	acceleration_structure_features.pNext = &ray_tracing_pipeline_features;
	acceleration_structure_features.accelerationStructure = VK_TRUE;
	acceleration_structure_features.accelerationStructureCaptureReplay = VK_TRUE;
	acceleration_structure_features.accelerationStructureIndirectBuild = VK_FALSE;
	acceleration_structure_features.accelerationStructureHostCommands = VK_FALSE;
	acceleration_structure_features.descriptorBindingAccelerationStructureUpdateAfterBind = VK_FALSE;

	// -- PREPARE FOR HAVING MORE EXTENSION BECAUSE WE NEED RAYTRACING CAPABILITIES
	std::vector<const char*> extensions(device_extensions);

	// COPY ALL NECESSARY EXTENSIONS FOR RAYTRACING TO THE EXTENSION
	extensions.insert(extensions.begin(), device_extensions_for_raytracing.begin(),
											device_extensions_for_raytracing.end());


	// information to create logical device (sometimes called "device") 
	VkDeviceCreateInfo device_create_info{};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());		// number of queue create infos
	device_create_info.pQueueCreateInfos = queue_create_infos.data();								// list of queue create infos so device can create required queues
	device_create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());			// number of enabled logical device extensions
	device_create_info.ppEnabledExtensionNames = extensions.data();									// list of enabled logical device extensions 
	device_create_info.flags = 0;
	device_create_info.pEnabledFeatures = NULL;


	device_create_info.pNext = &acceleration_structure_features;
	//device_create_info.pNext = &features2;

	//// physical device features the logical device will be using 
	//VkPhysicalDeviceFeatures device_features{};
	//device_features.samplerAnisotropy = VK_TRUE;
	//device_create_info.pEnabledFeatures = &device_features;

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
	vkGetDeviceQueue(MainDevice.logical_device, indices.compute_family, 0, &compute_queue);

}

void VulkanRenderer::create_vma_allocator()
{
	allocator = Allocator(MainDevice.logical_device, MainDevice.physical_device, instance);
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
	swap_chain_create_info.surface = surface;																				// swapchain surface
	swap_chain_create_info.imageFormat = surface_format.format;																// swapchain format
	swap_chain_create_info.imageColorSpace = surface_format.colorSpace;														// swapchain color space
	swap_chain_create_info.presentMode = present_mode;																		// swapchain presentation mode 
	swap_chain_create_info.imageExtent = extent;																			// swapchain image extents
	swap_chain_create_info.minImageCount = image_count;																		// minimum images in swapchain
	swap_chain_create_info.imageArrayLayers = 1;																			// number of layers for each image in chain
	swap_chain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
																			| VK_IMAGE_USAGE_TRANSFER_DST_BIT;				// what attachment images will be used as 
	swap_chain_create_info.preTransform = swap_chain_details.surface_capabilities.currentTransform;							// transform to perform on swap chain images
	swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;												// dont do blending; everything opaque
	swap_chain_create_info.clipped = VK_TRUE;																				// of course activate clipping ! :) 
	
	// get queue family indices
	QueueFamilyIndices indices = get_queue_families(MainDevice.physical_device);
	
	// if graphics and presentation families are different then swapchain must let images be shared between families
	if (indices.graphics_family != indices.presentation_family) {

		uint32_t queue_family_indices[] = {
						(uint32_t)indices.graphics_family,
						(uint32_t)indices.presentation_family
		};

		swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;									// image share handling
		swap_chain_create_info.queueFamilyIndexCount = 2;														// number of queues to share images between
		swap_chain_create_info.pQueueFamilyIndices = queue_family_indices;										// array of queues to share between 

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

	swap_chain_images.clear();
	//swap_chain_images.resize(swapchain_image_count);

	for (size_t i = 0; i < images.size(); i++) {

		VkImage image = images[static_cast<uint32_t>(i)];
		// store image handle
		SwapChainImage swap_chain_image{};
		swap_chain_image.image = image;
		swap_chain_image.image_view = create_image_view(image, swap_chain_image_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		
		/*VkCommandBuffer cmdBuffer = begin_command_buffer(MainDevice.logical_device, graphics_command_pool);
		transition_image_layout_for_command_buffer(cmdBuffer, swap_chain_image.image, VK_IMAGE_LAYOUT_UNDEFINED, 
													VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 1, VK_IMAGE_ASPECT_COLOR_BIT);

		end_and_submit_command_buffer(MainDevice.logical_device, graphics_command_pool, graphics_queue, cmdBuffer);*/
		// add to swapchain image list 
		swap_chain_images.push_back(swap_chain_image);
		
	}

}

void VulkanRenderer::create_offscreen_graphics_pipeline()
{

	std::string rasterizer_shader_dir = "../Resources/Shader/rasterizer/";

	compile_shader(rasterizer_shader_dir, "shader.vert");
	compile_shader(rasterizer_shader_dir, "shader.frag");

	auto vertex_shader_code = read_file(get_shader_spv_dir(rasterizer_shader_dir, "shader.vert"));
	auto fragment_shader_code = read_file(get_shader_spv_dir(rasterizer_shader_dir, "shader.frag"));

	// build shader modules to link to graphics pipeline
	VkShaderModule vertex_shader_module = create_shader_module(vertex_shader_code);
	VkShaderModule fragment_shader_module = create_shader_module(fragment_shader_code);

	// shader stage creation information
	// vertex stage creation information
	VkPipelineShaderStageCreateInfo vertex_shader_create_info{};
	vertex_shader_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertex_shader_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertex_shader_create_info.module = vertex_shader_module;
	vertex_shader_create_info.pName = "main";																	// entry point into shader

	// fragment stage creation information
	VkPipelineShaderStageCreateInfo fragment_shader_create_info{};
	fragment_shader_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragment_shader_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragment_shader_create_info.module = fragment_shader_module;
	fragment_shader_create_info.pName = "main";																	// entry point into shader

	VkPipelineShaderStageCreateInfo shader_stages[] = { vertex_shader_create_info,
														fragment_shader_create_info };


	// how the data for a single vertex (including info such as position, color, texture coords, normals, etc) is as a whole 
	VkVertexInputBindingDescription binding_description{};
	binding_description.binding = 0;																												// can bind multiple streams of data, this defines which one 
	binding_description.stride = sizeof(Vertex);																									// size of a single vertex object
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;																					// how to move between data after each vertex.
																																																	// VK_VERTEX_INPUT_RATE_VERTEX : Move on to the next vertex
																																																	// VK_VERTEX_INPUT_RATE_INSTANCE : Move on to the next instance
	// how the data for an attribute is defined within a vertex
	std::array<VkVertexInputAttributeDescription, 4> attribute_describtions;

	// Position attribute
	attribute_describtions[0].binding = 0;																																// which binding the data is at (should be same as above)
	attribute_describtions[0].location = 0;																																// location in shader where data will be read from
	attribute_describtions[0].format = VK_FORMAT_R32G32B32_SFLOAT;																			// format data will take (also helps define size of data)
	attribute_describtions[0].offset = offsetof(Vertex, pos);																									// where this attribute is defined in the data for a single vertex

	// normal coord attribute
	attribute_describtions[1].binding = 0;																																// which binding the data is at (should be same as above)
	attribute_describtions[1].location = 1;																																// location in shader where data will be read from
	attribute_describtions[1].format = VK_FORMAT_R32G32B32_SFLOAT;																					// format data will take (also helps define size of data)
	attribute_describtions[1].offset = offsetof(Vertex, normal);
																				// where this attribute is defined in the data for a single vertex
	attribute_describtions[2].binding = 0;																																// which binding the data is at (should be same as above)
	attribute_describtions[2].location = 2;																																// location in shader where data will be read from
	attribute_describtions[2].format = VK_FORMAT_R32G32B32_SFLOAT;																					// format data will take (also helps define size of data)
	attribute_describtions[2].offset = offsetof(Vertex, color);

	// texture coord attribute
	attribute_describtions[3].binding = 0;																																// which binding the data is at (should be same as above)
	attribute_describtions[3].location = 3;																																// location in shader where data will be read from
	attribute_describtions[3].format = VK_FORMAT_R32G32_SFLOAT;																					// format data will take (also helps define size of data)
	attribute_describtions[3].offset = offsetof(Vertex, texture_coords);																				// where this attribute is defined in the data for a single vertex



	// CREATE PIPELINE
	// 1.) Vertex input 
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
	viewport.width = (float)swap_chain_extent.width;																										// width of viewport 
	viewport.height = (float)swap_chain_extent.height;																									// height of viewport
	viewport.minDepth = 0.0f;																																					// min framebuffer depth
	viewport.maxDepth = 1.0f;																																					// max framebuffer depth

	// create a scissor info struct
	VkRect2D scissor{};
	scissor.offset = { 0,0 };																																							// offset to use region from 
	scissor.extent = swap_chain_extent;																																// extent to describe region to use, starting at offset

	VkPipelineViewportStateCreateInfo viewport_state_create_info{};
	viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state_create_info.viewportCount = 1;
	viewport_state_create_info.pViewports = &viewport;
	viewport_state_create_info.scissorCount = 1;
	viewport_state_create_info.pScissors = &scissor;

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
	color_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
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
	VkResult result = vkCreatePipelineLayout(MainDevice.logical_device, &pipeline_layout_create_info, nullptr, &offscreen_pipeline_layout);

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
	graphics_pipeline_create_info.renderPass = offscreen_render_pass;															// renderpass description the pipeline is compatible with
	graphics_pipeline_create_info.subpass = 0;																					// subpass of renderpass to use with pipeline

	// pipeline derivatives : can create multiple pipelines that derive from one another for optimization
	graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;								// existing pipeline to derive from ...
	graphics_pipeline_create_info.basePipelineIndex = -1;																// or index of pipeline being created to derive from (in case creating multiple at once)

	// create graphics pipeline 
	result = vkCreateGraphicsPipelines(MainDevice.logical_device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &offscreen_graphics_pipeline);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create a graphics pipeline!");

	}

	// Destroy shader modules, no longer needed after pipeline created
	vkDestroyShaderModule(MainDevice.logical_device, vertex_shader_module, nullptr);
	vkDestroyShaderModule(MainDevice.logical_device, fragment_shader_module, nullptr);

}

void VulkanRenderer::create_offscreen_textures()
{
	
	offscreen_images.resize(swap_chain_images.size());

	VkCommandBuffer cmdBuffer = begin_command_buffer(MainDevice.logical_device, graphics_command_pool);

	for (int index = 0; index < swap_chain_images.size(); index++) {

		OffscreenTexture image{};
		image.image = create_image(swap_chain_extent.width, swap_chain_extent.height, 1,
									swap_chain_image_format/*offscreen_format*/,
									VK_IMAGE_TILING_OPTIMAL,
									VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
									| VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
									VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
									&image.image_memory);

		image.image_view = create_image_view(image.image, swap_chain_image_format/*offscreen_format*/,
										VK_IMAGE_ASPECT_COLOR_BIT, 1);

		// --- WE NEED A DIFFERENT LAYOUT FOR USAGE 
		transition_image_layout_for_command_buffer(cmdBuffer, image.image,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1, VK_IMAGE_ASPECT_COLOR_BIT);

		offscreen_images[index] = image;

	}

	end_and_submit_command_buffer(MainDevice.logical_device, graphics_command_pool, graphics_queue, cmdBuffer);

	VkFormat depth_format = choose_supported_format({ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT,  VK_FORMAT_D24_UNORM_S8_UINT },
													VK_IMAGE_TILING_OPTIMAL,
													VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	// create depth buffer image
	// MIP LEVELS: for depth texture we only want 1 level :)
	offscreen_depth_buffer_image = create_image(swap_chain_extent.width, swap_chain_extent.height, 1, depth_format, VK_IMAGE_TILING_OPTIMAL,
										VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
										VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &offscreen_depth_buffer_image_memory);

	// depth buffer image view 
	// MIP LEVELS: for depth texture we only want 1 level :)
	offscreen_depth_buffer_image_view = create_image_view(offscreen_depth_buffer_image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT |
																							VK_IMAGE_ASPECT_STENCIL_BIT, 1);

	// --- WE NEED A DIFFERENT LAYOUT FOR USAGE 
	VkCommandBuffer cmdBuffer2 = begin_command_buffer(MainDevice.logical_device, graphics_command_pool);
	transition_image_layout_for_command_buffer(cmdBuffer2, offscreen_depth_buffer_image,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1, VK_IMAGE_ASPECT_DEPTH_BIT |
																						VK_IMAGE_ASPECT_STENCIL_BIT);
	end_and_submit_command_buffer(MainDevice.logical_device, graphics_command_pool, graphics_queue, cmdBuffer2);
	


}

void VulkanRenderer::create_offscreen_render_pass()
{

	// Color attachment of render pass
	VkAttachmentDescription color_attachment{};
	color_attachment.format = swap_chain_image_format;//offscreen_format;													// format to use for attachment
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;												// number of samples to write for multisampling
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;									// describes what to do with attachment before rendering
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;								// describes what to do with attachment after rendering 
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;			// describes what to do with stencil before rendering
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;		// describes what to do with stencil after rendering

	// framebuffer data will be stored as an image, but images can be given different layouts 
	// to give optimal use for certain operations
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;				// image data layout before render pass starts
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;					// image data layout after render pass (to change to)

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
	subpass.pDepthStencilAttachment = &depth_attachment_reference;

	// need to determine when layout transitions occur using subpass dependencies
	std::array<VkSubpassDependency,1> subpass_dependencies{};
	// conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	// transition must happen after ....

	subpass_dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;						// subpass index (VK_SUBPASS_EXTERNAL = Special value meaning outside of renderpass)
	subpass_dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;	// pipeline stage 
	subpass_dependencies[0].srcAccessMask = 0;				// stage access mask (memory access)

	// but must happen before ...
	subpass_dependencies[0].dstSubpass = 0;
	subpass_dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpass_dependencies[0].dependencyFlags = 0;

	//subpass_dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;						// subpass index (VK_SUBPASS_EXTERNAL = Special value meaning outside of renderpass)
	//subpass_dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;	// pipeline stage 
	//subpass_dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;				// stage access mask (memory access)

	//// but must happen before ...
	//subpass_dependencies[0].dstSubpass = 0;
	//subpass_dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	//subpass_dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	//subpass_dependencies[0].dependencyFlags = 0;

	//subpass_dependencies[1].srcSubpass = 0;													// subpass index (VK_SUBPASS_EXTERNAL = Special value meaning outside of renderpass)
	//subpass_dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;	// pipeline stage 
	//subpass_dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;	// stage access mask (memory access)

	//// but must happen before ...
	//subpass_dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	//subpass_dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	//subpass_dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	//subpass_dependencies[1].dependencyFlags = 0;

	std::array<VkAttachmentDescription, 2> render_pass_attachments = { color_attachment, depth_attachment };

	// create info for render pass 
	VkRenderPassCreateInfo render_pass_create_info{};
	render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_create_info.attachmentCount = static_cast<uint32_t>(render_pass_attachments.size());
	render_pass_create_info.pAttachments = render_pass_attachments.data();
	render_pass_create_info.subpassCount = 1;
	render_pass_create_info.pSubpasses = &subpass;
	render_pass_create_info.dependencyCount = static_cast<uint32_t>(subpass_dependencies.size());
	render_pass_create_info.pDependencies = subpass_dependencies.data();

	VkResult result = vkCreateRenderPass(MainDevice.logical_device, &render_pass_create_info, nullptr, &offscreen_render_pass);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create render pass!");

	}

}

void VulkanRenderer::create_offscreen_framebuffers()
{

	// resize framebuffer size to equal swap chain image count
	offscreen_framebuffer.resize(swap_chain_images.size());

	for (size_t i = 0; i < offscreen_framebuffer.size(); i++) {

		std::array<VkImageView, 2> attachments = {
							offscreen_images[i].image_view,
							depth_buffer_image_view
		};

		VkFramebufferCreateInfo frame_buffer_create_info{};
		frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frame_buffer_create_info.renderPass = offscreen_render_pass;																				// render pass layout the framebuffer will be used with
		frame_buffer_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		frame_buffer_create_info.pAttachments = attachments.data();																// list of attachments (1:1 with render pass)
		frame_buffer_create_info.width = swap_chain_extent.width;																// framebuffer width
		frame_buffer_create_info.height = swap_chain_extent.height;																// framebuffer height
		frame_buffer_create_info.layers = 1;																											// framebuffer layer 

		VkResult result = vkCreateFramebuffer(MainDevice.logical_device, &frame_buffer_create_info, nullptr, &offscreen_framebuffer[i]);

		if (result != VK_SUCCESS) {

			throw std::runtime_error("Failed to create framebuffer!");

		}

	}

}

void VulkanRenderer::create_post_renderpass()
{

	// Color attachment of render pass
	VkAttachmentDescription color_attachment{};
	color_attachment.format = swap_chain_image_format;													// format to use for attachment
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;												// number of samples to write for multisampling
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;									// describes what to do with attachment before rendering
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;								// describes what to do with attachment after rendering 
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;		// describes what to do with stencil before rendering
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;		// describes what to do with stencil after rendering

	// framebuffer data will be stored as an image, but images can be given different layouts 
	// to give optimal use for certain operations
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;						// image data layout before render pass starts
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
	subpass.pDepthStencilAttachment = &depth_attachment_reference;

	// need to determine when layout transitions occur using subpass dependencies
	std::array<VkSubpassDependency, 2> subpass_dependencies;

	// conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	// transition must happen after ....
	subpass_dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;						// subpass index (VK_SUBPASS_EXTERNAL = Special value meaning outside of renderpass)
	subpass_dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;	// pipeline stage 
	subpass_dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;				// stage access mask (memory access)

	// but must happen before ...
	subpass_dependencies[0].dstSubpass = 0;
	subpass_dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	subpass_dependencies[0].dependencyFlags = 0;

	subpass_dependencies[1].srcSubpass = 0;													// subpass index (VK_SUBPASS_EXTERNAL = Special value meaning outside of renderpass)
	subpass_dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;	// pipeline stage 
	subpass_dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;	// stage access mask (memory access)

	// but must happen before ...
	subpass_dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpass_dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpass_dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpass_dependencies[1].dependencyFlags = 0;

	std::array<VkAttachmentDescription, 2> render_pass_attachments = { color_attachment, depth_attachment };

	// create info for render pass 
	VkRenderPassCreateInfo render_pass_create_info{};
	render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_create_info.attachmentCount = static_cast<uint32_t>(render_pass_attachments.size());
	render_pass_create_info.pAttachments = render_pass_attachments.data();
	render_pass_create_info.subpassCount = 1;
	render_pass_create_info.pSubpasses = &subpass;
	render_pass_create_info.dependencyCount = static_cast<uint32_t>(subpass_dependencies.size());
	render_pass_create_info.pDependencies = subpass_dependencies.data();

	VkResult result = vkCreateRenderPass(MainDevice.logical_device, &render_pass_create_info, nullptr, &post_render_pass);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create render pass!");

	}


}

void VulkanRenderer::create_post_pipeline()
{

	std::string post_shader_dir = "../Resources/Shader/post/";
	std::string post_vert_shader = "post.vert";
	std::string post_frag_shader = "post.frag";

	auto vertex_shader_code		= read_file(get_shader_spv_dir(post_shader_dir, post_vert_shader));
	auto fragment_shader_code	= read_file(get_shader_spv_dir(post_shader_dir, post_frag_shader));

	compile_shader(post_shader_dir, post_vert_shader);
	compile_shader(post_shader_dir, post_frag_shader);

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

	VkPipelineShaderStageCreateInfo shader_stages[] = { vertex_shader_create_info,
														fragment_shader_create_info };


	// how the data for a single vertex (including info such as position, color, texture coords, normals, etc) is as a whole 
	VkVertexInputBindingDescription binding_description{};
	binding_description.binding = 0;																																				// can bind multiple streams of data, this defines which one 
	binding_description.stride = sizeof(Vertex);																															// size of a single vertex object
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;																					// how to move between data after each vertex.
																																																	// VK_VERTEX_INPUT_RATE_VERTEX : Move on to the next vertex
																																																	// VK_VERTEX_INPUT_RATE_INSTANCE : Move on to the next instance
	// how the data for an attribute is defined within a vertex
	std::array<VkVertexInputAttributeDescription, 3> attribute_describtions;

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

	// normal coord attribute
	attribute_describtions[2].binding = 0;																																// which binding the data is at (should be same as above)
	attribute_describtions[2].location = 2;																																// location in shader where data will be read from
	attribute_describtions[2].format = VK_FORMAT_R32G32B32_SFLOAT;																					// format data will take (also helps define size of data)
	attribute_describtions[2].offset = offsetof(Vertex, normal);																				// where this attribute is defined in the data for a single vertex

	// CREATE PIPELINE
	// 1.) Vertex input 
	VkPipelineVertexInputStateCreateInfo vertex_input_create_info{};
	vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_create_info.vertexBindingDescriptionCount = 0;
	vertex_input_create_info.pVertexBindingDescriptions = nullptr;																	// list of vertex binding descriptions(data spacing/ stride information)
	vertex_input_create_info.vertexAttributeDescriptionCount = 0;
	vertex_input_create_info.pVertexAttributeDescriptions = nullptr;													// list of vertex attribute descriptions (data format and where to bind to/from)

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
	viewport.width = (float)swap_chain_extent.width;																										// width of viewport 
	viewport.height = (float)swap_chain_extent.height;																									// height of viewport
	viewport.minDepth = 0.0f;																																					// min framebuffer depth
	viewport.maxDepth = 1.0f;																																					// max framebuffer depth

	// create a scissor info struct
	VkRect2D scissor{};
	scissor.offset = { 0,0 };																																							// offset to use region from 
	scissor.extent = swap_chain_extent;																																// extent to describe region to use, starting at offset

	VkPipelineViewportStateCreateInfo viewport_state_create_info{};
	viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state_create_info.viewportCount = 1;
	viewport_state_create_info.pViewports = &viewport;
	viewport_state_create_info.scissorCount = 1;
	viewport_state_create_info.pScissors = &scissor;

	// RASTERIZER
	VkPipelineRasterizationStateCreateInfo rasterizer_create_info{};
	rasterizer_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer_create_info.depthClampEnable = VK_FALSE;																								// change if fragments beyond near/far plane are clipped (default) or clamped to plane
	rasterizer_create_info.rasterizerDiscardEnable = VK_FALSE;																						// you don't output anything to a framebuffer but just output data ! 
	rasterizer_create_info.polygonMode = VK_POLYGON_MODE_FILL;																			// how to handle filling points between vertices 
	rasterizer_create_info.lineWidth = 1.0f;																															// how thic lines should be when drawn
	rasterizer_create_info.cullMode = VK_CULL_MODE_NONE;																					// backface culling as standard
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
	color_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
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

	std::array<VkDescriptorSetLayout, 1> descriptor_set_layouts = { post_descriptor_set_layout };

	VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
	pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_create_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
	pipeline_layout_create_info.pSetLayouts = descriptor_set_layouts.data();
	pipeline_layout_create_info.pushConstantRangeCount = 1;
	pipeline_layout_create_info.pPushConstantRanges = &post_push_constant_range;

	// create pipeline layout
	VkResult result = vkCreatePipelineLayout(MainDevice.logical_device, &pipeline_layout_create_info, nullptr, &post_pipeline_layout);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create pipeline layout!");

	}

	// -- DEPTH STENCIL TESTING --
	VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info{};
	depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_create_info.depthTestEnable = VK_FALSE;																							// enable checking depth to determine fragment write
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
	graphics_pipeline_create_info.layout = post_pipeline_layout;																// pipeline layout pipeline should use 
	graphics_pipeline_create_info.renderPass = post_render_pass;															// renderpass description the pipeline is compatible with
	graphics_pipeline_create_info.subpass = 0;																					// subpass of renderpass to use with pipeline

	// pipeline derivatives : can create multiple pipelines that derive from one another for optimization
	graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;								// existing pipeline to derive from ...
	graphics_pipeline_create_info.basePipelineIndex = -1;																// or index of pipeline being created to derive from (in case creating multiple at once)

	// create graphics pipeline 
	result = vkCreateGraphicsPipelines(MainDevice.logical_device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &post_graphics_pipeline);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create a graphics pipeline!");

	}

	// Destroy shader modules, no longer needed after pipeline created
	vkDestroyShaderModule(MainDevice.logical_device, vertex_shader_module, nullptr);
	vkDestroyShaderModule(MainDevice.logical_device, fragment_shader_module, nullptr);

}

void VulkanRenderer::create_post_descriptor()
{

	// UNIFORM VALUES DESCRIPTOR SET LAYOUT
	//ubo_view_projection Binding info
	VkDescriptorSetLayoutBinding post_sampler_layout_binding{};
	post_sampler_layout_binding.binding = 0;													// binding point in shader (designated by binding number in shader)
	post_sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;							// type of descriptor (uniform, dynamic uniform, image sampler, etc)
	post_sampler_layout_binding.descriptorCount = 1;																							// number of descriptors for binding
	post_sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;											// we need to say at which shader we bind this uniform to
	post_sampler_layout_binding.pImmutableSamplers = nullptr;																			// for texture: can make sampler data unchangeable (immutable) by specifying in layout

	std::vector<VkDescriptorSetLayoutBinding> layout_bindings = { post_sampler_layout_binding };

	// create descriptor set layout with given bindings
	VkDescriptorSetLayoutCreateInfo layout_create_info{};
	layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_create_info.bindingCount = static_cast<uint32_t>(layout_bindings.size());														// only have 1 for the ubo_view_projection
	layout_create_info.pBindings = layout_bindings.data();																										// array of binding infos 

	// create descriptor set layout
	VkResult result = vkCreateDescriptorSetLayout(MainDevice.logical_device, &layout_create_info, nullptr, &post_descriptor_set_layout);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create descriptor set layout!");

	}

	VkDescriptorPoolSize post_pool_size{};
	post_pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	post_pool_size.descriptorCount = static_cast<uint32_t>(1);

	// list of pool sizes 
	std::vector<VkDescriptorPoolSize> descriptor_pool_sizes = { post_pool_size };

	VkDescriptorPoolCreateInfo pool_create_info{};
	pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_create_info.maxSets = static_cast<uint32_t>(swap_chain_images.size());												// maximum number of descriptor sets that can be created from pool
	pool_create_info.poolSizeCount = static_cast<uint32_t>(descriptor_pool_sizes.size());									// amount of pool sizes being passed
	pool_create_info.pPoolSizes = descriptor_pool_sizes.data();																			// pool sizes to create pool with

	// create descriptor pool
	result = vkCreateDescriptorPool(MainDevice.logical_device, &pool_create_info, nullptr, &post_descriptor_pool);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create a descriptor pool!");

	}

	// resize descriptor set list so one for every buffer
	post_descriptor_set.resize(swap_chain_images.size());

	std::vector<VkDescriptorSetLayout> set_layouts(swap_chain_images.size(), post_descriptor_set_layout);

	// descriptor set allocation info
	VkDescriptorSetAllocateInfo set_alloc_info{};
	set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	set_alloc_info.descriptorPool = post_descriptor_pool;																										// pool to allocate descriptor set from
	set_alloc_info.descriptorSetCount = static_cast<uint32_t>(swap_chain_images.size());									// number of sets to allocate
	set_alloc_info.pSetLayouts = set_layouts.data();																										// layouts to use to allocate sets (1:1 relationship)

	// allocate descriptor sets (multiple)
	result = vkAllocateDescriptorSets(MainDevice.logical_device, &set_alloc_info, post_descriptor_set.data());

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create descriptor sets!");

	}

}

void VulkanRenderer::update_post_descriptor_set()
{

	// update all of descriptor set buffer bindings
	for (size_t i = 0; i < swap_chain_images.size(); i++) {

		// texture image info
		VkDescriptorImageInfo image_info{};
		image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		image_info.imageView = offscreen_images[i].image_view;
		image_info.sampler = texture_sampler;

		// descriptor write info
		VkWriteDescriptorSet descriptor_write{};
		descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write.dstSet = post_descriptor_set[i];
		descriptor_write.dstBinding = 0;
		descriptor_write.dstArrayElement = 0;
		descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor_write.descriptorCount = 1;
		descriptor_write.pImageInfo = &image_info;

		// update new descriptor set
		vkUpdateDescriptorSets(MainDevice.logical_device, 1, &descriptor_write, 0, nullptr);

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
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;						// image data layout before render pass starts
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
	subpass.pDepthStencilAttachment = &depth_attachment_reference;

	// need to determine when layout transitions occur using subpass dependencies
	std::array<VkSubpassDependency, 1> subpass_dependencies;

	// conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	// transition must happen after ....
	subpass_dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;						// subpass index (VK_SUBPASS_EXTERNAL = Special value meaning outside of renderpass)
	subpass_dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;	// pipeline stage 
	subpass_dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;				// stage access mask (memory access)

	// but must happen before ...
	subpass_dependencies[0].dstSubpass = 0;
	subpass_dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpass_dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

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

	VkCommandBuffer command_buffer = begin_command_buffer(MainDevice.logical_device, graphics_command_pool);
	ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
	end_and_submit_command_buffer(MainDevice.logical_device, graphics_command_pool, graphics_queue, command_buffer);

	// wait until no actions being run on device before destroying
	vkDeviceWaitIdle(MainDevice.logical_device);
	//clear font textures from cpu data
	ImGui_ImplVulkan_DestroyFontUploadObjects();

}

void VulkanRenderer::render_gui()
{

	// Start the Dear ImGui frame
		// ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// ImGui::ShowDemoWindow();
	//ImGui::PushFont(roboto_medium);
	// render your GUI
	ImGui::Begin("GUI v1.1.2");

	if (ImGui::CollapsingHeader("Hot shader reload")) {

		if (ImGui::Button("All shader!")) {

			hot_reload_all_shader();

		}

	}

	ImGui::Separator();

	ImGui::Checkbox("Ray tracing", &raytracing);

	ImGui::Separator();

	//ImGui::ShowDemoWindow();

	if (ImGui::CollapsingHeader("Graphic Settings")) {

		if (ImGui::TreeNode("Directional Light")) {
			ImGui::Separator();
			ImGui::SliderFloat("Ambient intensity", &direcional_light_ambient_intensity, 0.0f, 50.0f);
			ImGui::Separator();
			// Edit a color (stored as ~4 floats)
			ImGui::ColorEdit3("Directional Light Color", directional_light_color);
			ImGui::Separator();
			ImGui::SliderFloat3("Light Direction", directional_light_direction, -1.f, 1.0f);

			ImGui::TreePop();
		}

	}

	ImGui::Separator();

	if (ImGui::CollapsingHeader("GUI Settings")) {

		ImGuiStyle& style = ImGui::GetStyle();

		if (ImGui::SliderFloat("Frame Rounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f")) {
			style.GrabRounding = style.FrameRounding; // Make GrabRounding always the same value as FrameRounding
		}
		{ bool border = (style.FrameBorderSize > 0.0f);  if (ImGui::Checkbox("FrameBorder", &border)) { style.FrameBorderSize = border ? 1.0f : 0.0f; } }
		ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f, "%.0f");
	}

	ImGui::Separator();

	if (ImGui::CollapsingHeader("KEY Bindings")) {

		ImGui::Text("WASD for moving Forward, backward and to the side\n QE for rotating ");

	}

	ImGui::Separator();

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();

}

void VulkanRenderer::object_to_VkGeometryKHR(Mesh* mesh, VkAccelerationStructureGeometryKHR& acceleration_structure_geometry, 
														VkAccelerationStructureBuildRangeInfoKHR& acceleration_structure_build_range_info) {

	// LOAD ALL NECESSARY FUNCTIONS STRAIGHT IN THE BEGINNING
	// all functionality from extensions has to be loaded in the beginning
	// we need a reference to the device location of our geometry laying on the graphics card
	// we already uploaded objects and created vertex and index buffers respectively
	PFN_vkGetBufferDeviceAddressKHR pvkGetBufferDeviceAddressKHR = (PFN_vkGetBufferDeviceAddressKHR)
		vkGetDeviceProcAddr(MainDevice.logical_device, "vkGetBufferDeviceAddress");

	// all starts with the address of our vertex and index data we already uploaded 
	// in buffers earlier when loading the meshes/models
	VkBufferDeviceAddressInfo vertex_buffer_device_address_info{};
	vertex_buffer_device_address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	vertex_buffer_device_address_info.buffer = mesh->get_vertex_buffer();
	vertex_buffer_device_address_info.pNext = nullptr;

	VkBufferDeviceAddressInfo index_buffer_device_address_info{};
	index_buffer_device_address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	index_buffer_device_address_info.buffer = mesh->get_index_buffer();
	index_buffer_device_address_info.pNext = nullptr;

	// receiving address to move on 
	VkDeviceAddress vertex_buffer_address = pvkGetBufferDeviceAddressKHR(MainDevice.logical_device, &vertex_buffer_device_address_info);
	VkDeviceAddress index_buffer_address = pvkGetBufferDeviceAddressKHR(MainDevice.logical_device, &index_buffer_device_address_info);

	// convert to const address for further processing
	VkDeviceOrHostAddressConstKHR  vertex_device_or_host_address_const{};
	vertex_device_or_host_address_const.deviceAddress = vertex_buffer_address;

	VkDeviceOrHostAddressConstKHR index_device_or_host_address_const{};
	index_device_or_host_address_const.deviceAddress = index_buffer_address;

	VkAccelerationStructureGeometryTrianglesDataKHR acceleration_structure_triangles_data{};
	acceleration_structure_triangles_data.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
	acceleration_structure_triangles_data.pNext = nullptr;
	acceleration_structure_triangles_data.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	acceleration_structure_triangles_data.vertexData = vertex_device_or_host_address_const;
	acceleration_structure_triangles_data.vertexStride = sizeof(Vertex);
	acceleration_structure_triangles_data.maxVertex = mesh->get_vertex_count();
	acceleration_structure_triangles_data.indexType = VK_INDEX_TYPE_UINT32;
	acceleration_structure_triangles_data.indexData = index_device_or_host_address_const;

	// can also be instances or AABBs; not covered here
	// but to identify as triangles put it ito these struct
	VkAccelerationStructureGeometryDataKHR acceleration_structure_geometry_data{};
	acceleration_structure_geometry_data.triangles = acceleration_structure_triangles_data;

	acceleration_structure_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	acceleration_structure_geometry.pNext = nullptr;
	acceleration_structure_geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	acceleration_structure_geometry.geometry = acceleration_structure_geometry_data;
	acceleration_structure_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

	// we have triangles so divide the number of vertices with 3!!
		// for our simple case a no brainer
		// take entire data to build BLAS
	acceleration_structure_build_range_info.primitiveCount = mesh->get_index_count() / 3; // number of indices is truly the stick point here
	acceleration_structure_build_range_info.primitiveOffset = 0;
	acceleration_structure_build_range_info.firstVertex = 0;
	acceleration_structure_build_range_info.transformOffset = 0;

}


void VulkanRenderer::create_acceleration_structure_infos_BLAS(BuildAccelerationStructure& build_as_structure, BlasInput& blas_input,
															VkDeviceSize& current_scretch_size, VkDeviceSize& current_size)
{

	PFN_vkGetAccelerationStructureBuildSizesKHR pvkGetAccelerationStructureBuildSizesKHR = (PFN_vkGetAccelerationStructureBuildSizesKHR)
		vkGetDeviceProcAddr(MainDevice.logical_device, "vkGetAccelerationStructureBuildSizesKHR");


	build_as_structure.build_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	build_as_structure.build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	build_as_structure.build_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	build_as_structure.build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	build_as_structure.build_info.geometryCount = static_cast<uint32_t>(blas_input.as_geometry.size());
	build_as_structure.build_info.pGeometries = blas_input.as_geometry.data();

	build_as_structure.range_info = blas_input.as_build_offset_info.data();

	build_as_structure.size_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

	std::vector<uint32_t> max_primitive_cnt(blas_input.as_build_offset_info.size());
	
	for (int temp = 0; temp < blas_input.as_build_offset_info.size(); temp++)
		max_primitive_cnt[temp] = blas_input.as_build_offset_info[temp].primitiveCount;

	pvkGetAccelerationStructureBuildSizesKHR(MainDevice.logical_device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
																				&build_as_structure.build_info,
																				max_primitive_cnt.data(),
																				&build_as_structure.size_info);
	
	current_size = build_as_structure.size_info.accelerationStructureSize;
	current_scretch_size = build_as_structure.size_info.buildScratchSize;

}

void VulkanRenderer::create_single_blas(VkCommandBuffer command_buffer, BuildAccelerationStructure& build_as_structure, 
										VkDeviceAddress scratch_device_or_host_address)
{
	PFN_vkCreateAccelerationStructureKHR pvkCreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR)
		vkGetDeviceProcAddr(MainDevice.logical_device, "vkCreateAccelerationStructureKHR");

	PFN_vkCmdBuildAccelerationStructuresKHR pvkCmdBuildAccelerationStructuresKHR = (PFN_vkCmdBuildAccelerationStructuresKHR)
		vkGetDeviceProcAddr(MainDevice.logical_device, "vkCmdBuildAccelerationStructuresKHR");


	VkAccelerationStructureCreateInfoKHR acceleration_structure_create_info{};
	acceleration_structure_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	acceleration_structure_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	acceleration_structure_create_info.size = build_as_structure.size_info.accelerationStructureSize;

	create_buffer(MainDevice.physical_device, MainDevice.logical_device, build_as_structure.size_info.accelerationStructureSize, 
												VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | 
												VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
												VK_BUFFER_USAGE_TRANSFER_DST_BIT,
												VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &build_as_structure.single_blas.buffer, 
												&build_as_structure.single_blas.memory);

	acceleration_structure_create_info.buffer = build_as_structure.single_blas.buffer;
	pvkCreateAccelerationStructureKHR(MainDevice.logical_device, &acceleration_structure_create_info, nullptr, &build_as_structure.single_blas.accel);


	build_as_structure.build_info.dstAccelerationStructure = build_as_structure.single_blas.accel;
	build_as_structure.build_info.scratchData.deviceAddress = scratch_device_or_host_address;


	pvkCmdBuildAccelerationStructuresKHR(command_buffer, 1, &build_as_structure.build_info, &build_as_structure.range_info);


}

void VulkanRenderer::create_BLAS()
{

	// LOAD ALL NECESSARY FUNCTIONS STRAIGHT IN THE BEGINNING
	// all functionality from extensions has to be loaded in the beginning
	// we need a reference to the device location of our geometry laying on the graphics card
	// we already uploaded objects and created vertex and index buffers respectively

	PFN_vkCmdBuildAccelerationStructuresKHR pvkCmdBuildAccelerationStructuresKHR = (PFN_vkCmdBuildAccelerationStructuresKHR)
		vkGetDeviceProcAddr(MainDevice.logical_device, "vkCmdBuildAccelerationStructuresKHR");

	PFN_vkGetBufferDeviceAddressKHR pvkGetBufferDeviceAddressKHR = (PFN_vkGetBufferDeviceAddressKHR)
		vkGetDeviceProcAddr(MainDevice.logical_device, "vkGetBufferDeviceAddress");

	PFN_vkCreateAccelerationStructureKHR pvkCreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR)
		vkGetDeviceProcAddr(MainDevice.logical_device, "vkCreateAccelerationStructureKHR");

	std::vector<BlasInput> blas_input(scene->get_model_count());

	for (unsigned int model_index = 0; model_index < scene->get_model_count(); model_index++) {

		std::shared_ptr<Model> mesh_model = scene->get_model_list()[model_index];
		//blas_input.emplace_back();
		blas_input[model_index].as_geometry.reserve(mesh_model->get_mesh_count());
		blas_input[model_index].as_build_offset_info.reserve(mesh_model->get_mesh_count());

		for (size_t mesh_index = 0; mesh_index < mesh_model->get_mesh_count(); mesh_index++) {

			VkAccelerationStructureGeometryKHR acceleration_structure_geometry{};
			VkAccelerationStructureBuildRangeInfoKHR acceleration_structure_build_range_info{};

			object_to_VkGeometryKHR(mesh_model->get_mesh(mesh_index), acceleration_structure_geometry, acceleration_structure_build_range_info);
			// this only specifies the acceleration structure 
			// we are building it in the end for the whole model with the build command

			blas_input[model_index].as_geometry.push_back(acceleration_structure_geometry);
			blas_input[model_index].as_build_offset_info.push_back(acceleration_structure_build_range_info);

		}
	}


	std::vector<BuildAccelerationStructure> build_as_structures;
	build_as_structures.resize(scene->get_model_count());
		
	VkDeviceSize max_scratch_size = 0;
	VkDeviceSize total_size_all_BLAS = 0;

	for (unsigned int i = 0; i < scene->get_model_count(); i++) {

		VkDeviceSize current_scretch_size = 0;
		VkDeviceSize current_size = 0;

		create_acceleration_structure_infos_BLAS(build_as_structures[i], blas_input[i], current_scretch_size, current_size);

		total_size_all_BLAS += current_size;
		max_scratch_size = std::max(max_scratch_size, current_scretch_size);
	}

	VkBuffer scratch_buffer;
	VkDeviceMemory scratch_buffer_memory;

	create_buffer(MainDevice.physical_device, MainDevice.logical_device, max_scratch_size,
																VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
																VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
																VK_BUFFER_USAGE_TRANSFER_DST_BIT,
																VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT | 
																VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
																&scratch_buffer,
																&scratch_buffer_memory);

	VkBufferDeviceAddressInfo scratch_buffer_device_address_info{};
	scratch_buffer_device_address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	scratch_buffer_device_address_info.buffer = scratch_buffer;

	VkDeviceAddress scratch_buffer_address = pvkGetBufferDeviceAddressKHR(MainDevice.logical_device, &scratch_buffer_device_address_info);

	VkDeviceOrHostAddressKHR scratch_device_or_host_address{};
	scratch_device_or_host_address.deviceAddress = scratch_buffer_address;


	VkCommandBuffer command_buffer = begin_command_buffer(MainDevice.logical_device, compute_command_pool);

	for (size_t i = 0; i < scene->get_model_count(); i++) {
		
		create_single_blas(command_buffer, build_as_structures[i], scratch_buffer_address);

		VkMemoryBarrier barrier;
		barrier.pNext = nullptr;
		barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
		barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

		vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 
							VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr);

	}

	end_and_submit_command_buffer(MainDevice.logical_device, compute_command_pool, compute_queue, command_buffer);

	for (auto& b : build_as_structures) {
		blas.emplace_back(b.single_blas);
	}

	vkDestroyBuffer(MainDevice.logical_device, scratch_buffer, nullptr);
	vkFreeMemory(MainDevice.logical_device, scratch_buffer_memory, nullptr);

}

void VulkanRenderer::create_TLAS()
{

	// LOAD ALL NECESSARY FUNCTIONS STRAIGHT IN THE BEGINNING
	// all functionality from extensions has to be loaded in the beginning
	// we need a reference to the device location of our geometry laying on the graphics card
	// we already uploaded objects and created vertex and index buffers respectively
	PFN_vkGetAccelerationStructureBuildSizesKHR pvkGetAccelerationStructureBuildSizesKHR = (PFN_vkGetAccelerationStructureBuildSizesKHR)
																	vkGetDeviceProcAddr(MainDevice.logical_device, "vkGetAccelerationStructureBuildSizesKHR");

	PFN_vkCreateAccelerationStructureKHR pvkCreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR)
																	vkGetDeviceProcAddr(MainDevice.logical_device, "vkCreateAccelerationStructureKHR");

	PFN_vkGetBufferDeviceAddressKHR pvkGetBufferDeviceAddressKHR = (PFN_vkGetBufferDeviceAddressKHR)
																	vkGetDeviceProcAddr(MainDevice.logical_device, "vkGetBufferDeviceAddress");

	PFN_vkCmdBuildAccelerationStructuresKHR pvkCmdBuildAccelerationStructuresKHR = (PFN_vkCmdBuildAccelerationStructuresKHR)
																	vkGetDeviceProcAddr(MainDevice.logical_device, "vkCmdBuildAccelerationStructuresKHR");

	PFN_vkGetAccelerationStructureDeviceAddressKHR pvkGetAccelerationStructureDeviceAddressKHR = (PFN_vkGetAccelerationStructureDeviceAddressKHR)
																	vkGetDeviceProcAddr(MainDevice.logical_device, "vkGetAccelerationStructureDeviceAddressKHR");


	std::vector<VkAccelerationStructureInstanceKHR> tlas_instances;
	tlas_instances.reserve(scene->get_model_count());

	for (size_t model_index = 0; model_index < scene->get_model_count(); model_index++) {

		// glm uses column major matrices so transpose it for Vulkan want row major here
		glm::mat4 transpose_transform = glm::transpose(scene->get_model_matrix(static_cast<int>(model_index)));
		VkTransformMatrixKHR out_matrix;
		memcpy(&out_matrix, &transpose_transform, sizeof(VkTransformMatrixKHR));

		VkAccelerationStructureDeviceAddressInfoKHR acceleration_structure_device_address_info{};
		acceleration_structure_device_address_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		acceleration_structure_device_address_info.accelerationStructure = blas[model_index].accel;

		VkDeviceAddress acceleration_structure_device_address = pvkGetAccelerationStructureDeviceAddressKHR(MainDevice.logical_device, &acceleration_structure_device_address_info);

		VkAccelerationStructureInstanceKHR geometry_instance{};
		geometry_instance.transform = out_matrix;
		geometry_instance.instanceCustomIndex = model_index;					// gl_InstanceCustomIndexEXT
		geometry_instance.mask = 0xFF;
		geometry_instance.instanceShaderBindingTableRecordOffset = 0;
		geometry_instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		geometry_instance.accelerationStructureReference = acceleration_structure_device_address;
		geometry_instance.instanceShaderBindingTableRecordOffset = 0;										// same hit group for all objects

		tlas_instances.emplace_back(geometry_instance);
	}

	VkCommandBuffer command_buffer = begin_command_buffer(MainDevice.logical_device, compute_command_pool);

	VkBuffer geometry_instance_buffer;
	VkDeviceMemory geometry_instance_buffer_memory;

	create_geometry_instance_buffer(geometry_instance_buffer, geometry_instance_buffer_memory, tlas_instances);

	VkBufferDeviceAddressInfo geometry_instance_buffer_device_address_info{};
	geometry_instance_buffer_device_address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	geometry_instance_buffer_device_address_info.buffer = geometry_instance_buffer;

	VkDeviceAddress geometry_instance_buffer_address = pvkGetBufferDeviceAddressKHR(MainDevice.logical_device, &geometry_instance_buffer_device_address_info);

	// Make sure the copy of the instance buffer are copied before triggering the acceleration structure build
	VkMemoryBarrier barrier;
	barrier.pNext = nullptr;
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 
										0, 1, &barrier, 0, nullptr, 0, nullptr);

	VkAccelerationStructureGeometryInstancesDataKHR acceleration_structure_geometry_instances_data{};
	acceleration_structure_geometry_instances_data.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	acceleration_structure_geometry_instances_data.pNext = nullptr;
	acceleration_structure_geometry_instances_data.data.deviceAddress = geometry_instance_buffer_address;

	VkAccelerationStructureGeometryKHR topAS_acceleration_structure_geometry{};
	topAS_acceleration_structure_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	topAS_acceleration_structure_geometry.pNext = nullptr;
	topAS_acceleration_structure_geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	topAS_acceleration_structure_geometry.geometry.instances = acceleration_structure_geometry_instances_data;

	// find sizes
	VkAccelerationStructureBuildGeometryInfoKHR acceleration_structure_build_geometry_info{};
	acceleration_structure_build_geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	acceleration_structure_build_geometry_info.pNext = nullptr;
	acceleration_structure_build_geometry_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	acceleration_structure_build_geometry_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	acceleration_structure_build_geometry_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	acceleration_structure_build_geometry_info.srcAccelerationStructure = VK_NULL_HANDLE;
	acceleration_structure_build_geometry_info.geometryCount = 1;
	acceleration_structure_build_geometry_info.pGeometries = &topAS_acceleration_structure_geometry;

	VkAccelerationStructureBuildSizesInfoKHR acceleration_structure_build_sizes_info{};
	acceleration_structure_build_sizes_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	acceleration_structure_build_sizes_info.pNext = nullptr;
	acceleration_structure_build_sizes_info.accelerationStructureSize = 0;
	acceleration_structure_build_sizes_info.updateScratchSize = 0;
	acceleration_structure_build_sizes_info.buildScratchSize = 0;

	uint32_t count_instance = static_cast<uint32_t>(tlas_instances.size());
	pvkGetAccelerationStructureBuildSizesKHR(MainDevice.logical_device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_KHR, 
																						&acceleration_structure_build_geometry_info,
																						&count_instance,
																						&acceleration_structure_build_sizes_info);

	// now we got the sizes 
	create_buffer(MainDevice.physical_device, MainDevice.logical_device, acceleration_structure_build_sizes_info.accelerationStructureSize, 
																						VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
																						VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | 
																						VK_BUFFER_USAGE_TRANSFER_DST_BIT,
																						VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | 
																						VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, 
																						&tlas.top_level_acceleration_structure_buffer, 
																						&tlas.top_level_acceleration_structure_buffer_memory);

	VkAccelerationStructureCreateInfoKHR acceleration_structure_create_info{};
	acceleration_structure_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	acceleration_structure_create_info.pNext = nullptr;
	acceleration_structure_create_info.createFlags = 0;
	acceleration_structure_create_info.buffer = tlas.top_level_acceleration_structure_buffer;
	acceleration_structure_create_info.offset = 0;
	acceleration_structure_create_info.size = acceleration_structure_build_sizes_info.accelerationStructureSize;
	acceleration_structure_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	acceleration_structure_create_info.deviceAddress = 0;

	pvkCreateAccelerationStructureKHR(MainDevice.logical_device, &acceleration_structure_create_info, nullptr, &tlas.top_level_acceleration_structure);

	VkBuffer scratch_buffer;
	VkDeviceMemory scratch_buffer_memory;

	create_buffer(MainDevice.physical_device, MainDevice.logical_device, acceleration_structure_build_sizes_info.buildScratchSize,
																		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
																		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
																		VK_BUFFER_USAGE_TRANSFER_DST_BIT,
																		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
																		VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
																		&scratch_buffer,
																		&scratch_buffer_memory);

	VkBufferDeviceAddressInfo scratch_buffer_device_address_info{};
	scratch_buffer_device_address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	scratch_buffer_device_address_info.buffer = scratch_buffer;

	VkDeviceAddress scratch_buffer_address = pvkGetBufferDeviceAddressKHR(MainDevice.logical_device, &scratch_buffer_device_address_info);

	// update build info
	acceleration_structure_build_geometry_info.scratchData.deviceAddress = scratch_buffer_address;
	acceleration_structure_build_geometry_info.srcAccelerationStructure = VK_NULL_HANDLE;
	acceleration_structure_build_geometry_info.dstAccelerationStructure = tlas.top_level_acceleration_structure;


	VkAccelerationStructureBuildRangeInfoKHR acceleration_structure_build_range_info{};
	acceleration_structure_build_range_info.primitiveCount = scene->get_model_count();
	acceleration_structure_build_range_info.primitiveOffset = 0;
	acceleration_structure_build_range_info.firstVertex = 0;
	acceleration_structure_build_range_info.transformOffset = 0;
	
	VkAccelerationStructureBuildRangeInfoKHR* acceleration_structure_build_range_infos =
																			&acceleration_structure_build_range_info;

	pvkCmdBuildAccelerationStructuresKHR(command_buffer, 1, &acceleration_structure_build_geometry_info,
																								&acceleration_structure_build_range_infos);

	end_and_submit_command_buffer(MainDevice.logical_device, compute_command_pool, compute_queue, command_buffer);

	vkDestroyBuffer(MainDevice.logical_device, scratch_buffer, nullptr);
	vkFreeMemory(MainDevice.logical_device, scratch_buffer_memory, nullptr);

	vkDestroyBuffer(MainDevice.logical_device, geometry_instance_buffer, nullptr);
	vkFreeMemory(MainDevice.logical_device, geometry_instance_buffer_memory, nullptr);

}

void VulkanRenderer::create_geometry_instance_buffer(VkBuffer& geometry_instance_buffer, VkDeviceMemory& geometry_instance_buffer_memory,
														std::vector<VkAccelerationStructureInstanceKHR> tlas_instances)
{

	VkDeviceSize geometry_instance_buffer_size = sizeof(VkAccelerationStructureInstanceKHR) * tlas_instances.size();

	// temporary buffer to "stage" vertex data before transfering to GPU
	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;

	// create buffer and allocate memory to it
	create_buffer(MainDevice.physical_device, MainDevice.logical_device, geometry_instance_buffer_size, 
						VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						&staging_buffer, &staging_buffer_memory);

	// Map memory to vertex buffer
	void* data;																																			// 1.) create pointer to a point in normal memory
	vkMapMemory(MainDevice.logical_device, staging_buffer_memory, 0, geometry_instance_buffer_size, 0, &data);							// 2.) map the vertex buffer memory to that point
	memcpy(data, tlas_instances.data(), (size_t)geometry_instance_buffer_size);																// 3.) copy memory from vertices vector to the point
	vkUnmapMemory(MainDevice.logical_device, staging_buffer_memory);																	// 4.) unmap the vertex buffer memory

	// create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
	// buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by it and not CPU (host)
	create_buffer(MainDevice.physical_device, MainDevice.logical_device, geometry_instance_buffer_size, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
																| VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
																VK_BUFFER_USAGE_TRANSFER_DST_BIT,
																VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT | 
																VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
																&geometry_instance_buffer, &geometry_instance_buffer_memory);

	// copy staging buffer to vertex buffer on GPU
	copy_buffer(MainDevice.logical_device, compute_queue, compute_command_pool,
				staging_buffer, geometry_instance_buffer, geometry_instance_buffer_size);

	// clean up staging buffer parts
	vkDestroyBuffer(MainDevice.logical_device, staging_buffer, nullptr);
	vkFreeMemory(MainDevice.logical_device, staging_buffer_memory, nullptr);

}

void VulkanRenderer::create_raytracing_pipeline() {


	PFN_vkCreateRayTracingPipelinesKHR pvkCreateRayTracingPipelinesKHR =
								(PFN_vkCreateRayTracingPipelinesKHR)vkGetDeviceProcAddr(MainDevice.logical_device, "vkCreateRayTracingPipelinesKHR");

	std::string raytracing_shader_dir	= "../Resources/Shader/raytracing/";
	std::string raygen_shader			= "raytrace.rgen";
	std::string chit_shader				= "raytrace.rchit";
	std::string miss_shader				= "raytrace.rmiss";
	std::string shadow_shader			= "shadow.rmiss";

	compile_shader(raytracing_shader_dir, raygen_shader);
	compile_shader(raytracing_shader_dir, chit_shader);
	compile_shader(raytracing_shader_dir, miss_shader);
	compile_shader(raytracing_shader_dir, shadow_shader);

	auto raygen_shader_code		= read_file(get_shader_spv_dir(raytracing_shader_dir, raygen_shader));
	auto raychit_shader_code	= read_file(get_shader_spv_dir(raytracing_shader_dir, chit_shader));
	auto raymiss_shader_code	= read_file(get_shader_spv_dir(raytracing_shader_dir, miss_shader));
	auto shadow_shader_code		= read_file(get_shader_spv_dir(raytracing_shader_dir, shadow_shader));

	// build shader modules to link to graphics pipeline
	VkShaderModule raygen_shader_module		= create_shader_module(raygen_shader_code);
	VkShaderModule raychit_shader_module	= create_shader_module(raychit_shader_code);
	VkShaderModule raymiss_shader_module	= create_shader_module(raymiss_shader_code);
	VkShaderModule shadow_shader_module		= create_shader_module(shadow_shader_code);

	// create all shader stage infos for creating a group
	VkPipelineShaderStageCreateInfo rgen_shader_stage_info{};
	rgen_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	rgen_shader_stage_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	rgen_shader_stage_info.module = raygen_shader_module;
	rgen_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo rmiss_shader_stage_info{};
	rmiss_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	rmiss_shader_stage_info.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
	rmiss_shader_stage_info.module = raymiss_shader_module;
	rmiss_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo shadow_shader_stage_info{};
	shadow_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shadow_shader_stage_info.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
	shadow_shader_stage_info.module = shadow_shader_module;
	shadow_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo rchit_shader_stage_info{};
	rchit_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	rchit_shader_stage_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	rchit_shader_stage_info.module = raychit_shader_module;
	rchit_shader_stage_info.pName = "main";

	// we have all shader stages together
	std::array<VkPipelineShaderStageCreateInfo, 4> shader_stages = { rgen_shader_stage_info ,
																	rmiss_shader_stage_info ,
																	shadow_shader_stage_info ,
																	rchit_shader_stage_info };

	enum StageIndices {
		eRaygen,
		eMiss,
		eMiss2,
		eClosestHit,
		eShaderGroupCount
	};

	shader_groups.reserve(4);
	VkRayTracingShaderGroupCreateInfoKHR shader_group_create_infos[4];

	shader_group_create_infos[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
	shader_group_create_infos[0].pNext = nullptr;
	shader_group_create_infos[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	shader_group_create_infos[0].generalShader = eRaygen;
	shader_group_create_infos[0].closestHitShader = VK_SHADER_UNUSED_KHR;
	shader_group_create_infos[0].anyHitShader = VK_SHADER_UNUSED_KHR;
	shader_group_create_infos[0].intersectionShader = VK_SHADER_UNUSED_KHR;
	shader_group_create_infos[0].pShaderGroupCaptureReplayHandle = nullptr;

	shader_groups.push_back(shader_group_create_infos[0]);

	shader_group_create_infos[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
	shader_group_create_infos[1].pNext = nullptr;
	shader_group_create_infos[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	shader_group_create_infos[1].generalShader = eMiss;
	shader_group_create_infos[1].closestHitShader = VK_SHADER_UNUSED_KHR;
	shader_group_create_infos[1].anyHitShader = VK_SHADER_UNUSED_KHR;
	shader_group_create_infos[1].intersectionShader = VK_SHADER_UNUSED_KHR;
	shader_group_create_infos[1].pShaderGroupCaptureReplayHandle = nullptr;

	shader_groups.push_back(shader_group_create_infos[1]);

	shader_group_create_infos[2].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
	shader_group_create_infos[2].pNext = nullptr;
	shader_group_create_infos[2].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	shader_group_create_infos[2].generalShader = eMiss2;
	shader_group_create_infos[2].closestHitShader = VK_SHADER_UNUSED_KHR;
	shader_group_create_infos[2].anyHitShader = VK_SHADER_UNUSED_KHR;
	shader_group_create_infos[2].intersectionShader = VK_SHADER_UNUSED_KHR;
	shader_group_create_infos[2].pShaderGroupCaptureReplayHandle = nullptr;

	shader_groups.push_back(shader_group_create_infos[2]);

	shader_group_create_infos[3].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
	shader_group_create_infos[3].pNext = nullptr;
	shader_group_create_infos[3].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
	shader_group_create_infos[3].generalShader = VK_SHADER_UNUSED_KHR;
	shader_group_create_infos[3].closestHitShader = eClosestHit;
	shader_group_create_infos[3].anyHitShader = VK_SHADER_UNUSED_KHR;
	shader_group_create_infos[3].intersectionShader = VK_SHADER_UNUSED_KHR;
	shader_group_create_infos[3].pShaderGroupCaptureReplayHandle = nullptr;

	shader_groups.push_back(shader_group_create_infos[3]);

	std::vector<VkDescriptorSetLayout> layouts;
	layouts.push_back(descriptor_set_layout);
	layouts.push_back(raytracing_descriptor_set_layout);

	VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
	pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_create_info.setLayoutCount = static_cast<uint32_t>(layouts.size());
	pipeline_layout_create_info.pSetLayouts = layouts.data();
	pipeline_layout_create_info.pushConstantRangeCount = 1;
	pipeline_layout_create_info.pPushConstantRanges = &pc_ray_ranges;

	VkResult result = vkCreatePipelineLayout(MainDevice.logical_device, &pipeline_layout_create_info, nullptr, &raytracing_pipeline_layout);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create raytracing pipeline layout!");

	}

	VkPipelineLibraryCreateInfoKHR pipeline_library_create_info{};
	pipeline_library_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
	pipeline_library_create_info.pNext = nullptr;
	pipeline_library_create_info.libraryCount = 0;
	pipeline_library_create_info.pLibraries = nullptr;

	VkRayTracingPipelineCreateInfoKHR raytracing_pipeline_create_info{};
	raytracing_pipeline_create_info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
	raytracing_pipeline_create_info.pNext = nullptr;
	raytracing_pipeline_create_info.flags = 0;
	raytracing_pipeline_create_info.stageCount = static_cast<uint32_t>(shader_stages.size());
	raytracing_pipeline_create_info.pStages = shader_stages.data();
	raytracing_pipeline_create_info.groupCount = static_cast<uint32_t>(shader_groups.size());
	raytracing_pipeline_create_info.pGroups = shader_groups.data();
	/*raytracing_pipeline_create_info.pLibraryInfo = &pipeline_library_create_info;
	raytracing_pipeline_create_info.pLibraryInterface = NULL;*/
	// TODO: HARDCODED FOR NOW;
	raytracing_pipeline_create_info.maxPipelineRayRecursionDepth = 2;
	raytracing_pipeline_create_info.layout = raytracing_pipeline_layout;

	result = pvkCreateRayTracingPipelinesKHR(MainDevice.logical_device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, 
										&raytracing_pipeline_create_info, nullptr, &raytracing_pipeline);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create raytracing pipeline!");

	}

	vkDestroyShaderModule(MainDevice.logical_device, raygen_shader_module, nullptr);
	vkDestroyShaderModule(MainDevice.logical_device, raymiss_shader_module, nullptr);
	vkDestroyShaderModule(MainDevice.logical_device, raychit_shader_module, nullptr);
	vkDestroyShaderModule(MainDevice.logical_device, shadow_shader_module, nullptr);

}

void VulkanRenderer::create_shader_binding_table()
{

	// load in functionality for raytracing shader group handles
	PFN_vkGetRayTracingShaderGroupHandlesKHR pvkGetRayTracingShaderGroupHandlesKHR = (PFN_vkGetRayTracingShaderGroupHandlesKHR)
										vkGetDeviceProcAddr(MainDevice.logical_device, "vkGetRayTracingShaderGroupHandlesKHR");


	raytracing_properties = VkPhysicalDeviceRayTracingPipelinePropertiesKHR{};
	raytracing_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

	VkPhysicalDeviceProperties2 properties{};
	properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	properties.pNext = &raytracing_properties;

	vkGetPhysicalDeviceProperties2(MainDevice.physical_device, &properties);

	uint32_t handle_size = raytracing_properties.shaderGroupHandleSize;
	uint32_t handle_size_aligned = align_up(handle_size, raytracing_properties.shaderGroupHandleAlignment);

	uint32_t group_count = static_cast<uint32_t>(shader_groups.size());
	uint32_t sbt_size = group_count * handle_size_aligned;

	std::vector<uint8_t> handles(sbt_size);

	VkResult result = pvkGetRayTracingShaderGroupHandlesKHR(MainDevice.logical_device,
										raytracing_pipeline, 0, group_count, sbt_size, 
										handles.data());

	if(result != VK_SUCCESS) {
		
		throw std::runtime_error("Failed to get ray tracing shader group handles!");

	}

	const VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	const VkMemoryPropertyFlags memoryUsageFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	create_buffer(MainDevice.physical_device, MainDevice.logical_device, handle_size,
														bufferUsageFlags,
														memoryUsageFlags,
														&raygen_shader_binding_table_buffer,
														&raygen_shader_binding_table_buffer_memory);

	create_buffer(MainDevice.physical_device, MainDevice.logical_device, 2 * handle_size,
														bufferUsageFlags,
														memoryUsageFlags,
														&miss_shader_binding_table_buffer,
														&miss_shader_binding_table_buffer_memory);

	create_buffer(MainDevice.physical_device, MainDevice.logical_device, handle_size,
														bufferUsageFlags,
														memoryUsageFlags,
														&hit_shader_binding_table_buffer,
														&hit_shader_binding_table_buffer_memory);
	void* mapped_raygen = nullptr;
	vkMapMemory(MainDevice.logical_device, raygen_shader_binding_table_buffer_memory, 0, VK_WHOLE_SIZE, 0, &mapped_raygen);

	void* mapped_miss = nullptr;
	vkMapMemory(MainDevice.logical_device, miss_shader_binding_table_buffer_memory, 0, VK_WHOLE_SIZE, 0, &mapped_miss);

	void* mapped_rchit = nullptr;
	vkMapMemory(MainDevice.logical_device, hit_shader_binding_table_buffer_memory, 0, VK_WHOLE_SIZE, 0, &mapped_rchit);

	memcpy(mapped_raygen, handles.data(), handle_size);
	memcpy(mapped_miss, handles.data() + handle_size_aligned, handle_size * 2);
	memcpy(mapped_rchit, handles.data() + handle_size_aligned * 3, handle_size);

}

void VulkanRenderer::create_raytracing_descriptor_pool()
{

	std::array<VkDescriptorPoolSize,4> descriptor_pool_sizes;

	descriptor_pool_sizes[0].type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	descriptor_pool_sizes[0].descriptorCount = 1;

	descriptor_pool_sizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	descriptor_pool_sizes[1].descriptorCount = 1;

	descriptor_pool_sizes[2].type = VK_DESCRIPTOR_TYPE_SAMPLER;
	descriptor_pool_sizes[2].descriptorCount = 2;

	descriptor_pool_sizes[3].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	descriptor_pool_sizes[3].descriptorCount = MAX_TEXTURE_COUNT;

	VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
	descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_pool_create_info.poolSizeCount = static_cast<uint32_t>(descriptor_pool_sizes.size());
	descriptor_pool_create_info.pPoolSizes = descriptor_pool_sizes.data();
	descriptor_pool_create_info.maxSets = static_cast<uint32_t>(swap_chain_images.size());

	VkResult result = vkCreateDescriptorPool(MainDevice.logical_device, &descriptor_pool_create_info, nullptr, &raytracing_descriptor_pool);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create command pool!");

	}

}

void VulkanRenderer::create_object_description_buffer()
{

	VkDeviceSize buffer_size = sizeof(ObjectDescription) * scene->get_number_of_object_descriptions();

	// temporary buffer to "stage" index data before transfering to GPU
	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;

	// create buffer and allocate memory to it
	create_buffer(MainDevice.physical_device, MainDevice.logical_device, buffer_size,
												VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
												VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
												VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
												&staging_buffer, &staging_buffer_memory);

	// Map memory to index buffer
	void* data;																																			// 1.) create pointer to a point in normal memory
	vkMapMemory(MainDevice.logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);							// 2.) map the vertex buffer memory to that point
	memcpy(data, scene->get_object_descriptions().data(), (size_t)buffer_size);																	// 3.) copy memory from vertices vector to the point
	vkUnmapMemory(MainDevice.logical_device, staging_buffer_memory);																	// 4.) unmap the vertex buffer memory

	// create buffer for index data on GPU access only area
	// create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
	// buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by it and not CPU (host)
	create_buffer(MainDevice.physical_device, MainDevice.logical_device, buffer_size, 
												VK_BUFFER_USAGE_TRANSFER_DST_BIT |
												VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
												VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
												VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | 
												VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
												&object_description_buffer,
												&object_description_buffer_memory);

	// copy staging buffer to vertex buffer on GPU
	copy_buffer(MainDevice.logical_device, compute_queue, compute_command_pool, staging_buffer, object_description_buffer, buffer_size);

	// clean up staging buffer parts
	vkDestroyBuffer(MainDevice.logical_device, staging_buffer, nullptr);
	vkFreeMemory(MainDevice.logical_device, staging_buffer_memory, nullptr);

	// update the object description set
	// update all of descriptor set buffer bindings
	for (size_t i = 0; i < swap_chain_images.size(); i++) {

		VkDescriptorBufferInfo object_descriptions_buffer_info{};
		// image_info.sampler = VK_DESCRIPTOR_TYPE_SAMPLER;
		object_descriptions_buffer_info.buffer = object_description_buffer;
		object_descriptions_buffer_info.offset = 0;
		object_descriptions_buffer_info.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet descriptor_object_descriptions_writer{};
		descriptor_object_descriptions_writer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_object_descriptions_writer.pNext = nullptr;
		descriptor_object_descriptions_writer.dstSet = descriptor_sets[i];
		descriptor_object_descriptions_writer.dstBinding = OBJECT_DESCRIPTION_BINDING;
		descriptor_object_descriptions_writer.dstArrayElement = 0;
		descriptor_object_descriptions_writer.descriptorCount = 1;
		descriptor_object_descriptions_writer.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptor_object_descriptions_writer.pImageInfo = nullptr;
		descriptor_object_descriptions_writer.pBufferInfo = &object_descriptions_buffer_info;
		descriptor_object_descriptions_writer.pTexelBufferView = nullptr;																		// information about buffer data to bind

		std::vector<VkWriteDescriptorSet> write_descriptor_sets = { descriptor_object_descriptions_writer };

		// update the descriptor sets with new buffer/binding info
		vkUpdateDescriptorSets(MainDevice.logical_device, static_cast<uint32_t>(write_descriptor_sets.size()),
									write_descriptor_sets.data(), 0, nullptr);
	}

}

void VulkanRenderer::create_raytracing_descriptor_set_layouts() {

	{
		std::array<VkDescriptorSetLayoutBinding, 4> descriptor_set_layout_bindings;

		// here comes the top level acceleration structure
		descriptor_set_layout_bindings[0].binding = TLAS_BINDING;
		descriptor_set_layout_bindings[0].descriptorCount = 1;
		descriptor_set_layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		descriptor_set_layout_bindings[0].pImmutableSamplers = nullptr;
		// load them into the raygeneration and chlosest hit shader
		descriptor_set_layout_bindings[0].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR |
													VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		// here comes to previous rendered image
		descriptor_set_layout_bindings[1].binding = OUT_IMAGE_BINDING;
		descriptor_set_layout_bindings[1].descriptorCount = 1;
		descriptor_set_layout_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		descriptor_set_layout_bindings[1].pImmutableSamplers = nullptr;
		// load them into the raygeneration and chlosest hit shader
		descriptor_set_layout_bindings[1].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR |
														VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

		descriptor_set_layout_bindings[2].binding = TEXTURES_BINDING;
		descriptor_set_layout_bindings[2].descriptorCount = static_cast<uint32_t>(texture_images.size());
		descriptor_set_layout_bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		descriptor_set_layout_bindings[2].pImmutableSamplers = nullptr;
		// load them into the raygeneration and chlosest hit shader
		descriptor_set_layout_bindings[2].stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

		descriptor_set_layout_bindings[3].binding = SAMPLER_BINDING_RT;
		descriptor_set_layout_bindings[3].descriptorCount = 1;
		descriptor_set_layout_bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		descriptor_set_layout_bindings[3].pImmutableSamplers = nullptr;
		// load them into the raygeneration and chlosest hit shader
		descriptor_set_layout_bindings[3].stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
		descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptor_set_layout_create_info.bindingCount = static_cast<uint32_t>(descriptor_set_layout_bindings.size());
		descriptor_set_layout_create_info.pBindings = descriptor_set_layout_bindings.data();

		VkResult result = vkCreateDescriptorSetLayout(MainDevice.logical_device, &descriptor_set_layout_create_info, nullptr, &raytracing_descriptor_set_layout);

		if (result != VK_SUCCESS) {

			throw std::runtime_error("Failed to create raytracing descriptor set layout!");

		}
	}
	

}

void VulkanRenderer::create_raytracing_descriptor_sets()
{
	
	// resize descriptor set list so one for every buffer
	raytracing_descriptor_set.resize(swap_chain_images.size());

	std::vector<VkDescriptorSetLayout> set_layouts(swap_chain_images.size(), raytracing_descriptor_set_layout);

	VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
	descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;;
	descriptor_set_allocate_info.descriptorPool = raytracing_descriptor_pool;
	descriptor_set_allocate_info.descriptorSetCount = static_cast<uint32_t>(swap_chain_images.size());
	descriptor_set_allocate_info.pSetLayouts = set_layouts.data();

	VkResult result = vkAllocateDescriptorSets(MainDevice.logical_device, &descriptor_set_allocate_info, raytracing_descriptor_set.data());

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to allocate raytracing descriptor set!");

	}
		
	for (size_t i = 0; i < swap_chain_images.size(); i++) {

		VkWriteDescriptorSetAccelerationStructureKHR descriptor_set_acceleration_structure{};
		descriptor_set_acceleration_structure.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		descriptor_set_acceleration_structure.pNext = nullptr;
		descriptor_set_acceleration_structure.accelerationStructureCount = 1;
		descriptor_set_acceleration_structure.pAccelerationStructures = &tlas.top_level_acceleration_structure;

		VkWriteDescriptorSet write_descriptor_set_acceleration_structure{};
		write_descriptor_set_acceleration_structure.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_descriptor_set_acceleration_structure.pNext = &descriptor_set_acceleration_structure;
		write_descriptor_set_acceleration_structure.dstSet = raytracing_descriptor_set[i];
		write_descriptor_set_acceleration_structure.dstBinding = TLAS_BINDING;
		write_descriptor_set_acceleration_structure.dstArrayElement = 0;
		write_descriptor_set_acceleration_structure.descriptorCount = 1;
		write_descriptor_set_acceleration_structure.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		write_descriptor_set_acceleration_structure.pImageInfo = nullptr;
		write_descriptor_set_acceleration_structure.pBufferInfo = nullptr;
		write_descriptor_set_acceleration_structure.pTexelBufferView = nullptr;

		VkDescriptorImageInfo image_info{};
		image_info.imageView = offscreen_images[i].image_view;
		image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkWriteDescriptorSet descriptor_image_writer{};
		descriptor_image_writer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_image_writer.pNext = nullptr;
		descriptor_image_writer.dstSet = raytracing_descriptor_set[i];
		descriptor_image_writer.dstBinding = OUT_IMAGE_BINDING;
		descriptor_image_writer.dstArrayElement = 0;
		descriptor_image_writer.descriptorCount = 1;
		descriptor_image_writer.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		descriptor_image_writer.pImageInfo = &image_info;
		descriptor_image_writer.pBufferInfo = nullptr;
		descriptor_image_writer.pTexelBufferView = nullptr;

		// texture image info
		std::vector<VkDescriptorImageInfo> image_info_textures;
		image_info_textures.resize(texture_image_views.size());
		for (int i = 0; i < texture_image_views.size(); i++) {
			image_info_textures[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			image_info_textures[i].imageView = texture_image_views[i];
			image_info_textures[i].sampler = nullptr;
		}

		// descriptor write info
		VkWriteDescriptorSet textures_descriptions_writer{};
		textures_descriptions_writer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		textures_descriptions_writer.dstSet = raytracing_descriptor_set[i];
		textures_descriptions_writer.dstBinding = TEXTURES_BINDING;
		textures_descriptions_writer.dstArrayElement = 0;
		textures_descriptions_writer.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		textures_descriptions_writer.descriptorCount = static_cast<uint32_t>(image_info_textures.size());
		textures_descriptions_writer.pImageInfo = image_info_textures.data();

		VkDescriptorImageInfo sampler_info;
		sampler_info.imageView = nullptr;
		sampler_info.sampler = texture_sampler;

		// descriptor write info
		VkWriteDescriptorSet descriptor_write_sampler{};
		descriptor_write_sampler.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write_sampler.dstSet = raytracing_descriptor_set[i];
		descriptor_write_sampler.dstBinding = SAMPLER_BINDING_RT;
		descriptor_write_sampler.dstArrayElement = 0;
		descriptor_write_sampler.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		descriptor_write_sampler.descriptorCount = 1;
		descriptor_write_sampler.pImageInfo = &sampler_info;

		std::vector<VkWriteDescriptorSet> write_descriptor_sets = { write_descriptor_set_acceleration_structure,
																	descriptor_image_writer,
																	textures_descriptions_writer, 
																	descriptor_write_sampler };

		// update the descriptor sets with new buffer/binding info
		vkUpdateDescriptorSets(MainDevice.logical_device, static_cast<uint32_t>(write_descriptor_sets.size()),
			write_descriptor_sets.data(), 0, nullptr);

	}
	

}

void VulkanRenderer::create_descriptor_set_layouts()
{

	std::array<VkDescriptorSetLayoutBinding, 3> descriptor_set_layout_bindings;
	// UNIFORM VALUES DESCRIPTOR SET LAYOUT
	//ubo_view_projection Binding info
	descriptor_set_layout_bindings[0].binding = UBO_VIEW_PROJECTION_BINDING;													// binding point in shader (designated by binding number in shader)
	descriptor_set_layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;							// type of descriptor (uniform, dynamic uniform, image sampler, etc)
	descriptor_set_layout_bindings[0].descriptorCount = 1;																							// number of descriptors for binding
	descriptor_set_layout_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT |
													VK_SHADER_STAGE_RAYGEN_BIT_KHR;												// we need to say at which shader we bind this uniform to
	descriptor_set_layout_bindings[0].pImmutableSamplers = nullptr;																			// for texture: can make sampler data unchangeable (immutable) by specifying in layout

	// our model matrix which updates every frame for each object
	descriptor_set_layout_bindings[1].binding = UBO_DIRECTIONS_BINDING;
	descriptor_set_layout_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptor_set_layout_bindings[1].descriptorCount = 1;
	descriptor_set_layout_bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT |
										VK_SHADER_STAGE_FRAGMENT_BIT |
										VK_SHADER_STAGE_RAYGEN_BIT_KHR |
										VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	descriptor_set_layout_bindings[1].pImmutableSamplers = nullptr;

	descriptor_set_layout_bindings[2].binding = OBJECT_DESCRIPTION_BINDING;
	descriptor_set_layout_bindings[2].descriptorCount = 1;
	descriptor_set_layout_bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptor_set_layout_bindings[2].pImmutableSamplers = nullptr;
	// load them into the raygeneration and chlosest hit shader
	descriptor_set_layout_bindings[2].stageFlags =	VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT |
													VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

	// create descriptor set layout with given bindings
	VkDescriptorSetLayoutCreateInfo layout_create_info{};
	layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_create_info.bindingCount = static_cast<uint32_t>(descriptor_set_layout_bindings.size());														// only have 1 for the ubo_view_projection
	layout_create_info.pBindings = descriptor_set_layout_bindings.data();																										// array of binding infos 

	// create descriptor set layout
	VkResult result = vkCreateDescriptorSetLayout(MainDevice.logical_device, &layout_create_info, nullptr, &descriptor_set_layout);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create descriptor set layout!");

	}

	// CREATE TEXTURE SAMPLER DESCRIPTOR SET LAYOUT
	// texture binding info
	VkDescriptorSetLayoutBinding sampler_layout_binding{};
	sampler_layout_binding.binding = SAMPLER_BINDING;
	sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	sampler_layout_binding.descriptorCount = 1;
	sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT |
										VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	sampler_layout_binding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding sampled_image_layout_binding{};
	sampled_image_layout_binding.binding = TEXTURES_BINDING;
	sampled_image_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	sampled_image_layout_binding.descriptorCount = MAX_TEXTURE_COUNT;
	sampled_image_layout_binding.stageFlags =	VK_SHADER_STAGE_FRAGMENT_BIT |
												VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	sampled_image_layout_binding.pImmutableSamplers = nullptr;

	std::vector<VkDescriptorSetLayoutBinding> texture_layout_bindings = {	sampler_layout_binding,
																			sampled_image_layout_binding };

	// create a descriptor set layout with given bindings for texture 
	VkDescriptorSetLayoutCreateInfo texture_layout_create_info{};
	texture_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	texture_layout_create_info.bindingCount = static_cast<uint32_t>(texture_layout_bindings.size());
	texture_layout_create_info.pBindings = texture_layout_bindings.data();

	// create descriptor set layout
	result = vkCreateDescriptorSetLayout(MainDevice.logical_device, &texture_layout_create_info, nullptr, &sampler_set_layout);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create sampler set layout!");

	}

}

void VulkanRenderer::create_push_constant_range()
{

	// define push constant values (no 'create' needed)
	push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;														// shader stage push constant will go to 
	push_constant_range.offset = 0;																															// offset into given data to pass tp push constant
	push_constant_range.size = sizeof(PushConstantRaster);	
	
	// define push constant values (no 'create' needed)
	pc_ray_ranges.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR |
								VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
								VK_SHADER_STAGE_MISS_BIT_KHR;																// shader stage push constant will go to 
	pc_ray_ranges.offset = 0;																															// offset into given data to pass tp push constant
	pc_ray_ranges.size = sizeof(PushConstantRaytracing);	// size of data being passed

	post_push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT |
										VK_SHADER_STAGE_FRAGMENT_BIT;
	post_push_constant_range.offset = 0;
	post_push_constant_range.size = sizeof(PushConstantPost);

}

void VulkanRenderer::create_rasterizer_graphics_pipeline()
{
	std::string rasterizer_shader_dir	= "../Resources/Shader/rasterizer/";

	compile_shader(rasterizer_shader_dir, "shader.vert");
	compile_shader(rasterizer_shader_dir, "shader.frag");

	auto vertex_shader_code = read_file(get_shader_spv_dir(rasterizer_shader_dir, "shader.vert"));
	auto fragment_shader_code = read_file(get_shader_spv_dir(rasterizer_shader_dir, "shader.frag"));

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
	std::array<VkVertexInputAttributeDescription, 4> attribute_describtions;

	// Position attribute
	attribute_describtions[0].binding = 0;																																// which binding the data is at (should be same as above)
	attribute_describtions[0].location = 0;																																// location in shader where data will be read from
	attribute_describtions[0].format = VK_FORMAT_R32G32B32_SFLOAT;																			// format data will take (also helps define size of data)
	attribute_describtions[0].offset = offsetof(Vertex, pos);																									// where this attribute is defined in the data for a single vertex

	// normal coord attribute
	attribute_describtions[1].binding = 0;																																// which binding the data is at (should be same as above)
	attribute_describtions[1].location = 1;																																// location in shader where data will be read from
	attribute_describtions[1].format = VK_FORMAT_R32G32B32_SFLOAT;																					// format data will take (also helps define size of data)
	attribute_describtions[1].offset = offsetof(Vertex, normal);																				// where this attribute is defined in the data for a single vertex

	// normal coord attribute
	attribute_describtions[2].binding = 0;																																// which binding the data is at (should be same as above)
	attribute_describtions[2].location = 2;																																// location in shader where data will be read from
	attribute_describtions[2].format = VK_FORMAT_R32G32B32_SFLOAT;																					// format data will take (also helps define size of data)
	attribute_describtions[2].offset = offsetof(Vertex, color);

	// texture coord attribute
	attribute_describtions[3].binding = 0;																																// which binding the data is at (should be same as above)
	attribute_describtions[3].location = 3;																																// location in shader where data will be read from
	attribute_describtions[3].format = VK_FORMAT_R32G32_SFLOAT;																					// format data will take (also helps define size of data)
	attribute_describtions[3].offset = offsetof(Vertex, texture_coords);																				// where this attribute is defined in the data for a single vertex

	// CREATE PIPELINE
	// 1.) Vertex input 
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
	viewport.height = (float) swap_chain_extent.height;																									// height of viewport
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
	// MIP LEVELS: for depth texture we only want 1 level :)
	depth_buffer_image = create_image(swap_chain_extent.width, swap_chain_extent.height, 1, depth_format, VK_IMAGE_TILING_OPTIMAL,
																	VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
																	VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &depth_buffer_image_memory);

	// depth buffer image view 
	// MIP LEVELS: for depth texture we only want 1 level :)
	depth_buffer_image_view = create_image_view(depth_buffer_image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

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
		frame_buffer_create_info.renderPass = post_render_pass;																				// render pass layout the framebuffer will be used with
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

	{
	
		VkCommandPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;									// we are ready now to re-record our command buffers
		pool_info.queueFamilyIndex = queue_family_indices.graphics_family;														// queue family type that buffers from this command pool will use 

		// create a graphics queue family command pool
		VkResult result = vkCreateCommandPool(MainDevice.logical_device, &pool_info, nullptr, &graphics_command_pool);

		if(result != VK_SUCCESS) {

			throw std::runtime_error("Failed to create command pool!");

		}

	}

	{
	
		VkCommandPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;									// we are ready now to re-record our command buffers
		pool_info.queueFamilyIndex = queue_family_indices.compute_family;														// queue family type that buffers from this command pool will use 

		// create a graphics queue family command pool
		VkResult result = vkCreateCommandPool(MainDevice.logical_device, &pool_info, nullptr, &compute_command_pool);

		if (result != VK_SUCCESS) {

			throw std::runtime_error("Failed to create command pool!");

		}

	}
}

void VulkanRenderer::create_command_buffers()
{

	// resize command buffer count to have one for each framebuffer
	command_buffers.resize(swap_chain_images.size());

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
	in_flight_fences.resize(MAX_FRAME_DRAWS);
	images_in_flight_fences.resize(swap_chain_images.size(), VK_NULL_HANDLE);

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
			(vkCreateFence(MainDevice.logical_device, &fence_create_info, nullptr, &in_flight_fences[i])		!= VK_SUCCESS)
			){

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
	sampler_create_info.maxLod = (float) max_levels;
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
	VkDeviceSize directions_buffer_size = sizeof(ubo_directions);

	// one uniform buffer for each image (and by extension, command buffer)
	vp_uniform_buffer.resize(swap_chain_images.size());
	vp_uniform_buffer_memory.resize(swap_chain_images.size());
	directions_uniform_buffer.resize(swap_chain_images.size());
	directions_uniform_buffer_memory.resize(swap_chain_images.size());

	// temporary buffer to "stage" vertex data before transfering to GPU
	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;

	// create uniform buffers 
	for (size_t i = 0; i < swap_chain_images.size(); i++) {

		// -- VIEW PROJECTION UBO
		// create buffer and allocate memory to it
		create_buffer(MainDevice.physical_device, MainDevice.logical_device, vp_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&staging_buffer, &staging_buffer_memory);

		// Map memory to vertex buffer
		void* data;																																			// 1.) create pointer to a point in normal memory
		vkMapMemory(MainDevice.logical_device, staging_buffer_memory, 0, vp_buffer_size, 0, &data);							// 2.) map the vertex buffer memory to that point
		memcpy(data, &ubo_view_projection, (size_t)vp_buffer_size);																// 3.) copy memory from vertices vector to the point
		vkUnmapMemory(MainDevice.logical_device, staging_buffer_memory);																	// 4.) unmap the vertex buffer memory

		// create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
		// buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by it and not CPU (host)
		create_buffer(MainDevice.physical_device, MainDevice.logical_device, vp_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
			VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			&vp_uniform_buffer[i], &vp_uniform_buffer_memory[i]);

		// copy staging buffer to vertex buffer on GPU
		copy_buffer(MainDevice.logical_device, graphics_queue, graphics_command_pool, staging_buffer, vp_uniform_buffer[i], vp_buffer_size);

		// clean up staging buffer parts
		vkDestroyBuffer(MainDevice.logical_device, staging_buffer, nullptr);
		vkFreeMemory(MainDevice.logical_device, staging_buffer_memory, nullptr);

		// -- DIRECTIONS UBO
		// create buffer and allocate memory to it
		create_buffer(MainDevice.physical_device, MainDevice.logical_device, directions_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
								VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
								VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
								&staging_buffer, &staging_buffer_memory);

		// Map memory to vertex buffer																																	// 1.) create pointer to a point in normal memory
		vkMapMemory(MainDevice.logical_device, staging_buffer_memory, 0, directions_buffer_size, 0, &data);							// 2.) map the vertex buffer memory to that point
		memcpy(data, &ubo_directions, (size_t)directions_buffer_size);																// 3.) copy memory from vertices vector to the point
		vkUnmapMemory(MainDevice.logical_device, staging_buffer_memory);																	// 4.) unmap the vertex buffer memory

		// create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
		// buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by it and not CPU (host)
		create_buffer(MainDevice.physical_device, MainDevice.logical_device, directions_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
			VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			&directions_uniform_buffer[i], &directions_uniform_buffer_memory[i]);

		// copy staging buffer to vertex buffer on GPU
		copy_buffer(MainDevice.logical_device, graphics_queue, graphics_command_pool, staging_buffer, directions_uniform_buffer[i], directions_buffer_size);

		// clean up staging buffer parts
		vkDestroyBuffer(MainDevice.logical_device, staging_buffer, nullptr);
		vkFreeMemory(MainDevice.logical_device, staging_buffer_memory, nullptr);


	}
}

void VulkanRenderer::create_descriptor_pool_uniforms()
{

	// CREATE UNIFORM DESCRIPTOR POOL
	// type of descriptors + how many descriptors, not descriptor sets (combined makes the pool size)
	// ViewProjection Pool 
	VkDescriptorPoolSize vp_pool_size{};
	vp_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vp_pool_size.descriptorCount = static_cast<uint32_t>(vp_uniform_buffer.size());

	// DIRECTION POOL
	VkDescriptorPoolSize directions_pool_size{};
	directions_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	directions_pool_size.descriptorCount = static_cast<uint32_t>(directions_uniform_buffer.size());

	VkDescriptorPoolSize object_descriptions_pool_size{};
	object_descriptions_pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	object_descriptions_pool_size.descriptorCount = static_cast<uint32_t>(sizeof(ObjectDescription) * MAX_OBJECTS);

	// list of pool sizes 
	std::vector<VkDescriptorPoolSize> descriptor_pool_sizes = { vp_pool_size , directions_pool_size, object_descriptions_pool_size };

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

}

void VulkanRenderer::create_descriptor_pool_sampler()
{

	// CREATE SAMPLER DESCRIPTOR POOL
	// TEXTURE SAMPLER POOL
	VkDescriptorPoolSize sampler_pool_size{};
	sampler_pool_size.type = VK_DESCRIPTOR_TYPE_SAMPLER;
	sampler_pool_size.descriptorCount = 2;

	VkDescriptorPoolSize sampled_image_pool_size{};
	sampled_image_pool_size.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	sampled_image_pool_size.descriptorCount = MAX_TEXTURE_COUNT;

	std::vector<VkDescriptorPoolSize> descriptor_pool_sizes = { sampler_pool_size , sampled_image_pool_size };

	VkDescriptorPoolCreateInfo sampler_pool_create_info{};
	sampler_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	sampler_pool_create_info.maxSets = 1;
	sampler_pool_create_info.poolSizeCount = static_cast<uint32_t>(descriptor_pool_sizes.size());;
	sampler_pool_create_info.pPoolSizes = descriptor_pool_sizes.data();

	// create descriptor pool
	VkResult result = vkCreateDescriptorPool(MainDevice.logical_device, &sampler_pool_create_info, nullptr, &sampler_descriptor_pool);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create a sampler descriptor pool!");

	}

}

void VulkanRenderer::create_descriptor_pool_object_description()
{

	// CREATE SAMPLER DESCRIPTOR POOL
	// TEXTURE SAMPLER POOL
	VkDescriptorPoolSize object_description_pool_size{};
	object_description_pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	object_description_pool_size.descriptorCount = MAX_OBJECTS;

	VkDescriptorPoolCreateInfo object_description_pool_create_info{};
	object_description_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	object_description_pool_create_info.maxSets = 3;//MAX_OBJECTS;
	object_description_pool_create_info.poolSizeCount = 1;
	object_description_pool_create_info.pPoolSizes = &object_description_pool_size;

	// create descriptor pool
	VkResult result = vkCreateDescriptorPool(MainDevice.logical_device, &object_description_pool_create_info, nullptr, &object_description_pool);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create a object description descriptor pool!");

	}

}

void VulkanRenderer::create_gui()
{
	create_gui_context();
	create_fonts_and_upload();
}

void VulkanRenderer::create_gui_context()
{

	// UI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	float size_pixels = 18;
	io.Fonts->AddFontFromFileTTF("../ExternalLib/IMGUI/misc/fonts/Roboto-Medium.ttf", size_pixels);
	io.Fonts->AddFontFromFileTTF("../ExternalLib/IMGUI/misc/fonts/Cousine-Regular.ttf", size_pixels);
	io.Fonts->AddFontFromFileTTF("../ExternalLib/IMGUI/misc/fonts/DroidSans.ttf", size_pixels);
	io.Fonts->AddFontFromFileTTF("../ExternalLib/IMGUI/misc/fonts/Karla-Regular.ttf", size_pixels);
	io.Fonts->AddFontFromFileTTF("../ExternalLib/IMGUI/misc/fonts/ProggyClean.ttf", size_pixels);
	io.Fonts->AddFontFromFileTTF("../ExternalLib/IMGUI/misc/fonts/ProggyTiny.ttf", size_pixels);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	ImGui_ImplGlfw_InitForVulkan(window->get_window(), true);

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

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = instance;
	init_info.PhysicalDevice = MainDevice.physical_device;
	init_info.Device = MainDevice.logical_device;
	init_info.QueueFamily = get_queue_families(MainDevice.physical_device).graphics_family;
	init_info.Queue = graphics_queue;
	init_info.DescriptorPool = gui_descriptor_pool;
	init_info.PipelineCache = VK_NULL_HANDLE;																					// we do not need those 
	init_info.MinImageCount = MAX_FRAME_DRAWS;
	init_info.ImageCount = MAX_FRAME_DRAWS;
	init_info.Allocator = VK_NULL_HANDLE;
	init_info.CheckVkResultFn = VK_NULL_HANDLE;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&init_info, post_render_pass);

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

		// VIEW PROJECTION DESCRIPTOR
		// buffer info and data offset info
		VkDescriptorBufferInfo ubo_directions_buffer_info{};
		ubo_directions_buffer_info.buffer = directions_uniform_buffer[i];																							// buffer to get data from 
		ubo_directions_buffer_info.offset = 0;																																// position of start of data
		ubo_directions_buffer_info.range = sizeof(ubo_directions);																						// size of data

		// data about connection between binding and buffer
		VkWriteDescriptorSet ubo_directions_set_write{};
		ubo_directions_set_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		ubo_directions_set_write.dstSet = descriptor_sets[i];																									// descriptor set to update 
		ubo_directions_set_write.dstBinding = 1;																														// binding to update (matches with binding on layout/shader)
		ubo_directions_set_write.dstArrayElement = 0;																											// index in array to update
		ubo_directions_set_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;										// type of descriptor
		ubo_directions_set_write.descriptorCount = 1;																												// amount to update
		ubo_directions_set_write.pBufferInfo = &ubo_directions_buffer_info;																		// information about buffer data to bind

		std::vector<VkWriteDescriptorSet> write_descriptor_sets = { ubo_view_projection_set_write,
																	ubo_directions_set_write };

		// update the descriptor sets with new buffer/binding info
		vkUpdateDescriptorSets(MainDevice.logical_device, static_cast<uint32_t>(write_descriptor_sets.size()),
													write_descriptor_sets.data(), 0, nullptr);
	}

}

int VulkanRenderer::create_texture(std::string filename)
{

	// create texture image and get its location in array
	int texture_image_location = create_texture_image(filename);

	// create image view and add to list
	VkImageView image_view = create_image_view(texture_images[texture_image_location], VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_ASPECT_COLOR_BIT, texture_mip_levels[texture_image_location]);
	texture_image_views.push_back(image_view);

	// create texture descriptor set here
	// int descriptor_location = create_texture_descriptor(image_view);

	return texture_image_location;

}

int VulkanRenderer::create_texture_image(std::string filename)
{
	
	int width, height;
	VkDeviceSize image_size;
	stbi_uc* image_data = load_texture_file(filename, &width, &height, &image_size);

	// find the number of mip level we want to create 
	int mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

	// update the max level amount; we will need it later for creating the sampler
	max_levels = std::min(max_levels, mip_levels);

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
	texture_image = create_image(width, height, mip_levels, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
		VK_IMAGE_USAGE_TRANSFER_DST_BIT |
		VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&texture_image_memory);

	// copy data to image
	// transition image to be DST for copy operation
	transition_image_layout(MainDevice.logical_device, graphics_queue, graphics_command_pool, texture_image, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mip_levels);

	// copy data to image
	copy_image_buffer(MainDevice.logical_device, graphics_queue, graphics_command_pool, image_staging_buffer,
		texture_image, width, height);

	// transition image to be shader readable for shader stage

	//transition_image_layout(MainDevice.logical_device, graphics_queue, graphics_command_pool, texture_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	//											VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mip_levels);


	// add texture data to vector for reference
	texture_images.push_back(texture_image);
	texture_images_memory.push_back(texture_image_memory);

	// update mip level
	texture_mip_levels.push_back(mip_levels);

	// destroy staging buffers
	vkDestroyBuffer(MainDevice.logical_device, image_staging_buffer, nullptr);
	vkFreeMemory(MainDevice.logical_device, image_staging_buffer_memory, nullptr);

	// generate mipmaps
	generate_mipmaps(MainDevice.physical_device, MainDevice.logical_device, graphics_command_pool, graphics_queue,
		texture_image, VK_FORMAT_R8G8B8A8_SRGB, width, height, mip_levels);

	// return index to element in vector
	return static_cast<uint32_t>(texture_images.size() - 1);

}

void VulkanRenderer::create_sampler_array_descriptor_set()
{

	// descriptor set allocation info 
	VkDescriptorSetAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = sampler_descriptor_pool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = &sampler_set_layout;

	// allocte descriptor sets
	VkResult result = vkAllocateDescriptorSets(MainDevice.logical_device, &alloc_info, &sampler_descriptor_set);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to allocate texture descriptor sets!");

	}

	// texture image info
	std::vector<VkDescriptorImageInfo> image_info;
	image_info.resize(texture_image_views.size());
	for (int i = 0; i < texture_image_views.size(); i++) {
		image_info[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		image_info[i].imageView = texture_image_views[i];
		image_info[i].sampler = nullptr;
	}

	// descriptor write info
	VkWriteDescriptorSet descriptor_write{};
	descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_write.dstSet = sampler_descriptor_set;
	descriptor_write.dstBinding = TEXTURES_BINDING;
	descriptor_write.dstArrayElement = 0;
	descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	descriptor_write.descriptorCount = static_cast<uint32_t>(image_info.size());
	descriptor_write.pImageInfo = image_info.data();

	VkDescriptorImageInfo sampler_info;
	sampler_info.imageView = nullptr;
	sampler_info.sampler = texture_sampler;

	// descriptor write info
	VkWriteDescriptorSet descriptor_write_sampler{};
	descriptor_write_sampler.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_write_sampler.dstSet = sampler_descriptor_set;
	descriptor_write_sampler.dstBinding = SAMPLER_BINDING;
	descriptor_write_sampler.dstArrayElement = 0;
	descriptor_write_sampler.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	descriptor_write_sampler.descriptorCount = 1;
	descriptor_write_sampler.pImageInfo = &sampler_info;

	std::vector<VkWriteDescriptorSet> write_descriptor_sets = { descriptor_write,
																 descriptor_write_sampler };

	// update new descriptor set
	vkUpdateDescriptorSets(MainDevice.logical_device, static_cast<uint32_t>(write_descriptor_sets.size()),
							write_descriptor_sets.data(), 0, nullptr);

}

int VulkanRenderer::create_model(std::string modelFile)
{

	ObjLoader obj_loader(	MainDevice.physical_device, MainDevice.logical_device, graphics_queue,
							graphics_command_pool);

	std::vector<std::string> textureNames = obj_loader.load_textures(modelFile);
	std::vector<int> matToTex(textureNames.size());

	// Loop over textureNames and create textures for them
	for (size_t i = 0; i < textureNames.size(); i++)
	{
		// If material had no texture, set '0' to indicate no texture, texture 0 will be reserved for a default texture
		if (!textureNames[i].empty())
		{
			int offset = 1; // because place 0 has plain texture
			// Otherwise, create texture and set value to index of new texture
			int location = create_texture(textureNames[i]);
			matToTex[i] = location;

		}
		else {

			matToTex[i] = 0;

		}

	}
	std::shared_ptr<Model> new_model = obj_loader.load_model(modelFile, matToTex);

	create_sampler_array_descriptor_set();

	scene->add_model(new_model);

	return scene->get_model_count() - 1;

}

stbi_uc* VulkanRenderer::load_texture_file(std::string file_name, int* width, int* height, VkDeviceSize* image_size)
{

	// number of channels image uses
	int channels;
	// load pixel data for image
	//std::string file_loc = "../Resources/Textures/" + file_name;
	stbi_uc* image = stbi_load(file_name.c_str(), width, height, &channels, STBI_rgb_alpha);

	if (!image) {

		throw std::runtime_error("Failed to load a texture file! (" + file_name + ")");

	}

	// calculate image size using given and known data
	*image_size = *width * *height * 4;

	return image;

}

void VulkanRenderer::update_uniform_buffers(uint32_t image_index)
{
 
	/*void* data;
	vkMapMemory(MainDevice.logical_device, vp_uniform_buffer_memory[image_index], 0, sizeof(UboViewProjection), 0, &data);
	memcpy(data, &ubo_view_projection, sizeof(UboViewProjection));
	vkUnmapMemory(MainDevice.logical_device, vp_uniform_buffer_memory[image_index]);

	vkMapMemory(MainDevice.logical_device, directions_uniform_buffer_memory[image_index], 0, sizeof(UboDirections), 0, &data);
	memcpy(data, &ubo_directions, sizeof(UboDirections));
	vkUnmapMemory(MainDevice.logical_device, directions_uniform_buffer_memory[image_index]);*/

	//VkCommandBuffer cmdBuffer = begin_command_buffer(MainDevice.logical_device, graphics_command_pool);

	//VkCommandBufferBeginInfo buffer_begin_info{};
	//buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	////buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	//// start recording commands to command buffer
	//VkResult result = vkBeginCommandBuffer(command_buffers[image_index], &buffer_begin_info);

	update_light_direction(glm::vec3(directional_light_direction[0], directional_light_direction[1], directional_light_direction[2]));

	auto usage_stage_flags = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR | 
							VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		 

	VkBufferMemoryBarrier before_barrier_uvp;
	before_barrier_uvp.pNext = nullptr;
	before_barrier_uvp.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	before_barrier_uvp.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	before_barrier_uvp.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	before_barrier_uvp.buffer = vp_uniform_buffer[image_index];
	before_barrier_uvp.offset = 0;
	before_barrier_uvp.size = sizeof(ubo_view_projection);
	before_barrier_uvp.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	before_barrier_uvp.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	VkBufferMemoryBarrier before_barrier_directions;
	before_barrier_directions.pNext = nullptr;
	before_barrier_directions.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	before_barrier_directions.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	before_barrier_directions.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	before_barrier_directions.buffer = vp_uniform_buffer[image_index];
	before_barrier_directions.offset = 0;
	before_barrier_directions.size = sizeof(ubo_directions);
	before_barrier_directions.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	before_barrier_directions.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	vkCmdPipelineBarrier(command_buffers[image_index], usage_stage_flags,
		VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &before_barrier_uvp, 0, nullptr);
	vkCmdPipelineBarrier(command_buffers[image_index], usage_stage_flags,
		VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &before_barrier_directions, 0, nullptr);

	vkCmdUpdateBuffer(command_buffers[image_index], vp_uniform_buffer[image_index], 0, sizeof(UboViewProjection), &ubo_view_projection);
	vkCmdUpdateBuffer(command_buffers[image_index], directions_uniform_buffer[image_index], 0, sizeof(UboDirections), &ubo_directions);

	VkBufferMemoryBarrier after_barrier_uvp;
	after_barrier_uvp.pNext = nullptr;
	after_barrier_uvp.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	after_barrier_uvp.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	after_barrier_uvp.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	after_barrier_uvp.buffer = vp_uniform_buffer[image_index];
	after_barrier_uvp.offset = 0;
	after_barrier_uvp.size = sizeof(UboViewProjection);
	after_barrier_uvp.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	after_barrier_uvp.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	VkBufferMemoryBarrier after_barrier_directions;
	after_barrier_directions.pNext = nullptr;
	after_barrier_directions.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	after_barrier_directions.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	after_barrier_directions.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	after_barrier_directions.buffer = vp_uniform_buffer[image_index];
	after_barrier_directions.offset = 0;
	after_barrier_directions.size = sizeof(UboDirections);
	after_barrier_directions.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	after_barrier_directions.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	vkCmdPipelineBarrier(command_buffers[image_index], VK_PIPELINE_STAGE_TRANSFER_BIT,
		usage_stage_flags, 0, 0, nullptr, 1, &after_barrier_uvp, 0, nullptr);
	vkCmdPipelineBarrier(command_buffers[image_index], VK_PIPELINE_STAGE_TRANSFER_BIT,
			usage_stage_flags, 0, 0, nullptr, 1, &after_barrier_directions, 0, nullptr);

	//// stop recording to command buffer
	//result = vkEndCommandBuffer(command_buffers[image_index]);

	//if (result != VK_SUCCESS) {

	//	throw std::runtime_error("Failed to stop recording a command buffer!");

	//}

	//end_and_submit_command_buffer(MainDevice.logical_device, graphics_command_pool, graphics_queue, cmdBuffer);

}

void VulkanRenderer::recreate_swap_chain()
{

	// capture case of minification
	int width = 0, height = 0;
	glfwGetFramebufferSize(window->get_window(), &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window->get_window(), &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(MainDevice.logical_device);
	vkQueueWaitIdle(graphics_queue);

	clean_up_swapchain();

	create_swap_chain();
	create_depthbuffer_image();
	create_render_pass();
	create_rasterizer_graphics_pipeline();
	create_uniform_buffers();
	create_descriptor_pool_uniforms();
	create_descriptor_sets();
	create_command_buffers();

	// init the offscreen render pass 
	create_offscreen_textures();
	create_offscreen_render_pass();
	create_offscreen_framebuffers();
	create_offscreen_graphics_pipeline();

	// all post
	create_post_renderpass();
	create_post_descriptor();
	update_post_descriptor_set();
	create_post_pipeline();
	create_framebuffers();

	images_in_flight_fences.resize(swap_chain_images.size(), VK_NULL_HANDLE);

}

void VulkanRenderer::update_raytracing_descriptor_set(uint32_t image_index)
{

	VkWriteDescriptorSetAccelerationStructureKHR descriptor_set_acceleration_structure{};
	descriptor_set_acceleration_structure.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	descriptor_set_acceleration_structure.pNext = nullptr;
	descriptor_set_acceleration_structure.accelerationStructureCount = 1;
	descriptor_set_acceleration_structure.pAccelerationStructures = &(tlas.top_level_acceleration_structure);

	VkWriteDescriptorSet write_descriptor_set_acceleration_structure{};
	write_descriptor_set_acceleration_structure.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_descriptor_set_acceleration_structure.pNext = &descriptor_set_acceleration_structure;
	write_descriptor_set_acceleration_structure.dstSet = raytracing_descriptor_set[image_index];
	write_descriptor_set_acceleration_structure.dstBinding = TLAS_BINDING;
	write_descriptor_set_acceleration_structure.dstArrayElement = 0;
	write_descriptor_set_acceleration_structure.descriptorCount = 1;
	write_descriptor_set_acceleration_structure.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	write_descriptor_set_acceleration_structure.pImageInfo = nullptr;
	write_descriptor_set_acceleration_structure.pBufferInfo = nullptr;
	write_descriptor_set_acceleration_structure.pTexelBufferView = nullptr;

	VkDescriptorBufferInfo object_description_buffer_info{};
	object_description_buffer_info.buffer = object_description_buffer;
	object_description_buffer_info.offset = 0;
	object_description_buffer_info.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet object_description_buffer_write{};
	object_description_buffer_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	object_description_buffer_write.dstSet = descriptor_sets[image_index];
	object_description_buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	object_description_buffer_write.dstBinding = OBJECT_DESCRIPTION_BINDING;
	object_description_buffer_write.pBufferInfo = &object_description_buffer_info;
	object_description_buffer_write.descriptorCount = 1;

	std::vector<VkWriteDescriptorSet> write_descriptor_sets = {	write_descriptor_set_acceleration_structure,
																object_description_buffer_write };

	vkUpdateDescriptorSets(MainDevice.logical_device, static_cast<uint32_t>(write_descriptor_sets.size()),
		write_descriptor_sets.data(), 0, nullptr);

}

void VulkanRenderer::record_commands(uint32_t image_index)
{

	PFN_vkGetBufferDeviceAddressKHR pvkGetBufferDeviceAddressKHR = (PFN_vkGetBufferDeviceAddressKHR)
												vkGetDeviceProcAddr(MainDevice.logical_device, "vkGetBufferDeviceAddress");
	PFN_vkCmdTraceRaysKHR pvkCmdTraceRaysKHR = (PFN_vkCmdTraceRaysKHR)
												vkGetDeviceProcAddr(MainDevice.logical_device, "vkCmdTraceRaysKHR");

	if (raytracing) {
		
		uint32_t handle_size = raytracing_properties.shaderGroupHandleSize;
		uint32_t handle_size_aligned = align_up(handle_size, raytracing_properties.shaderGroupHandleAlignment);
		
		PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>
								(vkGetDeviceProcAddr(MainDevice.logical_device, "vkGetBufferDeviceAddressKHR"));

		VkBufferDeviceAddressInfoKHR bufferDeviceAI{};
		bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		bufferDeviceAI.buffer = raygen_shader_binding_table_buffer;

		rgen_region.deviceAddress = vkGetBufferDeviceAddressKHR(MainDevice.logical_device, &bufferDeviceAI);
		rgen_region.stride = handle_size_aligned;
		rgen_region.size = handle_size_aligned;

		bufferDeviceAI.buffer = miss_shader_binding_table_buffer;
		miss_region.deviceAddress = vkGetBufferDeviceAddressKHR(MainDevice.logical_device, &bufferDeviceAI);
		miss_region.stride = handle_size_aligned;
		miss_region.size = handle_size_aligned;

		bufferDeviceAI.buffer = hit_shader_binding_table_buffer;
		hit_region.deviceAddress = vkGetBufferDeviceAddressKHR(MainDevice.logical_device, &bufferDeviceAI);
		hit_region.stride = handle_size_aligned;
		hit_region.size = handle_size_aligned;

		// for GCC doen't allow references on rvalues go like that ... 
		pc_ray.clear_color = { 0.2f,0.65f,0.4f,1.0f };
		// just "Push" constants to given shader stage directly (no buffer)
		vkCmdPushConstants(command_buffers[image_index],
			raytracing_pipeline_layout,
			VK_SHADER_STAGE_RAYGEN_BIT_KHR |
			VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
			VK_SHADER_STAGE_MISS_BIT_KHR,								
			0,																								
			sizeof(PushConstantRaytracing),										
			&pc_ray);

		vkCmdBindPipeline(command_buffers[image_index], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, raytracing_pipeline);

		std::array<VkDescriptorSet, 2> sets = { descriptor_sets[image_index],
												raytracing_descriptor_set[image_index] };

		vkCmdBindDescriptorSets(command_buffers[image_index], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, raytracing_pipeline_layout,
			0, static_cast<uint32_t>(sets.size()), sets.data(),
			0, nullptr);

		pvkCmdTraceRaysKHR(command_buffers[image_index], &rgen_region, &miss_region,
							&hit_region, &call_region,
							swap_chain_extent.width, swap_chain_extent.height, 1);

	}
	else {

		// this falg is not longer needed because we synchronize with fences and semaphores
		// buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;					// buffer can be resubmitted when it has already been submitted and is awaiting execution

		// information about how to begin a render pass (only needed for graphical applications)
		VkRenderPassBeginInfo render_pass_begin_info{};
		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass = offscreen_render_pass;//render_pass;																				// render pass to begin 
		render_pass_begin_info.renderArea.offset = { 0,0 };																				// start point of render pass in pixels 
		render_pass_begin_info.renderArea.extent = swap_chain_extent;													// size of region to run render pass on (starting at offset)

		// make sure the order you put the values into the array matches with the attchment order you have defined previous
		std::array<VkClearValue, 2> clear_values = {};
		clear_values[0].color = { 0.2f, 0.65f,0.4f, 1.0f };
		clear_values[1].depthStencil = { 1.0f , 0};

		render_pass_begin_info.pClearValues = clear_values.data();
		render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass_begin_info.framebuffer = offscreen_framebuffer[image_index];//swap_chain_framebuffers[current_image];	// used framebuffer depends on the swap chain and therefore is changing for each command buffer

		/*transition_image_layout_for_command_buffer(command_buffers[current_image], offscreen_images[current_image].image,
														VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
														1 , VK_IMAGE_ASPECT_COLOR_BIT);*/
														// begin render pass
		vkCmdBeginRenderPass(command_buffers[image_index], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

		// bind pipeline to be used in render pass
		vkCmdBindPipeline(command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, offscreen_graphics_pipeline);

		for (unsigned int m = 0; m < scene->get_model_count(); m++) {

			// for GCC doen't allow references on rvalues go like that ... 
			pc_raster.model = scene->get_model_matrix(m);
			// just "Push" constants to given shader stage directly (no buffer)
			vkCmdPushConstants(command_buffers[image_index],
				offscreen_pipeline_layout,
				VK_SHADER_STAGE_VERTEX_BIT,								// stage to push constants to 
				0,														// offset to push constants to update
				sizeof(PushConstantRaster),								// size of data being pushed 
				&pc_raster);											// using model of current mesh (can be array)

			for (unsigned int k = 0; k < scene->get_mesh_count(m); k++) {

				// list of vertex buffers we want to draw 
				VkBuffer vertex_buffers[] = { scene->get_vertex_buffer(m,k) };					// buffers to bind 
				VkDeviceSize offsets[] = { 0 };																																											// offsets into buffers being bound
				vkCmdBindVertexBuffers(command_buffers[image_index], 0, 1, vertex_buffers, offsets);	// command to bind vertex buffer before drawing with them

				// bind mesh index buffer with 0 offset and using the uint32 type
				vkCmdBindIndexBuffer(command_buffers[image_index], scene->get_index_buffer(m, k), 0, VK_INDEX_TYPE_UINT32);			// command to bind index buffer before drawing with them

				// danamic offset amount
				// uint32_t dynamic_offset = static_cast<uint32_t>(model_uniform_alignment) * static_cast<uint32_t>(m);

				std::array<VkDescriptorSet, 2> descriptor_set_group = { descriptor_sets[image_index],
																		sampler_descriptor_set
																		/*sampler_descriptor_sets[scene->get_texture_id(m,k)]*/ };

				// bind descriptor sets 
				vkCmdBindDescriptorSets(command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, offscreen_pipeline_layout,
															0, static_cast<uint32_t>(descriptor_set_group.size()),
															descriptor_set_group.data(), 0, nullptr);

				// execute pipeline
				vkCmdDrawIndexed(command_buffers[image_index], static_cast<uint32_t>(scene->get_index_count(m, k)), 1, 0, 0, 0);

			}

		}
		
		// end render pass 
		vkCmdEndRenderPass(command_buffers[image_index]);

	}

	// information about how to begin a render pass (only needed for graphical applications)
	VkRenderPassBeginInfo render_pass_begin_info{};
	render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.renderPass = post_render_pass;												// render pass to begin 
	render_pass_begin_info.renderArea.offset = { 0,0 };													// start point of render pass in pixels 
	render_pass_begin_info.renderArea.extent = swap_chain_extent;										// size of region to run render pass on (starting at offset)

	// make sure the order you put the values into the array matches with the attchment order you have defined previous
	std::array<VkClearValue, 2> clear_values = {};
	clear_values[0].color = { 0.2f, 0.65f,0.4f, 1.0f };
	clear_values[1].depthStencil = { 1.0f , 0};

	render_pass_begin_info.pClearValues = clear_values.data();
	render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
	render_pass_begin_info.framebuffer = swap_chain_framebuffers[image_index];							// used framebuffer depends on the swap chain and therefore is changing for each command buffer

	transition_image_layout_for_command_buffer(command_buffers[image_index], offscreen_images[image_index].image,
													VK_IMAGE_LAYOUT_GENERAL,
													VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, VK_IMAGE_ASPECT_COLOR_BIT);

	/*transition_image_layout_for_command_buffer(command_buffers[image_index], swap_chain_images[image_index].image,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 1, VK_IMAGE_ASPECT_COLOR_BIT);*/

	// begin render pass
	vkCmdBeginRenderPass(command_buffers[image_index], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
	auto aspectRatio = static_cast<float>(swap_chain_extent.width) / static_cast<float>(swap_chain_extent.height);
	PushConstantPost pc_post{};
	pc_post.aspect_ratio = aspectRatio;
	vkCmdPushConstants(command_buffers[image_index], post_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT |
		VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantPost), &pc_post);
	vkCmdBindPipeline(command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, post_graphics_pipeline);
	vkCmdBindDescriptorSets(command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS,
		post_pipeline_layout, 0, 1, &post_descriptor_set[image_index], 0, nullptr);
	vkCmdDraw(command_buffers[image_index], 3, 1, 0, 0);

	// Rendering
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffers[image_index]);

	// end render pass 
	vkCmdEndRenderPass(command_buffers[image_index]);

	/*transition_image_layout_for_command_buffer(command_buffers[image_index], swap_chain_images[image_index].image,
		 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED, 1, VK_IMAGE_ASPECT_COLOR_BIT);*/

	transition_image_layout_for_command_buffer(command_buffers[image_index], offscreen_images[image_index].image,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, 1, VK_IMAGE_ASPECT_COLOR_BIT);


	// stop recording to command buffer
	// result = vkEndCommandBuffer(command_buffers[image_index]);

	//if (result != VK_SUCCESS) {

	//	throw std::runtime_error("Failed to stop recording a command buffer!");

	//}
	
	
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
	vkGetPhysicalDeviceProperties(MainDevice.physical_device, &device_properties);

	// min_uniform_buffer_offset = device_properties.limits.minUniformBufferOffsetAlignment;

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

		if (queue_family.queueCount > 0 && queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) {

			indices.compute_family = index;

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


VkImage VulkanRenderer::create_image(uint32_t width, uint32_t height, uint32_t mip_levels, VkFormat format, VkImageTiling tiling, 
																		VkImageUsageFlags use_flags, VkMemoryPropertyFlags prop_flags, VkDeviceMemory* image_memory)
{

	// CREATE image
	// image creation info
	VkImageCreateInfo image_create_info{};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;													// type of image (1D, 2D, 3D)
	image_create_info.extent.width = width;																		// width if image extent
	image_create_info.extent.height = height;																	// height if image extent
	image_create_info.extent.depth = 1;																				// height if image extent
	image_create_info.mipLevels = mip_levels;																	// number of mipmap levels 
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

VkImageView VulkanRenderer::create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels)
{

	VkImageViewCreateInfo view_create_info{};
	view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_create_info.image = image;																	// image to create view for 
	view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;												// typ of image
	view_create_info.format = format;
	view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;									// allows remapping of rgba components to other rgba values 
	view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	// subresources allow the view to view only a part of an image 
	view_create_info.subresourceRange.aspectMask = aspect_flags;									// which aspect of an image to view (e.g. color bit for viewing color)
	view_create_info.subresourceRange.baseMipLevel = 0;												// start mipmap level to view from
	view_create_info.subresourceRange.levelCount = mip_levels;										// number of mipmap levels to view 
	view_create_info.subresourceRange.baseArrayLayer = 0;											// start array level to view from 
	view_create_info.subresourceRange.layerCount = 1;												// number of array levels to view 

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
	//sampler_descriptor_sets.push_back(descriptor_set);

	// return descriptor set location
	return 0;// static_cast<uint32_t>(sampler_descriptor_sets.size()) - 1;

}

void VulkanRenderer::check_changed_framebuffer_size()
{

	if (window->framebuffer_size_has_changed()) {

		int width, height;
		glfwGetFramebufferSize(window->get_window(), &width, &height);

		ubo_view_projection.projection = glm::perspective(glm::radians(40.0f), (float)width / (float)height,
															0.1f, 1000.f);

		window->reset_framebuffer_has_changed();
		framebuffer_resized = true;

	}
}

void VulkanRenderer::clean_up_gui()
{

	// clean up of GUI stuff
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	vkDestroyDescriptorPool(MainDevice.logical_device, gui_descriptor_pool, nullptr);

}

void VulkanRenderer::clean_up_swapchain()
{

	// wait until no actions being run on device before destroying
	vkDeviceWaitIdle(MainDevice.logical_device);

	for (auto framebuffer : swap_chain_framebuffers) {

		vkDestroyFramebuffer(MainDevice.logical_device, framebuffer, nullptr);

	}

	vkDestroyPipeline(MainDevice.logical_device, graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(MainDevice.logical_device, pipeline_layout, nullptr);
	vkDestroyRenderPass(MainDevice.logical_device, render_pass, nullptr);

	// -- CLEAN DEPTH BUFFER
	// only destroy the view; keep image and memory
	vkDestroyImageView(MainDevice.logical_device, depth_buffer_image_view, nullptr);
	vkDestroyImage(MainDevice.logical_device, depth_buffer_image, nullptr);
	vkFreeMemory(MainDevice.logical_device, depth_buffer_image_memory, nullptr);

	for (auto image : swap_chain_images) {

		vkDestroyImageView(MainDevice.logical_device, image.image_view, nullptr);

	}

	vkDestroySwapchainKHR(MainDevice.logical_device, swapchain, nullptr);


	for (size_t i = 0; i < swap_chain_images.size(); i++) {

		vkDestroyBuffer(MainDevice.logical_device, vp_uniform_buffer[i], nullptr);
		vkFreeMemory(MainDevice.logical_device, vp_uniform_buffer_memory[i], nullptr);
		vkDestroyBuffer(MainDevice.logical_device, directions_uniform_buffer[i], nullptr);
		vkFreeMemory(MainDevice.logical_device, directions_uniform_buffer_memory[i], nullptr);
	}

	// -- POST 
	vkDestroyRenderPass(MainDevice.logical_device, post_render_pass, nullptr);
	vkDestroyPipeline(MainDevice.logical_device, post_graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(MainDevice.logical_device, post_pipeline_layout, nullptr);
	vkDestroyDescriptorSetLayout(MainDevice.logical_device, post_descriptor_set_layout, nullptr);
	vkDestroyDescriptorPool(MainDevice.logical_device, post_descriptor_pool, nullptr);

	// -- OFFSCREEN
	vkDestroyPipeline(MainDevice.logical_device, offscreen_graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(MainDevice.logical_device, offscreen_pipeline_layout, nullptr);
	vkDestroyRenderPass(MainDevice.logical_device, offscreen_render_pass, nullptr);

	for (auto framebuffer : offscreen_framebuffer) {

		vkDestroyFramebuffer(MainDevice.logical_device, framebuffer, nullptr);

	}

	for (auto image : offscreen_images) {

		vkDestroyImageView(MainDevice.logical_device, image.image_view, nullptr);
		vkDestroyImage(MainDevice.logical_device, image.image, nullptr);
		vkFreeMemory(MainDevice.logical_device, image.image_memory, nullptr);

	}

	// depth buffer
	vkDestroyImageView(MainDevice.logical_device, offscreen_depth_buffer_image_view, nullptr);
	vkDestroyImage(MainDevice.logical_device, offscreen_depth_buffer_image, nullptr);
	vkFreeMemory(MainDevice.logical_device, offscreen_depth_buffer_image_memory, nullptr);

	// -- UNIFORM VALUES CLEAN UP
	// desriptor pool size depends on number of images in swapchain, therefore clean it up here
	vkDestroyDescriptorPool(MainDevice.logical_device, descriptor_pool, nullptr);

}

void VulkanRenderer::clean_up_raytracing()
{
	// -- EXPLICITLY LOAD FUNCTIONS
	PFN_vkDestroyAccelerationStructureKHR pvkDestroyAccelerationStructureKHR =
										(PFN_vkDestroyAccelerationStructureKHR)vkGetDeviceProcAddr(MainDevice.logical_device, "vkDestroyAccelerationStructureKHR");


	vkDestroyPipeline(MainDevice.logical_device, raytracing_pipeline, nullptr);
	vkDestroyPipelineLayout(MainDevice.logical_device, raytracing_pipeline_layout, nullptr);

	vkDestroyDescriptorSetLayout(MainDevice.logical_device, raytracing_descriptor_set_layout, nullptr);

	vkDestroyDescriptorPool(MainDevice.logical_device, object_description_pool, nullptr);
	vkDestroyDescriptorPool(MainDevice.logical_device, raytracing_descriptor_pool, nullptr);
	
	vkDestroyBuffer(MainDevice.logical_device, object_description_buffer, nullptr);
	vkFreeMemory(MainDevice.logical_device, object_description_buffer_memory, nullptr);

	pvkDestroyAccelerationStructureKHR(MainDevice.logical_device, tlas.top_level_acceleration_structure, nullptr);
	vkDestroyBuffer(MainDevice.logical_device, tlas.top_level_acceleration_structure_buffer, nullptr);
	vkFreeMemory(MainDevice.logical_device, tlas.top_level_acceleration_structure_buffer_memory, nullptr);
	
	for (size_t index = 0; index < blas.size(); index++) {

		pvkDestroyAccelerationStructureKHR(MainDevice.logical_device, blas[index].accel, nullptr);
		vkDestroyBuffer(MainDevice.logical_device, blas[index].buffer, nullptr);
		vkFreeMemory(MainDevice.logical_device, blas[index].memory, nullptr);

	}

	vkDestroyBuffer(MainDevice.logical_device, shader_binding_table_buffer, nullptr);
	vkFreeMemory(MainDevice.logical_device, shader_binding_table_buffer_memory, nullptr);


	/*for (int i = 0; i < swap_chain_images.size(); i++) {

		vkDestroyImageView(MainDevice.logical_device, ray_tracing_images[i].raytracing_image_view, nullptr);
		vkFreeMemory(MainDevice.logical_device, ray_tracing_images[i].ray_tracing_image_memory, nullptr);
		vkDestroyImage(MainDevice.logical_device, ray_tracing_images[i].raytracing_image, nullptr);

	}*/

}

void VulkanRenderer::clean_up()
{

	PFN_vkDestroyDebugUtilsMessengerEXT pvkDestroyDebugUtilsMessengerEXT =
										(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	// wait until no actions being run on device before destroying
	vkDeviceWaitIdle(MainDevice.logical_device);

	// -- SUBSUMMARIZE ALL SWAPCHAIN DEPENDEND THINGS
	clean_up_swapchain();

	for (auto model : scene->get_model_list()) {

		for (int i = 0; i < model->get_mesh_count(); i++) {

			model->get_mesh(i)->destroy_buffers();

		}

	}

	// --- COMPUTE STUFF
	vkDestroyCommandPool(MainDevice.logical_device, compute_command_pool, nullptr);

	// -- CLEAN UP RAYTRACING STUFF
	 clean_up_raytracing();

	 //instead of recreate command pool from scretch empty command buffers
	vkFreeCommandBuffers(MainDevice.logical_device, graphics_command_pool, 
												static_cast<uint32_t>(command_buffers.size()), command_buffers.data());

	// -- DESTROY ALL LAYOUTS
	vkDestroyDescriptorSetLayout(MainDevice.logical_device, descriptor_set_layout, nullptr);
	vkDestroyDescriptorSetLayout(MainDevice.logical_device, sampler_set_layout, nullptr);

	// -- TEXTURE REALTED
	vkDestroyDescriptorPool(MainDevice.logical_device, sampler_descriptor_pool, nullptr);
	vkDestroySampler(MainDevice.logical_device, texture_sampler, nullptr);

	for (size_t i = 0; i < texture_images.size(); i++) {

		vkDestroyImageView(MainDevice.logical_device, texture_image_views[i], nullptr);
		vkDestroyImage(MainDevice.logical_device, texture_images[i], nullptr);
		vkFreeMemory(MainDevice.logical_device, texture_images_memory[i], nullptr);

	}

	// free all space we allocated for all model matrices for each object in our scene
	/*#if defined (_WIN32) 
		_aligned_free(model_transfer_space);
	#elif defined (__linux__)
		std::free(model_transfer_space);
	#endif	*/

	for (int i = 0; i < MAX_FRAME_DRAWS; i++) {

		vkDestroySemaphore(MainDevice.logical_device, render_finished[i], nullptr);
		vkDestroySemaphore(MainDevice.logical_device, image_available[i], nullptr);
		vkDestroyFence(MainDevice.logical_device, in_flight_fences[i], nullptr);

	}

	vkDestroyCommandPool(MainDevice.logical_device, graphics_command_pool, nullptr);

	clean_up_gui();

	if (ENABLE_VALIDATION_LAYERS) {
		pvkDestroyDebugUtilsMessengerEXT(instance, debug_messenger, NULL);
	}

	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyDevice(MainDevice.logical_device, nullptr);
	vkDestroyInstance(instance, nullptr);

}

VulkanRenderer::~VulkanRenderer()
{
}