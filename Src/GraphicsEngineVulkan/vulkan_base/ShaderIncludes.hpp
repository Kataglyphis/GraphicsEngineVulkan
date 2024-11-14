#pragma once
#include "VulkanRendererConfig.hpp"
#include <filesystem>
#include <string>
#include <vector>
#include "spdlog/spdlog.h"

namespace ShaderIncludes {

std::vector<std::string> shaderIncludeRelativeResourcesPaths = {
    "Shaders/pbr/",
    "Shaders/pbr/brdf/",
    "Shaders/common/",
    "Shaders/hostDevice/",
};

std::vector<std::string> shaderIncludeRelativeIncludePaths = { "renderer/", "renderer/pushConstants/", "scene/" };

std::string getShaderIncludes()
{
    std::stringstream shaderIncludes;
    std::filesystem::path cwd = std::filesystem::current_path();
    for (int i = 0; i < shaderIncludeRelativeResourcesPaths.size(); i++) {
        shaderIncludes << " -I ";
        shaderIncludes << cwd.string();
        shaderIncludes << RELATIVE_RESOURCE_PATH;
        shaderIncludes << shaderIncludeRelativeResourcesPaths[i];
    }

    for (int i = 0; i < shaderIncludeRelativeIncludePaths.size(); i++) {
        shaderIncludes << " -I ";
        shaderIncludes << cwd.string();
        shaderIncludes << RELATIVE_INCLUDE_PATH;
        shaderIncludes << shaderIncludeRelativeIncludePaths[i];
    }

    spdlog::info("The shader includes are the following: {}", shaderIncludes.str());

    return shaderIncludes.str();
}

}// namespace ShaderIncludes
