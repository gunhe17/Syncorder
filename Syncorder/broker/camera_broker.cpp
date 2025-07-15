#pragma once

#include <any>
#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>

// local
#include <Syncorder/error/exception.h>
#include <Syncorder/broker/common/base.h>
#include <Syncorder/model/camera_model.h>


/**
 * @class Broker
 */

class CameraBroker : public BaseBroker {
public:
    CameraBroker() {}
    ~CameraBroker() {}

protected:
    void _broker() override {
        if (!buffer_ || !dequeue_) {
            std::cout << "[DEBUG] Missing buffer or dequeue function!" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            return;  // 이제 조건부로만 실행됨
        }

        // Function pointer casting 검증
        typedef void* (*DequeueFunc)(void*);
        
        auto dequeue_func = reinterpret_cast<DequeueFunc>(dequeue_);

        void* data = nullptr;
        try {
            data = dequeue_func(buffer_);
        } catch (const std::exception& e) {
            std::cout << "[ERROR] Exception in dequeue_func: " << e.what() << std::endl;
            return;
        } catch (...) {
            std::cout << "[ERROR] Unknown exception in dequeue_func" << std::endl;
            return;
        }
        
        if (data != nullptr) {
            processed_count_++;
            
            _process(data);
        
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

private:
    void _process(void* data) {
        auto* camera_data = static_cast<CameraBufferData*>(data);
        std::cout << "[CameraBroker] Processing timestamp: " << camera_data->mf_ts_ << std::endl;
        delete camera_data;
    }
};