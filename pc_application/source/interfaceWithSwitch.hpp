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

#include "serializeUnserializeData.hpp"

#define SERVER_PORT 6978

class CommunicateWithSwitch {
private:
	enum DataFlag : uint8_t {
		SET_GAME_INFO,
	};

	struct sockaddr_in serv_addr;

	int sockfd = 0;

	uint8_t connectedToServer;

	// Important to note, messages can be sent at ANY time
	bool sendSocketHelper(void* data, uint16_t size);

	bool readSocketHelper(void* data, uint16_t size);

	bool blockUntilReady();

	void unserializeData(uint8_t* buf, uint16_t bufSize, DataFlag flag);

	void handleSocketError();

public:
	CommunicateWithSwitch();

	void setIpAddress(char* ip);

	// Called in the main loop
	void listenForSwitchCommands();

	~CommunicateWithSwitch();
};