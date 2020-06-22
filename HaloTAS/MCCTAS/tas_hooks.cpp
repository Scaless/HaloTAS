#include "pch.h"
#include "tas_hooks.h"
#include "windows_utilities.h"

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

static HWND                     g_window = nullptr;
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dContext = nullptr;
static ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static bool g_ShowMenu = false;
static std::once_flag           g_isInitialized;
static bool g_isInitD3D = false;
static WNDPROC OriginalWndProcHandler = nullptr;

typedef HRESULT(__stdcall* D3D11End_t)(ID3D11DeviceContext* ctx, ID3D11Asynchronous* async);
HRESULT __stdcall hkD3D11End(ID3D11DeviceContext* ctx, ID3D11Asynchronous* async);
D3D11End_t originalD3D11End;

typedef HRESULT(__stdcall* D3D11Present_t)(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags);
HRESULT __stdcall hkD3D11Present(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags);
D3D11Present_t originalD3D11Present;

typedef uint8_t(__fastcall* MCCGetHalo1Input_t)(int64_t, int64_t, char*);
uint8_t __fastcall hkMCCGetHalo1Input(int64_t functionAddr, int64_t unknown, char* outValue);
MCCGetHalo1Input_t originalMCCHalo1Input;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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

void tas_hooks::apply_patch(patch& p)
{
	if (p.address == nullptr)
		return;
	if (p.applied)
		return;

	size_t patchSize = p.patch_data.size();

	// Get copy of original data
	p.original_data.resize(patchSize);
	memcpy_s(p.original_data.data(), p.original_data.capacity(), p.address, patchSize);

	// Overwrite original with patch
	patch_memory(p.address, p.patch_data.data(), patchSize);

	p.applied = true;
}

void tas_hooks::restore_patch(patch& p)
{
	if (p.applied && p.address != nullptr) {
		// Restore original data
		patch_memory(p.address, p.original_data.data(), p.original_data.size());
		p.original_data.clear();
		p.applied = false;
	}
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

//BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam) {
//	const DWORD TITLE_SIZE = 1024;
//	TCHAR windowTitle[TITLE_SIZE];
//
//	GetWindowText(hWnd, windowTitle, TITLE_SIZE);
//
//	std::wstring title(&windowTitle[0]);
//	if (title.find(L"Halo: The Master Chief Collection") != std::string::npos) {
//		g_window = hWnd;
//	}
//
//	return TRUE;
//}

tas_hooks::tas_hooks()
{
	ref_halo_dlls();
	generate_hooks();
	generate_patches();
}

tas_hooks::~tas_hooks()
{
	deref_halo_dlls();

	if (OriginalWndProcHandler) {
		SetWindowLongPtr(g_window, GWLP_WNDPROC, (LONG_PTR)OriginalWndProcHandler);
	}
}

void tas_hooks::generate_hooks()
{
	if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success) {
		// See https://github.com/Rebzzel/kiero/blob/master/METHODSTABLE.txt for available functions
		auto methods = kiero::getMethodsTable();

		originalD3D11Present = (D3D11Present_t)methods[8];
		originalD3D11End = (D3D11End_t)methods[89];
		
		mHooks.emplace_back(hook(&(PVOID&)originalD3D11Present, hkD3D11Present));
		mHooks.emplace_back(hook(&(PVOID&)originalD3D11End, hkD3D11End));
	}

	// Halo 1 Input Function
	//int64_t** Halo1FuncAddr = (int64_t**)0x18224B620;
	//auto mccvtableptr = (MCCGetHalo1Input_t*)(**Halo1FuncAddr + 0x100);
	//originalMCCHalo1Input = (MCCGetHalo1Input_t)(*mccvtableptr);
	//mHooks.emplace_back(hook(&(PVOID&)originalMCCHalo1Input, hkMCCGetHalo1Input));
}

void tas_hooks::generate_patches()
{
	patch devmode_patch;
	devmode_patch.patch_data = {0xB0, 0x01};
	devmode_patch.address = reinterpret_cast<uint8_t*>(0x18077FD2F);
	mPatches.push_back(devmode_patch);
}

void tas_hooks::attach_all() {
	for (auto& hook : mHooks) {
		hook_function(hook.original_function, hook.replaced_function);
	}
	for (auto& patch : mPatches) {
		apply_patch(patch);
	}
}

void tas_hooks::detach_all() {
	for (auto& hook : mHooks) {
		unhook_function(hook.original_function, hook.replaced_function);
	}
	for (auto& patch : mPatches) {
		restore_patch(patch);
	}
}

LRESULT CALLBACK hWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ImGuiIO& io = ImGui::GetIO();
	POINT mPos;
	GetCursorPos(&mPos);
	ScreenToClient(hWnd, &mPos);
	ImGui::GetIO().MousePos.x = mPos.x;
	ImGui::GetIO().MousePos.y = mPos.y;

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

HRESULT __stdcall hkD3D11End(ID3D11DeviceContext* ctx, ID3D11Asynchronous* async) {
	return originalD3D11End(ctx, async);
}

HRESULT __stdcall hkD3D11Present(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags) {

	std::call_once(g_isInitialized, [&]() {
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.WantCaptureMouse = true;
		ImGui::StyleColorsDark();

		HRESULT ret = SwapChain->GetDevice(__uuidof(ID3D11Device), (PVOID*)&g_pd3dDevice);
		if (SUCCEEDED(ret)) {
			g_pd3dDevice->GetImmediateContext(&g_pd3dContext);
			g_pSwapChain = SwapChain;
		}
		else {
			return originalD3D11Present(SwapChain, SyncInterval, Flags);
		}

		DXGI_SWAP_CHAIN_DESC sd;
		g_pSwapChain->GetDesc(&sd);
		g_window = sd.OutputWindow;
		OriginalWndProcHandler = (WNDPROC)SetWindowLongPtr(g_window, GWLP_WNDPROC, (LONG_PTR)hWndProc);

		ImGui_ImplWin32_Init(g_window);
		ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dContext);

		ID3D11Texture2D* pBackBuffer;
		SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
		g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView);
		pBackBuffer->Release();

		g_isInitD3D = true;
		});

	if(g_isInitD3D)
		CustomPresent(g_pd3dDevice, g_pd3dContext, nullptr);

	return originalD3D11Present(SwapChain, SyncInterval, Flags);
}

uint8_t __fastcall hkMCCGetHalo1Input(int64_t functionAddr, int64_t unknown, char* vkeyTable) {

	// Pre-Input
	auto MCCReturn = originalMCCHalo1Input(functionAddr, unknown, vkeyTable);
	// Post-Input

	// This is where we set our own inputs

	//static bool flipflop = false;
	//flipflop = !flipflop;
	//vkeyTable[4 + VK_TAB] = flipflop ? 1 : 0;

	return MCCReturn;
}
