#include "interfaceWithSwitch.hpp"

CommunicateWithSwitch::CommunicateWithSwitch() {
	connectedToServer = false;
	// https://gist.github.com/browny/5211329
	// THIS IS THE CLIENT
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		// Socket creation failed
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port   = htons(SERVER_PORT);

	// Set socket to be non blocking when accepting connections
	fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL) | O_NONBLOCK);

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
		handleZedNetError();
		return 1;
	}
	int res = recv(sockfd, data, size, MSG_WAITALL);
	if(res == -1) {
		// An error occured
		handleZedNetError();
		return 1;
	} else {
		// Data was recieved
		if(res == size) {
			// Data was the right size
			return 0;
		} else {
			// Data was the wrong size
			handleZedNetError();
			return 1;
		}
	}
}

void CommunicateWithSwitch::setIpAddress(char* ip) {
	if(inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
		// Inet pton error
	}

	if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		// Could not connect
	}
}

CommunicateWithSwitch::~CommunicateWithSwitch() { }