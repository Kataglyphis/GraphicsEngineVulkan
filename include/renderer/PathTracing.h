#pragma once
#include <vulkan/vulkan.h>
#include <PushConstantPathTracing.h>
#include <VulkanDevice.h>
#include <VulkanSwapChain.h>

class PathTracing
{
public:

	PathTracing();

	void			init(VulkanDevice* device, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);

	void			shaderHotReload(std::vector<VkDescriptorSetLayout> descriptor_set_layouts);

	void			recordCommands(	VkCommandBuffer& commandBuffer, uint32_t image_index,
									VulkanImage& vulkanImage,
									VulkanSwapChain* vulkanSwapChain,
									const std::vector<VkDescriptorSet>& descriptorSets);

	void			cleanUp();

	~PathTracing();

private:

	VulkanDevice* device;

	VkPipelineLayout pipeline_layout;
	VkPipeline pipeline;
	VkPushConstantRange pc_range;
	PushConstantPathTracing push_constant{};

	float					timeStampPeriod{ 0 };
	uint64_t				pathTracingTiming;
	uint32_t				query_count{ 2 };
	std::vector<uint64_t>	queryResults;
	VkQueryPool				queryPool;

	struct {

		uint32_t maxComputeWorkGroupCount[3];
		uint32_t maxComputeWorkGroupInvocations;
		uint32_t maxComputeWorkGroupSize[3];

	} computeLimits;

	struct SpecializationData {

		// standard values 
		uint32_t specWorkGroupSizeX = 16;
		uint32_t specWorkGroupSizeY = 8;
		uint32_t specWorkGroupSizeZ = 0;

	};

	SpecializationData specializationData;

	void createQueryPool();
	void createPipeline(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
};

