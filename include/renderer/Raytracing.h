#pragma once
#include <vulkan/vulkan.h>
#include <PushConstantRayTracing.h>
#include <VulkanBuffer.h>
#include <VulkanSwapChain.h>

class Raytracing
{
public:

	Raytracing();

	void init(	VulkanDevice* device,
				const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);

	void shaderHotReload(std::vector<VkDescriptorSetLayout> descriptor_set_layouts);

	void recordCommands(VkCommandBuffer& commandBuffer, 
						VulkanSwapChain* vulkanSwapChain,
						const std::vector<VkDescriptorSet>& descriptorSets);

	void cleanUp();

	~Raytracing();

private:

	VulkanDevice*					device;
	VulkanSwapChain*				vulkanSwapChain;

	VkPipeline						graphicsPipeline;
	VkPipelineLayout				pipeline_layout;
	PushConstantRaytracing			pc;
	VkPushConstantRange				pc_ranges;

	std::vector<VkRayTracingShaderGroupCreateInfoKHR> shader_groups;
	VulkanBuffer					shaderBindingTableBuffer;
	VulkanBuffer					raygenShaderBindingTableBuffer;
	VulkanBuffer					missShaderBindingTableBuffer;
	VulkanBuffer					hitShaderBindingTableBuffer;

	VkStridedDeviceAddressRegionKHR rgen_region{};
	VkStridedDeviceAddressRegionKHR miss_region{};
	VkStridedDeviceAddressRegionKHR hit_region{};
	VkStridedDeviceAddressRegionKHR call_region{};


	VkPhysicalDeviceRayTracingPipelinePropertiesKHR raytracing_properties{};

	void							createPCRange();
	void							createGraphicsPipeline(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
	void							createSBT();
};

