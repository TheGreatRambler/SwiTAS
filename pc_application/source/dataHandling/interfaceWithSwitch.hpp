#pragma once

// clang-format off
#define SEND_QUEUE_DATA(queue, type) { \
	while(true) { \
		type structData; \
		if(queue.try_dequeue(structData)) { \
			uint8_t data; \
			uint16_t size; \
			serializingProtocol.dataToBinary<type>(structData, &data, &size); \
			uint16_t dataSize = htons(size); \
			serverConnection.Send((uint8_t*) &dataSize, sizeof(dataSize)); \
			serverConnection.Send((uint8_t*) &structData.flag, sizeof(DataFlag)); \
			serverConnection.Send(&data, size); \
		} else { \
			break; \
		} \
	} \
}
// clang-format on

#include <atomic>
#include <concurrentqueue.h>
#include <condition_variable>
#include <errno.h>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <zpp.hpp>

// Active sockets are the client
#include "../thirdParty/clsocket/ActiveSocket.h"
#include "serializeUnserializeData.hpp"
#include "networkingStructures.hpp"

#define SERVER_PORT 6978

class CommunicateWithSwitch {
private:
	// Sorry, not doing that anymore, a queue for each
	moodycamel::ConcurrentQueue<Protocol::SetProjectName> SetProjectName_Queue;
	moodycamel::ConcurrentQueue<Protocol::SetCurrentFrame> SetCurrentFrame_Queue;
	moodycamel::ConcurrentQueue<Protocol::ModifyFrame> ModifyFrame_Queue;

	CActiveSocket serverConnection;

	std::atomic_bool connectedToServer;

	std::shared_ptr<std::thread> networkThread;

	std::mutex ipMutex;

	std::condition_variable cv;

	// Whether to keep going
	// this can be set by anybody and will determine if networking continues
	std::atomic_bool keepReading;

	// Protcol for serializing
	SerializeProtocol serializingProtocol;

	void unserializeData(uint8_t* buf, uint16_t bufSize, DataFlag flag);

	bool handleSocketError(int res);

	void runDataInQueue() {}

public:
	CommunicateWithSwitch();

	void attemptConnectionToServer(const char* ip);

	// Called in the network thread
	void listenForSwitchCommands();

	void initNetwork();

	void endNetwork();

	// Add ANY data to queue
	template <typename T> void addDataToQueue(T data, DataFlag dataType);

	~CommunicateWithSwitch();
};