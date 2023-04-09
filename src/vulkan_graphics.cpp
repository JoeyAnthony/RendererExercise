#include "vulkan_graphics.h"
#include <GLFW/glfw3.h>

#include <map>
#include <set>

#include "logger.h"

bool VulkanGraphics::Initialize()
{
	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Vulkan exercise";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "Backpack";
	app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	app_info.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;

	LOG; //insert new line
	LOG << "Enable VULKAN extensions";
	// All required extensions
	std::vector<const char*> required_extensions;
	required_extensions.reserve(20);

	// Load required extensions from GLFW
	uint32_t glfw_extension_count = 0;
	const char** glfw_extensions;
	glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
	// Set extension count info for glfw only
	if (glfw_extension_count > 0) {
		create_info.enabledExtensionCount = glfw_extension_count;
		create_info.ppEnabledExtensionNames = glfw_extensions;
	}
	for (int i = 0; i < glfw_extension_count; i++) {
		required_extensions.push_back(glfw_extensions[i]);
		LOG << "GLFW extension found: " << glfw_extensions[i];
	}

	// Add multiplatform extensions (MacOS)
	//required_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
	//create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

	// Set extension count info
	create_info.enabledExtensionCount = required_extensions.size();
	create_info.ppEnabledExtensionNames = required_extensions.data();

	LOG; //insert new line
	LOG << "Enumerate VULKAN extensions";
	// Enumerate optional extensions, only checks glfw extensions
	uint32_t extension_count = 0;
	uint32_t glfw_extensions_found = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
	if (extension_count > 0) {
		std::vector <VkExtensionProperties> extensions(extension_count);
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

		for (int i = 0; i < extension_count; i++) {
			bool extension_found = false;

			for (int j = 0; j < glfw_extension_count; j++) {
				if (std::string(extensions[i].extensionName).compare(glfw_extensions[j]) == 0) {
					LOG << extensions[i].extensionName << "\t\tFOUND";
					extension_found = true;
					glfw_extensions_found++;
					break;
				}
			}

			if (!extension_found) {
				LOG << extensions[i].extensionName << "\t\tMISSING";
			}
		}
	}

	if (glfw_extensions_found == glfw_extension_count) {
		LOG << "All GLFW extensions found. Found: " << glfw_extension_count;
	}
	else {
		LOG << "Not all GLFW extensions found. Found: " << glfw_extension_count;
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
	LOG << "VULKAN instance created\tSUCCESS";
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
				LOG << validation_layers_[validation_index] << "\tFOUND";
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

void VulkanGraphics::SetValidationLayers(VkInstanceCreateInfo &create_info)
{
	//Enable validation Layers
	create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers_.size());
	create_info.ppEnabledLayerNames = validation_layers_.data();
	LOG << "Validation layers set";
}

void VulkanGraphics::Edulcorate()
{
	vkDestroySurfaceKHR(instance_, vulkan_surface_, nullptr);
	vkDestroyDevice(vulkan_device_, nullptr);
	vkDestroyInstance(instance_, nullptr);
}

void VulkanGraphics::CreateVulkanSurface(const WindowData& window_data)
{
	VkWin32SurfaceCreateInfoKHR surface_create_info{};
	surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surface_create_info.hinstance = window_data.hinstance;
	surface_create_info.hwnd = window_data.hwnd;

	VkResult res = vkCreateWin32SurfaceKHR(instance_, &surface_create_info, nullptr, &vulkan_surface_);
	if (res == VK_SUCCESS) {
		LOG << "Create Vulkan surface\tSUCCESS";
	}
	else {
		LOG << "Create Vulkan surface failed with error: " << res << "\tFAILURE";
	}
}

void VulkanGraphics::PopulateDebugMessengerCreateInfoStruct(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;

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
		LOG << "Physical device"  << device_properties.deviceName << " score: " << score << "\tFOUND";
	}

	selected_device_ = device_scores.begin()->second;

	VkPhysicalDeviceProperties device_properties;
	vkGetPhysicalDeviceProperties(selected_device_, &device_properties);
	LOG << "Physical device" << device_properties.deviceName << "\tSELECTED";
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
		LOG << "Device is not suitable \tFAILURE";
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
		LOG << "Create Vulkan device\tSUCCESS";
	}
	else {
		LOG << "Create Vulkan device FAILED with error: " << res;
	}

	// Get created queues from the device
	vkGetDeviceQueue(vulkan_device_, indices.graphics_index.value(), 0, &device_queues_.graphics_queue);
	vkGetDeviceQueue(vulkan_device_, indices.present_index.value(), 0, &device_queues_.present_queue);
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
		if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && format.format == VK_FORMAT_R8G8B8A8_SRGB) {
			preferred_format = format;
			format_set = true;
		}
	}

	if (!format_set) {
		preferred_format = supported_formats[0];
		LOG << "Preferred swapcahin format not found t\FAILURE";
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

void VulkanGraphics::CreateSwapchain(const WindowData& window_data, VkPhysicalDevice device)
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
	sw_create_info.presentMode = sw_present_mode;
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

	QueueFamilyIndices indices = FindQueueFamilies(device);
	uint32_t queue_family_array[] = { indices.graphics_index.value(), indices.present_index.value() };
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


}

VulkanGraphics::VulkanGraphics(const WindowData& window_data)
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

	LOG;
	CreateVulkanSurface(window_data);
	SelectPhysicalDevice();
	CreateLogicalDevice();
}

VulkanGraphics::~VulkanGraphics()
{
	Edulcorate();
}
