#include "vulkan_graphics.h"
#include <GLFW/glfw3.h>

#include <map>
#include <set>
#include <string>

#include "logger.h"
#include "vulkan_shader.h"
#include "geometry-helpers.h"
#include "image_loader.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    if (messageSeverity >= VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        BP_LOG(LogSeverity::BP_DEBUG, LogColor::PURPLE) << pCallbackData->pMessage;
    }

    // Returning true is only used wen testing the validation layers themselves
    return VK_FALSE;
}

bool VulkanGraphics::AreExtensionsAvailable(const std::vector<const char*>& extensions) {
    LOG << "Enumerate VULKAN extensions";
    // Enumerate optional extensions, only checks glfw extensions
    std::set <std::string> to_check(extensions.begin(), extensions.end());
    uint32_t enumerated_extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &enumerated_extension_count, nullptr);

    if (enumerated_extension_count > 0) {
        std::vector <VkExtensionProperties> enumerated_extensions(enumerated_extension_count);
        vkEnumerateInstanceExtensionProperties(nullptr, &enumerated_extension_count, enumerated_extensions.data());

        uint32_t extensions_count = extensions.size();
        for (int j = 0; j < extensions_count; j++) {

            bool extension_found = false;
            for (int i = 0; i < enumerated_extension_count; i++) {

                if (std::string(enumerated_extensions[i].extensionName).compare(extensions[j]) == 0) {
                    LOG << "FOUND\t" << enumerated_extensions[i].extensionName;
                    extension_found = true;
                    to_check.erase(enumerated_extensions[i].extensionName);
                    break;
                }
            }
        }
    }
    else {
        LOG << "ERROR\t Could not enumerate supported extensions";
        return false;
    }

    uint32_t to_check_size = to_check.size();
    if (to_check_size > 0) {
        for (std::string ext : to_check) {
            LOG << "UNSUPPORTED\t extension is not supported: " << ext;
        }

        return false;
    }

    //if (glfw_extensions_found == glfw_extension_count) {
    //	LOG << "All GLFW extensions found. Found: " << glfw_extension_count;
    //}
    //else {
    //	LOG << "Not all GLFW extensions found. Found: " << glfw_extension_count;
    //}
    return true;
}

bool VulkanGraphics::Initialize()
{
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Vulkan exercise";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "Backpack";
    app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    app_info.apiVersion = VK_API_VERSION_1_3;

    // Create debug messaging struct
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
    PopulateDebugMessengerCreateInfoStruct(debug_create_info);

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    // Set debug create info struct
    create_info.pNext = &debug_create_info;

    LOG; //insert new line
    LOG << "Enable VULKAN extensions";
    // All required extensions
    std::vector<const char*> required_extensions = GetRequiredInstanceExtensions();

    // Set extension count info
    create_info.enabledExtensionCount = required_extensions.size();
    create_info.ppEnabledExtensionNames = required_extensions.data();

    LOG; //insert new line
    if (!AreExtensionsAvailable(required_extensions)) {
        LOG << "FAILURE\t Required extensions missing";
        return false;
    }

    // Set validation layers inside create_info struct
    if (enable_validation_layers_) {
        LOG;
        SetValidationLayers(create_info);
    }

    LOG; //insert new line
    LOG << "Create VULKAN Instance";
    // Create vulkan instance
    VkResult res = vkCreateInstance(&create_info, nullptr, &instance_);
    if (res != VK_SUCCESS) {
        LOG << "Vulkan initialization error: " << res;
        return false;
    }

    LOG;
    LOG << "SUCCESS\t VULKAN instance created";
    return true;
}

bool VulkanGraphics::CheckValidationLayerSupport()
{
    LOG << "Check validation layer support";
    uint32_t available_count;
    uint32_t validation_count = validation_layers_.size();
    vkEnumerateInstanceLayerProperties(&available_count, nullptr);

    VkLayerProperties* available_layers = new VkLayerProperties[available_count];
    vkEnumerateInstanceLayerProperties(&available_count, available_layers);

    uint32_t found_layer_count = 0;
    for (int validation_index = 0; validation_index < validation_count; validation_index++) {
        bool layer_found = false;
        for (int available_index = 0; available_index < available_count; available_index++) {
            // Check if layer name exists
            if (std::string(validation_layers_[validation_index]).compare(available_layers[available_index].layerName)) {
                LOG << "FOUND\t " << validation_layers_[validation_index];
                layer_found = true;
                found_layer_count++;
            }
            break;
        }

        if (!layer_found) {
            LOG << validation_layers_[validation_index] << "\tMISSING";
        }
    }

    delete[] available_layers;
    if (found_layer_count == validation_count) {
        return true;
    }
    else{
        return false;
    }
}

bool VulkanGraphics::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> extension_properties(extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, extension_properties.data());

    // Remove all available extensions from the set, if the set is empty, return true
    // Copy the list in a set to get a unique list
    std::set<std::string> unique_required_device_extensions(required_device_extensions_.begin(), required_device_extensions_.end());
    for (VkExtensionProperties& ext_property : extension_properties) {
        unique_required_device_extensions.erase(ext_property.extensionName);
    }

    return unique_required_device_extensions.empty();
}

void VulkanGraphics::DestroySwapchain()
{
    if (swapchain_data_.swapchain) {
        vkDestroySwapchainKHR(vulkan_device_, swapchain_data_.swapchain, nullptr);
    }

    for (auto image_view : swapchain_data_.image_views) {
        vkDestroyImageView(vulkan_device_, image_view, nullptr);
    }

    for (auto framebuffer : swapchain_data_.framebuffers) {
        vkDestroyFramebuffer(vulkan_device_, framebuffer, nullptr);
    }
}

void VulkanGraphics::SetValidationLayers(VkInstanceCreateInfo &create_info)
{
    //Enable validation Layers
    create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers_.size());
    create_info.ppEnabledLayerNames = validation_layers_.data();
    LOG << "Validation layers set";
}

