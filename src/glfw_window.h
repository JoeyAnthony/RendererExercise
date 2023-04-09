#pragma once
#include <string>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "bp_window.h"
#include <klein/klein.hpp>

class GLFWWindowImpl: public BP_Window {
private:
	GLFWwindow* window = nullptr;
	
public:
	GLFWWindowImpl();
	GLFWWindowImpl(const WindowConfig& config);
	~GLFWWindowImpl();

	void UpdateWindow();
	bool GetShouldClose();
	void Initialize(const WindowConfig& config);
	void Destroy();
	void GetWindowResolution(uint32_t& width, uint32_t& height);

	void GetWindowHandle(HWND& hwnd);
	VkSurfaceKHR CreateVulkanWindowSurface(VkInstance instance);
};
