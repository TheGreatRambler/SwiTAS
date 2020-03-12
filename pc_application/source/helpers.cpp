#include "helpers.hpp"

std::string HELPERS::resolvePath(std::string path) {
	wxFileName fullPath("../" + path, wxPATH_NATIVE);
	fullPath.MakeAbsolute();
	std::string res = fullPath.GetFullPath(wxPATH_NATIVE).ToStdString();
	return res;
};

std::vector<std::string> HELPERS::splitString(std::string s, char delim) {
	std::vector<std::string> result;
	std::stringstream ss(s);
	std::string item;

	while(std::getline(ss, item, delim)) {
		result.push_back(item);
	}

	return result;
}

std::string HELPERS::joinString(std::vector<std::string> vec, std::string delimiter) {
	// https://stackoverflow.com/a/40052831
	// clang-format off
    return std::accumulate(std::next(vec.begin()), vec.end(),
        vec[0],
    [&delimiter](std::string& a, std::string& b) {
        return a + delimiter + b;
    });
	// clang-format on
}