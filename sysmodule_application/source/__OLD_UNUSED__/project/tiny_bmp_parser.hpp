#pragma once

#include <arpa/inet.h>
#include <cstdint>
#include <cstdio>
#include <switch.h>

// From here https://stackoverflow.com/a/18675807

class BmpParser {
private:
	// The color index
	enum Color {
		RED   = 0,
		GREEN = 1,
		BLUE  = 2,
	};

	FILE* bmpFilePointer;

	uint8_t padSize;

	static constexpr uint8_t HEADER_SIZE = 14 + 40;

	uint16_t width;
	uint16_t height;

public:
	BmpParser();

	void createBmp(int w, int h, FILE* fptr);

	void loadBmp(FILE* fptr);

	uint32_t getColorValue(uint16_t x, uint16_t y);

	void setColorValue(uint16_t x, uint16_t y, uint32_t color);
};