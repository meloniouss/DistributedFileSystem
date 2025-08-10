#pragma once
#include "Command.hpp"

class CommandParser{

public:
	CommandParser();
	Command jsonToCommand(nlohmann::json &input);
	nlohmann::json textToJson(std::string input);
	std::string serializeCommand(const Command& cmd);
	Command deserializeCommand(const std::string& jsonStr);
};
