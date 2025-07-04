#pragma once

#include <iostream>


/**
 * @class exception
 */
class MediaFoundationError : public std::runtime_error {
public:
    MediaFoundationError(const std::string& msg) : std::runtime_error(msg) {}
};