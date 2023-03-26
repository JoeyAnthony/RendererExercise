#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_structs.hpp>

class VulkanGraphics {
	VkInstance instance;

	std::vector<const char*> validation_layers = { "VK_LAYER_KHRONOS_validation" };

public:
	bool Initialize();
	bool CheckValidationLayerSupport();
	bool EnableValidationLayers();
	void Edulcorate();

	VulkanGraphics();
	~VulkanGraphics();
};
