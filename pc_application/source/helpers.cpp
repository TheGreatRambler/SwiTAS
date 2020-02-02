#include "helpers.hpp"

std::string HELPERS::resolvePath(std::string path) {
	wxFileName fullPath("../" + path, wxPATH_NATIVE);
	fullPath.MakeAbsolute();
	std::string res = fullPath.GetFullPath(wxPATH_NATIVE).ToStdString();
	return res;
};