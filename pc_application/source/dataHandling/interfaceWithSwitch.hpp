#pragma once

#include <concurrentqueue.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unordered_map>
#include <zpp.hpp>

// Active sockets are the client
#include "../thirdParty/clsocket/ActiveSocket.h"
#include "serializeUnserializeData.hpp"

#define SERVER_PORT 6978

class CommunicateWithSwitch {
private:
	enum DataFlag : uint8_t {
		SET_GAME_INFO,
		NUM_OF_FLAGS,
	};

	// Map of queues
	// Instance is created, and then the pointer is shared between all threads
	// Queue is https://github.com/cameron314/concurrentqueue/blob/master/samples.md
	// Use http://www.cplusplus.com/reference/mutex/unique_lock/
	// Perhaps https://stackoverflow.com/questions/13103645/using-atomics-with-stdthread-in-c11
	// Data is added to queue with a template function from the parent thread side
	// Network thread will constantly send out data
	// Some queues are marked as outbound, others as inbound
	// PC and Switch will all have the same queues, just some will become
	// Inbound and other outbound
	std::unordered_map<DataFlag, ConcurrentQueue>* queueMap;

	CActiveSocket serverConnection;

	uint8_t connectedToServer;

	// Protcol for serializing
	SerializeProtocol serializingProtocol;

	void unserializeData(uint8_t* buf, uint16_t bufSize, DataFlag flag);

	bool handleSocketError(int res);

public:
	CommunicateWithSwitch();

	void attemptConnectionToServer(char* ip);

	// Called in the main loop
	void listenForSwitchCommands();

	// Add ANY data to queue
	template <typename T> void addDataToQueue(T data, DataFlag dataType);

	~CommunicateWithSwitch();
};