#include "HeartbeatTask.hpp"
#include "Networking/SocketHandler.hpp"
HeartbeatTask::HeartbeatTask(SocketHandler& socketHandler, SocketHandler::Connection& connection)
:
socketHandler_(socketHandler),
connection_(connection){}
void HeartbeatTask::execute(){
	std::string data = "";
	//TO-DO: must generate json of heartbeat then serialize
	socketHandler_.enqueueOutgoingMessage(connection_,data);
}
