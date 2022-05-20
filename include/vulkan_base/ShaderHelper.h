#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "VulkanDevice.h"

class ShaderHelper
{
public:

	ShaderHelper();

	void			compileShader(std::string shader_src_dir, std::string shader_name);
	std::string		getShaderSpvDir(std::string shader_src_dir, std::string shader_name);

	VkShaderModule	createShaderModule(VulkanDevice* device, const std::vector<char>& code);

	~ShaderHelper();

private:

	std::string target = " --target-env=vulkan1.3 ";

};

