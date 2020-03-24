#include "helpers.hpp"

std::string HELPERS::resolvePath(std::string path) {
	wxFileName fullPath("../" + path, wxPATH_NATIVE);
	fullPath.MakeAbsolute();
	std::string res = fullPath.GetFullPath(wxPATH_NATIVE).ToStdString();
	return res;
};

std::vector<std::string> HELPERS::splitString(const std::string s, char delim) {
	std::vector<std::string> result;
	std::stringstream ss(s);
	std::string item;

	while(std::getline(ss, item, delim)) {
		result.push_back(item);
	}

	return result;
}

std::string HELPERS::joinString(std::vector<std::string> vec, std::string delimiter) {
	if(vec.size() != 0) {
		// https://stackoverflow.com/a/40052831
		// clang-format off
		return std::accumulate(std::next(vec.begin()), vec.end(),
			vec[0],
		[&delimiter](std::string& a, std::string& b) {
			return a + delimiter + b;
		});
		// clang-format on
	} else {
		return "";
	}
}

float HELPERS::normalizeRadian(float angle) {
	float a = std::fmod(angle, 2 * M_PI);
	return a >= 0 ? a : (a + 2 * M_PI);
}

wxBitmapButton* HELPERS::getBitmapButton(wxWindow* parentFrame, rapidjson::Document* settings, const char* name) {
	wxImage resizedImage(HELPERS::resolvePath((*settings)["ui"][name].GetString()));
	resizedImage.Rescale((*settings)["ui"]["buttonWidth"].GetInt(), (*settings)["ui"]["buttonHeight"].GetInt());
	return new wxBitmapButton(parentFrame, wxID_ANY, *(new wxBitmap(resizedImage)));
}