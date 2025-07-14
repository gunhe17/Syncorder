#pragma once

#include <chrono>
#include <array>
#include <atomic>
#include <optional>
#include <iostream>
#include <memory>

// installed
#include <librealsense2/rs.hpp>

// local
#include <Syncorder/error/exception.h>
#include <Syncorder/buffer/common/base.h>


/**
 * @struct Data
 */

struct RealsenseBufferData {
    // frame data
    rs2::frameset frameset_;
    rs2::frame color_frame_;
    rs2::frame depth_frame_;
    
    // time
    std::chrono::system_clock::time_point sys_time_;
    double rs_timestamp_;
    
    // metadata
    int frame_number_;
    rs2_timestamp_domain timestamp_domain_;
    
    RealsenseBufferData() {}
    
    RealsenseBufferData(
        rs2::frameset frameset,
        std::chrono::system_clock::time_point sys_time,
        double rs_timestamp,
        int frame_number = 0,
        rs2_timestamp_domain timestamp_domain = RS2_TIMESTAMP_DOMAIN_HARDWARE_CLOCK
    ) {
        frameset_ = std::move(frameset);
        color_frame_ = frameset_.get_color_frame();
        depth_frame_ = frameset_.get_depth_frame();
        sys_time_ = sys_time;
        rs_timestamp_ = rs_timestamp;
        frame_number_ = frame_number;
        timestamp_domain_ = timestamp_domain;
    }
};


/**
 * @class Buffer
 */

constexpr std::size_t REALSENSE_RING_BUFFER_SIZE = 1024;

class RealsenseBuffer : public BBuffer<RealsenseBufferData, REALSENSE_RING_BUFFER_SIZE> {
protected:
    void onOverflow() noexcept override { std::cout << "[RealsenseBuffer Warning] Buffer overflow\n"; }
};