#include "pch.h"
#include "gui_interop.h"

#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include "windows_utilities.h"

enum class InteropRequestType : int32_t {
	PING = 0,
	GET_DLL_INFORMATION = 1,

	INVALID = -1
};

enum class InteropResponseType : int32_t {
	PONG = 0,

	DLL_INFORMATION_FOUND = 1,
	DLL_INFORMATION_NOT_FOUND = 2,

	INVALID = -1
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

bool create_response_dll_information(DLLInformationResponsePayload& payloadOut, std::wstring& dllName)
{
	std::vector<loaded_dll_info> dlls =
	{
		loaded_dll_info(dllName),
	};

	fill_loaded_dlls_info(dlls);

	if (dlls[0].info.SizeOfImage == 0) {
		return false;
	}

	dlls[0].name.copy(payloadOut.dll_name, dlls[0].name.size(), 0);
	payloadOut.base_address = (uint64_t)dlls[0].info.lpBaseOfDll;
	payloadOut.entry_point = (uint64_t)dlls[0].info.EntryPoint;
	payloadOut.image_size = (uint64_t)dlls[0].info.SizeOfImage;

	return true;
}

void pipe_server_main(std::atomic_bool* close)
{
	using namespace tas::literals;
	using namespace std;

	constexpr DWORD dwInBufferSize = 4_MiB;
	constexpr DWORD dwOutBufferSize = 4_MiB;

	vector<char> inBuffer(dwInBufferSize, 0);
	vector<char> outBuffer(dwOutBufferSize, 0);

	while (*close != true) {
		LPCWSTR lpcwPipeName = TEXT("\\\\.\\pipe\\MCCTAS");
		HANDLE hPipe = CreateNamedPipeW(
			lpcwPipeName,
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			dwInBufferSize,
			dwOutBufferSize,
			0,
			nullptr
			);

		if (hPipe == INVALID_HANDLE_VALUE) {
			return;
		}

		// Wait for the client to connect; if it succeeds, 
		// the function returns a nonzero value. If the function
		// returns zero, GetLastError returns ERROR_PIPE_CONNECTED. 
		auto clientConnected = ConnectNamedPipe(hPipe, NULL) ?
			TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

		if (clientConnected) {
			while (true) {
				if (*close) {
					break;
				}

				DWORD dwBytesRead, dwBytesWritten;

				auto readSuccess = ReadFile(
					hPipe,
					inBuffer.data(),
					dwInBufferSize,
					&dwBytesRead,
					nullptr
					);

				if (!readSuccess || dwBytesRead == 0) {
					break;
				}

				static int i = 0;
				i++;

				// Process the read buffer
				// Create a response
				// ...

				InteropRequestHeader requestHeader = { };
				memcpy_s(&requestHeader, sizeof(requestHeader), inBuffer.data(), sizeof(requestHeader));

				DWORD TotalWriteSize = 0;
				InteropResponseHeader responseHeader = { };

				//////////////////
				switch (requestHeader.type) {
				case InteropRequestType::PING:
				{
					responseHeader.type = InteropResponseType::PONG;
					responseHeader.payload_size = i;
					memcpy_s(outBuffer.data(), outBuffer.capacity(), &responseHeader, sizeof(responseHeader));
					TotalWriteSize = sizeof(responseHeader);
					break;
				}
				case InteropRequestType::GET_DLL_INFORMATION:
				{
					DLLInformationRequestPayload requestPayload;
					memcpy_s(&requestPayload, sizeof(requestPayload), inBuffer.data() + sizeof(requestHeader), sizeof(requestPayload));
					std::wstring dllName(requestPayload.dll_name);

					DLLInformationResponsePayload responsePayload;
					bool success = create_response_dll_information(responsePayload, dllName);

					if (success) {
						responseHeader.type = InteropResponseType::DLL_INFORMATION_FOUND;
						responseHeader.payload_size = sizeof(responsePayload);
						char* currentWritePos = outBuffer.data();
						size_t remainingWriteSize = outBuffer.capacity();
						
						memcpy_s(currentWritePos, remainingWriteSize, &responseHeader, sizeof(responseHeader));
						currentWritePos += sizeof(responseHeader);
						remainingWriteSize -= sizeof(responseHeader);

						memcpy_s(currentWritePos, remainingWriteSize, &responsePayload, sizeof(responsePayload));
						currentWritePos += sizeof(responsePayload);
						remainingWriteSize -= sizeof(responsePayload);

						TotalWriteSize = outBuffer.capacity() - remainingWriteSize;
					}
					else {
						responseHeader.type = InteropResponseType::DLL_INFORMATION_NOT_FOUND;
						responseHeader.payload_size = 0;
						memcpy_s(outBuffer.data(), outBuffer.capacity(), &responseHeader, sizeof(responseHeader));
						TotalWriteSize = sizeof(responseHeader);
					}

					break;
				}
				case InteropRequestType::INVALID:
				default:
				{
					responseHeader.type = InteropResponseType::INVALID;
					responseHeader.payload_size = 0;
					memcpy_s(outBuffer.data(), outBuffer.capacity(), &responseHeader, sizeof(responseHeader));
					TotalWriteSize = sizeof(responseHeader);
					break;
				}
				} // End Switch, Refactor this later :)

				auto writeSuccess = WriteFile(
					hPipe,
					outBuffer.data(),
					TotalWriteSize,
					&dwBytesWritten,
					nullptr
					);

				if (!writeSuccess || TotalWriteSize != dwBytesWritten) {
					break;
				}
			}

			FlushFileBuffers(hPipe);
			DisconnectNamedPipe(hPipe);
			CloseHandle(hPipe);
		}
	}

	*close = false;
}

void gui_interop::start_pipe_server()
{
	//std::future<void> serverFuture = std::async(std::launch::async, pipe_server_main, &mServerKillFlag);
	std::thread serverThread = std::thread(pipe_server_main, &mServerKillFlag);
	serverThread.detach();
}

void gui_interop::stop_pipe_server()
{
	mServerKillFlag = true;

	while (mServerKillFlag == true) {

	}
}

