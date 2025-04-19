/**
 * @file tools.h 
 * @brief IPK project 2 - Client for a chat server
 * @date 15-4-2025
 * Author: Jaroslav Mervart, xmervaj00
*/

#pragma once

#include <string>
#include <iostream>
#include <stdexcept> // std::stoi exceptions
#include <regex>
#include <vector>
#include <arpa/inet.h>

#define ERR_MISSING  10
#define ERR_INVALID  11
#define ERR_TIMEOUT  12
#define ERR_CONNECTION 13
#define ERR_SERVER   14
#define ERR_INTERNAL 99

#ifdef DEBUG_PRINT
#define printf_debug(format, ...) \
    do { \
        if (*#format) { \
            fprintf(stderr, "%s:%-4d | %15s | " format "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
        } \
        else { \
            fprintf(stderr, "%s:%-4d | %15s | \n", __FILE__, __LINE__, __func__); \
        } \
    } while (0)
#else
#define printf_debug(format, ...) (0)
#endif

class Toolkit
{
    private:
        // nothing so far
    public:
        static int catch_stoi(const std::string &str, int size, const std::string &flag);
        static bool only_allowed_chars(const std::string &str, const std::string &regex);
        static bool only_printable_chars(const std::string &str, bool allow_space_and_lf = false); // range (0x21-7E) + space and line feed (0x0A,0x20)
        
        static void append_uint8(std::vector<uint8_t>& buf, uint8_t value);
        static void append_uint16(std::vector<uint8_t>& buf, uint16_t value);
        static void append_string(std::vector<uint8_t>& buf, const std::string& s);
        
        static std::vector<uint8_t> build_confirm (uint16_t ref_msg_id);

        static std::vector<uint8_t> build_reply (
            uint16_t msg_id,
            uint8_t result, // 0 or 1
            uint16_t ref_msg_id, // id of message being replied to
            const std::string& msg_contents
        );

        static std::vector<uint8_t> build_auth (
            uint16_t msg_id, 
            const std::string& username,    
            const std::string& display_name, 
            const std::string& secret
        );
        
        static std::vector<uint8_t> build_join (
            uint16_t msg_id,
            const std::string& channel_id,
            const std::string& display_name
        );

        // err is identical to msg, except msg_type  
        // thus is_error has been added
        static std::vector<uint8_t> build_msg (
            uint16_t msg_id,
            const std::string& display_name,
            const std::string& msg_contents,
            bool is_error = false
        );

        static std::vector<uint8_t> build_ping (uint16_t msg_id);
        static std::vector<uint8_t> build_bye  (uint16_t msg_id, const std::string& display_name);
};
