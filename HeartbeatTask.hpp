#pragma once
#include "Networking/SocketHandler.hpp"
#include "Task.hpp"
class HeartbeatTask : public Task {
public:
	HeartbeatTask(SocketHandler& socketHandler, SocketHandler::Connection& connection);
	void execute() override;
private:
    SocketHandler& socketHandler_;
    SocketHandler::Connection& connection_;
};
