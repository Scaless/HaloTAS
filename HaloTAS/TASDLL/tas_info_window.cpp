#include "tas_info_window.h"
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
	glfwSwapInterval(0);

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

void tas_info_window::render_overlay(halo_engine & engine)
{
	if (!ImGui::CollapsingHeader("Overlay")) {
		return;
	}

	ImGui::Checkbox("Enabled", &currentInput.overlayOptions.enabled);
	ImGui::Checkbox("Show Primitives", &currentInput.overlayOptions.showPrimitives);
	ImGui::SliderFloat("Cull Distance", &currentInput.overlayOptions.cullDistance, 1, 250);
	ImGui::DragFloat("UI Scale", &currentInput.overlayOptions.uiScale, .1f, 1, 20);

	engine_snapshot snapshot = {};
	engine.get_snapshot(snapshot);

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

void tas_info_window::render_tas(halo_engine & engine)
{
	if (!ImGui::CollapsingHeader("TAS")) {
		return;
	}

	ImGui::Checkbox("Record", &currentInput.record);
	ImGui::SameLine();
	ImGui::Checkbox("Playback", &currentInput.playback);
	ImGui::SameLine();
	currentInput.loadPlayback = ImGui::Button("Load Playback");

	if (ImGui::Button("PatchMouse")) {
		engine.mouse_directinput_override_enable();
	}
	ImGui::SameLine();
	if (ImGui::Button("UnPatchMouse")) {
		engine.mouse_directinput_override_disable();
	}
}

void tas_info_window::render_menubar(halo_engine & engine)
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
		ImGui::EndMainMenuBar();
	}
}

void tas_info_window::render(halo_engine & engine)
{
	glfwMakeContextCurrent(window);
	ImGui::SetCurrentContext(imguiCtx);
	glfwPollEvents();
	
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	render_menubar(engine);

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	ImGui::SetNextWindowSize(ImVec2(width, height - 20));
	ImGui::SetNextWindowPos(ImVec2(0, 20));

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

	ImGui::Begin("Main", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar);

	ImGui::Text("Map: %s\t", ADDR_MAP_STRING);
	ImGui::SameLine();
	ImGui::Text("Tick: %d\t", *ADDR_SIMULATION_TICK_2);
	ImGui::SameLine();
	ImGui::Text("Frame: %d\t", *ADDR_FRAMES_SINCE_LEVEL_START);
	ImGui::SameLine();
	ImGui::Text("App: %.0f ms/frame (%.0f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::DragFloat("Game Speed", ADDR_GAME_SPEED, .005f, 0, 4);

	ImGui::Checkbox("Force Simulate", &currentInput.forceSimulate);
	*ADDR_SIMULATE = currentInput.forceSimulate ? 0 : 1;
	*ADDR_ALLOW_INPUT = (*ADDR_SIMULATE == 1 ? 0 : 1);

	render_overlay(engine);
	render_tas(engine);

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

	glfwMakeContextCurrent(window);
	glfwSwapBuffers(window);

	close = glfwWindowShouldClose(window);
}

tas_info_input& tas_info_window::getInput()
{
	return currentInput;
}

bool tas_info_window::shouldClose()
{
	return close;
}
