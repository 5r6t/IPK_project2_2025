/**
 * @file tools.cpp 
 * @brief IPK project 2 - Client for a chat server
 * @date 13-4-2025
 * Author: Jaroslav Mervart, xmervaj00
*/

#include "tools.h"

int Toolkit::catch_stoi(const std::string &str, int size, const std::string &flag) 
{
    try {
        int value = std::stoi(str);
        printf_debug("Parsed %s = %d", flag.c_str(), value);
        if (value < 0 || value > size) {
            throw std::out_of_range("Out of range");
        }
        return value;
    } catch (...) {
        std::cerr << "Error: " 
                  << flag 
                  << " must be a positive number <= " 
                  << size << "\n";
        exit(ERR_INVALID);
    }
}

bool Toolkit::only_allowed_chars(const std::string &str, const std::string &regex) 
{
    const std::regex pattern(regex);
    return std::regex_match(str, pattern);
}

bool Toolkit::only_printable_chars(const std::string &str, bool allow_space_and_lf) 
{
    if (allow_space_and_lf) {
        static const std::regex pattern("^[\\x0A\\x20-\\x7E]*$");
        return std::regex_match(str, pattern);
    } else {
        static const std::regex pattern("^[\\x21-\\x7E]*$");
        return std::regex_match(str, pattern);
    }
}


void Toolkit::append_uint8(std::vector<uint8_t>& buf, uint8_t value) 
{
    buf.push_back(value);
}

/**
 * @brief function to spread 2bytes into vector<uint8_t> in network byte order
 */
void Toolkit::append_uint16(std::vector<uint8_t>& buf, uint16_t value) 
{
    uint16_t net = htons(value);
    auto ptr = reinterpret_cast<uint8_t*>(&net);
    buf.push_back(ptr[0]);
    buf.push_back(ptr[1]);
}
void Toolkit::append_string(std::vector<uint8_t>& buf, const std::string& s) 
{
    buf.insert(buf.end(), s.begin(), s.end());
    buf.push_back(0); // null terminator - every string now terminated with 0
}

std::vector<uint8_t> Toolkit::build_confirm (uint16_t ref_msg_id) 
{
    std::vector<uint8_t> packet;
    append_uint8(packet, 0x00);
    append_uint16(packet, ref_msg_id);
    return packet;
}

std::vector<uint8_t> Toolkit::build_reply (uint16_t msg_id, 
    uint8_t result, uint16_t ref_msg_id, const std::string& msg_contents)
{
    std::vector<uint8_t> packet;
    append_uint8(packet, 0x01);
    append_uint16(packet, msg_id);
    append_uint8(packet, result);
    append_uint16(packet, ref_msg_id);
    append_string(packet, msg_contents);
    return packet;
}

std::vector<uint8_t> Toolkit::build_auth (uint16_t msg_id, 
    const std::string& username, const std::string& display_name, 
    const std::string& secret)
{
    std::vector<uint8_t> packet;
    append_uint8(packet, 0x02);
    append_uint16(packet, msg_id);
    append_string(packet, username);
    append_string(packet, display_name);
    append_string(packet, secret);
    return packet;   
}

std::vector<uint8_t> Toolkit::build_join (
    uint16_t msg_id, const std::string& channel_id, 
    const std::string& display_name)
{
    std::vector<uint8_t> packet;
    append_uint8(packet, 0x03);
    append_uint16(packet, msg_id);
    append_string(packet, channel_id);
    append_string(packet, display_name);
    return packet;
}

/**
 * @brief function to put data in vector<uint8_t>
 * @param is_error if true, replaces msg_type with err type
 */
std::vector<uint8_t> Toolkit::build_msg (
    uint16_t msg_id, const std::string& display_name,
    const std::string& msg_contents, bool is_error)
{
    std::vector<uint8_t> packet;

    append_uint8(packet, is_error ? 0xFE : 0x04);
    append_uint16(packet, msg_id);
    append_string(packet, display_name);
    append_string(packet, msg_contents);
    return packet;
}

std::vector<uint8_t> Toolkit::build_ping (uint16_t msg_id)
{
    std::vector<uint8_t> packet;
    append_uint8(packet, 0xFD);
    append_uint16(packet, msg_id);
    return packet;
    
}
std::vector<uint8_t> Toolkit::build_bye  (uint16_t msg_id, 
    const std::string& display_name)
{
    std::vector<uint8_t> packet;
    append_uint8(packet, 0xFF);
    append_uint16(packet, msg_id);
    append_string(packet, display_name);
    return packet;
}