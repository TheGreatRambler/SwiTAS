#include <cstdio>

#include "tiny_bmp_parser.hpp"

void BmpParser::createBmp(int w, int h, FILE* fptr) {
	// mimeType = "image/bmp"
	// https://stackoverflow.com/a/18675807
	// https://en.wikipedia.org/wiki/BMP_file_format

	// Create file
	// FILE* fptr = fopen(fileName, "rb");

	width  = w;
	height = h;

	unsigned char file[14] = {
		'B', 'M',            // magic
		0, 0, 0, 0,          // size in bytes
		0, 0,                // app data
		0, 0,                // app data
		HEADER_SIZE, 0, 0, 0 // start of data offset
	};

	unsigned char info[40] = {
		40, 0, 0, 0,      // info hd size
		0, 0, 0, 0,       // width
		0, 0, 0, 0,       // heigth
		1, 0,             // number color planes
		24, 0,            // bits per pixel
		0, 0, 0, 0,       // compression is none
		0, 0, 0, 0,       // image bits size
		0x13, 0x0B, 0, 0, // horz resoluition in pixel / m
		0x13, 0x0B, 0, 0, // vert resolutions (0x03C3 = 96 dpi, 0x0B13 = 72 dpi)
		0, 0, 0, 0,       // #colors in pallete
		0, 0, 0, 0,       // #important colors
	};

	padSize      = (4 - (w * 3) % 4) % 4;
	int sizeData = w * h * 3 + h * padSize;
	int sizeAll  = sizeData + sizeof(file) + sizeof(info);

	file[2] = (unsigned char)(sizeAll);
	file[3] = (unsigned char)(sizeAll >> 8);
	file[4] = (unsigned char)(sizeAll >> 16);
	file[5] = (unsigned char)(sizeAll >> 24);

	info[4] = (unsigned char)(w);
	info[5] = (unsigned char)(w >> 8);
	info[6] = (unsigned char)(w >> 16);
	info[7] = (unsigned char)(w >> 24);

	info[8]  = (unsigned char)(h);
	info[9]  = (unsigned char)(h >> 8);
	info[10] = (unsigned char)(h >> 16);
	info[11] = (unsigned char)(h >> 24);

	info[20] = (unsigned char)(sizeData);
	info[21] = (unsigned char)(sizeData >> 8);
	info[22] = (unsigned char)(sizeData >> 16);
	info[23] = (unsigned char)(sizeData >> 24);

	// Write the headers to the file
	fwrite(&file, sizeof(file), 1, fptr);
	fwrite(&info, sizeof(info), 1, fptr);

	bmpFilePointer = fptr;
}

void BmpParser::loadBmp(FILE* fptr) {
	// https://stackoverflow.com/a/17040962
	bmpFilePointer = fptr;
	unsigned char entireHeader[54];
	// read the 54-byte header
	// Not the image data, don't worry
	fread(entireHeader, sizeof(unsigned char), 54, fptr);

	// extract image height and width from header
	width  = *(int*)&entireHeader[18];
	height = *(int*)&entireHeader[22];

	padSize = (width * 3 + 3) & (~3);
}

uint32_t BmpParser::getColorValue(uint16_t x, uint16_t y) {
	// Have to return color in entirety, might need to change endianness
	uint32_t index = x * 3 + (height - y) * (3 * width + padSize);

	// Seek to location in file
	fseek(bmpFilePointer, HEADER_SIZE + index, SEEK_SET);

	unsigned char colors[3]; // BGR
	fread(&colors, sizeof(unsigned char), 3, bmpFilePointer);
	// Use AMS color function
	return RGBA8(colors[2], colors[1], colors[0], 0xFF);
	// Ignore padding, don't read or write it
}

void BmpParser::setColorValue(uint16_t x, uint16_t y, uint32_t color) {
	// Go to the correct spot in the file
	// The bitmap data starts with the lower left hand corner of the image
	// Only 3 colors and padsize, I dunno
	uint32_t index = x * 3 + (height - y) * (3 * width + padSize);
	fseek(bmpFilePointer, HEADER_SIZE + index, SEEK_SET);
	// Might need to color = htonl(color)
	uint8_t colors[3] = { (&color)[2], (&color)[1], (&color)[0] }; // BGR

	// Write 3 colors
	fwrite(&colors, sizeof(colors), 1, bmpFilePointer);
	if(x == width - 1) {
		// Write padding
		uint8_t nothing = 0;
		fwrite(&nothing, sizeof(nothing), padSize, bmpFilePointer);
	}
}