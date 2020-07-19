#include "pch.h"
#include "gui_interop.h"
#include "windows_utilities.h"
#include "halo1_engine.h"
#include "halo_types.h"
#include "dll_cache.h"
#include "global.h"

enum class InteropRequestType : int32_t {
	INVALID = -1,
	PING = 0,
	GET_DLL_INFORMATION = 1,
	SET_CAMERA_DETAILS = 2,
	EXECUTE_COMMAND = 3,
	GET_GAME_INFORMATION = 4,
	SET_HALO1_SKULL_ENABLED = 5,
	KILL_MCCTAS = 6,
};
std::unordered_map<int32_t, const wchar_t*> InteropRequestTypeString{
	{to_underlying(InteropRequestType::INVALID), L"INVALID"},
	{to_underlying(InteropRequestType::PING), L"PING"},
	{to_underlying(InteropRequestType::GET_DLL_INFORMATION), L"GET_DLL_INFORMATION"},
	{to_underlying(InteropRequestType::SET_CAMERA_DETAILS), L"SET_CAMERA_DETAILS"},
	{to_underlying(InteropRequestType::EXECUTE_COMMAND), L"EXECUTE_COMMAND"},
	{to_underlying(InteropRequestType::GET_GAME_INFORMATION), L"GET_GAME_INFORMATION"},
	{to_underlying(InteropRequestType::SET_HALO1_SKULL_ENABLED), L"SET_HALO1_SKULL_ENABLED"},
	{to_underlying(InteropRequestType::KILL_MCCTAS), L"KILL_MCCTAS"},
};

enum class InteropResponseType : int32_t {
	INVALID_REQUEST = -2,
	FAILURE = -1,
	SUCCESS = 0,
};
std::unordered_map<int32_t, const wchar_t*> InteropResponseTypeString{
	{to_underlying(InteropResponseType::INVALID_REQUEST), L"INVALID_REQUEST"},
	{to_underlying(InteropResponseType::FAILURE), L"FAILURE"},
	{to_underlying(InteropResponseType::SUCCESS), L"SUCCESS"},
};

struct InteropRequestHeader {
public:
	InteropRequestType type;
	int32_t payload_size;

	InteropRequestHeader()
		: type(InteropRequestType::INVALID), payload_size(0)
	{
	}
};

struct InteropResponseHeader {
public:
	InteropResponseType type;
	int32_t payload_size;

	InteropResponseHeader()
		: type(InteropResponseType::FAILURE), payload_size(0)
	{
	}
};

struct DLLInformationRequestPayload {
public:
	WCHAR dll_name[256];
};

struct DLLInformationResponsePayload {
public:
	WCHAR dll_name[256];
	uint64_t base_address;
	uint64_t entry_point;
	uint64_t image_size;

	DLLInformationResponsePayload()
		: dll_name(L""), base_address(0), entry_point(0), image_size(0)
	{
	}
};

struct SetCameraDetailsRequestPayload {
	float positionX;
	float positionY;
	float positionZ;
};

struct Halo1SetSkullEnabledRequestPayload {
	int32_t Skull;
	BOOL Enabled;
};

struct ExecuteCommandRequestPayload {
	char command[256];
};

struct Halo1GameInformation {
	int32_t Tick;
	BOOL SkullsEnabled[to_underlying(halo1::cheat::COUNT)];
};
struct GetGameInformationResponsePayload {
	BOOL Halo1Loaded;
	BOOL Halo2Loaded;
	BOOL Halo3Loaded;
	BOOL ODSTLoaded;
	BOOL ReachLoaded;
	BOOL Halo4Loaded;

	Halo1GameInformation Halo1Information;

	GetGameInformationResponsePayload()
	{
		Halo1Loaded = FALSE;
		Halo2Loaded = FALSE;
		Halo3Loaded = FALSE;
		ODSTLoaded = FALSE;
		ReachLoaded = FALSE;
		Halo4Loaded = FALSE;
		Halo1Information = {};
	}
};

struct InteropRequest {
	InteropRequestHeader header;
	const char* payload;
};
struct InteropResponse {
	InteropResponseHeader header;
	std::vector<char> payload;
};

