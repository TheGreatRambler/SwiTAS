#include "interfaceWithSwitch.hpp"

yas::shared_buffer InterfaceWithSwitch::getMainBuffer(SEND_FLAGS flagType) {
	// https://github.com/niXman/yas
	yas::shared_buffer data;
	if(flagType == SEND_FLAGS::INPUTS) {
		// data = // Do the thing here
	}

	// Return final buffer
	return yas::save<yasFlags>(
		// Starts with flag type, then data
		YAS_OBJECT("main", flagType, data));
}

InterfaceWithSwitch::InterfaceWithSwitch(std::shared_ptr<DataProcessing> dataProcessingInstance) {
	// Start up socket
	zed_net_init();
	// Open up one
	zed_net_udp_socket_open(&communicationSocket, port, 0);
	// Use the given class instance
	dataProcessing = dataProcessingInstance;
}

InterfaceWithSwitch::~InterfaceWithSwitch() {
	// Shutdown socket
	zed_net_shutdown();
}