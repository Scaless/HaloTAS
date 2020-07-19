#pragma once
#include "pch.h"
#include <string>

#pragma warning(push)
#pragma warning(disable: 26451 26495 26812)
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#pragma warning(pop)

/// <summary>
/// tas_logger wraps spdlog, which in turn uses the fmt library
/// Formatting should be done in the fmt syntax, NOT printf syntax
/// https://fmt.dev/latest/syntax.html
/// </summary>
class tas_logger
{
#pragma region Singleton
private:
	static tas_logger& get() {
		static tas_logger instance;
		return instance;
	}
protected:
	std::shared_ptr<spdlog::logger> multi_logger;
private:
	tas_logger();
#pragma endregion

	template<typename T, typename... Args>
	static void default_log(spdlog::level::level_enum log_level, const T* format, Args... args) {
		auto& logger = tas_logger::get();
		logger.multi_logger->log(log_level, format, args...);
	}

public:
	template<typename T, typename... Args>
	static void trace(const T* format, Args... args)
	{
		default_log(spdlog::level::trace, format, args...);
	}

	template<typename T, typename... Args>
	static void debug(const T* format, Args... args)
	{
		default_log(spdlog::level::debug, format, args...);
	}

	template<typename T, typename... Args>
	static void info(const T* format, Args... args)
	{
		default_log(spdlog::level::info, format, args...);
	}

	template<typename T, typename... Args>
	static void warning(const T* format, Args... args)
	{
		default_log(spdlog::level::warn, format, args...);
	}

	template<typename T, typename... Args>
	static void error(const T* format, Args... args)
	{
		default_log(spdlog::level::err, format, args...);
	}

	template<typename T, typename... Args>
	static void critical(const T* format, Args... args)
	{
		default_log(spdlog::level::critical, format, args...);
	}

	static void flush_and_exit();
};


