#pragma once

#include <spdlog/spdlog.h>
#include <memory>

class tas_logger
{
#pragma region Singleton
public:
	static tas_logger& get() {
		static tas_logger instance;
		return instance;
	}

private:
	tas_logger();
#pragma endregion

public:
	static void debug(const char* format, ...);
	static void info(const char* format, ...);
	static void warning(const char* format, ...);
	static void error(const char* format, ...);
	static void fatal(const char* format, ...);

	static void flush_and_exit();
};

