// ImGui - standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)
// (GL3W is a helper library to access OpenGL functions since there is no standard header to access modern OpenGL functions easily. Alternatives are GLEW, Glad, etc.)

#define GLFW_EXPOSE_NATIVE_WIN32

#include <inttypes.h>
#include <string>
#include <map>
#include <set>
#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include <GL/gl3w.h>    // This example is using gl3w to access OpenGL functions (because it is small). You may use glew/glad/glLoadGen/etc. whatever already works for you.
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <dwmapi.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <vector>

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

	float* pos = (float*)0x400B8D60;
	float* campos = (float*)0x400B8DA4;
	float* dirx = (float*)0x400B8F28;
	float* diry = (float*)0x400B8F3C;
	float* look = (float*)0x400B8F34;

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

	//// One color for each vertex. They were generated randomly.
	//static const GLfloat g_color_buffer_data[] = {
	//	0.583f,  0.771f,  0.014f,
	//	0.609f,  0.115f,  0.436f,
	//	0.327f,  0.483f,  0.844f,
	//	0.822f,  0.569f,  0.201f,
	//	0.435f,  0.602f,  0.223f,
	//	0.310f,  0.747f,  0.185f,
	//	0.597f,  0.770f,  0.761f,
	//	0.559f,  0.436f,  0.730f,
	//	0.359f,  0.583f,  0.152f,
	//	0.483f,  0.596f,  0.789f,
	//	0.559f,  0.861f,  0.639f,
	//	0.195f,  0.548f,  0.859f,
	//	0.014f,  0.184f,  0.576f,
	//	0.771f,  0.328f,  0.970f,
	//	0.406f,  0.615f,  0.116f,
	//	0.676f,  0.977f,  0.133f,
	//	0.971f,  0.572f,  0.833f,
	//	0.140f,  0.616f,  0.489f,
	//	0.997f,  0.513f,  0.064f,
	//	0.945f,  0.719f,  0.592f,
	//	0.543f,  0.021f,  0.978f,
	//	0.279f,  0.317f,  0.505f,
	//	0.167f,  0.620f,  0.077f,
	//	0.347f,  0.857f,  0.137f,
	//	0.055f,  0.953f,  0.042f,
	//	0.714f,  0.505f,  0.345f,
	//	0.783f,  0.290f,  0.734f,
	//	0.722f,  0.645f,  0.174f,
	//	0.302f,  0.455f,  0.848f,
	//	0.225f,  0.587f,  0.040f,
	//	0.517f,  0.713f,  0.338f,
	//	0.053f,  0.959f,  0.120f,
	//	0.393f,  0.621f,  0.362f,
	//	0.673f,  0.211f,  0.457f,
	//	0.820f,  0.883f,  0.371f,
	//	0.982f,  0.099f,  0.879f
	//};

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

	//GLuint colorbuffer;
	//glGenBuffers(1, &colorbuffer);
	//glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

	GLuint programID = LoadShaders("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");

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
			ImGui::Text("Frames since level start = %d\t", *ADDR_FRAMES_SINCE_LEVEL_START);
			ImGui::SameLine();
			ImGui::Text("Player Position: (%f,%f,%f)", pos[0], pos[1], pos[2]);
			ImGui::SameLine();
			ImGui::Text("Shoot Position?: (%f,%f,%f)", campos[0], campos[1], campos[2]);
			ImGui::SameLine();
			ImGui::Text("Look Direction: (%f,%f,%f)", look[0], look[1], look[2]);

			if (ImGui::Button("Close")) {
				break;
			}

			//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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
			
			int count = 0;
			//std::set<std::string> types;
			for (int i = 0x40000000; i < 0x41B40000; i += 4) {
				uint32_t* b = (uint32_t*)i;
				uint32_t* c = (uint32_t*)(i + 4);
				if (*b == 0x68656164u) {
					count++;
					//types.insert(std::to_string(*b));
					//ImGui::Text("t: %08x", *c);
				}
			}
			ImGui::Text("Count: %d", count);

			if (ImGui::CollapsingHeader("Manual Input"))
			{
				ImGui::InputFloat3("Position", pos);

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

			if (ImGui::CollapsingHeader("Current Input"))
			{
				ImGui::PushItemWidth(200);
				
				/*int tempLeftMouse = *ADDR_LEFTMOUSE;
				if (tempLeftMouse > 0) {
					ImGui::SliderInt("LeftMouse", &tempLeftMouse, 0, 255);
				}

				int tempRightMouse = *ADDR_RIGHTMOUSE;
				if (tempRightMouse > 0) {
					ImGui::SliderInt("RightMouse", &tempRightMouse, 0, 255);
				}

				ImGui::SliderFloat("Vertical View Angle", ADDR_UPDOWNVIEW, -1.492f, 1.492f);
				ImGui::SliderFloat("Horizontal View Angle", ADDR_LEFTRIGHTVIEW, 0, pi*2.0f);*/

				for (int n = 0; n < 104; n++)
				{
					int tempInputVal = (int)ADDR_KEYBOARD_INPUT[n];
					if (tempInputVal > 0) {
						ImGui::SliderInt(KEY_PRINT_CODES[n].c_str(), &tempInputVal, 0, 255);
					}
				}

				ImGui::PopItemWidth();
			}

			//if (ImGui::CollapsingHeader("Recorded Map"))
			//{
			//	//ImGui::Columns(2, "recordMap", true);
			//	//ImGui::Separator();
			//	int reverseInputCount = 0;
			//	for (auto iter = allInputs.rbegin(); iter != allInputs.rend(); ++iter) {
			//		reverseInputCount++;
			//		uint64_t id = iter->first;
			//		InputMoment im = iter->second;
			//		for (int n = 0; n < 104; n++)
			//		{
			//			int tempInputVal = (int)im.inputBuf[n];
			//			if (tempInputVal > 0) {
			//				ImGui::Text("%" PRIu64, id);
			//				ImGui::NextColumn();
			//				ImGui::PushID(KEY_PRINT_CODES[n].c_str());
			//				ImGui::SliderInt(KEY_PRINT_CODES[n].c_str(), &tempInputVal, 0, 255);
			//				ImGui::PopID();
			//				ImGui::NextColumn();
			//			}
			//		}
			//		break;
			//	}
			//}
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
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// DRAW OUR GAME OVERLAY
		int width, height, x, y;
		glfwGetWindowSize(window, &width, &height);
		glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.5f, 1000.0f);

		glm::vec3 playerPos(pos[0], pos[1], pos[2]);
		glm::vec3 dir(look[0], look[1], look[2]);
		glm::vec3 lookAt = playerPos + dir;

		// Camera matrix
		glm::mat4 View = glm::lookAt(
			playerPos, // Camera is at (4,3,3), in World Space
			lookAt, // and looks at the origin
			glm::vec3(0, 0, 1)  // Head is up (set to 0,-1,0 to look upside-down)
		);

		for (int i = 0x40000000; i < 0x41B40000; i += 4) {
			uint32_t* b = (uint32_t*)i;
			uint32_t* c = (uint32_t*)(i + 4);

			if (*b == 0x68656164u) {
				float* poss = (float*)(i + (29 * 4));

				glm::vec3 color;

				switch (*c) {
				case 0x1130u: // Elite
					color.b = .8f;
					break;
				case 0xE78u: // Marine
					color.g = .8f;
					break;
				case 0xD1Cu: // ?
					color.r = 1;
					break;
				default:
					color.r = .1f;
					color.g = .1f;
					color.b = .1f;
					break;
				}

				glm::mat4 Model = glm::mat4(1.0f);
				Model = glm::translate(Model, glm::vec3(poss[0], poss[1], poss[2] - .5f));
				Model = glm::scale(Model, glm::vec3(.05f, .05f, .05f));

				// Our ModelViewProjection : multiplication of our 3 matrices
				glm::mat4 mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around

														   // Get a handle for our "MVP" uniform
														   // Only during the initialisation
				GLuint MatrixID = glGetUniformLocation(programID, "MVP");
				GLuint ColorID = glGetUniformLocation(programID, "fixedColor");

				// Send our transformation to the currently bound shader, in the "MVP" uniform
				// This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
				glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
				glUniform3f(ColorID, color.x, color.y, color.z);

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

				// Draw the triangle !
				glDrawArrays(GL_TRIANGLES, 0, 12 * 3); // 12*3 indices starting at 0 -> 12 triangles -> 6 squares
				glDisableVertexAttribArray(0);
			}
		}
		//


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
