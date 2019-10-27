#include "halo_map_data.h"

bool halo::mapdata::tag_id::operator==(const tag_id& other)
{
	return index == other.index &&
		identifier == other.identifier;
}
