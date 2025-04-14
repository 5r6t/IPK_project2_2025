/**
 * @file tcp_comm.cpp
 * @brief IPK project 2 - Client for a chat server
 * @date 14-4-2025
 * Author: Jaroslav Mervart, xmervaj00
*/

#include "client_comms.h"
#include "tools.h"

Client_Comms::Client_Comms(const std::string &ip, uint16_t port)
    : ip(ip), port(port) {}

int Client_Comms::get_socket() {
    return this->client_socket;
}

void Client_Comms::connect_tcp() {
    int family = AF_INET;
    int type = SOCK_STREAM;
    int protocol = IPPROTO_TCP;
    this->client_socket = socket(family, type, protocol);

    struct sockaddr_in server_addr {};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(this->port); // port from config
        inet_pton(AF_INET, this->ip.c_str(), &server_addr.sin_addr);
        
    socklen_t address_size = sizeof(server_addr);
    struct sockaddr *address = (struct sockaddr*)&server_addr;
    
    
    if (this->client_socket <= 0) {
        std::cerr << "ERROR: Cannot create TCP socket\n";  
        terminate_connection();
    }

    if  (connect(this->client_socket, address, address_size) != 0) {
        std::cerr << "ERROR: Cannot connect\n";
        terminate_connection();
    }
    printf_debug("%s", "TCP Connected succesfully");
}

void Client_Comms::terminate_connection() {
    if (client_socket != -1) {
        close(client_socket);
    }
    exit(0);
}


void Client_Comms::send_tcp_message(const std::string &msg) {
    // closed socket not handled for now
    int bytes_tx = send(this->client_socket, msg.c_str(), strlen(msg.c_str()), 0);
    if (bytes_tx < 0) {
        //perror("ERROR: send");
        std::cerr << "ERROR: Cannot send message: " << msg << "\n";
        return;
    }
}

std::string Client_Comms::receive_tcp_message() {
    while (true) {
        size_t pos = this->tcp_buffer.find("\r\n");
        if (pos != std::string::npos) {
            std::string msg = tcp_buffer.substr(0, pos);
            tcp_buffer.erase(0, pos + 2);
            return msg;
        }
        receive_tcp_chunk();
    }
}

void Client_Comms::receive_tcp_chunk() {
    printf_debug("%s", "Getting another TCP message chunk.");
    char temp[BUFFER_SIZE];
    int bytes_rx = recv(client_socket, temp, BUFFER_SIZE - 1, 0);
    if (bytes_rx < 0) {
        perror("ERROR: recv");
        return;
    }
    if (bytes_rx == 0) {
        std::cerr << "ERROR: Server has closed the connection\n.";
        terminate_connection();
    }
    temp[bytes_rx] = '\0';
    this->tcp_buffer += temp; // Add another chunk
}

void Client_Comms::send_udp_message(const std::string &msg) {
    printf_debug("%s", "Not implemented yet!");
    return;
}
