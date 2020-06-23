#include "pch.h"
#include "tas_hooks.h"
#include "windows_utilities.h"
#include "patch.h"
#include "hook.h"

#include <winternl.h>
#include "kiero.h"
#include <D3D11.h>
#include <string>
#include <mutex>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_impl_dx11.h>

static HWND g_window = nullptr;
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dContext = nullptr;
static ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static bool g_ShowMenu = false;
static std::once_flag g_isInitialized;
static bool g_isInitD3D = false;
static WNDPROC OriginalWndProcHandler = nullptr;
static std::vector<hook> gGlobalHooks;
static std::vector<hook> gGameHooks;
static std::vector<patch> gGamePatches;

// TODO-SCALES: Refactor this stuff out into respective classes
void attach_game_hooks();
void detach_game_hooks();
void attach_global_hooks();
void detach_global_hooks();
void initialize_d3d_imgui(IDXGISwapChain* SwapChain);
///////////////////////////////////////////////////////////////

typedef HMODULE(*LoadLibraryA_t)(LPCSTR lpLibFileName);
HMODULE hkLoadLibraryA(LPCSTR lpLibFileName);
LoadLibraryA_t originalLoadLibraryA;

typedef HMODULE(*LoadLibraryW_t)(LPCWSTR lpLibFileName);
HMODULE hkLoadLibraryW(LPCWSTR lpLibFileName);
LoadLibraryW_t originalLoadLibraryW;

typedef HMODULE(*LoadLibraryExA_t)(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
HMODULE hkLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
LoadLibraryExA_t originalLoadLibraryExA;

typedef HMODULE(*LoadLibraryExW_t)(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
HMODULE hkLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
LoadLibraryExW_t originalLoadLibraryExW;

typedef BOOL(*FreeLibrary_t)(HMODULE hLibModule);
BOOL hkFreeLibrary(HMODULE hLibModule);
FreeLibrary_t originalFreeLibrary;

typedef HRESULT(*D3D11Present_t)(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags);
HRESULT hkD3D11Present(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags);
D3D11Present_t originalD3D11Present;

typedef uint8_t(*MCCGetHalo1Input_t)(int64_t, int64_t, char*);
uint8_t hkMCCGetHalo1Input(int64_t functionAddr, int64_t unknown, char* outValue);
MCCGetHalo1Input_t originalMCCHalo1Input;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

tas_hooks::tas_hooks()
{
	gGamePatches.emplace_back(L"halo1.dll", 0x077FD2F, std::vector<uint8_t>{ 0xB0, 0x01 });
}
tas_hooks::~tas_hooks()
{
	if (OriginalWndProcHandler) {
		SetWindowLongPtr(g_window, GWLP_WNDPROC, (LONG_PTR)OriginalWndProcHandler);
	}
}

void attach_global_hooks() {
	// D3D11 Hooking
	if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success) {
		// See https://github.com/Rebzzel/kiero/blob/master/METHODSTABLE.txt for available functions
		auto methods = kiero::getMethodsTable();

		originalD3D11Present = (D3D11Present_t)methods[8];
		gGlobalHooks.emplace_back(&(PVOID&)originalD3D11Present, hkD3D11Present);
	}

	// Windows API Hooking
	originalLoadLibraryA = LoadLibraryA;
	gGlobalHooks.emplace_back(&(PVOID&)originalLoadLibraryA, hkLoadLibraryA);
	originalLoadLibraryW = LoadLibraryW;
	gGlobalHooks.emplace_back(&(PVOID&)originalLoadLibraryW, hkLoadLibraryW);
	originalLoadLibraryExA = LoadLibraryExA;
	gGlobalHooks.emplace_back(&(PVOID&)originalLoadLibraryExA, hkLoadLibraryExA);
	originalLoadLibraryExW = LoadLibraryExW;
	gGlobalHooks.emplace_back(&(PVOID&)originalLoadLibraryExW, hkLoadLibraryExW);
	originalFreeLibrary = FreeLibrary;
	gGlobalHooks.emplace_back(&(PVOID&)originalFreeLibrary, hkFreeLibrary);

	// Attach the hooks
	for (auto& hk : gGlobalHooks) {
		hk.attach();
	}
}

void detach_global_hooks() {
	for (auto& hk : gGlobalHooks) {
		hk.detach();
	}
}

void attach_game_hooks() {
	for (auto& hook : gGameHooks) {
		hook.attach();
	}
	for (auto& patch : gGamePatches) {
		patch.apply();
	}
}

void detach_game_hooks() {
	for (auto& hook : gGameHooks) {
		hook.detach();
	}
	for (auto& patch : gGamePatches) {
		patch.restore();
	}
}

void tas_hooks::attach_all() {
	attach_global_hooks();
	attach_game_hooks();
}

void tas_hooks::detach_all() {
	detach_game_hooks();
	detach_global_hooks();
}

LRESULT CALLBACK hWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ImGuiIO& io = ImGui::GetIO();
	POINT mPos;
	GetCursorPos(&mPos);
	ScreenToClient(hWnd, &mPos);
	ImGui::GetIO().MousePos.x = static_cast<float>(mPos.x);
	ImGui::GetIO().MousePos.y = static_cast<float>(mPos.y);

	io.MouseDrawCursor = g_ShowMenu;

	if (uMsg == WM_KEYUP)
	{
		if (wParam == VK_DELETE)
		{
			g_ShowMenu = !g_ShowMenu;
		}
	}

	if (g_ShowMenu)
	{
		switch (uMsg) {
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_INPUT:
			ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
			return true;
			break;
		default:
			break;
		}
	}

	return CallWindowProc(OriginalWndProcHandler, hWnd, uMsg, wParam, lParam);
}