void handle_response_dll_information(const InteropRequest& request, InteropResponse& response)
{
	DLLInformationRequestPayload requestPayload;
	memcpy_s(&requestPayload, sizeof(requestPayload), request.payload, sizeof(requestPayload));
	std::wstring dllName(requestPayload.dll_name);

	auto dll = dll_cache::get_info(dllName);

	if (!dll.has_value()) {
		response.header.type = InteropResponseType::FAILURE;
		return;
	}

	DLLInformationResponsePayload payloadOut;

	dllName.copy(payloadOut.dll_name, dllName.size(), 0);
	payloadOut.base_address = (uint64_t)dll.value();
	// TODO-SCALES: Remove these values from propagation since they are not used
	payloadOut.entry_point = 0;
	payloadOut.image_size = 0;

	response.header.type = InteropResponseType::SUCCESS;
	response.header.payload_size = sizeof(payloadOut);

	response.payload.reserve(response.header.payload_size);
	memcpy_s(response.payload.data(), response.payload.capacity(), &payloadOut, sizeof(payloadOut));
}

void handle_response_set_camera_details(const InteropRequest& request, InteropResponse& response) {

	// TODO-SCALES - Fix the addresses for this
	response.header.type = InteropResponseType::FAILURE;
	return;

	SetCameraDetailsRequestPayload cameraDetailsPayload;
	memcpy_s(&cameraDetailsPayload, sizeof(cameraDetailsPayload), request.payload, sizeof(cameraDetailsPayload));

	auto H1DLL = dll_cache::get_info(HALO1_DLL_WSTR);
	if (!H1DLL.has_value()) {
		return;
	}

	auto H1Base = H1DLL.value();

	float* cameraPositionArr = reinterpret_cast<float*>((uint8_t*)H1Base + 0x2199338);

	cameraPositionArr[0] = cameraDetailsPayload.positionX;
	cameraPositionArr[1] = cameraDetailsPayload.positionY;
	cameraPositionArr[2] = cameraDetailsPayload.positionZ;

	response.header.type = InteropResponseType::SUCCESS;
}

void handle_response_execute_command(const InteropRequest& request, InteropResponse& response) {

	ExecuteCommandRequestPayload commandPayload;
	memcpy_s(&commandPayload, sizeof(commandPayload), request.payload, sizeof(commandPayload));

	halo1_engine::execute_command(commandPayload.command);

	tas_logger::info("Executed Halo1 Command: {}", commandPayload.command);

	response.header.type = InteropResponseType::SUCCESS;
}

void handle_response_get_game_information(const InteropRequest& /*request*/, InteropResponse& response) {

	auto h1DLL = dll_cache::get_info(HALO1_DLL_WSTR);
	auto h2DLL = dll_cache::get_info(HALO2_DLL_WSTR);
	auto h3DLL = dll_cache::get_info(HALO3_DLL_WSTR);
	auto hodstDLL = dll_cache::get_info(HALOODST_DLL_WSTR);
	auto hreachDLL = dll_cache::get_info(HALOREACH_DLL_WSTR);
	auto h4DLL = dll_cache::get_info(HALO4_DLL_WSTR);

	GetGameInformationResponsePayload gameInfoPayload = {};

	gameInfoPayload.Halo1Loaded = h1DLL.has_value() ? TRUE : FALSE;
	gameInfoPayload.Halo2Loaded = h2DLL.has_value() ? TRUE : FALSE;
	gameInfoPayload.Halo3Loaded = h3DLL.has_value() ? TRUE : FALSE;
	gameInfoPayload.ODSTLoaded = hodstDLL.has_value() ? TRUE : FALSE;
	gameInfoPayload.ReachLoaded = hreachDLL.has_value() ? TRUE : FALSE;
	gameInfoPayload.Halo4Loaded = h4DLL.has_value() ? TRUE : FALSE;

	// TODO-SCALES: ???
	gameInfoPayload.Halo1Information.Tick = 999;

	if (gameInfoPayload.Halo1Loaded) {
		halo1::h1snapshot h1Snapshot = {};
		halo1_engine::get_game_information(h1Snapshot);

		for (int i = 0; i < to_underlying(halo1::cheat::COUNT); i++) {
			gameInfoPayload.Halo1Information.SkullsEnabled[i] = h1Snapshot.skulls[i];
		}
	}

	response.header.type = InteropResponseType::SUCCESS;
	response.header.payload_size = sizeof(gameInfoPayload);

	response.payload.reserve(response.header.payload_size);
	memcpy_s(response.payload.data(), response.payload.capacity(), &gameInfoPayload, sizeof(gameInfoPayload));
}

