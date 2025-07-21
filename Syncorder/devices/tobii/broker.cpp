#pragma once

#include <any>
#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>

// local
#include <Syncorder/error/exception.h>
#include <Syncorder/devices/common/broker_base.h>
#include <Syncorder/devices/tobii/model.h>


/**
 * @class Broker
 */

class TobiiBroker : public TBBroker<TobiiBufferData> {
private:
    std::ofstream csv_;

public:
    TobiiBroker() {
        csv_.open("tobii_data.csv");
        csv_
            <<"left_x,"
            <<"left_y,"
            <<"right_x,"
            <<"right_y,"
            <<"left_valid,"
            <<"right_valid,"
            <<"left_pupil_diameter,"
            <<"right_pupil_diameter,"
            <<"left_pupil_valid,"
            <<"right_pupil_valid,"
            <<"sys_time_ms,"
            <<"device_timestamp,"
            <<"validity_flags\n";
    }
    ~TobiiBroker() {}

protected:
    void _process(const TobiiBufferData& data) override {
        _write(data);

        std::cout << "[TobiiBroker] Processing gaze data\n";
    }

private:
    void _write(const TobiiBufferData& data) {
        csv_
            << data.left_x_ << ","
            << data.left_y_ << ","
            << data.right_x_ << ","
            << data.right_y_ << ","
            << data.left_valid_ << ","
            << data.right_valid_ << ","
            << data.left_pupil_diameter_ << ","
            << data.right_pupil_diameter_ << ","
            << data.left_pupil_valid_ << ","
            << data.right_pupil_valid_ << ","
            << std::chrono::duration_cast<std::chrono::milliseconds>(data.sys_time_.time_since_epoch()).count() << ","
            << data.device_timestamp_ << ","
            << data.validity_flags_
            << "\n";
    }
};