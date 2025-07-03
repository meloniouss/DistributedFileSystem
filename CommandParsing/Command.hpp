#include <string.h>
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
};

struct Command {
	CommandType cmdType;
	std::optional<std::string> senderType; //node
	std::optional<std::string> senderAddress; //ip
	std::optional<int> senderPort;
	nlohmann::json parameters;
};


