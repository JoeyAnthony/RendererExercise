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

	// Load required extensions from GLFW
	uint32_t glfw_extension_count = 0;
	const char** glfw_extensions;
	glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
	if (glfw_extension_count > 0) {
		create_info.enabledExtensionCount = glfw_extension_count;
		create_info.ppEnabledExtensionNames = glfw_extensions;
	}
	create_info.enabledLayerCount = 0;
	for (int i = 0; i < glfw_extension_count; i++) {
		LOG << "GLFW extension found: " << glfw_extensions[i];
	}

	// Enumerate optional extensions
	uint32_t extension_count = 0;
	uint32_t glfw_extensions_found = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
	if (extension_count > 0) {
		std::vector <VkExtensionProperties> extensions(extension_count);
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

		for (int i = 0; i < extension_count; i++) {
			LOG << "Extension found: " << extensions[i].extensionName;

			for (int j = 0; j < glfw_extension_count; j++) {
				if (std::string(extensions[i].extensionName).compare(glfw_extensions[j]) == 0) {
					glfw_extensions_found++;
				}
			}
		}
	}

	if (glfw_extensions_found == glfw_extension_count) {
		LOG << "All GLFW extensions found. Found: " << glfw_extension_count;
	}
	else {
		LOG << "Not all GLFW extensions found. Found: " << glfw_extension_count;
	}

	// Create vulkan instance
	VkResult res = vkCreateInstance(&create_info, nullptr, &instance);
	if (res != VK_SUCCESS) {
		LOG << "Vulkan initialization error: " << res;
		return false;
	}

	return true;
}

void VulkanGraphics::Edulcorate()
{
	vkDestroyInstance(instance, nullptr);
}

VulkanGraphics::VulkanGraphics()
{
	Initialize();
}

VulkanGraphics::~VulkanGraphics()
{
	Edulcorate();
}
