#pragma once

#include <string>
#include <iostream>
#include <stdexcept> // std::stoi exceptions
#include <regex>
#include <thread>

#define ERR_MISSING 10
#define ERR_INVALID 11
#define ERR_INTERNAL 99

#ifdef DEBUG_PRINT
#define printf_debug(format, ...) fprintf(stderr, "%s:%-4d | %15s | " format "\n", __FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#define printf_debug(format, ...) (0)
#endif

class Toolkit
{
    private:
        /* data */
    public:
        static int catch_stoi(const std::string &str, int size, const std::string &flag);
        static bool only_allowed_chars(const std::string &str, const std::string &regex);
        static bool only_printable_chars(const std::string &str, bool allow_space_and_lf = false); // range (0x21-7E) + space and line feed (0x0A,0x20)
};
