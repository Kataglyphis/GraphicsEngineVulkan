#pragma once
#include <vulkan/vulkan.h>
#include "VulkanDevice.h"

#include "Scene.h"
#include "TopLevelAccelerationStructure.h"
#include "BottomLevelAccelerationStructure.h"
#include "CommandBufferManager.h"

struct BuildAccelerationStructure {

	VkAccelerationStructureBuildGeometryInfoKHR build_info;
	VkAccelerationStructureBuildSizesInfoKHR size_info;
	const VkAccelerationStructureBuildRangeInfoKHR* range_info;
	BottomLevelAccelerationStructure single_blas;

};

struct BlasInput {

	std::vector<VkAccelerationStructureGeometryKHR> as_geometry;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR> as_build_offset_info;

};

class ASManager
{
public:

	ASManager();

	VkAccelerationStructureKHR& getTLAS() { return tlas.vulkanAS; };

	void createASForScene(	VulkanDevice* device,
							VkCommandPool commandPool,
							Scene* scene);

	void createBLAS(VulkanDevice* device, 
					VkCommandPool commandPool, 
					Scene* scene);

	void createTLAS(VulkanDevice* device,
					VkCommandPool commandPool,
					Scene* scene);

	void cleanUp();

	~ASManager();

private:

	VulkanDevice*			vulkanDevice;
	CommandBufferManager	commandBufferManager;
	VulkanBufferManager		vulkanBufferManager;

	std::vector<BottomLevelAccelerationStructure>	blas;
	TopLevelAccelerationStructure					tlas;

	void createSingleBlas(	VulkanDevice* device, 
							VkCommandBuffer command_buffer,
							BuildAccelerationStructure& build_as_structure,
							VkDeviceAddress scratch_device_or_host_address);

	void createAccelerationStructureInfosBLAS(	VulkanDevice* device, 
												BuildAccelerationStructure& build_as_structure,
												BlasInput& blas_input,
												VkDeviceSize& current_scretch_size,
												VkDeviceSize& current_size);

	void objectToVkGeometryKHR(		VulkanDevice* device, 
									Mesh* mesh,
									VkAccelerationStructureGeometryKHR& acceleration_structure_geometry,
									VkAccelerationStructureBuildRangeInfoKHR& acceleration_structure_build_range_info);

};

