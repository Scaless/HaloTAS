#pragma once
#include "pch.h"
#include <atomic>
#include <memory>
#include "windows_pipe_server.h"

class gui_interop
{
private:
	std::unique_ptr<windows_pipe_server> pipe_server;

public:
	gui_interop();
	~gui_interop();

private:

	static void answer_request(windows_pipe_server::LPPIPEINST pipe);
};

