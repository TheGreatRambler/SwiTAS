#include "networkInterface.hpp"

// Decided upon using https://github.com/DFHack/clsocket

bool CommunicateWithNetwork::readData(uint8_t* data, uint16_t sizeToRead) {
	// Info about pointers here: https://stackoverflow.com/a/4318446
	// Will return true on error
	uint16_t numOfBytesSoFar = 0;
	while(numOfBytesSoFar != sizeToRead) {
		// Have to read at the right index with the right num of bytes
		int res = networkConnection->Receive(sizeToRead - numOfBytesSoFar, &data[numOfBytesSoFar]);
		if(res == 0) {
			connectedToSocket = false;
#ifdef SERVER_IMP
			waitForNetworkConnection();
#endif
			return true;
		} else if(res == -1) {
			// Socket has encountered an error
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
		}
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
			return;
		} else {
			handleSocketError(-1);
		}
		// Wait briefly
		std::this_thread::yield();
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
		if(error != CSimpleSocket::CSocketError::SocketTimedout) {
#ifdef SERVER_IMP
			LOGD << networkConnection->DescribeError(error);
#endif
#ifdef CLIENT_IMP
			wxLogMessage(networkConnection->DescribeError(error));
#endif
		}
		// TODO if the error is about the network cutting off, run waitForNetworkConnection again if server
		// If client, notify app
		return true;
	} else {
		return false;
	}
}

#ifdef CLIENT_IMP
uint8_t CommunicateWithNetwork::attemptConnectionToServer(std::string ip) {
	std::unique_lock<std::mutex> lk(ipMutex);
	if(!networkConnection->Open(ip.c_str(), SERVER_PORT)) {
		// There was an error
		connectedToSocket = false;
		return false;
	} else {
		// We good, let the network thread know
		ipAddress         = ip;
		connectedToSocket = true;
		cv.notify_one();
		return true;
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
		std::this_thread::yield();
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