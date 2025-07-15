#pragma once

#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>
#include <iomanip>

// local
#include <Syncorder/error/exception.h>
#include <Syncorder/broker/common/base.h>
#include <Syncorder/model/tobii_model.h>


/**
 * @class Broker
 */

class TobiiBroker : public BaseBroker {
public:
    TobiiBroker() {}
    ~TobiiBroker() {}

protected:
    void _broker() override {
        if (!buffer_ || !dequeue_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            return;
        }

        typedef void* (*DequeueFunc)(void*);
        auto dequeue_func = reinterpret_cast<DequeueFunc>(dequeue_);

        void* data = dequeue_func(buffer_);
        
        if (data != nullptr) {
            processed_count_++;
            _process(data);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

private:
    void _process(void* data) {
        auto* tobii_data = static_cast<TobiiBufferData*>(data);
        
        std::cout << "[TobiiBroker] Processing gaze data - "
                  << "Left: (" << std::fixed << std::setprecision(3) 
                  << tobii_data->left_x_ << ", " << tobii_data->left_y_ 
                  << ") " << (tobii_data->left_valid_ ? "Valid" : "Invalid")
                  << " | Right: (" << tobii_data->right_x_ << ", " << tobii_data->right_y_ 
                  << ") " << (tobii_data->right_valid_ ? "Valid" : "Invalid") << std::endl;
        
        delete tobii_data;
    }
};