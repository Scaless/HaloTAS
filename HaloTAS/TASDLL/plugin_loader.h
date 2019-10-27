#pragma once

#include <string>
#include <iostream>
#include <unordered_map>
#include <filesystem>
#include "plugin.h"
#include "tinyxml2.h"

class plugin_loader
{
	// Tag Class, Plugin
	std::unordered_map<std::string, plugin> plugins;
	bool plugins_loaded{ false };

	tag_property make_property_recursive(tinyxml2::XMLElement* element);
public:
	bool loaded();
	void reload_plugins();
	void load_plugin_by_path(const std::filesystem::path& plugin_file_path);

	plugin* get_plugin(std::string tag_class);
};

