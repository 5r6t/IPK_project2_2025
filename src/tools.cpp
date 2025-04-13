/**
 * @file tools.cpp 
 * @brief IPK project 2 - Client for a chat server
 * @date 13-4-2025
 * Author: Jaroslav Mervart, xmervaj00
*/

#include "tools.h"

int Toolkit::catch_stoi(const std::string &str, int size, const std::string &flag) {
    try {
        int value = std::stoi(str);
        printf_debug("Parsed %s = %d", flag.c_str(), value);
        if (value < 0 || value > size) {
            throw std::out_of_range("Out of range");
        }
        return value;
    } catch (...) {
        std::cerr << "Error: " << flag << " must be a positive number <= " << size << "\n";
        exit(ERR_INVALID);
    }
}

bool Toolkit::only_allowed_chars(const std::string &str, const std::string &regex) {
    const std::regex pattern(regex);
    return std::regex_match(str, pattern);
}

bool Toolkit::only_printable_chars(const std::string &str, bool allow_space_and_lf) {
    if (allow_space_and_lf) {
        static const std::regex pattern("^[\\x0A\\x20-\\x7E]*$");
        return std::regex_match(str, pattern);
    } else {
        static const std::regex pattern("^[\\x21-\\x7E]*$");
        return std::regex_match(str, pattern);
    }
}