void CustomPresent(ID3D11Device* device, ID3D11DeviceContext* ctx, IDXGISwapChain* swapChain) {
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (g_ShowMenu) {
		ImGui::ShowDemoWindow();
	}

	ImGui::EndFrame();
	ImGui::Render();
	ctx->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

HMODULE hkLoadLibraryA(LPCSTR lpLibFileName) {
	auto result = originalLoadLibraryA(lpLibFileName);

	int wchars_num = MultiByteToWideChar(CP_UTF8, 0, lpLibFileName, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[wchars_num];
	MultiByteToWideChar(CP_UTF8, 0, lpLibFileName, -1, wstr, wchars_num);

	tas_logger::log(L"LoadLibraryA: %s\r\n", wstr);

	return result;
}

HMODULE hkLoadLibraryW(LPCWSTR lpLibFileName) {
	auto result = originalLoadLibraryW(lpLibFileName);

	tas_logger::log(L"LoadLibraryW: %s\r\n", lpLibFileName);

	return result;
}

HMODULE hkLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
	auto result = originalLoadLibraryExA(lpLibFileName, hFile, dwFlags);

	int wchars_num = MultiByteToWideChar(CP_UTF8, 0, lpLibFileName, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[wchars_num];
	MultiByteToWideChar(CP_UTF8, 0, lpLibFileName, -1, wstr, wchars_num);

	tas_logger::log(L"LoadLibraryExA: %s\r\n", wstr);

	return result;
}

HMODULE hkLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
	auto result = originalLoadLibraryExW(lpLibFileName, hFile, dwFlags);

	std::wstring libName(lpLibFileName);
	tas_logger::log(L"LoadLibraryExW: %s\r\n", lpLibFileName);

	// TODO-SCALES: apply to all loadlibrary calls
	if (libName == L"halo1.dll") {
		tas_logger::log(L"Installing Hooks & Patches For %s\r\n", libName.c_str());
		attach_game_hooks();
	}

	return result;
}

BOOL hkFreeLibrary(HMODULE hLibModule) {
	wchar_t buffer[MAX_PATH];
	GetModuleFileName(hLibModule, buffer, sizeof(buffer) / sizeof(TCHAR));
	tas_logger::log(L"FreeLibrary: %s\r\n", buffer);

	return originalFreeLibrary(hLibModule);
}

void initialize_d3d_imgui(IDXGISwapChain* SwapChain) {
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.WantCaptureMouse = true;
	ImGui::StyleColorsDark();

	if (FAILED(SwapChain->GetDevice(__uuidof(ID3D11Device), (PVOID*)&g_pd3dDevice))) {
		return;
	}
	g_pd3dDevice->GetImmediateContext(&g_pd3dContext);
	g_pSwapChain = SwapChain;

	DXGI_SWAP_CHAIN_DESC sd;
	g_pSwapChain->GetDesc(&sd);
	g_window = sd.OutputWindow;
	OriginalWndProcHandler = (WNDPROC)SetWindowLongPtr(g_window, GWLP_WNDPROC, (LONG_PTR)hWndProc);

	if (!ImGui_ImplWin32_Init(g_window)) {
		return;
	}
	if (!ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dContext)) {
		return;
	}

	ID3D11Texture2D* pBackBuffer;
	if (FAILED(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer))) {
		pBackBuffer->Release();
		return;
	}
	if (FAILED(g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView))) {
		pBackBuffer->Release();
		return;
	}
	pBackBuffer->Release();

	g_isInitD3D = true;
}

HRESULT hkD3D11Present(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags) {
	std::call_once(g_isInitialized, initialize_d3d_imgui, SwapChain);

	if (g_isInitD3D)
		CustomPresent(g_pd3dDevice, g_pd3dContext, nullptr);

	return originalD3D11Present(SwapChain, SyncInterval, Flags);
}

uint8_t hkMCCGetHalo1Input(int64_t functionAddr, int64_t unknown, char* vkeyTable) {

	// Pre-Input
	auto MCCReturn = originalMCCHalo1Input(functionAddr, unknown, vkeyTable);
	// Post-Input

	// This is where we set our own inputs

	//static bool flipflop = false;
	//flipflop = !flipflop;
	//vkeyTable[4 + VK_TAB] = flipflop ? 1 : 0;

	return MCCReturn;
}
