#pragma once

#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>

#include "Room.h"

class Session
    : public User,
      public boost::enable_shared_from_this<Session>
{
public:

    Session(
        boost::asio::ip::udp::socket& socket,
        boost::asio::ip::udp::endpoint& remote_endpoint,
        std::string username
    ) : User(std::move(username)), socket_(socket), remote_endpoint_(remote_endpoint) {}

    void deliver(const ChatMessage& message) override {
        std::cout << message.text() << " " << username() << std::endl;
        message_queue_.push_back(message);
        while(!message_queue_.empty()) {
            send_message(message_queue_.front());
            message_queue_.pop_front();
        }
    }

    boost::shared_ptr<Room> room() const {
        return room_;
    }

    void set_room(const boost::shared_ptr<Room>& room) {
        room_ = room;
    }

private:

    void send_message(ChatMessage& message) {
        socket_.async_send_to(
            boost::asio::buffer(message.data()),
            remote_endpoint_,
            boost::bind(
                &Session::handle_send,
                this,
                message,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred
            )
        );
    }

    void handle_send(
        ChatMessage message,
        const boost::system::error_code &error,
        std::size_t bytes_transferred
    ) {
        // LOG
    }

    MessageQueue message_queue_;

    boost::shared_ptr<Room> room_;
    boost::asio::ip::udp::socket& socket_;
    boost::asio::ip::udp::endpoint remote_endpoint_;

};
