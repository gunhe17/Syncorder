#pragma once

// installed
#include "tobii_research.h"
#include "tobii_research_eyetracker.h"
#include "tobii_research_streams.h"

// local
#include <Syncorder/error/exception.h>

class TobiiCallback {
private:
    static inline TobiiCallback* instance_ = nullptr;

public:
    TobiiCallback() {}
    ~TobiiCallback() {}

public:
    void setup() {
        instance_ = this;
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
        std::cout << "_process() -> L:" << left_valid << " R:" << right_valid << std::endl;
        
        // 항상 출력해서 데이터 확인
        printf("Timestamp: %lld | ", (long long)timestamp);
        printf("L:(%.3f,%.3f)[%s] ", left_x, left_y, left_valid ? "Valid" : "Invalid");
        printf("R:(%.3f,%.3f)[%s]", right_x, right_y, right_valid ? "Valid" : "Invalid");
        printf("\n");
        
        // 유효한 데이터만 별도 처리
        if (left_valid || right_valid) {
            std::cout << "✅ 유효한 gaze data 감지!" << std::endl;
        }
    }
};