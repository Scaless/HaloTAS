#include "console_parser.h"
#include <string>
#include <unordered_map>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

enum class CommandSchemaParameterType { STRING, INT, FLOAT };

struct CommandSchemaParameter {
    std::string mName;
    bool mOptional;
    CommandSchemaParameterType mType;
};

const std::unordered_map<std::string, ConsoleCommand> StrToCommand = {
    {"mode", ConsoleCommand::MODE_SWITCH},
    {"console_color", ConsoleCommand::CONSOLE_COLOR}
};

const std::unordered_map<ConsoleCommand, std::vector<CommandSchemaParameter>> SchemaTable = {
    {ConsoleCommand::MODE_SWITCH,
        std::vector<CommandSchemaParameter> {
            {"new_mode", false, CommandSchemaParameterType::STRING }
        }
    },
    {ConsoleCommand::CONSOLE_COLOR,
        std::vector<CommandSchemaParameter> {
            {"r", false, CommandSchemaParameterType::INT },
            {"g", false, CommandSchemaParameterType::INT },
            {"b", false, CommandSchemaParameterType::INT },
        }
    },
};

bool try_get_float(const std::string& input, float& outValue) {
    try {
        outValue = std::stof(input);
        return true;
    }
    catch (std::invalid_argument&) {
        return false;
    }
    catch (std::out_of_range&) {
        tas_logger::warning("Could not convert to float, out of range: {}", input);
        return false;
    }
    catch (...) {
        return false;
    }
}

bool try_get_integer(const std::string& input, int& outValue) {
    try {
        outValue = std::stoi(input);
        return true;
    }
    catch (std::invalid_argument&) {
        return false;
    }
    catch (std::out_of_range&) {
        tas_logger::warning("Could not convert to float, out of range: {}", input);
        return false;
    }
    catch (...) {
        return false;
    }
}

std::optional<ConsoleCommand> get_command(const std::string& str) {
    std::string temp = boost::algorithm::to_lower_copy(str);
    auto it = StrToCommand.find(temp);
    if (it != StrToCommand.end()) {
        return it->second;
    }
    else {
        return std::nullopt;
    }
}

std::optional<std::vector<CommandSchemaParameter>> get_parameters(ConsoleCommand command) {
    auto it = SchemaTable.find(command);
    if (it != SchemaTable.end()) {
        return it->second;
    }
    else {
        return std::nullopt;
    }
}

std::optional<ParsedCommand> parse_command_string(const std::string& str)
{
    boost::char_separator<char> separators(" ");
    boost::tokenizer<boost::char_separator<char>> tokens(str, separators);

    std::vector<std::string> stringTokens;
    for (const auto& t : tokens) {
        stringTokens.push_back(t);
    }

    // Need at least 1 string
    if (stringTokens.size() < 1) {
        return std::nullopt;
    }

    // Get Command Type
    auto commandType = get_command(stringTokens[0]);
    if (!commandType.has_value()) {
        return std::nullopt;
    }

    // Check Parameters
    ParsedCommand pCommand(commandType.value());

    auto optExpectedParameters = get_parameters(pCommand.mCommand);
    if (!optExpectedParameters.has_value()) {
        // Parameters not found for command
        return std::nullopt;
    }

    int numGivenParameters = stringTokens.size() - 1;
    auto expectedParameters = optExpectedParameters.value();
    if (numGivenParameters != expectedParameters.size()) {
        // Number of parameters given does not match expected
        return std::nullopt;
    }

    // Validate given parameters convert properly
    for (size_t i = 0; i < expectedParameters.size(); i++) {
        switch (expectedParameters[i].mType)
        {
        case CommandSchemaParameterType::FLOAT:
        {
            float e;
            if (!try_get_float(stringTokens[i + 1], e)) {
                // Failed to convert parameter
                return std::nullopt;
            }
            pCommand.mParameters.push_back(e);
            break;
        }
        case CommandSchemaParameterType::INT:
        {
            int32_t e;
            if (!try_get_integer(stringTokens[i + 1], e)) {
                // Failed to convert parameter
                return std::nullopt;
            }
            pCommand.mParameters.push_back(e);
            break;
        }
        case CommandSchemaParameterType::STRING:
        {
            pCommand.mParameters.push_back(stringTokens[i + 1]);
            break;
        }
        default:
            // Invalid parameter type, fix configuration
            return std::nullopt;
        }
    }

    return pCommand;
}

bool ParsedCommand::is_global()
{
    switch (mCommand)
    {
    case ConsoleCommand::MODE_SWITCH:
        return true;
    default:
        return false;
    }
}
