#include "pch.h"
#include "tas_hooks.h"

#include "kiero.h"
#include <D3D11.h>
#include <DXGI.h>
#include <detours/detours.h>
#include <algorithm>
#include <string>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_impl_dx11.h>
#include <mutex>

static HWND                     g_hWnd = nullptr;
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static std::once_flag           g_isInitialized;
static bool g_isInitD3D = false;

typedef HRESULT (__stdcall* D3D11End_t)(ID3D11DeviceContext* ctx, ID3D11Asynchronous* async);
HRESULT __stdcall hkD3D11End(ID3D11DeviceContext* ctx, ID3D11Asynchronous* async);
D3D11End_t originalD3D11End;

typedef HRESULT (__stdcall* D3D11Present_t)(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags);
HRESULT __stdcall hkD3D11Present(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags);
D3D11Present_t originalD3D11Present;

typedef uint8_t (__fastcall* MCCGetHalo1Input_t)(int64_t, int64_t, char*);
uint8_t __fastcall hkMCCGetHalo1Input(int64_t functionAddr, int64_t unknown, char* outValue);
MCCGetHalo1Input_t originalMCCHalo1Input;

void tas_hooks::ref_halo_dlls()
{
	modH1DLL = LoadLibrary(H1DLLPATH);
	modH2DLL = LoadLibrary(H2DLLPATH);
	modH3DLL = LoadLibrary(H3DLLPATH);
	modH4DLL = LoadLibrary(H4DLLPATH);
	modReachDLL = LoadLibrary(REACHDLLPATH);
	modODSTDLL = LoadLibrary(ODSTDLLPATH);
}

void tas_hooks::deref_halo_dlls()
{
	if (modH1DLL)
		FreeLibrary(modH1DLL);
	if (modH2DLL)
		FreeLibrary(modH2DLL);
	if (modH3DLL)
		FreeLibrary(modH3DLL);
	if (modH4DLL)
		FreeLibrary(modH4DLL);
	if (modReachDLL)
		FreeLibrary(modReachDLL);
	if (modODSTDLL)
		FreeLibrary(modODSTDLL);
}

void tas_hooks::detours_error(LONG detourResult) {
	if (detourResult != NO_ERROR) {
		throw;
	}
}

void tas_hooks::hook_function(PVOID* originalFunc, PVOID replacementFunc)
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	detours_error(DetourAttach(originalFunc, replacementFunc));
	detours_error(DetourTransactionCommit());
}

void tas_hooks::unhook_function(PVOID* originalFunc, PVOID replacementFunc)
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(originalFunc, replacementFunc);
	DetourTransactionCommit();
}

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam) {
	const DWORD TITLE_SIZE = 1024;
	TCHAR windowTitle[TITLE_SIZE];

	GetWindowText(hWnd, windowTitle, TITLE_SIZE);

	std::wstring title(&windowTitle[0]);
	if (title.find(L"Halo: The Master Chief Collection") != std::string::npos) {
		g_hWnd = hWnd;
	}

	return TRUE;
}

tas_hooks::tas_hooks()
{
	ref_halo_dlls();
}

tas_hooks::~tas_hooks()
{
	deref_halo_dlls();
}

void tas_hooks::attach_all() {

	/*
		// Get hwnd for the main MCC window
		EnumWindows(EnumWindowsProc, NULL);

		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success) {
			auto methods = kiero::getMethodsTable();

			originalD3D11Present = (D3D11Present_t)methods[8];
			originalD3D11End = (D3D11End_t)methods[89];

			// Load symbols to look for other functions...
			// IDXGISwapChain::Present = 8
			// ID3D11DeviceContext::Begin = 88
			// ID3D11DeviceContext::End = 89
			//for (int i = 0; i < 200; i++) {
			//	originalD3D11Present = (D3D11Present_t)methods[i];
			//	auto e = originalD3D11Present;
			//}

			hook_function(&(PVOID&)originalD3D11Present, hkD3D11Present);
			hook_function(&(PVOID&)originalD3D11End, hkD3D11End);
		}
	*/

	int64_t** Halo1FuncAddr = (int64_t**)0x18224B620;
	auto mccvtableptr = (MCCGetHalo1Input_t*)(**Halo1FuncAddr + 0x100);
	originalMCCHalo1Input = (MCCGetHalo1Input_t)(*mccvtableptr);

	hook_function(&(PVOID&)originalMCCHalo1Input, hkMCCGetHalo1Input);

}

void tas_hooks::detach_all() {
	//unhook_function(&(PVOID&)originalD3D11Present, hkD3D11Present);
	//unhook_function(&(PVOID&)originalD3D11End, hkD3D11End);

	unhook_function(&(PVOID&)originalMCCHalo1Input, hkMCCGetHalo1Input);
}

void CustomPresent(ID3D11Device* device, ID3D11DeviceContext* ctx, IDXGISwapChain* swapChain) {

	if (g_isInitD3D) {
		// Draw Commands
		ImGui_ImplDX11_NewFrame();
		
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}
}

HRESULT __stdcall hkD3D11End(ID3D11DeviceContext* ctx, ID3D11Asynchronous* async) {
	CustomPresent(g_pd3dDevice, g_pd3dContext, nullptr);
	return originalD3D11End(ctx, async);
}

HRESULT __stdcall hkD3D11Present(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags) {

	std::call_once(g_isInitialized, [&]() {
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::StyleColorsDark();

		SwapChain->GetDevice(__uuidof(g_pd3dDevice), reinterpret_cast<void**>(&g_pd3dDevice));
		g_pd3dDevice->GetImmediateContext(&g_pd3dContext);

		ImGui_ImplWin32_Init(g_hWnd);
		ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dContext);
		g_isInitD3D = true;
	});

	return originalD3D11Present(SwapChain, SyncInterval, Flags);
}

uint8_t __fastcall hkMCCGetHalo1Input(int64_t functionAddr, int64_t unknown, char* vkeyTable) {

	// Pre-Input
	auto MCCReturn = originalMCCHalo1Input(functionAddr, unknown, vkeyTable);
	// Post-Input
	
	// This is where we set our own inputs
	
	static bool flipflop = false;
	flipflop = !flipflop;
	vkeyTable[4 + VK_TAB] = flipflop ? 1 : 0;

	return MCCReturn;
}
