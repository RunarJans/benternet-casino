#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <map>
#include <random>
#include <ctime>
#include <zmq.hpp>

std::mutex mtx;

void heartbeatLoop(zmq::socket_t& push_socket) {
while (true) {
std::string msg = "heartbeat?>paardenrace";
push_socket.send(zmq::buffer(msg), zmq::send_flags::none);
std::this_thread::sleep_for(std::chrono::seconds(5));
}
}

int main() {
zmq::context_t context(1);


zmq::socket_t sub_socket(context, zmq::socket_type::sub);
sub_socket.connect("tcp://benternet.pxl-ea-ict.be:24042");

zmq::socket_t push_socket(context, zmq::socket_type::push);
push_socket.connect("tcp://benternet.pxl-ea-ict.be:24041");

sub_socket.setsockopt(ZMQ_SUBSCRIBE, "paardenrace?>", 13);

std::thread heartbeat(heartbeatLoop, std::ref(push_socket));

std::vector<std::string> paarden = {
    "Bliksem", "Stormhoef", "Penspony", "DustRunner", "Snelslof"
};

std::cout << "Paardenrace-service actief..." << std::endl;

while (true) {
    zmq::message_t msg;
    sub_socket.recv(msg, zmq::recv_flags::none);
    std::string received = msg.to_string();

    std::string payload = received.substr(13); // alles na paardenrace?>
    size_t sep = payload.find('>');
    if (sep == std::string::npos) continue;

    std::string naam = payload.substr(0, sep);
    std::string gokStr = payload.substr(sep + 1);

    int gok = std::stoi(gokStr);
    if (gok < 1 || gok > 5) {
        std::string fout = "paardenrace!>" + naam + ">Foutieve input";
        push_socket.send(zmq::buffer(fout), zmq::send_flags::none);
        continue;
    }

    int winnaar = std::rand() % 5 + 1;
    std::string result = "Winnaar: " + std::to_string(winnaar) + " (" + paarden[winnaar - 1] + ")";
    result += (gok == winnaar ? " - Je had juist." : " - Je had fout.");

    std::string reply = "paardenrace!>" + naam + ">" + result;
    push_socket.send(zmq::buffer(reply), zmq::send_flags::none);

    // Log naar logging service
    std::string log = "log?>paardenrace>" + naam + ">gokte op " + std::to_string(gok) + ", " + result;
    push_socket.send(zmq::buffer(log), zmq::send_flags::none);
}

heartbeat.join();
return 0;
}