#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <random>
#include <zmq.hpp>

void heartbeatLoop(zmq::socket_t& push_socket) {
    while (true) {
        std::string heartbeat = "heartbeat?>custom_dice";
        push_socket.send(zmq::buffer(heartbeat), zmq::send_flags::none);
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

int main() {
    zmq::context_t context(1);
    zmq::socket_t sub_socket(context, zmq::socket_type::sub);
    zmq::socket_t push_socket(context, zmq::socket_type::push);

    sub_socket.connect("tcp://benternet.pxl-ea-ict.be:24042");
    push_socket.connect("tcp://benternet.pxl-ea-ict.be:24041");

    sub_socket.setsockopt(ZMQ_SUBSCRIBE, "custom_dice?>", 13);

    std::thread heartbeat(heartbeatLoop, std::ref(push_socket));

    std::cout << "🎲 Custom Dice Service actief..." << std::endl;

    while (true) {
        zmq::message_t msg;
        sub_socket.recv(msg, zmq::recv_flags::none);
        std::string received = msg.to_string();

        std::string payload = received.substr(13); // na custom_dice?>
        size_t sep = payload.find('>');
        if (sep == std::string::npos) continue;

        std::string naam = payload.substr(0, sep);
        std::string sidesStr = payload.substr(sep + 1);

        if (sidesStr[0] != 'd' || sidesStr.size() < 2) {
            std::string fout = "custom_dice!>" + naam + ">Invalid format";
            push_socket.send(zmq::buffer(fout), zmq::send_flags::none);
            continue;
        }

        int sides = std::stoi(sidesStr.substr(1));
        if (sides < 1 || sides > 1000000) {
            std::string fout = "custom_dice!>" + naam + ">Invalid dice size";
            push_socket.send(zmq::buffer(fout), zmq::send_flags::none);
            continue;
        }

        int result = std::rand() % sides + 1;
        std::string reply = "custom_dice!>" + naam + ">" + sidesStr + "=" + std::to_string(result);
        push_socket.send(zmq::buffer(reply), zmq::send_flags::none);

        // 🔍 Logging toevoegen
        std::string logbericht = "log!>custom_dice>" + naam + ">gooide " + sidesStr + " = " + std::to_string(result);
        push_socket.send(zmq::buffer(logbericht), zmq::send_flags::none);

        std::cout << "[LOG] " << logbericht << std::endl;
    }

    heartbeat.join();
    return 0;
}
