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

void AllocateGPUMemory(VkDevice vulkan_device, VkPhysicalDevice selected_device, VkBuffer buffer, VkDeviceSize size, VkDeviceMemory& memory, VkMemoryPropertyFlags memory_properties, VkAllocationCallbacks* p_allocate_info)
{
    // Get memory requirements
    VkMemoryRequirements mem_requirements{};
    vkGetBufferMemoryRequirements(vulkan_device, buffer, &mem_requirements);

    // Allocate memory for the buffer
    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = FindMemoryTypes(selected_device, mem_requirements.memoryTypeBits, memory_properties);

    VkResult res = vkAllocateMemory(vulkan_device, &alloc_info, p_allocate_info, &memory);
    if (res != VK_SUCCESS) {
        LOG << "FAILURE\t Failed to allocate buffer memory";
        return;
    }
}

bool FormatHasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D16_UNORM_S8_UINT;
}

void CmdGenerateMipmaps(VkCommandBuffer cmd_buffer, VkPhysicalDevice physical_device, VkFormat format, VkImage image, uint32_t width, uint32_t height, uint32_t mip_levels)
{
    VkFormatProperties format_properties;
    vkGetPhysicalDeviceFormatProperties(physical_device, format, &format_properties);
    if(!(format_properties.optimalTilingFeatures | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
    {
        LOG << "FAILURE\t Device doesn't support linear filtering";
        return;
    }

    VkImageMemoryBarrier img_barrier{};
    img_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    img_barrier.image = image;
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    int32_t mip_width = width;
    int32_t mip_height = height;
    // Skip the main texture in the loop
    for (int32_t i = 1; i < mip_levels; i++) {
        // Transition previous texture to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
        img_barrier.subresourceRange.baseMipLevel = i - 1;
        img_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        img_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        img_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &img_barrier);

        VkImageBlit blit_info{};
        blit_info.srcOffsets[0] = { 0, 0, 0 };
        blit_info.srcOffsets[1] = { mip_width, mip_height, 1 };
        blit_info.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit_info.srcSubresource.baseArrayLayer = 0;
        blit_info.srcSubresource.layerCount = 1;
        blit_info.srcSubresource.mipLevel = i - 1;

        // Make destination mip resolution half of the source mip
        mip_width = mip_width > 1 ? mip_width / 2 : 1;
        mip_height = mip_height > 1 ? mip_height / 2 : 1;

        blit_info.dstOffsets[0] = { 0, 0, 0 };
        blit_info.dstOffsets[1] = { mip_width, mip_height, 1 };
        blit_info.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit_info.dstSubresource.baseArrayLayer = 0;
        blit_info.dstSubresource.layerCount = 1;
        blit_info.dstSubresource.mipLevel = i;

        vkCmdBlitImage(cmd_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit_info, VK_FILTER_LINEAR);

        // Needs barriers after each blit to make the currently blitted image a transfer src
    }

    // Transition last mip from transfer_dst to transfer_src for consistency.
    img_barrier.subresourceRange.baseMipLevel = mip_levels - 1;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    img_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &img_barrier);
}

void CmdCopyBuffer(VkCommandBuffer cmd_buffer, VkBuffer src, VkBuffer dst, VkDeviceSize size)
{
    VkBufferCopy copy_region{};
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = size;

    vkCmdCopyBuffer(cmd_buffer, src, dst, 1, &copy_region);
}

void CreateImage(uint32_t width, uint32_t height, uint32_t mip_levels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memory_property_flags, VkDevice& vulkan_device, VkPhysicalDevice& selected_device, VkImage& image, VkDeviceMemory& memory)
{
    VkImageCreateInfo image_create{};
    image_create.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create.imageType = VK_IMAGE_TYPE_2D;
    image_create.extent.width = width;
    image_create.extent.height = height;
    image_create.extent.depth = 1;
    image_create.mipLevels = mip_levels;
    image_create.arrayLayers = 1;
    image_create.format = format;
    image_create.tiling = tiling;
    image_create.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    image_create.usage = usage;
    if (usage == 0) {
        // Transfer src and destination for 
        image_create.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
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

void CmdTransitionImageLayout(VkCommandBuffer cmd_buffer, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // Ignored because we usually don't want to change the queue family
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;  // Offset of the mipmap levels
    barrier.subresourceRange.levelCount = mip_levels;    // Amount of mipmap levels to change
    barrier.subresourceRange.baseArrayLayer = 0;// Offset of the array
    barrier.subresourceRange.layerCount = 1;    // Amount of layers to change in the array

    // Pipeline stage usage before and after. So which stages of the pipeline was it used and in which is the new one?
    VkPipelineStageFlags src_stage_mask = 0;
    VkPipelineStageFlags dst_stage_mask = 0;
    if(new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if(FormatHasStencilComponent(format))
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else
    {
        // Put it to color aspect mask like at the top of the function
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        //
        src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;

        // Sets which attachment the render pass should access
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        src_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        src_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    }
    else if(old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        // Add stencil component if the format has one
        if(FormatHasStencilComponent(format))
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }

        src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage_mask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }

    vkCmdPipelineBarrier(cmd_buffer,
        src_stage_mask,     // All stages before this stage has to execute
        dst_stage_mask,    // All work after this stage has to wait
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
