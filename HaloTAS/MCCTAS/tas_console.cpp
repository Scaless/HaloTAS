#include "pch.h"
#include "tas_hooks.h"
#include "tas_console.h"
#include "halo1_engine.h"

tas_console::tas_console()
{
	clear_history();
}

void tas_console::clear_buffer()
{
	mCommandBuffer.clear();
}

void tas_console::clear_history()
{
	mCommandHistory.clear();
	mCurrentIndex = 0;
}

void tas_console::history_cursor_up()
{
	mCurrentIndex = std::clamp<int>(mCurrentIndex + 1, 0, (int)mCommandHistory.size());
	/*int history_index = mCurrentIndex - 1;
	if (mCurrentIndex >= 1 && mCommandHistory.size() > history_index) {
		mCommandBuffer = mCommandHistory[history_index];
	}*/
}

void tas_console::history_cursor_down()
{
	mCurrentIndex = std::clamp<int>(mCurrentIndex - 1, 0, (int)mCommandHistory.size());
}

void tas_console::execute(std::string& buffer)
{
	std::string command(buffer);
	if (command.length() <= 0) {
		return;
	}
	// Remove newlines from enter press
	command.erase(std::remove(command.begin(), command.end(), '\n'), command.end());
	command.erase(std::remove(command.begin(), command.end(), '\r'), command.end());

	mCommandHistory.push_back(command);

	auto parsed_command = parse_command_string(command);
	// Check if it's a global command
	if (parsed_command.has_value() && parsed_command.value().is_global()) {
		execute_global(parsed_command.value());
	}
	// Otherwise pass on to the specific mode
	else {
		switch (mConsoleMode)
		{
		case tas_console::console_mode::MCCTAS:
		{
			// TODO-SCALES: Handler for tas commands
			break;
		}
		case tas_console::console_mode::HALO1DEV:
		{
			if (parsed_command.has_value()) {
				execute_h1dev(parsed_command.value());
			}
			else {
				execute_h1raw(command);
			}
			break;
		}
		case tas_console::console_mode::HALO2DEV:
		{
			execute_h2dev(parsed_command.value());
			break;
		}
		case tas_console::console_mode::HALO3DEV:
		{
			execute_h3dev(parsed_command.value());
			break;
		}
		default:
			break;
		}
	}

	mCurrentIndex = 0;
	clear_buffer();
}

void tas_console::render(int windowWidth)
{
	const float PADDING = 10.0f;
	const float MARGIN = 2.0f;
	const float CONSOLE_HEIGHT = 40.0f;

	ImGuiIO& io = ImGui::GetIO();

	int color_style_count = 0;
	ImGui::PushStyleColor(ImGuiCol_FrameBg, tasgui::FadedBackground); color_style_count++;
	ImGui::PushStyleColor(ImGuiCol_Text, tasgui::Magenta); color_style_count++;
	
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
	ImVec2 window_pos_pivot = ImVec2(0.0f, 1.0f);

	// Console Output Window
	ImVec2 console_output_window_pos = ImVec2(PADDING, io.DisplaySize.y - PADDING - CONSOLE_HEIGHT - MARGIN - 300 - MARGIN);
	ImGui::SetNextWindowPos(console_output_window_pos, ImGuiCond_Always, window_pos_pivot);
	ImGui::SetNextWindowBgAlpha(0.5f); // Transparent background
	if (ImGui::Begin("MCCTAS Console Output", nullptr, window_flags)) {
		render_console_output();
	}
	ImGui::End();


	// History Window
	ImVec2 history_window_pos = ImVec2(PADDING, io.DisplaySize.y - PADDING - CONSOLE_HEIGHT - MARGIN);
	ImGui::SetNextWindowPos(history_window_pos, ImGuiCond_Always, window_pos_pivot);
	ImGui::SetNextWindowBgAlpha(0.25f); // Transparent background
	if (ImGui::Begin("MCCTAS Console History", nullptr, window_flags)) {
		render_history();
	}
	ImGui::End();

	// Console input window
	ImVec2 console_window_pos = ImVec2(PADDING, io.DisplaySize.y - PADDING);
	ImGui::SetNextWindowPos(console_window_pos, ImGuiCond_Always, window_pos_pivot);
	ImGui::SetNextWindowSize(ImVec2(windowWidth - PADDING * 2, CONSOLE_HEIGHT));
	ImGui::SetNextWindowBgAlpha(0.25f); // Transparent background
	if (ImGui::Begin("MCCTAS Console", nullptr, window_flags))
	{
		ImGui::AlignTextToFramePadding();
		ImGui::SetNextItemWidth(100);
		switch (mConsoleMode)
		{
		case tas_console::console_mode::MCCTAS:
			ImGui::Text("mcctas(");
			break;
		case tas_console::console_mode::HALO1DEV:
			ImGui::Text("halo1(");
			break;
		case tas_console::console_mode::HALO2DEV:
			ImGui::Text("halo2(");
			break;
		case tas_console::console_mode::HALO3DEV:
			ImGui::Text("halo3(");
			break;
		default:
			tas_logger::error("Invalid console mode set: {}", mConsoleMode);
			ImGui::Text("Invalid(");
			ImGui::End();
			return;
		}
		ImGui::SameLine();

		if (!ImGui::IsAnyItemActive())
			ImGui::SetKeyboardFocusHere();

		// Fill remaining space with console input
		float command_line_width = std::max<float>(static_cast<float>(ImGui::GetContentRegionAvail().x), 720.0f);
		ImGui::SetNextItemWidth(command_line_width);
		ImGuiInputTextFlags command_line_flags = ImGuiInputTextFlags_EnterReturnsTrue;
		if (ImGui::InputText("", &mCommandBuffer, command_line_flags)) {
			execute(mCommandBuffer);
		}
	}
	ImGui::End();

	// Cleanup
	if (color_style_count)
		ImGui::PopStyleColor(color_style_count);
}

