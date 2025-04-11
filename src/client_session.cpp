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
    std::cout << "-----------------------------------------\n"
              << "Supported commands:\n"
              << "  /auth <username> <secret> <displayname>\n"
              << "  /join <channel>\n"
              << "  /rename <displayname>\n"
              << "  /help\n"
              << "Current display name: " << this->display_name << std::endl
              << "-----------------------------------------\n";
}

std::atomic<bool>stop_requested= false;

void Client_Session::handle_sigint(int) {
    //stop_requested = true;
    graceful_exit(); // avoid getline blocking ctrl+c
}

void Client_Session::graceful_exit() {
    printf_debug("%s", "Graceful shutdown requested.");
    
    //if (/* socket open & authenticated */) {
        // send BYE message here }

    // close socket if open
    // set state = ClientState::End;
    
    exit(0); // final termination
}


void Client_Session::run() {
    std::signal(SIGINT, handle_sigint);
    std::string line;

    while(std::getline(std::cin, line)) {
        if (stop_requested) break;

        if(line.empty()) continue;

        
        if(line[0] == '/') {
            handle_command(line);
        } else {
            handle_chat_msg(line);
        } 
    }

    if (std::cin.eof()) graceful_exit(); // ctrl+d
}

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
     *      (auth) --- server: _        -> client: BYE  --- ((end)) // ctrl+c on clientside caused this ig
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
     */

void Client_Session::handle_command(const std::string &line) {
    printf_debug("%s", line.c_str());

    std::istringstream iss(line);
    std::string command;
    std::vector<std::string> args;
    std::string temp;

    iss >> command;
    while (iss >> temp) {
        args.push_back(temp);
    }

    if (command == "/auth") {
        auth(args); // assuming state is Start
    } else if (command == "/join") {
        join(args);
    } else if (command == "/rename") {
        rename(args);
    } else if (command == "/help") {
        print_local_help();
    } else {
        std::cerr << "Error: Invalid command. Get some /help.\n";
    }
}

void Client_Session::auth(const std::vector<std::string>& args) {
    if (this->state != ClientState::Start) {
        std::cerr << "Error: Cannot authenticate again.\n";
        return;
    }
    if ( !(args.size() == 3)) {
        std::cerr << "Error: Missing arguments, try again.\n";
        return;
    }

    this->state = ClientState::Auth; // Checks passed 

    auto username = args.at(0);
    auto secret = args.at(1);
    rename(std::vector<std::string>{args[2]});
    printf_debug("NOT sending AUTH msg { Username: %s } { Secret: %s }...", username.c_str(), secret.c_str());

    this->state=ClientState::Open; // Reply from server received
}

void Client_Session::join(const std::vector<std::string>& args) {
    if (this->state != ClientState::Open) {
        std::cerr << "Error: To join a channel, you first must authenticate.\n";
        return;
    }
    if ( !(args.size() == 1)
            || args.at(0).size() > 20) {
        std::cerr << "Error: No or invalid ChannelID, try again.\n";
        return;
    }

    this->state = ClientState::Join;

    printf_debug("NOT JOINing channel %s ...", args.at(0).c_str());
}

void Client_Session::rename(const std::vector<std::string>& args) {
    if ( !(args.size() == 1)) {
        std::cerr << "Error: No or invalid username, try again.\n";
        return;
    }
    this->display_name = args.at(0);
    printf_debug("Changed DisplayName to '%s'", this->display_name.c_str()); 
}

void Client_Session::handle_chat_msg(const std::string &line) {
    printf_debug("%s", line.c_str());
    if (this->state != ClientState::Open) {
        std::cerr << "Error: To send a message, you must first authenticate.\n";
        return;
    }
    printf_debug("NOT sending MSG %s ...", line.c_str());

}

bool Client_Session::check_message_content(const std::string &content, msg_param param) {
    switch (param)
    {
    case Username:
    case ChannelID:
        return content.size() <= 20 && Toolkit::only_allowed_chars(content, "^[a-zA-Z0-9_-]+$");
        break;
    
    case Secret:
        return content.size() <= 128 && Toolkit::only_allowed_chars(content, "^[a-zA-Z0-9_-]+$");
        break;

    case DisplayName:
        return content.size() <= 20 && Toolkit::only_printable_chars(content, false);
        break;
    
    case MessageContent:
        return content.size() <= 60000 && Toolkit::only_printable_chars(content, true);
        break;

    default:
        return false;
        break;
    }
}