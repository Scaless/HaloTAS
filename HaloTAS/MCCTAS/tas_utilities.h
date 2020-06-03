#pragma once

#include <cstdint>

namespace tas::literals {
	constexpr auto operator""_KiB(const uint64_t x) { return (1024ULL) * x; }
	constexpr auto operator""_MiB(const uint64_t x) { return (1024ULL * 1024ULL) * x; }
	constexpr auto operator""_GiB(const uint64_t x) { return (1024ULL * 1024ULL * 1024ULL) * x; }
	constexpr auto operator""_TiB(const uint64_t x) { return (1024ULL * 1024ULL * 1024ULL * 1024ULL) * x; }

	constexpr auto operator""_KB(const uint64_t x) { return (1000ULL) * x; }
	constexpr auto operator""_MB(const uint64_t x) { return (1000ULL * 1000ULL) * x; }
	constexpr auto operator""_GB(const uint64_t x) { return (1000ULL * 1000ULL * 1000ULL) * x; }
	constexpr auto operator""_TB(const uint64_t x) { return (1000ULL * 1000ULL * 1000ULL * 1000ULL) * x; }
}

