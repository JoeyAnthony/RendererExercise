#include "app.h"
#include "w_window.h"

void GraphicsApplication::Initialize()
{
	WindowConfig config;
	app_window.Initialize(config);
}

void GraphicsApplication::StartLoop()
{
	while (shouldRun) {
		app_window.UpdateWindow();
	}
}

void GraphicsApplication::Destroy()
{

}

GraphicsApplication::GraphicsApplication() : shouldRun(true)
{
}

GraphicsApplication::~GraphicsApplication()
{

}
