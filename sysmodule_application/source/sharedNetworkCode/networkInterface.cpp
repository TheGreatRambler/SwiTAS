#include "networkInterface.hpp"

// Decided upon using https://github.com/DFHack/clsocket

bool CommunicateWithNetwork::readData(void* data, uint32_t sizeToRead) {
	// Info about pointers here: https://stackoverflow.com/a/4318446
	// Will return true on error
	uint8_t* dataPointer     = (uint8_t*)data;
	uint32_t numOfBytesSoFar = 0;
	while(numOfBytesSoFar != sizeToRead) {
		yieldThread();
		// Have to read at the right index with the right num of bytes
		int res = networkConnection->Receive(sizeToRead - numOfBytesSoFar, &dataPointer[numOfBytesSoFar]);
		if(res == 0) {
			return true;
		} else if(res == -1) {
			// If errors are nonfatal, attempt another goaround
			// TODO limit number of tries to prevent a brick
			if(handleSocketError("during read")) {
				return true;
			}
		} else {
			// Just num of bytes
			// Now have to read this less bytes next time
			numOfBytesSoFar += res;
		}
	}
	// If here, success! Operation has encountered no error
	return false;
}

bool CommunicateWithNetwork::sendData(void* data, uint32_t sizeToSend) {
	uint8_t* dataPointer     = (uint8_t*)data;
	uint32_t numOfBytesSoFar = 0;
	while(numOfBytesSoFar != sizeToSend) {
		yieldThread();
		int res = networkConnection->Send(&dataPointer[numOfBytesSoFar], sizeToSend - numOfBytesSoFar);
		if(res == 0) {
			return true;
		} else if(res == -1) {
			// If errors are nonfatal, attempt another goaround
			// TODO limit number of tries to prevent a brick
			if(handleSocketError("during send")) {
				return true;
			}
		} else {
			numOfBytesSoFar += res;
		}
	}
	return false;
}

void CommunicateWithNetwork::handleFatalError() {
#ifdef __SWITCH__
	LOGD << "Network fataled";
#endif
#ifdef CLIENT_IMP
	wxLogMessage("Network fataled");
#endif
	connectedToSocket     = false;
	otherSideDisconnected = true;
	networkConnection->Close();
#ifdef SERVER_IMP
	waitForNetworkConnection();
#endif
#ifdef CLIENT_IMP
	networkConnection = new CActiveSocket();

	networkConnection->Initialize();
	waitForIPSelection();
#endif
	prepareNetworkConnection();
}

CommunicateWithNetwork::CommunicateWithNetwork(std::function<void(CommunicateWithNetwork*)> sendCallback, std::function<void(CommunicateWithNetwork*)> recieveCallback) {
	// Should keep reading network at the beginning
	keepReading           = true;
	connectedToSocket     = false;
	otherSideDisconnected = false;

	sendQueueDataCallback    = sendCallback;
	recieveQueueDataCallback = recieveCallback;

	// Start the thread, this means that this class goes on the main thread
	networkThread = std::make_shared<std::thread>(&CommunicateWithNetwork::initNetwork, this);
}

void CommunicateWithNetwork::initNetwork() {
#ifdef CLIENT_IMP
	// Create the socket straight off
	networkConnection = new CActiveSocket();

	networkConnection->Initialize();
#endif

#ifdef SERVER_IMP
	listeningServer.Initialize();

	listeningServer.SetBlocking();
	listeningServer.SetReceiveTimeout(SOCKET_TIMEOUT_SECONDS, SOCKET_TIMEOUT_MICROSECONDS);

	// Listen on localhost
	listeningServer.Listen(NULL, SERVER_PORT);

	waitForNetworkConnection();
#endif

#ifdef CLIENT_IMP
	waitForIPSelection();
#endif

	prepareNetworkConnection();

	// This will loop forever until keepReading is set to false
	listenForCommands();
}

#ifdef CLIENT_IMP
void CommunicateWithNetwork::waitForIPSelection() {
	{
		std::unique_lock<std::mutex> lk(ipMutex);
		while(!connectedToSocket)
			cv.wait(lk);
	}
}
#endif

void CommunicateWithNetwork::prepareNetworkConnection() {
	if(keepReading) {
		// Set to blocking for all data
		networkConnection->SetBlocking();

		// Block for 5 seconds to recieve a byte
		// Within 5 seconds
		networkConnection->SetReceiveTimeout(SOCKET_TIMEOUT_SECONDS, SOCKET_TIMEOUT_MICROSECONDS);
	}
}

