#pragma once
#include <inttypes.h>
#include <string>
#include <algorithm>

namespace halo::mapdata {
	
	struct color_bgra8 {
		uint8_t b;
		uint8_t g;
		uint8_t r;
		uint8_t a;
	};

	struct color_rgb32 {
		float r;
		float g;
		float b;
	};

	struct color_rgba32 {
		float r;
		float g;
		float b;
		float a;
	};

	struct tag_id {
		uint16_t index;
		uint16_t identifier;

		bool operator==(const tag_id& other);
	};

	struct tag_reference {
		uint32_t tag_class;
		const char* tag_path; // unused?
		uint32_t unknown;
		tag_id tag_index;
	};

	struct tag_reflexive {
		uint32_t data_count;
		void* data_address;
		uint32_t unknown;
	};

	struct tag_entry {
		uint32_t tag_class;
		uint32_t tag_class_secondary;
		uint32_t tag_class_tertiary;
		tag_id tag_index;
		const char* tag_path;
		void* tag_data;
		uint32_t unknown;
		uint32_t unknown2;

		std::string tag_class_str() {
			return tag_class_to_str(tag_class);
		}
		std::string tag_class_secondary_str() {
			return tag_class_to_str(tag_class_secondary);
		}
		std::string tag_class_tertiary_str() {
			return tag_class_to_str(tag_class_tertiary);
		}

	private:
		std::string tag_class_to_str(uint32_t tag_class) {
			char c[4];
			memcpy(&c, &tag_class, 4);
			std::reverse(c, c + 4);
			std::string str(c, 4);
			return str;
		}
	};

	struct tag_header {
		tag_entry* tag_array;
		tag_id first_tag_id;
		uint32_t map_id; // ???
		uint32_t tag_count;
		uint32_t vertex_count;
		uint32_t vertex_offset;
		uint32_t index_count;
		uint32_t index_offset;
		uint32_t model_data_size;
		uint32_t footer; // 0x74616773 = "tags"
	};
}

class halo_map_data
{

};

