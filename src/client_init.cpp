/**
 * @file client_init.cpp
 * @brief IPK project 2 - Chat client
 * @date 16-4-2025
 * Author: Jaroslav Mervart, xmervaj00
*/

#include "client_init.h"
#include "tools.h"

bool Client_Init::is_tcp() const { return protocol == "tcp"; }
std::string Client_Init::get_hostname() const { return hostname; }
uint16_t    Client_Init::get_port()     const { return port; }
uint16_t    Client_Init::get_timeout()  const { return timeout; }
uint8_t     Client_Init::get_retries()  const { return retries; }

void Client_Init::set_protocol(std::string protocol) 
{
    if (protocol != "tcp" && protocol != "udp") {
        std::cerr << "Error: " << protocol << " is not valid\n";
        exit(ERR_INVALID);
    }
    this->protocol = protocol;
}

void Client_Init::set_hostname(std::string host) 
{
    this->hostname = host;
}

void Client_Init::set_port(std::string port) 
{
    int p = Toolkit::catch_stoi(port, std::numeric_limits<uint16_t>::max(),"Ports");
    this->port = static_cast<uint16_t>(p);
}

void Client_Init::set_udp_timeout(std::string timeout) 
{
    int t = Toolkit::catch_stoi(timeout, std::numeric_limits<uint16_t>::max(),"UDP timeout");
    this->timeout = static_cast<uint16_t>(t);
}

void Client_Init::set_udp_retries(std::string retries) 
{
    int r = Toolkit::catch_stoi(retries, std::numeric_limits<uint8_t>::max(),"UDP retries");
    this->retries = static_cast<uint8_t>(r);
}

void Client_Init::print_help() 
{
    std::cout << "Usage: ./ipk25chat-client -t <tcp|udp> -s <hostname|ip> [-p port] [-d timeout] [-r retries] [-h]\n\n"
    << "Options:\n"
    << "  -t <proto>     Set transport protocol (tcp or udp). Required.\n"
    << "  -s <server>    Set server IP address or hostname. Required.\n"
    << "  -p <port>      Set server port (default: 4567).\n"
    << "  -d <timeout>   Set UDP confirmation timeout in ms (default: 250).\n"
    << "  -r <retries>   Set number of UDP retransmissions (default: 3).\n"
    << "  -h             Show this help message and exit.\n\n"
    << "Examples:\n"
    << "  ./ipk25chat-client -t tcp -s 127.0.0.1\n"
    << "  ./ipk25chat-client -t udp -s ipk.fit.vutbr.cz -p 10000\n"
    << "  ./ipk25chat-client -t udp -s 127.0.0.1 -p 3000 -d 100 -r 1\n";
    exit(0);
}

void Client_Init::validate() 
{
    printf_debug("Transport: %s", protocol.c_str());
    printf_debug("Hostname:        %s", hostname.c_str());
    printf_debug("Port:      %u", port);
    printf_debug("Timeout:   %u ms", timeout);
    printf_debug("Retries:   %u", retries);
    if (this->protocol == "" || this->hostname == "" ) {
        std::cerr << "Protocol or IP not selected, display help with '-h'.\n";
        exit(ERR_INVALID);
    }
}
