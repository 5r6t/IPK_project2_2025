/**
 * @file client_session.cpp
 * @brief IPK project 2 - Chat client
 * @date 15-4-2025
 * Author: Jaroslav Mervart, xmervaj00
*/

#include "client_session.h"
#include "tools.h"

Client_Session* Client_Session::active_instance = nullptr;

Client_Session::Client_Session(const Client_Init &config)
    : config(config) {
    active_instance = this;
    this->comms = std::make_unique<Client_Comms>(
        config.get_hostname(), config.is_tcp(), config.get_port());
}

std::atomic<bool> stop_requested = false;

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
    stop_requested = true;
    if (active_instance) {
        active_instance->graceful_exit();
    }
}

void Client_Session::graceful_exit(int ex_code) {
    if (config.is_tcp() == true) {
        std::string bye_msg = "BYE FROM " + this->display_name + "\r\n";
        comms->send_tcp_message(bye_msg);
    }
    comms->terminate_connection(ex_code);  // closes socket and exits
}


void Client_Session::run(){
    fd_set rfds;
    std::string cmd_buffer;

    std::signal(SIGINT, handle_sigint);
    comms->resolve_ip();
    comms->connect_set();
    this->state = ClientState::Start;

    while(true) {
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(comms->get_socket(), &rfds);
        int max_fd = std::max(STDIN_FILENO, comms->get_socket()) + 1;
        printf_debug("Waiting on stdin (%d) and socket (%d)", STDIN_FILENO, comms->get_socket());
        int active = select(max_fd, &rfds, nullptr, nullptr, nullptr);

        if (active < 0) {
            perror("Select");
            break;
        }
        
        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            if (!std::getline(std::cin, cmd_buffer)) {
                graceful_exit(); // Ctrl+D or error
            }
    
            if (cmd_buffer.empty()) continue;
    
            if (cmd_buffer[0] == '/') { handle_command(cmd_buffer); } 
            else {  handle_chat_msg(cmd_buffer); }
        }

        if (FD_ISSET(comms->get_socket(), &rfds)) {
            if (config.is_tcp()) {
                std::string tcp_msg = comms->receive_tcp_message();
                handle_tcp_response(tcp_msg);
                /*
                while (true) {
                    size_t pos = comms->buffer.find("\r\n");
                    if (pos == std::string::npos) break;
                
                    std::string msg = comms->buffer.substr(0, pos);
                    comms->buffer.erase(0, pos + 2);
                    handle_tcp_response(msg);
                }
                */
            } else {
                std::vector<uint8_t> udp_msg = comms->receive_udp_message();
                handle_udp_response(udp_msg);
            }
        }
        if (stop_requested) break;
    }
}

void Client_Session::handle_chat_msg(const std::string &line) {
    if (this->state != ClientState::Open) {
        std::cout << "ERROR: You must authenticate first. See '/help'.\n";
        return;
    }
    if (check_message_content(line, MessageContent)) {
        printf_debug("sending MSG %s ...", line.c_str());
        // MSG FROM {DisplayName} IS {MessageContent}\r\n
        std::string msg = "MSG FROM " + this->display_name + " IS " + line + "\r\n";
        send_message(msg);    
    } else {
        std::cout << "ERROR: Invalid format of MessageContent, try again.\n";
    }
}

void Client_Session::handle_command(const std::string &line) {
    printf_debug("%s", line.c_str());

    std::istringstream iss(line);
    std::string command;
    std::vector<std::string> args;
    std::string temp;

    iss >> command; // first value is command
    while (iss >> temp) {
        args.push_back(temp); // the rest are arguments
    }

    if (command == "/auth") {
        send_auth(args);
    } else if (command == "/join") {
        send_join(args);
    } else if (command == "/rename") {
        rename(args);
    } else if (command == "/help") {
        print_local_help();
    } else {
        std::cout << "ERROR: Invalid command. Get some '/help'.\n";
    }
}

