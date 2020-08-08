#pragma once

#include "pch.h"
#include "console_parser.h"

class tas_console
{
private:
	enum class console_mode { MCCTAS, HALO1DEV, HALO2DEV, HALO3DEV };

	char mCommandBuffer[1024] = "";
	std::vector<std::string> mCommandHistory;
	int mVisibleHistoryLines = 4;
	int mCurrentIndex = 0;
	console_mode mMode = console_mode::MCCTAS;

public:
	tas_console() = default;
	~tas_console() = default;

	void clear_buffer();
	void clear_history();
	void history_cursor_up();
	void history_cursor_down();
	char* buffer();
	size_t buffer_size();
	void execute();
	void render(int windowWidth);
	void render_console();
	void render_history();

private:
	void execute_global(const ParsedCommand& command);
	void execute_h1raw(const std::string& command);
	void execute_h1dev(const ParsedCommand& command);
	void execute_h2dev(const ParsedCommand& command);
	void execute_h3dev(const ParsedCommand& command);
};
