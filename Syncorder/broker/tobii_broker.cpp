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

class TobiiBroker : public TBBroker<TobiiBufferData> {
protected:
    void _process(const TobiiBufferData& data) override {
        std::cout << "[TobiiBroker] Processing gaze data - "
                  << "Left: (" << std::fixed << std::setprecision(3) << data.left_x_ << ", " << data.left_y_ << ") "
                  << (data.left_valid_ ? "Valid" : "Invalid") << " |" 
                  << "Right: (" << data.right_x_ << ", " << data.right_y_ << ") "
                  << (data.right_valid_ ? "Valid" : "Invalid") << std::endl;
    }
};