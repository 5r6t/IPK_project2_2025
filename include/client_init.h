/**
 * @file client_init.h
 * @brief IPK project 2 - Client for a chat server
 * @date 13-4-2025
 * Author: Jaroslav Mervart, xmervaj00
*/

#pragma once

#include "client_init.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <netdb.h> // getaddrinfo
#include <arpa/inet.h> // inet_ntop
#include <limits>

class Client_Init {
    public:
        void set_protocol(std::string protocol); // values tcp or udp
        void set_ip(std::string host); // server IP or hostname
        void set_port(std::string port); // Server port -- uint16 (expected value)
        void set_udp_timeout(std::string timeout); // set UDP confirmation timeout (in milliseconds) - uint16
        void set_udp_retries(std::string max_num); // set Maximum number of UDP retransmissions -- uint8
        void print_help();
        void validate(); 
        std::string get_ip() const;
        std::string get_protocol() const;
        uint16_t get_port() const;
        uint16_t get_timeout() const;
        uint8_t get_retries() const;

    private:
        std::string protocol = "";
        std::string ip = "";
        uint16_t port = 4567;
        uint16_t timeout = 250;
        uint8_t retries = 3;
};