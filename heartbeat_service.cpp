#include <iostream>
#include <string>
#include <zmq.hpp>

int main() {
zmq::context_t context(1);
zmq::socket_t sub_socket(context, zmq::socket_type::sub);


sub_socket.connect("tcp://benternet.pxl-ea-ict.be:24042");
sub_socket.setsockopt(ZMQ_SUBSCRIBE, "heartbeat?>", 11);

std::cout << "Heartbeat monitor actief...\n" << std::endl;

while (true) {
    zmq::message_t msg;
    sub_socket.recv(msg, zmq::recv_flags::none);
    std::string received = msg.to_string();

    std::string service = received.substr(11); // alles na "heartbeat?>"
    std::cout << "Heartbeat ontvangen van service: " << service << std::endl;
}

return 0;
}