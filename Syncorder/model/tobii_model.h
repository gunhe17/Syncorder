#pragma once

#include <chrono>
#include "tobii_research.h"


/**
 * @struct TobiiBufferData - Global Tobii Data Structure
 */

struct TobiiBufferData {
    // gaze data
    double left_x_, left_y_;
    double right_x_, right_y_;
    bool left_valid_, right_valid_;
    
    // pupil data
    double left_pupil_diameter_;
    double right_pupil_diameter_;
    bool left_pupil_valid_, right_pupil_valid_;
    
    // time
    std::chrono::system_clock::time_point sys_time_;
    int64_t device_timestamp_;
    
    // metadata
    int validity_flags_;
    
    TobiiBufferData() : 
        left_valid_(false), right_valid_(false),
        left_pupil_valid_(false), right_pupil_valid_(false),
        validity_flags_(0) {}
    
    TobiiBufferData(
        double left_x, double left_y,
        double right_x, double right_y,
        bool left_valid, bool right_valid,
        std::chrono::system_clock::time_point sys_time,
        int64_t device_timestamp,
        int validity_flags = 0
    ) {
        left_x_ = left_x; left_y_ = left_y;
        right_x_ = right_x; right_y_ = right_y;
        left_valid_ = left_valid; right_valid_ = right_valid;
        sys_time_ = sys_time;
        device_timestamp_ = device_timestamp;
        validity_flags_ = validity_flags;
        
        // Initialize pupil data as invalid by default
        left_pupil_valid_ = right_pupil_valid_ = false;
        left_pupil_diameter_ = right_pupil_diameter_ = 0.0;
    }
};