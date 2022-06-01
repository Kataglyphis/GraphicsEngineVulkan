#include "PathTracing.h"
#include "ShaderHelper.h"
#include "File.h"
#include <array>
#include <algorithm>

PathTracing::PathTracing()
{
}

void PathTracing::init(VulkanDevice* device, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
{
	this->device = device;

	VkPhysicalDeviceProperties physicalDeviceProps = device->getPhysicalDeviceProperties();
	timeStampPeriod = physicalDeviceProps.limits.timestampPeriod;

	// save the limits for handling all special cases later on 
	computeLimits.maxComputeWorkGroupCount[0] = physicalDeviceProps.limits.maxComputeWorkGroupCount[0];
	computeLimits.maxComputeWorkGroupCount[1] = physicalDeviceProps.limits.maxComputeWorkGroupCount[1];
	computeLimits.maxComputeWorkGroupCount[2] = physicalDeviceProps.limits.maxComputeWorkGroupCount[2];

	computeLimits.maxComputeWorkGroupInvocations = physicalDeviceProps.limits.maxComputeWorkGroupInvocations;

	computeLimits.maxComputeWorkGroupSize[0] = physicalDeviceProps.limits.maxComputeWorkGroupSize[0];
	computeLimits.maxComputeWorkGroupSize[1] = physicalDeviceProps.limits.maxComputeWorkGroupSize[1];
	computeLimits.maxComputeWorkGroupSize[2] = physicalDeviceProps.limits.maxComputeWorkGroupSize[2];

	queryResults.resize(query_count);
	createQueryPool();

	createPipeline(descriptorSetLayouts);

}

void PathTracing::shaderHotReload(std::vector<VkDescriptorSetLayout> descriptor_set_layouts)
{

	vkDestroyPipeline(device->getLogicalDevice(), pipeline, nullptr);
	createPipeline(descriptor_set_layouts);

}

void PathTracing::recordCommands(	VkCommandBuffer& commandBuffer, 
									uint32_t image_index, 
									VulkanSwapChain* vulkanSwapChain,
									const std::vector<VkDescriptorSet>& descriptorSets)
{
	// we have reset the pool; hence start by 0 
	uint32_t query = 0;

	vkCmdBeginQuery(commandBuffer, queryPool, 0, 0);

	vkCmdWriteTimestamp(commandBuffer, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
						queryPool, query++);

	VkExtent2D imageSize = vulkanSwapChain->getSwapChainExtent();
	push_constant.width = imageSize.width;
	push_constant.height = imageSize.height;
	push_constant.clearColor = { 0.2f,0.65f,0.4f,1.0f };

	vkCmdPushConstants(commandBuffer,
						pipeline_layout,
						VK_SHADER_STAGE_COMPUTE_BIT,
						0,
						sizeof(PushConstantPathTracing),
						&push_constant);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
					pipeline);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
		pipeline_layout, 0, 1, &descriptorSets[image_index],
		0, 0);


	uint32_t workGroupCountX = std::max((imageSize.width + specializationData.specWorkGroupSizeX - 1) / specializationData.specWorkGroupSizeX, 1U);
	uint32_t workGroupCountY = std::max((imageSize.height + specializationData.specWorkGroupSizeY - 1) / specializationData.specWorkGroupSizeY, 1U);
	uint32_t workGroupCountZ = 0;

	vkCmdDispatch(commandBuffer, workGroupCountX, workGroupCountY, workGroupCountZ);

	vkCmdWriteTimestamp(commandBuffer, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
						queryPool, query++);
	VkResult result = vkGetQueryPoolResults(
								device->getLogicalDevice(),
								queryPool,
								0,
								query_count,
								queryResults.size() * sizeof(uint64_t),
								queryResults.data(),
								static_cast<VkDeviceSize>(sizeof(uint64_t)),
								VK_QUERY_RESULT_64_BIT);

	if (result != VK_NOT_READY) {
		pathTracingTiming = (static_cast<float>(queryResults[1] - queryResults[0])* timeStampPeriod) / 1000000.f;
	}

	vkCmdEndQuery(commandBuffer, queryPool, 0);
}

void PathTracing::cleanUp()
{
	vkDestroyPipeline(device->getLogicalDevice(), pipeline, nullptr);
	vkDestroyPipelineLayout(device->getLogicalDevice(), pipeline_layout, nullptr);

	vkDestroyQueryPool(device->getLogicalDevice(), queryPool, nullptr);

}

