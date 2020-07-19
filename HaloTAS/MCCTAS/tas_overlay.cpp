#include "pch.h"
#include "tas_overlay.h"
#include "tas_console.h"

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
static bool gShowSpeedometer = false;
static bool gIsInitD3D = false;
static WNDPROC gOriginalWndProcHandler = nullptr;
static int gWndWidth = 0, gWndHeight = 0;
static tas_console gConsole;

const size_t MAX_SPEEDS = 5 * 30; // 5 seconds
static boost::circular_buffer<float> speeds(MAX_SPEEDS);

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
		if (wParam == VK_END) {
			gShowSpeedometer = !gShowSpeedometer;
			return true;
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

void speedometer_render() {
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(1080, 540));
	ImGui::Begin("Speedometer");

	std::vector<float> display(speeds.begin(), speeds.end());

	float maxValue = 0;
	for (auto& h : display) {
		if (h > maxValue) {
			maxValue = h;
		}
	}
	maxValue = std::clamp<float>(maxValue + .05f, .1f, 5.0f);

	int value_count = display.size() > INT_MAX ? INT_MAX : static_cast<int>(display.size());
	ImGui::PlotHistogram("SPEED", display.data(), value_count, 0, "OK", 0.0f, maxValue, ImVec2(1000.0f, 480.0f));

	ImGui::End();
}

void InputRender(const tas_input& /*Input*/) {
	ImGui::Begin("Inputs");

	/*int count = Input.tick_count();

	for (int i = 0; i < count; i++) {
		ImGui::Text(std::to_string(i).c_str());
		ImGui::Indent();
		{
			auto tick = Input.get_inputs_at_tick(i);
			int tickCount = tick.count();
			for (int t = 0; t < tickCount; t++) {
				auto frameInput = tick.get_input_at_frame(t);
				ImGui::Text("%d", t);
			}
		}
		ImGui::Unindent();
	}*/

	ImGui::End();
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

	void render(const tas_input& Input)
	{
		if (!gIsInitD3D) {
			return;
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		if (gShowMenu) {
			ImGui::ShowDemoWindow();
			InputRender(Input);
		}
		if (gShowConsole) {
			gConsole.render(gWndWidth);
		}
		if (gShowSpeedometer) {
			speedometer_render();
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

	void add_speed_value(float speed)
	{
		speeds.push_back(speed);
	}
}