void VulkanGraphics::Edulcorate()
{
    DestroySwapchain();

    if (vulkan_surface_) {
        vkDestroySurfaceKHR(instance_, vulkan_surface_, nullptr);
    }

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyFence(vulkan_device_, fence_in_flight_[i], nullptr);
        vkDestroySemaphore(vulkan_device_, sem_image_available_[i], nullptr);
        vkDestroySemaphore(vulkan_device_, sem_render_finished_[i], nullptr);
    }

    vkDestroyRenderPass(vulkan_device_, render_pass_, nullptr);
    vkDestroyPipeline(vulkan_device_, pipeline_, nullptr);
    vkDestroyPipelineLayout(vulkan_device_, pipeline_layout_, nullptr);
    DestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);
    vkDestroyDescriptorSetLayout(vulkan_device_, descriptor_set_layout_, nullptr);

    // Destroy vertex and index buffers
    vkDestroyBuffer(vulkan_device_, triangle_buffer_, nullptr);
    vkFreeMemory(vulkan_device_, triangle_buffer_memory_, nullptr);
    vkDestroyBuffer(vulkan_device_, index_buffer_, nullptr);
    vkFreeMemory(vulkan_device_, index_buffer_memory_, nullptr);

    // Destroy images
    vkDestroyImage(vulkan_device_, texture_image_, nullptr);
    vkFreeMemory(vulkan_device_, texture_image_memory_, nullptr);

    // Destroy uniform buffers
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(vulkan_device_, index_buffer_, nullptr);
        vkFreeMemory(vulkan_device_, index_buffer_memory_, nullptr);
    }

    vkDestroyDescriptorPool(vulkan_device_, descriptor_pool_, nullptr);

    // Destroy unform buffer shader layout
    vkDestroyDescriptorSetLayout(vulkan_device_, descriptor_set_layout_, nullptr);

    vkDestroyDevice(vulkan_device_, nullptr);
    vkDestroyInstance(instance_, nullptr);

    vkDeviceWaitIdle(vulkan_device_);
}

void VulkanGraphics::CreateVulkanSurface(const WindowData& window_data)
{
    VkWin32SurfaceCreateInfoKHR surface_create_info{};
    surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_create_info.hinstance = window_data.hinstance;
    surface_create_info.hwnd = window_data.hwnd;

    VkResult res = vkCreateWin32SurfaceKHR(instance_, &surface_create_info, nullptr, &vulkan_surface_);
    if (res == VK_SUCCESS) {
        LOG << "SUCCESS\t Create Vulkan surface";
    }
    else {
        LOG << "FAILURE\t Create Vulkan surface failed with error: " << res;
    }
}

std::vector<const char*> VulkanGraphics::GetRequiredInstanceExtensions()
{
    uint32_t glfw_extension_count = 0;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    // Add debug messages when validation layers are enabled
    std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
    if (enable_validation_layers_) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    // Add multiplatform extensions (MacOS)
    //required_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    //create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    return extensions;
}

void VulkanGraphics::EnableVulkanDebugMessages()
{
    if (!enable_validation_layers_) {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT message_create{};
    PopulateDebugMessengerCreateInfoStruct(message_create);

    if (CreateDebugUtilsMessengerEXT(instance_, &message_create, nullptr, &debug_messenger_) == VK_SUCCESS)
    {
        LOG << "SUCCESS\t Enabled Vulkan debug messaging";
    }
    else {
        LOG_S(LogSeverity::BP_ERROR) << "FAILURE\t Failed to enable Vulkan debug messaging";
    }
}

void VulkanGraphics::PopulateDebugMessengerCreateInfoStruct(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
    createInfo.pUserData = nullptr;

    //createInfo.pfnUserCallback;
}

void VulkanGraphics::SelectPhysicalDevice()
{
    LOG << "Searching for physical device";

    uint32_t device_count;
    vkEnumeratePhysicalDevices(instance_, &device_count, nullptr);

    std::vector<VkPhysicalDevice> found_devices(device_count);
    vkEnumeratePhysicalDevices(instance_, &device_count, found_devices.data());

    std::multimap<uint32_t, VkPhysicalDevice> device_scores;

    for (int i = 0; i < device_count; i++) {
        VkPhysicalDevice device = found_devices[i];
        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(device, &device_properties);

        uint32_t score = DetermineDeviceScore(device);

        if (!IsDeviceSuitable(device)) {
            LOG << device_properties.deviceName << score << "\tUNSUITABLE";
            score = 0;
        }

        device_scores.insert({ score, device });
        LOG << "FOUND\t Physical device "  << device_properties.deviceName << " score: " << score;
    }

    selected_device_ = device_scores.begin()->second;

    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(selected_device_, &device_properties);
    LOG << "SELECTED\t Physical device" << device_properties.deviceName;
}

QueueFamilyIndices VulkanGraphics::FindQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices {};
    uint32_t queue_count;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, nullptr);

    std::vector<VkQueueFamilyProperties> family_properties(queue_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, family_properties.data());

    // Loop over queue families and specify their queue indices
    for (int i = 0; i < queue_count; i++) {
        // The indices don't have to be in the same queue family, but if they are, the performance is better
        if (family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_index = i;
        }
        
        VkBool32 present_supported = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vulkan_surface_, &present_supported);
        if (present_supported) {
            indices.present_index = i;
        }
        // Select multiple queue famillies for now to be compatible with the tutorial for swap chain creation
        //break; // Break because on my device (Intel Iris) the first queue family supports both graphics and presentation
    }
    
    return indices;
}

uint32_t VulkanGraphics::DetermineDeviceScore(VkPhysicalDevice device)
{
    // Use VkPhysicalDeviceProperties2 or pNext value
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures device_features;
    VkPhysicalDeviceMemoryProperties memory_properties;

    vkGetPhysicalDeviceProperties(device, &device_properties);
    vkGetPhysicalDeviceFeatures(device, &device_features);
    vkGetPhysicalDeviceMemoryProperties(device, &memory_properties);

    uint32_t score = 0;
    //device_properties.limits.;
    if (device_properties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        score += 100;

    }
    else if (device_properties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
        score += 50;
    }

    return score;
}

