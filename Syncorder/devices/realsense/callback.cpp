#pragma once

// installed
#include <librealsense2/rs.hpp>

// local
#include <Syncorder/devices/realsense/buffer.cpp> //TODO: include buffer
#include <Syncorder/error/exception.h>


/**
 * @class Callback
 */

class RealsenseCallback {
private:
    static inline RealsenseCallback* instance_ = nullptr;
    void* buffer_;

    // flag
    std::atomic<bool> first_frame_received_;

public:
    RealsenseCallback() {}
    ~RealsenseCallback() {}

public:
    void setup(void* buffer) {
        instance_ = this;
        buffer_ = buffer;

        first_frame_received_.store(false);
    }

    bool warmup() {
        auto start = std::chrono::steady_clock::now();
        auto end = std::chrono::milliseconds(10000);
        
        while (!first_frame_received_.load()) {
            auto elapsed = std::chrono::steady_clock::now() - start;
            if (elapsed >= end) {
                std::cout << "[ERROR] warmup timeout\n";
                return false;
            }
        }
        
        std::cout << "[RealSense] warmup clear\n";

        return true;
    }

    static void onFrameset(const rs2::frame& frame) {
        if (instance_) {
            instance_->_onFrameset(frame);
        }
    }

private:
    void _onFrameset(const rs2::frame& frame) {
        // flag
        if (!first_frame_received_.load()) first_frame_received_.store(true);

        if (rs2::frameset fs = frame.as<rs2::frameset>()) {
            if (buffer_) {
                auto* rs_buffer = static_cast<RealsenseBuffer*>(buffer_);
                RealsenseBufferData data = _map(fs);
                rs_buffer->enqueue(std::move(data));
            }
        }
    }

private:
    RealsenseBufferData _map(const rs2::frameset& fs) {
        return RealsenseBufferData(
            fs,
            std::chrono::system_clock::now(),
            fs.get_timestamp()
        );
    }
};