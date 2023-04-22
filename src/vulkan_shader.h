#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <vulkan/vulkan.hpp>

#include "logger.h"


class VulkanShaderLoader {
	std::vector<char> bytes;
	VkShaderModuleCreateInfo create_info;
	std::vector<VkShaderModule> shader_modules;

public:
	std::vector<char> LoadShader(std::string path);
	VkShaderModule CreateShaderModule(std::vector<char>& bytes, VkDevice& device, const VkAllocationCallbacks* pAllocator);
	void DestroyCreatedShaderModules(VkDevice& device, const VkAllocationCallbacks* pAllocator);

	VulkanShaderLoader();
};
