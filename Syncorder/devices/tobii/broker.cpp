#pragma once

#include <any>
#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <filesystem>

// local
#include <Syncorder/gonfig/gonfig.h>
#include <Syncorder/error/exception.h>
#include <Syncorder/devices/common/broker_base.h>
#include <Syncorder/devices/tobii/model.h>


/**
 * @class Broker
 */

class TobiiBroker : public TBBroker<TobiiBufferData> {
private:
    std::ofstream csv_;
    std::string output_;

public:
    TobiiBroker() {
        output_ = gonfig.output_directory + "tobii/";

        std::filesystem::create_directories(output_);

        csv_.open(output_ + "tobii_data.csv");
        csv_
            <<"left_gaze_display_x,"
            <<"left_gaze_display_y,"
            <<"right_gaze_display_x,"
            <<"right_gaze_display_y,"
            <<"left_gaze_validity,"
            <<"right_gaze_validity,"
            <<"left_pupil_diameter,"
            <<"right_pupil_diameter,"
            <<"left_pupil_validity,"
            <<"right_pupil_validity,"
            <<"system_time_stamp,"
            <<"device_time_stamp,"
            <<"left_eye_detected,"
            <<"right_eye_detected,"
            <<"is_tracking,"
            <<"overall_validity\n";
    }
    ~TobiiBroker() {}

public:
    void cleanup() {}

protected:
    void _process(const TobiiBufferData& data) override {
        _write(data);

        std::cout << "[TobiiBroker] Processing gaze data\n";
    }

private:
    void _write(const TobiiBufferData& data) {
        csv_
            << data.left_gaze_point_display_x << ","
            << data.left_gaze_point_display_y << ","
            << data.right_gaze_point_display_x << ","
            << data.right_gaze_point_display_y << ","
            << data.left_gaze_point_validity << ","
            << data.right_gaze_point_validity << ","
            << data.left_pupil_diameter << ","
            << data.right_pupil_diameter << ","
            << data.left_pupil_validity << ","
            << data.right_pupil_validity << ","
            << data.system_time_stamp << ","
            << data.device_time_stamp << ","
            << data.left_eye_detected << ","
            << data.right_eye_detected << ","
            << data.is_tracking << ","
            << data.overall_validity
            << "\n";
    }
};