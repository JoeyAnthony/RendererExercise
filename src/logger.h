#pragma once
#include <iostream>
#include <string>
#include <thread>
#include <filesystem>
#include <sstream>

#ifdef WIN32
#include <Windows.h>
#endif // WIN32

enum LogSeverity : short { BP_INFO = 3, BP_WARNING = 2, BP_DEBUG = 4, BP_SYSTEM, BP_ASSERT, BP_ERROR = 1 };

#ifdef WIN32
enum LogColor : short { BLUE = 1, GREEN = 2, CYAN = 3, RED = 4, PURPLE = 5, YELLOW = 6, DEFAULT = 7 };
#else
enum LogColor : short { DEFAULT, RED, BLUE, ORANGE, PURPLE, YELLOW };
#endif // WIN32


class Logger {

	std::stringstream stream;

public:
	int reshade_log_level = 3;
	bool logTime;
	bool logDate;
	bool logLine;
	bool logFile;
	short logColor;

private:
	std::string GetChanngelString(LogSeverity tag)
	{
		switch (tag) {
		case LogSeverity::BP_INFO: return "INFO";
		case LogSeverity::BP_WARNING: return "WARNING";
		case LogSeverity::BP_DEBUG: return "DEBUG";
		case LogSeverity::BP_SYSTEM: return "SYSTEM";
		case LogSeverity::BP_ASSERT: return "ASSERT";
		default: return "INFO";
		}
	}

public:

	std::stringstream& LogWithSettings(const LogSeverity& channel = BP_INFO, short color = DEFAULT, const char* file = "", size_t line = 0) {
		logColor = color;
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		std::time_t now_c = std::chrono::system_clock::to_time_t(now);
		std::tm now_tm = *std::localtime(&now_c);
		char buffer[30];
		strftime(buffer, sizeof(buffer), "%F %X", &now_tm);
		reshade_log_level = channel;

		stream << buffer << " ";
		stream << std::this_thread::get_id() << " ";
		stream << std::filesystem::path(file).filename() << " ";
		stream << line << " ";
		stream << GetChanngelString(channel);
		stream << ": ";
		return stream;
	}

	Logger() : logTime(false), logDate(false), logLine(0), logFile(false), logColor(DEFAULT) {

	}

	~Logger() {
#ifdef WIN32
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, logColor);
#endif // WIN32

#ifdef RESHADE_LOG_OUTPUT
		reshade::log_message(reshade_log_level, stream.str().c_str());
#else 
		std::cout << stream.str() << std::endl;
#endif // RESHADE_LOG_OUTPUT


#ifdef WIN32
		SetConsoleTextAttribute(hConsole, 7); //reset color
#endif // WIN32
	}
};

// All settings
#define BP_LOG(severity, color) Logger().LogWithSettings(severity, color, __FILE__, __LINE__)
// Severity
#define LOG_S(severity)			BP_LOG(severity, DEFAULT)
// Default
#define LOG						LOG_S(BP_INFO)
