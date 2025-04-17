/**
 * @file client_session.h
 * @brief IPK project 2 - Client for a chat server
 * @date 15-4-2025
 * Author: Jaroslav Mervart, xmervaj00
*/

#pragma once

#include <string>
#include <vector>

#include <iostream>
#include <sstream>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h> // read(), close()

#include <csignal>
#include <atomic>
#include <memory> // unique_ptr

#include "client_init.h"
#include "client_comms.h"

extern std::atomic<bool> stop_requested; 

class Client_Session {
    public:
        Client_Session(const Client_Init &config);
        void run();

    private:
        static Client_Session* active_instance;
        const Client_Init &config;
        std::unique_ptr<Client_Comms> comms; // Create instance of Client_Comms to use

        std::string display_name;
        enum msg_param {MessageID, Username, ChannelID, Secret, DisplayName, MessageContent};

        enum class ClientState {
            Start,
            Auth,
            Open,
            Join
            // End
        };
        ClientState state;

        struct ParsedMessage {
            std::string type;         // e.g. REPLY OK/NOK, MSG/ERR/BYE FROM
            std::string display_name; // Sender
            std::string content;      // Content of the message received
        };

        void handle_chat_msg(const std::string& line);
        void handle_command(const std::string& line);

        void print_local_help();
        void send_auth(const std::vector<std::string>& args);
        void send_join(const std::vector<std::string>& args);
        void rename(const std::vector<std::string>& args);

        static void handle_sigint(int);
        void graceful_exit(int ex_code = 0);                

        void send_message(const std::string& msg);  // junction function between protocols
        void send_message(const std::vector<uint8_t>& msg);  // junction function between protocols
        std::string receive_message();              // junction function between protocols

        bool check_message_content(const std::string &content, msg_param param);
        void handle_server_message(std::string &msg);
        std::optional<ParsedMessage> parse_message(const std::string &msg);
};