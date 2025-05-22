#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include <zmq.hpp>

int main() {
    zmq::context_t context(1);
    zmq::socket_t push_socket(context, zmq::socket_type::push);
    push_socket.connect("tcp://benternet.pxl-ea-ict.be:24041");

    const std::string topic = "weer!>";

    std::cout << "🌤️ Weer-service actief...\n";

    while (true) {
        std::ifstream file("weerinfo.txt");
        std::string line;
        if (file && std::getline(file, line)) {
            std::string bericht = topic + line;
            push_socket.send(zmq::buffer(bericht), zmq::send_flags::none);
            std::cout << "📤 Verzonden naar clients: " << bericht << std::endl;
        } else {
            std::cerr << "⚠️  Kon bestand niet openen of lezen\n";
        }

        std::this_thread::sleep_for(std::chrono::seconds(10));  // elke 10 seconden
    }

    return 0;
}
