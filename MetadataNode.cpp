#include "MetadataNode.hpp"
#include "Networking/SocketHandler.hpp"
#include "TaskFactory.hpp"
#include <atomic>
#include <iostream>

void MetadataNode::start(int port){
    Debug.Log("Starting server");
    socketHandler.startListenServer(port); // get cmdline args
    socketHandler.acceptConnection();
    running = true;
    messageConsumer = std::thread(&MetadataNode::consumeMessage, this);
    taskConsumer = std::thread(&MetadataNode::consumeTask, this);
    ioThread = std::thread([this] {socketHandler.run();});
}

void MetadataNode::stop(){
    Debug.Log("Closing Server");
   socketHandler.closeListenServer();
    running = false;
    
    socketHandler.global_message_queue_.push(""); //empty msg = kill because wait_for_pop is blocking -- running bool never gets checked if queue is empty.
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
    SocketHandler::ReceivedMessage msg;
    while(running){
	socketHandler.global_message_queue_.wait_and_pop(msg); // wait for queue to not be empty
	if (!running || msg.data.empty()) break;
	Command cmd = commandParser.deserializeCommand(msg.data);
	auto currentTask = TaskFactory::createTask(cmd, socketHandler, *socketHandler.connection_Map_[msg.connId]); // CHANGE THIS TO nodeUuid map once added.
	taskQueue.push(std::move(currentTask));
    }
}

// when we receive register_node command, get the node-uuid from it. 
// then establish the mapping.
// 

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
