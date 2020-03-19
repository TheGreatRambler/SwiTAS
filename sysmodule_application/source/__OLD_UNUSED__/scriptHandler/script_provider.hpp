#pragma once

#include "script_init.hpp"
#include <istream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

class ScriptProvider {
private:
	mutable std::mutex mutex;
	std::queue<std::shared_ptr<struct controlMsg>> internalQueue;

protected:
	void pushToQueue(std::shared_ptr<struct controlMsg> msg) {
		std::lock_guard<std::mutex> guard(mutex);
		return internalQueue.push(msg);
	}

	std::shared_ptr<struct controlMsg> pullFromQueue() {
		std::lock_guard<std::mutex> guard(mutex);
		std::shared_ptr<struct controlMsg> msg = internalQueue.front();
		internalQueue.pop();
		return msg;
	}

public:
	bool queueIsEmpty() {
		std::lock_guard<std::mutex> guard(mutex);
		return internalQueue.empty();
	}
	int queueSize() {
		std::lock_guard<std::mutex> guard(mutex);
		return internalQueue.size();
	}
	virtual bool isGood() {
		return false;
	};

	virtual std::shared_ptr<struct controlMsg> nextLine();
	virtual bool hasNextLine();
	virtual void populateQueue();
};

class LineStreamScriptProvider : public ScriptProvider {
private:
	std::istream& stream;

protected:
	virtual void afterEOF() {}
	virtual bool shouldPopulate() {
		return queueSize() < 15;
	}

public:
	LineStreamScriptProvider(std::istream& mStream)
		: stream(mStream) {}

	bool isGood() {
		return stream.good();
	}

	bool hasNextLine();
	void populateQueue();
};

class LineFileScriptProvider : public LineStreamScriptProvider {
	std::ifstream stream;

protected:
	void afterEOF() {
		stream.close();
	}
	bool shouldPopulate() {
		return stream.is_open() && queueSize() < 15;
	}

public:
	LineFileScriptProvider(std::string fileName)
		: LineStreamScriptProvider(stream)
		, stream(fileName) {}
};

class PressAProvider : public ScriptProvider {
public:
	PressAProvider() {
		pushToQueue(std::make_shared<struct controlMsg>(lineAsControlMsg(1, "KEY_A", "0;0", "0;0")));
		pushToQueue(std::make_shared<struct controlMsg>(lineAsControlMsg(2, "KEY_A", "0;0", "0;0")));
	}

	bool isGood() {
		return true;
	}
};

class PressLRProvider : public ScriptProvider {
public:
	PressLRProvider() {
		pushToQueue(std::make_shared<struct controlMsg>(lineAsControlMsg(1, "KEY_L;KEY_R", "0;0", "0;0")));
		pushToQueue(std::make_shared<struct controlMsg>(lineAsControlMsg(2, "KEY_L;KEY_R", "0;0", "0;0")));
	}

	bool isGood() {
		return true;
	}
};
