#pragma once

#include <thread>
#include <chrono>

/**
 * @class Base Broker
 */

class BaseBroker {
protected:
    // buffer
    void* buffer_;
    void* dequeue_;

    // flag
    std::atomic<bool> running_;
    std::atomic<int> processed_count_;

    std::thread processing_thread_;

public:
    BaseBroker() 
    : 
        running_(false), 
        processed_count_(0) 
    {}
    
    virtual ~BaseBroker() { stop(); }

public:
    void setup(void* buffer, void* dequeue) {
        buffer_ = buffer;
        dequeue_ = dequeue;
    }

    void run() {
        // flag
        running_ = true;

        // thread
        processing_thread_ = std::thread(&BaseBroker::_loop, this);
    }

    void stop() {
        // flag
        running_ = false;

        // thread
        if (processing_thread_.joinable()) processing_thread_.join();
    }

protected:
    virtual void _broker() = 0;

private:
    void _loop() {
        while (running_) _broker();
    }
};