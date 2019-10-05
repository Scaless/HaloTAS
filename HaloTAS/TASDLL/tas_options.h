#pragma once

#include <unordered_map>
#include "tas_logger.h"
#include <boost/optional.hpp>
#include <boost/variant.hpp>

// Hotkeys
// Window state
// Renderer options
class tas_options
{
private:
	typedef boost::variant<std::string, int8_t, int16_t, int32_t, float> options_variant;

	std::unordered_map<std::string, options_variant> options{};

	static tas_options& get() {
		static tas_options instance;
		return instance;
	}

	tas_options();
	void load_config();

public:
	tas_options(tas_options const&) = delete;
	void operator=(tas_options const&) = delete;

	static void save_config();
	static void delete_config();

	// Get Value
	template<typename T>
	static boost::optional<T> get_value(std::string key);
	// Set Value
	template<typename T>
	static void set_value(std::string key, T value);
};

template<typename T>
inline boost::optional<T> tas_options::get_value(std::string key)
{
	auto& inst = tas_options::get();

	auto option = inst.options.find(key);
	if (option == inst.options.end()) {
		tas_logger::warning("options: tried to get value for non-existent key '%s'", key.c_str());
		return boost::none;
	}
	
	try {
		return boost::get<T>(inst.options[key]);
	}
	catch (...) {
		tas_logger::error("options: bad cast accessing value for key %s: %s", key);
	}

	return boost::none;
}

template<typename T>
inline void tas_options::set_value(std::string key, T value)
{
	auto& inst = tas_options::get();
	options_variant storedValue (value);
	inst.options[key] = storedValue;
}
