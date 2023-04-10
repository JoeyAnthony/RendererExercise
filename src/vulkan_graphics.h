#pragma 
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_structs.hpp>

#include <Windows.h>

#include <optional>

#include "bp_window.h"

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

struct WindowData {
	HWND hwnd;
	HINSTANCE hinstance;
	VkSurfaceKHR vulkan_surface;
	uint32_t window_width;  // In pixels
	uint32_t window_height; // In pixels
};

class VulkanGraphics {
	VkInstance instance_;
	VkSurfaceKHR vulkan_surface_;
	VkPhysicalDevice selected_device_ = VK_NULL_HANDLE;
	VkDevice vulkan_device_;
	bool enable_validation_layers_ = false;
	DeviceQueues device_queues_;

	// Validation layers used in this application
	const std::vector<const char*> validation_layers_ = { "VK_LAYER_KHRONOS_validation" };
	// Required device extensions
	const std::vector<const char*> required_device_extensions_ = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkSwapchainKHR swapchain_;

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
	bool CheckValidationLayerSupport();
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	void Edulcorate();
	void CreateVulkanSurface(const WindowData& window_data);

	bool AreExtensionsAvailable(const std::vector<const char*>& extensions);

	// Messaging
	std::vector<const char*> GetRequiredInstanceExtensions();
	// ~Messaging

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

	bool CreateSwapchain(const WindowData& window_data, VkPhysicalDevice device);

	VulkanGraphics(const WindowData& window_data);
	~VulkanGraphics();
};
