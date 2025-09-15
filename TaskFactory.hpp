#pragma once
#include <cstddef>
#include <memory>
#include "CommandParsing/CommandParser.hpp"
#include "EchoTask.hpp"
#include "Networking/SocketHandler.hpp"
#include "Tasks/Task.hpp"
#include "CommandParsing/Command.hpp"
class TaskFactory {
public:
    static std::unique_ptr<Task> createTask(Command& cmd, SocketHandler& socketHandler, SocketHandler::Connection& connection){
	auto cmdType = cmd.cmdType;
	if(cmdType == CommandType::ECHO){
	    return std::make_unique<EchoTask>(cmd,socketHandler,connection);	    	
	}
	return nullptr;
    }
private:
};

