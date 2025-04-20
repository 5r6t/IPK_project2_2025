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
#include <vector>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h> // getaddrinfo
#include <sys/time.h> // timeval struct

#define BUFFER_SIZE 65536 // 64kb is 2^16 + 4
#define TCP_TIMEOUT 5000 // 5 second timeout

class Client_Comms {
    public:
        int client_socket = -1;
        int get_socket(); // for FD_SET() in client_session
        uint16_t next_msg_id();
        Client_Comms(const std::string &hostname, bool protocol, uint16_t port, uint16_t timeout);

        void connect_set();
        // TCP
        void resolve_ip();
        void connect_tcp();
        void send_tcp_message(const std::string &msg);
        std::string receive_tcp_message();
        void receive_tcp_chunk();
        // 
        std::optional<std::string> timed_tcp_reply();               // string for TCP
        std::optional<std::vector<uint8_t>> timed_udp_reply();      // vector for UDP

        // UDP
        void set_udp();
        void send_udp_message(const std::vector<uint8_t>& pac);
        std::vector<uint8_t> receive_udp_message();


        void terminate_connection(int ex_code = 0);
        std::string buffer;
    private:
        std::string host_name;
        std::string ip_address;
        sockaddr_in udp_address;
        sockaddr_in dynamic_address{}; // zeroed
        bool has_dyn_addr = false;

        bool tproto;
        uint16_t port;
        uint16_t udp_timeout;
        uint16_t msg_id_cnt = 0;
        void send_udp_packet(const std::vector<uint8_t>& pac);
        std::vector<uint8_t> receive_udp_packet();
};
