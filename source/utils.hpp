#pragma once

#include <ctime>
#include <thread>
#include <chrono>

#include <switch.h>

float millisecondsSinceEpoch() {
	struct timeval tv;
	gettimeofday(&tv, NULL);

	// This should be good enough
	return (float) tv.tv_sec * 1000 + (float) tv.tv_usec / 1000;
}

// Sleep a thread for a specified number of milliseconds
void sleepMs(int milliseconds) {
	struct timespec ts;
	ts.tv_sec = milliseconds / 1000;
	ts.tv_nsec = (milliseconds % 1000) * 1000000;
	nanosleep(&ts, NULL);
}

// Call a function after a specified amount of time
void setTimeout(auto function, int delay) {
	std::thread t([=]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(delay));
		function();
	});
	t.detach();
}