#include <boost/thread.hpp>
#include <boost/array.hpp>
#include <boost/asio.hpp>

#include <iostream>

#include "ChatMessage.h"

using boost::asio::ip::udp;

class Client {
public:

    Client(
        boost::asio::io_service& io_service,
        const std::string& host,
        const std::string& port
    ) : io_service_(io_service), socket_(io_service) {
        udp::resolver resolver(io_service_);
        udp::resolver::query query(udp::v4(), host, port);
        udp::resolver::iterator iterator = resolver.resolve(query);
        endpoint_ = *iterator;
        boost::asio::connect(socket_, iterator);

        startReceive();
    }

    void send(const ChatMessage& msg) {
        socket_.send(boost::asio::buffer(msg.data(), msg.size()));
    }

    ~Client() {
        socket_.close();
    }

private:

    void startReceive() {
        socket_.async_receive_from(
            boost::asio::buffer(message_.data()),
            endpoint_,
            boost::bind(
                &Client::handleReceive,
                this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred
            )
        );
    }

    void handleReceive(
        const boost::system::error_code& error,
        std::size_t bytes_transferred
    ) {
        if(!error) {
            message_.decode();
            std::cout << message_.text() << std::endl;
        }
        startReceive();
    }

    udp::socket socket_;
    udp::endpoint endpoint_;
    boost::asio::io_service& io_service_;

    ChatMessage message_;

};

int main(int argc, char* argv[]) {

    try {

        if(argc != 3) {
            std::cerr << "Use: <host> <port>" << std::endl;
        }

        boost::asio::io_service io_service;

        Client client(io_service, argv[1], argv[2]);

        boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));

        std::string input;
        while(std::getline(std::cin, input)) {
            ChatMessage msg(input);
            client.send(msg);
        }

        client.~Client();
        t.join();

    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

}