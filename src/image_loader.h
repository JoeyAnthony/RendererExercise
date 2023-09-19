#pragma once
#include "vulkan/vulkan.hpp"

// Loads texture in host visible memory
size_t LoadTexture(const VkDevice& vulkan_device, const VkPhysicalDevice& selected_device, VkBuffer& buffer, VkDeviceMemory& device_memory, int& width, int& height, int& channels);

VkImageView CreateImageView(VkDevice vulkan_device, VkImage image, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB,
	VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D, const VkImageSubresourceRange* sub_resource_range = nullptr);
