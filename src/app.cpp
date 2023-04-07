#include "app.h"

void GraphicsApplication::Initialize()
{
	app_window = new GLFWWindowImpl();

	WindowData w_data{};
	app_window->GetWindowHandle(w_data.hwnd);
	w_data.hinstance = GetModuleHandle(nullptr);
	graphics = new VulkanGraphics(w_data);
}

void GraphicsApplication::Edulcorate()
{
	delete app_window;
	delete graphics;
}

void GraphicsApplication::Run()
{
	while (app_window->GetShouldClose() == false) {
		app_window->UpdateWindow();
	}
}

GraphicsApplication::GraphicsApplication() : shouldRun(true)
{
}

GraphicsApplication::~GraphicsApplication()
{
	Edulcorate();
}
