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

class RealsenseBroker : public TBBroker<RealsenseBufferData> {
protected:
    void _process(const RealsenseBufferData& data) override {
        std::cout << "[RealsenseBroker] Processing frame #" 
                  << data.frame_number_ 
                  << " (Depth: " << (data.has_depth_ ? "Yes" : "No")
                  << ", Color: " << (data.has_color_ ? "Yes" : "No") << ")" << std::endl;
    }
};