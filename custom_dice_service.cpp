#include <string>
#include <iostream>
#include <random>
#include <regex>
#include <zmq.hpp>

int main() {
    const int max_sides = 10000;

    zmq::context_t context(1);

    zmq::socket_t sub_socket(context, zmq::socket_type::sub);
    sub_socket.connect("tcp://benternet.pxl-ea-ict.be:24042");
    sub_socket.setsockopt(ZMQ_SUBSCRIBE, "custom_dice?>", 13);

    zmq::socket_t push_socket(context, zmq::socket_type::push);
    push_socket.connect("tcp://benternet.pxl-ea-ict.be:24041");

    std::random_device rd;
    std::mt19937 gen(rd());

    std::cout << "🎲 Custom Dice Service actief (max d" << max_sides << ")" << std::endl;

    while (true) {
        zmq::message_t msg;
        sub_socket.recv(msg, zmq::recv_flags::none);
        std::string input = msg.to_string();

        std::cout << "[Ontvangen] " << input << std::endl;

        // Regex: custom_dice?>(naam)>(vraag)
        std::regex pattern(R"(custom_dice\?\>([^>]+)>(.+))");
        std::smatch match;

        if (std::regex_match(input, match, pattern)) {
            std::string name = match[1];
            std::string vraag = match[2];

            // Check of vraag correct is: dX
            std::regex dice_pattern(R"(d(\d+))");
            std::smatch dice_match;

            if (std::regex_match(vraag, dice_match, dice_pattern)) {
                int sides = std::stoi(dice_match[1]);

                if (sides < 2 || sides > max_sides) {
                    std::string error = "custom_dice!>" + name + ">d" + std::to_string(sides) +
                                        "=Invalid dice: must be between 2 and " + std::to_string(max_sides);
                    push_socket.send(zmq::buffer(error), zmq::send_flags::none);
                    std::cout << "[Fout] " << error << std::endl;
                } else {
                    std::uniform_int_distribution<> distrib(1, sides);
                    int result = distrib(gen);

                    std::string response = "custom_dice!>" + name + ">d" + std::to_string(sides) + "=" + std::to_string(result);
                    push_socket.send(zmq::buffer(response), zmq::send_flags::none);
                    std::cout << "[Verstuurd] " << response << std::endl;
                }
            } else {
                std::string error = "custom_dice!>" + name + ">Invalid input: use d[number] (e.g. d20)";
                push_socket.send(zmq::buffer(error), zmq::send_flags::none);
                std::cout << "[Fout] Geen geldig dX-formaat." << std::endl;
            }

        } else {
            std::cout << "[Ongeldig berichtformaat – genegeerd]" << std::endl;
        }
    }

    return 0;
}
