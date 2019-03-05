
#include <string>
#include <map>
#include <set>
#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <dwmapi.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <vector>
#include <boost/math/constants/constants.hpp>
#include <atomic>

#include "halo_engine.h"
#include "halo_constants.h"
#include "gameobject.h"
#include "render_text.h"
#include "render_cube.h"
#include "render_opengl.h"
#include "tas_overlay.h"

struct InputMoment {
	uint32_t checkpointId;
	uint32_t tick;
	uint8_t inputBuf[104];
	int32_t inputMouseX, inputMouseY;
	float cameraYaw, cameraPitch;
	glm::vec3 cameraLocation;
	uint8_t leftMouse,middleMouse,rightMouse;
};

union InputKey {
	uint64_t fullKey;
	struct {
		uint32_t subLevel;
		uint32_t inputCounter;
	} subKey;
};

void patch_frame_begin_func(halo_engine& engine) {
	engine.patch_program_memory(ADDR_PATCH_FRAME_BEGIN_JUMP_FUNC, PATCH_FRAME_BEGIN_FUNC_BYTES, 15);
}

static bool record = false;
static bool playback = false;
static bool tickStall = false;
static int32_t last_input_tick;
static uint32_t last_frame;

static std::map<uint64_t, InputMoment> stored_inputs_map;
static float drift = 0;

extern "C" __declspec(dllexport) void WINAPI CustomFrameStart() {

	const uint32_t frames = *ADDR_FRAMES_SINCE_LEVEL_START;
	const int32_t tick = *ADDR_SIMULATION_TICK;
	const uint32_t subLevel = *ADDR_CHECKPOINT_INDICATOR;

	if (playback) {
		*ADDR_DINPUT_MOUSEX = 0;
		*ADDR_DINPUT_MOUSEY = 0;
	}

	if (tick == last_input_tick)
		return;
	
	if (tickStall) {
		*ADDR_SIMULATION_TICK = last_input_tick;
		*ADDR_SIMULATION_TICK_2 = last_input_tick;
		return;
	}

	last_input_tick = tick;

	if (record)
	{
		std::string currentMap{ ADDR_MAP_STRING };
		std::replace(currentMap.begin(), currentMap.end(), '\\', '.');
		std::ofstream logFile(currentMap + ".hbin", std::ios::app | std::ios::binary); // levels.a10.a10.hbin

		InputMoment im;
		for (auto i = 0; i < sizeof(im.inputBuf); i++) {
			im.inputBuf[i] = ADDR_KEYBOARD_INPUT[i];
		}
		im.checkpointId = *ADDR_CHECKPOINT_INDICATOR;
		im.tick = tick;
		//im.inputMouseX = *ADDR_DINPUT_MOUSEX;
		//im.inputMouseY = *ADDR_DINPUT_MOUSEY;
		im.cameraYaw = *ADDR_PLAYER_YAW_ROTATION_RADIANS;
		im.cameraPitch = *ADDR_PLAYER_PITCH_ROTATION_RADIANS;
		im.leftMouse = *ADDR_LEFTMOUSE;
		im.middleMouse = *ADDR_MIDDLEMOUSE;
		im.rightMouse = *ADDR_RIGHTMOUSE;
		im.cameraLocation = *ADDR_CAMERA_POSITION;

		logFile.write(reinterpret_cast<char*>(&im), sizeof(im));
		logFile.close();
	}

	if(playback)
	{
		InputKey ik;
		ik.subKey.subLevel = subLevel;
		ik.subKey.inputCounter = tick;
		if(stored_inputs_map.count(ik.fullKey))
		{
			InputMoment savedIM = stored_inputs_map[ik.fullKey];

			memcpy(ADDR_KEYBOARD_INPUT, savedIM.inputBuf, sizeof(savedIM.inputBuf));
			*ADDR_PLAYER_YAW_ROTATION_RADIANS = savedIM.cameraYaw;
			*ADDR_PLAYER_PITCH_ROTATION_RADIANS = savedIM.cameraPitch;
			//*ADDR_DINPUT_MOUSEX = savedIM.inputMouseX;
			//*ADDR_DINPUT_MOUSEY = savedIM.inputMouseY;
			*ADDR_LEFTMOUSE = savedIM.leftMouse;
			*ADDR_MIDDLEMOUSE = savedIM.middleMouse;
			*ADDR_RIGHTMOUSE = savedIM.rightMouse;
			//*ADDR_CAMERA_POSITION = savedIM.cameraLocation;

			drift = glm::distance(*ADDR_CAMERA_POSITION,savedIM.cameraLocation);
		}
	}
}

