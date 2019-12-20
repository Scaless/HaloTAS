#include "halo_map_data.h"

std::string halo::mapdata::tag_entry::tag_class_to_str(uint32_t tag_class)
{
	char c[4];
	memcpy(&c, &tag_class, 4);
	std::reverse(c, c + 4);
	std::string str(c, 4);
	return str;
}

std::string halo::mapdata::tag_entry::tag_class_str()
{
	return tag_class_to_str(tag_class);
}

std::string halo::mapdata::tag_entry::tag_class_secondary_str()
{
	return tag_class_to_str(tag_class_secondary);
}

std::string halo::mapdata::tag_entry::tag_class_tertiary_str()
{
	return  tag_class_to_str(tag_class_tertiary);
}