bool VulkanGraphics::IsDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = FindQueueFamilies(device);
    bool queues_found = indices.IsComplete();

    bool extensions_supported = CheckDeviceExtensionSupport(device);

    // Only check swapchain if extensions are supported, swapchain support is an extension
    bool swapchain_adequate = false;
    if (extensions_supported) {
        SwapChainDetails sw_details = QuerySwapchainSupport(device);
        swapchain_adequate = !sw_details.formats.empty() && !sw_details.present_modes.empty();
    }
    else {
        LOG << "Extensions not supported, no swapchain support";
    }

    bool success = queues_found && extensions_supported && swapchain_adequate;
    if (!success) {
        LOG << "FAILURE\t Device is not suitable";
    }

    return success;
}

void VulkanGraphics::CreateLogicalDevice()
{
    // Find queue slot to signal what command queues vulkan should create.
    QueueFamilyIndices indices = FindQueueFamilies(selected_device_);
    if (!indices.IsComplete()) {
        LOG << "Required queue families not found for currently selected device";
        //return;
    }

    std::vector<VkDeviceQueueCreateInfo> device_queues;
    std::set<uint32_t> queues_to_create{ indices.graphics_index.value(), indices.present_index.value() };

    // Populate queue creation structs for each queue to be created
    float queue_priority = 1.0f;
    for (uint32_t queue_index: queues_to_create) {
        // Specify struct to create a graphics device queue
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_index;
        queue_create_info.queueCount = 1; // How many queues to create of the type specified by queueFamilyIndex
        queue_create_info.pQueuePriorities = &queue_priority;

        device_queues.push_back(queue_create_info);
    }

    // Specify which features will be used, shaders etc.
    VkPhysicalDeviceFeatures device_features{};

    // Create device create info struct
    VkDeviceCreateInfo device_create_info {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pQueueCreateInfos = device_queues.data();
    device_create_info.queueCreateInfoCount = static_cast<uint32_t>(device_queues.size());
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.ppEnabledExtensionNames = required_device_extensions_.data();
    device_create_info.enabledExtensionCount = required_device_extensions_.size();

    // Only to support older Vulkan versions. In the new versions of Vulkan there is no distinction between instance and device specific validation layers
    if (enable_validation_layers_) {
        device_create_info.ppEnabledLayerNames = validation_layers_.data();
        device_create_info.enabledLayerCount = validation_layers_.size();
    }
    else {
        device_create_info.enabledLayerCount = 0;
    }
    
    // Create Vulkan device that can be used to process data
    VkResult res = vkCreateDevice(selected_device_, &device_create_info, nullptr, &vulkan_device_);
    if (res == VK_SUCCESS) {
        LOG << "SUCCESS\t Create Vulkan device";
    }
    else {
        LOG << "Create Vulkan device FAILED with error: " << res;
    }

    // Get created queues from the device
    vkGetDeviceQueue(vulkan_device_, indices.graphics_index.value(), 0, &device_queues_.graphics_queue);
    vkGetDeviceQueue(vulkan_device_, indices.present_index.value(), 0, &device_queues_.present_queue);
}

void VulkanGraphics::ResizeBuffer(uint32_t width, uint32_t height)
{
    resize_necessary_ = true;
    win_width_ = width;
    win_height_ = height;
}

VkInstance VulkanGraphics::GetVulkanInstance()
{
    return instance_;
}

void VulkanGraphics::SetVulkanSurface(const VkSurfaceKHR& surface)
{
    vulkan_surface_ = surface;
}

SwapChainDetails VulkanGraphics::QuerySwapchainSupport(VkPhysicalDevice device)
{
    SwapChainDetails sw_details {};

    // Get swapchain capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vulkan_surface_, &sw_details.capabilities);

    // Get supported image formats
    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkan_surface_, &format_count, nullptr);

    if (format_count > 0) {
        sw_details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkan_surface_, &format_count, sw_details.formats.data());
    }

    // Get supported present modes
    uint32_t mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkan_surface_, &mode_count, nullptr);
    if (mode_count > 0) {
        sw_details.present_modes.resize(mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkan_surface_, &mode_count, sw_details.present_modes.data());
    }

    return sw_details;
}

VkSurfaceFormatKHR VulkanGraphics::GetPreferredSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& supported_formats)
{
    bool format_set = false;
    VkSurfaceFormatKHR preferred_format;
    for (const VkSurfaceFormatKHR& format : supported_formats) {
        if ((format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) && (format.format == VK_FORMAT_B8G8R8A8_SRGB)) {
            preferred_format = format;
            format_set = true;
            break;
        }
    }

    if (!format_set) {
        preferred_format = supported_formats[0];
        LOG << "FAILURE\t Preferred swapcahin format not found";
    }

    return preferred_format;
}

VkPresentModeKHR VulkanGraphics::GetPreferredSwapchainPresentMode(const std::vector<VkPresentModeKHR>& present_modes)
{
    VkPresentModeKHR preferred_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (const VkPresentModeKHR& mode : present_modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            preferred_mode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
    }

    switch (preferred_mode)
    {
    case VK_PRESENT_MODE_IMMEDIATE_KHR:
        LOG << "Swapchain presentmode set: VK_PRESENT_MODE_IMMEDIATE_KHR";
        break;
    case VK_PRESENT_MODE_MAILBOX_KHR:
        LOG << "Swapchain presentmode set: VK_PRESENT_MODE_MAILBOX_KHR";
        break;
    case VK_PRESENT_MODE_FIFO_KHR:
        LOG << "Swapchain presentmode set: VK_PRESENT_MODE_FIFO_KHR";
        break;
    case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
        LOG << "Swapchain presentmode set: VK_PRESENT_MODE_FIFO_RELAXED_KHR";
        break;
    case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
        LOG << "Swapchain presentmode set: VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR";
        break;
    case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
        LOG << "Swapchain presentmode set: VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR";
        break;
    case VK_PRESENT_MODE_MAX_ENUM_KHR:
        LOG << "Swapchain presentmode set: VK_PRESENT_MODE_MAX_ENUM_KHR";
        break;
    default:
        LOG << "Swapchain presentmode set: NONE";
        break;
    }

    return preferred_mode;
}

