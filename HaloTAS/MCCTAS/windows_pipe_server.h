#pragma once
#include "pch.h"
#include <functional>
#include <atomic>

/*
The pipe implementation is based on the Microsoft example here, with some changes:
https://docs.microsoft.com/en-us/windows/win32/ipc/named-pipe-server-using-overlapped-i-o=
*/

using namespace tas::literals;
class windows_pipe_server
{
public:
	static constexpr size_t PIPE_BUFFER_SIZE = 4_MiB;
private:
	static constexpr size_t PIPE_TIMEOUT = 5000; // Milliseconds
	enum class PipeState : DWORD {
		CONNECTING,
		READING,
		WRITING
	};
public:
	typedef struct
	{
		OVERLAPPED oOverlap;
		HANDLE hPipeInst;
		char chRequest[PIPE_BUFFER_SIZE];
		DWORD cbRead;
		char chReply[PIPE_BUFFER_SIZE];
		DWORD cbToWrite;
		PipeState dwState;
		BOOL fPendingIO;
	} PIPEINST, * LPPIPEINST;

private:
	PIPEINST Pipe = {};
	HANDLE Event = {};
	std::atomic_bool KillServer = false;

	void (*HandleRequestCallback)(LPPIPEINST) = nullptr;

public:

	void server_main();
	void set_request_callback(void (*foo)(LPPIPEINST));
	void stop();

private:
	bool server_init();
	void server_cleanup();

	void disconnect_and_reconnect();
	BOOL connect_to_new_client();

	void answer_request();

};

