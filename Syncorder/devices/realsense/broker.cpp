#pragma once

#include <any>
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
private:
    std::ofstream csv_;

public:
    RealsenseBroker() {
        csv_.open("realsense.csv");
        csv_<< "has_color,has_depth,frame_number,sys_time_ms,device_timestamp\n";

    }
    ~RealsenseBroker() {}

protected:
    void _process(const RealsenseBufferData& data) override {
        _write(data);

        std::cout << "[RealsenseBroker] Processing timestamp\n";
    }

private:
    void _write(const RealsenseBufferData& data) {
        csv_
            << (data.has_color_ ? "1" : "0") << ","
            << (data.has_depth_ ? "1" : "0") << ","
            << data.frame_number_ << ","
            << std::chrono::duration_cast<std::chrono::milliseconds>(data.sys_time_.time_since_epoch()).count() << ","
            << data.device_timestamp_
            << "\n";
    }

};