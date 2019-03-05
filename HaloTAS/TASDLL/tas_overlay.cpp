#include "tas_overlay.h"
#include <unordered_set>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>

tas_overlay::tas_overlay()
{
	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);

	window = glfwCreateWindow(1280, 720, "TAS Overlay", NULL, NULL);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	gl3wInit();

	textRenderer = std::unique_ptr<render_text>(new render_text);
	cubeRenderer = std::unique_ptr<render_cube>(new render_cube);

	imguiCtx = ImGui::CreateContext();
	ImGui::SetCurrentContext(imguiCtx);
	ImGui_ImplGlfwGL3_Init(window, true, NULL);
	ImGui::StyleColorsDark();

	currentInput = {};
}

tas_overlay::~tas_overlay()
{
	glfwDestroyWindow(window);
	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext(imguiCtx);
}

GLFWwindow* tas_overlay::glfw_window()
{
	return window;
}

void tas_overlay::make_context_current()
{
	glfwMakeContextCurrent(window);
}

void tas_overlay::update_position(HWND haloWindow)
{
	glfwMakeContextCurrent(window);

	RECT windowRect, clientRect;
	if (GetWindowRect(haloWindow, &windowRect) && GetClientRect(haloWindow, &clientRect)) {

		int width, height, x, y;
		glfwGetWindowSize(window, &width, &height);
		glfwGetWindowPos(window, &x, &y);

		int targetWidth, targetHeight, targetX, targetY;

		targetX = windowRect.left + 8;
		targetY = windowRect.top + 30;

		targetWidth = clientRect.right - clientRect.left;
		targetHeight = clientRect.bottom - clientRect.top;

		if (targetX != x || targetY != y) {
			glfwSetWindowPos(window, targetX, targetY);
		}
		if (targetWidth != width || targetHeight != height) {
			glfwSetWindowSize(window, targetWidth, targetHeight);
		}

		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		ImGui::SetCurrentContext(imguiCtx);
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(float(display_w), float(display_h)));
	}
}

const tas_overlay_input& tas_overlay::render_and_get_input(halo_engine& engine)
{
	ImGui::SetCurrentContext(imguiCtx);

	glfwPollEvents();
	ImGui_ImplGlfwGL3_NewFrame();

	update_position(engine.window_handle());

	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);

	ImGui::Begin("TAS Overlay", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);

	currentInput.overlayClose = ImGui::Button("Close");

	ImGui::Text("Current Map: %s\t", ADDR_MAP_STRING);
	ImGui::SameLine();
	ImGui::Text("Ticks since level start = %d\t", *ADDR_SIMULATION_TICK_2);
	ImGui::SameLine();
	ImGui::Text("Frames since level start = %d\t", *ADDR_FRAMES_SINCE_LEVEL_START);
	ImGui::SameLine();
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::Checkbox("Run Injected Code", ADDR_RUN_FRAME_BEGIN_CODE);
	ImGui::SameLine();
	ImGui::Checkbox("Record", &currentInput.record);
	ImGui::SameLine();
	currentInput.loadPlayback = ImGui::Button("Load Playback");
	{
		currentInput.loadPlayback = true;
	}
	ImGui::SameLine();
	ImGui::Checkbox("Playback", &currentInput.playback);

	if (currentInput.showPrimitives) {
		ImGui::SliderFloat("Cull Distance", &currentInput.cullDistance, 1, 250);
		ImGui::DragFloat("UI Scale", &currentInput.UIScale, .1f, 1, 20);
	}

	if (ImGui::Button("PatchMouse")) {
		engine.mouse_directinput_override_enable();
	}
	ImGui::SameLine();
	if (ImGui::Button("UnPatchMouse")) {
		engine.mouse_directinput_override_disable();
	}

	ImGui::DragFloat("Game Speed", ADDR_GAME_SPEED, .005f, 0, 4);

	ImGui::Checkbox("Force Simulate", &currentInput.forceSimulate);
	*ADDR_SIMULATE = currentInput.forceSimulate ? 0 : 1;
	*ADDR_ALLOW_INPUT = (*ADDR_SIMULATE == 1 ? 0 : 1);

	ImGui::SameLine();
	ImGui::Checkbox("Show Primitives", &currentInput.showPrimitives);

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

	// Rendering
	glViewport(0, 0, display_w, display_h);
	glClearColor(0, 0, 0, 0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float horizontalFovRadians = **ADDR_PTR_TO_CAMERA_HORIZONTAL_FIELD_OF_VIEW_IN_RADIANS;
	float verticalFov = horizontalFovRadians * (float)display_h / (float)display_w;
	verticalFov -= .03f; // Have to offset by this to get correct ratio for 16:9, need to look into this further
	glm::mat4 Projection = glm::perspectiveFov(verticalFov, (float)display_w, (float)display_h, 0.5f, currentInput.cullDistance);

	glm::vec3 playerPos = *ADDR_CAMERA_POSITION;
	glm::vec3 dir(ADDR_CAMERA_LOOK_VECTOR[0], ADDR_CAMERA_LOOK_VECTOR[1], ADDR_CAMERA_LOOK_VECTOR[2]);
	glm::vec3 lookAt = playerPos + dir;

	// Camera matrix
	glm::mat4 View = glm::lookAt(playerPos, lookAt, glm::vec3(0, 0, 1));

	if (currentInput.showPrimitives) {
		for (auto& v : snapshot.gameObjects) {

			glm::vec3 color = glm::vec3(1.0f);
			std::string name = "UNKNOWN";
			glm::vec3 modelPos = glm::vec3(v->unit_x, v->unit_y, v->unit_z);

			if (glm::distance(modelPos, playerPos) > currentInput.cullDistance)
				continue;

			if (KNOWN_TAGS.count(v->tag_id) > 0) {
				color = KNOWN_TAGS.at(v->tag_id).displayColor;
				name = KNOWN_TAGS.at(v->tag_id).displayName;
			}

			cubeRenderer->draw_cube(Projection, View, modelPos, .01f, color);

			std::stringstream ss;
			ss << name << " [" << static_cast<void*>(v) << "]";
			glm::vec3 textPos = glm::vec3(v->unit_x, v->unit_y - .05f, v->unit_z + .05f);
			textRenderer->draw_text(ss.str(), currentInput.UIScale, color, Projection, View, textPos, playerPos);
		}
	}

	ImGui::End();

	// Draw our UI
	ImGui::Render();
	ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(window);

	return currentInput;
}
