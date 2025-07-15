#pragma once

#include <any>
#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>

// local
#include <Syncorder/error/exception.h>
#include <Syncorder/devices/common/broker_base.h>
#include <Syncorder/devices/camera/model.h>


/**
 * @class Broker
 */

class CameraBroker : public TBBroker<CameraBufferData> {
public:
    CameraBroker() {}
    ~CameraBroker() {}

protected:
    void _process(const CameraBufferData& data) override {
        std::cout << "[CameraBroker] Processing timestamp: " << data.mf_ts_ << std::endl;
    }
};