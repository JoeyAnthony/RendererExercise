#include "app.h"

void GraphicsApplication::Initialize()
{
	app_window = new GLFWWindowImpl();
	graphics = new VulkanGraphics();
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
