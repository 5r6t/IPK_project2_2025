#pragma once

#include <string>
#include <iostream>
#include <stdexcept> // std::stoi exceptions


#define ERR_MISSING 10
#define ERR_INVALID 11
#define ERR_INTERNAL 99

#ifdef DEBUG_PRINT
#define printf_debug(format, ...) fprintf(stderr, "%s:%-4d | %15s | " format "\n", __FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#define printf_debug(format, ...) (0)
#endif

int catch_stoi(const std::string &str, int size, const std::string &flag);
