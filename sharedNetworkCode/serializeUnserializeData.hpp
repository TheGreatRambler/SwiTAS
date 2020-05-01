#pragma once

#include "include/zpp.hpp"
#include <cstdint>
#include <cstring>
#include <memory>

#include "networkingStructures.hpp"

class SerializeProtocol {
private:
	// Will be converted to bytes
	std::vector<unsigned char> serializingData;

public:
	// Both of these functions are deliberately designed to deal with any kind of struct
	template <typename T> void binaryToData(T& outputData, uint8_t* data, uint32_t size) {
		// Create the archive
		zpp::serializer::memory_view_input_archive in(data, size);

		// This simple
		in(outputData);
	}

	template <typename T> void dataToBinary(T inputData, uint8_t** data, std::size_t* size) {
		serializingData.clear();
		// Create the archive
		zpp::serializer::memory_output_archive out(serializingData);

		out(inputData);

		// Copy to pointer
		// An unsigned char is one byte, so this is it
		*size = (uint32_t)serializingData.size();
		// Needs to be freed afterwards
		*data = (uint8_t*)malloc(*size);
		memcpy(*data, serializingData.data(), *size);
	}
};