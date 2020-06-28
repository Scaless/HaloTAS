#include "pch.h"
#include "tas_overlay.h"

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

static std::vector<std::string> command_history;
static char tas_command_line_buffer[512];

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
		if (wParam == VK_OEM_3) // VK_OEM_3 = TILDE
		{
			if (gShowConsole) {
				gShowConsole = false;
				ZeroMemory(tas_command_line_buffer, sizeof(tas_command_line_buffer));
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
			ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
			return true;
		default:
			break;
		}
	}

	if (gShowConsole) {
		switch (uMsg) {
		case WM_KEYDOWN:
		case WM_KEYUP:
		default:
			ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
			return true;
			break;
		}
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
	void render_console() {
		const float DISTANCE = 10.0f;
		
		ImGuiStyle& style = ImGui::GetStyle();
		ImGuiIO& io = ImGui::GetIO();
		
		ImVec2 console_window_pos = ImVec2(DISTANCE, io.DisplaySize.y - DISTANCE);
		ImVec2 window_pos_pivot = ImVec2(0.0f, 1.0f);
		ImGui::SetNextWindowPos(console_window_pos, ImGuiCond_Always, window_pos_pivot);
		ImGui::SetNextWindowBgAlpha(0.25f); // Transparent background
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
		if (ImGui::Begin("MCCTAS Console", nullptr, window_flags))
		{
			ImGuiInputTextFlags command_line_flags = ImGuiInputTextFlags_EnterReturnsTrue;
			ImGui::AlignTextToFramePadding();
			//ImGui::Text("MCCTAS(");
			ImGui::Text("halo(");
			ImGui::SameLine();
			ImGui::SetKeyboardFocusHere();
			float consoleWidth = std::max<float>(static_cast<float>(gWndWidth - 180), 720.0f);
			ImGui::SetNextItemWidth(consoleWidth);
			if (ImGui::InputText("", tas_command_line_buffer, sizeof(tas_command_line_buffer), command_line_flags)) {
				tas_logger::info("Executed MCCTAS Command: {}", tas_command_line_buffer);
				std::string command(tas_command_line_buffer);
				if (command.length() > 0) {
					command_history.push_back(command);
				}
				ZeroMemory(tas_command_line_buffer, sizeof(tas_command_line_buffer));
			}
		}
		ImGui::End();

		ImVec2 history_window_pos = ImVec2(DISTANCE, io.DisplaySize.y - DISTANCE - 60);
		ImGui::SetNextWindowPos(history_window_pos, ImGuiCond_Always, window_pos_pivot);
		ImGui::SetNextWindowBgAlpha(0.25f); // Transparent background
		if (ImGui::Begin("MCCTAS Console History", nullptr, window_flags)) {
			for (auto& command : command_history) {
				ImGui::Text(command.c_str());
			}
		}
		ImGui::End();
	}

	void initialize(IDXGISwapChain* SwapChain)
	{
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.WantCaptureMouse = true;
		io.Fonts->AddFontFromFileTTF("consola.ttf", 30);
		ImGuiStyle& style = ImGui::GetStyle();
		style.ScaleAllSizes(1.5f);
		style.WindowRounding = 0.0f;
		style.WindowBorderSize = 0.0f;

		ImGui::StyleColorsDark();

		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 0.0f, 1.0f, .05f));
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(255, 0, 255)));

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
			render_console();
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

