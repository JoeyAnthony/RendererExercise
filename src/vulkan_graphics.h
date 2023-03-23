#pragma once
#include <vulkan/vulkan.hpp>

class VulkanGraphics {
	VkInstance instance;

public:
	bool Initialize();
	void Edulcorate();

	VulkanGraphics();
	~VulkanGraphics();
};
