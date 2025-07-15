#pragma once

#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>

// local
#include <Syncorder/error/exception.h>
#include <Syncorder/broker/common/base.h>
#include <Syncorder/model/realsense_model.h>


/**
 * @class Broker
 */

class RealsenseBroker : public BaseBroker {
public:
    RealsenseBroker() {}
    ~RealsenseBroker() {}

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
        auto* realsense_data = static_cast<RealsenseBufferData*>(data);
        
        std::cout << "[RealsenseBroker] Processing frame #" 
                  << realsense_data->frame_number_ 
                  << " (Depth: " << (realsense_data->has_depth_ ? "Yes" : "No")
                  << ", Color: " << (realsense_data->has_color_ ? "Yes" : "No") << ")" << std::endl;
        
        delete realsense_data;
    }
};