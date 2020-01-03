#pragma once

#include <cstdint>
#include <yas/serialize.hpp>
#include <yas/std_types.hpp>
#include <zed_net.h>

#include "dataProcessing.hpp"

enum SEND_FLAGS : uint8_t {
	INPUTS
};

class InterfaceWithSwitch {
private:
	std::shared_ptr<DataProcessing> dataProcessing;

	constexpr static std::size_t yasFlags = yas::mem | yas::binary;

	zed_net_socket_t communicationSocket;
	constexpr static unsigned short port = 4466;

	// Get the buffer to be sent to the switch, the full one
	yas::shared_buffer getMainBuffer(SEND_FLAGS flagType);

public:
	InterfaceWithSwitch(std::shared_ptr<DataProcessing> dataProcessingInstance);

	~InterfaceWithSwitch();
};