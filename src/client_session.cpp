
/**
 * @file client_session.cpp
 * @brief IPK project 2 - Chat client
 * @date 5-4-2025
 * Author: Jaroslav Mervart, xmervaj00
 */
#include "client_session.h"

Client_Session::Client_Session(const Client_Init &config) : config(config) {}

void Client_Session::print_local_help() {
    std::cout << "Supported commands:\n"
              << "  /auth <username> <secret> <displayname>\n"
              << "  /join <channel>\n"
              << "  /rename <displayname>\n"
              << "  /help\n";
}

void Client_Session::run() {
    std::string line;
    while(std::getline(std::cin, line)) {
        if(line.empty()) continue;
        
        if(line[0] == '/') {
            handle_command(line);
        } else {
            //handle_chat_msg(line);
        }
    }
}

void Client_Session::handle_command(const std::string &line) {
    std::istringstream iss(line);
    std::string command;
    std::vector<std::string> args;
    std::string temp;

    iss >> command;

    while (iss >> temp) {
        args.push_back(temp);
    }

    if (command == "/auth") {

    } else if (command == "/join") {

    } else if (command == "/rename") {

    } else if (command == "/help") {

    } else {
        std::cerr << "Error: Invalid command. Get some /help." << std::endl; 
    }
}