#include "image_loader.h"
#include "logger.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <vulkan/vulkan.hpp>

#include "vk_helper_functions.h"

size_t LoadTexture(const VkDevice& vulkan_device, const VkPhysicalDevice& selected_device, VkBuffer& buffer, VkDeviceMemory& device_memory, int& width, int& height, int& channels)
{
	stbi_uc* stbi_im = stbi_load("../textures/statue.jpg", &width, &height, &channels, STBI_rgb_alpha);
	if (!stbi_im) {
		LOG << "ERROR\t image not loaded";
		return 0;
	}
	
	VkDeviceSize image_size = width * height * STBI_rgb_alpha; // 4 is same as channels?

	// Create staging buffer
	CreateBuffer(vulkan_device, selected_device, image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, buffer, device_memory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Copy image data
	void* data;
	vkMapMemory(vulkan_device, device_memory, 0, image_size, 0, &data);
	memcpy(data, stbi_im, image_size);

	vkUnmapMemory(vulkan_device, device_memory);
	stbi_image_free(stbi_im);

	return image_size;
}
