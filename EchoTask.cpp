#include "EchoTask.hpp"
#include "Networking/SocketHandler.hpp"

EchoTask::EchoTask(Command cmd, SocketHandler& socketHandler, SocketHandler::Connection& connection)
:
command(cmd),
socketHandler_(socketHandler), 
connection_(connection)
{
}

void EchoTask::execute(){
    std::string data = ""; //make metohd that parses command data
    // take command and send the data back to it
    socketHandler_.enqueueOutgoingMessage(connection_, data);
}