void Client_Session::send_auth(const std::vector<std::string>& args) 
{
    if (this->state != ClientState::Start) {
        std::cout << "ERROR: Cannot authenticate again.\n";
        return;
    }
    if (args.size() != 3) { // /auth {Username} {Secret} {DisplayName}
        std::cout << "ERROR: Missing arguments, try again.\n";
        return;
    }

    this->state = ClientState::Auth;
    auto username = args.at(0);
    auto secret = args.at(1);
    rename(std::vector<std::string>{args[2]});

    if (!check_message_content(username, Username) 
        || !check_message_content(secret, Secret)) {
        std::cout << "ERROR: Invalid Username/Secret format.\n";
        this->state = ClientState::Open;
        return;
    }

    std::optional<std::string> tcp_reply;
    std::optional<std::vector<uint8_t>> udp_reply;

    if (config.is_tcp()) {
        auto auth_msg = "AUTH " + username + " AS " + this->display_name 
                        + " USING " + secret + "\r\n"; 
        send_message(auth_msg); 
        tcp_reply = comms->timed_tcp_reply(); 
    } else {
        auto msg_id = comms->next_msg_id();
        auto auth_msg = Toolkit::build_auth(msg_id, username, 
                                            this->display_name, secret);
        send_message(auth_msg);
        udp_reply = comms->receive_udp_message();
    }

    if (!tcp_reply && !udp_reply) {
        std::cout << "ERROR: Authentication timed out.\n";
        graceful_exit();
        return;
    }
    if (config.is_tcp()) {
        std::string tcp_confirmation = *tcp_reply;
        handle_tcp_response(tcp_confirmation);
    } else {
        std::vector<uint8_t> udp_confirmation = *udp_reply;
        handle_udp_response(udp_confirmation);
    }
}

void Client_Session::send_join(const std::vector<std::string>& args) 
{
    if (this->state != ClientState::Open) 
    {
        std::cout << "ERROR: To join a channel, you first must authenticate.\n";
        return;
    }

    if (args.size() != 1) 
    {   // /join {ChannelID}
        std::cout << "ERROR: No ChannelID, try again.\n";
        return;
    }

    this->state = ClientState::Join;

    auto channel_id = args.at(0);
    if (!check_message_content(channel_id, ChannelID)) 
    {   // JOIN {ChannelID} AS {DisplayName}\r\n
        std::cout << "ERROR: Invalid ChannelID format, try again.\n";
        return;
    }

    std::optional<std::string> tcp_reply;
    std::optional<std::vector<uint8_t>> udp_reply;

    if (config.is_tcp()) 
    {
        auto join_msg = "JOIN " + args.at(0) + " AS " + this->display_name + "\r\n"; 
        send_message(join_msg);
        tcp_reply = comms->timed_tcp_reply();
    } else {
        auto msg_id = comms->next_msg_id();
        auto join_msg = Toolkit::build_join(msg_id, channel_id, this->display_name);
        send_message(join_msg);
        udp_reply = comms->timed_udp_reply();   
    }

    if (!tcp_reply && !udp_reply) 
    {
        std::cout << "ERROR: Join timed out.\n";
        graceful_exit(ERR_TIMEOUT);
        return;
    }

    if (config.is_tcp()) {
        std::string tcp_confirmation = *tcp_reply;
        handle_tcp_response(tcp_confirmation);
    } else {
        std::vector<uint8_t> udp_confirmation = *udp_reply;
        handle_udp_response(udp_confirmation);
    }
}

void Client_Session::rename(const std::vector<std::string>& args) 
{
    if ( !(args.size() == 1)) 
    {   // /rename {DisplayName}
        std::cout << "ERROR: No or multiple usernames selected, try again.\n";
        return;
    }
    if (check_message_content(args.at(0), DisplayName)) 
    {
        this->display_name = args.at(0);
        printf_debug("Changed DisplayName to '%s'", this->display_name.c_str()); 
    } else {
        std::cout << "ERROR: Invalid DisplayName format, try again.\n";
    }
}

