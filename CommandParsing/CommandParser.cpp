#include "CommandParser.hpp"
#include <sstream>
// stubs

nlohmann::json CommandParser::textToJson(std::string input){
	//make to lowercase?
    std::istringstream iss(input);
    std::string command, filename, path;
    iss >> command >> filename >> path;

    nlohmann::json result;

    if (command == "add") {
        result["cmdType"] = "ADD_FILE";
        result["file"] = filename;
        result["path"] = path;
    } else if (command == "delete") {
        result["cmdType"] = "DELETE_FILE";
        result["file"] = filename;
    } else {
        result["error"] = "unknown command";
    }

    return result;
}

CommandType parseCommandType(const std::string &input){
	if(input == "ADD_FILE") return CommandType::ADD_FILE;
	if(input == "RECEIVE_CHUNK") return CommandType::RECEIVE_CHUNK;
	if(input == "SEND_CHUNK") return CommandType::SEND_CHUNK;
	if(input == "STORE_CHUNK") return CommandType::STORE_CHUNK;
	if(input == "DELETE_CHUNK") return CommandType::DELETE_CHUNK;
	if(input == "REPLICATE_CHUNK") return CommandType::REPLICATE_CHUNK;
	if(input == "HEARTBEAT") return CommandType::HEARTBEAT;
	return CommandType::UNKNOWN;

}
Command CommandParser::jsonToCommand(nlohmann::json &input){
	Command returnCmd;
	returnCmd.cmdType = parseCommandType(input["cmdType"]);
	input.erase("cmdType");
	returnCmd.parameters = std::move(input);
	returnCmd.senderAddress = ""; //set global vars for this
	returnCmd.senderPort = std::nullopt;
	returnCmd.senderType = std::nullopt;
	return returnCmd;
}

std::string CommandParser::serializeCommand(const Command& cmd) {
        nlohmann::json j;
        j["cmdType"] = (cmd.cmdType);
        if (cmd.senderType) j["senderType"] = *cmd.senderType;
        j["senderAddress"] = cmd.senderAddress;
        if (cmd.senderPort) j["senderPort"] = *cmd.senderPort;
        j["parameters"] = cmd.parameters;
        return j.dump();
    }

Command CommandParser::deserializeCommand(const std::string& jsonStr) {
        nlohmann::json j = nlohmann::json::parse(jsonStr);
        return jsonToCommand(j);

}
