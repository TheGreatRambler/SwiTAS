#include "communicate_with_pc.hpp"

// Decided upon using https://github.com/DFHack/clsocket

void CommunicateWithPC::unserializeData(uint8_t* buf, uint16_t bufSize, DataFlag flag) {
	// The buffer itself
	// https://github.com/niXman/yas/blob/master/include/yas/buffers.hpp#L67
	// https://github.com/niXman/yas/blob/master/examples/one_func_cond/main.cpp
	if(flag == DataFlag::CREATE_SAVESTATE) {
		// Create savestate
	} else if(flag == DataFlag::LOAD_SAVESTATE) {
		// Load savestate
	} else if(flag == DataFlag::GO_TO_FRAME) {
		// Go to the specified frame
	} else if(flag == DataFlag::NEW_FRAME) {
		// Create a new frame at the end
	}
}

CommunicateWithPC::CommunicateWithPC() {
	connectedToClient = false;
	server.Initialize();

	// Set to blocking for all data
	server.SetBlocking();

	// Block for 3 second to recieve all data
	server.SetReceiveTimeout(3);

	server.Listen("127.0.0.1", SERVER_PORT);
}

bool CommunicateWithPC::handleSocketError(int res) {
	// Return true if it's an error, fatal or wouldblock
	//   false if there is no error
	if(res == -1) {
		client->TranslateSocketError();
		CSimpleSocket::CSocketError error = client->GetSocketError();
		// It's okay if it would have blocked, just means there is no data
		if(error != CSimpleSocket::SocketEwouldblock) {
			// Would block is harmless
			puts(client->DescribeError());
		}
		return true;
	} else {
		return false;
	}
}

void CommunicateWithPC::listenForPCCommands() {
	// Socket connected, do things
	// Some info: MSG_WAITALL is needed to make sure the socket waits for the specified amount of
	// 	data, so this is changed in zed_net
	// The format works by preceding each message with a uint16_t with the size of the message, then the message right after it
	if(connectedToClient) {
		while(true) {
			uint16_t dataSize;

			// Block for all this because it's in a main loop anyway

			if(handleSocketError(client->Receive(sizeof(dataSize), (uint8_t*)&dataSize))) {
				// This breaks the while loop
				break;
			}

			// Get the number back to the correct representation
			// https://linux.die.net/man/3/ntohl
			dataSize = ntohs(dataSize);

			// Get the flag now, just a uint8_t
			DataFlag flag;
			if(handleSocketError(client->Receive(sizeof(flag), (uint8_t*)&flag))) {
				break;
			}
			// Flag now tells us the data we expect to recieve

			// The message worked, so get the data
			uint8_t* dataToRead;
			if(handleSocketError(client->Receive(dataSize, dataToRead))) {
				break;
			}

			// Have the data now, unserialize with zpp
			unserializeData(dataToRead, dataSize, flag);
			// Have the data, TODO something with it
		}
	} else {
		// Accept new clients
		client = server.Accept();
		if(client != NULL) {
			connectedToClient = true;
		}
	}
}

CommunicateWithPC::~CommunicateWithPC() {
	if(connectedToClient) {
		delete client;
	}
	server.Close();
}