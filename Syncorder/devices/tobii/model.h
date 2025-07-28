#pragma once

#include <chrono>
#include "tobii_research.h"
#include "tobii_research_calibration.h"
#include "tobii_research_eyetracker.h"
#include "tobii_research_streams.h"


/**
 * @struct TobiiBufferData - Global Tobii Data Structure
 */

struct TobiiBufferData {
    
    // timestamp
    int64_t device_time_stamp;                          // Eye tracker device 내부 clock 기준 시간 (microseconds)
    int64_t system_time_stamp;                          // Host computer system clock 기준 시간 (microseconds)
    
    // left gaze point
    float left_gaze_point_display_x;                    // Display area 상 시선 X 좌표 (0.0~1.0, 왼쪽=0.0, 오른쪽=1.0)
    float left_gaze_point_display_y;                    // Display area 상 시선 Y 좌표 (0.0~1.0, 위=0.0, 아래=1.0)
    float left_gaze_point_3d_x;                         // User coordinate system 시선 X 좌표 (mm, 오른쪽=양수)
    float left_gaze_point_3d_y;                         // User coordinate system 시선 Y 좌표 (mm, 위=양수)
    float left_gaze_point_3d_z;                         // User coordinate system 시선 Z 좌표 (mm, 사용자 쪽=양수)
    TobiiResearchValidity left_gaze_point_validity;     // 왼쪽 눈 시선 data 유효성
    
    // right gaze point
    float right_gaze_point_display_x;                   // Display area 상 시선 X 좌표 (0.0~1.0)
    float right_gaze_point_display_y;                   // Display area 상 시선 Y 좌표 (0.0~1.0)
    float right_gaze_point_3d_x;                        // User coordinate system 시선 X 좌표 (mm)
    float right_gaze_point_3d_y;                        // User coordinate system 시선 Y 좌표 (mm)
    float right_gaze_point_3d_z;                        // User coordinate system 시선 Z 좌표 (mm)
    TobiiResearchValidity right_gaze_point_validity;    // 오른쪽 눈 시선 data 유효성

    // left gaze origin
    float left_gaze_origin_x;                           // User coordinate system 왼쪽 눈 X 위치 (mm)
    float left_gaze_origin_y;                           // User coordinate system 왼쪽 눈 Y 위치 (mm)
    float left_gaze_origin_z;                           // User coordinate system 왼쪽 눈 Z 위치 (mm, 거리)
    TobiiResearchValidity left_gaze_origin_validity;    // 왼쪽 눈 위치 data 유효성

    // right gaze origin
    float right_gaze_origin_x;                          // User coordinate system 오른쪽 눈 X 위치 (mm)
    float right_gaze_origin_y;                          // User coordinate system 오른쪽 눈 Y 위치 (mm)
    float right_gaze_origin_z;                          // User coordinate system 오른쪽 눈 Z 위치 (mm, 거리)
    TobiiResearchValidity right_gaze_origin_validity;   // 오른쪽 눈 위치 data 유효성

    // left pupil
    float left_pupil_diameter;                          // 왼쪽 눈 동공 직경 (mm)
    TobiiResearchValidity left_pupil_validity;          // 왼쪽 눈 동공 data 유효성

    // right pupil
    float right_pupil_diameter;                         // 오른쪽 눈 동공 직경 (mm)
    TobiiResearchValidity right_pupil_validity;         // 오른쪽 눈 동공 data 유효성

    // timestamp
    int64_t sync_system_request_time;                   // Computer가 동기화 요청을 보낸 시간
    int64_t sync_device_time;                           // Eye tracker가 요청을 받은 시간
    int64_t sync_system_response_time;                  // Computer가 응답을 받은 시간
    TobiiResearchValidity sync_validity;                // timestamp

    // status
    bool left_eye_detected;                             // 왼쪽 눈 감지 여부
    bool right_eye_detected;                            // 오른쪽 눈 감지 여부
    bool is_tracking;                                   // 현재 tracking 진행 중 여부
    TobiiResearchValidity overall_validity;             // 전체 data bundle의 유효성
};