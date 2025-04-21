/**
 * @file client_comms.cpp
 * @brief IPK project 2 - Client for a chat server
 * @date 21-4-2025
 * Author: Jaroslav Mervart, xmervaj00
*/

#include "client_comms.h"
#include "tools.h"

Client_Comms::Client_Comms(const std::string &hostname, bool protocol, uint16_t port, uint16_t timeout)
    : host_name(hostname), tproto(protocol), port(port), udp_timeout(timeout){}

int Client_Comms::get_socket() {
    return this->client_socket;
}

uint16_t Client_Comms::next_msg_id() {
    return this->msg_id_cnt++;
}

void Client_Comms::connect_set() {
    if (this->tproto) {
        connect_tcp();
    } else {
        set_udp(); // no connect, just sets socket
    }
}

void Client_Comms::terminate_connection(int ex_code) {
    if (client_socket != -1) {
        close(client_socket);
    }
    exit(ex_code);
}

void Client_Comms::resolve_ip() {
    printf_debug("Resolving hostname...");
    struct addrinfo hints{}, *result = nullptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;

    if (this->tproto) {
        hints.ai_socktype = SOCK_STREAM;
    } else {
        hints.ai_socktype = SOCK_DGRAM;
    }

    if (getaddrinfo(host_name.c_str(), nullptr, &hints, &result) != 0) {
        std::cerr << "ERROR: Unable to resolve domain name: " << host_name << "\n";
        exit(ERR_INVALID);
    }

    for (auto next = result; next != nullptr; next = next->ai_next) {
        if (next->ai_family == AF_INET) {
            struct sockaddr_in* ipv4 = (struct sockaddr_in*)next->ai_addr;

            if (this->tproto) {
                char ip_buf[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(ipv4->sin_addr), ip_buf, sizeof(ip_buf));
                this->ip_address = ip_buf;
            } else {
                this->udp_address = *ipv4;  // entire sockaddr_in
                this->udp_address.sin_port = htons(this->port); // add port 

                char buf[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &udp_address.sin_addr, buf, sizeof(buf));
                printf_debug("Resolved UDP address = %s, port = %d", buf, ntohs(udp_address.sin_port));

            }
            break; // First resolved address is used
        }
    }
    printf_debug("Success, hostname resolved.");
    freeaddrinfo(result);
}

/**
 *   MMMMMMM   OOOO  HHHH
 *      H     O      H   H
 *      H     O      HHHHH
 *      H     O      H 
 *      H      OOOO  H
*/

void Client_Comms::connect_tcp() 
{
    int family = AF_INET;
    int type = SOCK_STREAM;
    int protocol = IPPROTO_TCP;
    this->client_socket = socket(family, type, protocol);

    struct sockaddr_in server_addr {};
        server_addr.sin_family = family;
        server_addr.sin_port = htons(this->port);
        inet_pton(AF_INET, this->ip_address.c_str(), &server_addr.sin_addr);
        
    socklen_t address_size = sizeof(server_addr);
    struct sockaddr *address = (struct sockaddr*)&server_addr;
    
    
    if (this->client_socket <= 0) {
        std::cout << "ERROR: Cannot create TCP socket\n";  
        terminate_connection(ERR_INTERNAL);
    }

    if  (connect(this->client_socket, address, address_size) != 0) {
        perror("ERROR: Can't connect to socket");
        std::cout << "ERROR: Cannot connect\n";
        terminate_connection(ERR_SERVER);
    }
    printf_debug("%s", "TCP Connected succesfully");
}

void Client_Comms::send_tcp_message(const std::string &msg) {
    int bytes_tx = send(this->client_socket, msg.c_str(), strlen(msg.c_str()), 0);
    if (bytes_tx < 0) {
        std::cerr << "ERROR: Cannot send message: " << msg << "\n";
        return;
    }
}

std::string Client_Comms::receive_tcp_message() 
{
    while (true) {
        size_t pos = this->buffer.find("\r\n");
        if (pos != std::string::npos) {
            std::string msg = buffer.substr(0, pos);
            buffer.erase(0, pos + 2);
            return msg;
        }
        receive_tcp_chunk();
    }
}

