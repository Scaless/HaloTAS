#include "plugin_loader.h"
#include "tas_logger.h"
#include <filesystem>
#include <string>

tag_property plugin_loader::make_property_recursive(tinyxml2::XMLElement* element)
{
	tag_property new_prop;

	new_prop.type = plugin::get_element_type(element->Name());

	if (element->Attribute("name")) {
		new_prop.name = element->Attribute("name");
	}
	if (element->Attribute("note")) {
		new_prop.note = element->Attribute("note");
	}
	if (element->Attribute("info_img")) {
		new_prop.info_img = element->Attribute("info_img");
	}
	new_prop.visible = element->BoolAttribute("visible", true);

	if (element->Attribute("offset")) {
		std::string offset_str = element->Attribute("offset");
		new_prop.local_offset = std::stoi(offset_str, 0, 16);
		if (new_prop.local_offset < 0) {
			tas_logger::warning("Offset is negative for property: %s", new_prop.name);
		}
	}
	
	switch (new_prop.type) {
	case TAG_PROPERTY_TYPE::REFLEXIVE:
		if (element->Attribute("size")) {
			std::string sizeStr = element->Attribute("size");
			new_prop.reflexive_size = std::stoi(sizeStr);

			if (new_prop.reflexive_size <= 0) {
				tas_logger::warning("Reflexive size is less than or equal to 0 for property: %s", new_prop.name);
			}
		}

		for (auto child = element->FirstChildElement(); child != nullptr; child = child->NextSiblingElement()) {
			tag_property childProperty = make_property_recursive(child);
			new_prop.reflexive.push_back(childProperty);
		}
		break;

	case TAG_PROPERTY_TYPE::ENUM16:
	case TAG_PROPERTY_TYPE::ENUM32:
	case TAG_PROPERTY_TYPE::BITMASK8:
	case TAG_PROPERTY_TYPE::BITMASK16:
	case TAG_PROPERTY_TYPE::BITMASK32:
		for (auto child = element->FirstChildElement(); child != nullptr; child = child->NextSiblingElement()) {
			tag_property childProperty = make_property_recursive(child);
			new_prop.reflexive.push_back(childProperty);
		}
		break;

	case TAG_PROPERTY_TYPE::OPTION:
		if (element->Attribute("value")) {
			std::string valueStr = element->Attribute("value");
			new_prop.option_value = std::stoi(valueStr);
		}
		break;
	}

	return new_prop;
}

bool plugin_loader::loaded()
{
	return plugins_loaded;
}

void plugin_loader::reload_plugins()
{
	plugins.clear();

	std::string plugin_root = "HaloTASFiles/Plugins";

	for (auto& p : std::filesystem::recursive_directory_iterator(plugin_root)) {
		if (!p.is_regular_file())
			continue;

		auto extension = p.path().extension();
		if (extension == ".xml" || extension == ".ent") {
			load_plugin_by_path(p.path());
		}
	}

	plugins_loaded = true;
}

void plugin_loader::load_plugin_by_path(const std::filesystem::path& plugin_file_path)
{
	using std::string;

	try {
		plugin plug;

		tinyxml2::XMLDocument doc;
		doc.LoadFile(plugin_file_path.string().c_str());

		auto rootElement = doc.FirstChildElement("plugin");

		plug.tag_class = rootElement->Attribute("class");
		plug.author = rootElement->Attribute("author");
		plug.version = rootElement->Attribute("version");
		plug.header_size = rootElement->Attribute("headersize");

		for (auto element = rootElement->FirstChildElement(); element != nullptr; element = element->NextSiblingElement()) {
			tag_property rootElement = make_property_recursive(element);
			plug.properties.push_back(rootElement);
		}

		plugins[plug.tag_class] = plug;
	}
	catch (std::exception& e) {
		auto errStr = plugin_file_path.string();
		tas_logger::error("Tried to load plugin (%s) but failed: %s", errStr, e.what());
		return;
	}
}

plugin* plugin_loader::get_plugin(std::string tag_class)
{
	if (plugins.count(tag_class)) {
		return &plugins[tag_class];
	}

	return nullptr;
}
