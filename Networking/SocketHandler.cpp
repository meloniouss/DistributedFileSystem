#include <istream>
#include <iostream>
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
	//to-do!
	//this is client-sided code -> connecting to other nodes
}


void SocketHandler::acceptConnection(){


	//to-do : modify to accept use of Connection struct


	auto new_socket = std::make_shared<tcp::socket>(io_context_);
	acceptor_.async_accept(*new_socket, [this, new_socket](std::error_code ec){
		if(!ec){
			active_connections_.insert({next_conn_id_, new_socket});
			connection_buffers_[next_conn_id_] = std::make_unique<asio::streambuf>();
			next_conn_id_++;
		}
		acceptConnection();
	});
}
void SocketHandler::sendData(std::shared_ptr<tcp::socket> socket, const std::string& data){
	uint32_t data_size = data.size();  
	std::ostringstream text_buffer;
	text_buffer << data_size <<"\r"<< data; // \r is delimiter
}

void SocketHandler::readData(std::shared_ptr<tcp::socket> socket){// this function will pass the message into the message queue}
	asio::async_read_until(*socket, _, "\r", [this,socket](std::error_code ec, size_t bytes_transferred){
		//after async read finishes
		if(!ec){
			std::istringstream is(_read_buffer_);
			std::string istream_data_length;
			std::getline(is, istream_data_length, '\r');
			uint32_t data_length = 0;
			try{
				data_length = std::stoi(istream_data_length);
			}
			catch(std::exception &e){std::cerr << "Error: (check async reading logic)" << e.what(); return;}
		
			// call readBody function now (async read data_length bytes)

		}
		else{
			std::cerr << "Error: Failed to header (check readData() function).";
		}
	});
}

