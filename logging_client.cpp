#include <iostream>
#include <fstream>
#include <string>
#include <zmq.hpp>

int main() {
    zmq::context_t context(1);
    zmq::socket_t sub_socket(context, zmq::socket_type::sub);

    sub_socket.connect("tcp://benternet.pxl-ea-ict.be:24042");

    std::string filter = "log!>";
    sub_socket.setsockopt(ZMQ_SUBSCRIBE, filter.c_str(), filter.size());

    std::ofstream logfile("logs.txt", std::ios::app);  // append mode

    std::cout << "📄 Logging-client actief, schrijven naar logs.txt...\n";

    while (true) {
        zmq::message_t msg;
        sub_socket.recv(msg, zmq::recv_flags::none);
        std::string log = msg.to_string();

        std::cout << log << std::endl;

        logfile << log << std::endl;  // schrijf naar bestand
        logfile.flush(); // forceer schrijven naar disk
    }

    return 0;
}
