// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <GLFW/glfw3.h>

extern bool* ADDR_SIMULATE = reinterpret_cast<bool*>(0x00721E8C);
extern bool* ADDR_ALLOW_INPUT = reinterpret_cast<bool*>(0x006B15F8);
volatile extern int32_t* ADDR_SIMULATION_TICK = reinterpret_cast<int32_t*>(0x400002F4);
volatile extern int32_t* ADDR_FRAMES_ABSOLUTE = reinterpret_cast<int32_t*>(0x007C3100);

DWORD WINAPI Main_Thread(HMODULE hDLL) {
	if (!glfwInit())
	{
		// Initialization failed
	}
	GLFWwindow* window = glfwCreateWindow(200, 200, "dummy", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	while (*ADDR_SIMULATION_TICK <= 0) {

	}

	while (true) {
		glfwSwapBuffers(window);
		glfwPollEvents();

		Sleep(30);
		if (*ADDR_FRAMES_ABSOLUTE > 100) {
			*ADDR_SIMULATE = 0;
			*ADDR_ALLOW_INPUT = 1;
		}
	}

	glfwTerminate();
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DWORD dwThreadID;
		CreateThread(0, 0x1000, (LPTHREAD_START_ROUTINE)Main_Thread, hModule, 0, &dwThreadID);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
