#pragma once

// clang-format off
#define SEND_QUEUE_DATA(Flag) { \
	while(true) { \
		Protocol::Struct_##Flag structData; \
		if(Queue_##Flag.try_dequeue(structData)) { \
			uint8_t data; \
			uint16_t size; \
			serializingProtocol.dataToBinary<Protocol::Struct_##Flag>(structData, &data, &size); \
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

// clang-format off
// The data is just shoved onto the queue and wxWidgets can read it during idle or something
#define RECIEVE_QUEUE_DATA(Flag) \
	if (currentFlag == DataFlag::##Flag) { \
		Protocol::Struct_##Flag data = serializingProtocol.binaryToData<Protocol::Struct_##Flag>(dataToRead, dataSize); \
		Queue_##Flag.enqueue(data); \
	} \
// clang-format on

// clang-format off
#define ADD_QUEUE(Flag) moodycamel::ConcurrentQueue<Protocol::Struct_##Flag> Queue_##Flag;
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
	ADD_QUEUE(SetProjectName)
	ADD_QUEUE(SetCurrentFrame)
	ADD_QUEUE(ModifyFrame)
	ADD_QUEUE(IsPaused)

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

	// For easy access, these variables stay global
	uint8_t* dataToRead;
	uint16_t dataSize;
	DataFlag currentFlag;

	bool handleSocketError(int res);

	// This will read data until all is recieved
	bool readData(uint8_t* data, uint16_t sizeToRead);

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