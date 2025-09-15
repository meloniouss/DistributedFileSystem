#pragma once
#include "Tasks/Task.hpp"
#include "Networking/SocketHandler.hpp"
#include "CommandParsing/Command.hpp"
class EchoTask : public Task {
public:
	EchoTask(Command cmd,SocketHandler &socketHandler, SocketHandler::Connection& connection);
	void execute() override;
private:
    SocketHandler& socketHandler_;
    SocketHandler::Connection& connection_;
    Command command;
};
