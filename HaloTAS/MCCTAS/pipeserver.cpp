#include "pch.h"

void wait_for_object(HANDLE object)
{
	DWORD dw;
	MSG msg;

	for (;;)
	{
		dw = MsgWaitForMultipleObjectsEx(1, &object, INFINITE, QS_ALLINPUT, 0);

		if (dw == WAIT_OBJECT_0) break;
		if (dw == WAIT_OBJECT_0 + 1)
		{
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) DispatchMessage(&msg);
			continue;
		}
		srvfail(L"sleep() messageloop", GetLastError());
	}
}

HANDLE server_pipe;
HANDLE io_event;

void pipe_connection(void)
{
	OVERLAPPED overlapped;
	DWORD dw, err;

	SecureZeroMemory(&overlapped, sizeof(overlapped));
	overlapped.hEvent = io_event;

	if (!ReadFile(server_pipe, input_buffer, sizeof(input_buffer) - 1, NULL, &overlapped))
	{
		err = GetLastError();
		if (err == ERROR_IO_PENDING)
		{
			wait_for_object(io_event);
			if (!GetOverlappedResult(server_pipe, &overlapped, &dw, FALSE))
			{
				srvfail(L"Read from pipe failed asynchronously.", GetLastError());
			}
		}
		else
		{
			srvfail(L"Read from pipe failed synchronously.", GetLastError());
		}
	}
	else
	{
		if (!GetOverlappedResult(server_pipe, &overlapped, &dw, FALSE))
		{
			srvfail(L"GetOverlappedResult failed reading from pipe.", GetLastError());
		}
	}

	input_buffer[dw] = '\0';

	process_command();

	if (!WriteFile(server_pipe, &output_struct,
		((char*)&output_struct.output_string - (char*)&output_struct) + output_struct.string_length,
		NULL, &overlapped))
	{
		err = GetLastError();
		if (err == ERROR_IO_PENDING)
		{
			wait_for_object(io_event);
			if (!GetOverlappedResult(server_pipe, &overlapped, &dw, FALSE))
			{
				srvfail(L"Write to pipe failed asynchronously.", GetLastError());
			}
		}
		else
		{
			srvfail(L"Write to pipe failed synchronously.", GetLastError());
		}
	}
	else
	{
		if (!GetOverlappedResult(server_pipe, &overlapped, &dw, FALSE))
		{
			srvfail(L"GetOverlappedResult failed writing to pipe.", GetLastError());
		}
	}

	if (!FlushFileBuffers(server_pipe)) srvfail(L"FlushFileBuffers failed.", GetLastError());
	if (!DisconnectNamedPipe(server_pipe)) srvfail(L"DisconnectNamedPipe failed.", GetLastError());
}

void server(void)
{
	OVERLAPPED overlapped;
	DWORD err, dw;

	// Create the named pipe

	server_pipe = CreateNamedPipe(pipe_name, PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE | FILE_FLAG_OVERLAPPED, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 1, buffer_size, buffer_size, 0, NULL);
	if (server_pipe == INVALID_HANDLE_VALUE) srvfail(L"CreateNamedPipe failed.", GetLastError());

	// Wait for connections

	io_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (io_event == NULL) srvfail(L"CreateEvent(io_event) failed.", GetLastError());

	for (;;)
	{
		SecureZeroMemory(&overlapped, sizeof(overlapped));
		overlapped.hEvent = io_event;

		if (!ConnectNamedPipe(server_pipe, &overlapped))
		{
			err = GetLastError();
			if (err == ERROR_PIPE_CONNECTED)
			{
				pipe_connection();
			}
			else if (err == ERROR_IO_PENDING)
			{
				wait_for_object(io_event);
				if (!GetOverlappedResult(server_pipe, &overlapped, &dw, FALSE))
				{
					srvfail(L"Pipe connection failed asynchronously.", GetLastError());
				}
				pipe_connection();
			}
			else
			{
				srvfail(L"Pipe connection failed synchronously.", GetLastError());
			}
		}
		else
		{
			if (!GetOverlappedResult(server_pipe, &overlapped, &dw, FALSE))
			{
				srvfail(L"GetOverlappedResult failed connecting pipe.", GetLastError());
			}
			pipe_connection();
		}
	}
}