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

	void createASForScene(	VulkanDevice* device,
							VkCommandPool commandPool,
							Scene* scene,
							TopLevelAccelerationStructure& tlas,
							std::vector<BottomLevelAccelerationStructure>& blas);

	void createBLAS(VulkanDevice* device, 
					VkCommandPool commandPool, 
					Scene* scene, 
					std::vector<BottomLevelAccelerationStructure>& blas);

	void createTLAS(VulkanDevice* device,
					VkCommandPool commandPool,
					Scene* scene,
					TopLevelAccelerationStructure& tlas,
					std::vector<BottomLevelAccelerationStructure>& blas);

	void cleanUp();

	~ASManager();

private:

	CommandBufferManager	commandBufferManager;
	VulkanBufferManager		vulkanBufferManager;

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

