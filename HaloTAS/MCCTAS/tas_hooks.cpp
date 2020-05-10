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
static HMODULE					g_hModule = nullptr;
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static std::once_flag           g_isInitialized;
static bool g_isInitD3D = false;

typedef HRESULT(__stdcall* D3D11End_t)(ID3D11DeviceContext* ctx, ID3D11Asynchronous* async);
HRESULT __stdcall hkD3D11End(ID3D11DeviceContext* ctx, ID3D11Asynchronous* async);
D3D11End_t originalD3D11End;

typedef HRESULT(__stdcall* D3D11Present_t)(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags);
HRESULT __stdcall hkD3D11Present(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags);
D3D11Present_t originalD3D11Present;

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

void tas_hooks::attach_all(HWND hwnd, HMODULE hMod) {

	g_hModule = hMod;

	EnumWindows(EnumWindowsProc, NULL);

	if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success) {
		auto methods = kiero::getMethodsTable();

		originalD3D11Present = (D3D11Present_t)methods[8];
		originalD3D11End = (D3D11End_t)methods[89];

		// Load symbols to look for other functions...
		// IDXGISwapChain::Present = 8
		// ID3D11DeviceContext::Begin = 88
		// ID3D11DeviceContext::End = 89
		/*for (int i = 0; i < 200; i++) {
			originalD3D11Present = (D3D11Present_t)methods[i];
			auto e = originalD3D11Present;
		}*/

		hook_function(&(PVOID&)originalD3D11Present, hkD3D11Present);
		hook_function(&(PVOID&)originalD3D11End, hkD3D11End);
	}
}

void tas_hooks::detach_all() {
	unhook_function(&(PVOID&)originalD3D11Present, hkD3D11Present);
	unhook_function(&(PVOID&)originalD3D11End, hkD3D11End);
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
