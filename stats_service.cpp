#include <iostream>
#include <string>
#include <unordered_map>
#include <zmq.hpp>

// Struct om statistieken per speler bij te houden
struct Statistiek {
    int juist = 0;
    int totaal = 0;
};

int main() {
    zmq::context_t context{1};
    zmq::socket_t sub_socket{context, zmq::socket_type::sub};
    sub_socket.connect("tcp://benternet.pxl-ea-ict.be:24042");

    const std::string filter = "paardenrace!>";
    sub_socket.setsockopt(ZMQ_SUBSCRIBE, filter.c_str(), filter.size());

    std::unordered_map<std::string, Statistiek> stats;

    std::cout << "✅ Stats-service actief..." << std::endl;

    while (true) {
        zmq::message_t msg;
        sub_socket.recv(msg, zmq::recv_flags::none);
        std::string data = msg.to_string();

        // Verwacht formaat: paardenrace!>Runar>Winnaar: 2 (DustRunner) - Je had juist!
        size_t start = data.find("!>") + 2;
        size_t midden = data.find(">", start);
        std::string naam = data.substr(start, midden - start);
        std::string inhoud = data.substr(midden + 1);

        if (inhoud.find("Je had juist") != std::string::npos) {
            stats[naam].juist++;
            stats[naam].totaal++;
        } else if (inhoud.find("Je had fout") != std::string::npos) {
            stats[naam].totaal++;
        } else {
            continue;
        }

        // Print statistieken
        std::cout << "\n📊 Statistieken na gok van " << naam << ":\n";
        for (const auto& [speler, stat] : stats) {
            double ratio = stat.totaal == 0 ? 0 : (double)stat.juist / stat.totaal * 100;
            std::cout << "- " << speler << ": " << stat.juist << " juist op " << stat.totaal << " gokken (" << ratio << "% winrate)\n";
        }
    }

    return 0;
}
