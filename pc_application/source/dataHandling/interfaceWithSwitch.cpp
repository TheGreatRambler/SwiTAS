#include "interfaceWithSwitch.hpp"

// Decided upon using https://github.com/DFHack/clsocket

void CommunicateWithSwitch::unserializeData(uint8_t* buf, uint16_t bufSize, DataFlag flag) {
	// The buffer itself
	// https://github.com/niXman/yas/blob/master/include/yas/buffers.hpp#L67
	// https://github.com/niXman/yas/blob/master/examples/one_func_cond/main.cpp
	if(flag == DataFlag::SET_GAME_INFO) {
		// Game info has been recieved
	}
}

CommunicateWithSwitch::CommunicateWithSwitch() {
#ifdef __WIN32__
	WORD versionWanted = MAKEWORD(1, 1);
	WSADATA wsaData;
	WSAStartup(versionWanted, &wsaData);
#endif

	connectedToServer = false;
	// https://gist.github.com/browny/5211329
	// THIS IS THE CLIENT
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		// Socket creation failed
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port   = htons(SERVER_PORT);

// Set socket to be non blocking when accepting connections
#ifdef __WIN32__
	fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL) | O_NONBLOCK);
#else
#endif
	// This puts the options of serv_addr on the socket itself
	if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) {
		// Binding failed
	}
}

bool CommunicateWithSwitch::sendSocketHelper(void* data, uint16_t size) {
	write(sockfd, data, size);
}

bool CommunicateWithSwitch::readSocketHelper(void* data, uint16_t size) {
	// Small helper for a commonly used operation
	int count;
	// Check if socket has any data
	ioctl(sockfd, FIONREAD, &count);
	if(count == 0) {
		// No data yet
		// This shouldn't occur if waiting was done beforehand
		return 1;
	} else if(count == -1) {
		// There was an error
		handleSocketError();
		return 1;
	}
	int res = recv(sockfd, (void*)data, size, MSG_WAITALL);
	if(res == -1) {
		// An error occured
		handleSocketError();
		return 1;
	} else {
		// Data was recieved
		if(res == size) {
			// Data was the right size
			return 0;
		} else {
			// Data was the wrong size
			handleSocketError();
			return 1;
		}
	}
}

bool CommunicateWithSwitch::blockUntilReady() {
	fd_set writefd;
	timeval timeout;
	FD_ZERO(&writefd);
	FD_SET(connfd, &writefd);
	// Initialize timeout for socket blocking
	// 1 second timeout
	timeout.tv_sec  = 1;
	timeout.tv_usec = 0;
	if(select(FD_SETSIZE, &writefd, NULL, NULL, &timeout)) {
		handleSocketError();
		return 1;
	}
	return 0;
}

void CommunicateWithSwitch::setIpAddress(char* ip) {
	if(inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
		// Inet pton error
	}

	if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		// Could not connect
	}
}

void CommunicateWithSwitch::listenForSwitchCommands() {
	// Socket connected, do things
	// Some info: MSG_WAITALL is needed to make sure the socket waits for the specified amount of
	// 	data, so this is changed in zed_net
	// The format works by preceding each message with a uint16_t with the size of the message, then the message right after it
	while(true) {
		uint16_t dataSize;
		if(readSocketHelper(&dataSize, sizeof(dataSize))) {
			// Don't run, no data is avaliable at the moment
			break;
		}

		// Get the number back to the correct representation
		// https://linux.die.net/man/3/ntohl
		dataSize = ntohs(dataSize);

		// Get the flag now
		// Blocking because committed to data
		if(blockUntilReady())
			break;
		DataFlag flag;
		if(readSocketHelper(&flag, sizeof(flag)))
			break;
		// Flag now tells us the data we expect to recieve

		// The message worked, so get the data
		uint8_t* dataToRead;
		if(readSocketHelper(dataToRead, dataSize))
			break;

		// Have the data now, unserialize with YAS
		unserializeData(dataToRead, dataSize, flag);
		// Have the data, TODO something with it
	}
}

CommunicateWithSwitch::~CommunicateWithSwitch() {
#ifdef __WIN32__
	WSACleanup();
#endif
	close(sockfd);
}