#pragma once
#include "../include/asio.hpp"
#include "../ThreadSafeQueue.hpp"
class SocketHandler{

struct Connection{
	std::shared_ptr<asio::ip::tcp::socket> socket;
	asio::streambuf read_buffer;
	ThreadSafeQueue<std::string> send_queue;	
	bool write_in_progress = false;	
	mutable std::mutex mtx;
};

public:
	void startListenServer(int port);
	void closeListenServer();
	void sendData(Connection& conn, std::string& data);
	void establishConnection(asio::ip::address ip_address, int port); // client side
	void acceptConnection(); // server-side
	void readData(Connection& currentConnection);
	void readHeader(Connection& currentConnection);
	void readBody(Connection& currentConnection, size_t data_length);
	int findConnIdForSocket(const std::shared_ptr<asio::ip::tcp::socket>& socket);
	void run();
	void doAsyncWrite(Connection& conn);
private:
	asio::ip::tcp::acceptor acceptor_;
	asio::ip::tcp::socket socket_;
	asio::io_context io_context_;
	int next_conn_id_ = 1;
	ThreadSafeQueue<std::string> global_message_queue_;
	std::unordered_map<int, Connection> connection_Map_;
};
