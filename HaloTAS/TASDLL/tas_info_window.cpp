#include "globals.h"
#include "tas_info_window.h"
#include "tas_input_handler.h"
#include <unordered_set>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

tas_info_window::tas_info_window()
{
	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(1280, 720, "Game Info", NULL, NULL);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	gl3wInit();

	atlas = new ImFontAtlas();
	atlas->AddFontDefault();
	atlas->Build();

	imguiCtx = ImGui::CreateContext(atlas);
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 150");
}

tas_info_window::~tas_info_window()
{
	glfwMakeContextCurrent(window);
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext(imguiCtx);
	glfwDestroyWindow(window);
}

void tas_info_window::render_overlay()
{
	if (!ImGui::CollapsingHeader("Overlay")) {
		return;
	}

	ImGui::Checkbox("Enabled", &currentInput.overlayOptions.enabled);
	ImGui::Checkbox("Show Primitives", &currentInput.overlayOptions.showPrimitives);
	ImGui::SliderFloat("Cull Distance", &currentInput.overlayOptions.cullDistance, 1, 250);
	ImGui::DragFloat("UI Scale", &currentInput.overlayOptions.uiScale, .1f, 1, 20);

	engine_snapshot snapshot = {};
	gEngine->get_snapshot(snapshot);

	std::unordered_set<uint32_t> objectCategories;
	for (auto& v : snapshot.gameObjects) {
		if (objectCategories.find(v->tag_id) == objectCategories.end()) {
			objectCategories.insert(v->tag_id);
		}
	}
	auto player = GetPlayerObject(snapshot.gameObjects);

	if (ImGui::CollapsingHeader("Objects"))
	{
		auto countf = 1;
		for (auto& tagId : objectCategories) {
			std::string tagName = "UNKNOWN" + std::to_string(countf);
			if (KNOWN_TAGS.count(tagId)) {
				tagName = KNOWN_TAGS.at(tagId).displayName;
			}
			ImGui::PushID(countf);
			ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
			if (ImGui::CollapsingHeader(tagName.c_str())) {
				auto count = 1;
				for (auto& v : snapshot.gameObjects) {
					if (v->tag_id == tagId) {
						ImGui::PushID(count * countf + count);

						ImGui::Text("Addr: %p", v);
						ImGui::SameLine();

						ImGui::PushItemWidth(300);
						ImGui::DragFloat3("POS", &(v->unit_x), .1f, -10000, 10000);

						ImGui::SameLine();
						glm::vec3 eulerRotation = glm::eulerAngles(v->rotationQuaternion);
						if (ImGui::DragFloat3("ROT", &eulerRotation.x, .1f, -10000, 10000)) {
							v->rotationQuaternion = glm::quat(eulerRotation);
						}
						ImGui::PopItemWidth();

						ImGui::SameLine();
						if (ImGui::Button("MoveThisToMe")) {
							if (player != nullptr) {
								v->unit_x = player->unit_x + 1;
								v->unit_y = player->unit_y;
								v->unit_z = player->unit_z;
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("MoveMeToThis")) {
							if (player != nullptr) {
								player->unit_x = v->unit_x;
								player->unit_y = v->unit_y;
								player->unit_z = v->unit_z + .5f;
							}
						}
						count++;
						ImGui::PopID();
					}
				}
			}
			ImGui::PopStyleColor();
			countf++;
			ImGui::PopID();
		}
	}

	if (ImGui::CollapsingHeader("Manual Input"))
	{
		ImGui::Text("Camera Position: (%f,%f,%f)", ADDR_CAMERA_POSITION->x, ADDR_CAMERA_POSITION->y, ADDR_CAMERA_POSITION->z);
		ImGui::SameLine();
		ImGui::Text("Look Direction: (%f,%f,%f)", ADDR_CAMERA_LOOK_VECTOR[0], ADDR_CAMERA_LOOK_VECTOR[1], ADDR_CAMERA_LOOK_VECTOR[2]);

		ImGui::DragFloat3("Camera Movement", glm::value_ptr(*ADDR_CAMERA_POSITION), .1f);
		ImGui::DragFloat3("Camera Angle", ADDR_CAMERA_LOOK_VECTOR, .05f);

		int tempLeftMouse = *ADDR_LEFTMOUSE;
		if (ImGui::SliderInt("LeftMouse", &tempLeftMouse, 0, 255)) {
			*ADDR_LEFTMOUSE = (uint8_t)tempLeftMouse;
		}

		int tempRightMouse = *ADDR_RIGHTMOUSE;
		if (ImGui::SliderInt("RightMouse", &tempRightMouse, 0, 255)) {
			*ADDR_RIGHTMOUSE = (uint8_t)tempRightMouse;
		}

		ImGui::SliderInt("RawMouseX", ADDR_DINPUT_MOUSEX, -5, 5);
		ImGui::SliderInt("RawMouseY", ADDR_DINPUT_MOUSEY, -5, 5);

		ImGui::SliderFloat("Vertical View Angle", ADDR_UPDOWNVIEW, -1.492f, 1.492f);
		ImGui::SliderFloat("Horizontal View Angle", ADDR_LEFTRIGHTVIEW, 0, glm::pi<float>() * 2.0f);

		ImGui::Columns(6, "inputmap", true);
		ImGui::Separator();

		for (int n = 0; n < 104; n++)
		{
			bool colorChanged = false;
			if (ADDR_KEYBOARD_INPUT[n] > 0) {
				ImGui::PushStyleColor(ImGuiCol_SliderGrab, (ImVec4)ImColor::ImColor(0.0f, 0.5f, 0.0f));
				colorChanged = true;
			}

			ImGui::PushID(KEY_PRINT_CODES[n].c_str());
			int tempInputVal = (int)ADDR_KEYBOARD_INPUT[n];
			if (ImGui::SliderInt(KEY_PRINT_CODES[n].c_str(), &tempInputVal, 0, 255)) {
				ADDR_KEYBOARD_INPUT[n] = (uint8_t)tempInputVal;
			}
			ImGui::PopID();

			if (colorChanged)
				ImGui::PopStyleColor();

			ImGui::NextColumn();
		}
		ImGui::Columns(1);
	}

	if (ImGui::CollapsingHeader("Current Input"))
	{
		ImGui::PushItemWidth(200);

		for (int n = 0; n < 104; n++)
		{
			int tempInputVal = (int)ADDR_KEYBOARD_INPUT[n];
			if (tempInputVal > 0) {
				ImGui::SliderInt(KEY_PRINT_CODES[n].c_str(), &tempInputVal, 0, 255);
			}
		}

		ImGui::PopItemWidth();
	}
}

void tas_info_window::render_tas()
{
	if (!ImGui::CollapsingHeader("TAS")) {
		return;
	}

	ImGui::Checkbox("Record", &currentInput.record);
	ImGui::SameLine();
	ImGui::Checkbox("Playback", &currentInput.playback);
	ImGui::SameLine();
	currentInput.loadPlayback = ImGui::Button("Load Playback");
	ImGui::SameLine();
	if (ImGui::Button("PatchMouse")) {
		gEngine->mouse_directinput_override_enable();
	}
	ImGui::SameLine();
	if (ImGui::Button("UnPatchMouse")) {
		gEngine->mouse_directinput_override_disable();
	}

	if (ImGui::Button("Load Checkpoint")) {
		*ADDR_LOAD_CHECKPOINT = 1;
	}
	ImGui::SameLine();
	if (ImGui::Button("Save Checkpoint")) {
		*ADDR_SAVE_CHECKPOINT = 1;
	}
	ImGui::SameLine();
	if (ImGui::Button("Restart Level (Full?)")) {
		*ADDR_RESTART_LEVEL = 1;
	}
	ImGui::SameLine();
	if (ImGui::Button("Restart Level (Partial?)")) {
		*ADDR_RESTART_LEVEL_FULL = 1;
	}

	if (ImGui::Button("PAUSE")) {
		*ADDR_GAME_SPEED = 0;
	}
	ImGui::SameLine();
	if (ImGui::Button("PLAY")) {
		*ADDR_GAME_SPEED = 1;
	}

	if (ImGui::Button("SAVE")) {
		gInputHandler->save_inputs();
	}
}

static int32_t tick_delete_start = 0, tick_delete_end = 0;
static int32_t tick_insert_start = 0, tick_insert_count = 0;
static int32_t new_input_multiple_count = 0;
static int32_t remove_input_multiple_count = 0;
static std::string current_item;

void tas_info_window::render_inputs()
{
	if (ImGui::BeginCombo("##levelSelect", current_item.c_str()))
	{
		for (const auto& level : gInputHandler->get_loaded_levels()) {

			bool is_selected = level == current_item; // You can store your selection however you want, outside or inside your objects
			const char* levelCstr = level.c_str();
			if (ImGui::Selectable(levelCstr, is_selected))
				current_item = level;
			if (is_selected)
				ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
		}

		ImGui::EndCombo();
	}

	if (current_item.empty()) {
		return;
	}

	auto input = gInputHandler->get_inputs(current_item);
	if (input == nullptr) {
		return;
	}

	if (ImGui::CollapsingHeader("TAS Input")) {

		ImGui::Text("Remove Ticks");
		ImGui::SameLine();
		ImGui::PushItemWidth(100);
		ImGui::InputInt("##DeleteTickStart", &tick_delete_start);
		ImGui::SameLine();
		ImGui::InputInt("##DeleteTickEnd", &tick_delete_end);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		if (ImGui::Button("Delete Tick Range")) {
			input->remove_tick_range(tick_delete_start, tick_delete_end);
		}

		ImGui::Text("Insert Ticks");
		ImGui::SameLine();
		ImGui::PushItemWidth(100);
		ImGui::InputInt("##InsertTickStart", &tick_insert_start);
		ImGui::SameLine();
		ImGui::InputInt("##InsertTickEnd", &tick_insert_count);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		if (ImGui::Button("Insert Tick Range")) {
			input->insert_tick_range(tick_insert_start, tick_insert_count);
		}

		ImGui::Columns(5);
		ImGui::SetColumnWidth(0, 60);
		ImGui::SetColumnWidth(1, 100);
		ImGui::SetColumnWidth(2, 100);
		ImGui::SetColumnWidth(3, 200);
		ImGui::SetColumnWidth(4, 200);
		ImGui::Text("Tick"); ImGui::NextColumn();
		ImGui::Text("Pitch"); ImGui::NextColumn();
		ImGui::Text("Yaw"); ImGui::NextColumn();
		ImGui::Text("KB Input"); ImGui::NextColumn();
		ImGui::Text("Mouse Input"); ImGui::NextColumn();
		ImGui::Columns(1);

		ImGui::BeginChild("Inputs##TASINPUT", ImGui::GetContentRegionAvail(), true, ImGuiWindowFlags_HorizontalScrollbar);

		ImGui::Columns(5);
		ImGui::SetColumnWidth(0, 52);
		ImGui::SetColumnWidth(1, 100);
		ImGui::SetColumnWidth(2, 100);
		ImGui::SetColumnWidth(3, 200);
		ImGui::SetColumnWidth(4, 200);

		int count = 0;
		for(auto it = input->input_buffer()->begin(); it != input->input_buffer()->end(); ++it) {
			ImGui::PushID(count);
			
			int styles = 0;
			if (gInputHandler->get_current_playback_tick() == count) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1, 0, 0, 1));
				styles += 2;
			}

			ImGui::Text("%d", count);
			//ImGui::Text("%d", it->tick);
			ImGui::NextColumn();
			ImGui::DragFloat("##Pitch", &(it->cameraPitch), .1f);
			ImGui::NextColumn();
			ImGui::DragFloat("##Yaw", &(it->cameraYaw), .1f);
			ImGui::NextColumn();

			// INPUTS
			////////////////////////
			if (ImGui::Button("+")) {
				ImGui::OpenPopup("new_input");
			}
			if (ImGui::BeginPopup("new_input")) {

				for (int key = 0; key < KEYS::KEY_COUNT; key++) {
					// Add the key to the current input
					if (ImGui::BeginMenu(KEY_PRINT_CODES[key].c_str()))
					{
						if (ImGui::MenuItem("x1")) {
							// Add input
							input->set_kb_input(count, (KEYS)key, 1);
						}
						if (ImGui::BeginMenu("xX")) {
							
							ImGui::SliderInt("##new_input_count", &new_input_multiple_count, 1, 100);
							ImGui::SameLine();
							if (ImGui::Button("Add##new_input_count")) {
								for (int i = count; i < count + new_input_multiple_count; i++) {
									input->set_kb_input(i, (KEYS)key, 1);
								}
							}
							ImGui::EndMenu();
						}
						ImGui::EndMenu();
					}

				}
				ImGui::EndPopup();
			}
			ImGui::SameLine();

			for (int key = 0; key < KEYS::KEY_COUNT; key++) {
				if (it->inputBuf[key]) {
					const char* btnId = KEY_PRINT_CODES[key].c_str();
					ImGui::PushID(btnId);
					if (ImGui::Button(btnId)) {
						ImGui::OpenPopup("key_dialog");
					}
					if (ImGui::BeginPopup("key_dialog")) {
						if (ImGui::MenuItem("Remove")) {
							// Remove input
							input->set_kb_input(count, (KEYS)key, 0);
						}
						if (ImGui::BeginMenu("Remove #")) {

							ImGui::SliderInt("##remove_input_count", &remove_input_multiple_count, 1, 100);
							ImGui::SameLine();
							if (ImGui::Button("Remove##remove_input_count")) {
								for (int i = count; i < count + remove_input_multiple_count; i++) {
									input->set_kb_input(i, (KEYS)key, 0);
								}
							}
							ImGui::EndMenu();
						}
						ImGui::EndPopup();
					}
					ImGui::PopID();
					ImGui::SameLine();
				}
			}
			
			ImGui::NextColumn();

			ImGui::PushItemWidth(50);
			ImGui::Text("LMB:");
			ImGui::SameLine();
			if (it->leftMouse > 0) {
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, .5f, 0, 1));
				ImGui::DragScalar("##LMB", ImGuiDataType_U8, &(it->leftMouse), 1);
				ImGui::PopStyleColor();
			}
			else {
				ImGui::DragScalar("##LMB", ImGuiDataType_U8, &(it->leftMouse), 1);
			}
			ImGui::SameLine();

			ImGui::Text("RMB:");
			ImGui::SameLine();
			if (it->rightMouse > 0) {
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, .5f, 0, 1));
				ImGui::DragScalar("##RMB", ImGuiDataType_U8, &(it->rightMouse), 1);
				ImGui::PopStyleColor();
			}
			else {
				ImGui::DragScalar("##RMB", ImGuiDataType_U8, &(it->rightMouse), 1);
			}
			ImGui::PopItemWidth();
			ImGui::NextColumn();

			if (*ADDR_SIMULATION_TICK_2 == it->tick && *ADDR_GAME_SPEED > 0) {
				//ImGui::SetScrollHereY();
			}

			if (styles > 0)
			{
				ImGui::PopStyleColor(styles);
			}

			ImGui::PopID();
			count++;
		}

		ImGui::Columns(1);

		ImGui::EndChild();
	}

	gInputHandler->reload_playback_buffer(input);


}

