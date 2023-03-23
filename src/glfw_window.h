#pragma once
#include <string>

//#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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

class GLFWWindowImpl {
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
};