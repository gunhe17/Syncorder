#pragma once

// installed
#include <chrono>
#include <librealsense2/rs.hpp>


/**
 * @struct
 */

struct RealsenseBufferData {
    // frame data
    rs2::frameset frameset_;
    rs2::frame color_frame_;
    rs2::frame depth_frame_;
    
    // time
    std::chrono::system_clock::time_point sys_time_;
    double device_timestamp_;
    
    // metadata
    uint64_t frame_number_;
    bool has_depth_;
    bool has_color_;
    
public:
    RealsenseBufferData()
    : 
        has_depth_(false), 
        has_color_(false), 
        frame_number_(0) 
    {}

    RealsenseBufferData(
        rs2::frameset frameset,
        std::chrono::system_clock::time_point sys_time,
        double device_timestamp
    ) {
        frameset_ = frameset;
        sys_time_ = sys_time;
        device_timestamp_ = device_timestamp;
        
        // Extract frames
        if (auto depth = frameset.get_depth_frame()) {
            depth_frame_ = depth;
            has_depth_ = true;
        }
        
        if (auto color = frameset.get_color_frame()) {
            color_frame_ = color;
            has_color_ = true;
        }
        
        frame_number_ = frameset.get_frame_number();
    }
};