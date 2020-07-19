#pragma once

#include <cstdint>
#include <type_traits>

namespace {
	constexpr auto operator""_KiB(const uint64_t x) { return (1024ULL) * x; }
	constexpr auto operator""_MiB(const uint64_t x) { return (1024ULL * 1024ULL) * x; }
	constexpr auto operator""_GiB(const uint64_t x) { return (1024ULL * 1024ULL * 1024ULL) * x; }
	constexpr auto operator""_TiB(const uint64_t x) { return (1024ULL * 1024ULL * 1024ULL * 1024ULL) * x; }

	constexpr auto operator""_KB(const uint64_t x) { return (1000ULL) * x; }
	constexpr auto operator""_MB(const uint64_t x) { return (1000ULL * 1000ULL) * x; }
	constexpr auto operator""_GB(const uint64_t x) { return (1000ULL * 1000ULL * 1000ULL) * x; }
	constexpr auto operator""_TB(const uint64_t x) { return (1000ULL * 1000ULL * 1000ULL * 1000ULL) * x; }
}

// Utility to get an enum value as the underlying type
template <typename E>
constexpr auto to_underlying(E e) noexcept
{
	return static_cast<std::underlying_type_t<E>>(e);
}

// Get pointer to existing values using offsets, dereferencing at each step
template <typename T>
T* value_ptr(void* baseAddress, int64_t baseOffset, std::initializer_list<int64_t> additionalOffsets) {

	std::byte* finalAddress = (std::byte*)baseAddress;

	if (finalAddress == nullptr)
		return nullptr;

	finalAddress += baseOffset;

	for (auto& offset : additionalOffsets) {
		if (finalAddress == nullptr)
			return nullptr;

		finalAddress = (std::byte*) * ((uintptr_t*)finalAddress);

		if (finalAddress == nullptr)
			return nullptr;

		finalAddress += offset;
	}

	if (finalAddress) {
		return (T*)finalAddress;
	}

	return nullptr;
}

// Get pointer to existing value with base and offset
template <typename T>
T* value_ptr(void* baseAddress, int64_t baseOffset) {

	std::byte* finalAddress = (std::byte*)baseAddress;
	finalAddress += baseOffset;
	return (T*)finalAddress;
}