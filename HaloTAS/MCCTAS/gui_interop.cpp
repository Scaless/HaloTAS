#include "pch.h"
#include "gui_interop.h"

#include <thread>
#include <chrono>
#include <vector>
#include <string>

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

				// Process the read buffer
				// ...

				// Create a response
				// ...
				outBuffer[0] = 0;
				outBuffer[1] = 0;
				outBuffer[2] = 0;
				outBuffer[3] = 0;
				outBuffer[4] = 0;

				static int i = 0;
				i++;
				std::string Message = "Test " + std::to_string(i) + "\n";

				Message.copy(&(outBuffer.data()[5]), Message.length());
				DWORD TotalWriteSize = 5;
				//DWORD TotalWriteSize = Message.length() + 5;

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

