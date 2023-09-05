#include "vk_helper_functions.h"
#include "logger.h"

uint32_t FindMemoryTypes(VkPhysicalDevice selected_device, uint32_t type_filter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties props{};
    vkGetPhysicalDeviceMemoryProperties(selected_device, &props);

    for (int i = 0; i < props.memoryTypeCount; i++) {
        // Check if the bitfield of the type filter corresponds to the index
        if ((type_filter & (1 << i)) &&
            // AND the property flags with the expected properties, then check if the resulting value is what we want
            // Making sure we don't just look for a non-zero value
            (properties & props.memoryTypes[i].propertyFlags) == properties) {
            return i;
        }
    }

    return 0;
}

void CreateBuffer(VkDevice vulkan_device, VkPhysicalDevice selected_device, VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VkDeviceMemory& memory, VkMemoryPropertyFlags memory_properties, VkAllocationCallbacks* p_allocate_info)
{
    VkBufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = size;
    create_info.usage = usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult res = vkCreateBuffer(vulkan_device, &create_info, nullptr, &buffer);
    if (res != VK_SUCCESS) {
        LOG << "FAILURE\t Failed creating vertex buffer, error: " << res;
    }

    // Get memory requirements
    VkMemoryRequirements mem_requirements{};
    vkGetBufferMemoryRequirements(vulkan_device, buffer, &mem_requirements);

    // Allocate memory for the buffer
    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = FindMemoryTypes(selected_device, mem_requirements.memoryTypeBits, memory_properties);

    res = vkAllocateMemory(vulkan_device, &alloc_info, p_allocate_info, &memory);
    if (res != VK_SUCCESS) {
        LOG << "FAILURE\t Failed to allocate buffer memory";
        return;
    }

    // Maybe make offset a parameter later for multiple buffer allocation in the same memory space
    vkBindBufferMemory(vulkan_device, buffer, memory, 0);
}

void CmdCopyBuffer(VkCommandBuffer cmd_buffer, VkBuffer src, VkBuffer dst, VkDeviceSize size)
{
    VkBufferCopy copy_region{};
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = size;

    vkCmdCopyBuffer(cmd_buffer, src, dst, 1, &copy_region);
}

void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memory_property_flags, VkDevice& vulkan_device, VkPhysicalDevice& selected_device, VkImage& image, VkDeviceMemory& memory)
{
    VkImageCreateInfo image_create{};
    image_create.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create.imageType = VK_IMAGE_TYPE_2D;
    image_create.extent.width = width;
    image_create.extent.height = height;
    image_create.extent.depth = 1;
    image_create.mipLevels = 1;
    image_create.arrayLayers = 1;
    image_create.format = format;
    image_create.tiling = tiling;
    image_create.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    image_create.usage = usage;
    if (usage == 0) {
        image_create.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    image_create.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create.flags = 0;

    VkResult res = vkCreateImage(vulkan_device, &image_create, nullptr, &image);
    if (res != VK_SUCCESS) {
        LOG << "ERROR\t Failed to create image, error " << res;
    }

    VkMemoryRequirements mem_requirements{};
    vkGetImageMemoryRequirements(vulkan_device, image, &mem_requirements);

    VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = mem_requirements.size;
    allocate_info.memoryTypeIndex = FindMemoryTypes(selected_device, mem_requirements.memoryTypeBits, memory_property_flags);

    res = vkAllocateMemory(vulkan_device, &allocate_info, nullptr, &memory);
    if (res != VK_SUCCESS) {
        LOG << "ERROR\t Failed allocating image memory " << res;
    }

    vkBindImageMemory(vulkan_device, image, memory, 0);
}

void CmdTransitionImageLayout(VkCommandBuffer cmd_buffer, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags stage_before = 0;
    VkPipelineStageFlags stage_after = 0;
    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        stage_before = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        stage_after = VK_PIPELINE_STAGE_TRANSFER_BIT;

        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        stage_before = VK_PIPELINE_STAGE_TRANSFER_BIT;
        stage_after = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    }

    vkCmdPipelineBarrier(cmd_buffer,
        stage_before,     // Before and after pipeline stages
        stage_after,    // Per region condition
        0,
        0, nullptr, // Memory Barriers
        0, nullptr, // Buffer Memory Barriers
        1, &barrier // Image Memory barriers
    );
}

void CmdCopyBufferToImage(VkCommandBuffer cmd_buffer, VkBuffer src, VkImage dst, uint32_t width, uint32_t height)
{
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageSubresource.mipLevel = 0;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };

    vkCmdCopyBufferToImage(cmd_buffer, src, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

}

VkCommandBuffer BeginSingleTimeCommandBuffer(VkDevice vulkan_device, VkCommandPool cmd_pool)
{
    // Usually, creating a separate command pool for short lived command buffers can help with memory optimizations
// Use the flag VK_COMMAND_POOL_CREATE_TRANSIENT_BIT when creating a command pool in that case

// Create command buffer to make transfer calls
    VkCommandBufferAllocateInfo command_buffer_info{};
    command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_info.commandBufferCount = 1;
    command_buffer_info.commandPool = cmd_pool;
    command_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer cmd_buffer{};
    VkResult res = vkAllocateCommandBuffers(vulkan_device, &command_buffer_info, &cmd_buffer);
    if (res != VK_SUCCESS) {
        LOG << "FAILURE\t Couldn't allocate command buffer for copying";
    }

    VkCommandBufferBeginInfo cmd_transfer_begin{};
    cmd_transfer_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmd_transfer_begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Single call for allocation, so specify to the driver
    cmd_transfer_begin.pInheritanceInfo = nullptr;

    // Start command buffer recording with BeginCommandBuffer
    res = vkBeginCommandBuffer(cmd_buffer, &cmd_transfer_begin);
    if (res != VK_SUCCESS) {
        LOG << "FAILURE\t Failed creating command buffer for copying";
    }

    return cmd_buffer;
}

void EndSingleTimeCommandBuffer(VkDevice vulkan_device, VkQueue graphics_queue, VkCommandPool cmd_pool, const VkCommandBuffer& cmd_buffer)
{
    vkEndCommandBuffer(cmd_buffer);

    // Submit the command pool
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd_buffer;

    // Because we just wait for the queue to finish, we don't need syncing objects
    vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue);

    vkFreeCommandBuffers(vulkan_device, cmd_pool, 1, &cmd_buffer);
}