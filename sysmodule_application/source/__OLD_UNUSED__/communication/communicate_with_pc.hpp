#pragma once

// Passive sockets are the server
#include "../third_party/clsocket/ActiveSocket.h"
#include "../third_party/clsocket/PassiveSocket.h"
#include "serialize_unserialize_data.hpp"

#define SERVER_PORT 6978

class CommunicateWithPC {
private:
	enum DataFlag : uint8_t {
		CREATE_SAVESTATE,
		LOAD_SAVESTATE,
		GO_TO_FRAME,
		NEW_FRAME,
	};

	CPassiveSocket server;
	CActiveSocket* client;

	uint8_t connectedToClient;

	void unserializeData(uint8_t* buf, uint16_t bufSize, DataFlag flag);

	bool handleSocketError(int res);

public:
	CommunicateWithPC();

	// Called in the main loop
	void listenForPCCommands();

	~CommunicateWithPC();
};