void Client_Comms::receive_tcp_chunk() {
    printf_debug("Getting another TCP message chunk...");

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(client_socket, &rfds);

    struct timeval tv;
    tv.tv_sec = TCP_TIMEOUT / 1000;
    tv.tv_usec = (TCP_TIMEOUT % 1000) * 1000;

    int ready = select(client_socket + 1, &rfds, nullptr, nullptr, &tv);
    if (ready <= 0) {
        std::cerr << "ERROR: recv() timeout or error.\n";
        std::string err = "ERR FROM " + this->ip_address + " IS incomplete message\r\n";
        send_tcp_message(err);
        terminate_connection(ERR_SERVER);
        return;
    }

    char temp[BUFFER_SIZE];
    int bytes_rx = recv(client_socket, temp, BUFFER_SIZE - 1, 0);
    if (bytes_rx < 0) {
        perror("ERROR: recv");
        return;
    }

    if (bytes_rx == 0) {
        std::cout << "ERROR: Server has closed the connection.\n";
        terminate_connection(ERR_SERVER);
        return;
    }

    temp[bytes_rx] = '\0';
    buffer += temp;
}

std::optional<std::string> Client_Comms::timed_tcp_reply() 
{
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(client_socket, &rfds);

    struct timeval tv;
    tv.tv_sec = TCP_TIMEOUT / 1000;
    tv.tv_usec = (TCP_TIMEOUT % 1000) * 1000;

    int ready = select(client_socket + 1, &rfds, nullptr, nullptr, &tv);
    if (ready <= 0) {
        return std::nullopt;
    }

    return receive_tcp_message();
}

/**
  *   H    H  HOOOO   HHHO
  *   H    H  H    O  H   H
  *   H    H  H    O  HHHHO
  *   O    O  H    O  H
  *    OOOO   HOOOO   H
*/

void Client_Comms::set_udp() 
{
    int family = AF_INET;
    int type = SOCK_DGRAM;
    int protocol = 0;
    this->client_socket = socket(family, type, protocol);
    if (this->client_socket <= 0) {
        std::cerr << "ERROR: Cannot create socket.\n";
        terminate_connection(ERR_INTERNAL);
    }
}

void Client_Comms::send_udp_message(const std::vector<uint8_t>& pac) 
{
    printf_debug("Sending UDP message.");
    send_udp_packet(pac);
}

void Client_Comms::send_udp_packet(const std::vector<uint8_t>& pac) 
{
    printf_debug("Sending UDP packet.");
    int flags = 0;

    sockaddr_in *in_addr = has_dyn_addr ? &dynamic_address : &udp_address;
    sockaddr* address = (sockaddr*) in_addr;

    socklen_t address_size = sizeof(*in_addr);
    
    int bytes_tx = sendto(this->client_socket, pac.data(), pac.size(), 
                          flags, address, address_size);
    if (bytes_tx < 0) {
        std::cerr << "ERROR: Cannot send, try again.\n";
    }                          
    return;
}

std::vector<uint8_t> Client_Comms::receive_udp_message() 
{
    printf_debug("Receiving UDP message.");
    return receive_udp_packet();              
}

std::vector<uint8_t> Client_Comms::receive_udp_packet() 
{
    printf_debug("Receiving UDP packet...");

    std::vector<uint8_t> data;
    char temp[BUFFER_SIZE];

    sockaddr_in src_addr{};
    socklen_t addr_len = sizeof(src_addr);

    int bytes_rx = recvfrom(client_socket, temp, BUFFER_SIZE,
                            0, (sockaddr*)&src_addr, &addr_len);

    if (bytes_rx < 0) 
    {
        perror("ERROR: recvfrom");
        return {};
    }

    if (!has_dyn_addr) 
    {
        if ((uint8_t)temp[0] == 0x01) 
        {
            dynamic_address = src_addr;
            has_dyn_addr = true;
            printf_debug("Stored dynamic server address: port %d", ntohs(src_addr.sin_port));
        }
    }

    data.insert(data.end(), temp, temp + bytes_rx); // copying into vector
    return data;
}


std::optional<std::vector<uint8_t>> Client_Comms::timed_udp_reply() 
{
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(client_socket, &rfds);

    struct timeval tv;
    tv.tv_sec =  this->udp_timeout / 1000;
    tv.tv_usec = (this->udp_timeout % 1000) * 1000;

    int ready = select(client_socket + 1, &rfds, nullptr, nullptr, &tv);
    if (ready <= 0) {
        return std::nullopt;
    }

    return receive_udp_message();
}
