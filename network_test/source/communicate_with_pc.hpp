#pragma once

// Passive sockets are the server
#include "serialize_unserialize_data.hpp"
#include "third_party/clsocket/ActiveSocket.h"
#include "third_party/clsocket/PassiveSocket.h"
#include <zpp.hpp>

#define SERVER_PORT 6978

enum Btn : uint8_t {
	A,
	B,
	X,
	Y,
	L,
	R,
	ZL,
	ZR,
	SL,
	SR,
	DUP,
	DDOWN,
	DLEFT,
	DRIGHT,
	PLUS,
	MINUS,
	HOME,
	CAPT,
	LS,
	RS,
	BUTTONS_SIZE,
};

enum DataFlag : uint8_t {
	RECIEVE_FRAME,
};

struct RecieveFrame : public zpp::serializer::polymorphic {
	DataFlag flag = DataFlag::RECIEVE_FRAME;

	uint32_t frameNum;

	Btn button;

	// Needed for zpp
	friend zpp::serializer::access;
	template <typename Archive, typename Self> static void serialize(Archive& archive, Self& self) {
		archive(self.frameNum, self.button);
	}
};

class CommunicateWithPC {
private:
	CPassiveSocket server;
	CActiveSocket* client;

	uint8_t connectedToClient;

	void unserializeData(uint8_t* buf, uint16_t bufSize, DataFlag flag);

	bool handleSocketError(int res);

public:
	CommunicateWithPC();

	// Called in the main loop
	void listenForPCCommands();

	~CommunicateWithPC();
};