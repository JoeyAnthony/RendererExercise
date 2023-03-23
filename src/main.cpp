#include "app.h"
#include "logger.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

int main() {

	GraphicsApplication app;

	app.Initialize();

	app.Run();

	return 0;
}