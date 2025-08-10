#pragma once
#include "./Networking/SocketHandler.hpp"
#include "ThreadSafeQueue.hpp"
#include "CommandParsing/Command.hpp"
#include "CommandParsing/CommandParser.hpp"
#include "Tasks/Task.hpp"
class MetadataNode{
public:
	//temp methods not planned out yet, likely don't need much threading
	//maybe allow for user input to connect and stuff or use cmdline args
	void start();
private:
	SocketHandler _socketHandler;
	CommandParser _commandParser;
	std::unordered_map<int, SocketHandler::Connection> connection_map; // make threadsafe, use mutex
	ThreadSafeQueue<Command> messageQueue;
	ThreadSafeQueue<Task> taskQueue;
};
