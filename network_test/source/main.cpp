#include <atomic>
#include <concurrentqueue.h>
#include <cstdio>
#include <memory>
#include <queue>

#include "networking/networkInterface.hpp"

int main(int argc, char* argv[]) {
	puts("Starting network test");
	// Starts its own thread
	CommunicateWithNetwork* communicateWithPC = new CommunicateWithNetwork();
	uint8_t keepGoing                         = true;
	while(keepGoing) {
		if(communicateWithPC->isConnected()) {
			// Check the queues for data and print for debugging
			CHECK_QUEUE(communicateWithPC, ModifyFrame, {
				printf("%d", data.frame);
				// cool
			})

			CHECK_QUEUE(communicateWithPC, SetProjectName, {
				puts(data.name.c_str());
				keepGoing = false;
				puts("We done bois");
				// cool
			})

			CHECK_QUEUE(communicateWithPC, SetCurrentFrame, {
				printf("%d", data.frame);
				// cool
			})
		}
	}
	communicateWithPC->endNetwork();
}