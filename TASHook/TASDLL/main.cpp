// ImGui - standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)
// (GL3W is a helper library to access OpenGL functions since there is no standard header to access modern OpenGL functions easily. Alternatives are GLEW, Glad, etc.)

#define GLFW_EXPOSE_NATIVE_WIN32

#include <inttypes.h>
#include <string>
#include <map>
#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include <GL/gl3w.h>    // This example is using gl3w to access OpenGL functions (because it is small). You may use glew/glad/glLoadGen/etc. whatever already works for you.
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Error %d: %s\n", error, description);
}

// KEYS is a layout of the keyboard keys in memory (one byte per key)
enum KEYS {
	ESC = 0,
	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,
	PrntScrn,
	ScrollLock,
	PauseBreak,
	Tilde,
	NUM_1,
	NUM_2,
	NUM_3,
	NUM_4,
	NUM_5,
	NUM_6,
	NUM_7,
	NUM_8,
	NUM_9,
	NUM_0,
	Minus,
	Equal,
	Backspace,
	Tab,
	Q,
	W,
	E,
	R,
	T,
	Y,
	U,
	I,
	O,
	P,
	LeftBracket,
	RightBracket,
	Pipe,
	CapsLock,
	A,
	S,
	D,
	F,
	G,
	H,
	J,
	K,
	L,
	Colon,
	Quote,
	Enter,
	LShift,
	Z,
	X,
	C,
	V,
	B,
	N,
	M,
	Comma,
	Period,
	ForwardSlash,
	RShift,
	LCtrl,
	LWin,
	LAlt,
	Space,
	RAlt,
	RWin,
	KeyThatLiterallyNoOneHasEverUsed,
	RightCtrl,
	Up,
	Down,
	Left,
	Right
};

std::string KEY_PRINT_CODES[] = {
	"ESC",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"F11",
	"F12",
	"PrntScrn",
	"ScrollLock",
	"PauseBreak",
	"Tilde",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"0",
	"Minus",
	"Equal",
	"Backspace",
	"Tab",
	"Q",
	"W",
	"E",
	"R",
	"T",
	"Y",
	"U",
	"I",
	"O",
	"P",
	"[",
	"]",
	"|",
	"CapsLock",
	"A",
	"S",
	"D",
	"F",
	"G",
	"H",
	"J",
	"K",
	"L",
	"Colon",
	"Quote",
	"Enter",
	"LShift",
	"Z",
	"X",
	"C",
	"V",
	"B",
	"N",
	"M",
	",",
	".",
	"/",
	"RShift",
	"LCtrl",
	"LWin",
	"LAlt",
	"Space",
	"RAlt",
	"RWin",
	"Menu",
	"RCtrl",
	"Up",
	"Down",
	"Left",
	"Right",
	"Insert",
	"Home",
	"PageUp",
	"Delete",
	"End",
	"PageDown",
	"NumLock",
	"NUM_/",
	"NUM_*",
	"NUM_0",
	"NUM_1",
	"NUM_2",
	"NUM_3",
	"NUM_4",
	"NUM_5",
	"NUM_6",
	"NUM_7",
	"NUM_8",
	"NUM_9",
	"NUM_-",
	"NUM_+",
	"NUM_ENTER",
	"NUM_.",
};

struct InputMoment {
	uint8_t inputBuf[104];
	float updown, leftright;
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

DWORD WINAPI Main_Thread(HMODULE hDLL)
{
	int32_t* ADDR_FRAMES_SINCE_LEVEL_START = (int32_t*)0x00746F88;
	int32_t* ADDR_INPUT_TICK = (int32_t*)0x006F1D8C;
	float* ADDR_LEFTRIGHTVIEW = (float*)0x402AD4B8;
	float* ADDR_UPDOWNVIEW = (float*)0x402AD4BC;
	char* ADDR_MAP_STRING = (char*)0x006A8174;
	uint32_t* ADDR_CHECKPOINT_INDICATOR = (uint32_t*)0x00746F90;
	uint8_t* ADDR_KEYBOARD_INPUT = (uint8_t*)0x006B1620;
	uint8_t* ADDR_LEFTMOUSE = (uint8_t*)0x006B1818;
	uint8_t* ADDR_MIDDLEMOUSE = (uint8_t*)0x006B1819;
	uint8_t* ADDR_RIGHTMOUSE = (uint8_t*)0x006B181A;
	bool* ADDR_SIMULATE = (bool*)0x00721E8C;
	bool* ADDR_ALLOW_INPUT = (bool*)0x006B15F8;

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
	glfwSwapInterval(0); //no vsync
	gl3wInit();

	// Setup ImGui binding
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
	ImGui_ImplGlfwGL3_Init(window, true);

	// Setup style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	//bool show_demo_window = true;
	//bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	float pi = 3.1415f;

	// Set up OPENGL drawing stuff
	static const GLfloat g_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f,  1.0f, 0.0f,
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

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		counter++;

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
				im.updown = *ADDR_UPDOWNVIEW;
				im.leftright = *ADDR_LEFTRIGHTVIEW;
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
			*ADDR_UPDOWNVIEW = savedIM.updown;
			*ADDR_LEFTRIGHTVIEW = savedIM.leftright;
			*ADDR_LEFTMOUSE = savedIM.leftmouse;
			*ADDR_RIGHTMOUSE = savedIM.rightmouse;
		}

		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		// 1. Show a simple window.
		// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets automatically appears in a window called "Debug".
		{
			int width, height, x, y;
			glfwGetWindowSize(window, &width, &height);
			glfwGetWindowPos(window, &x, &y);
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(float(width), float(height)));

