#pragma once

#include <atomic>

class gui_interop
{
private:
	std::atomic_bool mServerKillFlag{ false };

public:
	gui_interop() = default;
	~gui_interop() = default;

	void start_pipe_server();
	void stop_pipe_server();
};

