#pragma once

#include <string>
#include <unordered_map>
#include <vector>

struct core_cache_entry {
	std::vector<char> runtime_data;
	uint32_t rng;
	char map_string[32];
	char description[256];
};

class halo_core_cache
{
private:
	// id, data
	std::unordered_map<uint32_t, core_cache_entry> mCoreCache;
public:
	halo_core_cache() = default;
	~halo_core_cache() = default;

	void add_to_cache(uint32_t id, const char* data, size_t data_size);
	bool load_cache_entry_stage1(uint32_t id);
	void load_cache_entry_stage2(uint32_t id);
	std::vector<uint32_t> get_cached_ticks();
	void clear();
	
};

