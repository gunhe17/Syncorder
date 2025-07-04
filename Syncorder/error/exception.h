#pragma once

#include <iostream>
#include <stdexcept>
#include <string>

/**
 * @macro EXCEPTION - Exception handling wrapper
 */
#define EXCEPTION(...)                      \
    try {                                   \
        __VA_ARGS__                         \
    } catch (const std::exception& e) {     \
        std::cout << "Error: " << e.what(); \
        return false;                       \
    }


/**
 * @class MediaFoundationError - Media Foundation specific error
 */
class MediaFoundationError : public std::runtime_error {
public:
    MediaFoundationError(const std::string& msg) : std::runtime_error(msg) {}
};

/**
 * @class DeviceError - Base device error class
 */
class DeviceError : public std::runtime_error {
public:
    DeviceError(const std::string& message, int code = 1000) 
        : std::runtime_error(message), code_(code) {}
    
    int getCode() const { return code_; }

private:
    int code_;
};