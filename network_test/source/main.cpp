#include <atomic>
#include <concurrentqueue.h>
#include <cstdio>
#include <memory>
#include <queue>

#include "networking/networkInterface.hpp"

int main(int argc, char* argv[]) {
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
				// cool
			})

			CHECK_QUEUE(communicateWithPC, SetCurrentFrame, {
				keepGoing = false;
				puts("We done bois");
				// cool
			})
		}
	}
	communicateWithPC->endNetwork();
}