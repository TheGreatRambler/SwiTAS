#include "networkInterface.hpp"

// Decided upon using https://github.com/DFHack/clsocket

bool CommunicateWithNetwork::readData(void* data, uint32_t sizeToRead) {
	// Info about pointers here: https://stackoverflow.com/a/4318446
	// Will return true on error
	uint8_t* dataPointer     = (uint8_t*)data;
	uint32_t numOfBytesSoFar = 0;
	while(numOfBytesSoFar < sizeToRead) {
// Have to read at the right index with the right num of bytes
#ifdef SERVER_IMP
		LOGD << "size: " << sizeToRead << " numbytessofar: " << numOfBytesSoFar;
#endif
		int res = networkConnection->Receive(sizeToRead - numOfBytesSoFar, &dataPointer[numOfBytesSoFar]);
#ifdef SERVER_IMP
		LOGD << "read error: " << res;
#endif
		if(res == 0 || res == -1) {
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
		yieldThread();
	}
	// If here, success! Operation has encountered no error
	return false;
}

bool CommunicateWithNetwork::sendData(void* data, uint32_t sizeToSend) {
	uint8_t* dataPointer     = (uint8_t*)data;
	uint32_t numOfBytesSoFar = 0;
	while(numOfBytesSoFar < sizeToSend) {
		int res = networkConnection->Send(&dataPointer[numOfBytesSoFar], sizeToSend - numOfBytesSoFar);
		if(res == 0 || res == -1) {
			// If errors are nonfatal, attempt another goaround
			// TODO limit number of tries to prevent a brick
			if(handleSocketError("during send")) {
				return true;
			}
		} else {
			numOfBytesSoFar += res;
		}
		yieldThread();
	}
	return false;
}

void CommunicateWithNetwork::handleFatalError() {
#ifdef SERVER_IMP
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
	LOGD << "Server initialized";

	listeningServer.SetBlocking();
	listeningServer.SetReceiveTimeout(SOCKET_TIMEOUT_SECONDS);

	// Listen on localhost
	listeningServer.Listen(NULL, SERVER_PORT);
	LOGD << "Server listening";

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
	// Wait until string is good
	{
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
			yieldThread();
		}
	}
}
#endif

void CommunicateWithNetwork::prepareNetworkConnection() {
	// Set to blocking for all data
	networkConnection->SetBlocking();

	// Block for 5 seconds to recieve a byte
	// Within 5 seconds
	networkConnection->SetReceiveTimeout(SOCKET_TIMEOUT_SECONDS);
}

#ifdef SERVER_IMP
void CommunicateWithNetwork::waitForNetworkConnection() {
	// This will block INDEFINITELY if there is no client, so
	// Literally nothing can happen until this finishes
	networkConnection = NULL;
	while(true) {
		LOGD << "Waiting for connection";
		// This will block until an error or otherwise
		networkConnection = listeningServer.Accept();
		// We only care about the first connection
		if(networkConnection != NULL) {
			// Connection established, stop while looping
			LOGD << "Client connected";
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
	// Stop reading
	keepReading = false;
	// Wait for thread to end
	networkThread->join();
}

bool CommunicateWithNetwork::handleSocketError(const char* extraMessage) {
	// Return true if it's a fatal error that should try to reconnect sockets
	//   false if there is no error
	networkConnection->TranslateSocketError();
	CSimpleSocket::CSocketError e = networkConnection->GetSocketError();
#ifdef SERVER_IMP
	LOGD << "net error: " << e;
#endif
	// Some errors are just annoying and spam
	using E = CSimpleSocket::CSocketError;
	// clang-format off
	if (e == E::SocketSuccess
		|| e == E::SocketTimedout
		|| e == E::SocketEwouldblock
		|| e == E::SocketEinprogress
		|| e == E::SocketInterrupted) {
		// clang-format on
		return false;
	} else {
#ifdef SERVER_IMP
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
	if(!connectedToSocket) {
		std::unique_lock<std::mutex> lk(ipMutex);
		if(!networkConnection->Open(ip.c_str(), SERVER_PORT)) {
			// There was an error
			handleSocketError("during connection attempt");
			return false;
		} else {
			// We good, let the network thread know
			ipAddress         = ip;
			connectedToSocket = true;
			cv.notify_one();
			return true;
		}
	} else {
		// The program is already connected, need to do this differently
		return false;
	}
}
#endif

void CommunicateWithNetwork::listenForCommands() {
#ifdef SERVER_IMP
	LOGD << "Client has connected";
#endif
	// Socket connected, do things
	// The format works by preceding each message with a uint16_t with the size of the message, then the message right after it
	while(keepReading.load()) {
		// First, check over every outgoing queue to detect outgoing data
		sendQueueDataCallback(this);

		// Check if socket even has data before doing anything
		if(networkConnection->Select()) {
			uint16_t secretKey;
			if(readData(&secretKey, sizeof(secretKey))) {
				handleFatalError();
				continue;
			}

			if(ntohs(secretKey) != SECRET_PACKET_KEY) {
				// For lightweight safety, append a secret packet key to identify the start of packets
				// Don't handle fatal error
				continue;
			}

			if(readData(&dataSize, sizeof(dataSize))) {
				handleFatalError();
				continue;
			}

			// Get the number back to the correct representation
			// https://linux.die.net/man/3/ntohl
			dataSize = ntohl(dataSize);

#ifdef SERVER_IMP
			LOGD << dataSize;
#endif

			// Get the flag now, just a uint8_t, no endian conversion, I think
			if(readData(&currentFlag, sizeof(currentFlag))) {
				handleFatalError();
				continue;
			}
			// Flag now tells us the data we expect to recieve

			dataToRead = (uint8_t*)malloc(dataSize);

			// The message worked, so get the data
			if(readData(dataToRead, dataSize)) {
				free(dataToRead);
				handleFatalError();
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

CommunicateWithNetwork::~CommunicateWithNetwork() {
	if(networkConnection != nullptr) {
		networkConnection->Close();
	}

#ifdef SERVER_IMP
	listeningServer.Close();
#endif

	delete networkConnection;
}