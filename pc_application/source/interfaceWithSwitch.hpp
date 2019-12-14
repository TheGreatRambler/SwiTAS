#pragma once

#include <cstdint>
#include <yas/serialize.hpp>
#include <yas/std_types.hpp>
#include <zed_net.h>

#include "dataProcessing.hpp"

enum SEND_FLAG  : uint8_t {
    INPUTS
}

class InterfaceWithSwitch {
private:
	std::shared_ptr<DataProcessing> dataProcessing;

	constexpr static std::size_t yasFlags = yas::mem | yas::binary;

	zed_net_socket_t communicationSocket;
	constexpr static unsigned short port = 4466;

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
		// Start up socket
		zed_net_init ();
		// Open up one
		zed_net_udp_socket_open (&communicationSocket, port, 0);
		// Use the given class instance
		dataProcessing = dataProcessingInstance;
	}

	~InterfaceWithSwitch () {
		// Shutdown socket
		zed_net_shutdown ()
	}
}