VkExtent2D VulkanGraphics::GetPreferredSwapchainExtend(const WindowData& window_data, const VkSurfaceCapabilitiesKHR& capabilities)
{
    // If the currentextend is the special value 0xFFFFFFFF, min/max extends can be set freely
    // Otherwise, min/max extends are equal or lower than current extend

    // Clamp window width and height between the min/max image extends of the swapchain
    VkExtent2D swapchain_extends {window_data.window_width, window_data.window_height};
    if (capabilities.currentExtent.width == 0xFFFFFFFF) {
        swapchain_extends.width = std::clamp(swapchain_extends.width, capabilities.maxImageExtent.width, capabilities.minImageExtent.width);
        swapchain_extends.height = std::clamp(swapchain_extends.height, capabilities.maxImageExtent.height, capabilities.minImageExtent.height);
    }
    else {
        swapchain_extends = capabilities.currentExtent;
    }

    return swapchain_extends;
}

bool VulkanGraphics::CreateSwapchain(const WindowData& window_data, VkPhysicalDevice device)
{
    SwapChainDetails sw_detail = QuerySwapchainSupport(device);
    VkSurfaceFormatKHR sw_format = GetPreferredSwapchainSurfaceFormat(sw_detail.formats);
    VkPresentModeKHR sw_present_mode = GetPreferredSwapchainPresentMode(sw_detail.present_modes);
    VkExtent2D sw_extend = GetPreferredSwapchainExtend(window_data, sw_detail.capabilities);

    // Image count is minimum + 1 or the maximum
    uint32_t sw_image_count = sw_detail.capabilities.minImageCount + 1;
    if (sw_detail.capabilities.maxImageCount > 0 && sw_image_count > sw_detail.capabilities.maxImageCount) {
        sw_image_count = sw_detail.capabilities.maxImageCount;
    }

    // Create swapchain info structure
    VkSwapchainCreateInfoKHR sw_create_info {};
    sw_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    sw_create_info.surface = vulkan_surface_;
    sw_create_info.minImageCount = sw_image_count;
    sw_create_info.imageFormat = sw_format.format;
    sw_create_info.imageExtent = sw_extend;
    
    // Specifies the image array per frame. This can be used for 3d but also for shadow maps or other image collections.
    // Using this is more performat because of access locality. It's faster to access them.
    sw_create_info.imageArrayLayers = 1;

    /*
    * In this tutorial we're going to render directly to them, which means that they're used as color attachment.
    * It is also possible that you'll render images to a separate image first to perform operations like post-processing.
    * In that case you may use a value like VK_IMAGE_USAGE_TRANSFER_DST_BIT instead and use a memory operation to transfer the rendered image to a swap chain image.
    */
    sw_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Choose whether there are only one or multiple queue families being used
    // For sake of the tutorial we will use concurrent mode
    QueueFamilyIndices indices = FindQueueFamilies(device);
    uint32_t queue_family_array[] = { indices.graphics_index.value(), indices.present_index.value() };
    // If the queue family indices between the queue families are not the same, the functionality is being shared
    if (indices.graphics_index != indices.present_index) {
        sw_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        sw_create_info.queueFamilyIndexCount = 2;
        sw_create_info.pQueueFamilyIndices = queue_family_array;
    }
    else {
        sw_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        sw_create_info.queueFamilyIndexCount = 0; // Optional
        sw_create_info.pQueueFamilyIndices = nullptr; // Optional
    }

    sw_create_info.preTransform = sw_detail.capabilities.currentTransform;
    sw_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    sw_create_info.presentMode = sw_present_mode;
    sw_create_info.clipped = VK_TRUE;
    sw_create_info.oldSwapchain = VK_NULL_HANDLE;

    VkResult res = vkCreateSwapchainKHR(vulkan_device_, &sw_create_info, nullptr, &swapchain_data_.swapchain);
    if (res == VK_SUCCESS) {
        LOG << "SUCCESS\t Create swapchain";
    }
    else {
        LOG << "FAILURE\t Create swapchain failed with error: " << res;
        return false;
    }


    // Store swapchain image data
    swapchain_data_.format = sw_format.format;
    swapchain_data_.extent = sw_extend;

    // Get swapchain images
    uint32_t created_image_count = 0;
    vkGetSwapchainImagesKHR(vulkan_device_, swapchain_data_.swapchain, &created_image_count, nullptr);

    swapchain_data_.images.resize(created_image_count);
    vkGetSwapchainImagesKHR(vulkan_device_, swapchain_data_.swapchain, &created_image_count, swapchain_data_.images.data());

    return true;
}

bool VulkanGraphics::CreateImageViews()
{
    // For 3D and multiple swapchain image layers, view would have to be created for each layer as well
    uint32_t count = swapchain_data_.images.size();
    swapchain_data_.image_views.resize(count);
    uint32_t success = 0;
    for (int i = 0; i < count; i++) {
        VkImageViewCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = swapchain_data_.images[i];
        create_info.format = swapchain_data_.format;
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;

        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.layerCount = 1;
        create_info.subresourceRange.levelCount = 1;
        
        if (vkCreateImageView(vulkan_device_, &create_info, nullptr, &swapchain_data_.image_views[i]) == VK_SUCCESS) {
            success++;
        }
        else {
            LOG << "FAILURE\t Failed creatim image view";
        }
    }

    return success == count;
}

void VulkanGraphics::CreateRenderPass() {
    VkAttachmentDescription color_attachment{};
    color_attachment.format = swapchain_data_.format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

    // For color and depth data
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    // For stencil buffers
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Reference attachments
    VkAttachmentReference attachment_reference{};
    attachment_reference.attachment = 0;
    attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Describe subpass
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attachment_reference;

    // Specify subpass dependancy. Make the render pass depend an thus wait on a subpass to finish first
    // This is to delay the implicit image layer transition at the start of the render pass because there isno image available at that point.
    // Since the color attachment stage waits until an image is available (done with semaphores),
    // we can make the transition depend on that stage.
    VkSubpassDependency dependancy{};
    // Make external subpass wait ont first subpass
    dependancy.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependancy.dstSubpass = 0;

    // Define the stage that has to be waited on by the extern al stage and in which subpass
    dependancy.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependancy.srcAccessMask = 0;

    // It has to wait on the write operation of the color attachment stage
    dependancy.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependancy.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


    // Create the render pass
    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependancy;

    if (vkCreateRenderPass(vulkan_device_, &render_pass_info, nullptr, &render_pass_) == VK_SUCCESS) {
        LOG << "SUCCESS \t Created render pass";
    }
    else {
        LOG << "FAILURE \t Failed to create render pass";
    }
}

