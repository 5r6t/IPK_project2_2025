/**
 * @file client_session.h
 * @brief IPK project 2 - Chat client
 * @date 5-4-2025
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

extern std::atomic<bool> stop_requested; 

class Client_Session {
    public:
        Client_Session(const Client_Init &config);
        void run();

    private:
        static Client_Session* active_instance;
        const Client_Init &config;

        std::string protocol;
        int sockfd = -1;
        std::string display_name;

        void handle_chat_msg(const std::string& line);
        void handle_command(const std::string& line);

        void print_local_help();
        void auth(const std::vector<std::string>& args);   // sends auth request
        void join(const std::vector<std::string>& args);   // sends join rqst
        void rename(const std::vector<std::string>& args); // sends rename rqst
        
        enum class ClientState {
            Start,
            Auth,
            Open,
            Join,
            End
        };
        ClientState state = ClientState::Start;

        enum msg_param {MessageID, Username, ChannelID, Secret, DisplayName, MessageContent};
        bool check_message_content(const std::string &content, msg_param param);

        static void handle_sigint(int);
        void graceful_exit();
};