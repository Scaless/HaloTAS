#include "tas_options.h"
#include <filesystem>
#include <string>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

using boost::property_tree::ptree;
namespace fs = std::experimental::filesystem;

tas_options::tas_options()
{
	load_config();
}

void tas_options::load_config()
{
	try {
		//auto& inst = get();
		std::string rootPath = fs::current_path().string();
		std::string configFilePath = rootPath + "/HaloTASFiles/config.txt";

		if (std::filesystem::exists(configFilePath)) {
			std::ifstream is(configFilePath.c_str(), std::ifstream::in);
			boost::archive::text_iarchive archiveIn(is);
			archiveIn >> options;
			is.close();
		}
	}
	catch (const std::exception& e) {
		tas_logger::error("options: failed to load config.txt: %s", e.what());
	}
}

void tas_options::save_config()
{
	try {
		auto& inst = get();
		std::string rootPath = fs::current_path().string();
		std::string configFilePath = rootPath + "/HaloTASFiles/config.txt";
		std::ofstream os(configFilePath.c_str(), std::ofstream::out | std::ofstream::trunc);
		boost::archive::text_oarchive archiveOut(os);
		archiveOut << inst.options;
		os.close();
	}
	catch (const std::exception& e) {
		tas_logger::error("options: failed to save config.txt: %s", e.what());
	}
}

void tas_options::delete_config()
{
	try {
		std::string rootPath = fs::current_path().string();
		std::string configFilePath = rootPath + "/HaloTASFiles/config.txt";
		fs::remove(configFilePath);
	}
	catch (const std::exception& e) {
		tas_logger::error("options: failed to delete config.txt: %s", e.what());
	}
}
