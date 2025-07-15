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

class CameraBroker : public TBBroker<CameraBufferData> {
public:
    CameraBroker() {}
    ~CameraBroker() {}

protected:
    void _process(const CameraBufferData& data) override {
        std::cout << "[CameraBroker] Processing timestamp: " << data.mf_ts_ << std::endl;
    }
};