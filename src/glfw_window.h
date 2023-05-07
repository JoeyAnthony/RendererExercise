#pragma once
#include <string>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "bp_window.h"
#include <klein/klein.hpp>

class GLFWWindowImpl: public BP_Win_Window {
private:
	GLFWwindow* window = nullptr;
	
public:
	GLFWWindowImpl();
	GLFWWindowImpl(const WindowConfig& config);
	~GLFWWindowImpl();

	void UpdateWindow() override;
	bool GetShouldClose() override;
	void Initialize(const WindowConfig& config) override;
	void Destroy() override;
	void GetWindowResolution(uint32_t& width, uint32_t& height) override;

	void GetWindowHandle(HWND& hwnd) override;
	VkSurfaceKHR CreateVulkanWindowSurface(VkInstance instance) override;

	// Inherited via BP_Win_Window
	virtual WindowData GetWindowData() override;
};
