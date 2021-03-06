#pragma once
#include <string>
#include <filesystem>
#include <vector>
#include "VulkanRendererConfig.hpp"

namespace ShaderIncludes{

	std::vector<std::string> shaderIncludeRelativeResourcesPaths = 
										{"Shaders/brdf/",
										"Shaders/common/",
										"Shaders/hostDevice/",
										"renderer/",
										"renderer/pushConstants/",
										};

	std::vector<std::string> shaderIncludeRelativeIncludePaths = 
										{"renderer/",
										"renderer/pushConstants/",
										"scene/"
										};

	std::string getShaderIncludes() {
		std::stringstream shaderIncludes;
		std::filesystem::path cwd = std::filesystem::current_path();
		for(int i = 0; i < shaderIncludeRelativeResourcesPaths.size(); i++) {
			shaderIncludes << " -I ";
			shaderIncludes << cwd.string();
			shaderIncludes << RELATIVE_RESOURCE_PATH;
			shaderIncludes << shaderIncludeRelativeResourcesPaths[i];

		}

		for(int i = 0; i < shaderIncludeRelativeIncludePaths.size(); i++) {
			shaderIncludes << " -I ";
			shaderIncludes << cwd.string();
			shaderIncludes << RELATIVE_INCLUDE_PATH;
			shaderIncludes << shaderIncludeRelativeIncludePaths[i];

		}

		return shaderIncludes.str();
	}

}
