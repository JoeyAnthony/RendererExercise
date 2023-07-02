#include "image_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <vulkan/vulkan.hpp>

#include "vk_helper_functions.h"

void CreateTextureImage(VkDevice vulkan_device, VkPhysicalDevice selected_device)
{
	int width, height, channels;
	stbi_uc* buffer = stbi_load("textures/statue.jpg", &width, &height, &channels, STBI_rgb_alpha);
	
	VkDeviceSize image_size = width * height * 4; // 4 is same as channels?

	// Create staging buffer
	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	CreateBuffer(vulkan_device, selected_device, image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, staging_buffer, staging_buffer_memory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Copy image data
	void* data;
	vkMapMemory(vulkan_device, staging_buffer_memory, 0, image_size, 0, &data);
	memcpy(data, buffer, image_size);

	vkUnmapMemory(vulkan_device, staging_buffer_memory);
	stbi_image_free(buffer);
}
