#pragma warning(disable:4996)

#define _USE_MATH_DEFINES

// #define HALO_VANILLA
#define HALO_CUSTOMED

#include "HaloConstants.h"
#include <inttypes.h>
#include <string>
#include <map>
#include <set>
#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <dwmapi.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <vector>
#include <math.h>

#pragma comment(lib, "dwmapi.lib")

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Error %d: %s\n", error, description);
	MessageBox(0, description, "", MB_OK);
}

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

struct InputMoment {
	uint8_t inputBuf[104];
	//float updown, leftright;
	int32_t mouseX, mouseY;
	uint8_t leftmouse, rightmouse;
};

union InputKey {
	uint64_t fullKey;
	struct {
		uint32_t subLevel;
		uint32_t inputCounter;
	} subKey;
};

HWND g_HWND = NULL;
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	DWORD lpdwProcessId;
	GetWindowThreadProcessId(hwnd, &lpdwProcessId);
	if (lpdwProcessId == lParam)
	{
		g_HWND = hwnd;
		return FALSE;
	}
	return TRUE;
}

void PatchMouseEnableManualInput() {
	unsigned long OldProtection, blah;
	//give that address read and write permissions and store the old permissions at oldProtection
	VirtualProtect((LPVOID)(ADDR_PATCH_DINPUT_MOUSE), 7, PAGE_EXECUTE_READWRITE, &OldProtection);

	//write the memory into the program and overwrite previous value
	//memcpy((LPVOID)ADDR_PATCH_DINPUT_MOUSE, valueToWrite, byteNum);
	std::copy_n(PATCH_DINPUT_MOUSE_BYTES, 7, ADDR_PATCH_DINPUT_MOUSE);

	//reset the permissions of the address back to oldProtection after writting memory
	VirtualProtect((LPVOID)(ADDR_PATCH_DINPUT_MOUSE), 7, OldProtection, &blah);
}

void UnPatchMouseDisableManualInput() {
	unsigned long OldProtection, blah;
	//give that address read and write permissions and store the old permissions at oldProtection
	VirtualProtect((LPVOID)(ADDR_PATCH_DINPUT_MOUSE), 7, PAGE_EXECUTE_READWRITE, &OldProtection);

	//write the memory into the program and overwrite previous value
	//memcpy((LPVOID)ADDR_PATCH_DINPUT_MOUSE, valueToWrite, byteNum);
	std::copy_n(PATCH_DINPUT_MOUSE_ORIGINAL, 7, ADDR_PATCH_DINPUT_MOUSE);

	//reset the permissions of the address back to oldProtection after writting memory
	VirtualProtect((LPVOID)(ADDR_PATCH_DINPUT_MOUSE), 7, OldProtection, &blah);
}

GameObject* GetPlayerObject(std::vector<GameObject*> objects) {
	for (auto& v : objects) {
		if (v->tag_id == 3588) {
			return v;
		}
	}
}