void VulkanGraphics::CreateDescriptorSetLayout()
{
    // Tell the shader what type of object it can access, it's binding position and in which shader stage
    // In this case it's a uniform buffer
    VkDescriptorSetLayoutBinding layout{};
    layout.binding = 0;
    layout.descriptorCount = 1;
    layout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layout.pImmutableSamplers = nullptr;

    // Create info holds a list of descriptor layouts
    VkDescriptorSetLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    create_info.bindingCount = 1;
    create_info.pBindings = &layout;

    VkResult res = vkCreateDescriptorSetLayout(vulkan_device_, &create_info, nullptr, &descriptor_set_layout_);
    if (res != VK_SUCCESS) {
        LOG << "FAILURE\t Failed to create descriptor seet layout, error:" << res;
    }
}

void VulkanGraphics::CreateGraphicsPipeline()
{
    // Load shaders
    VulkanShaderLoader shader_loader;
    VkShaderModule vertex_shader = shader_loader.CreateShaderModule(shader_loader.LoadShader("..\\src\\shaders\\v_triangle.spv"), vulkan_device_, nullptr);
    VkShaderModule fragment_shader = shader_loader.CreateShaderModule(shader_loader.LoadShader("..\\src\\shaders\\f_triangle.spv"), vulkan_device_, nullptr);
    
    // Initialize shader stages
    VkPipelineShaderStageCreateInfo vertex_pipeline{};
    vertex_pipeline.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_pipeline.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_pipeline.module = vertex_shader;
    vertex_pipeline.pName = "main";

    VkPipelineShaderStageCreateInfo fragment_pipeline{};
    fragment_pipeline.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_pipeline.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_pipeline.module = fragment_shader;
    fragment_pipeline.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[]{ vertex_pipeline, fragment_pipeline };

    // Create Vertex input pipeline state
    VkVertexInputBindingDescription binding_desc = geometry_triangle_helpers::GetVertexBindingDescription();
    auto attribute_desc_array = geometry_triangle_helpers::GetVertexAttributeDescription();

    VkPipelineVertexInputStateCreateInfo vertex_input_pipeline_state{};
    vertex_input_pipeline_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_pipeline_state.pVertexBindingDescriptions = &binding_desc;
    vertex_input_pipeline_state.pVertexAttributeDescriptions = attribute_desc_array.data();
    vertex_input_pipeline_state.vertexBindingDescriptionCount = 1;
    vertex_input_pipeline_state.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_desc_array.size());

    // Add dynamic states to the pipeline
    // Viewport and scissor allow changing these settings at render time
    uint32_t dynamic_state_count = 2;
    VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    // Define dynamic stages of the pipeline
    VkPipelineDynamicStateCreateInfo dynamic_pipeline_state{};
    dynamic_pipeline_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_pipeline_state.pDynamicStates = &dynamic_states[0];
    dynamic_pipeline_state.dynamicStateCount = dynamic_state_count;

    // Define how the vertex data should be interpreted
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state{};
    input_assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_state.primitiveRestartEnable = VK_FALSE;

    // Define the viewport
    VkViewport viewport{};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = swapchain_data_.extent.width;
    viewport.height = swapchain_data_.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Define scissor
    VkRect2D scissor{};
    scissor.offset = VkOffset2D{0, 0};
    scissor.extent = swapchain_data_.extent;

    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.pViewports = &viewport;
    viewport_state.viewportCount = 1;
    viewport_state.pScissors = &scissor;
    viewport_state.scissorCount = 1;

    // Create rasterize state
    VkPipelineRasterizationStateCreateInfo rasterizer_state{};
    rasterizer_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer_state.depthClampEnable = VK_FALSE; // Clamps fragments outside the near-and-far plane to them. Requires enabling a GPU feature.
    rasterizer_state.rasterizerDiscardEnable = VK_FALSE; // Discards geometry
    
    // Face culling
    rasterizer_state.polygonMode = VK_POLYGON_MODE_FILL; // Using any mode other than fill requires enabling a GPU feature.
    rasterizer_state.lineWidth = 1.0f;
    rasterizer_state.cullMode = VK_CULL_MODE_NONE;
    rasterizer_state.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    // Depth settings
    rasterizer_state.depthClampEnable = VK_FALSE;
    rasterizer_state.depthBiasConstantFactor = 0.0f;
    rasterizer_state.depthBiasClamp = 0.0f;
    rasterizer_state.depthBiasConstantFactor = 0.0f;

    // Create Multisampling state
    VkPipelineMultisampleStateCreateInfo multisampling_state{};
    multisampling_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling_state.sampleShadingEnable = VK_FALSE;
    multisampling_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling_state.minSampleShading = 1.0f;
    multisampling_state.pSampleMask = nullptr;
    multisampling_state.alphaToCoverageEnable = VK_FALSE;
    multisampling_state.alphaToOneEnable = VK_FALSE;

    // Depth testing

    // Framebuffer color blending attachment
    // Each framebuffer should have an attachment, a list of the attachments are being passed to the creation struct
    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_CONSTANT_ALPHA;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

    // Create color blending state for all frame buffers
    VkPipelineColorBlendStateCreateInfo color_blend_state{};
    color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state.logicOpEnable = VK_FALSE;
    color_blend_state.logicOp = VK_LOGIC_OP_COPY;
    color_blend_state.attachmentCount = 1;
    color_blend_state.pAttachments = &color_blend_attachment; // pass attachments
    color_blend_state.blendConstants[0] = 0.0f;
    color_blend_state.blendConstants[1] = 0.0f;
    color_blend_state.blendConstants[2] = 0.0f;
    color_blend_state.blendConstants[3] = 0.0f;

    // Describes the uniform data used in shaders
    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &descriptor_set_layout_;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(vulkan_device_, &pipeline_layout_info, nullptr, &pipeline_layout_) == VK_SUCCESS) {
        LOG << "SUCCESS\t Created pipeline layout";
    }
    else {
        LOG << "Failure\t Couldn't create pipeline layout";
    }

    // Create graphics pipeline
    VkGraphicsPipelineCreateInfo graphics_pipeline_info{};
    graphics_pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphics_pipeline_info.stageCount = 2;
    graphics_pipeline_info.pStages = shader_stages;
    graphics_pipeline_info.pVertexInputState = &vertex_input_pipeline_state;
    graphics_pipeline_info.pInputAssemblyState = &input_assembly_state;
    graphics_pipeline_info.pRasterizationState = &rasterizer_state;
    graphics_pipeline_info.pMultisampleState = &multisampling_state;
    graphics_pipeline_info.pDepthStencilState = nullptr;
    graphics_pipeline_info.pColorBlendState = &color_blend_state;
    graphics_pipeline_info.pDynamicState = &dynamic_pipeline_state;
    graphics_pipeline_info.pViewportState = &viewport_state;

    graphics_pipeline_info.layout = pipeline_layout_;
    graphics_pipeline_info.renderPass = render_pass_;
    graphics_pipeline_info.subpass = 0; // Specify the subpass where this graphics pipeline will be used
    graphics_pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    graphics_pipeline_info.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(vulkan_device_, VK_NULL_HANDLE, 1, &graphics_pipeline_info, nullptr, &pipeline_) == VK_SUCCESS) {
        LOG << "SUCCESS\t Created graphics pipeline";
    }
    else {
        LOG << "Failure\t Couldn't create graphics pipeline";
    }

    shader_loader.DestroyCreatedShaderModules(vulkan_device_, nullptr);
}

