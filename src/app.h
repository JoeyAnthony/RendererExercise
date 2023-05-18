#pragma once
#include "glfw_window.h"
#include "dx12_renderer.h"
#include "klein/klein.hpp"
#include "vulkan_graphics.h"

class GraphicsApplication {

public:
	GLFWWindowImpl* app_window = nullptr;
	VulkanGraphics* graphics = nullptr;

	bool shouldRun;

	void Initialize();
	void Edulcorate();
	void Run();
	void RenderFrame();

	GraphicsApplication();
	~GraphicsApplication();
};