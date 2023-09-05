#pragma once
#include "vulkan/vulkan.hpp"

// Loads texture in host visible memory
size_t LoadTexture(const VkDevice& vulkan_device, const VkPhysicalDevice& selected_device, VkBuffer& buffer, VkDeviceMemory& device_memory, int& width, int& height, int& channels);
