#include <iostream>
#include <string>
#include <istream>
#include <sstream>
#include "SocketHandler.hpp"
using asio::ip::tcp;

void SocketHandler::startListenServer(int port){
  acceptor_.open(asio::ip::tcp::v4());
    acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));
    acceptor_.listen();
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
	//used for connecting to metadata node (it's the central node), need to implement
}

void SocketHandler::acceptConnection(){
	auto new_socket = std::make_shared<tcp::socket>(io_context_);
	acceptor_.async_accept(*new_socket, [this, new_socket](std::error_code ec){
		if(!ec){
			auto newConnection = std::make_unique<Connection>();
			newConnection->socket = new_socket;
			connection_Map_.emplace(next_conn_id_, std::move(newConnection));
			readData(*connection_Map_[next_conn_id_]); // starts async chain of events
			next_conn_id_++;
		}
		acceptConnection(); // async_accept only handles one connection at a time, so we call acceptConnection() again to re-register the callback for the next incoming connection.
	});
}

void SocketHandler::enqueueOutgoingMessage(Connection& conn, std::string& msg){
	std::lock_guard<std::mutex> lock(conn.mtx);
	//transforms command json string into length-prefixed data to be sent
	std::ostringstream cleaned_data; cleaned_data << msg.size() << "\r" << msg;
	std::string to_send = cleaned_data.str();
	conn.send_queue.push(to_send);
	if(conn.write_in_progress) return;
	conn.write_in_progress = true;
	doAsyncWrite(conn);
}

void SocketHandler::doAsyncWrite(Connection& conn){
	std::string data;
	{std::lock_guard<std::mutex> lock(conn.mtx);
	// check if queue is empty if it is then write false, return else pop from queue and do write
	if(conn.send_queue.empty()){
		conn.write_in_progress = false;
		return;
	}
	data = conn.send_queue.front();
	conn.send_queue.pop();
	}

	asio::async_write(*conn.socket, asio::buffer(data),[this, &conn](std::error_code ec, size_t bytes_transfered){
		if(!ec){
			doAsyncWrite(conn);
		}
		else{
			conn.write_in_progress = true;
		}
	});
}

void SocketHandler::readData(Connection& currentConnection){
	readHeader(currentConnection); //kinda useless method but it looks cleaner ig
}

// network messages are length-prefixed (for now... seems kind of pointless for fixed-size chunks but whatever)
// gotta add protobuf or some other file format support (HDF5, Parquet, maybe fucking CSV)
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
			readData(currentConnection);
		}
		else{
			std::cerr << "Error: Failed to header (check readBody() function).";
		}
	});
}
SocketHandler::SocketHandler() // don't fucking touch this
    : io_context_(),
      acceptor_(io_context_),
      socket_(io_context_)
{
	std::cout << "Creating socket handler" << std::endl;
}

