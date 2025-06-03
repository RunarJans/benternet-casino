#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <random>
#include <mutex>
#include <zmq.hpp>

// Gedeelde mutex voor logging (optioneel)
std::mutex mtx;

// Custom Dice Logic
void runCustomDice(zmq::context_t& context) {
    zmq::socket_t sub(context, zmq::socket_type::sub);
    zmq::socket_t push(context, zmq::socket_type::push);
    sub.connect("tcp://benternet.pxl-ea-ict.be:24042");
    push.connect("tcp://benternet.pxl-ea-ict.be:24041");
    sub.setsockopt(ZMQ_SUBSCRIBE, "custom_dice?>", 13);

    std::cout << "🎲 Custom Dice actief...\n";

    while (true) {
        zmq::message_t msg;
        sub.recv(msg, zmq::recv_flags::none);
        std::string received = msg.to_string();

        std::string payload = received.substr(13);
        size_t sep = payload.find('>');
        if (sep == std::string::npos) continue;

        std::string naam = payload.substr(0, sep);
        std::string sidesStr = payload.substr(sep + 1);

        if (sidesStr[0] != 'd' || sidesStr.size() < 2) {
            std::string fout = "custom_dice!>" + naam + ">Invalid format";
            push.send(zmq::buffer(fout), zmq::send_flags::none);
            continue;
        }

        int sides = std::stoi(sidesStr.substr(1));
        if (sides < 1 || sides > 1000000) {
            std::string fout = "custom_dice!>" + naam + ">Invalid dice size";
            push.send(zmq::buffer(fout), zmq::send_flags::none);
            continue;
        }

        int result = std::rand() % sides + 1;
        std::string reply = "custom_dice!>" + naam + ">" + sidesStr + "=" + std::to_string(result);
        push.send(zmq::buffer(reply), zmq::send_flags::none);

        std::string log = "log!>custom_dice>" + naam + ">gooide " + sidesStr + " = " + std::to_string(result);
        push.send(zmq::buffer(log), zmq::send_flags::none);
    }
}

// Paardenrace Logic
void runPaardenrace(zmq::context_t& context) {
    zmq::socket_t sub(context, zmq::socket_type::sub);
    zmq::socket_t push(context, zmq::socket_type::push);
    sub.connect("tcp://benternet.pxl-ea-ict.be:24042");
    push.connect("tcp://benternet.pxl-ea-ict.be:24041");
    sub.setsockopt(ZMQ_SUBSCRIBE, "paardenrace?>", 13);

    std::vector<std::string> paarden = {"Bliksem", "Stormhoef", "Penspony", "DustRunner", "Snelslof"};
    std::cout << "🐎 Paardenrace actief...\n";

    while (true) {
        zmq::message_t msg;
        sub.recv(msg, zmq::recv_flags::none);
        std::string received = msg.to_string();

        std::string payload = received.substr(13);
        size_t sep = payload.find('>');
        if (sep == std::string::npos) continue;

        std::string naam = payload.substr(0, sep);
        std::string gokStr = payload.substr(sep + 1);

        int gok = std::stoi(gokStr);
        if (gok < 1 || gok > 5) {
            std::string fout = "paardenrace!>" + naam + ">Foutieve input";
            push.send(zmq::buffer(fout), zmq::send_flags::none);
            continue;
        }

        int winnaar = std::rand() % 5 + 1;
        std::string result = "Winnaar: " + std::to_string(winnaar) + " (" + paarden[winnaar - 1] + ")";
        result += (gok == winnaar ? " - Je had juist." : " - Je had fout.");

        std::string reply = "paardenrace!>" + naam + ">" + result;
        push.send(zmq::buffer(reply), zmq::send_flags::none);

        std::string log = "log!>paardenrace>" + naam + ">gokte op " + std::to_string(gok) + ", " + result;
        push.send(zmq::buffer(log), zmq::send_flags::none);
    }
}

// Heartbeat
void heartbeatLoop(zmq::context_t& context) {
    zmq::socket_t push(context, zmq::socket_type::push);
    push.connect("tcp://benternet.pxl-ea-ict.be:24041");

    while (true) {
        push.send(zmq::buffer("heartbeat?>custom_dice"), zmq::send_flags::none);
        push.send(zmq::buffer("heartbeat?>paardenrace"), zmq::send_flags::none);
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

int main() {
    zmq::context_t context(1);

    std::thread threadDice(runCustomDice, std::ref(context));
    std::thread threadPaarden(runPaardenrace, std::ref(context));
    std::thread threadHeartbeat(heartbeatLoop, std::ref(context));

    threadDice.join();
    threadPaarden.join();
    threadHeartbeat.join();

    return 0;
}