void load_inputs()
{
	stored_inputs_map.clear();
	
	std::string currentMap{ ADDR_MAP_STRING };
	std::replace(currentMap.begin(), currentMap.end(), '\\', '.');

	std::ifstream logFile(currentMap + ".hbin", std::ios::app | std::ios::binary);
	
	char buf[sizeof(InputMoment)];
	while(!logFile.eof())
	{
		logFile.read(buf, sizeof(InputMoment));
		InputMoment* im = reinterpret_cast<InputMoment*>(&buf);

		InputKey ik;
		ik.subKey.subLevel = im->checkpointId;
		ik.subKey.inputCounter = im->tick;
		stored_inputs_map[ik.fullKey] = *im;
	}
}

DWORD WINAPI Main_Thread(HMODULE hDLL)
{
	std::unique_ptr<halo_engine> engine(new halo_engine);

	auto dllHandle = GetModuleHandle(TEXT("TASDLL"));
	auto addr = reinterpret_cast<int>(GetProcAddress(dllHandle, "_CustomFrameStart@0"));
	PATCH_FRAME_BEGIN_FUNC_BYTES[0] = 0xE8; // x86 CALL
	addr -= (int)ADDR_FRAME_BEGIN_FUNC_OFFSET; // Call location
	memcpy_s(&PATCH_FRAME_BEGIN_FUNC_BYTES[1], sizeof(addr), &addr, sizeof(addr));
	patch_frame_begin_func(*engine);

	LPVOID LivesplitMemPoolAddr = reinterpret_cast<LPVOID>(0x44000000);
	SIZE_T LivesplitMemPoolSize = 1024;
	void* LivesplitMemPool = VirtualAlloc(LivesplitMemPoolAddr, LivesplitMemPoolSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	// Setup GLFW
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;

	std::unique_ptr<tas_overlay> overlay(new tas_overlay);

	gl3wInit();

	// Setup ImGui binding
	ImGui::CreateContext();
	ImGui_ImplGlfwGL3_Init(overlay->glfw_window(), true, NULL);
	ImGui::StyleColorsDark();

	bool forceSimulate = true;
	bool showPrimitives = false;
	float cullDistance = 15;
	float UIScale = 1;
	std::unique_ptr<render_text> textRenderer(new render_text);
	std::unique_ptr<render_cube> cubeRenderer(new render_cube);
	std::vector<GameObject*> gameObjects;
	std::map<uint32_t, bool> mp;
	std::vector<DataPool*> dataPools;
	DataPool* objectDataPool = nullptr;

	int memOffset = 0;
	// Scan memory for object pools
	for (uint32_t i = 0; i < TAG_ARRAY_LENGTH_BYTES / 4; i++) {
		if (memcmp(&MAGIC_DATAPOOLHEADER, &(ADDR_TAGS_ARRAY[i]), sizeof(MAGIC_DATAPOOLHEADER)) == 0) {
			DataPool* pool = (DataPool*)(&ADDR_TAGS_ARRAY[i] - 10);
			memcpy((char*)LivesplitMemPool + memOffset, &pool->Name, sizeof(pool->Name));
			memOffset += sizeof(pool->Name);
			dataPools.push_back(pool);

			if (std::string(pool->Name) == "object") {
				objectDataPool = pool;
			}
		}
	}

	engine->update_window_handle();

	// Main loop
	while (!glfwWindowShouldClose(overlay->glfw_window()))
	{
		overlay->make_context_current();
		gameObjects.clear();
		mp.clear();

		if (objectDataPool != nullptr)
		{
			int count = objectDataPool->ObjectCount;
			for (int i = 0; i < count; i++) {
				uint32_t* basePtr = (uint32_t*)&objectDataPool->ObjectPointers[i];

				if ((int)*basePtr == 0) {
					continue;
				}
				if ((int)*basePtr < 0x40000000 || (int)*basePtr > 0x41B40000) {
					continue;
				}

				GameObject* object = (GameObject*)(*basePtr - 6 * 4);
				gameObjects.push_back(object);
			}
		}

		// Move window position/size to match Halo's
		HWND haloWindow = engine->window_handle();
		RECT windowRect, clientRect;
		if (haloWindow) {
			if (GetWindowRect(haloWindow, &windowRect) && GetClientRect(haloWindow, &clientRect)) {

				int width, height, x, y;
				glfwGetWindowSize(overlay->glfw_window(), &width, &height);
				glfwGetWindowPos(overlay->glfw_window(), &x, &y);

				int targetWidth, targetHeight, targetX, targetY;

				targetX = windowRect.left + 8;
				targetY = windowRect.top + 30;

				targetWidth = clientRect.right - clientRect.left;
				targetHeight = clientRect.bottom - clientRect.top;

				if (targetX != x || targetY != y) {
					glfwSetWindowPos(overlay->glfw_window(), targetX, targetY);
				}
				if (targetWidth != width || targetHeight != height) {
					glfwSetWindowSize(overlay->glfw_window(), targetWidth, targetHeight);
				}
			}
		}

		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();
		{ 

			// ImGui Rendering
			int width, height, x, y;
			glfwGetWindowSize(overlay->glfw_window(), &width, &height);
			glfwGetWindowPos(overlay->glfw_window(), &x, &y);
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(float(width), float(height)));

			ImGui::Begin("Halo TAS", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);

			if (ImGui::Button("Close")) {
				break;
			}

			ImGui::Text("Current Map: %s\t", ADDR_MAP_STRING);
			ImGui::SameLine();
			ImGui::Text("Ticks since level start = %d\t", *ADDR_SIMULATION_TICK_2);
			ImGui::SameLine();
			ImGui::Text("Frames since level start = %d\t", *ADDR_FRAMES_SINCE_LEVEL_START);
			ImGui::SameLine();
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			ImGui::Checkbox("Run Injected Code", ADDR_RUN_INJECTED_CODE);
			ImGui::SameLine();
			ImGui::Checkbox("Record", &record);
			ImGui::SameLine();
			if(ImGui::Button("Load Playback"))
			{
				load_inputs();
			}
			ImGui::SameLine();
			ImGui::Checkbox("Playback", &playback);
			ImGui::SameLine();
			ImGui::Text("Drift: (%f)", drift);

			if (showPrimitives) {
				ImGui::SliderFloat("Cull Distance", &cullDistance, 1, 250);
				ImGui::DragFloat("UI Scale", &UIScale, .1f, 1, 20);
			}

			if (ImGui::Button("PatchMouse")) {
				engine->mouse_directinput_override_enable();
			}
			ImGui::SameLine();
			if (ImGui::Button("UnPatchMouse")) {
				engine->mouse_directinput_override_disable();
			}

			if (ImGui::Button("Test HUD")) {
				engine->print_hud_text(L"TE ST");
			}

			ImGui::DragFloat("Game Speed", ADDR_GAME_SPEED, .005f, 0, 4);

			ImGui::Checkbox("Force Simulate", &forceSimulate);
			*ADDR_SIMULATE = forceSimulate ? 0 : 1;
			*ADDR_ALLOW_INPUT = (*ADDR_SIMULATE == 1 ? 0 : 1);

			ImGui::SameLine();
			ImGui::Checkbox("Show Primitives", &showPrimitives);

			for (auto& v : gameObjects) {
				auto val = v->tag_id;
				if (!mp.count(val)) {
					mp[val] = false;
				}
			}

			if (ImGui::CollapsingHeader("Objects"))
			{
				auto countf = 1;
				for (auto& m : mp) {
					std::string tagName = "UNKNOWN" + std::to_string(countf);
					if (KNOWN_TAGS.count(m.first)) {
						tagName = KNOWN_TAGS.at(m.first).displayName;
					}
					ImGui::PushID(countf);
					ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
					if (ImGui::CollapsingHeader(tagName.c_str())) {
						auto count = 1;
						for (auto& v : gameObjects) {
							if (v->tag_id == m.first) {
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
									auto player = GetPlayerObject(gameObjects);

									v->unit_x = player->unit_x + 1;
									v->unit_y = player->unit_y;
									v->unit_z = player->unit_z;
								}
								ImGui::SameLine();
								if (ImGui::Button("MoveMeToThis")) {
									auto player = GetPlayerObject(gameObjects);

									player->unit_x = v->unit_x;
									player->unit_y = v->unit_y;
									player->unit_z = v->unit_z + .5f;
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
				ImGui::SliderFloat("Horizontal View Angle", ADDR_LEFTRIGHTVIEW, 0, boost::math::float_constants::pi * 2.0f);

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

		// Rendering
		int display_w, display_h;
		glfwGetFramebufferSize(overlay->glfw_window(), &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0,0,0,0);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float horizontalFovRadians = **ADDR_PTR_TO_CAMERA_HORIZONTAL_FIELD_OF_VIEW_IN_RADIANS;
		float verticalFov = horizontalFovRadians * (float)display_h / (float)display_w;
		verticalFov -= .03f; // Have to offset by this to get correct ratio for 16:9, need to look into this further
		glm::mat4 Projection = glm::perspectiveFov(verticalFov, (float)display_w, (float)display_h, 0.5f, cullDistance);

		glm::vec3 playerPos = *ADDR_CAMERA_POSITION;
		glm::vec3 dir(ADDR_CAMERA_LOOK_VECTOR[0], ADDR_CAMERA_LOOK_VECTOR[1], ADDR_CAMERA_LOOK_VECTOR[2]);
		glm::vec3 lookAt = playerPos + dir;

		// Camera matrix
		glm::mat4 View = glm::lookAt(playerPos, lookAt, glm::vec3(0, 0, 1));

		if (showPrimitives) {
			for (auto& v : gameObjects) {

				glm::vec3 color;
				std::string name;
				glm::vec3 modelPos = glm::vec3(v->unit_x, v->unit_y, v->unit_z);

				if (glm::distance(modelPos, playerPos) > cullDistance)
					continue;

				if (mp[v->tag_id] == true) {
					color.b = 1.0f;
					name = "UNKNOWN";
				}
				else {
					if (KNOWN_TAGS.count(v->tag_id) > 0) {
						color = KNOWN_TAGS.at(v->tag_id).displayColor;
						name = KNOWN_TAGS.at(v->tag_id).displayName;
					}
					else {
						color.r = 1;
						name = "UNKNOWN";
					}
				}

				cubeRenderer->draw_cube(Projection, View, modelPos, .01f, color);

				std::stringstream ss;
				ss << name << " [" << static_cast<void*>(v) << "]";
				glm::vec3 textPos = glm::vec3(v->unit_x, v->unit_y - .05f, v->unit_z + .05f);
				textRenderer->draw_text(ss.str(), UIScale, color, Projection, View, textPos, playerPos);
			}
		}

		ImGui::End();

		// Draw our UI
		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
		
		glfwSwapBuffers(overlay->glfw_window());
	}

	VirtualFree(LivesplitMemPool, 0, MEM_RELEASE);
	
	// Cleanup
	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();

	FreeLibraryAndExitThread(hDLL, NULL);
}

// Entry point for DLL
INT APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID /*Reserved*/)
{
	DWORD dwThreadID;

	if (Reason == DLL_PROCESS_ATTACH) {
		CreateThread(0, 0x1000, (LPTHREAD_START_ROUTINE)Main_Thread, hDLL, 0, &dwThreadID);
	}

	return TRUE;
}
