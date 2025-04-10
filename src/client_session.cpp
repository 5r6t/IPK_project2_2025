
/**
 * @file client_session.cpp
 * @brief IPK project 2 - Chat client
 * @date 5-4-2025
 * Author: Jaroslav Mervart, xmervaj00
 */
#include "client_session.h"
#include "tools.h"

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

void Client_Session::fsm_handler(ClientState new_state, const std::string &from_command) {
    /**
     * '_' means no input received/sent
     * '*' means any input (all possible messages)
     * 
     * (start) --- server:ERR, BYE -> client: _    --- ((end))
     * (start) --- server: _       -> client: AUTH --- (auth)
     * 
     *      (auth) --- server: !REPLY   -> client: AUTH --- (auth)
     *      (auth) --- server: ERR, BYE -> client: _    --- ((end)) 
     *      (auth) --- server: MSG      -> client: ERR  --- ((end)) 
     *      (auth) --- server: _        -> client: BYE  --- ((end)) 
     *      (auth) --- server: REPLY    -> client: _    --- (open) 
     * 
     *          (open) --- server: MSG      -> client: _    --- (open)
     *          (open) --- server: _        -> client: MSG  --- (open)
     *          (open) --- server: ERR, BYE -> client: _    --- ((end))
     *          (open) --- server: *REPLY   -> client: ERR  --- ((end))
     *          (open) --- server: _        -> client: BYE  --- ((end))
     *          (open) --- server: _        -> client: JOIN --- (join)
     * 
     *              (join) -- server: MSG       -> client: _   --- (join)
     *              (join) -- server: *REPLY    -> client: _   --- (open)
     *              (join) -- server: ERR, BYE  -> client: _   --- ((end))
     *              (join) -- server: _         -> client: BYE --- ((end))
     */
    return;
}


void Client_Session::handle_command(const std::string &line) {
    std::istringstream iss(line);
    std::string command;
    std::vector<std::string> args;
    std::string temp;

    iss >> command;
    printf_debug("%s", line.c_str());
    while (iss >> temp) {
        args.push_back(temp);
    }

    if (command == "/auth") {
        auth(args);
    } else if (command == "/join") {
        join(args);
    } else if (command == "/rename") {
        rename(args);
    } else if (command == "/help") {
        print_local_help();
    } else {
        std::cerr << "Error: Invalid command. Get some /help." << std::endl; 
    }
}

void Client_Session::auth(const std::vector<std::string>& args) {
    if ( !(args.size() == 3)) {
        std::cerr << "Error: No or invalid ChannelID, try again." << std::endl;
        return;
    }
    auto username = args.at(0);
    auto secret = args.at(1);
    rename(std::vector<std::string>{args[2]});
    printf_debug("NOT sending AUTH msg { Username: %s } { Secret: %s }...", username.c_str(), secret.c_str());
}

void Client_Session::join(const std::vector<std::string>& args) {
    if ( !(args.size() == 1)) {
        std::cerr << "Error: No or invalid ChannelID, try again." << std::endl;
        return;
    }
    printf_debug("NOT JOINing channel %s ...", args.at(0).c_str());
}

void Client_Session::rename(const std::vector<std::string>& args) {
    if ( !(args.size() == 1)) {
        std::cerr << "Error: No or invalid username, try again." << std::endl;
        return;
    }
    this->display_name = args.at(0);
    printf_debug("Changed DisplayName to '%s'", this->display_name.c_str()); 
}