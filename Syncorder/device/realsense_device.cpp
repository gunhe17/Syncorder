#pragma once

#include <algorithm>
#include <thread>

// installed
#include <librealsense2/rs.hpp>

// local
#include <Syncorder/error/exception.h>
#include <Syncorder/device/base.h>


/**
 * @class Callback
 */

class RealsenseDevice : public BDevice {
private:
    rs2::pipeline pipe_;
    rs2::config config_;
    
    void* callback_;

public:
    RealsenseDevice(int device_id = 0)
    : 
        BDevice(device_id)
    {}
    
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
        if (!callback_) {
            throw RealsenseDeviceError("Callback not set before warmup");
        }
        
        auto func = reinterpret_cast<void(*)(const rs2::frame&)>(callback_);
        pipe_.start(func);
        
        std::this_thread::sleep_for(std::chrono::seconds(3));
        
        return true;
    }
    
    bool _start() override {
        return true;
    }
    
    bool _stop() override {
        pipe_.stop();

        return true;
    }
    
    bool _cleanup() override {

        return true;
    }

private:
    void _createConfig() {
        config_.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_RGB8, 30);
        config_.enable_stream(RS2_STREAM_DEPTH, 640, 480, RS2_FORMAT_Z16, 30);
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
};