void handle_response_set_halo1_skull_enabled(const InteropRequest& request, InteropResponse& response) {
	Halo1SetSkullEnabledRequestPayload skullSetPayload;
	memcpy_s(&skullSetPayload, sizeof(skullSetPayload), request.payload, sizeof(skullSetPayload));

	auto skull = (halo1::cheat)skullSetPayload.Skull;

	halo1_engine::set_cheat_enabled(skull, skullSetPayload.Enabled);

	response.header.type = InteropResponseType::SUCCESS;
}

void handle_response_kill_mcctas(const InteropRequest& /*request*/, InteropResponse& response) {
	global::kill_mcctas();

	response.header.type = InteropResponseType::SUCCESS;
}

void gui_interop::answer_request(windows_pipe_server::LPPIPEINST pipe)
{
	auto request_execution_start_time = std::chrono::steady_clock::now();

	InteropRequest request = { };
	memcpy_s(&request.header, sizeof(request.header), pipe->chRequest, sizeof(request.header));
	request.payload = pipe->chRequest + sizeof(request.header);

	InteropResponse response = { };

	switch (request.header.type) {
	case InteropRequestType::PING:
		response.header.type = InteropResponseType::SUCCESS;
		break;
	case InteropRequestType::GET_DLL_INFORMATION:
		handle_response_dll_information(request, response);
		break;
	case InteropRequestType::SET_CAMERA_DETAILS:
		handle_response_set_camera_details(request, response);
		break;
	case InteropRequestType::EXECUTE_COMMAND:
		handle_response_execute_command(request, response);
		break;
	case InteropRequestType::GET_GAME_INFORMATION:
		handle_response_get_game_information(request, response);
		break;
	case InteropRequestType::SET_HALO1_SKULL_ENABLED:
		handle_response_set_halo1_skull_enabled(request, response);
		break;
	case InteropRequestType::KILL_MCCTAS:
		handle_response_kill_mcctas(request, response);
		break;
	case InteropRequestType::INVALID:
	default:
	{
		response.header.type = InteropResponseType::INVALID_REQUEST;
		break;
	}
	}

	DWORD TotalWriteSize = 0;

	// Copy Header
	memcpy_s(pipe->chReply, windows_pipe_server::PIPE_BUFFER_SIZE, &response.header, sizeof(response.header));
	TotalWriteSize += sizeof(response.header);

	// Copy Payload
	if (response.header.payload_size > 0) {
		memcpy_s(
			pipe->chReply + sizeof(response.header),
			windows_pipe_server::PIPE_BUFFER_SIZE - sizeof(response.header),
			response.payload.data(),
			response.header.payload_size);
		TotalWriteSize += response.header.payload_size;
	}

	pipe->cbToWrite = TotalWriteSize;

	auto requestStr = InteropRequestTypeString.at(to_underlying(request.header.type));
	auto responseStr = InteropResponseTypeString.at(to_underlying(response.header.type));

	auto request_execution_stop_time = std::chrono::steady_clock::now();
	auto diff = request_execution_stop_time - request_execution_start_time;
	auto execution_time_ms = std::chrono::duration <double, std::milli>(diff).count();
	tas_logger::trace(L"Pipe serviced request {} with response {} in {:0.3f} ms", requestStr, responseStr, execution_time_ms);
}

gui_interop::gui_interop()
{
	tas_logger::debug("Starting Interop Pipe Server");
	pipe_server = std::make_unique<windows_pipe_server>();
	pipe_server->set_request_callback(&answer_request);
	std::thread serverThread = std::thread(&windows_pipe_server::server_main, pipe_server.get());
	serverThread.detach();
}

gui_interop::~gui_interop()
{
	pipe_server->stop();
}
