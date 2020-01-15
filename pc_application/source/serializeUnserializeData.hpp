#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
#include <zpp.h>

// Structs for the
struct Protocol_SetProjectName {
	std::string name;
}

namespace SerializeProtocol {
	// clang-format on
	// Will be converted to bytes
	std::vector<unsigned char> serializingData;

	// Both of these functions are deliberately designed to deal with any kind of struct
	template <typename T> std::shared_ptr<T> binaryToFrame(uint8_t * data, uint16_t size) {
		// Load with YAS
		std::shared_ptr<T> inputData = std::make_shared<T>();

		// Create the archive
		zpp::serializer::memory_input_archive in(serializingData);

		// This simple
		in(inputData);

		serializingData.clear();
		return inputData;
	}

	template <typename T> void frameToBinary(std::shared_ptr<T> inputData, uint8_t * data, uint16_t * size) {
		// Create the archive
		zpp::serializer::memory_output_archive out(serializingData);

		out(inputData);

		// Copy to pointer
		// An unsigned char is one byte, so this is it
		*size = (uint16_t)serializingData.size();
		memcpy(data, serializingData.data(), *size);
		serializingData.clear();
	}
}