#ifdef SERVER_IMP
void CommunicateWithNetwork::waitForNetworkConnection() {
	// This will block INDEFINITELY if there is no client, so
	// Literally nothing can happen until this finishes
	networkConnection = NULL;
	while(keepReading) {
		// This will block until an error or otherwise
		networkConnection = listeningServer.Accept();
		// We only care about the first connection
		if(networkConnection != NULL) {
			// Connection established, stop while looping
			connectedToSocket = true;
			break;
		} else {
			handleSocketError("during server accept attempts");
		}
		// Wait briefly
		yieldThread();
	}
}
#endif

bool CommunicateWithNetwork::hasOtherSideJustDisconnected() {
	if(otherSideDisconnected.load()) {
		otherSideDisconnected = false;
		return true;
	} else {
		return false;
	}
}

void CommunicateWithNetwork::endNetwork() {
	keepReading = false;

#ifdef CLIENT_IMP
	// This will automatically handle if the network has never been connected to
	if(!isConnected()) {
		std::unique_lock<std::mutex> lk(ipMutex);
		connectedToSocket = true;
		cv.notify_one();
	}
#endif

	// Wait for thread to end
	networkThread->join();

	if(networkConnection != nullptr) {
		networkConnection->Close();
	}

#ifdef SERVER_IMP
	listeningServer.Close();
#endif

	delete networkConnection;
}

bool CommunicateWithNetwork::handleSocketError(const char* extraMessage) {
	// Return true if it's a fatal error that should try to reconnect sockets
	//   false if there is no error
	networkConnection->TranslateSocketError();
	CSimpleSocket::CSocketError e = networkConnection->GetSocketError();
	lastError                     = e;
	// Some errors are just annoying and spam
	// clang-format off
	if (e == E::SocketSuccess
		|| e == E::SocketTimedout
		|| e == E::SocketEwouldblock
		|| e == E::SocketEinprogress
		|| e == E::SocketInterrupted) {
		// clang-format on
		return false;
	} else {
#ifdef __SWITCH__
		LOGD << std::string(networkConnection->DescribeError(e)) << " " << std::string(extraMessage);
#endif
#ifdef CLIENT_IMP
		wxLogMessage(wxString::FromUTF8(networkConnection->DescribeError(e)) + " " + extraMessage);
#endif
		// clang-format off
		if(e == E::SocketConnectionRefused
			|| e == E::SocketNotconnected
			|| e == E::SocketConnectionAborted
			|| e == E::SocketProtocolError
			|| e == E::SocketFirewallError
			|| e == E::SocketConnectionReset) {
			// clang-format on
			return true;
		} else {
			return false;
		}
	}
}

#ifdef CLIENT_IMP
uint8_t CommunicateWithNetwork::attemptConnectionToServer(std::string ip) {
	if(networkConnection->Open(ip.c_str(), SERVER_PORT)) {
		// We good, let the network thread know
		// Either connected successfully or a disconnect has been requested
		{
			std::lock_guard<std::mutex> lk(ipMutex);
			connectedToSocket = true;
		}
		cv.notify_one();
		ipAddress = ip;
		return true;
	} else {
		// There was an error
		handleSocketError("during connection attempt");
		return false;
	}
}
#endif

void CommunicateWithNetwork::listenForCommands() {
	networkError = false;

	readThread = std::make_shared<std::thread>(&CommunicateWithNetwork::readFunc, this);

	while(keepReading) {
		if(networkError) {
			handleFatalError();
			networkError = false;
		} else {
			// Send data in this thread to save on threads
			sendQueueDataCallback(this);
		}

		yieldThread();
	}

	// Stop read thread
	readThread->join();
}

void CommunicateWithNetwork::readFunc() {
	while(keepReading) {
		if(!networkError) {
			if(readData(&dataSize, sizeof(dataSize))) {
				networkError = true;
				continue;
			}

			// Get the number back to the correct representation
			// https://linux.die.net/man/3/ntohl
			dataSize = ntohl(dataSize);

			// Get the flag now, just a uint8_t, no endian conversion, I think
			if(readData(&currentFlag, sizeof(currentFlag))) {
				networkError = true;
				continue;
			}
			// Flag now tells us the data we expect to recieve

			dataToRead = (uint8_t*)malloc(dataSize);

			// The message worked, so get the data
			if(readData(dataToRead, dataSize)) {
				free(dataToRead);
				networkError = true;
				continue;
			}

			// Now, check over incoming queues, they will absorb the data if they correspond with the flag
			// Keep in mind, this is not the main thread, so can't act upon the data instantly
			recieveQueueDataCallback(this);

			// Free memory
			free(dataToRead);
		}

		yieldThread();
	}
}