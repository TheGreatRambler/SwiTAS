#pragma once

#include "script_provider.hpp"
#include <memory>
#include <mutex>
#include <thread>

void populateLoop(void* _);
void startPopulatorThread();
void pushProvider(std::shared_ptr<ScriptProvider> provider);
std::shared_ptr<ScriptProvider> pullProvider();
bool queueEmpty();
