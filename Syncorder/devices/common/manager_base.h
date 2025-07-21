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
    virtual void stop() = 0;
    virtual void cleanup() = 0;

    virtual std::string __name__() const = 0;

    virtual bool __is_setup__() const { return is_setup_.load(); }
    virtual bool __is_warmup__() const { return is_warmup_.load(); }
    virtual bool __is_running__() const { return is_running_.load(); }

protected:
    std::atomic<bool> is_setup_{false};
    std::atomic<bool> is_warmup_{false};
    std::atomic<bool> is_running_{false};
};