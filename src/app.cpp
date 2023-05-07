#include "app.h"

void GraphicsApplication::Initialize()
{
	app_window = new GLFWWindowImpl();
	graphics = new VulkanGraphics(app_window);
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
