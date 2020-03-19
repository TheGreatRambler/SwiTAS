#include "script_provider.hpp"
#include <cstdlib>
#include <mutex>
#include <thread>

#include "script_populator.hpp"

/* mutable */ std::mutex mutex;
std::queue<std::shared_ptr<ScriptProvider>> queue;

void populateLoop(void* _) {
	while(true) {
		if(!queueEmpty()) {
			auto provider = pullProvider();
			if(provider->queueIsEmpty()) {
				provider->populateQueue();
			}
			provider->populateQueue();
		}
		svcSleepThread(6250000);
	}
}

void startPopulatorThread() {
	Thread thread;
	Result rc = threadCreate(&thread, populateLoop, NULL, NULL, 0x4000, 0x2B, -2);
	if(R_FAILED(rc))
		fatalThrow(rc);
	rc = threadStart(&thread);
	if(R_FAILED(rc))
		fatalThrow(rc);
}

void pushProvider(std::shared_ptr<ScriptProvider> provider) {
	std::lock_guard<std::mutex> guard(mutex);
	queue.push(provider);
}

std::shared_ptr<ScriptProvider> pullProvider() {
	std::lock_guard<std::mutex> guard(mutex);
	auto provider = queue.front();
	queue.pop();
	return provider;
}

bool queueEmpty() {
	std::lock_guard<std::mutex> guard(mutex);
	return queue.empty();
}
