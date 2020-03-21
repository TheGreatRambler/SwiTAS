#pragma once

// clang-format off
#define SEND_QUEUE_DATA(Flag) { \
	while(true) { \
		Protocol::Struct_##Flag structData; \
		if(self->Queue_##Flag.try_dequeue(structData)) { \
			uint8_t* data; \
			std::size_t size; \
			self->serializingProtocol.dataToBinary<Protocol::Struct_##Flag>(structData, &data, &size); \
			uint16_t dataSize = htons((uint16_t)size); \
			self->networkConnection->Send((uint8_t*) &dataSize, sizeof(dataSize)); \
			self->networkConnection->Send((uint8_t*) &structData.flag, sizeof(DataFlag)); \
			self->networkConnection->Send(data, size); \
			free(data); \
		} else { \
			break; \
		} \
	} \
}
// clang-format on

// clang-format off
// The data is just shoved onto the queue and wxWidgets can read it during idle or something
#define RECIEVE_QUEUE_DATA(Flag) \
	if (self->currentFlag == DataFlag::Flag) { \
		Protocol::Struct_##Flag data; \
		self->serializingProtocol.binaryToData<Protocol::Struct_##Flag>(data, self->dataToRead, self->dataSize); \
		self->Queue_##Flag.enqueue(data); \
	} \
// clang-format on

// clang-format off
#define ADD_QUEUE(Flag) moodycamel::ConcurrentQueue<Protocol::Struct_##Flag> Queue_##Flag;
// clang-format on

// Use this from other parts of the program to send data over the network
// clang-format off
#define ADD_TO_QUEUE(Flag, networkImp, bodyOfCode) { \
	Protocol::Struct_##Flag data; \
	bodyOfCode \
	networkImp->Queue_##Flag.enqueue(data); \
}
// clang-format on

// clang-format off
#define CHECK_QUEUE(networkInstance, Flag, codeBody) { \
	Protocol::Struct_##Flag data; \
	while (networkInstance->Queue_##Flag.try_dequeue(data)) { \
	 codeBody \
	} \
}
// clang-format on

#include <atomic>
#include <condition_variable>
#include <errno.h>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>
#include <thread>
#include <functional>
#include <unistd.h>
#include <unordered_map>

#include "include/zpp.hpp"
#include "include/concurrentqueue.h"

// Active sockets are the client
#ifdef SERVER_IMP
#include "thirdParty/clsocket/PassiveSocket.h"
#endif
#include "thirdParty/clsocket/ActiveSocket.h"
#include "serializeUnserializeData.hpp"
#include "networkingStructures.hpp"

#define SERVER_PORT 6978

class CommunicateWithNetwork {
private:
#ifdef SERVER_IMP
	CPassiveSocket listeningServer;
#endif

	std::atomic_bool connectedToSocket;

	std::shared_ptr<std::thread> networkThread;

	std::function<void(CommunicateWithNetwork*)> sendQueueDataCallback;
	std::function<void(CommunicateWithNetwork*)> recieveQueueDataCallback;

	std::mutex ipMutex;
	std::condition_variable cv;

#ifdef CLIENT_IMP
	std::string ipAddress;
#endif

	// Whether to keep going
	// this can be set by anybody and will determine if networking continues
	std::atomic_bool keepReading;

	bool handleSocketError(int res);

	// This will read data until all is recieved
	bool readData(uint8_t* data, uint16_t sizeToRead);

public:
	// Protcol for serializing
	SerializeProtocol serializingProtocol;
	CActiveSocket* networkConnection;

	ADD_QUEUE(SendRunFrame)
	ADD_QUEUE(RecieveGameFramebuffer)
	ADD_QUEUE(RecieveGameInfo)
	ADD_QUEUE(SendFlag)
	ADD_QUEUE(RecieveFlag)
	ADD_QUEUE(RecieveApplicationConnected)

	CommunicateWithNetwork(std::function<void(CommunicateWithNetwork*)> sendCallback, std::function<void(CommunicateWithNetwork*)> recieveCallback);

#ifdef CLIENT_IMP
	void attemptConnectionToServer(std::string ip);
#endif

#ifdef SERVER_IMP
	void waitForNetworkConnection();
#endif

	void prepareNetworkConnection();

	// Called in the network thread
	void listenForCommands();

	void initNetwork();

	void endNetwork();

	bool isConnected() {
		return connectedToSocket.load();
	}

	// This stuff needs to be global for callback reasons
	uint16_t dataSize;
	DataFlag currentFlag;
	uint8_t* dataToRead;

	~CommunicateWithNetwork();
};