void tas_console::render_console()
{

}

void tas_console::render_console_output()
{
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	ImGui::BeginChild("ScrollingRegion", ImVec2(720, 300), false, flags);

	auto new_vsprintf = tas_hooks::get_sprintf_values();

	for (auto& s : new_vsprintf) {
		if (s.find("mode") != std::string::npos) { continue; }
		if (s.find("HaloTAS") != std::string::npos) { continue; }
		if (s.find("MCCTAS") != std::string::npos) { continue; }
		if (s.find("mcctas") != std::string::npos) { continue; }
		if (s.find("Current Engine") != std::string::npos) { continue; }
		if (s.find("halo1(") != std::string::npos) { continue; }
		if (s.find("unit_get_health (player0)") != std::string::npos) { continue; }

		tas_logger::warning(s.c_str());
		//ImGui::Text(s.c_str());
	}

	ImGui::EndChild();
}

void tas_console::render_history()
{
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	ImGui::BeginChild("ScrollingRegion", ImVec2(720, 300), false, flags);

	int id = 0;
	for (auto s = mCommandHistory.cbegin(); s != mCommandHistory.cend(); s = std::next(s)) {
		ImGui::PushID(id);
		if (ImGui::Button("<")) {
			if (mCurrentIndex >= 1 && mCommandHistory.size() > id) {
				mCommandBuffer = mCommandHistory[id];
			}
		}
		ImGui::SameLine();
		ImGui::AlignTextToFramePadding();
		ImGui::Text(s->c_str());
		ImGui::PopID();
		id++;
	}

	ImGui::SetScrollHereY(1.0f);

	ImGui::EndChild();
}

void tas_console::execute_global(const ParsedCommand& command)
{
	tas_logger::info("Executed MCCTAS command: {}", command.mCommand);

	switch (command.mCommand)
	{
	case ConsoleCommand::HELP: 
	{
		tas_logger::info(
			"Available commands:\r\n"
			"help      | <- You are here\r\n"
			"mode XXX  | Switch the current console mode. Valid modes: 'tas', 'h1dev'\r\n"
			"    'tas' - Console for entering TAS commands\r\n"
			"    'h1dev' - Halo 1 developer console, requires a valid engine handle\r\n"
		);
	} break;
	case ConsoleCommand::MODE_SWITCH:
	{
		auto newModeString = std::get<std::string>(command.mParameters[0]);
		if (newModeString == "h3dev") {
			mConsoleMode = console_mode::HALO3DEV;
		}
		if (newModeString == "h2dev") {
			mConsoleMode = console_mode::HALO2DEV;
		}
		if (newModeString == "h1dev") {
			if (tas_hooks::get_loaded_engine() == GameEngineType::Halo1) {
				mConsoleMode = console_mode::HALO1DEV;
			}
			else {
				tas_logger::warning("Can't switch to h1dev because we don't have an engine handle! If you are in-game, save&quit and load back into the level. "
				"If you are in the MCC menus, open the Missions screen of a different game, then load back into the game you want."); 
			}
		}
		if (newModeString == "tas") {
			mConsoleMode = console_mode::MCCTAS;
		}
	} break;
	}
}

void tas_console::execute_h1raw(const std::string& command)
{
	tas_hooks::execute_halo1_command(command);
	//halo1_engine::execute_command(command.c_str());
	tas_logger::info("Executed HALO1DEV command: {}", command);
}

void tas_console::execute_h1dev(const ParsedCommand& /*command*/)
{

}

void tas_console::execute_h2dev(const ParsedCommand& /*command*/)
{

}

void tas_console::execute_h3dev(const ParsedCommand& /*command*/)
{

}
