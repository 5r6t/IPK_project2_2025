/**
 * @file client_session.cpp
 * @brief IPK project 2 - Chat client
 * @date 5-4-2025
 * Author: Jaroslav Mervart, xmervaj00
 */
#include "client_session.h"
#include "tools.h"

Client_Session* Client_Session::active_instance = nullptr;

Client_Session::Client_Session(const Client_Init &config)
    : config(config) {
        active_instance = this;
};

void Client_Session::print_local_help() {
    std::cout << "-----------------------------------------\n"
              << "Supported commands:\n"
              << "  /auth <username> <secret> <displayname>\n"
              << "  /join <channel>\n"
              << "  /rename <displayname>\n"
              << "  /help\n"
              << "Status:\n"
              << "  Current display name: " << this->display_name << "\n"
              << "  Authenticated: " << (this->state == ClientState::Open ? "true" : "false") << "\n"
              << "-----------------------------------------\n";
}

std::atomic<bool>stop_requested= false;
void Client_Session::handle_sigint(int) {
    //stop_requested = true;
    if (active_instance) {
        active_instance->graceful_exit();
    }
}

void Client_Session::graceful_exit() {
    printf_debug("%s", "Gracefuly exiting...");
    if (config.get_protocol() == "tcp" && this->state != ClientState::Start) {
        std::string bye_msg = "BYE FROM " + this->display_name + "\r\n";
        send_message(bye_msg);
    } else if (this->state != ClientState::Start) { // udp
        printf_debug("%s", "UDP shutdown requested.");
    }
    close(client_socket);
    exit(0);
}

void Client_Session::run() {
    std::signal(SIGINT, handle_sigint);
    std::string line;

    while(std::getline(std::cin, line)) {
        // this if has no effect, ctrl+d kills getline, handle_sigint handles ctrl+c
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
     *          (open) --- server: MSG      -> client: _    --- (open)  // server can send messages
     *          (open) --- server: _        -> client: MSG  --- (open)  // client can send messages
     *          (open) --- server: ERR, BYE -> client: _    --- ((end)) // server sends error, client quits
     *          (open) --- server: *REPLY   -> client: ERR  --- ((end)) // idk 
     *          (open) --- server: _        -> client: BYE  --- ((end)) // client says bye, program ends
     *          (open) --- server: _        -> client: JOIN --- (join)  // client can join
     * 
     *              (join) -- server: MSG       -> client: _   --- (join) 
     *              (join) -- server: *REPLY    -> client: _   --- (open)
     *              (join) -- server: ERR, BYE  -> client: _   --- ((end))
     */
void Client_Session::handle_chat_msg(const std::string &line) {
    if (this->state != ClientState::Open) {
        std::cerr << "Error: To send a message, you must first authenticate. See '/help'\n";
        return;
    }
    printf_debug("sending MSG %s ...", line.c_str());
    // MSG FROM {DisplayName} IS {MessageContent}\r\n
    std::string msg = "MSG FROM " + this->display_name + " IS " + line + "\r\n";
    send_message(msg);
}

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
        std::cerr << "Error: Invalid command. Get some '/help'.\n";
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

    if (config.get_protocol() == "tcp") connect_tcp();
    // AUTH {Username} AS {DisplayName} USING {Secret}\r\n
    auto auth_msg = "AUTH " + username + " AS " + this->display_name + " USING " + secret + "\r\n"; 
    send_message(auth_msg);

    this->state=ClientState::Open; // Reply from server received?
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

    // JOIN {ChannelID} AS {DisplayName}\r\n
    auto join_msg = "JOIN " + args.at(0) + " AS " + this->display_name + "\r\n";
    send_message(join_msg);

    this->state = ClientState::Open;
}

void Client_Session::rename(const std::vector<std::string>& args) {
    if ( !(args.size() == 1)) {
        std::cerr << "Error: No or invalid username, try again.\n";
        return;
    }
    this->display_name = args.at(0);
    printf_debug("Changed DisplayName to '%s'", this->display_name.c_str()); 
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

void Client_Session::connect_tcp() {
    int family = AF_INET;
    int type = SOCK_STREAM;
    int protocol = IPPROTO_TCP;
    auto port = config.get_port();
    auto ipv4_address = config.get_ip();
    this->client_socket = socket(family, type, protocol);

    struct sockaddr_in server_addr {};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port); // port from config
        inet_pton(AF_INET, ipv4_address.c_str(), &server_addr.sin_addr);
        
    socklen_t address_size = sizeof(server_addr);
    struct sockaddr *address = (struct sockaddr*)&server_addr;
    
    
    if (this->client_socket <= 0) {
        std::cerr << "Error: Cannot create TCP socket\n";  
        graceful_exit();
    }

    if  (connect(this->client_socket, address, address_size) != 0) {
        std::cerr << "Error: Cannot connect\n";
        graceful_exit();
    }
    printf_debug("%s", "TCP Connected succesfully");
}

void Client_Session::send_message(const std::string &msg) {
    if (config.get_protocol() == "tcp") {
        send_tcp_message(msg);
    } else {
        send_udp_message(msg);
    }
}

void Client_Session::send_tcp_message(const std::string &msg) {
    // closed socket not handled for now
    int bytes_tx = send(this->client_socket, msg.c_str(), strlen(msg.c_str()), 0);
    if (bytes_tx < 0) {
        //perror("ERROR: send");
        std::cerr << "Error: Cannot send message: " << msg << "\n";
        return;
    }
}

void Client_Session::send_udp_message(const std::string &msg) {
    printf_debug("%s", "Not implemented yet!");
    return;
}