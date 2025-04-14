/**
 * @file ipk25chat-client.cpp
 * @brief IPK project 2 - Client for a chat server
 * @date 13-4-2025
 * Author: Jaroslav Mervart, xmervaj00
*/

#include "tools.h"
#include "client_init.h"
#include "client_session.h"
#include <set>

 int main(int argc, char **argv) {
    Client_Init config;

    // Small function to check if the next argument is present
    std::set<std::string> params = {"-t", "-s", "-p", "-d", "-r", "-h"};
    auto get_next_arg = [&](int &i, const std::string &flag) -> std::string {
        if (i + 1 < argc && !params.contains(argv[i + 1])) {
            return argv[++i];
        }
        std::cerr << "Error: Missing argument for " << flag << "\n";
        exit(ERR_MISSING);
    };

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-t") {
            config.set_protocol(get_next_arg(i, arg));
        }
        else if (arg == "-s") {
            config.set_ip(get_next_arg(i, arg));
        }
        else if (arg == "-p") {
            config.set_port(get_next_arg(i, arg));
        }
        else if (arg == "-d") {
            config.set_udp_timeout(get_next_arg(i, arg));
        }
        else if (arg == "-r") {
            config.set_udp_retries(get_next_arg(i, arg));
        }
        else if (arg == "-h" || arg == "--help") {
            config.print_help(); // help exits the program
        }
        else {
            exit(ERR_INVALID); // unexpected input
        }
    }

    config.validate();
    Client_Session session(config);
    session.run();
    return 0;
}
