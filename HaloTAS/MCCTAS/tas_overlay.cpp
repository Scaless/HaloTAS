#include "pch.h"
#include "tas_overlay.h"
#include "tas_console.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_impl_dx11.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK imgui_wndproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void reset_wndproc_handler();

static HWND gWindow = nullptr;
static ID3D11Device* gD3DDevice = nullptr;
static ID3D11DeviceContext* gD3DContext = nullptr;
static ID3D11RenderTargetView* gRenderTargetView = nullptr;
static IDXGISwapChain* gSwapChain = nullptr;
static bool gShowMenu = false;
static bool gShowConsole = false;
static bool gIsInitD3D = false;
static WNDPROC gOriginalWndProcHandler = nullptr;
static int gWndWidth = 0, gWndHeight = 0;
static tas_console gConsole;

LRESULT CALLBACK imgui_wndproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ImGuiIO& io = ImGui::GetIO();
	POINT mPos;
	GetCursorPos(&mPos);
	ScreenToClient(hWnd, &mPos);
	ImGui::GetIO().MousePos.x = static_cast<float>(mPos.x);
	ImGui::GetIO().MousePos.y = static_cast<float>(mPos.y);

	io.MouseDrawCursor = gShowMenu;

	RECT rect;
	if (GetClientRect(hWnd, &rect)) {
		gWndWidth = rect.right - rect.left;
		gWndHeight = rect.bottom - rect.top;
	}

	static bool ignore_next_tilde = false;
	if (uMsg == WM_KEYDOWN) {
		if (gShowConsole) {
			if (wParam == VK_UP) {
				gConsole.history_cursor_up();
			}
			if (wParam == VK_DOWN) {
				gConsole.history_cursor_down();
			}
			if (wParam == VK_OEM_3) // VK_OEM_3 = TILDE
			{
				gShowConsole = false;
				gConsole.clear_buffer();
				ignore_next_tilde = true;
				return true;
			}
		}
	}
	if (uMsg == WM_KEYUP)
	{
		if (!gShowConsole) {
			if (wParam == VK_DELETE)
			{
				gShowMenu = !gShowMenu;
				return true;
			}
		}
		if (!gShowMenu) {
			if (wParam == VK_OEM_3) // VK_OEM_3 = TILDE
			{
				if (ignore_next_tilde) {
					ignore_next_tilde = false;
				}
				else {
					gShowConsole = !gShowConsole;
					return true;
				}
			}
		}
	}

	if (gShowMenu)
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
		default:
			ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
			return true;
			break;
		}
	}

	if (gShowConsole) {
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
		return true;
	}

	return CallWindowProc(gOriginalWndProcHandler, hWnd, uMsg, wParam, lParam);
}

void reset_wndproc_handler()
{
	if (gOriginalWndProcHandler) {
		SetWindowLongPtr(gWindow, GWLP_WNDPROC, (LONG_PTR)gOriginalWndProcHandler);
	}
}

namespace tas::overlay {
	void initialize(IDXGISwapChain* SwapChain)
	{
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGuiStyle& style = ImGui::GetStyle();
		style.ScaleAllSizes(1.5f);
		style.WindowRounding = 0.0f;
		style.WindowBorderSize = 0.0f;

		ImGui::StyleColorsDark();

		if (FAILED(SwapChain->GetDevice(__uuidof(ID3D11Device), (PVOID*)&gD3DDevice))) {
			tas_logger::error(L"SwapChain->GetDevice failed");
			return;
		}
		gD3DDevice->GetImmediateContext(&gD3DContext);
		gSwapChain = SwapChain;

		DXGI_SWAP_CHAIN_DESC sd;
		gSwapChain->GetDesc(&sd);
		gWindow = sd.OutputWindow;
		gOriginalWndProcHandler = (WNDPROC)SetWindowLongPtr(gWindow, GWLP_WNDPROC, (LONG_PTR)imgui_wndproc);

		if (!ImGui_ImplWin32_Init(gWindow)) {
			tas_logger::error(L"ImGui_ImplWin32_Init failed");
			return;
		}
		if (!ImGui_ImplDX11_Init(gD3DDevice, gD3DContext)) {
			tas_logger::error(L"ImGui_ImplDX11_Init failed");
			return;
		}

		ID3D11Texture2D* pBackBuffer;
		if (FAILED(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer))) {
			pBackBuffer->Release();
			tas_logger::error(L"SwapChain->GetBuffer failed");
			return;
		}
		if (FAILED(gD3DDevice->CreateRenderTargetView(pBackBuffer, NULL, &gRenderTargetView))) {
			pBackBuffer->Release();
			tas_logger::error(L"gD3DDevice->CreateRenderTargetView failed");
			return;
		}
		pBackBuffer->Release();

		gIsInitD3D = true;
	}

	void render()
	{
		if (!gIsInitD3D) {
			return;
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		if (gShowMenu) {
			ImGui::ShowDemoWindow();
		}
		if (gShowConsole) {
			gConsole.render(gWndWidth);
		}

		ImGui::EndFrame();
		ImGui::Render();
		gD3DContext->OMSetRenderTargets(1, &gRenderTargetView, NULL);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	void shutdown()
	{
		reset_wndproc_handler();
	}
}

