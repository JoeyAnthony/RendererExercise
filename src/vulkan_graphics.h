#pragma 
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_structs.hpp>

#include <Windows.h>

#include <optional>

#include "bp_window.h"

const unsigned short MAX_FRAMES_IN_FLIGHT = 2;

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

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);

/*
* This function retreives the function pointer for vkCreateDebugUtilsMessengerEXT and calls it with the paramaters passed to this function.
* Because the create function only has to be called once, DebugCallback can be used freely afterwards.
*/
static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

/*
* This function retreives the function pointer for vkDestroyDebugUtilsMessengerEXT and calls it with the paramaters passed to this function.
* DebugCallback cannot be used anymore after this function was called.
*/
static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debug_messenger, pAllocator);
    }
}

class VulkanGraphics {
    BP_Window* app_window;
    VkInstance instance_;
    VkDebugUtilsMessengerEXT debug_messenger_;
    VkSurfaceKHR vulkan_surface_;
    VkPhysicalDevice selected_device_ = VK_NULL_HANDLE;
    VkDevice vulkan_device_;
    bool enable_validation_layers_ = false;
    DeviceQueues device_queues_;

    VkRenderPass render_pass_;
    VkPipelineLayout pipeline_layout_;
    VkPipeline pipeline_;

    // Validation layers used in this application
    const std::vector<const char*> validation_layers_ = { "VK_LAYER_KHRONOS_validation"/*, "VK_LAYER_LUNARG_api_dump"*/};
    // Required device extensions
    const std::vector<const char*> required_device_extensions_ = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME, "VK_KHR_external_memory_win32", VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME, "VK_KHR_external_semaphore_win32", VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME };

    BP_SwapchainInfo swapchain_data_;
    VkCommandPool command_pool;

    // Per in-flight frames
    uint32_t current_frame = 0;
    std::vector<VkCommandBuffer> command_buffers;
    std::vector<VkSemaphore> sem_image_available;
    std::vector<VkSemaphore> sem_render_finished;
    std::vector<VkFence> fence_in_flight;

    bool resize_necessary = false;
    uint32_t win_width = 600, win_height = 600;

    VkBuffer triangle_buffer_;
    VkDeviceMemory triangle_buffer_memory;

private:
    bool Initialize();
    void SetValidationLayers(VkInstanceCreateInfo& create_info);
    void PopulateDebugMessengerCreateInfoStruct(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    /*
    * Select physical device based on whether it's a dedicated GPU or not
    */
    void SelectPhysicalDevice();

    void CreateLogicalDevice();

    
public:
    void ResizeBuffer(uint32_t width, uint32_t height);

public:
    // Messaging

    std::vector<const char*> GetRequiredInstanceExtensions();
    void EnableVulkanDebugMessages();
    // ~Messaging

    bool CheckValidationLayerSupport();
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
    void DestroySwapchain();
    void Edulcorate();
    void CreateVulkanSurface(const WindowData& window_data);

    bool AreExtensionsAvailable(const std::vector<const char*>& extensions);


    /*
    * Enumerate all Queue Families to check if the physical device supports the required queue types
    * Currently it's VK_QUEUE_GRAPHICS_BIT
    */
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

    /*
    * Gives a score to a physical device, a higher score is a higher preference
    */
    uint32_t DetermineDeviceScore(VkPhysicalDevice device);

    /*
    * Checks if this device is suitable to run this application
    */
    bool IsDeviceSuitable(VkPhysicalDevice device);

    //void CreateVulkanSurface(WindowData window_data);

    VkInstance GetVulkanInstance();

    void SetVulkanSurface(const VkSurfaceKHR& surface);

    SwapChainDetails QuerySwapchainSupport(VkPhysicalDevice device);

    VkSurfaceFormatKHR GetPreferredSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& supported_formats);

    VkPresentModeKHR GetPreferredSwapchainPresentMode(const std::vector<VkPresentModeKHR>& present_modes);

    VkExtent2D GetPreferredSwapchainExtend(const WindowData& window_data, const VkSurfaceCapabilitiesKHR& capabilities);

    uint32_t FindMemoryTypes(uint32_t type_filter, VkMemoryPropertyFlags properties);

    bool CreateSwapchain(const WindowData& window_data, VkPhysicalDevice device);

    bool CreateImageViews();

    void CreateRenderPass();

    void CreateGraphicsPipeline();

    void CreateFramebuffers();

    void CreateCommandPool();

    void CreateVertexBuffer();

    void CreateCommandBuffer();

    void RecordCommandBuffer(const VkCommandBuffer& cmd_buffer, uint32_t img_index);

    void CreateSyncObjects();


    void RenderFrame();

    void RecreateSwapchain(const WindowData& window_data, VkPhysicalDevice device);

    VulkanGraphics(BP_Window* window);
    ~VulkanGraphics();
};
