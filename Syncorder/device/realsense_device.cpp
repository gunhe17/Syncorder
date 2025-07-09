#pragma once

#include <algorithm>

// installed
#include <librealsense2/rs.hpp>

// local
#include <Syncorder/error/exception.h>
#include <Syncorder/device/base.h>


/**
 * @class Callback
 */
class RealsenseCallback {
public:
    RealsenseCallback() {}
    
    void onFrame(const rs2::frameset& frames) {
        // TODO: Frame processing logic
        // CameraDevice의 OnReadSample과 유사한 역할
        std::cout << "Frame received";
    }
};


/**
 * @class Device
 */
class RealsenseDevice : public BDevice {
private:
    rs2::pipeline pipe_;
    rs2::config config_;
    rs2::colorizer color_map_;
    rs2::align align_to_color_;
    std::unique_ptr<RealsenseCallback> callback_;
    
    // Frame data
    rs2::frameset current_frames_;
    rs2::frame depth_frame_;
    rs2::frame color_frame_;

public:
    RealsenseDevice(int device_id = 0)
    : 
        BDevice(device_id),
        align_to_color_(RS2_STREAM_COLOR)
    {
        callback_ = std::make_unique<RealsenseCallback>();
    }
    
    ~RealsenseDevice() {
        cleanup();
    }

public:      
    bool _setup() override {
        _startRS();
        
        config_ = _createConfig();
        _validateConfig();

        return true;
    }
    
    bool _warmup() override {
        // Auto-exposure stabilization (save-to-disk example 참고)
        pipe_.start(config_);
        
        // Capture 30 frames to give autoexposure, etc. a chance to settle
        for (auto i = 0; i < 30; ++i) {
            pipe_.wait_for_frames();
        }
        
        pipe_.stop();
        return true;
    }
    
    bool _start() override {
        _runPipeline();
        return true;
    }
    
    bool _stop() override {
        pipe_.stop();
        return true;
    }
    
    bool _cleanup() override {
        // Pipeline은 자동으로 정리됨
        callback_.reset();
        return true;
    }

private:
    void _startRS() {
        // librealsense는 별도 초기화 불필요
        // MediaFoundation의 MFStartup과 달리 context는 자동 관리
    }
    
    rs2::config _createConfig() {
        rs2::config cfg;
        
        // Enable color and depth streams (capture example 참고)
        cfg.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_RGB8, 30);
        cfg.enable_stream(RS2_STREAM_DEPTH, 640, 480, RS2_FORMAT_Z16, 30);
        
        return cfg;
    }
    
    void _validateConfig() {
        rs2::context ctx;
        auto device_list = ctx.query_devices();
        
        if (device_list.size() == 0) {
            throw ArducamDeviceError("RealSense device를 찾을 수 없습니다");
        }
        
        if (device_id_ >= static_cast<int>(device_list.size())) {
            throw ArducamDeviceError("잘못된 device index입니다");
        }
        
        // Device가 요구하는 stream을 지원하는지 확인
        auto device = device_list[device_id_];
        auto sensors = device.query_sensors();
        
        bool color_supported = false;
        bool depth_supported = false;
        
        for (auto& sensor : sensors) {
            auto profiles = sensor.get_stream_profiles();
            for (auto& profile : profiles) {
                if (profile.stream_type() == RS2_STREAM_COLOR) color_supported = true;
                if (profile.stream_type() == RS2_STREAM_DEPTH) depth_supported = true;
            }
        }
        
        if (!color_supported || !depth_supported) {
            throw ArducamDeviceError("Device가 Color/Depth stream을 지원하지 않습니다");
        }
    }

    void _runPipeline() {
        pipe_.start(config_);
    }
    
public:
    // Frame access methods (CameraDevice의 callback 대신)
    rs2::frameset getLatestFrames() {
        current_frames_ = pipe_.wait_for_frames();
        
        // Apply filters (capture example 참고)
        current_frames_ = current_frames_.apply_filter(color_map_);
        current_frames_ = current_frames_.apply_filter(align_to_color_);
        
        return current_frames_;
    }
    
    rs2::frame getColorFrame() {
        if (current_frames_) {
            return current_frames_.get_color_frame();
        }
        return rs2::frame{};
    }
    
    rs2::frame getDepthFrame() {
        if (current_frames_) {
            return current_frames_.get_depth_frame();
        }
        return rs2::frame{};
    }
    
    // Frame data access (save-to-disk example 참고)
    void* getColorData() {
        color_frame_ = getColorFrame();
        return color_frame_ ? const_cast<void*>(color_frame_.get_data()) : nullptr;
    }
    
    void* getDepthData() {
        depth_frame_ = getDepthFrame();
        return depth_frame_ ? const_cast<void*>(depth_frame_.get_data()) : nullptr;
    }
    
    // Frame properties
    int getColorWidth() { return color_frame_ ? color_frame_.as<rs2::video_frame>().get_width() : 0; }
    int getColorHeight() { return color_frame_ ? color_frame_.as<rs2::video_frame>().get_height() : 0; }
    int getDepthWidth() { return depth_frame_ ? depth_frame_.as<rs2::video_frame>().get_width() : 0; }
    int getDepthHeight() { return depth_frame_ ? depth_frame_.as<rs2::video_frame>().get_height() : 0; }
    
    // Timestamp (hardware clock 사용)
    double getFrameTimestamp() {
        return current_frames_ ? current_frames_.get_timestamp() : 0.0;
    }
};