#include "pch.h"
#include "tas_console.h"
#include "halo1_engine.h"

void tas_console::clear_buffer()
{
	ZeroMemory(mCommandBuffer, sizeof(mCommandBuffer));
}

void tas_console::clear_history()
{
	mCommandHistory.clear();
	mCurrentIndex = 0;
}

void tas_console::history_cursor_up()
{
	mCurrentIndex = std::clamp<int>(mCurrentIndex - 1, 0, (int)mCommandHistory.size());
}

void tas_console::history_cursor_down()
{
	mCurrentIndex = std::clamp<int>(mCurrentIndex + 1, 0, (int)mCommandHistory.size());
}

char* tas_console::buffer()
{
	return mCommandBuffer;
}

size_t tas_console::buffer_size()
{
	return sizeof(mCommandBuffer);
}

void tas_console::execute()
{
	std::string command(mCommandBuffer);
	if (command.length() <= 0) {
		return;
	}

	mCommandHistory.push_back(command);

	auto parsed_command = parse_command_string(command);
	// Check if it's a global command
	if (parsed_command.has_value() && parsed_command.value().is_global()) {
		execute_global(parsed_command.value());
	}
	// Otherwise pass on to the specific mode
	else {
		switch (mMode)
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
	const float DISTANCE = 10.0f;

	ImGuiIO& io = ImGui::GetIO();

	int colorStyles = 0;
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 0.0f, 1.0f, .05f)); colorStyles++;
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(255, 0, 255))); colorStyles++;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
	ImVec2 window_pos_pivot = ImVec2(0.0f, 1.0f);

	// History Window
	ImVec2 history_window_pos = ImVec2(DISTANCE, io.DisplaySize.y - DISTANCE - 62);
	ImGui::SetNextWindowPos(history_window_pos, ImGuiCond_Always, window_pos_pivot);
	ImGui::SetNextWindowBgAlpha(0.25f); // Transparent background

	if (ImGui::Begin("MCCTAS Console History", nullptr, window_flags)) {
		render_history();
	}
	ImGui::End();

	// Console input window
	ImVec2 console_window_pos = ImVec2(DISTANCE, io.DisplaySize.y - DISTANCE);
	ImGui::SetNextWindowPos(console_window_pos, ImGuiCond_Always, window_pos_pivot);
	ImGui::SetNextWindowBgAlpha(0.25f); // Transparent background
	if (ImGui::Begin("MCCTAS Console", nullptr, window_flags))
	{
		ImGui::AlignTextToFramePadding();
		ImGui::SetNextItemWidth(100);
		switch (mMode)
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
			tas_logger::error("Invalid console mode set: {}", mMode);
			ImGui::End();
			return;
		}
		ImGui::SameLine();

		if (!ImGui::IsAnyItemActive())
			ImGui::SetKeyboardFocusHere();

		float consoleWidth = std::max<float>(static_cast<float>(windowWidth - 180), 720.0f);
		ImGui::SetNextItemWidth(consoleWidth);
		ImGuiInputTextFlags command_line_flags = ImGuiInputTextFlags_EnterReturnsTrue;
		if (ImGui::InputText("", mCommandBuffer, sizeof(mCommandBuffer), command_line_flags)) {
			execute();
		}
	}
	ImGui::End();

	// Cleanup
	if (colorStyles)
		ImGui::PopStyleColor(colorStyles);
}

void tas_console::render_console()
{

}

void tas_console::render_history()
{
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	ImGui::BeginChild("ScrollingRegion", ImVec2(720, 300), false, flags);

	int id = 0;
	for (auto s = mCommandHistory.cbegin(); s != mCommandHistory.cend(); s = std::next(s)) {
		ImGui::PushID(id);
		if (ImGui::Button("<")) {
			strcpy_s(mCommandBuffer, sizeof(mCommandBuffer), s->c_str());
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
	case ConsoleCommand::MODE_SWITCH:
	{
		auto newModeString = std::get<std::string>(command.mParameters[0]);
		if (newModeString == "h3dev") {
			mMode = console_mode::HALO3DEV;
		}
		if (newModeString == "h2dev") {
			mMode = console_mode::HALO2DEV;
		}
		if (newModeString == "h1dev") {
			mMode = console_mode::HALO1DEV;
		}
		if (newModeString == "tas") {
			mMode = console_mode::MCCTAS;
		}
	}
	default:
		break;
	}
}

void tas_console::execute_h1raw(const std::string& command)
{
	halo1_engine::execute_command(command.c_str());
	tas_logger::info("Executed HALO1DEV command: {}", command);
}

void tas_console::execute_h1dev(const ParsedCommand& command)
{

}

void tas_console::execute_h2dev(const ParsedCommand& command)
{

}

void tas_console::execute_h3dev(const ParsedCommand& command)
{

}
