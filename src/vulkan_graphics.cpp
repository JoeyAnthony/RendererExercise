#include "vulkan_graphics.h"
#include <GLFW/glfw3.h>

#include <map>
#include <set>
#include <string>

#include "logger.h"
#include "vulkan_shader.h"

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	BP_LOG(LogSeverity::BP_DEBUG, LogColor::PURPLE) << pCallbackData->pMessage;

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

void VulkanGraphics::SetValidationLayers(VkInstanceCreateInfo &create_info)
{
	//Enable validation Layers
	create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers_.size());
	create_info.ppEnabledLayerNames = validation_layers_.data();
	LOG << "Validation layers set";
}

void VulkanGraphics::Edulcorate()
{
	if (swapchain_data_.swapchain) {
		vkDestroySwapchainKHR(vulkan_device_, swapchain_data_.swapchain, nullptr);
	}
	if (vulkan_surface_) {
		vkDestroySurfaceKHR(instance_, vulkan_surface_, nullptr);
	}

	for (auto image_view : swapchain_data_.image_views) {
		vkDestroyImageView(vulkan_device_, image_view, nullptr);
	}

	DestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);
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
		return true;
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

void VulkanGraphics::CreateGraphicsPipeline()
{
	// Load shaders
	VulkanShaderLoader shader_loader;
	VkShaderModule vertex_shader = shader_loader.CreateShaderModule(shader_loader.LoadShader(".\\triangle.vert"), vulkan_device_, nullptr);
	VkShaderModule fragment_shader = shader_loader.CreateShaderModule(shader_loader.LoadShader(".\\triangle.frag"), vulkan_device_, nullptr);
	
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

	VkPipelineShaderStageCreateInfo* shader_stages[]{ &vertex_pipeline, &fragment_pipeline };

	// Create Vertex input pipeline state
	// Set to nothing because we defined vertices in the shader for now
	VkPipelineVertexInputStateCreateInfo vertex_input_pipeline_state{};
	vertex_input_pipeline_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_pipeline_state.pVertexBindingDescriptions = nullptr;
	vertex_input_pipeline_state.pVertexAttributeDescriptions = nullptr;
	vertex_input_pipeline_state.vertexBindingDescriptionCount = 0;
	vertex_input_pipeline_state.vertexAttributeDescriptionCount = 0;

	// Add dynamic states to the pipeline
	// Viewport and scissor allow changing these settings at render time
	uint32_t dynamic_state_count = 2;
	VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

	VkPipelineDynamicStateCreateInfo dynamic_pipeline_state{};
	dynamic_pipeline_state.sType - VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_pipeline_state.pDynamicStates = &dynamic_states[0];
	dynamic_pipeline_state.dynamicStateCount = dynamic_state_count;

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



	shader_loader.DestroyCreatedShaderModules(vulkan_device_, nullptr);
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
	EnableVulkanDebugMessages();

	LOG;
	CreateVulkanSurface(window_data);
	SelectPhysicalDevice();
	CreateLogicalDevice();
	CreateSwapchain(window_data, selected_device_);
	CreateImageViews();
	CreateGraphicsPipeline();
}

VulkanGraphics::~VulkanGraphics()
{
	Edulcorate();
}
