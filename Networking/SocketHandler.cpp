
#include <iostream>
#include <string>
#include <istream>
#include <sstream>
#include "SocketHandler.hpp"
using asio::ip::tcp;

void SocketHandler::startListenServer(int port){
	acceptor_ = tcp::acceptor(io_context_, tcp::endpoint(tcp::v4(), port));
	socket_ = tcp::socket(io_context_);
}
void SocketHandler::run(){
	io_context_.run();
}
void SocketHandler::closeListenServer(){
	acceptor_.close();
	socket_.close();
	io_context_.stop();
}

void SocketHandler::establishConnection(asio::ip::address ip_address,int port){
}


void SocketHandler::acceptConnection(){
	auto new_socket = std::make_shared<tcp::socket>(io_context_);
	acceptor_.async_accept(*new_socket, [this, new_socket](std::error_code ec){
		if(!ec){
			Connection newConnection;
			newConnection.socket = new_socket;
			connection_Map_.emplace(next_conn_id_, std::move(newConnection));
			next_conn_id_++;
		}
		acceptConnection();
	});
}

//transforms command json string into length-prefixed data to be sent
void SocketHandler::enqueueOutgoingMessage(Connection& conn, std::string& msg){
	std::lock_guard<std::mutex> lock(conn.mtx);
	std::ostringstream cleaned_data; cleaned_data << msg.size() << "\r" << msg;
	std::string to_send = cleaned_data.str();
	conn.send_queue.push(to_send);
	if(conn.write_in_progress) return;
	conn.write_in_progress = true;
	doAsyncWrite(conn);
}

void SocketHandler::doAsyncWrite(Connection& conn){
	//don't forget error handling (what happens if queue is empty?)
	//DON'T FORGET --> USE LOCK_GUARD && MUTEX WHEN MODIFYING STATE SUCH AS CHANGING "WRITE_IN_PROGRESS"
	//pop from queue, async::write what is inside the queue.
}

void SocketHandler::readData(Connection& currentConnection){
	readHeader(currentConnection);
}

// network messages are length-prefixed
void SocketHandler::readHeader(Connection& currentConnection){
	asio::async_read_until(*currentConnection.socket, currentConnection.read_buffer, "\r", [this, &currentConnection](std::error_code ec, size_t bytes_transferred){
		size_t data_length = 0;
		if(!ec){
			std::istream is(&currentConnection.read_buffer);
			std::string istream_data_length;
			std::getline(is, istream_data_length, '\r');
			try{
				data_length = std::stoi(istream_data_length);
			}
			catch(std::exception &e){std::cerr << "Error: (check async reading logic)" << e.what(); return;}
			currentConnection.read_buffer.consume(data_length+1); //+1 to ignore delimiter, CAREFUL WITH  THIS
			readBody(currentConnection, data_length);
		}
		else{
			std::cerr << "Error: Failed to header (check readData() function).";
		}
	});

}

void SocketHandler::readBody(Connection& currentConnection, size_t data_length){
	asio::async_read(*currentConnection.socket, currentConnection.read_buffer, asio::transfer_exactly(data_length), [this, &currentConnection, data_length](std::error_code ec2, size_t bytes_transferred){
		if(!ec2){
			std::istream body_stream(&currentConnection.read_buffer);
			std::string message_body(data_length, '\0');
			body_stream.read(&message_body[0], data_length);
			currentConnection.read_buffer.consume(data_length);
			global_message_queue_.push(message_body);
			// -> taskfactory thread will pop from queue, deserialize, jsonToCommand, then task factory will generate task and add to task queue.
			readData(currentConnection);
		}
		else{
			std::cerr << "Error: Failed to header (check readBody() function).";
		}
	});
}
