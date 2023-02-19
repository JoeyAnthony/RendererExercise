#pragma once
#include "w_window.h"
#include "dx12_renderer.h"
#include "klein/klein.hpp"
#include "app.h"

class GraphicsApplication {
	ApplicationWindow app_window;

public:
	bool shouldRun;

	void Initialize();
	void StartLoop();
	void Destroy();

	GraphicsApplication();
	~GraphicsApplication();
};