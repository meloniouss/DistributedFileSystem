#include "Command.hpp"

class CommandParser{

public: 
	Command jsonToCommand(nlohmann::json input);
	nlohmann::json textToJson(std::string input);
};
