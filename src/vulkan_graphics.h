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
		success *= present_index.has_value();
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

	vkSurfaceFormarKHR


	VulkanGraphics(const WindowData& window_data);
	~VulkanGraphics();
};
