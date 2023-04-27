#pragma once
#include "glfw_window.h"
#include "dx12_renderer.h"
#include "klein/klein.hpp"
#include "vulkan_graphics.h"

class GraphicsApplication {
	GLFWWindowImpl* app_window = nullptr;
	VulkanGraphics* graphics = nullptr;

public:
	bool shouldRun;

	void Initialize();
	void Edulcorate();
	void Run();
	void RenderFrame();

	GraphicsApplication();
	~GraphicsApplication();
};