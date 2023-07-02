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

void Copy_Buffer(VkDevice vulkan_device, VkCommandPool cmd_pool, VkQueue graphics_queue, VkBuffer src, VkBuffer dst, VkDeviceSize size)
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

    VkBufferCopy copy_region{};
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = size;

    vkCmdCopyBuffer(cmd_buffer, src, dst, 1, &copy_region);
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