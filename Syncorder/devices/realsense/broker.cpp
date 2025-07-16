#pragma once

#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>

// local
#include <Syncorder/error/exception.h>
#include <Syncorder/devices/common/broker_base.h>
#include <Syncorder/devices/realsense/model.h>


/**
 * @class Broker
 */

class RealsenseBroker : public TBBroker<RealsenseBufferData> {
protected:
    void _process(const RealsenseBufferData& data) override {
        std::cout << "[RealsenseBroker] Processing frame #" << data.frame_number_ << std::endl;
    }
};