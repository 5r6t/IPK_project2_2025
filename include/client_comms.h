/**
 * @file tcp_comm.h
 * @brief IPK project 2 - Client for a chat server
 * @date 14-4-2025
 * Author: Jaroslav Mervart, xmervaj00
*/

#pragma once

#include <iostream>
#include <cstring>
#include <string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


#define BUFFER_SIZE 65536 // 64kb is 2^16 + 4

class Client_Comms {
    public:
        int client_socket = -1;
        Client_Comms(const std::string &ip, uint16_t port);
        
        void connect_tcp();
        void send_tcp_message(const std::string &msg);
        void send_udp_message(const std::string &msg);
        std::string receive_tcp_message();

        int get_socket(); // for FD_SET()
        void terminate_connection();
        std::string tcp_buffer;
    private:
        std::string ip;
        uint16_t port;

        void receive_tcp_chunk();
};
