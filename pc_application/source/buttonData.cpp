#include "buttonData.hpp"

Glib::RefPtr<Gdk::Pixbuf> ButtonData::getNewIcon(std::string path) {
	// https://stackoverflow.com/questions/5894344/gtkmm-how-to-put-a-pixbuf-in-a-treeview
	try {
		return Gdk::Pixbuf::create_from_file(path);
	} catch(const Glib::FileError& fileNotFoundError) {
		puts(fileNotFoundError.what().c_str());
	}
}

void ButtonData::setupButtonMapping(rapidjson::Document* mainSettings) {
	// Set up button mapping with the data obtained via JSON
	for(auto& b : (*mainSettings)["buttons"].GetObject()) {
		Btn chosenButton = stringToButton[b.name.GetString()];

		std::string scriptName   = b.value["scriptName"].GetString();
		std::string viewName     = b.value["viewName"].GetString();
		std::string onIconImage  = b.value["onIconImage"].GetString();
		std::string offIconImage = b.value["offIconImage"].GetString();
		std::string keybindName  = b.value["triggerKeybind"].GetString();
		// Get the gtk keyvalue from a gtk function
		// https://developer.gnome.org/gdk3/stable/gdk3-Keyboard-Handling.html#gdk-keyval-from-name
		ButtonInfo thisButtonInfo;
		thisButtonInfo.scriptName    = scriptName;
		thisButtonInfo.viewName      = viewName;
		thisButtonInfo.onIcon        = getNewIcon(onIconImage);
		thisButtonInfo.offIcon       = getNewIcon(offIconImage);
		thisButtonInfo.toggleKeybind = gdk_keyval_from_name(keybindName.c_str());
		buttonMapping[chosenButton]  = thisButtonInfo;
	}
}