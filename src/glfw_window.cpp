#include "glfw_window.h"
#include <GLFW/glfw3.h>
#include <klein/klein.hpp>

#include "logger.h"

GLFWWindowImpl::GLFWWindowImpl()
{
	WindowConfig config;
	Initialize(config);
}

GLFWWindowImpl::GLFWWindowImpl(const WindowConfig& config)
{
	Initialize(config);
}

GLFWWindowImpl::~GLFWWindowImpl()
{
	Destroy();
}

void GLFWWindowImpl::UpdateWindow()
{
	glfwPollEvents();
}

bool GLFWWindowImpl::GetShouldClose()
{
	return glfwWindowShouldClose(window);
}

void GLFWWindowImpl::Initialize(const WindowConfig& config)
{
	glfwInit();

	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(config.width, config.height, "Vulkan", nullptr, nullptr);
}

void GLFWWindowImpl::Destroy()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void GLFWWindowImpl::GetWindowResolution(uint32_t& width, uint32_t& height)
{
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	width = static_cast<uint32_t>(w);
	height = static_cast<uint32_t>(h);
}

void GLFWWindowImpl::GetWindowHandle(HWND& hwnd)
{
	hwnd = glfwGetWin32Window(window);
}

VkSurfaceKHR GLFWWindowImpl::CreateVulkanWindowSurface(VkInstance instance)
{
	VkSurfaceKHR surface;
	glfwCreateWindowSurface(instance, window, nullptr, &surface);

	return surface;
}

GLFWwindow* GLFWWindowImpl::GLFWGetWindow()
{
	return window;
}

WindowData GLFWWindowImpl::GetWindowData()
{
	WindowData w_data{};
	GetWindowHandle(w_data.hwnd);
	w_data.hinstance = GetModuleHandle(nullptr);
	GetWindowResolution(w_data.window_width, w_data.window_height);
	return w_data;
}
