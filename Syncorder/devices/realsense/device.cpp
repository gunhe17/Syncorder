﻿#pragma once

#include <algorithm>
#include <thread>
#include <filesystem>

// installed
#include <librealsense2/rs.hpp>

// local
#include <Syncorder/gonfig/gonfig.h>
#include <Syncorder/error/exception.h>
#include <Syncorder/devices/common/device_base.h>


/**
 * @class Callback
 */

class RealsenseDevice : public BDevice {
private:
    rs2::pipeline pipe_;
    rs2::config config_;
    
    void* callback_;

    // *BAG
    std::string bag_path_;

public:
    RealsenseDevice(int device_id = 0)
    : 
        BDevice(device_id)
    {
        auto unique = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        bag_path_ = gonfig.output_path + "realsense/" + std::to_string(unique) + ".bag";
    }
    
    ~RealsenseDevice() {
        cleanup();
    }

public:
    bool pre_setup(void* callback) {
        callback_ = callback;

        return true;
    }
    
    bool _setup() override {
        _createConfig();
        _validateDevice();
        
        return true;
    }
    
    bool _warmup() override {
        _readSource();
        
        return true;
    }
    
    bool _start() override {
        return true;
    }
    
    bool _stop() override {
        auto device = pipe_.get_active_profile().get_device();
        if (auto recorder = device.as<rs2::recorder>()) recorder.pause();

        pipe_.stop();

        return true;
    }
    
    bool _cleanup() override {

        return true;
    }

private:
    void _createConfig() {
        config_.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_RGB8, 60);
        config_.enable_stream(RS2_STREAM_DEPTH, 640, 480, RS2_FORMAT_Z16, 60);

        std::filesystem::create_directories(std::filesystem::path(bag_path_).parent_path());
        config_.enable_record_to_file(bag_path_);
    }
    
    void _validateDevice() {
        rs2::context ctx;
        auto device_list = ctx.query_devices();
        
        if (device_list.size() == 0) {
            throw RealsenseDeviceError("No RealSense devices found");
        }
        
        if (device_id_ >= static_cast<int>(device_list.size())) {
            throw RealsenseDeviceError("Device index " + std::to_string(device_id_) + " out of range (0-" + std::to_string(device_list.size()-1) + ")");
        }
    }

    void _readSource() {
        if (!callback_) {
            throw RealsenseDeviceError("Callback not set before warmup");
        }
        
        auto func = reinterpret_cast<void(*)(const rs2::frame&)>(callback_);
        pipe_.start(config_, func);
    }
};