#pragma once
#include "vulkan/vulkan.hpp"

// Loads texture in host visible memory
size_t LoadTexture(const VkDevice& vulkan_device, const VkPhysicalDevice& selected_device, VkBuffer& buffer, VkDeviceMemory& device_memory, int& width, int& height, int& channels);

VkImageView CreateImageView(VkDevice vulkan_device, VkImage image, VkFormat format,
	VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D, VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT);
