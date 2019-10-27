#pragma once
#include <string>
#include <vector>
#include <inttypes.h>
#include "halo_map_data.h"

enum class TAG_PROPERTY_TYPE {
	ENUM16,
	ENUM32,
	BITMASK8,
	BITMASK16,
	BITMASK32,
	
	OPTION,

	FLOAT,
	INT8,
	INT16,
	INT32,
	STRING4,
	STRING32,
	REFERENCE, // == dependency
	REFLEXIVE, // == struct
	INDEX,
	COLOR_RGB32, // colorRGB, 4 bytes per color
	COLOR_RGBA8, // colorbyte, 1 byte per color
	COLOR_ARGB32, // colorARGB, 4 bytes per color

	UNKNOWN
};

struct tag_property {
	TAG_PROPERTY_TYPE type;
	std::string name;
	std::string note;
	std::string info_img;
	int32_t local_offset;
	bool visible;
	std::vector<tag_property> reflexive; // REFLEXIVE structures can have nested types
	int32_t reflexive_size;
	int32_t option_value;
};

struct plugin
{
	std::string tag_class;
	std::string author;
	std::string version;
	std::string header_size;

	std::vector<tag_property> properties;

	static TAG_PROPERTY_TYPE get_element_type(const std::string& type_str);

	void imgui_draw_plugin(halo::mapdata::tag_entry& tag);
};

