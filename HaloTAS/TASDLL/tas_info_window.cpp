#include "tas_info_window.h"
#include "tas_input_handler.h"
#include "tas_options.h"
#include "render_d3d9.h"
#include <unordered_set>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace halo::addr;

tas_info_window::tas_info_window()
{
	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	auto& gEngine = halo_engine::get();
	RECT haloClientRect = gEngine.window_client_rect();

	if (!IsRectEmpty(&haloClientRect)) {
		int targetWidth = haloClientRect.right - haloClientRect.left;
		int targetHeight = haloClientRect.bottom - haloClientRect.top;
		window = glfwCreateWindow(targetWidth, targetHeight, "Game Info", NULL, NULL);
		glfwSetWindowSize(window, 1280, 720);
	}
	else {
		window = glfwCreateWindow(1280, 720, "Game Info", NULL, NULL);
	}

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

//static glm::vec3 camDistance;
//static bool camFollow = false;
void tas_info_window::render_overlay()
{
	engine_snapshot snapshot = {};
	auto& engine = halo_engine::get();
	engine.get_snapshot(snapshot);

	/*auto debugA = reinterpret_cast<uint8_t*>(0x006AC568);
	if (*debugA == 0x90 && player != nullptr) {
		(*CAMERA_POSITION).x = player->unit_x + camDistance.x;
		(*CAMERA_POSITION).y = player->unit_y + camDistance.y;
		(*CAMERA_POSITION).z = player->unit_z + camDistance.z;
	}*/

	if (!ImGui::CollapsingHeader("Overlay##Overlay")) {
		return;
	}

	auto player = GetPlayerObject(snapshot.gameObjects);

	std::unordered_set<uint32_t> objectCategories;
	for (auto& v : snapshot.gameObjects) {
		if (objectCategories.find(v->tag_id) == objectCategories.end()) {
			objectCategories.insert(v->tag_id);
		}
	}

	ImGui::Checkbox("Enabled", &currentInput.overlayOptions.enabled);
	//ImGui::Checkbox("Show Primitives", &currentInput.overlayOptions.showPrimitives);
	if (ImGui::TreeNode("Options##Overlay")) {
		ImGui::SliderFloat("Cull Distance", &currentInput.overlayOptions.cullDistance, 1, 250);
		ImGui::DragFloat("UI Scale", &currentInput.overlayOptions.uiScale, .1f, 1, 20);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Objects##Overlay"))
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

						ImGui::Text("%p", v);
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
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Manual Input##Overlay"))
	{
		ImGui::Text("Camera Position: (%f,%f,%f)", CAMERA_POSITION->x, CAMERA_POSITION->y, CAMERA_POSITION->z);
		ImGui::SameLine();
		ImGui::Text("Look Direction: (%f,%f,%f)", CAMERA_LOOK_VECTOR[0], CAMERA_LOOK_VECTOR[1], CAMERA_LOOK_VECTOR[2]);

		ImGui::DragFloat3("Camera Movement", glm::value_ptr(*CAMERA_POSITION), .1f);
		ImGui::DragFloat3("Camera Angle", CAMERA_LOOK_VECTOR, .05f);

		int tempLeftMouse = *LEFTMOUSE;
		if (ImGui::SliderInt("LeftMouse", &tempLeftMouse, 0, 255)) {
			*LEFTMOUSE = (uint8_t)tempLeftMouse;
		}

		int tempRightMouse = *RIGHTMOUSE;
		if (ImGui::SliderInt("RightMouse", &tempRightMouse, 0, 255)) {
			*RIGHTMOUSE = (uint8_t)tempRightMouse;
		}

		ImGui::SliderInt("RawMouseX", DINPUT_MOUSEX, -5, 5);
		ImGui::SliderInt("RawMouseY", DINPUT_MOUSEY, -5, 5);

		ImGui::SliderFloat("Vertical View Angle", UPDOWNVIEW, -1.492f, 1.492f);
		ImGui::SliderFloat("Horizontal View Angle", LEFTRIGHTVIEW, 0, glm::pi<float>() * 2.0f);

		ImGui::Columns(6, "inputmap", true);
		ImGui::Separator();

		for (int n = 0; n < 104; n++)
		{
			bool colorChanged = false;
			if (KEYBOARD_INPUT[n] > 0) {
				ImGui::PushStyleColor(ImGuiCol_SliderGrab, (ImVec4)ImColor::ImColor(0.0f, 0.5f, 0.0f));
				colorChanged = true;
			}

			ImGui::PushID(KEY_PRINT_CODES[n].c_str());
			int tempInputVal = (int)KEYBOARD_INPUT[n];
			if (ImGui::SliderInt(KEY_PRINT_CODES[n].c_str(), &tempInputVal, 0, 255)) {
				KEYBOARD_INPUT[n] = (uint8_t)tempInputVal;
			}
			ImGui::PopID();

			if (colorChanged)
				ImGui::PopStyleColor();

			ImGui::NextColumn();
		}
		ImGui::Columns(1);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Current Input"))
	{
		ImGui::PushItemWidth(200);

		for (int n = 0; n < 104; n++)
		{
			int tempInputVal = (int)KEYBOARD_INPUT[n];
			if (tempInputVal > 0) {
				ImGui::SliderInt(KEY_PRINT_CODES[n].c_str(), &tempInputVal, 0, 255);
			}
		}

		ImGui::PopItemWidth();
		ImGui::TreePop();
	}
}

static int32_t int_handler_tick = 0;
void tas_info_window::render_tas()
{
	if (!ImGui::CollapsingHeader("TAS")) {
		return;
	}

	auto& gEngine = halo_engine::get();

	ImGui::Checkbox("Record", &currentInput.record);
	ImGui::SameLine();
	if (ImGui::Checkbox("Playback", &currentInput.playback)) {
		if (currentInput.playback) {
			gEngine.mouse_directinput_override_enable();
		}
		else {
			gEngine.mouse_directinput_override_disable();
		}
	};
	ImGui::SameLine();
	currentInput.loadPlayback = ImGui::Button("Load Playback");
	
	ImGui::SameLine();
	if (ImGui::Button("SAVE")) {
		auto& gInputHandler = tas_input_handler::get();
		gInputHandler.save_inputs();
	}

	if (ImGui::TreeNode("Engine Functions"))
	{
		if (ImGui::Button("Load Checkpoint")) {
			gEngine.load_checkpoint();
		}
		ImGui::SameLine();
		if (ImGui::Button("Save Checkpoint")) {
			gEngine.save_checkpoint();
		}
		ImGui::SameLine();
		if (ImGui::Button("Restart Level")) {
			gEngine.map_reset();
		}
		ImGui::SameLine();
		if (ImGui::Button("core_save")) {
			gEngine.core_save();
		}
		ImGui::SameLine();
		if (ImGui::Button("core_load")) {
			gEngine.core_load();
		}

		ImGui::TreePop();
	}

	// Remove debug camera until it is fully understood and works
	/*static bool enableDebugCamera{ false };
	if (ImGui::Checkbox("Enable Debug Camera", &enableDebugCamera)) {
		gEngine.set_debug_camera(enableDebugCamera);
	}
	ImGui::Checkbox("Cam Follow", &camFollow);
	ImGui::DragFloat3("Cam Distance", glm::value_ptr(camDistance), .05f, -20.0f, 20.0f);*/

	if (ImGui::TreeNode("TAS Functions"))
	{
		static int advanceTicks = 1;
		ImGui::SliderInt("Ticks To Advance", &advanceTicks, 1, 100);

		if (ImGui::Button("PAUSE")) {
			*GAME_SPEED = 0;
			gEngine.fast_forward_to(0);
		}
		ImGui::SameLine();
		if (ImGui::Button("PLAY")) {
			*GAME_SPEED = 1;
		}
		ImGui::SameLine();
		if (ImGui::Button("REVERSE X TICK")) {
			gEngine.fast_forward_to(std::clamp(*SIMULATION_TICK - advanceTicks, 1, INT_MAX));
		}
		ImGui::SameLine();
		if (ImGui::Button("ADVANCE X TICK")) {
			gEngine.fast_forward_to(*SIMULATION_TICK + advanceTicks);
		}

		ImGui::TreePop();
	}

	render_inputs();
}

void tas_info_window::render_d3d()
{
	if (!ImGui::CollapsingHeader("D3D")) {
		return;
	}

	auto& d3d = render_d3d9::get();
	auto& engine = halo_engine::get();

	// Enabled
	static bool enabled = false;
	if (ImGui::Checkbox("Enabled##D3D", &enabled)) {
		d3d.SetEnabled(enabled);
	}

	static bool automaticallyLoadBasedOnCurrentBSP = true;
	ImGui::Checkbox("Auto-load based on current BSP", &automaticallyLoadBasedOnCurrentBSP);
	auto& models = d3d.Models();
	auto bspNameObj = engine.current_bsp_name() + ".obj";

	if (!automaticallyLoadBasedOnCurrentBSP) {
		if (ImGui::TreeNode("Models")) {

			ImGui::Columns(3);

			for (auto& it : models) {
				ImGui::Checkbox(it.first.c_str(), &it.second.active);
				ImGui::NextColumn();
			}

			ImGui::Columns(1);
			ImGui::TreePop();
		}
	}
	else {
		for (auto& it : models) {
			it.second.active = (bspNameObj == it.first);
		}
	}

	if (ImGui::TreeNode("Options")) {
		// Fill Mode
		ImGui::PushItemWidth(200);
		const char* fillModeStr[] = { "WIREFRAME", "SOLID" };
		_D3DFILLMODE fillModes[] = { D3DFILL_WIREFRAME, D3DFILL_SOLID };
		static int selectedFillMode = 1;
		if (ImGui::Combo("Fill Mode##D3D", &selectedFillMode, fillModeStr, IM_ARRAYSIZE(fillModeStr))) {
			d3d.SetFillMode(fillModes[selectedFillMode]);
		}
		ImGui::PopItemWidth();

		// Cull Mode
		ImGui::PushItemWidth(200);
		const char* cullModeStr[] = { "CW", "CCW", "NONE" };
		D3DCULL cullModes[] = { D3DCULL_CW, D3DCULL_CCW, D3DCULL_NONE };
		static int selectedCullMode = 0;
		if (ImGui::Combo("Cull Mode##D3D", &selectedCullMode, cullModeStr, IM_ARRAYSIZE(cullModeStr))) {
			d3d.SetCullMode(cullModes[selectedCullMode]);
		}
		ImGui::PopItemWidth();

		// Material Color
		ImGui::Text("Material Color:");
		ImGui::SameLine();
		static ImVec4 materialColor{ 1,1,1,0.5f };
		if (ImGui::ColorEdit4("MaterialColor##D3D", (float*)& materialColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
			d3d.SetMaterialColor(D3DXCOLOR(materialColor.x, materialColor.y, materialColor.z, materialColor.w));
		}

		// Light Color
		ImGui::Text("Light Color:");
		ImGui::SameLine();
		static ImVec4 lightColor{ 0.5f, 0.5f, 0.5f, 0.2f };
		if (ImGui::ColorEdit4("LightColor##D3D", (float*)& lightColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
			d3d.SetLightColor(D3DXCOLOR(lightColor.x, lightColor.y, lightColor.z, lightColor.w));
		}

		// FOV Offset
		ImGui::Text("FOV Offset:");
		ImGui::SameLine();
		static float fovOffset{ 0.0f };
		if (ImGui::InputFloat("FoVOffset##D3D", &fovOffset, .001f)) {
			d3d.SetFoVOffset(fovOffset);
		}

		// Alpha Mode
		if (ImGui::Button("Toggle Alpha Mode##D3D")) {
			d3d.ToggleTransparencyMode();
		}

		// Cull Distance
		ImGui::PushItemWidth(200);
		if (ImGui::DragFloat("Cull Distance##D3D", &d3d.CullDistance(), 0.2f, 0.1f, 1000.0f)) {
			tas_options::set_value("d3d9::cullDistance", d3d.CullDistance());
		}
		ImGui::PopItemWidth();

		ImGui::TreePop();
	}

}

static int32_t tick_delete_start = 0, tick_delete_end = 0;
static int32_t tick_insert_start = 0, tick_insert_count = 0;
static int32_t new_input_multiple_count = 0;
static int32_t remove_input_multiple_count = 0;
static int32_t project_pitch_down_count = 0;
static int32_t project_yaw_down_count = 0;
static int32_t project_pitch_yaw_down_count = 0;
static int32_t lerp_end_tick = -1;
static std::string current_item;
static int fixEditorTickOffset = 1;

float lerp(float a, float b, float frac) {
	return (a * (1.0f - frac)) + (b * frac);
}

void tas_info_window::render_inputs()
{
	auto& gInputHandler = tas_input_handler::get();

	if (ImGui::BeginCombo("##levelSelect", current_item.c_str()))
	{
		for (const auto& level : gInputHandler.get_loaded_levels()) {

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

	auto input = gInputHandler.get_inputs(current_item);
	if (input == nullptr) {
		return;
	}

	if (ImGui::TreeNode("TAS Input")) {

		ImGui::SliderInt("Editor Tick Offset", &fixEditorTickOffset, -20, 20);

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

		ImGui::Columns(7);
		ImGui::SetColumnWidth(0, 60);
		ImGui::SetColumnWidth(1, 60);
		ImGui::SetColumnWidth(2, 100);
		ImGui::SetColumnWidth(3, 100);
		ImGui::SetColumnWidth(4, 200);
		ImGui::SetColumnWidth(5, 300);
		ImGui::SetColumnWidth(6, 100);
		ImGui::Text("Pause"); ImGui::NextColumn();
		ImGui::Text("Tick"); ImGui::NextColumn();
		ImGui::Text("Pitch"); ImGui::NextColumn();
		ImGui::Text("Yaw"); ImGui::NextColumn();
		ImGui::Text("KB Input"); ImGui::NextColumn();
		ImGui::Text("Mouse Input"); ImGui::NextColumn();
		ImGui::Text("RNG"); ImGui::NextColumn();
		ImGui::Columns(1);

		ImVec2 calculatedSize = ImGui::GetContentRegionAvail();
		ImVec2 inputChildSize = ImVec2(calculatedSize.x, std::clamp(calculatedSize.y, 500.0f, FLT_MAX));
		ImGui::BeginChild("Inputs##TASINPUT", inputChildSize, true, ImGuiWindowFlags_HorizontalScrollbar);

		ImGui::Columns(7);
		ImGui::SetColumnWidth(0, 52);
		ImGui::SetColumnWidth(1, 60);
		ImGui::SetColumnWidth(2, 100);
		ImGui::SetColumnWidth(3, 100);
		ImGui::SetColumnWidth(4, 200);
		ImGui::SetColumnWidth(5, 300);
		ImGui::SetColumnWidth(6, 100);

		int count = 0;
		for (auto it = input->input_buffer()->begin(); it != input->input_buffer()->end(); ++it) {
			ImGui::PushID(count);

			int styles = 0;
			if (tas_input_handler::inputTickCounter == (count + fixEditorTickOffset)) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1, 0, 0, 1));
				styles += 2;
			}

			if (ImGui::Button("->##RunToHere")) {
				auto& gEngine = halo_engine::get();
				gEngine.fast_forward_to(count);
			}
			ImGui::NextColumn();

			ImGui::Text("%d", count);
			ImGui::NextColumn();
			if (ImGui::DragFloat("##Pitch", &(it->cameraPitch), .002f)) {
				*PLAYER_PITCH_ROTATION_RADIANS = it->cameraPitch;
			}

			ImGui::SameLine();
			if (ImGui::Button("*")) {
				ImGui::OpenPopup("pitch_options");
			}
			if (ImGui::BeginPopup("pitch_options")) {
				if (ImGui::BeginMenu("Project Down")) {

					if (ImGui::BeginMenu("Pitch")) {
						ImGui::SliderInt("##project_pitch_down", &project_pitch_down_count, 1, 200);
						ImGui::SameLine();
						if (ImGui::Button("Add##project_pitch_down")) {
							for (int i = count; i < count + project_pitch_down_count; i++) {
								input->set_pitch(i, it->cameraPitch);
							}
						}
						ImGui::EndMenu();
					}

					if (ImGui::BeginMenu("Yaw")) {
						ImGui::SliderInt("##project_yaw_down", &project_yaw_down_count, 1, 200);
						ImGui::SameLine();
						if (ImGui::Button("Add##project_yaw_down")) {
							for (int i = count; i < count + project_yaw_down_count; i++) {
								input->set_yaw(i, it->cameraYaw);
							}
						}
						ImGui::EndMenu();
					}

					if (ImGui::BeginMenu("Both")) {
						ImGui::SliderInt("##project_pitch_yaw_down", &project_pitch_yaw_down_count, 1, 200);
						ImGui::SameLine();
						if (ImGui::Button("Add##project_pitch_yaw_down")) {
							for (int i = count; i < count + project_pitch_yaw_down_count; i++) {
								input->set_view_angle(i, it->cameraPitch, it->cameraYaw);
							}
						}
						ImGui::EndMenu();
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("LERP")) {
					if (lerp_end_tick == -1)
						lerp_end_tick = count;

					ImGui::InputInt("End Tick", &lerp_end_tick);

					if (ImGui::Button("Lerp##lerp_mouse_input")) {

						if (lerp_end_tick > count) {

							int totalCount = lerp_end_tick - count;

							float startPitch = it->cameraPitch;
							float startYaw = it->cameraYaw;
							float endPitch = input->input_buffer()->at(lerp_end_tick).cameraPitch;
							float endYaw = input->input_buffer()->at(lerp_end_tick).cameraYaw;

							for (int i = 1; i < totalCount; i++) {
								float frac = (float)i / (float)totalCount;
								float pitch = lerp(startPitch, endPitch, frac);
								float yaw = lerp(startYaw, endYaw, frac);
								input->set_view_angle(count + i, pitch, yaw);
							}
						}

						lerp_end_tick = -1;
					}

					ImGui::EndMenu();
				}

				ImGui::EndPopup();
			}

			ImGui::NextColumn();

			if (ImGui::DragFloat("##Yaw", &(it->cameraYaw), .002f)) {
				*PLAYER_YAW_ROTATION_RADIANS = it->cameraYaw;
			}

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

							ImGui::SliderInt("##new_input_count", &new_input_multiple_count, 1, 200);
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

							ImGui::SliderInt("##remove_input_count", &remove_input_multiple_count, 1, 200);
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
			ImGui::SameLine();

			ImGui::Text("MMB:");
			ImGui::SameLine();
			if (it->middleMouse > 0) {
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, .5f, 0, 1));
				ImGui::DragScalar("##MMB", ImGuiDataType_U8, &(it->middleMouse), 1);
				ImGui::PopStyleColor();
			}
			else {
				ImGui::DragScalar("##MMB", ImGuiDataType_U8, &(it->middleMouse), 1);
			}
			ImGui::PopItemWidth();
			ImGui::NextColumn();

			ImGui::Text("%d", it->rng);
			ImGui::NextColumn();

			if (tas_input_handler::inputTickCounter == (count + fixEditorTickOffset) && *GAME_SPEED > 0) {
				ImGui::SetScrollHereY();
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

		ImGui::TreePop();
	}

	gInputHandler.reload_playback_buffer(input);
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
		if (ImGui::BeginMenu("Options")) {
			if (ImGui::MenuItem("Save Config")) {
				tas_options::save_config();
			}
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
	ImGui::Text("Map: %s\t", MAP_STRING);
	ImGui::SameLine();
	ImGui::Text("Tick: %d\t", *SIMULATION_TICK);
	ImGui::SameLine();
	ImGui::Text("Frame: %d\t", *FRAMES_ABSOLUTE);
	ImGui::SameLine();
	ImGui::Text("Frames Since Load: %d\t", *FRAMES_SINCE_LEVEL_START);
	ImGui::SameLine();
	ImGui::PushItemWidth(200);
	ImGui::DragFloat("Game Speed", GAME_SPEED, .005f, 0, 4);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	if (ImGui::Checkbox("Lock", &lockSpeed)) {
		lockedGameSpeed = *GAME_SPEED;
	}

	if (lockSpeed) {
		*GAME_SPEED = lockedGameSpeed;
	}

	//ImGui::Checkbox("Force Simulate", &currentInput.forceSimulate);
	*ALLOW_SIMULATE = currentInput.forceSimulate ? 0 : 1;
	*ALLOW_INPUT = (*ALLOW_SIMULATE == 1 ? 0 : 1);
}

void tas_info_window::render_rng() {
	if (ImGui::CollapsingHeader("RNG")) {
		auto& gInputHandler = tas_input_handler::get();
		ImGui::PushItemWidth(100);
		ImGui::InputInt("RNG VAL", RNG, INT_MIN, INT_MAX);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::Text("RNG Calls: %d", gInputHandler.get_rng_advances_since_last_tick());

		if (ImGui::TreeNode("Graph##RNG")) {

			if (ImGui::Button("Clear RNG Graph")) {
				gInputHandler.clear_rng_histogram_data();
			}
			ImGui::SameLine();
			if (ImGui::Button("Dummy RNG Value")) {
				gInputHandler.insert_dummy_rng_histogram_value();
			}

			auto histogram_data = gInputHandler.get_rng_histogram_data();

			auto data_max = std::max_element(std::begin(histogram_data), std::end(histogram_data));

			ImGui::PushItemWidth(ImGui::GetWindowContentRegionMax().x);
			ImGui::PlotHistogram("##RNG Histogram", histogram_data.data(), histogram_data.size(), 0, NULL, 0.0f, *data_max, ImVec2(0, 200));
			ImGui::PopItemWidth();

			ImGui::TreePop();
		}
	}
}

void tas_info_window::render_other()
{
	if (ImGui::CollapsingHeader("Misc")) {
		auto& gEngine = halo_engine::get();

		if (ImGui::TreeNode("Play Sound")) {
			static char fileText[256] = "sound\\dialog\\chief\\deathviolent";
			ImGui::InputText("File Test", fileText, IM_ARRAYSIZE(fileText));

			if (ImGui::Button("Play Sound")) {
				int tagIndex = gEngine.get_tag_index_from_path(0x736E6421, fileText);
				if (tagIndex != -1) {
					int16_t* f = new int16_t[2];
					f[0] = 0;
					f[1] = 0;
					halo::function::PLAY_SOUND(tagIndex, f, 0xffffffff, 0, 0, 0, 0);
					delete[] f;
				}
			}

			ImGui::TreePop();
		}
	}

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
	render_rng();
	render_d3d();
	render_overlay();
	render_other();
	render_tas();

	//ImGui::ShowDemoWindow();

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
