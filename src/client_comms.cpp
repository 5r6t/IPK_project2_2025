/**
 * @file client_comms.cpp
 * @brief IPK project 2 - Client for a chat server
 * @date 15-4-2025
 * Author: Jaroslav Mervart, xmervaj00
*/

#include "client_comms.h"
#include "tools.h"

Client_Comms::Client_Comms(const std::string &hostname, const std::string &protocol, uint16_t port)
    : host_name(hostname), tproto(protocol), port(port) {}

int Client_Comms::get_socket() {
    return this->client_socket;
}

void Client_Comms::connect_set() {
    if (this->tproto == "tcp") {
        connect_tcp();
    } else {
        set_udp(); // no connect, just sets socket
    }
}

std::optional<std::string> Client_Comms::timed_reply(int timeout_ms, bool is_tcp) {
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(client_socket, &rfds);

    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int ready = select(client_socket + 1, &rfds, nullptr, nullptr, &tv);
    if (ready <= 0) {
        return std::nullopt;
    }

    return is_tcp ? receive_tcp_message() : receive_udp_message();  
}

void Client_Comms::resolve_ip() {
    printf_debug("Resolving hostname...");
    struct addrinfo hints{}, *result = nullptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;

    if (tproto == "tcp") {
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

            if (tproto == "tcp") {
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

void Client_Comms::connect_tcp() {
    int family = AF_INET;
    int type = SOCK_STREAM;
    int protocol = IPPROTO_TCP;
    this->client_socket = socket(family, type, protocol);

    struct sockaddr_in server_addr {};
        server_addr.sin_family = family;
        server_addr.sin_port = htons(this->port); // port from config
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

void Client_Comms::terminate_connection(int ex_code) {
    if (client_socket != -1) {
        close(client_socket);
    }
    exit(ex_code);
}

void Client_Comms::send_tcp_message(const std::string &msg) {
    int bytes_tx = send(this->client_socket, msg.c_str(), strlen(msg.c_str()), 0);
    if (bytes_tx < 0) {
        //perror("ERROR: send");
        std::cerr << "ERROR: Cannot send message: " << msg << "\n";
        return;
    }
}

std::string Client_Comms::receive_tcp_message() {
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
    printf_debug("Getting another TCP message chunk.");

    char temp[BUFFER_SIZE];
    int bytes_rx = recv(client_socket, temp, BUFFER_SIZE - 1, 0);
    
    if (bytes_rx < 0) {
        perror("ERROR: recv");
        return;
    }
    if (bytes_rx == 0) {
        std::cout << "ERROR: Server has closed the connection\n."; // nowhere to send BYE
        terminate_connection(ERR_SERVER);
    }
    temp[bytes_rx] = '\0';
    this->buffer += temp; // Add another chunk
}
/**
  *   H    H  HOOOO   HHHO
  *   H    H  H    O  H   H
  *   H    H  H    O  HHHHO
  *   O    O  H    O  H
  *    OOOO   HOOOO   H
*/

void Client_Comms::set_udp() {
    int family = AF_INET;
    int type = SOCK_DGRAM;
    int protocol = 0;
    this->client_socket = socket(family, type, protocol);
    if (this->client_socket <= 0) {
        //perror("ERRORR: socket");
        std::cerr << "ERROR: Cannot create socket.\n";
        terminate_connection(ERR_INTERNAL);
    }
}

void Client_Comms::send_udp_message(const std::string &msg) {
    printf_debug("Sending UDP message.");

    int flags = 0;
    struct sockaddr *address = (struct sockaddr *) &udp_address;
    int address_size = sizeof(udp_address);
    int bytes_tx = sendto(client_socket, msg.c_str(), strlen(msg.c_str()),
                          flags, address, address_size);
    if (bytes_tx < 0) {
        perror("ERROR: sendto");
    }                          
    return;
}

std::string Client_Comms::receive_udp_message() {
    printf_debug("UDP receive not completely implemented.");


    struct sockaddr *address = (struct sockaddr *) &udp_address;
    socklen_t address_size = sizeof(udp_address);
    int flags = 0;
    //..
    char temp[BUFFER_SIZE];
    int bytes_rx = recvfrom(client_socket, temp, BUFFER_SIZE-1, 
                            flags, address, &address_size);

    if (bytes_rx < 0) 
    {
        perror("ERROR: recvfrom");
    }
    return {};
}
