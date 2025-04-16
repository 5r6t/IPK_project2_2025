/**
 * @file client_comms.cpp
 * @brief IPK project 2 - Client for a chat server
 * @date 15-4-2025
 * Author: Jaroslav Mervart, xmervaj00
*/

#include "client_comms.h"
#include "tools.h"

Client_Comms::Client_Comms(const std::string &hostname, uint16_t port)
    : host_name(hostname), port(port) {}

int Client_Comms::get_socket() {
    return this->client_socket;
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
        inet_pton(AF_INET, this->host_name.c_str(), &server_addr.sin_addr);
        
    socklen_t address_size = sizeof(server_addr);
    struct sockaddr *address = (struct sockaddr*)&server_addr;
    
    
    if (this->client_socket <= 0) {
        std::cout << "ERROR: Cannot create TCP socket\n";  
        terminate_connection(ERR_INTERNAL);
    }

    if  (connect(this->client_socket, address, address_size) != 0) {
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
    this->tcp_buffer += temp; // Add another chunk
}
/**
  *   H    H  HOOOO   HHHO
  *   H    H  H    O  H   H
  *   H    H  H    O  HHHHO
  *   O    O  H    O  H
  *    OOOO   HOOOO   H
*/
void Client_Comms::connect_udp() { // Unfinished
    int family = AF_INET;
    int type = SOCK_DGRAM;
    int protocol = 0;
    this->client_socket = socket(family, type, protocol);
    if (this->client_socket <= 0) {
        perror("ERRORR: socket");
        std::cerr << "ERROR: Cannot create socket.\n";
        terminate_connection(ERR_INTERNAL);
    }

}

void Client_Comms::send_udp_message(const std::string &msg) {
    printf_debug("Sending UDP message.");
    auto server_address = this->host_name;
    int flags = 0;
    struct sockaddr *address = (struct sockaddr *) &server_address;
    int address_size = sizeof(server_address);
    int bytes_tx = sendto(client_socket, msg.c_str(), strlen(msg.c_str()),
                          flags, address, address_size);
    if (bytes_tx < 0) {
        perror("ERROR: sendto");
    }                          
    return;
}

std::string Client_Comms::receive_udp_message() {
    printf_debug("UDP receive not implemented.");
    return {};
}
