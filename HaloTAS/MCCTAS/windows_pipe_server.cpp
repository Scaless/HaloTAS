#include "windows_pipe_server.h"

void windows_pipe_server::server_main()
{
	server_init();

	while (true) {
		if (KillServer) {
			server_cleanup();
			KillServer = false;
			return;
		}

		DWORD dwWait = WaitForSingleObject(Pipe.oOverlap.hEvent, 10);

		if (dwWait == WAIT_TIMEOUT) {
			continue;
		}

		if (dwWait != WAIT_OBJECT_0) {
			return;
		}

		// Get the result if the operation was pending. 
		if (Pipe.fPendingIO) {
			DWORD cbRet;
			auto fSuccess = GetOverlappedResult(
				Pipe.hPipeInst, // handle to pipe 
				&Pipe.oOverlap, // OVERLAPPED structure 
				&cbRet,         // bytes transferred 
				FALSE);         // do not wait 

			switch (Pipe.dwState)
			{
				// Pending connect operation 
			case PipeState::CONNECTING:
				if (!fSuccess)
				{
					//printf("Error %d.\n", GetLastError());
					return;
				}
				Pipe.dwState = PipeState::READING;
				break;

				// Pending read operation 
			case PipeState::READING:
				if (!fSuccess || cbRet == 0)
				{
					disconnect_and_reconnect();
					continue;
				}
				Pipe.cbRead = cbRet;
				Pipe.dwState = PipeState::WRITING;
				break;

				// Pending write operation 
			case PipeState::WRITING:
				if (!fSuccess || cbRet != Pipe.cbToWrite)
				{
					disconnect_and_reconnect();
					continue;
				}
				Pipe.dwState = PipeState::READING;
				break;

			default:
			{
				//printf("Invalid pipe state.\n");
				return;
			}
			}
		}

		// The pipe state determines which operation to do next. 
		switch (Pipe.dwState)
		{

		case PipeState::READING:
		{
			// READING_STATE: 
			// The pipe instance is connected to the client 
			// and is ready to read a request from the client. 
			auto fSuccess = ReadFile(
				Pipe.hPipeInst,
				Pipe.chRequest,
				PIPE_BUFFER_SIZE,
				&Pipe.cbRead,
				&Pipe.oOverlap);

			// The read operation completed successfully. 
			if (fSuccess && Pipe.cbRead != 0)
			{
				Pipe.fPendingIO = FALSE;
				Pipe.dwState = PipeState::WRITING;
				continue;
			}

			// The read operation is still pending. 
			auto dwErr = GetLastError();
			if (!fSuccess && (dwErr == ERROR_IO_PENDING))
			{
				Pipe.fPendingIO = TRUE;
				continue;
			}

			// An error occurred; disconnect from the client. 
			disconnect_and_reconnect();
			break;
		}

		case PipeState::WRITING:
		{
			// WRITING_STATE: 
			// The request was successfully read from the client. 
			// Get the reply data and write it to the client. 

			answer_request();

			DWORD cbRet;
			auto fSuccess = WriteFile(
				Pipe.hPipeInst,
				Pipe.chReply,
				Pipe.cbToWrite,
				&cbRet,
				&Pipe.oOverlap);

			// The write operation completed successfully. 
			if (fSuccess && cbRet == Pipe.cbToWrite)
			{
				Pipe.fPendingIO = FALSE;
				Pipe.dwState = PipeState::READING;
				continue;
			}

			// The write operation is still pending. 
			auto dwErr = GetLastError();
			if (!fSuccess && (dwErr == ERROR_IO_PENDING))
			{
				Pipe.fPendingIO = TRUE;
				continue;
			}

			// An error occurred; disconnect from the client. 

			disconnect_and_reconnect();
			break;
		}
		default:
		{
			//printf("Invalid pipe state.\n");
			return;
		}
		} // end switch

	} // end while
}

void windows_pipe_server::set_request_callback(void (*func)(LPPIPEINST))
{
	HandleRequestCallback = func;
}

void windows_pipe_server::stop()
{
	KillServer = true;
	while (KillServer) {
		// Spin until server stops
		Sleep(50);
	}
}

bool windows_pipe_server::server_init()
{
	LPCWSTR lpcwPipeName = TEXT("\\\\.\\pipe\\MCCTAS");

	Event = CreateEvent(
		NULL,    // default security attribute 
		TRUE,    // manual-reset event 
		TRUE,    // initial state = signaled 
		NULL);	 // unnamed event object 

	if (Event == NULL)
	{
		//printf("CreateEvent failed with %d.\n", GetLastError());
		return false;
	}

	Pipe.oOverlap.hEvent = Event;

	Pipe.hPipeInst = CreateNamedPipeW(
		lpcwPipeName,
		PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		PIPE_UNLIMITED_INSTANCES,
		PIPE_BUFFER_SIZE,
		PIPE_BUFFER_SIZE,
		PIPE_TIMEOUT,
		NULL);

	if (Pipe.hPipeInst == INVALID_HANDLE_VALUE)
	{
		//printf("CreateNamedPipe failed with %d.\n", GetLastError());
		return false;
	}

	Pipe.fPendingIO = connect_to_new_client();

	Pipe.dwState = Pipe.fPendingIO ?
		PipeState::CONNECTING : // still connecting 
		PipeState::READING;     // ready to read 

	return true;
}

void windows_pipe_server::server_cleanup()
{
	FlushFileBuffers(Pipe.hPipeInst);
	DisconnectNamedPipe(Pipe.hPipeInst);
	CloseHandle(Pipe.hPipeInst);
}

// This function is called when an error occurs or when the client 
// closes its handle to the pipe. Disconnect from this client, then 
// call ConnectNamedPipe to wait for another client to connect. 
void windows_pipe_server::disconnect_and_reconnect()
{
	// Disconnect the pipe instance. 
	if (!DisconnectNamedPipe(Pipe.hPipeInst))
	{
		//printf("DisconnectNamedPipe failed with %d.\n", GetLastError());
	}

	// Call a subroutine to connect to the new client. 
	Pipe.fPendingIO = connect_to_new_client();

	Pipe.dwState = Pipe.fPendingIO ?
		PipeState::CONNECTING : // still connecting 
		PipeState::READING;     // ready to read 
}

// This function is called to start an overlapped connect operation. 
// It returns TRUE if an operation is pending or FALSE if the 
// connection has been completed. 
BOOL windows_pipe_server::connect_to_new_client()
{
	BOOL fConnected, fPendingIO = FALSE;

	// Start an overlapped connection for this pipe instance. 
	fConnected = ConnectNamedPipe(Pipe.hPipeInst, &Pipe.oOverlap);

	// Overlapped ConnectNamedPipe should return zero. 
	if (fConnected)
	{
		//printf("ConnectNamedPipe failed with %d.\n", GetLastError());
		return 0;
	}

	switch (GetLastError())
	{
	case ERROR_IO_PENDING:
	{
		// The overlapped connection in progress. 
		fPendingIO = TRUE;
		break;
	}
	case ERROR_PIPE_CONNECTED:
	{
		// Client is already connected, so signal an event. 
		if (SetEvent(Pipe.oOverlap.hEvent))
			break;
	}
	default:
	{
		// If an error occurs during the connect operation... 
		//printf("ConnectNamedPipe failed with %d.\n", GetLastError());
		return 0;
	}
	}

	return fPendingIO;
}

void windows_pipe_server::answer_request()
{
	HandleRequestCallback(&Pipe);
}
