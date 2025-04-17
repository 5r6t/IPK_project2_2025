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
        int get_socket(); // for FD_SET() in client_session
        
        Client_Comms(const std::string &hostname, const std::string &protocol, uint16_t port);

        void connect_set();
        // TCP
        void resolve_ip();
        void connect_tcp();
        void send_tcp_message(const std::string &msg);
        std::string receive_tcp_message();
        // 
        std::optional<std::string> timed_reply(bool is_tcp = true);
        // UDP
        void set_udp();
        void send_udp_message(const std::string &msg);
        std::string receive_udp_message();


        void terminate_connection(int ex_code = 0);
        std::string buffer;
    private:
        std::string host_name;
        std::string ip_address;
        sockaddr_in udp_address;
        std::string tproto;
        uint16_t port;
        void receive_tcp_chunk();


};
