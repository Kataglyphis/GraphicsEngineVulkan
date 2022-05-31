#pragma once
#include <vulkan/vulkan.h>
#include <PushConstantRasterizer.h>
#include <Texture.h>
#include <VulkanDevice.h>
#include <VulkanSwapChain.h>
#include <Scene.h>

class Rasterizer
{
public:

	Rasterizer();

	void		init(	VulkanDevice* device, VulkanSwapChain* vulkanSwapChain, 
						const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
						VkCommandPool& commandPool);

	void		shaderHotReload(std::vector<VkDescriptorSetLayout> descriptor_set_layouts);

	Texture&	getOffscreenTexture(uint32_t index);

	void		setPushConstant(PushConstantRasterizer pushConstant);

	void		recordCommands(	VkCommandBuffer& commandBuffer, uint32_t image_index, Scene* scene,
								const std::vector<VkDescriptorSet>& descriptorSets);

	void		cleanUp();

	~Rasterizer();

private:

	VulkanDevice*					device;
	VulkanSwapChain*				vulkanSwapChain;

	CommandBufferManager			commandBufferManager;

	std::vector<VkFramebuffer>		framebuffer;
	std::vector<Texture>			offscreenTextures;
	Texture							depthBufferImage;

	VkPushConstantRange				push_constant_range;
	PushConstantRasterizer			pushConstant;

	VkPipeline						graphics_pipeline;
	VkPipelineLayout				pipeline_layout;
	VkRenderPass					render_pass;

	void							createTextures(VkCommandPool& commandPool);
	void							createGraphicsPipeline(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
	void							createRenderPass();
	void							createFramebuffer();
	void							createPushConstantRange();

};