void VulkanGraphics::CreateFramebuffers()
{
    // Create framebuffer for every image in the swapchain
    uint32_t sw_image_count = (uint32_t)swapchain_data_.image_views.size();
    swapchain_data_.framebuffers.resize(sw_image_count);
    for (int32_t i = 0; i < sw_image_count; i++) {
        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.width = swapchain_data_.extent.width;
        framebuffer_info.height = swapchain_data_.extent.height;
        framebuffer_info.pAttachments = &swapchain_data_.image_views[i];
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.renderPass = render_pass_;
        framebuffer_info.layers = 1;

        VkResult res = vkCreateFramebuffer(vulkan_device_, &framebuffer_info, nullptr, &swapchain_data_.framebuffers[i]);
        if (res == VK_SUCCESS) {
            LOG << "SUCCESS\t Created framebuffer";
        }
        else {
            LOG << "FAILURE\t Could not create framebuffer error " << res;
        }
    }
}

void VulkanGraphics::CreateCommandPool()
{
    VkCommandPoolCreateInfo command_pool_info{};
    command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_info.queueFamilyIndex = FindQueueFamilies(selected_device_).graphics_index.value();
    command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    
    VkResult res = vkCreateCommandPool(vulkan_device_, &command_pool_info, nullptr, &command_pool_);
    if (res == VK_SUCCESS) {
        LOG << "SUCCESS\t Created command pool";
    }
    else {
        LOG << "FAILURE\t Could not create command pool, error: " << res;
    }
}

