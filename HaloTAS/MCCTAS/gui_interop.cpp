#include "gui_interop.h"
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <unordered_map>
#include "windows_utilities.h"

enum class InteropRequestType : int32_t {
	PING = 0,
	GET_DLL_INFORMATION = 1,
	SET_CAMERA_DETAILS = 2,

	INVALID = -1
};
std::unordered_map<int32_t, const wchar_t*> InteropRequestTypeString {
	{to_underlying(InteropRequestType::PING), L"PING"},
	{to_underlying(InteropRequestType::GET_DLL_INFORMATION), L"GET_DLL_INFORMATION"},
	{to_underlying(InteropRequestType::SET_CAMERA_DETAILS), L"SET_CAMERA_DETAILS"},
	
	{to_underlying(InteropRequestType::INVALID), L"INVALID"},
};

enum class InteropResponseType : int32_t {
	SUCCESS = 0,

	DLL_INFORMATION_FOUND = 1,
	DLL_INFORMATION_NOT_FOUND = 2,

	INVALID = -1
};
std::unordered_map<int32_t, const wchar_t*> InteropResponseTypeString{
	{to_underlying(InteropResponseType::SUCCESS), L"SUCCESS"},
	{to_underlying(InteropResponseType::DLL_INFORMATION_FOUND), L"DLL_INFORMATION_FOUND"},
	{to_underlying(InteropResponseType::DLL_INFORMATION_NOT_FOUND), L"DLL_INFORMATION_NOT_FOUND"},

	{to_underlying(InteropResponseType::INVALID), L"INVALID"},
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
		: type(InteropResponseType::INVALID), payload_size(0)
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

	std::vector<loaded_dll_info> dlls =
	{
		loaded_dll_info(dllName),
	};

	fill_loaded_dlls_info(dlls);

	if (dlls[0].info.SizeOfImage == 0) {
		response.header.type = InteropResponseType::DLL_INFORMATION_NOT_FOUND;
		return;
	}

	DLLInformationResponsePayload payloadOut;

	dlls[0].name.copy(payloadOut.dll_name, dlls[0].name.size(), 0);
	payloadOut.base_address = (uint64_t)dlls[0].info.lpBaseOfDll;
	payloadOut.entry_point = (uint64_t)dlls[0].info.EntryPoint;
	payloadOut.image_size = (uint64_t)dlls[0].info.SizeOfImage;

	response.header.type = InteropResponseType::DLL_INFORMATION_FOUND;
	response.header.payload_size = sizeof(payloadOut);

	response.payload.reserve(response.header.payload_size);
	memcpy_s(response.payload.data(), response.payload.capacity(), &payloadOut, sizeof(payloadOut));
}

void handle_response_set_camera_details(const InteropRequest& request, InteropResponse& response) {
	
	SetCameraDetailsRequestPayload cameraDetailsPayload;
	memcpy_s(&cameraDetailsPayload, sizeof(cameraDetailsPayload), request.payload, sizeof(cameraDetailsPayload));

	float* cameraPositionArr = reinterpret_cast<float*>(0x182199338);

	cameraPositionArr[0] = cameraDetailsPayload.positionX;
	cameraPositionArr[1] = cameraDetailsPayload.positionY;
	cameraPositionArr[2] = cameraDetailsPayload.positionZ;
	
	response.header.type = InteropResponseType::SUCCESS;
}

void gui_interop::answer_request(windows_pipe_server::LPPIPEINST pipe)
{
	InteropRequest request = { };
	memcpy_s(&request.header, sizeof(request.header), pipe->chRequest, sizeof(request.header));
	request.payload = pipe->chRequest + sizeof(request.header);

	InteropResponse response = { };

	switch (request.header.type) {
	case InteropRequestType::PING:
	{
		response.header.type = InteropResponseType::SUCCESS;
		break;
	}
	case InteropRequestType::GET_DLL_INFORMATION:
	{
		handle_response_dll_information(request, response);
		break;
	}
	case InteropRequestType::SET_CAMERA_DETAILS:
	{
		handle_response_set_camera_details(request, response);
		break;
	}
	case InteropRequestType::INVALID:
	default:
	{
		response.header.type = InteropResponseType::INVALID;
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

	tas_logger::log(L"Pipe serviced request %s with response %s\r\n", requestStr, responseStr);
}

gui_interop::gui_interop()
{
	pipe_server = std::make_unique<windows_pipe_server>();
	pipe_server->set_request_callback(&answer_request);
	std::thread serverThread = std::thread(&windows_pipe_server::server_main, pipe_server.get());
	serverThread.detach();
}

gui_interop::~gui_interop()
{
	pipe_server->stop();
}
