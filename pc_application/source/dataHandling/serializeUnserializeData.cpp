#include "serializeUnserializeData.hpp"

template <typename T> std::shared_ptr<T> SerializeProtocol::binaryToFrame(uint8_t* data, uint16_t size) {
	// Load with YAS
	std::shared_ptr<T> inputData = std::make_shared<T>();

	// Create the archive
	zpp::serializer::memory_input_archive in(serializingData);

	// This simple
	in(inputData);

	serializingData.clear();
	return inputData;
}

template <typename T> void SerializeProtocol::frameToBinary(std::shared_ptr<T> inputData, uint8_t* data, uint16_t* size) {
	// Create the archive
	zpp::serializer::memory_output_archive out(serializingData);

	out(inputData);

	// Copy to pointer
	// An unsigned char is one byte, so this is it
	*size = (uint16_t)serializingData.size();
	memcpy(data, serializingData.data(), *size);
	serializingData.clear();
}