DWORD WINAPI Main_Thread(HMODULE hDLL)
{
	std::map<uint64_t, InputMoment> allInputs;

	EnumWindows(EnumWindowsProc, GetCurrentProcessId());
	RECT rect;

	bool isRecording = false;
	bool isPlayback = false;
	int counter = 0;
	int32_t lastInputCounter = -1;

	// Setup window
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
	GLFWwindow* window = glfwCreateWindow(800, 800, "Halo TAS Tools", NULL, NULL);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0); //0 = no vsync
	gl3wInit();

	// Setup ImGui binding
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplGlfwGL3_Init(window, true);

	// Setup style
	ImGui::StyleColorsDark();

	//float pi = 3.1415f;

	// Set up OPENGL drawing stuff
	// Our vertices. Three consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
	// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
	static const GLfloat g_vertex_buffer_data[] = {
		-1.0f,-1.0f,-1.0f, // triangle 1 : begin
		-1.0f,-1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f, // triangle 1 : end
		1.0f, 1.0f,-1.0f, // triangle 2 : begin
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f, // triangle 2 : end
		1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f
	};

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// This will identify our vertex buffer
	GLuint vertexbuffer;
	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);
	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	GLuint programID = LoadShaders("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");

	float defaultFOV = 38;
	float pistolZoomFOV = 18;
	bool showPrimitives = false;
	float cullDistance = 100;

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		counter++;

		std::vector<GameObject*> gameObjects;
		static std::map<uint32_t, bool> mp;
		
		if (showPrimitives) {
			for (int i = 0; i < TAG_ARRAY_LENGTH_BYTES / 4; i++) {
				if (ADDR_TAGS_ARRAY[i] == 0x68656164u) { // = "daeh" = "head" in little endian
					gameObjects.push_back((GameObject*)&ADDR_TAGS_ARRAY[i]);
				}
			}
		}

		std::sort(gameObjects.begin(), gameObjects.end(), GameObject_Sort);
		
		InputKey ik;
		ik.subKey.subLevel = *ADDR_CHECKPOINT_INDICATOR;
		ik.subKey.inputCounter = *ADDR_FRAMES_SINCE_LEVEL_START;

		// Move window position/size to match Halo's
		if (g_HWND) {
			if (GetWindowRect(g_HWND, &rect)) {
				glfwSetWindowPos(window, rect.left + 8, rect.top + 30);
				glfwSetWindowSize(window, rect.right - rect.left - 16, rect.bottom - rect.top - 38);
			}
		}

		if (isRecording) {
			if (*ADDR_FRAMES_SINCE_LEVEL_START != lastInputCounter) {
				lastInputCounter = *ADDR_FRAMES_SINCE_LEVEL_START;

				InputMoment im;
				for (int i = 0; i < sizeof(im.inputBuf); i++) {
					im.inputBuf[i] = ADDR_KEYBOARD_INPUT[i];
				}
				im.mouseX = *ADDR_DINPUT_MOUSEX;
				im.mouseY = *ADDR_DINPUT_MOUSEY;
				im.leftmouse = *ADDR_LEFTMOUSE;
				im.rightmouse = *ADDR_RIGHTMOUSE;

				allInputs[ik.fullKey] = im;
			}
		}

		if (isPlayback && allInputs.count(ik.fullKey) >= 1) {
			InputMoment savedIM = allInputs[ik.fullKey];
			for (int i = 0; i < sizeof(savedIM.inputBuf); i++) {
				ADDR_KEYBOARD_INPUT[i] = savedIM.inputBuf[i];
			}
			*ADDR_DINPUT_MOUSEX = savedIM.mouseX;
			*ADDR_DINPUT_MOUSEY = savedIM.mouseY;
			*ADDR_LEFTMOUSE = savedIM.leftmouse;
			*ADDR_RIGHTMOUSE = savedIM.rightmouse;
		}

		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();
		{ // ImGui Rendering
			int width, height, x, y;
			glfwGetWindowSize(window, &width, &height);
			glfwGetWindowPos(window, &x, &y);
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(float(width), float(height)));

			ImGui::Begin("Halo TAS", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

			if (ImGui::Button("Close")) {
				break;
			}

			ImGui::Text("Current Map: %s\t", ADDR_MAP_STRING);
			ImGui::SameLine();
			ImGui::Text("Frames since level start = %d\t", *ADDR_FRAMES_SINCE_LEVEL_START);
			ImGui::SameLine();
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			ImGui::Checkbox("Show Primitives", &showPrimitives);
			if (showPrimitives) {
				ImGui::SliderFloat("Cull Distance", &cullDistance, 1, 250);
			}

			if (ImGui::Button("PatchMouse")) {
				PatchMouseEnableManualInput();
			}
			ImGui::SameLine();
			if (ImGui::Button("UnPatchMouse")) {
				UnPatchMouseDisableManualInput();
			}

			static bool forceSimulate = true;
			ImGui::Checkbox("Force Simulate", &forceSimulate);
			*ADDR_SIMULATE = forceSimulate ? 0 : 1;
			*ADDR_ALLOW_INPUT = (*ADDR_SIMULATE == 1 ? 0 : 1);

			if (ImGui::Button("Clear Recording")) {
				isRecording = false;
				allInputs.clear();
			}
			ImGui::SameLine();
			ImGui::Checkbox("Record", &isRecording);
			ImGui::SameLine();
			ImGui::Checkbox("Play", &isPlayback);

			for (auto& v : gameObjects) {

				auto val = v->tag_id;

				if (!mp.count(val)) {
					mp[val] = false;
				}
			}
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
							ImGui::PushItemWidth(300);
							ImGui::DragFloat3(("POS: " + std::to_string(count)).c_str(), &(v->unit_x), .1f, -10000, 10000);
							//ImGui::DragFloat3(("ROT: " + std::to_string(count)).c_str(), &(v->unit_x), .1f, -10000, 10000);
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
							ImGui::SameLine();
							ImGui::Text("Addr: %p", v);
							count++;
							ImGui::PopID();
						}
					}
				}
				ImGui::PopStyleColor();
				countf++;
				ImGui::PopID();
			}

			if (ImGui::CollapsingHeader("Manual Input"))
			{
				ImGui::Text("Camera Position: (%f,%f,%f)", ADDR_CAMERA_POSITION[0], ADDR_CAMERA_POSITION[1], ADDR_CAMERA_POSITION[2]);
				ImGui::SameLine();
				ImGui::Text("Look Direction: (%f,%f,%f)", ADDR_CAMERA_LOOK_VECTOR[0], ADDR_CAMERA_LOOK_VECTOR[1], ADDR_CAMERA_LOOK_VECTOR[2]);

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
				ImGui::SliderFloat("Horizontal View Angle", ADDR_LEFTRIGHTVIEW, 0, M_PI*2.0f);

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
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0,0,0,0);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// DRAW OUR GAME OVERLAY
		int width, height;
		glfwGetWindowSize(window, &width, &height);

		float currentGameFOV_rad = **ADDR_PTR_TO_CAMERA_HORIZONTAL_FIELD_OF_VIEW_IN_RADIANS;
		float horizontalFOV = glm::degrees(currentGameFOV_rad);
		horizontalFOV = (horizontalFOV * height) / width;
		glm::mat4 Projection = glm::perspectiveFov(glm::radians(horizontalFOV), (float)width, (float)height, 0.5f, cullDistance);

		ImGui::Text("horfov: %f", horizontalFOV);

		glm::vec3 playerPos(ADDR_CAMERA_POSITION[0], ADDR_CAMERA_POSITION[1], ADDR_CAMERA_POSITION[2]);
		glm::vec3 dir(ADDR_CAMERA_LOOK_VECTOR[0], ADDR_CAMERA_LOOK_VECTOR[1], ADDR_CAMERA_LOOK_VECTOR[2]);
		glm::vec3 lookAt = playerPos + dir;
		ImGui::DragFloat3("campos", glm::value_ptr(playerPos));
		ImGui::DragFloat3("dir", glm::value_ptr(dir));
		ImGui::DragFloat3("lookAt", glm::value_ptr(lookAt));

		// Camera matrix
		glm::mat4 View = glm::lookAt(
			playerPos, 
			lookAt,
			glm::vec3(0, 0, 1)
		);

		//// TEST TRIANGLE DRAWING
		glUseProgram(programID);
		// 1st attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// Get a handle for our "MVP" uniform
		// Only during the initialisation
		GLuint MatrixID = glGetUniformLocation(programID, "MVP");
		GLuint ColorID = glGetUniformLocation(programID, "fixedColor");

		glm::mat4 model, mvp;
		glm::mat4 identity = glm::mat4(1.0f);
		glm::vec3 color;

		for (auto& v : gameObjects) {

			glm::vec3 color;

			if (mp[v->tag_id] == true) {
				color.b = 1.0f;
			}
			else {
				if (KNOWN_TAGS.count(v->tag_id) > 0) {
					color = KNOWN_TAGS.at(v->tag_id).displayColor;
				}
				else {
					color.r = 1;
				}
			}

			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(v->unit_x, v->unit_y, v->unit_z));

			model = glm::scale(model, glm::vec3(.025f, .025f, .025f));

			// Our ModelViewProjection : multiplication of our 3 matrices
			mvp = Projection * View * model; // Remember, matrix multiplication is the other way around

											 // Send our transformation to the currently bound shader, in the "MVP" uniform
											 // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
			glUniform3f(ColorID, color.x, color.y, color.z);

			// Draw the triangle !
			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDrawArrays(GL_TRIANGLES, 0, 12 * 3); // 12*3 indices starting at 0 -> 12 triangles -> 6 squares
												   //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		glDisableVertexAttribArray(0);

		ImGui::End();

		// Draw our UI
		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
		
		glfwSwapBuffers(window);
	}
	
	// Cleanup
	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	FreeLibraryAndExitThread(hDLL, NULL);

	return S_OK;
}

int main() {
	//Main_Thread(NULL);
}

INT APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID /*Reserved*/)
{
	DWORD dwThreadID;

	if (Reason == DLL_PROCESS_ATTACH) {
		CreateThread(0, 0x1000, (LPTHREAD_START_ROUTINE)Main_Thread, hDLL, 0, &dwThreadID);
	}

	return TRUE;
}
