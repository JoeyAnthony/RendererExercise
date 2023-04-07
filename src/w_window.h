#pragma once
#include <string>
#include <map>
#include <Windows.h>

#include "bp_window.h"

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
