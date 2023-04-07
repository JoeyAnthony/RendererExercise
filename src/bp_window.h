#pragma once
#include <string>
//#include <Windows.h>

//struct WindowConfig {
//	std::string window_name;
//	std::string window_text;
//	uint32_t size_x;
//	uint32_t size_y;
//	uint32_t pos_X;
//	uint32_t pos_y;
//	bool windowed;
//
//	WindowConfig() {
//		size_x = 1200;
//		size_y = 1000;
//		pos_X = CW_USEDEFAULT;
//		pos_y = CW_USEDEFAULT;
//		windowed = true;
//		window_name = "Graphics Application";
//		window_text = "Graphics Application";
//	}
//};

struct WindowConfig {
	std::string window_name;
	std::string window_text;
	uint32_t width;
	uint32_t height;
	uint32_t pos_X;
	uint32_t pos_y;
	bool windowed;

	WindowConfig() {
		width = 1000;
		height = 800;
		pos_X = 0;
		pos_y = 0;
		windowed = true;
		window_name = "Graphics Application";
		window_text = "Graphics Application";
	}
};

class BP_Window {

};