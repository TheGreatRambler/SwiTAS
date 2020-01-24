#pragma once

// For sockets
#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zpp.hpp>

// Active sockets are the client
#include "../thirdParty/clsocket/ActiveSocket.h"
#include "serializeUnserializeData.hpp"

#define SERVER_PORT 6978

class CommunicateWithSwitch {
private:
	enum DataFlag : uint8_t {
		SET_GAME_INFO,
	};

	CActiveSocket serverConnection;

	uint8_t connectedToServer;

	void unserializeData(uint8_t* buf, uint16_t bufSize, DataFlag flag);

	bool handleSocketError(int res);

public:
	CommunicateWithSwitch();

	void attemptConnectionToServer(char* ip);

	// Called in the main loop
	void listenForSwitchCommands();

	~CommunicateWithSwitch();
};