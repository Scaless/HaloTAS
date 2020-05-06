#include "halo_core_cache.h"
#include "halo_constants.h"
#include "halo_engine.h"
#include "lz4.h"
#include "tas_logger.h"
#include <fstream>

void halo_core_cache::add_to_cache(uint32_t id, const char* data, size_t data_size)
{
	core_cache_entry entry;

	const size_t max_dest_size = LZ4_compressBound(data_size);
	tas_logger::debug("core_cache_save: Allocating %d bytes for core save.", max_dest_size);
	entry.runtime_data.resize(max_dest_size);

	const int compressed_data_size = LZ4_compress_default(data, entry.runtime_data.data(), data_size, max_dest_size);
	
	if (compressed_data_size > 0) {
		tas_logger::info("core_cache_save: Added id %d to cache / Original[%d] Compressed[%d] Ratio[%.3f]",
			id, data_size, compressed_data_size, (float)compressed_data_size / data_size);
		entry.runtime_data.resize(compressed_data_size);
		entry.runtime_data.shrink_to_fit();
	}
	else {
		tas_logger::warning("core_cache_save: Failed to add id %d to cache.", id);
		return;
	}

	strcpy_s((char*)&entry.map_string, sizeof(entry.map_string), halo::addr::MAP_STRING);
	entry.rng = *halo::addr::RNG;
	ZeroMemory(entry.description, sizeof(entry.description));

	mCoreCache.emplace(id, entry);
}

bool halo_core_cache::load_cache_entry_stage1(uint32_t id)
{
	auto key = mCoreCache.find(id);

	if (key != mCoreCache.end()) {
		auto& entry = key->second;

		if (strcmp(halo::addr::MAP_STRING, entry.map_string) != 0) {
			auto& gEngine = halo_engine::get();
			std::string load_map_str = "map_name " + std::string(entry.map_string);
			gEngine.execute_command(load_map_str.c_str());
			return true;
		}
	}
	else {
		tas_logger::warning("core_cache_load: Failed to load cache entry with id %d (stage1).", id);
	}
	return false;
}

void halo_core_cache::load_cache_entry_stage2(uint32_t id)
{
	auto key = mCoreCache.find(id);

	if (key != mCoreCache.end()) {
		auto& entry = key->second;

		std::vector<char> decompressed_data;
		decompressed_data.resize(halo::constants::CORE_SAVE_SIZE);
		const int decompressed_size = LZ4_decompress_safe(entry.runtime_data.data(), decompressed_data.data(), entry.runtime_data.size(), halo::constants::CORE_SAVE_SIZE);

		if (decompressed_size > 0) {
			std::ofstream logFile("C:\\Users\\Scales\\Documents\\my games\\Halo\\core\\core.bin", std::ios::binary);
			logFile.write(reinterpret_cast<char*>(decompressed_data.data()), decompressed_data.size());
			logFile.close();
			
			*halo::addr::RNG = entry.rng;
			auto& gEngine = halo_engine::get();
			gEngine.execute_command("core_load");
			//memcpy_s(halo::addr::RUNTIME_DATA_BEGIN, halo::constants::CORE_SAVE_SIZE, decompressed_data.data(), decompressed_data.size());
		}
		else {
			tas_logger::warning("core_cache_load: Failed to decompress cache entry with id %d (stage2).", id);
			return;
		}
	}
	else {
		tas_logger::warning("core_cache_load: Failed to load cache entry with id %d (stage2).", id);
		return;
	}
}

std::vector<uint32_t> halo_core_cache::get_cached_ticks()
{
	std::vector<uint32_t> ticks;
	for (auto& t : mCoreCache) {
		ticks.push_back(t.first);
	}
	return ticks;
}

void halo_core_cache::clear()
{
	mCoreCache.clear();
}
