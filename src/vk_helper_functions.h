#pragma once

#include "vulkan/vulkan.hpp"
#include <optional>

// Specifies queue support for a queue family
struct QueueFamilyIndices {
    std::optional<uint32_t> graphics_index;
    std::optional<uint32_t> present_index;

    bool IsComplete() {
        bool success = graphics_index.has_value();
        success &= present_index.has_value();
        return success;
    }
};

struct SwapChainDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

struct DeviceQueues {
    VkQueue graphics_queue;
    VkQueue present_queue;
};

struct BP_SwapchainInfo {
    VkSwapchainKHR swapchain;
    VkFormat format;
    VkExtent2D extent;
    std::vector<VkImage> images;
    std::vector<VkImageView> image_views;
    std::vector<VkFramebuffer> framebuffers;
};

uint32_t FindMemoryTypes(VkPhysicalDevice selected_device, uint32_t type_filter, VkMemoryPropertyFlags properties);

void CreateBuffer(VkDevice vulkan_device, VkPhysicalDevice selected_device, VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VkDeviceMemory& memory, VkMemoryPropertyFlags memory_properties, VkAllocationCallbacks* p_allocate_info = nullptr);

void CmdCopyBuffer(VkCommandBuffer cmd_buffer, VkBuffer src, VkBuffer dst, VkDeviceSize size);

void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memory_property_flags, VkDevice& vulkan_device, VkPhysicalDevice& selected_device, VkImage& image, VkDeviceMemory& memory);

void CmdTransitionImageLayout(VkCommandBuffer cmd_buffer, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);

void CmdCopyBufferToImage(VkCommandBuffer cmd_buffer, VkBuffer src, VkImage dst, uint32_t width, uint32_t height);

VkCommandBuffer BeginSingleTimeCommandBuffer(VkDevice vulkan_device, VkCommandPool cmd_pool);
void EndSingleTimeCommandBuffer(VkDevice vulkan_device, VkQueue graphics_queue, VkCommandPool cmd_pool, const VkCommandBuffer& cmd_buffer);