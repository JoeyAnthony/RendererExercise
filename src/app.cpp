#include "app.h"

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto graphics = reinterpret_cast<VulkanGraphics*>(glfwGetWindowUserPointer(window));
	graphics->ResizeBuffer(width, height);
}

void GraphicsApplication::Initialize()
{
	app_window = new GLFWWindowImpl();
	graphics = new VulkanGraphics(app_window);

	// Give pointer of application to glfw to allow communication between static functions
	glfwSetWindowUserPointer(app_window->GLFWGetWindow(), this);
	glfwSetFramebufferSizeCallback(app_window->GLFWGetWindow(), framebufferResizeCallback);

}

void GraphicsApplication::Edulcorate()
{
	delete graphics;
	delete app_window;
}

void GraphicsApplication::Run()
{
	while (app_window->GetShouldClose() == false) {
		app_window->UpdateWindow();

		RenderFrame();
	}
}

void GraphicsApplication::RenderFrame()
{
	graphics->RenderFrame();
}

GraphicsApplication::GraphicsApplication() : shouldRun(true)
{
}

GraphicsApplication::~GraphicsApplication()
{
	Edulcorate();
}
