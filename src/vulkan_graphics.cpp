#include "vulkan_graphics.h"
#include <GLFW/glfw3.h>
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
	create_info.enabledLayerCount = 0;
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

	LOG; //insert new line
	LOG << "Create VULKAN Instance";
	// Create vulkan instance
	VkResult res = vkCreateInstance(&create_info, nullptr, &instance);
	if (res != VK_SUCCESS) {
		LOG << "Vulkan initialization error: " << res;
		return false;
	}

	return true;
}

bool VulkanGraphics::CheckValidationLayerSupport()
{
	LOG << "Check validation layer support";
	uint32_t available_count;
	uint32_t validation_count = validation_layers.size();
	vkEnumerateInstanceLayerProperties(&available_count, nullptr);

	VkLayerProperties* available_layers = new VkLayerProperties[available_count];
	vkEnumerateInstanceLayerProperties(&available_count, available_layers);

	uint32_t found_layer_count = 0;
	for (int validation_index = 0; validation_index < validation_count; validation_index++) {
		bool layer_found = false;
		for (int available_index = 0; available_index < available_count; available_index++) {
			// Check if layer name exists
			if (std::string(validation_layers[validation_index]).compare(available_layers[available_index].layerName)) {
				LOG << validation_layers[validation_index] << "\tFOUND";
				layer_found = true;
				found_layer_count++;
			}
			break;
		}

		if (!layer_found) {
			LOG << validation_layers[validation_index] << "\tMISSING";
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

bool VulkanGraphics::EnableValidationLayers()
{
	


	return false;
}

void VulkanGraphics::Edulcorate()
{
	vkDestroyInstance(instance, nullptr);
}

VulkanGraphics::VulkanGraphics()
{
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
		EnableValidationLayers();
	}
#endif // _DEBUG

	// Initialize vulkan
	Initialize();
}

VulkanGraphics::~VulkanGraphics()
{
	Edulcorate();
}
