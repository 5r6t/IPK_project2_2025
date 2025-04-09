
/**
 * @file client_init.cpp
 * @brief IPK project 2 - Chat client
 * @date 5-4-2025
 * Author: Jaroslav Mervart, xmervaj00
 */
#include "client_init.h"
#include "tools.h"

std::string Client_Init::get_protocol() const { return protocol; }
std::string Client_Init::get_ip()       const { return ip; }
uint16_t    Client_Init::get_port()     const { return port; }
uint16_t    Client_Init::get_timeout()  const { return timeout; }
uint8_t     Client_Init::get_retries()  const { return retries; }

void Client_Init::set_protocol(std::string protocol) {
    if (protocol != "tcp" && protocol != "udp") {
        std::cerr << "Error: " << protocol << " is not valid" << std::endl;
        exit(ERR_INVALID);
    }
    this->protocol = protocol;
}

void Client_Init::set_ip(std::string host) {
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4 only
    hints.ai_socktype = SOCK_STREAM; // TCP

    if (getaddrinfo(host.c_str(), nullptr, &hints, &result) != 0) {
        std::cerr << "Error: Unable to resolve IP or hostname: " << host << std::endl;
        exit(ERR_INVALID);
    }
    // Accept first valid result
    this->ip = host;
    freeaddrinfo(result);
}

void Client_Init::set_port(std::string port) {
    int p = catch_stoi(port, std::numeric_limits<uint16_t>::max(),"Ports");
    this->port = static_cast<uint16_t>(p);
}

void Client_Init::set_udp_timeout(std::string timeout) {
    int t = catch_stoi(timeout, std::numeric_limits<uint16_t>::max(),"UDP timeout");
    this->timeout = static_cast<uint16_t>(t);
}

void Client_Init::set_udp_retries(std::string retries) {
    int r = catch_stoi(retries, std::numeric_limits<uint8_t>::max(),"UDP retries");
    this->retries = static_cast<uint8_t>(r);
}

void Client_Init::print_help() {
    std::cout << "Usage: ./ipk25-chat -t <tcp|udp> -s <hostname|ip> [-p port] [-d timeout] [-r retries] [-h]\n\n"
    << "Options:\n"
    << "  -t <proto>     Set transport protocol (tcp or udp). Required.\n"
    << "  -s <server>    Set server IP address or hostname. Required.\n"
    << "  -p <port>      Set server port (default: 4567).\n"
    << "  -d <timeout>   Set UDP confirmation timeout in ms (default: 250).\n"
    << "  -r <retries>   Set number of UDP retransmissions (default: 3).\n"
    << "  -h             Show this help message and exit.\n\n"
    << "Examples:\n"
    << "  ./ipk25-chat -t tcp -s 127.0.0.1\n"
    << "  ./ipk25-chat -t udp -s ipk.fit.vutbr.cz -p 10000\n"
    << "  ./ipk25-chat -t udp -s 127.0.0.1 -p 3000 -d 100 -r 1\n";
    exit(0);
}

void Client_Init::validate() {
    printf_debug("Transport: %s", protocol.c_str());
    printf_debug("IP:        %s", ip.c_str());
    printf_debug("Port:      %u", port);
    printf_debug("Timeout:   %u ms", timeout);
    printf_debug("Retries:   %u", retries);

}
