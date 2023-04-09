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

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(config.width, config.height, "Vulkan", nullptr, nullptr);

	//uint32_t extension_count = 0;
	//vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

	//LOG << extension_count << " extensions supported";
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
