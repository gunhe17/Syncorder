#pragma once

// installed
#include "tobii_research.h"
#include "tobii_research_eyetracker.h"
#include "tobii_research_streams.h"

// local
#include <Syncorder/devices/tobii/buffer.cpp> //TODO: include buffer
#include <Syncorder/error/exception.h>


/**
 * @class Callback
 */

class TobiiCallback {
private:
    static inline TobiiCallback* instance_ = nullptr;
    
    void* buffer_;

public:
    TobiiCallback() {}
    ~TobiiCallback() {}

public:
    void setup(void* buffer) {
        instance_ = this;

        buffer_ = buffer;
    }

    static void onGaze(TobiiResearchGazeData* gaze_data, void* user_data) {
        auto* callback_instance = static_cast<TobiiCallback*>(user_data);
        if (callback_instance) {
            callback_instance->_onGaze(gaze_data);
        }
    }

private:
    void _onGaze(TobiiResearchGazeData* gaze_data) {
        if (!gaze_data) return;
        
        bool left_valid = (gaze_data->left_eye.gaze_point.validity == TOBII_RESEARCH_VALIDITY_VALID);
        bool right_valid = (gaze_data->right_eye.gaze_point.validity == TOBII_RESEARCH_VALIDITY_VALID);
        
        double left_x = left_valid ? gaze_data->left_eye.gaze_point.position_on_display_area.x : -1.0;
        double left_y = left_valid ? gaze_data->left_eye.gaze_point.position_on_display_area.y : -1.0;
        double right_x = right_valid ? gaze_data->right_eye.gaze_point.position_on_display_area.x : -1.0;
        double right_y = right_valid ? gaze_data->right_eye.gaze_point.position_on_display_area.y : -1.0;
        
        int64_t timestamp = gaze_data->device_time_stamp;
        
        _process(left_x, left_y, right_x, right_y, left_valid, right_valid, timestamp);
    }

    void _process(
        double left_x, 
        double left_y, 
        double right_x, 
        double right_y, 
        bool left_valid, 
        bool right_valid, 
        int64_t timestamp
    ) {
        if (buffer_) {
            auto* tobii_buffer = static_cast<TobiiBuffer*>(buffer_);
            TobiiBufferData data = _map(left_x, left_y, right_x, right_y, left_valid, right_valid, timestamp);
            tobii_buffer->enqueue(std::move(data));
        }
    }

    TobiiBufferData _map(
        double left_x, double left_y,
        double right_x, double right_y,
        bool left_valid, bool right_valid,
        int64_t timestamp
    ) {
        return TobiiBufferData(
            left_x, left_y, right_x, right_y,
            left_valid, right_valid,
            std::chrono::system_clock::now(),
            timestamp
        );
    }
};