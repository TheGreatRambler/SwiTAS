#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
#include <zpp.hpp>

// Structs for the
struct Protocol_SetProjectName {
	std::string name;
};

class SerializeProtocol {
private:
	// clang-format on
	// Will be converted to bytes
	std::vector<unsigned char> serializingData;

public:
	// Both of these functions are deliberately designed to deal with any kind of struct
<<<<<<< HEAD
	template <typename T> std::shared_ptr<T> binaryToData(uint8_t* data, uint16_t size) {
		// Load with YAS
		std::shared_ptr<T> inputData = std::make_shared<T>();

		// Create the archive
		zpp::serializer::memory_input_archive in(serializingData);

		// This simple
		in(inputData);

		serializingData.clear();
		return inputData;
	}

	template <typename T> void dataToBinary(std::shared_ptr<T> inputData, uint8_t* data, uint16_t* size) {
		// Create the archive
		zpp::serializer::memory_output_archive out(serializingData);

		out(inputData);

		// Copy to pointer
		// An unsigned char is one byte, so this is it
		*size = (uint16_t)serializingData.size();
		memcpy(data, serializingData.data(), *size);
		serializingData.clear();
	}
=======
	template <typename T> std::shared_ptr<T> binaryToFrame(uint8_t* data, uint16_t size);

	template <typename T> void frameToBinary(std::shared_ptr<T> inputData, uint8_t* data, uint16_t* size);
>>>>>>> bf27387cf34d6d321956bfe23f51b7cccf5ad259
};