/**
 * @file ipk-l4-scan.cpp
 * @brief IPK project 1 - Layer 4 scanner
 * @date 5-4-2025
 * Author: Jaroslav Mervart, xmervaj00
 */

#include "client_init.h"

 int main(int argc, char **argv) {
    Client_Init config;

    // Small function to check if the next argument is present
    auto get_next_arg = [&](int &i, const std::string &flag) -> std::string {
        if (i + 1 < argc && argv[i + 1][0] != '-') {
            return argv[++i];
        }
        std::cerr << "Error: Missing argument for " << flag << std::endl;
        exit(ERR_MISSING);
    };

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-t") {
            config.set_protocol(get_next_arg(i, arg));
            i++;
        }
        else if (arg == "-s") {
            config.set_ip(get_next_arg(i, arg));
            i++;
        }
        else if (arg == "-p") {
            config.set_port(get_next_arg(i, arg));
            i++;
        }
        else if (arg == "-d") {
            config.set_udp_timeout(get_next_arg(i, arg));
            i++;
        }
        else if (arg == "-r") {
            config.set_udp_retrans(get_next_arg(i, arg));
            i++;
        }
        else if (arg == "-h" || arg == "--help") {
            config.print_help(); // help exits the program
        }
        else {
            exit(ERR_INVALID); // unexpected input
        }
    }
    return 0;
}
