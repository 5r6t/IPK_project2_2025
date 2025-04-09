#include "tools.h"


int catch_stoi(const std::string &str, int size, const std::string &flag) {
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
