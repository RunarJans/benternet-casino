#include <string>
#include <iostream>
#include <random>
#include <zmq.hpp>

int main() {
    zmq::context_t context(1);

    zmq::socket_t sub_socket(context, zmq::socket_type::sub);
    zmq::socket_t push_socket(context, zmq::socket_type::push);
    zmq::socket_t forward_socket(context, zmq::socket_type::push);

    sub_socket.connect("tcp://benternet.pxl-ea-ict.be:24042");
    push_socket.connect("tcp://benternet.pxl-ea-ict.be:24041");
    forward_socket.connect("tcp://benternet.pxl-ea-ict.be:24041");

    std::string sub_topic = "dobbelsteen?>";
    sub_socket.setsockopt(ZMQ_SUBSCRIBE, sub_topic.c_str(), sub_topic.length());

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 6);

    std::cout << "🎲 Dobbelsteen-service actief op Benternet..." << std::endl;

    while (true) {
        zmq::message_t request;
        sub_socket.recv(request, zmq::recv_flags::none);
        std::string received_msg = request.to_string();

        if (received_msg.find(">") == std::string::npos) {
            std::cerr << "[⚠️  FOUT] Ongeldig verzoek ontvangen: " << received_msg << std::endl;
            continue;
        }

        std::string user = received_msg.substr(sub_topic.length());
        int roll = distrib(gen);

        std::string response = "dobbelsteen!>" + user + ">" + std::to_string(roll);
        push_socket.send(zmq::buffer(response), zmq::send_flags::none);
        forward_socket.send(zmq::buffer(response), zmq::send_flags::none);

        std::cout << "✅ Gegooid voor " << user << ": " << roll << std::endl;
    }

    return 0;
}