void tas_info_window::render_menubar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Close")) {
				close = true;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Paste", "CTRL+V")) {}
			ImGui::EndMenu();
		}
		const char* appText = "App: %.0f ms/frame (%.0f FPS)";
		ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - ImGui::CalcTextSize(appText).x);
		ImGui::Text(appText, 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::EndMainMenuBar();
	}
}

static float lockedGameSpeed = 1.0f;
static bool lockSpeed = false;
void tas_info_window::render_header()
{
	ImGui::Text("Map: %s\t", ADDR_MAP_STRING);
	ImGui::SameLine();
	ImGui::Text("Tick: %d\t", *ADDR_SIMULATION_TICK_2);
	ImGui::SameLine();
	ImGui::Text("Frame: %d\t", *ADDR_FRAMES_SINCE_LEVEL_START);
	ImGui::SameLine();
	//ImGui::Text("RNG: %d\t", *ADDR_RNG);
	//ImGui::SameLine();
	ImGui::PushItemWidth(100);
	ImGui::InputInt("RNG VAL", ADDR_RNG, INT_MIN, INT_MAX);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	ImGui::Text("RNG Calls: %d", gInputHandler->get_rng_advances_since_last_tick());
	ImGui::SameLine();
	if (ImGui::Button("Clear RNG Graph")) {
		gInputHandler->clear_rng_histogram_data();
	}
	ImGui::SameLine();
	if (ImGui::Button("Dummy RNG Value")) {
		gInputHandler->insert_dummy_rng_histogram_value();
	}

	auto histogram_data = gInputHandler->get_rng_histogram_data();

	auto data_max = std::max_element(std::begin(histogram_data), std::end(histogram_data));

	ImGui::PushItemWidth(ImGui::GetWindowContentRegionMax().x);
	ImGui::PlotHistogram("##RNG Histogram", histogram_data.data(), histogram_data.size(), 0, NULL, 0.0f, *data_max, ImVec2(0,200));
	ImGui::PopItemWidth();

	ImGui::PushItemWidth(200);
	ImGui::DragFloat("Game Speed", ADDR_GAME_SPEED, .005f, 0, 4);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	if (ImGui::Checkbox("Lock", &lockSpeed)) {
		lockedGameSpeed = *ADDR_GAME_SPEED;
	}

	if (lockSpeed) {
		*ADDR_GAME_SPEED = lockedGameSpeed;
	}

	ImGui::Checkbox("Force Simulate", &currentInput.forceSimulate);
	*ADDR_SIMULATE = currentInput.forceSimulate ? 0 : 1;
	*ADDR_ALLOW_INPUT = (*ADDR_SIMULATE == 1 ? 0 : 1);
}

void tas_info_window::render_imgui()
{
	ImGui::SetCurrentContext(imguiCtx);
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	ImGui::SetNextWindowSize(ImVec2(static_cast<float>(width), height - 20.0f));
	ImGui::SetNextWindowPos(ImVec2(0, 20));

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

	ImGui::Begin("Main", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

	render_menubar();
	render_header();
	render_overlay();
	render_tas();
	render_inputs();

	ImGui::ShowDemoWindow();

	ImGui::End();
	ImGui::PopStyleVar(2);

	// Rendering
	ImGui::Render();
	int display_w, display_h;
	glfwMakeContextCurrent(window);
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void tas_info_window::render()
{
	close = glfwWindowShouldClose(window);

	render_imgui();

	glfwSwapBuffers(window);
}

tas_info_input& tas_info_window::getInput()
{
	return currentInput;
}

bool tas_info_window::shouldClose()
{
	return close;
}