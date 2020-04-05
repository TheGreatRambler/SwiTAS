#include "networkInterface.hpp"

// Decided upon using https://github.com/DFHack/clsocket

bool CommunicateWithNetwork::readData(uint8_t* data, uint16_t sizeToRead) {
	// Info about pointers here: https://stackoverflow.com/a/4318446
	// Will return true on error
	uint16_t numOfBytesSoFar = 0;
	while(numOfBytesSoFar != sizeToRead) {
		// Have to read at the right index with the right num of bytes
		int res = networkConnection->Receive(sizeToRead - numOfBytesSoFar, &data[numOfBytesSoFar]);
		if(res == 0 || res == -1) {
			// Socket has either been shutdown on other side
			// Or has encountered an error
			handleSocketError(res);
			return true;
		} else {
			// Just num of bytes
			// Now have to read this less bytes next time
			numOfBytesSoFar += res;
		}
	}
	// If here, success! Operation has encountered no error
	return false;
}

CommunicateWithNetwork::CommunicateWithNetwork(std::function<void(CommunicateWithNetwork*)> sendCallback, std::function<void(CommunicateWithNetwork*)> recieveCallback) {
	// Should keep reading network at the beginning
	keepReading = true;

	sendQueueDataCallback    = sendCallback;
	recieveQueueDataCallback = recieveCallback;

	// Start the thread, this means that this class goes on the main thread
	networkThread = std::make_shared<std::thread>(&CommunicateWithNetwork::initNetwork, this);
}

void CommunicateWithNetwork::initNetwork() {
	connectedToSocket = false;

#ifdef CLIENT_IMP
	// Create the socket straight off
	networkConnection = new CActiveSocket();

	networkConnection->Initialize();
#endif

#ifdef SERVER_IMP
	listeningServer.Initialize();
	LOGD << "Server initialized";

	listeningServer.SetBlocking();

	// Listen on localhost
	listeningServer.Listen(NULL, SERVER_PORT);
	LOGD << "Server listening";

	waitForNetworkConnection();
#endif

	prepareNetworkConnection();

#ifdef CLIENT_IMP
	// Wait until string is good
	std::unique_lock<std::mutex> lk(ipMutex);
	/* To set IP, use
		std::lock_guard<std::mutex> lk(ipMutex);
		ipAddressServer = "[WHATEVER IP]";
		lk.unlock();
		cv.notify_one();
	*/
	// Wait for IP address of server to be set
	// http://www.cplusplus.com/reference/condition_variable/condition_variable/
	while(!connectedToSocket.load()) {
		cv.wait(lk);
	}

#endif

	// This will loop forever until keepReading is set to false
	listenForCommands();
}

void CommunicateWithNetwork::prepareNetworkConnection() {
	// Set to blocking for all data
	networkConnection->SetBlocking();

	// Block for 3 seconds to recieve a byte
	// Within 3 seconds
	networkConnection->SetReceiveTimeout(5);
}

#ifdef SERVER_IMP
void CommunicateWithNetwork::waitForNetworkConnection() {
	// This will block INDEFINITELY if there is no client, so
	// Literally nothing can happen until this finishes
	while(true) {
		LOGD << "Waiting for connection";
		// This will block until an error or otherwise
		networkConnection = listeningServer.Accept();
		// We only care about the first connection
		if(networkConnection != NULL) {
			// Connection established, stop while looping
			connectedToSocket = true;
			LOGD << "Client connected";
			break;
		} else {
			handleSocketError(listeningServer.GetSocketError());
		}
		// Wait briefly
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
#endif

void CommunicateWithNetwork::endNetwork() {
	// Stop reading
	keepReading = false;
	// Wait for thread to end
	networkThread->join();
}

bool CommunicateWithNetwork::handleSocketError(int res) {
	// Return true if it's an error, fatal or wouldblock
	//   false if there is no error
	if(res == -1) {
		networkConnection->TranslateSocketError();
		CSimpleSocket::CSocketError error = networkConnection->GetSocketError();
		// It's okay if it would have blocked, just means there is no data
		if(error == CSimpleSocket::SocketConnectionReset) {
			// Abnormal disconnect, go through reconnecting
			// More errors may need to be checked here
			connectedToSocket = false;
#ifdef SERVER_IMP
			networkConnection->Close();
			delete networkConnection;
			// The server will just block while listening for a new connection
			waitForNetworkConnection();
			// When done, reconfigure
			prepareNetworkConnection();
#endif
#ifdef CLIENT_IMP
			// Try to reconnect probably 10 times
			for(uint8_t i = 0; i < 10; i++) {
				// Attempt to reconnect to the same IP address
				attemptConnectionToServer(ipAddress);
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
			// I dunno what to do if failure
#endif
		} else if(error != CSimpleSocket::SocketEwouldblock) {
#ifdef SERVER_IMP
			LOGD << networkConnection->DescribeError();
#endif
#ifdef CLIENT_IMP
			wxLogMessage(networkConnection->DescribeError());
#endif
		}
		return true;
	} else {
		return false;
	}
}

#ifdef CLIENT_IMP
void CommunicateWithNetwork::attemptConnectionToServer(std::string ip) {
	std::unique_lock<std::mutex> lk(ipMutex);
	if(!networkConnection->Open(ip.c_str(), SERVER_PORT)) {
		// There was an error
		connectedToSocket = false;
	} else {
		// We good, let the network thread know
		ipAddress         = ip;
		connectedToSocket = true;
		cv.notify_one();
	}
}
#endif

void CommunicateWithNetwork::listenForCommands() {
#ifdef SERVER_IMP
	LOGD << "Client has connected";
#endif
	// Socket connected, do things
	// Some info: MSG_WAITALL is needed to make sure the socket waits for the specified amount of
	// 	data, so this is changed in zed_net
	// The format works by preceding each message with a uint16_t with the size of the message, then the message right after it
	while(keepReading.load()) {
		// First, check over every outgoing queue to detect outgoing data
		sendQueueDataCallback(this);

		// Check if socket even has data before doing anything
		if(networkConnection->Select()) {
			if(readData((uint8_t*)&dataSize, sizeof(dataSize))) {
				continue;
			}

			// Get the number back to the correct representation
			// https://linux.die.net/man/3/ntohl
			dataSize = ntohs(dataSize);

			// Get the flag now, just a uint8_t, no endian conversion, I think
			if(readData((uint8_t*)&currentFlag, sizeof(currentFlag))) {
				continue;
			}
			// Flag now tells us the data we expect to recieve

			dataToRead = (uint8_t*)malloc(dataSize);

			// The message worked, so get the data
			if(readData(dataToRead, dataSize)) {
				continue;
			}

			// Now, check over incoming queues, they will absorb the data if they correspond with the flag
			// Keep in mind, this is not the main thread, so can't act upon the data instantly
			recieveQueueDataCallback(this);

			// Free memory
			free(dataToRead);
		}
	}
}

CommunicateWithNetwork::~CommunicateWithNetwork() {
	if(networkConnection != nullptr) {
		networkConnection->Close();
	}

#ifdef SERVER_IMP
	listeningServer.Close();
#endif

	delete networkConnection;
}