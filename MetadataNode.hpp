#pragma once
#include "./Networking/SocketHandler.hpp"
#include "ThreadSafeQueue.hpp"
#include "CommandParsing/Command.hpp"
#include "CommandParsing/CommandParser.hpp"
#include "Tasks/Task.hpp"
#include "Logger.hpp"
class MetadataNode{
public:
	//temp methods not planned out yet, likely don't need much threading
	//maybe allow for user input to connect and stuff or use cmdline args
	void start(int port);
	void stop();
	MetadataNode();
	void consumeMessage();
	void consumeTask();
	std::condition_variable canConsumeMessage;
	std::condition_variable canConsumeTask;
	std::mutex messageConsumeLock;
	std::mutex taskConsumeLock;
	std::unordered_map<int, std::unique_ptr<SocketHandler::Connection>> connection_map; // make threadsafe, use mutex
	ThreadSafeQueue<Command> messageQueue;
	ThreadSafeQueue<Task> taskQueue;
	std::thread messageConsumer;
	std::thread taskConsumer;
	std::thread ioThread;
	Logger Debug;
private:
	SocketHandler _socketHandler;
	CommandParser _commandParser;
	std::atomic_bool running;
};
