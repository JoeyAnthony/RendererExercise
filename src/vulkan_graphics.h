#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_structs.hpp>

#include <Windows.h>
#include <glm/fwd.hpp>

#include "bp_window.h"

#include "vk_helper_functions.h"
#include "geometry-helpers.h"

const unsigned short MAX_FRAMES_IN_FLIGHT = 2;

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
    BP_Window* app_window_;
    VkInstance instance_;
    VkDebugUtilsMessengerEXT debug_messenger_;
    VkSurfaceKHR vulkan_surface_;
    VkPhysicalDevice selected_device_ = VK_NULL_HANDLE;
    VkSampleCountFlagBits device_sample_count;
    VkDevice vulkan_device_;
    bool enable_validation_layers_ = false;
    DeviceQueues device_queues_;

    VkRenderPass render_pass_;
    VkPipelineLayout pipeline_layout_;
    VkDescriptorSetLayout descriptor_set_layout_;
    VkPipeline pipeline_;

    // Validation layers used in this application
    const std::vector<const char*> validation_layers_ = { "VK_LAYER_KHRONOS_validation"/*, "VK_LAYER_LUNARG_api_dump"*/ };
    // Required device extensions
    const std::vector<const char*> required_device_extensions_ = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME, "VK_KHR_external_memory_win32", VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME, "VK_KHR_external_semaphore_win32", VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME };

    BP_SwapchainInfo swapchain_data_;
    VkCommandPool command_pool_;

    // Scene objects
    std::vector<backpack::Model3D> models;
    std::vector<MeshPushConstants> transforms;
    VkBuffer object_transforms_ubo;
    //VkDeviceMemory scene_memory; // scene_data + object_transforms

    // Per in-flight frames
    uint32_t current_frame_ = 0;
    std::vector<VkCommandBuffer> command_buffers_;
    std::vector<VkSemaphore> sem_image_available_;
    std::vector<VkSemaphore> sem_render_finished_;
    std::vector<VkFence> fence_in_flight_;
    std::vector<VkBuffer> uniform_buffers_;
    std::vector<VkDeviceMemory> uniform_memory_;
    std::vector<void*> uniform_mapped_memory_;

    // Compute
    std::vector<VkBuffer> storage_buffer_;
    std::vector<VkDeviceMemory> storage_memory_;
    VkDescriptorPool compute_desc_pool_;
    VkDescriptorSetLayout compute_desc_set_layout_;
    VkDescriptorSet compute_descriptor_set_;
    VkPipelineLayout compute_pipeline_layout_;
    VkPipeline compute_pipeline_;


    // ~Scene objects

    bool resize_necessary_ = false;
    uint32_t win_width_ = 600, win_height_ = 600;

    VkDescriptorPool descriptor_pool_;
    std::vector<VkDescriptorSet> descriptor_sets_;

    VkImage texture_image_;
    VkDeviceMemory texture_image_memory_;
    VkImageView texture_image_view_;
    VkSampler texture_sampler_;

    VkImage depth_image_;
    VkDeviceMemory depth_image_memory_;
    VkImageView depth_image_view_;

    VkImage color_image_;
    VkDeviceMemory color_image_memory_;
    VkImageView color_image_view_;

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

    VkSampleCountFlagBits GetDeviceSampleCount(VkPhysicalDevice physical_device);
    VkSampleCountFlagBits GetDeviceSampleCount(const VkPhysicalDeviceProperties& device_properties);

    SwapChainDetails QuerySwapchainSupport(VkPhysicalDevice device);

    VkSurfaceFormatKHR GetPreferredSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& supported_formats);

    VkPresentModeKHR GetPreferredSwapchainPresentMode(const std::vector<VkPresentModeKHR>& present_modes);

    VkExtent2D GetPreferredSwapchainExtend(const WindowData& window_data, const VkSurfaceCapabilitiesKHR& capabilities);

    // Return VK_FORMAT_UNDEFINED if not supported format could be found
    VkFormat FindSupportedFormat(const std::vector<VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    VkFormat FindDepthFormat();

    bool CreateSwapchain(const WindowData& window_data, VkPhysicalDevice device);

    bool CreateImageViews();

    void CreateRenderPass();

    void CreateDescriptorSetLayout();

    void CreateGraphicsPipeline();

    void CreateCommandPool();

    void CreateDepthResources();

    void CreateFramebuffers();

    BP_Texture CreateTextureImage();

    BP_Texture CreateColorResources();

    void CreateTextureImageViews();

    void CreateTextureSampler(BP_Texture texture);

    void CreateUniformBuffers();

    void CreateCommandBuffer();

    void CreateDescriptorPools();

    void CreateDescriptorSets();

    void RecordCommandBuffer(const VkCommandBuffer& cmd_buffer, uint32_t img_index);

    void CreateSyncObjects();

    void RenderFrame();

    void UpdateUniformBuffer(uint32_t current_frame);


    void RecreateSwapchain(const WindowData& window_data, VkPhysicalDevice device);



    //---------------------
    // Compute

    void CreateComputeResources();

    //---------------------


    void InitializeScene();
    void InitializeModels();
    void UpdateScene();

    VulkanGraphics(BP_Window* window);
    ~VulkanGraphics();
};
