#pragma once
#include "pch.h"
#include <variant>
#include <optional>

enum class ConsoleCommand { MODE_SWITCH, CONSOLE_COLOR };

struct ParsedCommand {
	ConsoleCommand mCommand;
	std::vector<std::variant<std::string, int32_t, float>> mParameters;

	ParsedCommand(ConsoleCommand command) : mCommand(command)
	{
	}
	~ParsedCommand()
	{
	}

	// Returns true if command should be executed regardless of which mode the console is in
	bool is_global();
};

std::optional<ParsedCommand> parse_command_string(const std::string& str);
