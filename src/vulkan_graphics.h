#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_structs.hpp>

class VulkanGraphics {
	VkInstance instance_;
	std::vector<const char*> validation_layers_ = { "VK_LAYER_KHRONOS_validation" };
	VkPhysicalDevice selected_device_ = VK_NULL_HANDLE;

	std::vector<const char*> validation_layers = { "VK_LAYER_KHRONOS_validation" };

public:
	bool Initialize();
	bool CheckValidationLayerSupport();
	bool EnableValidationLayers();
	void Edulcorate();
	void PopulateDebugMessengerCreateInfoStruct(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void selectPhysicalDevice();

	VulkanGraphics();
	~VulkanGraphics();
};