			ImGui::Begin("Halo TAS", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

			ImGui::Text("Current Map: %s\t", ADDR_MAP_STRING);
			ImGui::SameLine();
			ImGui::Text("Checkpoint Indicator: %u\t", ADDR_CHECKPOINT_INDICATOR);
			ImGui::SameLine();
			ImGui::Text("Frames since level start = %d\t", *ADDR_FRAMES_SINCE_LEVEL_START);
			ImGui::SameLine();
			ImGui::Text("Input tick = %d", *ADDR_INPUT_TICK);

			if (ImGui::Button("Close")) {
				break;
			}

			//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Text("Location: %d, %d, %d, %d", rect.top, rect.left, rect.right - rect.left - 16, rect.bottom - rect.top - 38);


			if (*ADDR_SIMULATE == 1)
				*ADDR_SIMULATE = 0;
			//ImGui::Checkbox("Freeze Game", ADDR_SIMULATE);
			*ADDR_ALLOW_INPUT = (*ADDR_SIMULATE == 1 ? 0 : 1);

			if (ImGui::Button("Clear Recording")) {
				isRecording = false;
				allInputs.clear();
			}
			ImGui::SameLine();
			ImGui::Checkbox("Record", &isRecording);
			ImGui::SameLine();
			ImGui::Checkbox("Play", &isPlayback);

			if (ImGui::CollapsingHeader("Input Map"))
			{
				int tempLeftMouse = *ADDR_LEFTMOUSE;
				if (ImGui::SliderInt("LeftMouse", &tempLeftMouse, 0, 255)) {
					*ADDR_LEFTMOUSE = (uint8_t)tempLeftMouse;
				}

				int tempRightMouse = *ADDR_RIGHTMOUSE;
				if (ImGui::SliderInt("RightMouse", &tempRightMouse, 0, 255)) {
					*ADDR_RIGHTMOUSE = (uint8_t)tempRightMouse;
				}

				ImGui::SliderFloat("Vertical View Angle", ADDR_UPDOWNVIEW, -1.492f, 1.492f);
				ImGui::SliderFloat("Horizontal View Angle", ADDR_LEFTRIGHTVIEW, 0, pi*2.0f);

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

			if (ImGui::CollapsingHeader("Recorded Map"))
			{
				//ImGui::Columns(2, "recordMap", true);
				//ImGui::Separator();
				int reverseInputCount = 0;
				for (auto iter = allInputs.rbegin(); iter != allInputs.rend(); ++iter) {
					reverseInputCount++;
					uint64_t id = iter->first;
					InputMoment im = iter->second;
					for (int n = 0; n < 104; n++)
					{
						int tempInputVal = (int)im.inputBuf[n];
						if (tempInputVal > 0) {
							ImGui::Text("%" PRIu64, id);
							ImGui::NextColumn();
							ImGui::PushID(KEY_PRINT_CODES[n].c_str());
							ImGui::SliderInt(KEY_PRINT_CODES[n].c_str(), &tempInputVal, 0, 255);
							ImGui::PopID();
							ImGui::NextColumn();
						}
					}
					break;
				}
			}
			ImGui::End();
		}

		// 2. Show another simple window. In most cases you will use an explicit Begin/End pair to name your windows.
		/*if (show_another_window)
		{
		ImGui::Begin("Another Window", &show_another_window);
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
		show_another_window = false;
		ImGui::End();
		}*/

		// 3. Show the ImGui demo window. Most of the sample code is in ImGui::ShowDemoWindow(). Read its code to learn more about Dear ImGui!
		//if (show_demo_window)
		//{
		//    ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
		//    ImGui::ShowDemoWindow(&show_demo_window);
		//}

		// Rendering
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

		// TEST TRIANGLE DRAWING
		//// 1st attribute buffer : vertices
		//glEnableVertexAttribArray(0);
		//glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		//glVertexAttribPointer(
		//	0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		//	3,                  // size
		//	GL_FLOAT,           // type
		//	GL_FALSE,           // normalized?
		//	0,                  // stride
		//	(void*)0            // array buffer offset
		//);
		//// Draw the triangle !
		//glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
		//glDisableVertexAttribArray(0);

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
