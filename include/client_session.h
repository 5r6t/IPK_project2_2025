 /**
 * @file client_session.h
 * @brief IPK project 2 - Chat client
 * @date 5-4-2025
 * Author: Jaroslav Mervart, xmervaj00
 */

 #pragma once

 #include "client_init.h"
 #include <string>
 #include <vector>
 
 #include <iostream>
 #include <sstream>
 
 #include <netinet/in.h>
 #include <arpa/inet.h>
 #include <sys/socket.h>
 #include <netdb.h>
 #include <unistd.h> // read(), close()
 #include <thread>   // std::thread
 
 
 enum class ClientState {
     Start,
     Auth,
     Open,
     Join,
     End
 };
 
 class Client_Session {
     public:
         Client_Session(const Client_Init &config);
         void run();
 
     private:
         void handle_command(const std::string& line);
         void handle_chat_msg(const std::string& line);
         void auth(const std::vector<std::string>& args);   // sends auth request
         void join(const std::vector<std::string>& args);   // sends join rqst
         void rename(const std::vector<std::string>& args); // sends rename rqst
         void print_local_help();
 
         int sockfd = -1;
         std::string display_name;
         bool is_authenticated = false;
         ClientState state = ClientState::Start;
 
         const Client_Init &config;
 };