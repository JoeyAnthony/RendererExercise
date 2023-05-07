#pragma once
#include <string>
#include <vulkan/vulkan.hpp>

#ifdef WIN32
#include <Windows.h>
#endif // WIN32

//struct WindowConfig {
//	std::string window_name;
//	std::string window_text;
//	uint32_t size_x;
//	uint32_t size_y;
//	uint32_t pos_X;
//	uint32_t pos_y;
//	bool windowed;
//
//	WindowConfig() {
//		size_x = 1200;
//		size_y = 1000;
//		pos_X = CW_USEDEFAULT;
//		pos_y = CW_USEDEFAULT;
//		windowed = true;
//		window_name = "Graphics Application";
//		window_text = "Graphics Application";
//	}
//};

struct WindowConfig {
	std::string window_name;
	std::string window_text;
	uint32_t width;
	uint32_t height;
	uint32_t pos_X;
	uint32_t pos_y;
	bool windowed;

	WindowConfig() {
		width = 1000;
		height = 800;
		pos_X = 0;
		pos_y = 0;
		windowed = true;
		window_name = "Graphics Application";
		window_text = "Graphics Application";
	}
};

struct WindowData {
	HWND hwnd;
	HINSTANCE hinstance;
	VkSurfaceKHR vulkan_surface;
	uint32_t window_width;  // In pixels
	uint32_t window_height; // In pixels
};

class BP_Window {
public:
	virtual void UpdateWindow() = 0;
	virtual bool GetShouldClose() = 0;
	virtual void Initialize(const WindowConfig& config) = 0;
	virtual void Destroy() = 0;
	virtual void GetWindowResolution(uint32_t& width, uint32_t& height) = 0;
	virtual VkSurfaceKHR CreateVulkanWindowSurface(VkInstance instance) = 0;
	virtual WindowData GetWindowData() = 0;
};

#ifdef WIN32
class BP_Win_Window : public BP_Window {
	virtual void GetWindowHandle(HWND& hwnd) = 0;
};
#endif // WIN32