PathTracing::~PathTracing()
{
}

void PathTracing::createQueryPool()
{

	VkQueryPoolCreateInfo queryPoolInfo = {};
	queryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	// This query pool will store pipeline statistics
	queryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
	// Pipeline counters to be returned for this pool
	queryPoolInfo.pipelineStatistics =
		VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;
	queryPoolInfo.queryCount = query_count;
	ASSERT_VULKAN(vkCreateQueryPool(device->getLogicalDevice(), &queryPoolInfo, NULL, &queryPool),
									"Failed to create query pool!");

}

void PathTracing::createPipeline(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
{

	VkPushConstantRange push_constant_range{};
	push_constant_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	push_constant_range.offset = 0;
	push_constant_range.size = sizeof(PushConstantPathTracing);

	VkPipelineLayoutCreateInfo compute_pipeline_layout_create_info{};
	compute_pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	compute_pipeline_layout_create_info.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	compute_pipeline_layout_create_info.pushConstantRangeCount = 1;
	compute_pipeline_layout_create_info.pPushConstantRanges = &push_constant_range;
	compute_pipeline_layout_create_info.pSetLayouts = descriptorSetLayouts.data();

	ASSERT_VULKAN(vkCreatePipelineLayout(device->getLogicalDevice(), &compute_pipeline_layout_create_info, nullptr,
		&pipeline_layout), "Failed to create compute path tracing pipeline layout!");

	// create pipeline 
	std::stringstream pathTracing_shader_dir;
	pathTracing_shader_dir << CMAKELISTS_DIR;
	pathTracing_shader_dir << "/Resources/Shader/path_tracing/";

	std::string pathTracing_shader = "path_tracing.comp";

	ShaderHelper shaderHelper;
	File pathTracingShaderFile(shaderHelper.getShaderSpvDir(pathTracing_shader_dir.str(), pathTracing_shader));
	std::vector<char> pathTracingShadercode = pathTracingShaderFile.readCharSequence();

	shaderHelper.compileShader(pathTracing_shader_dir.str(), pathTracing_shader);

	// build shader modules to link to graphics pipeline
	VkShaderModule pathTracingModule = shaderHelper.createShaderModule(device, pathTracingShadercode);

	// Specialization constant for workgroup size
	std::array<VkSpecializationMapEntry, 3> specEntries;

	specEntries[0].constantID = 0;
	specEntries[0].size = sizeof(specializationData.specWorkGroupSizeX);
	specEntries[0].offset = 0;

	specEntries[1].constantID = 1;
	specEntries[1].size = sizeof(specializationData.specWorkGroupSizeY);
	specEntries[1].offset = offsetof(SpecializationData, specWorkGroupSizeY);

	specEntries[2].constantID = 2;
	specEntries[2].size = sizeof(specializationData.specWorkGroupSizeZ);
	specEntries[2].offset = offsetof(SpecializationData, specWorkGroupSizeZ);

	VkSpecializationInfo specInfo{};
	specInfo.dataSize = sizeof(specializationData);
	specInfo.mapEntryCount = static_cast<uint32_t>(specEntries.size());
	specInfo.pMapEntries = specEntries.data();
	specInfo.pData = &specializationData;

	VkPipelineShaderStageCreateInfo compute_shader_integrate_create_info{};
	compute_shader_integrate_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	compute_shader_integrate_create_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	compute_shader_integrate_create_info.module = pathTracingModule;
	compute_shader_integrate_create_info.pSpecializationInfo = &specInfo;
	compute_shader_integrate_create_info.pName = "main";

	// -- COMPUTE PIPELINE CREATION --
	VkComputePipelineCreateInfo compute_pipeline_create_info{};
	compute_pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	compute_pipeline_create_info.stage = compute_shader_integrate_create_info;
	compute_pipeline_create_info.layout = pipeline_layout;
	compute_pipeline_create_info.flags = 0;
	// create compute pipeline 
	ASSERT_VULKAN(vkCreateComputePipelines(	device->getLogicalDevice(), VK_NULL_HANDLE, 1,
											&compute_pipeline_create_info, nullptr,
											&pipeline),
											"Failed to create a compute pipeline!");

	// Destroy shader modules, no longer needed after pipeline created
	vkDestroyShaderModule(device->getLogicalDevice(), pathTracingModule, nullptr);

}
