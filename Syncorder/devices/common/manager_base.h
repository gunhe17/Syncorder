#pragma once

#include <string>


/**
 * @class Base Manager
 */

class BManager {
public:
    virtual ~BManager() = default;

public:
    virtual void setup() = 0;
    virtual void warmup() = 0;
    virtual void start() = 0;

    virtual std::string __name__() const = 0;
};