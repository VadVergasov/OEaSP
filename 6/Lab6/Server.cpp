#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <iostream>
#include <map>
#include <set>
#include <unordered_map>

#include "Session.h"

using boost::asio::ip::udp;

class Server {
   public:
    Server(boost::asio::io_service &io_service, unsigned short port)
        : io_service_(io_service),
          socket_(io_service, udp::endpoint(udp::v4(), port)) {
        startReceive();
    }

   private:
    void startReceive() {
        socket_.async_receive_from(
            boost::asio::buffer(chat_message_.data()), remote_endpoint_,
            boost::bind(&Server::handleReceive, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
    }

    void handleReceive(const boost::system::error_code &error,
                       std::size_t bytes_transferred) {
        if (!error || error == boost::asio::error::message_size) {
            chat_message_.decode();

            std::cout << to_string(chat_message_.type()) << std::endl;

            if (!users_.count(remote_endpoint_) &&
                chat_message_.type() != MessageType::CONNECT) {
                handleNonJoinedUser();
            } else {
                switch (chat_message_.type()) {
                    case MessageType::CONNECT:
                        handleConnect();
                        break;
                    case MessageType::COMMAND:
                        handleCommand();
                        break;
                    case MessageType::MESSAGE:
                        handleMessage();
                        break;
                    default:
                        break;
                }
            }
        }

        startReceive();
    }

    void handleNonJoinedUser() {
        Session session(socket_, remote_endpoint_, "SERVER");
        session.deliver(ChatMessage("Need connect to some room."));
    }

    void handleConnect() {
        std::vector<std::string> splitted_message =
            chat_message_.splitted_text();
        if (splitted_message.size() == 2) {
            std::string username = splitted_message[1];
            if (users_.count(remote_endpoint_)) {
                Session session(socket_, remote_endpoint_, "SERVER");
                session.deliver(ChatMessage("Login is already exists."));
            } else {
                users_[remote_endpoint_] = boost::make_shared<Session>(
                    socket_, remote_endpoint_, username);
                users_[remote_endpoint_]->deliver(
                    ChatMessage("Welcome to chat."));
            }
        } else {
            Session session(socket_, remote_endpoint_, "SERVER");
            session.deliver(ChatMessage("Invalid command form."));
        }
    }

    void handleCommand() {
        const auto session = users_[remote_endpoint_];
        const std::string username = session->username();
        const std::string command = chat_message_.text();

        if (command == "/users_list") {
            session->deliver(ChatMessage(users_list(username)));
        } else if (command == "/rooms_list") {
            session->deliver(ChatMessage(rooms_list()));
        } else {
            std::pair<bool, std::string> result;

            if (boost::algorithm::istarts_with(command, "/create_room")) {
                result = create_room(username);
            } else if (boost::algorithm::istarts_with(command, "/join_room")) {
                result = join_room(username);
            } else if (boost::algorithm::istarts_with(command, "/leave_room")) {
                result = leave_room(username);
            }
            session->deliver(ChatMessage(result.second));
        }
    }

    void handleMessage() {
        const auto &session = users_[remote_endpoint_];
        const auto &room = session->room();
        const auto &username = session->username();

        if (room) {
            room->deliver(ChatMessage("[" + session->username() +
                                      "]: " + chat_message_.text()),
                          username);
        }
    }

    std::string users_list(const std::string &username) {
        std::string users =
            "Users number: " + std::to_string(users_.size()) + "\n";
        std::for_each(begin(users_), end(users_),
                      [&](const std::pair<udp::endpoint,
                                          boost::shared_ptr<Session>> &pair) {
                          const auto &[_, session] = pair;
                          const auto &username_ = session->username();
                          if (username_ != username) {
                              users += username_ + '\n';
                          }
                      });
        return users;
    }

    std::string rooms_list() {
        std::string rooms;
        std::for_each(
            begin(rooms_), end(rooms_),
            [&](const std::pair<std::string, boost::shared_ptr<Room>> &pair) {
                const auto &[room_name, room] = pair;
                if (room->is_public()) {
                    rooms += room_name + '\n';
                }
            });
        return rooms;
    }

    std::pair<bool, std::string> create_room(const std::string &username) {
        std::vector<std::string> splitted_message =
            chat_message_.splitted_text();

        if (splitted_message.size() != 3) {
            return {0, "Bad command"};
        }

        const std::string room_name = splitted_message[1];
        const std::string room_password = splitted_message[2];

        if (rooms_.count(room_name)) {
            return {0, "Room with name = [" + room_name + "] already exists."};
        }

        rooms_[room_name] = boost::make_shared<Room>(room_name, room_password);
        rooms_[room_name]->join(users_[remote_endpoint_]);
        users_[remote_endpoint_]->set_room(rooms_[room_name]);

        return {1, "OK"};
    }

    std::pair<bool, std::string> join_room(const std::string &username) {
        std::vector<std::string> splitted_message =
            chat_message_.splitted_text();

        if (splitted_message.size() != 3) {
            return {0, "Bad command"};
        }

        const std::string room_name = splitted_message[1];
        const std::string room_password = splitted_message[2];

        const auto room_iterator = rooms_.find(room_name);

        if (room_iterator == rooms_.end()) {
            return {0, "Wrong room name"};
        }

        const auto room = room_iterator->second;

        if (!room->is_password_valid(room_password)) {
            return {0, "Wrong password"};
        }

        room->join(users_[remote_endpoint_]);
        users_[remote_endpoint_]->set_room(room);

        return {1, "OK"};
    }

    std::pair<bool, std::string> leave_room(const std::string &username) {
        return {};
    }

    ChatMessage chat_message_;

    std::map<udp::endpoint, boost::shared_ptr<Session>> users_;
    std::unordered_map<std::string, boost::shared_ptr<Room>> rooms_;

    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    boost::asio::io_service &io_service_;
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Specify port to use!" << std::endl;
        return 1;
    }
    try {
        boost::asio::io_service io_service;

        Server server(io_service, std::atoi(argv[1]));

        io_service.run();

    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}