#pragma once
#include <vulkan/vulkan.h>

#include <VulkanDevice.h>
#include <VulkanSwapChain.h>
#include <PushConstantPost.h>

class PostStage
{
public:

	PostStage();

	void			init(	VulkanDevice* device, 
							VulkanSwapChain* vulkanSwapChain,
							const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);

	void			shaderHotReload(std::vector<VkDescriptorSetLayout> descriptor_set_layouts);

	VkRenderPass&	getRenderPass() { return render_pass; };
	VkSampler&		getOffscreenSampler() { return offscreenTextureSampler; };

	void			recordCommands(	VkCommandBuffer& commandBuffer, uint32_t image_index,
									const std::vector<VkDescriptorSet>& descriptorSets);
	void			cleanUp();

	~PostStage();

private:

	VulkanDevice*					device;
	VulkanSwapChain*				vulkanSwapChain;

	std::vector<VkFramebuffer>		framebuffers;
	Texture							depthBufferImage;
	VkFormat						depth_format;
	void							createDepthbufferImage();

	VkSampler						offscreenTextureSampler;
	void							createOffscreenTextureSampler();

	VkPushConstantRange				push_constant_range;
	VkRenderPass					render_pass;
	VkPipeline						graphics_pipeline;
	VkPipelineLayout				pipeline_layout;

	void							createPushConstantRange();
	void							createRenderpass();
	void							createGraphicsPipeline(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
	void							createFramebuffer();

};

