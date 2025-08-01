#pragma once

#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

// local
#include <Syncorder/gonfig/gonfig.h>
#include <Syncorder/error/exception.h>
#include <Syncorder/devices/common/broker_base.h>
#include <Syncorder/devices/realsense/model.h>

#include <librealsense2/rs.hpp>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


/**
 * @helper: struct
 */

struct DepthAnalysis {
    uint16_t center_depth = 0;
    float valid_pixel_ratio = 0.0f;
    uint16_t min_depth = 65535;
    uint16_t max_depth = 0;
    uint16_t avg_depth = 0;
};

struct ColorAnalysis {
    uint8_t center_r = 0;
    uint8_t center_g = 0;
    uint8_t center_b = 0;
    uint8_t avg_brightness = 0;
};

struct FrameTask {
    rs2::frame color_frame;
    rs2::frame depth_frame;
    uint64_t frame_number;
    bool has_color;
    bool has_depth;
    std::string output_dir;
    
    FrameTask(const RealsenseBufferData& data, const std::string& output) 
        : color_frame(data.color_frame_)
        , depth_frame(data.depth_frame_)
        , frame_number(data.frame_number_)
        , has_color(data.has_color_)
        , has_depth(data.has_depth_)
        , output_dir(output) {}
};


/**
 * @class Broker
 */

class RealsenseBroker : public TBBroker<RealsenseBufferData> {
private:
    std::ofstream csv_;
    std::string output_;

public:
    RealsenseBroker() {
        output_ = gonfig.output_path + "realsense/";

        std::filesystem::create_directories(output_);

        csv_.open(output_ + "realsense_data.csv");
        csv_ 
            << "DeviceTimestamp,"
            << "SystemTime,"
            << "CenterDepth,"
            << "HasColor,"
            << "HasDepth\n";
    }

    ~RealsenseBroker() {
        csv_.close();
    }

public:
    void cleanup() {
        // _convert();
    }

protected:
    void _process(const RealsenseBufferData& data) override {
        auto sys_ms = std::chrono::duration_cast<std::chrono::milliseconds>(data.sys_time_.time_since_epoch()).count();
        
        uint16_t center_depth_mm = 0;
        if (data.has_depth_) {
            auto depth_frame = data.depth_frame_.as<rs2::depth_frame>();
            float distance_m = depth_frame.get_distance(depth_frame.get_width() / 2, depth_frame.get_height() / 2);
            center_depth_mm = static_cast<uint16_t>(distance_m * 1000);
        }

        std::cout << "Device timestamp (ms): " << data.device_timestamp_ << ", "
            << "System time (ms): " << sys_ms << ", "
            << "Offset: " << (sys_ms - data.device_timestamp_) << " ms\n";


        csv_ 
            << std::fixed << std::setprecision(2) << data.device_timestamp_ << ","
            << sys_ms << ","
            << center_depth_mm << ","
            << (data.has_color_ ? 1 : 0) << ","
            << (data.has_depth_ ? 1 : 0) << "\n";
    }
};