/**
 * @file client_init.h
 * @brief IPK project 1 - Layer 4 scanner
 * @date 5-4-2025
 * Author: Jaroslav Mervart, xmervaj00
 */

#pragma once
#include <iostream>
#include <sstream>
#include <vector>
#include <set>
#include <string>
#include <pcap.h>
#include <cstring>

#define DEF_SERVER_PORT 4567
#define UDP_RETRANS 3 // Maximum number of UDP retransmissions
#define UDP_CONFIRM_TM 250 // UDP confirmation timeout (in milliseconds)

#define ERR_MISSING 10
#define ERR_INVALID 11
#define ERR_INTERNAL 99

class Client_Init {
    public:
        void set_protocol(std::string protocol); // values tcp or udp
        void set_ip(std::string host); // server IP or hostname
        void set_port(std::string port); // Server port -- uint16 (expected value)
        void set_udp_timeout(std::string timeout); // set UDP confirmation timeout (in milliseconds) - uint16
        void set_udp_retrans(std::string max_num); // set Maximum number of UDP retransmissions -- uint8
        void print_help();

    private:
};

/**
 * Arguments with "User provided" text in the value column are mandatory and 
 * have to be always specified when running the program. 
 * Other values indicate that the argument is optional with its default value 
 * defined in the column (or no value).
 */