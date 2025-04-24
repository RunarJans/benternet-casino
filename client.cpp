#include <string>
#include <iostream>
#include <sstream>
#include <zmq.hpp>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Gebruik: client.exe <topic> <gebruikersnaam> [vraag]\n";
        return 1;
    }

    std::string topic = argv[1];
    std::string naam = argv[2];
    std::string inhoud = "";

    if (argc >= 4) {
        std::ostringstream oss;
        for (int i = 3; i < argc; ++i) {
            if (i > 3) oss << " ";
            oss << argv[i];
        }
        inhoud = oss.str();
    }

    zmq::context_t context{1};
    zmq::socket_t push_socket{context, zmq::socket_type::push};
    zmq::socket_t sub_socket{context, zmq::socket_type::sub};

    push_socket.connect("tcp://benternet.pxl-ea-ict.be:24041");
    sub_socket.connect("tcp://benternet.pxl-ea-ict.be:24042");

    std::string sub_topic = topic + "!>" + naam + ">";
    sub_socket.setsockopt(ZMQ_SUBSCRIBE, sub_topic.c_str(), sub_topic.length());

    std::string bericht = topic + "?>" + naam + ">";
    if (!inhoud.empty()) bericht += inhoud;

    std::cout << "Verzonden aanvraag: " << bericht << std::endl;
    push_socket.send(zmq::buffer(bericht), zmq::send_flags::none);

    zmq::message_t antwoord;
    sub_socket.recv(antwoord, zmq::recv_flags::none);
    std::cout << "Ontvangen antwoord: " << antwoord.to_string() << std::endl;

    std::cout << "\nDruk ENTER om te sluiten...";
    std::cin.get();

    return 0;
}
