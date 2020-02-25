#pragma once

#define TRY_DEQUE(queue, data) ((queue).try_dequeue(data))

// clang-format off
#define SEND_DATA(type, structData) { \
	uint8_t data; \
	uint16_t size; \
	serializingProtocol.dataToBinary<type>(structData, &data, &size); \
	serverConnection.Send(&data, size); \
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

#define SERVER_PORT 6978

class CommunicateWithSwitch {
private:
	enum DataFlag : uint8_t {
		SET_PROJECT_NAME,
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

	// Sorry, not doing that anymore, a queue for each
	moodycamel::ConcurrentQueue<Protocol_SetProjectName> Protocol_SetProjectName_Queue;

	CActiveSocket serverConnection;

	uint8_t connectedToServer;

	std::shared_ptr<std::thread> networkThread;

	std::mutex ipMutex;
	std::string ipAddressServer;

	std::condition_variable cv;

	// Whether to keep going
	std::atomic_bool keepReading;

	// Protcol for serializing
	SerializeProtocol serializingProtocol;

	void unserializeData(uint8_t* buf, uint16_t bufSize, DataFlag flag);

	bool handleSocketError(int res);

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