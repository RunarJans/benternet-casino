#include <string>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <zmq.hpp>

struct Statistiek {
    int totaal = 0;
    int zessen = 0;
    std::vector<std::string> timestamps;
};

int main() {
    zmq::context_t context(1);

    zmq::socket_t sub_dobbel(context, zmq::socket_type::sub);
    zmq::socket_t sub_vraag(context, zmq::socket_type::sub);
    zmq::socket_t push_socket(context, zmq::socket_type::push);

    sub_dobbel.connect("tcp://benternet.pxl-ea-ict.be:24042");
    sub_vraag.connect("tcp://benternet.pxl-ea-ict.be:24042");
    push_socket.connect("tcp://benternet.pxl-ea-ict.be:24041");

    sub_dobbel.setsockopt(ZMQ_SUBSCRIBE, "dobbelsteen!>", 13);
    sub_vraag.setsockopt(ZMQ_SUBSCRIBE, "stats?>", 7);

    std::unordered_map<std::string, Statistiek> database;

    std::cout << "📊 Stats-service actief..." << std::endl;

    while (true) {
        zmq::pollitem_t items[] = {
            { static_cast<void*>(sub_dobbel), 0, ZMQ_POLLIN, 0 },
            { static_cast<void*>(sub_vraag), 0, ZMQ_POLLIN, 0 }
        };

        zmq::poll(items, 2, -1);

        if (items[0].revents & ZMQ_POLLIN) {
            zmq::message_t msg;
            sub_dobbel.recv(msg, zmq::recv_flags::none);
            std::string result = msg.to_string();

            std::string payload = result.substr(13);
size_t pos1 = payload.find(">");
size_t pos2 = payload.rfind(">");

if (pos1 == std::string::npos || pos2 == std::string::npos || pos1 == pos2) {
    std::cerr << "[⚠️  FOUT] Ongeldig dobbelsteenbericht (2 delimiters verwacht): " << payload << std::endl;
    continue;
}

std::string naam = payload.substr(0, pos1);
std::string worp_str = payload.substr(pos2 + 1);


            int worp = 0;
            try {
                worp = std::stoi(worp_str);
            } catch (...) {
                std::cerr << "[⚠️  FOUT] Kan worp niet parsen uit: " << worp_str << std::endl;
                continue;
            }

            database[naam].totaal++;
            if (worp == 6) {
                database[naam].zessen++;
                char buf[100];
                std::time_t now = std::time(nullptr);
                std::strftime(buf, sizeof(buf), "%F %T", std::localtime(&now));
                database[naam].timestamps.push_back(buf);
            }

            std::cout << "[📝 LOG] " << naam << " gooide " << worp << std::endl;
        }

        if (items[1].revents & ZMQ_POLLIN) {
            zmq::message_t msg;
            sub_vraag.recv(msg, zmq::recv_flags::none);
            std::string verzoek = msg.to_string();
            std::string naam = verzoek.substr(7, verzoek.length() - 8);

            std::ostringstream antwoord;
            if (database.find(naam) == database.end()) {
                antwoord << "stats!>" << naam << ">Nog geen worpen.";
            } else {
                auto& s = database[naam];
                double perc = (s.totaal > 0) ? (100.0 * s.zessen / s.totaal) : 0.0;
                antwoord << "stats!>" << naam << ">"
                         << std::fixed << std::setprecision(1)
                         << perc << "% (" << s.zessen << "/" << s.totaal << " keer 6)";
            }

            push_socket.send(zmq::buffer(antwoord.str()), zmq::send_flags::none);
            std::cout << "[📤 ANTWOORD] " << antwoord.str() << std::endl;
        }
    }
}
