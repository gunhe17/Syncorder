#pragma once

// installed
#include <librealsense2/rs.hpp>

// local
#include <Syncorder/buffer/realsense_buffer.cpp>
#include <Syncorder/error/exception.h>


/**
 * @class Callback
 */

class RealsenseCallback {
private:
    static inline RealsenseCallback* instance_ = nullptr;
    void* buffer_;

    // log
    std::atomic<int> frame_count_;

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
                RealsenseBufferData data(
                    fs,
                    std::chrono::system_clock::now(),
                    fs.get_timestamp()
                );
                rs_buffer->enqueue(std::move(data));
            }

            frame_count_++;
            
            auto color_frame = fs.get_color_frame();
            auto depth_frame = fs.get_depth_frame();
            
            _print(color_frame, depth_frame);
        }
    }

    void _print(const rs2::frame& color_frame, const rs2::frame& depth_frame) {
        auto now = std::chrono::high_resolution_clock::now();
        
        if (color_frame && depth_frame) {
            auto color_vf = color_frame.as<rs2::video_frame>();
            auto depth_vf = depth_frame.as<rs2::video_frame>();
            
            if (color_vf && depth_vf) {
                std::cout << "RS: Warmup frame " << frame_count_.load() 
                        << " - C:" << color_vf.get_width() << "x" << color_vf.get_height()
                        << " D:" << depth_vf.get_width() << "x" << depth_vf.get_height() 
                        << std::endl;
            }
        }
    }
};