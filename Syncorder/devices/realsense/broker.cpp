#pragma once

#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>

// local
#include <Syncorder/error/exception.h>
#include <Syncorder/devices/common/broker_base.h>
#include <Syncorder/devices/realsense/model.h>


/**
 * @class Broker
 */

class RealsenseBroker : public TBBroker<RealsenseBufferData> {
public:
    RealsenseBroker() {}
    ~RealsenseBroker() {}

protected:
    void _process(const RealsenseBufferData& data) override {
        std::cout << "[RealsenseBroker] Processing timestamp\n";
    }
};