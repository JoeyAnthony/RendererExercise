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

VkImageView CreateImageView(VkDevice vulkan_device, VkImage image, VkFormat format, VkImageViewType view_type, VkImageAspectFlags aspect_flags)
{
	VkImageViewCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	create_info.image = image;
	create_info.format = format;
	create_info.viewType = view_type;
	
	// Don't have to set these here, they are defined as 0 by default
	//create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	//create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	//create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	//create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    create_info.subresourceRange.aspectMask = aspect_flags;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.layerCount = 1;
    create_info.subresourceRange.levelCount = 1;

	VkImageView image_view;
	VkResult res = vkCreateImageView(vulkan_device, &create_info, nullptr, &image_view);
	if (res != VK_SUCCESS) {
		LOG << "FAILURE\t Failed create texture image view: " << res;
	}

	return image_view;
}
