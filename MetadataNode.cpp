#include "MetadataNode.hpp"
#include "Networking/SocketHandler.hpp"
#include <atomic>
#include <iostream>

void MetadataNode::start(int port){
    Debug.Log("Starting server");
    _socketHandler.startListenServer(port); // get cmdline args
    _socketHandler.acceptConnection();
    running = true;
    messageConsumer = std::thread(&MetadataNode::consumeMessage, this);
    taskConsumer = std::thread(&MetadataNode::consumeTask, this);
    ioThread = std::thread([this] {_socketHandler.run();});
}

void MetadataNode::stop(){
    Debug.Log("Closing Server");
   _socketHandler.closeListenServer();
    running = false;
    _socketHandler.global_message_queue_.push(""); //empty msg = kill because wait_for_pop is blocking -- running bool never gets checked if queue is empty.
    //taskQueue.push(StopServerTask);
    canConsumeMessage.notify_all();
    canConsumeTask.notify_all();
    if(messageConsumer.joinable()) messageConsumer.join();
    else Debug.LogError("messageConsumer thread not joinable");
    if(taskConsumer.joinable()) taskConsumer.join();
    else Debug.LogError("taskConsumer thread not joinable");
    if(ioThread.joinable()) ioThread.join();
    else Debug.LogError("ioThread thread not joinable");
}

static std::string getCmdLineArgs(){
    std::string input;
    std::cout << "Enter port: ";
    std::cin >> input;
    return input;
}

void MetadataNode::consumeMessage(){
    std::string msg;
    while(running){
	_socketHandler.global_message_queue_.wait_and_pop(msg); // wait for queue to not be empty
	if (!running || msg.empty()) break;
	// deserialize, make command, then make task, and add to ask queue
	Command cmd = _commandParser.deserializeCommand(msg);
	//Task currentTask = _taskFactory.createTask(cmd);
	//_taskQueue.enqueue(currentTask);
    }
}
void MetadataNode::consumeTask(){
    // wait for task queue to not be empty
    // task.execute();
    while(running){

	//Task activeTask;
	//taskQueue.wait_and_pop(activeTask);
	//if(!running || task_type is stopservertask)
    }
}
MetadataNode::MetadataNode()
    : running(false), Debug("MetadataNode") // initialize atomic_bool
{
}


int main(){
    std::string portStr = getCmdLineArgs(); 
    int port = std::stoi(portStr);
    std::cout.flush();
    MetadataNode _MetadataNode;
    _MetadataNode.start(port);

    std::cout << "Type 'q' to exit node\n";
    std::string input;
    while (true) {
        std::getline(std::cin, input);
        if (input == "q") {
            _MetadataNode.stop();
            break;
        }
    }

    return 0;
}
