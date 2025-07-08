#pragma once
#include "../include/asio.hpp"

class SocketHandler{

struct Connection{
	int conn_id;
	const std::shared_ptr<asio::ip::tcp::socket> socket;
	asio::streambuf read_buffer;
};

public:
	void startListenServer(int port);
	void closeListenServer();
	void sendData(std::shared_ptr<asio::ip::tcp::socket> socket, const std::string& data);
	void establishConnection(asio::ip::address ip_address, int port); // client side
	void acceptConnection(); // server-side
	void readData(std::shared_ptr<asio::ip::tcp::socket> socket);
	std::unordered_map<int, std::shared_ptr<asio::ip::tcp::socket>> active_connections_;
	int findConnIdForSocket(const std::shared_ptr<asio::ip::tcp::socket>& socket);
	void run();
private:
	asio::ip::tcp::acceptor acceptor_;
	asio::ip::tcp::socket socket_;
	asio::io_context io_context_;
	int next_conn_id_ = 1;
	std::unordered_map<int, std::unique_ptr<asio::streambuf>> connection_buffers_;
};
