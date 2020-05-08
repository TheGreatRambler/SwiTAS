#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
#include <zpp.hpp>

#include "networkingStructures.hpp"

class SerializeProtocol {
private:
	// Will be converted to bytes
	std::vector<unsigned char> serializingData;

public:
	// Both of these functions are deliberately designed to deal with any kind of struct
	template <typename T> T binaryToData(uint8_t* data, uint16_t size) {
		// Load with YAS
		T inputData;

		// Create the archive
		zpp::serializer::memory_view_input_archive in(data, size);

		// This simple
		in(inputData);

		return inputData;
	}

	template <typename T> void dataToBinary(T inputData, uint8_t* data, uint16_t* size) {
		serializingData.clear();
		// Create the archive
		zpp::serializer::memory_output_archive out(serializingData);

		out(inputData);

		// Copy to pointer
		// An unsigned char is one byte, so this is it
		*size = (uint16_t)serializingData.size();
		memcpy(data, serializingData.data(), *size);
	}
};