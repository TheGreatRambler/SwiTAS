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

rapidjson::Document HELPERS::getSettingsFile(std::string filename) {
	rapidjson::Document json;
	std::ifstream file(filename);
	std::string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
	// Allow comments in JSON
	json.Parse<rapidjson::kParseCommentsFlag>(content.c_str());
	return json;
}

rapidjson::Document HELPERS::getSettingsFromString(std::string jsonString) {
	rapidjson::Document json;
	// Allow comments in JSON
	json.Parse<rapidjson::kParseCommentsFlag>(jsonString.c_str());
	return json;
}

wxBitmapButton* HELPERS::getBitmapButton(wxWindow* parentFrame, rapidjson::Document* settings, const char* name) {
	wxImage resizedImage(HELPERS::resolvePath((*settings)["ui"][name].GetString()));
	resizedImage.Rescale((*settings)["ui"]["buttonWidth"].GetInt(), (*settings)["ui"]["buttonHeight"].GetInt());
	return new wxBitmapButton(parentFrame, wxID_ANY, *(new wxBitmap(resizedImage)));
}

wxBitmapButton* HELPERS::getSystemBitmapButton(wxWindow* parentFrame, wxArtID id) {
	return new wxBitmapButton(parentFrame, wxID_ANY, wxArtProvider::GetBitmap(id));
}

void HELPERS::addDarkmodeWindows(wxWindow* window) {
#ifdef _WIN32
	// Enable dark mode, super experimential, apparently
	// needs to be applied to every window, however
	SetWindowTheme(window->GetHWND(), L"DarkMode_Explorer", NULL);
	window->Refresh();
#endif
}

// https://stackoverflow.com/a/478960/9329945
// Executes command and gets output
std::string HELPERS::exec(const char* cmd) {
	wxArrayString outputArray;
	long resultCode = wxExecute(cmd, outputArray, wxEXEC_HIDE_CONSOLE);

	std::size_t numOfLines = outputArray.GetCount();
	if(numOfLines != 0) {
		std::string output = outputArray[0].ToStdString();
		for(std::size_t i = 1; i < numOfLines; i++) {
			output += ("\n" + outputArray[i].ToStdString());
		}
		return output;
	} else {
		return "";
	}
}

wxBitmap* HELPERS::getBitmapFromJPEGData(std::vector<uint8_t> jpegBuffer) {
	wxMemoryInputStream jpegStream(jpegBuffer.data(), jpegBuffer.size());
	wxImage jpegImage;
	jpegImage.LoadFile(jpegStream, wxBITMAP_TYPE_JPEG);
	return new wxBitmap(jpegImage);
}

int HELPERS::char2int(char input) {
	if(input >= '0' && input <= '9')
		return input - '0';
	if(input >= 'A' && input <= 'F')
		return input - 'A' + 10;
	if(input >= 'a' && input <= 'f')
		return input - 'a' + 10;
	else
		return 0;
}

void HELPERS::hex2bin(const char* src, uint8_t* target) {
	// https://stackoverflow.com/a/17261928/9329945
	while(*src && src[1]) {
		*(target++) = char2int(*src) * 16 + char2int(src[1]);
		src += 2;
	}
}

const uint16_t HELPERS::getHammingDistance(std::string hexString1, std::string hexString2) {
	// Both hex strings MUST have the same size
	uint8_t m1[hexString1.length() / 2];
	uint8_t m2[hexString2.length() / 2];
	std::size_t size = hexString1.length() / 2;

	hex2bin(hexString1.c_str(), m1);
	hex2bin(hexString2.c_str(), m2);

	// https://gist.github.com/Miguellissimo/2faa7e3c3e1800a6bf97
	uint16_t counter = 0;

	for(std::size_t i = 0; i != size; ++i) {
		uint8_t diff = m1[i] ^ m2[i];

		diff = (diff & (uint8_t)0x55) + ((diff >> 1) & (uint8_t)0x55);
		diff = (diff & (uint8_t)0x33) + ((diff >> 2) & (uint8_t)0x33);
		diff = (diff & (uint8_t)0x0f) + ((diff >> 4) & (uint8_t)0x0f);

		counter += diff;
	}

	return counter;
}

std::string HELPERS::makeRelative(std::string path, std::string rootDir) {
	wxFileName newPath(wxString::FromUTF8(path));
	newPath.MakeRelativeTo(wxString::FromUTF8(rootDir));
	return newPath.GetFullPath().ToStdString();
}
std::string HELPERS::makeFromRelative(std::string path, std::string rootDir) {
	return rootDir + "/" + path;
}