#include "interfaceWithSwitch.hpp"

// Decided upon using https://github.com/DFHack/clsocket

void CommunicateWithSwitch::unserializeData(uint8_t* buf, uint16_t bufSize, DataFlag flag) {
	// The buffer itself
	// https://github.com/niXman/yas/blob/master/include/yas/buffers.hpp#L67
	// https://github.com/niXman/yas/blob/master/examples/one_func_cond/main.cpp
	if(flag == DataFlag::SET_GAME_INFO) {
		// Game info has been recieved
	}
	// Use serializingProtocol
	// serializingProtocol.binaryToFrame();
}

CommunicateWithSwitch::CommunicateWithSwitch() {
	// Should keep reading network
	keepReading = true;

	// Start the thread
	networkThread = std::make_shared<std::thread>(&CommunicateWithSwitch::initNetwork, this);
}

void CommunicateWithSwitch::initNetwork() {
	connectedToServer = false;
	serverConnection.Initialize();

	// Set to blocking for all data
	serverConnection.SetBlocking();

	// Block for 3 second to recieve all data
	serverConnection.SetReceiveTimeout(3);

	while(!connectedToServer) {
		// Wait until string is good
		std::unique_lock<std::mutex> lk(ipMutex);
		/* To set IP, use
			std::lock_guard<std::mutex> lk(ipMutex);
			ipAddressServer = "[WHATEVER IP]";
			lk.unlock();
			cv.notify_one();
		*/
		// Wait for IP adress to be set
		cv.wait(lk, [this] { return !ipAddressServer.empty(); });
		// Use address
		attemptConnectionToServer(ipAddressServer.c_str());
		// Unlock lock
		lk.unlock();
		// If connected to server, start booling
		if(connectedToServer) {
			break;
		}
	}

	// Loop forever while you can
	while(keepReading) {
	}
}

void CommunicateWithSwitch::endNetwork() {
	// Stop reading
	keepReading = false;
	// Wait for thread to end
	networkThread->join();
}

bool CommunicateWithSwitch::handleSocketError(int res) {
	// Return true if it's an error, fatal or wouldblock
	//   false if there is no error
	if(res == -1) {
		serverConnection.TranslateSocketError();
		CSimpleSocket::CSocketError error = serverConnection.GetSocketError();
		// It's okay if it would have blocked, just means there is no data
		if(error != CSimpleSocket::SocketEwouldblock) {
			// Would block is harmless
			puts(serverConnection.DescribeError());
		}
		return true;
	} else {
		return false;
	}
}

void CommunicateWithSwitch::attemptConnectionToServer(const char* ip) {
	if(!serverConnection.Open(ip, SERVER_PORT)) {
		// There was an error
	} else {
		connectedToServer = true;
	}
}

void CommunicateWithSwitch::listenForSwitchCommands() {
	// Socket connected, do things
	// Some info: MSG_WAITALL is needed to make sure the socket waits for the specified amount of
	// 	data, so this is changed in zed_net
	// The format works by preceding each message with a uint16_t with the size of the message, then the message right after it
	if(connectedToServer) {
		while(true) {
			// First, check over every incoming queue to detect outgoing data
			Protocol_SetProjectName structData;
			if(TRY_DEQUE(Protocol_SetProjectName_Queue, structData)) {
				SEND_DATA(Protocol_SetProjectName, structData)
			}

			uint16_t dataSize;

			// Block for all this because it's in a main loop anyway

			if(handleSocketError(serverConnection.Receive(sizeof(dataSize), (uint8_t*)&dataSize))) {
				// This breaks the while loop
				break;
			}

			// Get the number back to the correct representation
			// https://linux.die.net/man/3/ntohl
			dataSize = ntohs(dataSize);

			// Get the flag now, just a uint8_t
			DataFlag flag;
			if(handleSocketError(serverConnection.Receive(sizeof(flag), (uint8_t*)&flag))) {
				break;
			}
			// Flag now tells us the data we expect to recieve

			// The message worked, so get the data
			uint8_t* dataToRead;
			if(handleSocketError(serverConnection.Receive(dataSize, dataToRead))) {
				break;
			}

			// Have the data now, unserialize with zpp
			unserializeData(dataToRead, dataSize, flag);
			// Have the data, TODO something with it
		}
	}
}

template <typename T> void CommunicateWithSwitch::addDataToQueue(T data, DataFlag dataType) {}

CommunicateWithSwitch::~CommunicateWithSwitch() {
	serverConnection.Close();
}