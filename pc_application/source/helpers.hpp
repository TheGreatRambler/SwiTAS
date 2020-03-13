#pragma once

#include <cmath>
#include <numeric>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>
#include <wx/filename.h>

namespace HELPERS {
	std::string resolvePath(std::string path);

	std::vector<std::string> splitString(const std::string s, char delim);
	std::string joinString(std::vector<std::string> strings, std::string delimiter);

	float normalizeRadian(float angle) {
		float a = fmod(angle, 2 * M_PI);
		return a >= 0 ? a : (a + 2 * M_PI);
	}
}