#pragma once
#include "app.h"
#include "logger.h"

int main() {
	GraphicsApplication app;

	app.Initialize();

	app.StartLoop();

	app.Destroy();

	return 0;
}