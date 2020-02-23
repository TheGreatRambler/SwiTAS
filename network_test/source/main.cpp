#include <atomic>
#include <concurrentqueue.h>
#include <memory>
#include <queue>
#include <thread>

#include "communicate_with_pc.hpp"

ConcurrentQueue<RecieveFrame> q;

void runListener() {
	CommunicateWithPC communicateWithPC;
	while(true) {
		communicateWithPC.listenForPCCommands();
	}
}

int main(int argc, char* argv[]) {
	std::thread listenerThread(&runListener);
}