#include "tas_logger.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

tas_logger::tas_logger()
{
	std::string logPath = "HaloTASFiles/HaloTAS.log";
	auto file_logger = spdlog::basic_logger_mt("basic_logger", logPath);
	spdlog::set_default_logger(file_logger);

	spdlog::set_pattern("[%H:%M:%S.%e] [%^-%L-%$] [thread %t] %v");
	spdlog::flush_on(spdlog::level::debug);
}

void tas_logger::debug(const char* format, ...)
{
	auto& logger = tas_logger::get();
	va_list args;
	va_start(args, format);
	char buffer[2048];
	vsprintf_s(buffer, format, args);
	spdlog::debug(buffer);
}

void tas_logger::info(const char* format, ...)
{
	auto& logger = tas_logger::get();
	va_list args;
	va_start(args, format);
	char buffer[2048];
	vsprintf_s(buffer, format, args);
	spdlog::info(buffer);
}

void tas_logger::warning(const char* format, ...)
{
	auto& logger = tas_logger::get();
	va_list args;
	va_start(args, format);
	char buffer[2048];
	vsprintf_s(buffer, format, args);
	spdlog::warn(buffer);
}

void tas_logger::error(const char* format, ...)
{
	auto& logger = tas_logger::get();
	va_list args;
	va_start(args, format);
	char buffer[2048];
	vsprintf_s(buffer, format, args);
	spdlog::error(buffer);
}

void tas_logger::fatal(const char* format, ...)
{
	auto& logger = tas_logger::get();
	va_list args;
	va_start(args, format);
	char buffer[2048];
	vsprintf_s(buffer, format, args);
	spdlog::critical(buffer);
}
