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

    // flag
    std::atomic<bool> first_frame_received_;

public:
    TobiiCallback() {}
    ~TobiiCallback() {}

public:
    void setup(void* buffer) {
        instance_ = this;
        buffer_ = buffer;
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
        
        std::cout << "[Tobii] warmup clear\n";

        return true;
    }

    static void onGaze(TobiiResearchGazeData* gaze_data, void* user_data) {
        auto* callback_instance = static_cast<TobiiCallback*>(user_data);
        if (callback_instance) {
            callback_instance->_onGaze(gaze_data);
        }
    }

private:
    void _onGaze(TobiiResearchGazeData* gaze_data) {
        if (!first_frame_received_.load()) first_frame_received_.store(true);
        if (!gaze_data || !buffer_) return;

        auto* tobii_buffer = static_cast<TobiiBuffer*>(buffer_);
        TobiiBufferData data = _map(gaze_data);
        tobii_buffer->enqueue(std::move(data));
    }

private:
    TobiiBufferData _map(TobiiResearchGazeData* gaze_data) {
        TobiiBufferData data = {};
        
        // timestamp
        data.device_time_stamp = gaze_data->device_time_stamp;
        data.system_time_stamp = gaze_data->system_time_stamp;
        
        // left gaze point
        data.left_gaze_point_display_x = gaze_data->left_eye.gaze_point.position_on_display_area.x;
        data.left_gaze_point_display_y = gaze_data->left_eye.gaze_point.position_on_display_area.y;
        data.left_gaze_point_3d_x = gaze_data->left_eye.gaze_point.position_in_user_coordinates.x;
        data.left_gaze_point_3d_y = gaze_data->left_eye.gaze_point.position_in_user_coordinates.y;
        data.left_gaze_point_3d_z = gaze_data->left_eye.gaze_point.position_in_user_coordinates.z;
        data.left_gaze_point_validity = gaze_data->left_eye.gaze_point.validity;

        // right gaze point
        data.right_gaze_point_display_x = gaze_data->right_eye.gaze_point.position_on_display_area.x;
        data.right_gaze_point_display_y = gaze_data->right_eye.gaze_point.position_on_display_area.y;
        data.right_gaze_point_3d_x = gaze_data->right_eye.gaze_point.position_in_user_coordinates.x;
        data.right_gaze_point_3d_y = gaze_data->right_eye.gaze_point.position_in_user_coordinates.y;
        data.right_gaze_point_3d_z = gaze_data->right_eye.gaze_point.position_in_user_coordinates.z;
        data.right_gaze_point_validity = gaze_data->right_eye.gaze_point.validity;

        // left gaze origin
        data.left_gaze_origin_x = gaze_data->left_eye.gaze_origin.position_in_user_coordinates.x;
        data.left_gaze_origin_y = gaze_data->left_eye.gaze_origin.position_in_user_coordinates.y;
        data.left_gaze_origin_z = gaze_data->left_eye.gaze_origin.position_in_user_coordinates.z;
        data.left_gaze_origin_validity = gaze_data->left_eye.gaze_origin.validity;

        // right gaze origin
        data.right_gaze_origin_x = gaze_data->right_eye.gaze_origin.position_in_user_coordinates.x;
        data.right_gaze_origin_y = gaze_data->right_eye.gaze_origin.position_in_user_coordinates.y;
        data.right_gaze_origin_z = gaze_data->right_eye.gaze_origin.position_in_user_coordinates.z;
        data.right_gaze_origin_validity = gaze_data->right_eye.gaze_origin.validity;

        // left pupil
        data.left_pupil_diameter = gaze_data->left_eye.pupil_data.diameter;
        data.left_pupil_validity = gaze_data->left_eye.pupil_data.validity;

        // right pupil
        data.right_pupil_diameter = gaze_data->right_eye.pupil_data.diameter;
        data.right_pupil_validity = gaze_data->right_eye.pupil_data.validity;

        // status
        data.left_eye_detected = (data.left_gaze_point_validity == TOBII_RESEARCH_VALIDITY_VALID);
        data.right_eye_detected = (data.right_gaze_point_validity == TOBII_RESEARCH_VALIDITY_VALID);
        data.is_tracking = (data.left_eye_detected || data.right_eye_detected);
        data.overall_validity = (data.left_eye_detected && data.right_eye_detected) ? TOBII_RESEARCH_VALIDITY_VALID : TOBII_RESEARCH_VALIDITY_INVALID;
        
        return data;
    }
};