// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// Windows
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <Psapi.h>
#include <winternl.h>
#include <Dbghelp.h>

// Standard library
#include <iostream>
#include <memory>
#include <cstdint>
#include <variant>
#include <optional>
#include <atomic>
#include <type_traits>
#include <functional>
#include <chrono>
#include <filesystem>
#include <mutex>
#include <thread>

#include <vector>
#include <string>
#include <unordered_map>

// Libraries
#include <d3d11.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_impl_dx11.h>

#pragma warning(push)
#pragma warning(disable: 26495 4201)
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 26451 26495 26812)
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#pragma warning(pop)

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/circular_buffer.hpp>

#include "kiero.h"

// TAS Utilities
#include "tas_utilities.h"
#include "tas_logger.h"
#include "halo_constants.h"

#endif //PCH_H
