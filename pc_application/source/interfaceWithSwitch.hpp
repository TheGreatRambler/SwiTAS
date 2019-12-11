#pragma once

#include <cstdint>
#include <yas/serialize.hpp>
#include <yas/std_types.hpp>

#include "dataProcessing.hpp"

enum SEND_FLAG  : uint8_t {
    INPUTS
}

class InterfaceWithSwitch {
private:
	std::shared_ptr<DataProcessing> dataProcessing;

	constexpr std::size_t yasFlags = yas::mem | yas::binary;

	// Get the buffer to be sent to the switch, the full one
	yas::shared_buffer getMainBuffer (SEND_FLAGS flagType) {
		// https://github.com/niXman/yas
		yas::shared_buffer data;
		if (flagType == SEND_FLAG::INPUTS) {
			// data = // Do the thing here
		}

		// Return final buffer
		return yas::save<yasFlags> (
			// Starts with flag type, then data
			YAS_OBJECT ("main", flagType, data));
	}

public:
	InterfaceWithSwitch (std::shared_ptr<DataProcessing> dataProcessingInstance) {
		// Use the given class instance
		dataProcessing = dataProcessingInstance;
	}
}