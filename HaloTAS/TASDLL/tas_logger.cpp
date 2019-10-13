#include "tas_logger.h"

#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

namespace logging = boost::log;
namespace keywords = boost::log::keywords;

tas_logger::tas_logger()
{
	logging::add_common_attributes();
	boost::log::register_simple_formatter_factory< boost::log::trivial::severity_level, char >("Severity");
	logging::add_file_log(
		keywords::file_name = "HaloTAS.log",
		keywords::format = "[%TimeStamp%]: [%Severity%] %Message%",
		keywords::auto_flush = true,
		keywords::open_mode = std::ios_base::out | std::ios_base::app,
		keywords::max_size = 64 * 1024 * 1024
	);
}

void tas_logger::debug(const char* format, ...)
{
	auto& logger = tas_logger::get();
	va_list args;
	va_start(args, format);
	char buffer[2048];
	vsprintf_s(buffer, format, args);
	BOOST_LOG_TRIVIAL(debug) << buffer;
}

void tas_logger::info(const char* format, ...)
{
	auto& logger = tas_logger::get();
	va_list args;
	va_start(args, format);
	char buffer[2048];
	vsprintf_s(buffer, format, args);
	BOOST_LOG_TRIVIAL(info) << buffer;
}

void tas_logger::warning(const char* format, ...)
{
	auto& logger = tas_logger::get();
	va_list args;
	va_start(args, format);
	char buffer[2048];
	vsprintf_s(buffer, format, args);
	BOOST_LOG_TRIVIAL(warning) << buffer;
}

void tas_logger::error(const char* format, ...)
{
	auto& logger = tas_logger::get();
	va_list args;
	va_start(args, format);
	char buffer[2048];
	vsprintf_s(buffer, format, args);
	BOOST_LOG_TRIVIAL(error) << buffer;
}

void tas_logger::fatal(const char* format, ...)
{
	auto& logger = tas_logger::get();
	va_list args;
	va_start(args, format);
	char buffer[2048];
	vsprintf_s(buffer, format, args);
	BOOST_LOG_TRIVIAL(fatal) << buffer;
}
