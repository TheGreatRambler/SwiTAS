#pragma once

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <zpp.hpp>

#include "serializeData.hpp"

#define SERVER_PORT 6978

class CommunicateWithSwitch {
private:
	struct sockaddr_in serv_addr;

	int sockfd = 0;

	uint8_t connectedToServer;

	bool sendSocketHelper(void* data, uint16_t size);

	bool readSocketHelper(void* data, uint16_t size);

	bool blockUntilReady();

	void unserializeData(uint8_t* buf, uint16_t bufSize, DataFlag flag);

	void handleSocketError();

public:
	CommunicateWithSwitch();

	void setIpAddress(char* ip);

	~CommunicateWithSwitch();
}