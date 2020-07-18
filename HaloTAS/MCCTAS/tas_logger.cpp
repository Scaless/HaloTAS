#include "tas_logger.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

tas_logger::tas_logger()
{
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	console_sink->set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
	console_sink->set_level(spdlog::level::info);

	auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("MCCTAS/MCCTAS_debug_log.txt", 5_MiB, 4, false);
	file_sink->set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
	file_sink->set_level(spdlog::level::trace);

	multi_logger = std::make_shared<spdlog::logger>("multi_sink", spdlog::sinks_init_list{ console_sink, file_sink });
	multi_logger->set_level(spdlog::level::trace);

	spdlog::set_default_logger(multi_logger);
	spdlog::set_level(spdlog::level::debug);
	spdlog::flush_on(spdlog::level::trace);
}

void tas_logger::flush_and_exit()
{
	spdlog::drop_all();
	spdlog::shutdown();
}
