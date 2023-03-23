#pragma once
#include <string>
#include <map>
#include <Windows.h>

struct WindowConfig {
	std::string window_name;
	std::string window_text;
	uint32_t size_x;
	uint32_t size_y;
	uint32_t pos_X;
	uint32_t pos_y;
	bool windowed;

	WindowConfig() {
		size_x = 1200;
		size_y = 1000;
		pos_X = CW_USEDEFAULT;
		pos_y = CW_USEDEFAULT;
		windowed = true;
		window_name = "Graphics Application";
		window_text = "Graphics Application";
	}
};

class ApplicationWindow {
	HINSTANCE hinstance;
	HWND hwindow;

public:
	bool should_quit;

	ApplicationWindow();
	~ApplicationWindow();

	void Initialize(const WindowConfig& config);
	void Destroy();
	void UpdateWindow();
	bool GetShouldQuit();
	LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
