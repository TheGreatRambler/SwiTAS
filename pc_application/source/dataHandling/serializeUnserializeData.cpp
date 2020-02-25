#include "serializeUnserializeData.hpp"

template <typename T> T SerializeProtocol::binaryToData(uint8_t* data, uint16_t size) {
	// Load with YAS
	T inputData;

	// Create the archive
	zpp::serializer::memory_input_archive in(serializingData);

	// This simple
	in(inputData);

	serializingData.clear();
	return inputData;
}

template <typename T> void SerializeProtocol::dataToBinary(T inputData, uint8_t* data, uint16_t* size) {
	// Create the archive
	zpp::serializer::memory_output_archive out(serializingData);

	out(inputData);

	// Copy to pointer
	// An unsigned char is one byte, so this is it
	*size = (uint16_t)serializingData.size();
	memcpy(data, serializingData.data(), *size);
	serializingData.clear();
}