bool Client_Session::check_message_content(const std::string &content, msg_param param) 
{
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

void Client_Session::send_message(const std::string& msg) {
    printf_debug("About to send %s", msg.c_str());
    comms->send_tcp_message(msg);
}
void Client_Session::send_message(const std::vector<uint8_t>& msg) {
    printf_debug("About to send UDP message");
    comms->send_udp_message(msg);
}
void Client_Session::handle_tcp_response(std::string &msg) {
    auto parsed_opt = parse_tcp_message(msg);
    if (!parsed_opt) {
        std::cout << "ERROR: Malformed message received: " << msg << "\n";
        std::string err = "ERR FROM " + this->display_name + " IS invalid message\r\n";
        send_message(err);
        graceful_exit();
        return;
    }
    const ParsedMessage& parsed = *parsed_opt;

    switch (this->state) {
        case ClientState::Auth:
            if (parsed.type == "REPLY OK") {
                std::cout << "Action Success: " << parsed.content << "\n";
                this->state = ClientState::Open;
            } else if (parsed.type == "REPLY NOK") {
                std::cout << "Action Failure: " << parsed.content << "\n";
                this->state = ClientState::Start;
            } else if (parsed.type == "ERR") {
                std::cout << "ERROR FROM " << parsed.display_name << ": " << parsed.content << "\n";
                graceful_exit(ERR_SERVER);
            } else {
                std::cout << "ERROR: Unexpected message in AUTH state: " << msg << "\n";
                graceful_exit(ERR_SERVER);
            }
            break;

        case ClientState::Open:
            if (parsed.type == "REPLY OK" || parsed.type == "REPLY NOK") {
                std::cout << "ERROR: Unexpected REPLY received: " << parsed.content << "\n";
                graceful_exit(ERR_SERVER);
            }
            // fall through is desired here - REPLY (N)OK is either handled or the rest is similar.
        case ClientState::Join:
            if (parsed.type == "MSG") {
                std::cout << parsed.display_name << ": " << parsed.content << "\n";
            } else if (parsed.type == "REPLY OK" || parsed.type == "REPLY NOK") {
                std::cout << (parsed.type == "REPLY OK" ? "Action Success: " : "Action Failure: ") << parsed.content << "\n";
                this->state = ClientState::Open;

            } else if (parsed.type == "ERR") {
                std::cout << "ERROR FROM " << parsed.display_name << ": " << parsed.content << "\n";
                graceful_exit(ERR_SERVER);
            } else if (parsed.type == "BYE") {
                std::cout << "ERROR FROM " << parsed.display_name << ": session ended\n";
                graceful_exit(ERR_SERVER);
            } else {
                std::cout << "ERROR: Unexpected message received: " << msg << "\n";
                std::string err = "ERR FROM " + this->display_name + " IS invalid message\r\n";
                send_message(err);
                graceful_exit(ERR_SERVER);
            }
            break;

        case ClientState::Start:
        default:
            std::cout << "ERROR: Message received in invalid client state: " << msg << "\n";
            break;
    }
}

std::optional<Client_Session::ParsedMessage> Client_Session::parse_tcp_message(const std::string &msg) 
{
    printf_debug("Parsing message: %s", msg.c_str());

    ParsedMessage result;

    if (msg.starts_with("REPLY OK IS ") || msg.starts_with("REPLY NOK IS ")) {
        bool is_ok = msg.starts_with("REPLY OK IS ");
        result.type = is_ok ? "REPLY OK" : "REPLY NOK";
        size_t prefix_len = is_ok ? strlen("REPLY OK IS ") : strlen("REPLY NOK IS ");
        result.content = msg.substr(prefix_len);

    } else if (msg.starts_with("MSG FROM ")) {
        result.type = "MSG";
        size_t from_pos = strlen("MSG FROM ");
        size_t is_pos = msg.find(" IS ", from_pos);
        if (is_pos != std::string::npos) {
            result.display_name = msg.substr(from_pos, is_pos - from_pos);
            result.content = msg.substr(is_pos + 4);
        }

    } else if (msg.starts_with("ERR FROM ")) {
        result.type = "ERR";
        size_t from_pos = strlen("ERR FROM ");
        size_t is_pos = msg.find(" IS ", from_pos);
        if (is_pos != std::string::npos) {
            result.display_name = msg.substr(from_pos, is_pos - from_pos);
            result.content = msg.substr(is_pos + 4);
        }

    } else if (msg.starts_with("BYE FROM ")) {
        result.type = "BYE";
        result.display_name = msg.substr(strlen("BYE FROM "));
    }

    return result;
}

void Client_Session::handle_udp_response(const std::vector<uint8_t>& pac) {
    if (pac.empty()) {
        std::cerr << "ERROR: Empty UDP packet received\n";
        return;
    }

    uint8_t type = pac[0];

    switch (type) {
        //case 0x00: return handle_udp_confirm(pac);
        case 0x01: return handle_udp_reply(pac);
        //case 0x03: return handle_udp_join(pac);
        //case 0x04: return handle_udp_msg(pac);
        case 0xFD: return handle_udp_ping(pac);
        //case 0xFE: return handle_udp_err(pac);
        case 0xFF: return handle_udp_bye(pac);
        default:
            std::cerr << "ERROR: Unknown UDP packet type: " << int(type) << "\n";
            break;
    }
}

void Client_Session::handle_udp_reply(const std::vector<uint8_t>& pac) {
    // server sent bye
    // confirm bye received
    // wait for possible retransmit
    uint8_t result = pac[3]; 
    if (result == 0) {
        printf_debug("Nok'd");
    } else {
        printf_debug("Ok'd");
    }
}

void Client_Session::handle_udp_ping(const std::vector<uint8_t>& pac) {
    printf_debug("Pinged ^w^");
    uint16_t msg_id = (pac[1] << 8) | pac[2];
    auto confirm_pac = Toolkit::build_confirm(msg_id);
    comms->send_udp_message(confirm_pac);
}

void Client_Session::handle_udp_bye(const std::vector<uint8_t>& pac) {
    // server sent bye
    // confirm bye received
    // wait for possible retransmit
    printf_debug("ending program");
    graceful_exit(0);
}