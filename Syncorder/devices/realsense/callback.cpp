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

public:
    RealsenseCallback() {}
    ~RealsenseCallback() {}

public:
    void setup(void* buffer) {
        instance_ = this;

        buffer_ = buffer;
    }

    static void onFrameset(const rs2::frame& frame) {
        if (instance_) {
            instance_->_onFrameset(frame);
        }
    }

private:
    void _onFrameset(const rs2::frame& frame) {
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