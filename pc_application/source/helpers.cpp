#include "helpers.hpp"

std::string HELPERS::resolvePath(std::string path) {
#ifdef __WXMSW__
	wxFileName fullPath("../" + path, wxPATH_NATIVE);
#endif
#ifdef DEBIAN_SYSTEM
	wxFileName fullPath("/usr/share/switas/" + path, wxPATH_NATIVE);
#endif
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

wxFileName HELPERS::getMainSettingsPath(std::string name) {
	wxFileName relativeToExecutable(wxStandardPaths::Get().GetExecutablePath());
	// Go one folder back
	relativeToExecutable.RemoveDir(relativeToExecutable.GetDirCount() - 1);
	relativeToExecutable.SetName(wxString::FromUTF8(name));
	relativeToExecutable.SetExt("json");
	if(relativeToExecutable.FileExists()) {
		// Prefer relative to the executable
		return relativeToExecutable;
	} else {
		// Use the home folder as a backup
		wxFileName inHomeFolder(wxStandardPaths::Get().GetUserConfigDir());
		inHomeFolder.SetName(wxString::FromUTF8(name));
		inHomeFolder.SetExt("json");
		if(inHomeFolder.FileExists()) {
			return inHomeFolder;
		} else {
			// Put in etc lol, needed for debian
			wxFileName etcFolder("/etc/switas/");
			etcFolder.SetName(wxString::FromUTF8(name));
			etcFolder.SetExt("json");
			return etcFolder;
		}
	}
}

wxColor HELPERS::getDefaultWindowBackground() {
	return wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE);
}

wxFileName HELPERS::popOffDirs(wxFileName path, uint8_t num) {
	for(uint8_t i = 0; i < num; i++) {
		path.RemoveDir(path.GetDirCount() - 1);
	}
	return path;
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
	return new wxBitmapButton(parentFrame, wxID_ANY, wxBitmap(resizedImage));
}

wxBitmapButton* HELPERS::getSystemBitmapButton(wxWindow* parentFrame, wxArtID id) {
	return new wxBitmapButton(parentFrame, wxID_ANY, wxArtProvider::GetBitmap(id));
}

void HELPERS::addDarkmodeWindows(wxWindow* window) {
#ifdef __WXMSW__
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

wxImage HELPERS::getImageFromJPEGData(std::vector<uint8_t> jpegBuffer) {
	wxMemoryInputStream jpegStream(jpegBuffer.data(), jpegBuffer.size());
	wxImage jpegImage;
	jpegImage.LoadFile(jpegStream, wxBITMAP_TYPE_JPEG);
	return jpegImage;
}

wxString HELPERS::calculateDhash(wxImage image, int dhashWidth, int dhashHeight) {
	// Returns dhash as binary, 010011010, in string
	// https://github.com/telows/Image-Bath/blob/master/dhash.cpp
	wxImage copy = image.ConvertToGreyscale();
	copy.Rescale(dhashWidth, dhashHeight, wxIMAGE_QUALITY_NORMAL);
	char dhash[(dhashWidth - 1) * dhashHeight];
	unsigned char* imagePointer = copy.GetData();
	int dhashIndex              = 0;
	for(int y = 0; y < dhashHeight; y++) {
		for(int x = 1; x < dhashWidth; x++) {
			int thisPixelPointer     = ((y * dhashWidth) + x) * 3;
			unsigned char leftPixel  = imagePointer[thisPixelPointer - 4];
			unsigned char rightPixel = imagePointer[thisPixelPointer];
			dhash[dhashIndex]        = leftPixel > rightPixel ? '1' : '0';
			dhashIndex++;
		}
	}
	return wxString(dhash, sizeof(dhash));
}

const int HELPERS::getHammingDistance(wxString string1, wxString string2) {
	int counter = 0;
	for(int i = 0; i < string1.size(); i++) {
		if(string1.GetChar(i) != string2.GetChar(i)) {
			counter++;
		}
	}
	return counter;
}

wxBitmap* HELPERS::getDefaultSavestateScreenshot() {
	wxImage defaultImg(1280, 720);
	// Set all of it to a pretty grey
	defaultImg.SetRGB(wxRect(0, 0, 1280, 720), 76, 82, 92);
	return new wxBitmap(defaultImg);
}

std::string HELPERS::makeRelative(std::string path, std::string rootDir) {
	wxFileName newPath(wxString::FromUTF8(path));
	newPath.MakeRelativeTo(wxString::FromUTF8(rootDir));
	return newPath.GetFullPath().ToStdString();
}

std::string HELPERS::makeFromRelative(std::string path, std::string rootDir) {
	return rootDir + "/" + path;
}