#pragma once
#include <string>
#include <optional>
#include "../include/nlohmann/json.hpp"


enum class CommandType {
	ADD_FILE,
	RECEIVE_CHUNK,
	SEND_CHUNK,
	STORE_CHUNK,
	DELETE_CHUNK,
	REPLICATE_CHUNK,
	UNKNOWN,
	HEARTBEAT,
	ECHO
};

struct Command {
	CommandType cmdType;
	std::optional<std::string> senderType; //node
	std::optional<std::string> senderAddress; //ip
	std::optional<int> senderPort; //replace this with NodeIdentity
	nlohmann::json parameters;
};


