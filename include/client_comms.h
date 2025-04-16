/**
 * @file client_comms.h
 * @brief IPK project 2 - Client for a chat server
 * @date 15-4-2025
 * Author: Jaroslav Mervart, xmervaj00
*/

#pragma once

#include <iostream>
#include <cstring>
#include <string>
#include <optional>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h> // getaddrinfo
#include <sys/time.h> // timeval struct

#define BUFFER_SIZE 65536 // 64kb is 2^16 + 4
#define TIMEOUT 5000 // 5 second timeout

class Client_Comms {
    public:
        int client_socket = -1;
        Client_Comms(const std::string &ip, uint16_t port);

        // TCP
        void connect_tcp();
        void send_tcp_message(const std::string &msg);
        std::string receive_tcp_message();
        //
        std::optional<std::string> timed_reply(int timeout_ms=TIMEOUT, bool is_tcp = true);
        // UDP
        void connect_udp();
        void send_udp_message(const std::string &msg);
        std::string receive_udp_message();


        int get_socket(); // for FD_SET()
        void terminate_connection(int ex_code = 0);
        std::string tcp_buffer;
    private:
        std::string host_name;
        uint16_t port;
        void receive_tcp_chunk();


};
