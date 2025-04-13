/**
 * @file client_session.cpp
 * @brief IPK project 2 - Chat client
 * @date 13-4-2025
 * Author: Jaroslav Mervart, xmervaj00
*/

#include "client_session.h"
#include "tools.h"

Client_Session* Client_Session::active_instance = nullptr;

Client_Session::Client_Session(const Client_Init &config)
    : config(config) {
        active_instance = this;
};

extern std::atomic<bool> stop_requested = false;

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

void Client_Session::handle_sigint(int) {
    //stop_requested = true;
    if (active_instance) {
        active_instance->graceful_exit();
    }
}

void Client_Session::graceful_exit() {
    printf_debug("%s", "Gracefuly exiting...");
    if (config.get_protocol() == "tcp") {
        std::string bye_msg = "BYE FROM " + this->display_name + "\r\n";
        send_message(bye_msg);
    } else { // udp
        printf_debug("%s", "UDP shutdown requested.");
    }
    close(client_socket);
    exit(0);
}

void Client_Session::run(){
    fd_set rfds;
    char input_buffer[BUFFER_SIZE];
    std::string cmd_buffer;

    std::signal(SIGINT, handle_sigint);
    connect_tcp();
    this->state = ClientState::Start;

    while(true) {
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(this->client_socket, &rfds);
        int max_fd = std::max(STDIN_FILENO, client_socket) + 1;
        int active = select(max_fd, &rfds, nullptr, nullptr, nullptr);
        
        if (active < 0) { // select failed
            perror("Select");
            break;
        }
        
        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            if (!std::getline(std::cin, cmd_buffer)) {
                graceful_exit(); // Ctrl+D or error
            }
    
            if (cmd_buffer.empty()) continue;
    
            if (cmd_buffer[0] == '/') {
                handle_command(cmd_buffer);
            } else {
                handle_chat_msg(cmd_buffer);
            }
        }

        if (FD_ISSET(client_socket, &rfds)) {
            std::string msg = receive_message();
            handle_server_message(msg);
        }
        if (stop_requested) break;
    }
}

    /**
     * '_' means no input received/sent
     * '*' means any input (all possible messages)
     * '!' means negative version
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
    if (check_message_content(line, MessageContent)) {
        printf_debug("sending MSG %s ...", line.c_str());
        // MSG FROM {DisplayName} IS {MessageContent}\r\n
        std::string msg = "MSG FROM " + this->display_name + " IS " + line + "\r\n";
        send_message(msg);    
    } else {
        std::cerr << "Error: Invalid format of MessageContent, try again.\n";
    }
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
        auth(args);
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

    if (config.get_protocol() == "tcp") {
        // AUTH {Username} AS {DisplayName} USING {Secret}\r\n
    
        if (check_message_content(username, Username) && check_message_content(secret, Secret)) {
            auto auth_msg = "AUTH " + username + " AS " + this->display_name + " USING " + secret + "\r\n"; 
            send_message(auth_msg);
        } else {
            std::cerr << "Error: Invalid ChannelID format.\n";
        }
        std::string confirmation = receive_message();
        if (confirmation.starts_with("REPLY OK IS ")) {
            std::cout << "Action Success: " << confirmation << "\n";
            this->state = ClientState::Open;
        } else if (confirmation.starts_with("REPLY NOK IS ")) {
            std::cout << "Action Failure: Invalid credentials, please try again\n";
            this->state = ClientState::Start; // does not trully adhere to specified fsm, for simplicity
        } else { // err, bye | msg from server
            std::cerr << confirmation;
            std::cerr << "Error: Server side error occured.\n";
            graceful_exit(); // does not differentiate between err,bye | msg for now
        }
    } else { // udp

    }

}

void Client_Session::join(const std::vector<std::string>& args) {
    if (this->state != ClientState::Open) {
        std::cerr << "Error: To join a channel, you first must authenticate.\n";
        return;
    }
    if ( !(args.size() == 1) || args.at(0).size() > 20) {
        std::cerr << "Error: No or invalid ChannelID, try again.\n";
        return;
    }

    this->state = ClientState::Join;
    if (check_message_content(args.at(0), ChannelID)) {
    // JOIN {ChannelID} AS {DisplayName}\r\n
        auto join_msg = "JOIN " + args.at(0) + " AS " + this->display_name + "\r\n";
        send_message(join_msg);        
    } else {
        std::cerr << "Error: Invalid ChannelID format, try again.\n";
    }

    this->state = ClientState::Open;
}

void Client_Session::rename(const std::vector<std::string>& args) {
    if ( !(args.size() == 1)) {
        std::cerr << "Error: No or multiple usernames selected, try again.\n";
        return;
    }
    if (check_message_content(args.at(0), DisplayName)) {
        this->display_name = args.at(0);
        printf_debug("Changed DisplayName to '%s'", this->display_name.c_str()); 
    } else {
        std::cerr << "Error: Invalid DisplayName format, try again.\n";
    }
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
// 010101010101010101001010010100101010010101001010100101010010100101010100101001010101011001010101101001010101
std::string Client_Session::receive_message() {
    char buffer[BUFFER_SIZE];

    int bytes_rx = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_rx < 0) {
        perror("ERROR: recv");
        return "";
    }
    buffer[bytes_rx] = '\0';
    return std::string(buffer);
}

void Client_Session::recv_loop() {
    std::string tcp_buffer;

    while (true) { //
        char temp[BUFFER_SIZE];
        ssize_t bytes = recv(this->client_socket, temp, BUFFER_SIZE - 1, 0);
        if (bytes <= 0) break;

        temp[bytes] = '\0';
        tcp_buffer += temp;

        while (true) {
            size_t pos = tcp_buffer.find("\r\n");
            if (pos == std::string::npos) break;

            std::string msg = tcp_buffer.substr(0, pos);
            tcp_buffer.erase(0, pos + 2);
            handle_server_message(msg);
        }
    }
}

void Client_Session::handle_server_message(std::string &msg) {
    std::cout << msg << "\n";
}