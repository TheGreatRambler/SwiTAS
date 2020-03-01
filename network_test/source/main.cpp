#include <atomic>
#include <concurrentqueue.h>
#include <memory>
#include <queue>

#include "networking/networkInterface.hpp"

int main(int argc, char* argv[]) {
	// Starts its own thread
	CommunicateWithNetwork communicateWithPC;
	while(true) {
		// Check the queues for data and print for debugging
	}
	communicateWithPC.endNetwork();
}