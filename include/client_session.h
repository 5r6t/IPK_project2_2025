/**
 * @file client_session.h
 * @brief IPK project 2 - Client for a chat server
 * @date 13-4-2025
 * Author: Jaroslav Mervart, xmervaj00
*/

#pragma once

#include "client_init.h"
#include <string>
#include <vector>

#include <iostream>
#include <sstream>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h> // read(), close()
#include <thread>
#include <csignal>
#include <atomic>

#define BUFFER_SIZE 65536 // 64kb is 2^16 + 4

extern std::atomic<bool> stop_requested; 

class Client_Session {
    public:
        Client_Session(const Client_Init &config);
        void run();

    private:
        static Client_Session* active_instance;
        const Client_Init &config;

        std::string tcp_buffer;

        int client_socket = -1;
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
            std::string type;         // REPLY OK, REPLY NOK, MSG FROM, ERR FROM, BYE FROM
            std::string display_name; // sender
            std::string content;      // of the message received
        };

        void handle_chat_msg(const std::string& line);
        void handle_command(const std::string& line);

        void print_local_help();
        void send_auth(const std::vector<std::string>& args);   // sends auth request
        void send_join(const std::vector<std::string>& args);   // sends join rqst
        void rename(const std::vector<std::string>& args); // sends rename rqst
        
        bool check_message_content(const std::string &content, msg_param param);

        static void handle_sigint(int);
        void graceful_exit();

        // NTWRK
        void connect_tcp();
        
        void send_message(const std::string &msg); // just to select appropriate protocol
        void send_tcp_message(const std::string &msg);
        void send_udp_message(const std::string &msg);
        
        
        std::string receive_message();
        std::string receive_tcp_message();
        void receive_tcp_chunk();

        
        void handle_server_message(std::string &msg);
        ParsedMessage parse_message(const std::string &msg);
};