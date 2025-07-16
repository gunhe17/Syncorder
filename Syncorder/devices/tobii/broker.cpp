#pragma once

#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>
#include <iomanip>

// local
#include <Syncorder/error/exception.h>
#include <Syncorder/devices/common/broker_base.h>
#include <Syncorder/devices/tobii/model.h>


/**
 * @class Broker
 */

class TobiiBroker : public TBBroker<TobiiBufferData> {
protected:
    void _process(const TobiiBufferData& data) override {
        std::cout << "[TobiiBroker] Processing gaze data\n";
    }
};