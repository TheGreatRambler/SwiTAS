#pragma once

#define FRAMEBUFFER_WIDTH 1280
#define FRAMEBUFFER_HEIGHT 720
// Used by all the framebuffer processing functions
// Has to be divisible by 4
#define DATA_BUFFER 300

#include <zpp.hpp>

struct SavestateHook {
	uint32_t startingFrame;

	// More stuff needed

	// The serializer method
	friend zpp::serializer::access;
	template <typename Archive, typename Self> static void serialize(Archive& archive, Self& self) {
		archive(self.startingFrame);
	}
};