#include "vulkan_shader.h"

VulkanShaderLoader::VulkanShaderLoader()
{
}

std::vector<char> VulkanShaderLoader::LoadShader(std::string path)
{
	std::ifstream read_stream(path, std::ios::ate | std::ios::binary);
	if (!read_stream.is_open()) {
		LOG << "Couldn't open shader file: " << path;
		return std::vector<char>();
	}

	uint64_t buffer_size = (uint64_t)read_stream.tellg();
	read_stream.seekg(0);
	std::vector<char> bytes(buffer_size);
	read_stream.read(bytes.data(), buffer_size);

	return bytes;
}

VkShaderModule VulkanShaderLoader::CreateShaderModule(std::vector<char>& bytes, VkDevice& device, const VkAllocationCallbacks* pAllocator)
{
	VkShaderModuleCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = bytes.size();
	create_info.pCode = reinterpret_cast<uint32_t*>(bytes.data());

	VkShaderModule module;
	if (vkCreateShaderModule(device, &create_info, pAllocator, &module) == VK_SUCCESS) {
		shader_modules.push_back(module);

		LOG << "SUCCESS Created shader";
	}
	else {
		LOG << "FAILURE Couldn't create shader";
	}

	return module;
}

void VulkanShaderLoader::DestroyCreatedShaderModules(VkDevice& device, const VkAllocationCallbacks* pAllocator)
{
	for (VkShaderModule& module : shader_modules) {
		vkDestroyShaderModule(device, module, pAllocator);
	}
}