void VulkanGraphics::CreateTextureImage()
{
    // Load image
    int width, height, channels;
    VkBuffer image_staging_buffer;
    VkDeviceMemory image_staging_memory;
    size_t image_size = LoadTexture(vulkan_device_, selected_device_, image_staging_buffer, image_staging_memory, width, height, channels);

    CreateImage(width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        vulkan_device_, selected_device_, texture_image_, texture_image_memory_);

    VkCommandBuffer cmd_buffer = BeginSingleTimeCommandBuffer(vulkan_device_, command_pool_);
    CmdTransitionImageLayout(cmd_buffer, texture_image_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CmdCopyBufferToImage(cmd_buffer, image_staging_buffer, texture_image_, width,height);
    CmdTransitionImageLayout(cmd_buffer, texture_image_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    EndSingleTimeCommandBuffer(vulkan_device_, device_queues_.graphics_queue, command_pool_, cmd_buffer);

    vkDestroyBuffer(vulkan_device_, image_staging_buffer, nullptr);
    vkFreeMemory(vulkan_device_, image_staging_memory, nullptr);
}

void VulkanGraphics::CreateVertexBuffer()
{
    uint64_t size = sizeof(geometry_triangle_helpers::Vertex) * geometry_triangle_helpers::quad_indices.size();
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_mem;

    // Create staging buffer
    // It functions as a source transfer buffer on cpu accesible memory
    CreateBuffer(vulkan_device_, selected_device_, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, staging_buffer, staging_buffer_mem, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Map buffer memory to cpu accessible memory
    // Then copy vertices for the gpu to load
    void* data; 
    vkMapMemory(vulkan_device_, staging_buffer_mem, 0, size, 0, &data);
    memcpy(data, geometry_triangle_helpers::quad_vertices.data(), (size_t)size);
    vkUnmapMemory(vulkan_device_, staging_buffer_mem);

    // Create triangle buffers and memory
    // Vertex buffer on gpu local memory, not accesible my the cpu and is faster
    CreateBuffer(vulkan_device_, selected_device_, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, triangle_buffer_, triangle_buffer_memory_, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // Copy data to local memory
    VkCommandBuffer cmd_buffer = BeginSingleTimeCommandBuffer(vulkan_device_, command_pool_);
    CmdCopyBuffer(cmd_buffer,staging_buffer, triangle_buffer_, size);
    EndSingleTimeCommandBuffer(vulkan_device_, device_queues_.graphics_queue, command_pool_, cmd_buffer);

    // Delete staging buffers
    vkDestroyBuffer(vulkan_device_, staging_buffer, nullptr);
    vkFreeMemory(vulkan_device_, staging_buffer_mem, nullptr);


}

void VulkanGraphics::CreateIndexBuffer()
{
    uint64_t size = geometry_triangle_helpers::quad_indices.size() * sizeof(uint32_t);
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_mem;

    // Create staging buffer
    CreateBuffer(vulkan_device_, selected_device_, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, staging_buffer, staging_buffer_mem, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(vulkan_device_, staging_buffer_mem, 0, size, 0, &data);
    memcpy(data, geometry_triangle_helpers::quad_indices.data(), (size_t)size);

    // Create index buffer
    CreateBuffer(vulkan_device_, selected_device_, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, index_buffer_, index_buffer_memory_, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkCommandBuffer cmd_buffer = BeginSingleTimeCommandBuffer(vulkan_device_, command_pool_);
    CmdCopyBuffer(cmd_buffer, staging_buffer, index_buffer_, size);
    EndSingleTimeCommandBuffer(vulkan_device_, device_queues_.graphics_queue, command_pool_, cmd_buffer);

    vkDestroyBuffer(vulkan_device_, staging_buffer, nullptr);
    vkFreeMemory(vulkan_device_, staging_buffer_mem, nullptr);
}

void VulkanGraphics::CreateUniformBuffers()
{
    uniform_buffers_.resize(MAX_FRAMES_IN_FLIGHT);
    uniform_memory_.resize(MAX_FRAMES_IN_FLIGHT);
    uniform_mapped_memory_.resize(MAX_FRAMES_IN_FLIGHT);

    VkDeviceSize size = sizeof(UniformBufferObject);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        CreateBuffer(vulkan_device_, selected_device_, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uniform_buffers_[i], uniform_memory_[i], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vkMapMemory(vulkan_device_, uniform_memory_[i], 0, size, 0, &uniform_mapped_memory_[i]);
    }
}

void VulkanGraphics::CreateCommandBuffer()
{
    command_buffers_.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = command_pool_;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = (uint32_t)command_buffers_.size();
    VkResult res = vkAllocateCommandBuffers(vulkan_device_, &allocate_info, command_buffers_.data());
    if (res != VK_SUCCESS) {
        LOG << "SUCCESS\t Couldn't create command buffer";
    }
}

void VulkanGraphics::CreateDescriptorPools()
{
    VkDescriptorPoolSize poolsize{};
    poolsize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolsize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    info.poolSizeCount = 1;
    info.pPoolSizes = &poolsize;
    info.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkResult res = vkCreateDescriptorPool(vulkan_device_, &info, nullptr, &descriptor_pool_);
    if (res != VK_SUCCESS) {
        LOG << "FAILURE\t Failed creatinf descriptor pool, error:" << res;
    }
}

void VulkanGraphics::CreateDescriptorSets()
{
    // Create allocation info struct
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptor_set_layout_);
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = descriptor_pool_;
    alloc_info.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    alloc_info.pSetLayouts = layouts.data();
    
    // Resize array
    descriptor_sets_.resize(MAX_FRAMES_IN_FLIGHT);
    VkResult res = vkAllocateDescriptorSets(vulkan_device_, &alloc_info, descriptor_sets_.data());
    if (res != VK_SUCCESS) {
        LOG << "FAILURE\t Failed creatinf descriptor sets, error:" << res;
    }

    // Configure descriptors by binding the uniform buffers
    // Could use arrays of descriptor sets as well instead of this loop configuring them one by one
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = uniform_buffers_[i];
        buffer_info.offset = 0;
        buffer_info.range = sizeof(UniformBufferObject);

		// Update the desciptor set configurations
		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = descriptor_sets_[i];
        write.dstBinding = 0;
        write.dstArrayElement = 0;
        
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write.descriptorCount = 1;

        write.pBufferInfo = &buffer_info;
        // Depends on what type of buffers that are being used
        write.pImageInfo = nullptr; // Optional
        write.pTexelBufferView = nullptr; // Optional

        vkUpdateDescriptorSets(vulkan_device_, 1, &write, 0, nullptr);
	}
}

void VulkanGraphics::RecordCommandBuffer(const VkCommandBuffer& cmd_buffer, uint32_t img_index)
{
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.pInheritanceInfo = nullptr;

    // Start command buffer recording with BeginCommandBuffer
    VkResult res = vkBeginCommandBuffer(cmd_buffer, &begin_info);
    if (res != VK_SUCCESS) {
        LOG << "FAILURE\t Failed creating command buffer";
    }

    // Begin render pass
    VkRenderPassBeginInfo begin_pass_info{};
    begin_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    begin_pass_info.renderPass = render_pass_;
    begin_pass_info.framebuffer = swapchain_data_.framebuffers[img_index];
    begin_pass_info.renderArea.offset = { 0,0 };
    begin_pass_info.renderArea.extent = swapchain_data_.extent;

    // Set clear color
    VkClearValue clear_color = { {{0.1f, 0.2f, 0.1f, 1.0f}} };
    begin_pass_info.clearValueCount = 1;
    begin_pass_info.pClearValues = &clear_color;

    // Define the commands in the render pass
    vkCmdBeginRenderPass(cmd_buffer, &begin_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);

    //Because viewport and scissors were set as dynamic states, it has to be set during rendering
    VkViewport viewport{};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = swapchain_data_.extent.width;
    viewport.height = swapchain_data_.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

    // Define scissor
    VkRect2D scissor{};
    scissor.offset = VkOffset2D{ 0, 0 };
    scissor.extent = swapchain_data_.extent;
    vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

    // Draw
    VkBuffer vertex_buffers[] = { triangle_buffer_ };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmd_buffer, 0, 1, vertex_buffers, offsets);
    vkCmdBindIndexBuffer(cmd_buffer, index_buffer_, 0, VK_INDEX_TYPE_UINT16);
    vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, 0, 1, &descriptor_sets_[current_frame_], 0, nullptr);
    vkCmdDrawIndexed(cmd_buffer, geometry_triangle_helpers::quad_indices.size(), 1, 0, 0, 0);

    // End render pass
    vkCmdEndRenderPass(cmd_buffer);

    res = vkEndCommandBuffer(cmd_buffer);
    if (res != VK_SUCCESS) {
        LOG << "Render pass failed";
    }
}

void VulkanGraphics::CreateSyncObjects()
{
    sem_image_available_.resize(MAX_FRAMES_IN_FLIGHT);
    sem_render_finished_.resize(MAX_FRAMES_IN_FLIGHT);
    fence_in_flight_.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    // Create sync objects
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkResult res_image_available = vkCreateSemaphore(vulkan_device_, &semaphore_info, nullptr, &sem_image_available_[i]);
        VkResult res_render_finished = vkCreateSemaphore(vulkan_device_, &semaphore_info, nullptr, &sem_render_finished_[i]);
        VkResult in_flight = vkCreateFence(vulkan_device_, &fence_info, nullptr, &fence_in_flight_[i]);

        if (res_image_available && res_render_finished && in_flight != VK_SUCCESS) {
            LOG << "Failed creating sync objects";
        }
    }
}

// Submits the command to the GPU
void VulkanGraphics::RenderFrame()
{
    // Wait for GPU to finish
    vkWaitForFences(vulkan_device_, 1, &fence_in_flight_[current_frame_], true, UINT64_MAX);

    // Get next image from swapchain
    uint32_t image_index = 0;
    VkResult sw_result = vkAcquireNextImageKHR(vulkan_device_, swapchain_data_.swapchain, UINT64_MAX, sem_image_available_[current_frame_], VK_NULL_HANDLE, &image_index);
    if (sw_result == VK_ERROR_OUT_OF_DATE_KHR || sw_result == VK_SUBOPTIMAL_KHR || resize_necessary_) {
        // Swapchain not compatible with the surface anymore
        resize_necessary_ = false;
        RecreateSwapchain(app_window_->GetWindowData(), selected_device_);
        return;
    }

    // Only wait for fences when the swapchain can render
    vkResetFences(vulkan_device_, 1, &fence_in_flight_[current_frame_]);

    // Make the command buffer able to record by resetting it. An already full buffer can't record
    vkResetCommandBuffer(command_buffers_[current_frame_], 0);

    RecordCommandBuffer(command_buffers_[current_frame_], image_index);

    UpdateUniformBuffer(current_frame_);

    // Submit the queue to the gpu
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // Tell vulkan at which stages to wait on give semaphores
    // Wait at the color attachment output stage untill an image is available.
    // The color attachment stageis defines when creating the render pass
    VkSemaphore semaphores[]{sem_image_available_[current_frame_] };
    VkPipelineStageFlags wait_stages[]{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = semaphores;
    submit_info.pWaitDstStageMask = wait_stages;

    // Submit the command buffer to use
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffers_[current_frame_];

    // Specify which semapores to signal when rendering is finished
    VkSemaphore signal_semaphores[]{sem_render_finished_[current_frame_] };
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    // Submit the command buffer on the grphics queue. The command pool is only ony used for storing 
    //command buffers in memory
    vkQueueSubmit(device_queues_.graphics_queue, 1, &submit_info, fence_in_flight_[current_frame_]);

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;
    
    VkSwapchainKHR swapchains[] = {swapchain_data_.swapchain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &swapchains[0];
    present_info.pResults = nullptr;
    present_info.pImageIndices = &image_index;

    sw_result = vkQueuePresentKHR(device_queues_.present_queue, &present_info);
    if (sw_result == VK_ERROR_OUT_OF_DATE_KHR || sw_result == VK_SUBOPTIMAL_KHR || resize_necessary_) {
        // Swapchain not compatible with the surface anymore
        resize_necessary_ = false;
        RecreateSwapchain(app_window_->GetWindowData(), selected_device_);
    }

    current_frame_ = (current_frame_ + 1) % (MAX_FRAMES_IN_FLIGHT);
}

void VulkanGraphics::UpdateUniformBuffer(uint32_t current_frame)
{
    static auto start_time = std::chrono::high_resolution_clock::now();

    auto current_time = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();

    UniformBufferObject ubo{};
    // rotate over z
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.f), glm::vec3(0.0f, 0.0f, 1.0f));
    // Look at the center from a distance
    ubo.view = glm::lookAt(glm::vec3(0.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    //90deg fov with swapchain aspect ration and near/far plane
    float aspect = swapchain_data_.extent.width / (float)swapchain_data_.extent.height;
    ubo.projection = glm::perspective(glm::radians(45.f), aspect, 0.1f, 100.0f);

    // glm is for opengl with an inverted y coordinate system, so we comensate for that
    ubo.projection[1][1] *= -1;

    memcpy(uniform_mapped_memory_[current_frame], &ubo, sizeof(ubo));
}

void VulkanGraphics::RecreateSwapchain(const WindowData& window_data, VkPhysicalDevice device)
{
    WindowData win = window_data;
    vkDeviceWaitIdle(vulkan_device_);
    DestroySwapchain();

    while (win.window_width == 0 || win.window_height == 0) {
        glfwWaitEvents();
        win = app_window_->GetWindowData();
    }

    CreateSwapchain(win, device);
    CreateImageViews();
    CreateFramebuffers();
}

VulkanGraphics::VulkanGraphics(BP_Window* window):
    app_window_(window)
{
    // Check if requested validation layers exist
    bool validation_layer_support = CheckValidationLayerSupport();
    if (validation_layer_support) {
        LOG << "Validation layers supported";
    }
    else {
        LOG << "Validation layers not supported";
        LOG << "Validation layers are disabled";
    }
    LOG; //insert new line

#ifdef _DEBUG
    if (validation_layer_support) {
        enable_validation_layers_ = true;
    }
#endif // _DEBUG

    // Initialize vulkan
    Initialize();
    EnableVulkanDebugMessages();

    WindowData window_data = app_window_->GetWindowData();

    LOG;
    CreateVulkanSurface(window_data);
    SelectPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapchain(window_data, selected_device_);
    CreateImageViews();
    CreateRenderPass();
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();
    CreateFramebuffers();
    CreateCommandPool();
    CreateTextureImage();
    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateUniformBuffers();
    CreateDescriptorPools();
    CreateDescriptorSets();
    CreateCommandBuffer();
    CreateSyncObjects();
}

VulkanGraphics::~VulkanGraphics()
{
    Edulcorate();
}
