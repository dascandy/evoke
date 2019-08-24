#include <iostream>
/*
#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

class Finder {
private:
    boost::asio::io_context io_context_;
    udp::socket serverSocket(io_context_, udp::endpoint(udp::v4(), 4198));
    std::thread thr_;
    std::function<void(const std::string&, const std::string&)> OnServer_;
    std::atomic<bool> exit_{false};
public:
    Finder(const std::vector<std::string>& builds, std::function<void(const std::string&, const std::string&)> OnServer) 
    : OnServer_(OnServer)
    , thr_([this](){ run(); })
    {
        serverSocket.open(udp::v4());
        udp::endpoint broadcast_endpoint{ba::ip::address_v4::broadcast(), 4198};
        serverSocket.send_to(boost::asio::buffer(buf), broadcast_endpoint);
    }
    ~Finder() {
        exit_ = true;
        thr_.join();
    }
    void run() {
        while (!exit_) {
            std::string recv_buf;
            udp::endpoint sender_endpoint;
            size_t len = serverSocket.receive_from(boost::asio::buffer(recv_buf), sender_endpoint);
            // TODO: parse buffer
            OnServer_(recv_buf.substr(0, 64), recv_buf.substr(64));
        }
    }
};
*/
