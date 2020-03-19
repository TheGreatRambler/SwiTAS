#pragma once

#include "third_party/lodepng.h"
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/stat.h>

struct rgba4444_t {
	union {
		struct {
			uint16_t r : 4, g : 4, b : 4, a : 4;
		} __attribute__((packed));
		uint16_t rgba;
	};
	constexpr rgba4444_t()
		: rgba(0) {}
	constexpr rgba4444_t(uint16_t raw)
		: rgba(raw) {}
	constexpr rgba4444_t(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
		: r(r)
		, g(g)
		, b(b)
		, a(a) {}
};

namespace Helpers {
	int mkpath(const char* file_path, mode_t mode) {
		for(char* p = strchr(file_path + 1, '/'); p; p = strchr(p + 1, '/')) {
			*p = '\0';
			if(mkdir(file_path, mode) == -1) {
				if(errno != EEXIST) {
					*p = '/';
					return -1;
				}
			}
			*p = '/';
		}
		return 0;
	}

	std::string getPageBinName(std::string projectFolder, uint16_t pageNum) {
		return projectFolder + "/frames/page" + std::to_string(pageNum) + ".bin";
	}
}