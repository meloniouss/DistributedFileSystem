#pragma once
#include "../include/asio.hpp"
#include "../ThreadSafeQueue.hpp"
class SocketHandler{
public:
	struct Connection{
	std::shared_ptr<asio::ip::tcp::socket> socket;
	asio::streambuf read_buffer;
	ThreadSafeQueue<std::string> send_queue;	
	bool write_in_progress = false;	
	mutable std::mutex mtx;

	    Connection() = default;
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    Connection(Connection&&) = delete;
    Connection& operator=(Connection&&) = delete;
};
	void startListenServer(int port);
	void closeListenServer();
	void enqueueOutgoingMessage(Connection& conn, std::string& data);
	void establishConnection(asio::ip::address ip_address, int port); // client side
	void acceptConnection(); // server-side
	void readData(Connection& currentConnection);
	void readHeader(Connection& currentConnection);
	void readBody(Connection& currentConnection, size_t data_length);
	int findConnIdForSocket(const std::shared_ptr<asio::ip::tcp::socket>& socket);
	void run();
	ThreadSafeQueue<std::string> global_message_queue_;
	void doAsyncWrite(Connection& conn);
	SocketHandler();
private:
	asio::io_context io_context_;
	asio::ip::tcp::socket socket_;
	asio::ip::tcp::acceptor acceptor_;
	int next_conn_id_ = 1;
	std::unordered_map<int, std::unique_ptr<Connection>> connection_Map_;
};
