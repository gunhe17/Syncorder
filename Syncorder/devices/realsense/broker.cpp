#pragma once

#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

// local
#include <Syncorder/gonfig/gonfig.h>
#include <Syncorder/error/exception.h>
#include <Syncorder/devices/common/broker_base.h>
#include <Syncorder/devices/realsense/model.h>

#include <librealsense2/rs.hpp>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


/**
 * @helper: struct
 */

struct DepthAnalysis {
    uint16_t center_depth = 0;
    float valid_pixel_ratio = 0.0f;
    uint16_t min_depth = 65535;
    uint16_t max_depth = 0;
    uint16_t avg_depth = 0;
};

struct ColorAnalysis {
    uint8_t center_r = 0;
    uint8_t center_g = 0;
    uint8_t center_b = 0;
    uint8_t avg_brightness = 0;
};

struct FrameTask {
    rs2::frame color_frame;
    rs2::frame depth_frame;
    uint64_t frame_number;
    bool has_color;
    bool has_depth;
    std::string output_dir;
    
    FrameTask(const RealsenseBufferData& data, const std::string& output) 
        : color_frame(data.color_frame_)
        , depth_frame(data.depth_frame_)
        , frame_number(data.frame_number_)
        , has_color(data.has_color_)
        , has_depth(data.has_depth_)
        , output_dir(output) {}
};


/**
 * @class Broker
 */

class RealsenseBroker : public TBBroker<RealsenseBufferData> {
private:
    std::ofstream csv_;
    std::string output_;

    std::vector<std::string> raw_files_;

    // thread
    std::thread worker_;
    std::queue<std::unique_ptr<FrameTask>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> stop_{false};
    
    thread_local static rs2::colorizer colorizer_;

public:
    RealsenseBroker() {
        output_ = gonfig.output_directory + "realsense/";

        std::filesystem::create_directories(output_);
        std::filesystem::create_directories(output_ + "color/");
        std::filesystem::create_directories(output_ + "depth/");

        csv_.open(output_ + "realsense_data.csv");
        csv_ 
            << "FrameNumber,"
            << "DeviceTimestamp,"
            << "SystemTime,"
            << "CenterDepth,"
            << "ValidPixelRatio,"
            << "MinDepth,"
            << "MaxDepth,"
            << "AvgDepth,"
            << "CenterR,"
            << "CenterG,"
            << "CenterB,"
            << "AvgBrightness,"
            << "ColorImageSaved,"
            << "DepthImageSaved,"
            << "ColorImagePath,"
            << "DepthImagePath\n";

        // thread
        worker_ = std::thread(&RealsenseBroker::_worker_loop, this);
    }

    ~RealsenseBroker() {
        csv_.close();

        // thread
        stop_.store(true);
        cv_.notify_one();
        if(worker_.joinable()) worker_.join();
    }

public:
    void cleanup() {
        // _convert();
    }

protected:
    void _process(const RealsenseBufferData& data) override {
        _write(data);
        
        {
            std::lock_guard<std::mutex> lock(mutex_);
            tasks_.push(std::make_unique<FrameTask>(data, output_));
        }
        cv_.notify_one();
        
        std::cout << "[RealsenseBroker] Processing timestamp\n";
    }

private:
    void _write(const RealsenseBufferData& data) {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(data.sys_time_.time_since_epoch()).count();

        std::string color_filename = data.has_color_ ? ("color/frame_" + std::to_string(data.frame_number_) + ".png") : "";
        ColorAnalysis color_analysis = _color(data);

        std::string depth_filename = data.has_depth_ ? ("depth/frame_" + std::to_string(data.frame_number_) + ".png") : "";
        DepthAnalysis depth_analysis = _depth(data);

        csv_ 
            << data.frame_number_ << ","
            << std::fixed << std::setprecision(2) << data.device_timestamp_ << ","
            << ms << ","
            << depth_analysis.center_depth << ","
            << std::fixed << std::setprecision(2) 
            << depth_analysis.valid_pixel_ratio << ","
            << depth_analysis.min_depth << ","
            << depth_analysis.max_depth << ","
            << depth_analysis.avg_depth << ","
            << static_cast<int>(color_analysis.center_r) << ","
            << static_cast<int>(color_analysis.center_g) << ","
            << static_cast<int>(color_analysis.center_b) << ","
            << static_cast<int>(color_analysis.avg_brightness) << ","
            << (data.has_color_ ? "YES" : "NO") << ","
            << (data.has_depth_ ? "YES" : "NO") << ","
            << color_filename << ","
            << depth_filename << "\n";
    }

    void _worker_loop() {
        while (true) {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return !tasks_.empty() || stop_.load(); });
            
            if (stop_.load() && tasks_.empty()) break;
            if (tasks_.empty()) continue;
            
            auto task = std::move(tasks_.front());
            tasks_.pop();
            lock.unlock();
            
            _process_task(*task);
        }
    }
    
    void _process_task(const FrameTask& task) {
        std::string frame_id = std::to_string(task.frame_number);
        
        // Color 저장 (PNG로 직접 저장)
        if (task.has_color) {
            auto color = task.color_frame.as<rs2::video_frame>();
            std::string path = task.output_dir + "color/frame_" + frame_id + ".png";
            stbi_write_png(path.c_str(), color.get_width(), color.get_height(),
                          color.get_bytes_per_pixel(), color.get_data(), 
                          color.get_stride_in_bytes());
        }
        
        // Depth 저장 (Colorized PNG)
        if (task.has_depth) {
            if (!colorizer_) colorizer_ = rs2::colorizer();
            
            auto colorized = colorizer_.process(task.depth_frame);
            auto depth_rgb = colorized.as<rs2::video_frame>();
            std::string path = task.output_dir + "depth/frame_" + frame_id + ".png";
            stbi_write_png(path.c_str(), depth_rgb.get_width(), depth_rgb.get_height(),
                          depth_rgb.get_bytes_per_pixel(), depth_rgb.get_data(),
                          depth_rgb.get_stride_in_bytes());
        }
    }
private:
    ColorAnalysis _color(const RealsenseBufferData& data) {
        ColorAnalysis analysis;
        
        if (!data.has_color_) return analysis;

        auto color_frame = data.color_frame_.as<rs2::video_frame>();
        const uint8_t* color_data = (const uint8_t*)color_frame.get_data();
        
        int width = color_frame.get_width();
        int height = color_frame.get_height();
        int bytes_per_pixel = color_frame.get_bytes_per_pixel();

        // 중앙 픽셀 RGB값
        int center_x = width / 2;
        int center_y = height / 2;
        int center_idx = (center_y * width + center_x) * bytes_per_pixel;
        
        if (bytes_per_pixel >= 3) {
            analysis.center_r = color_data[center_idx];
            analysis.center_g = color_data[center_idx + 1];
            analysis.center_b = color_data[center_idx + 2];
        }

        // 평균 밝기 계산 (샘플링으로 성능 최적화)
        uint32_t brightness_sum = 0;
        int sample_count = 0;
        int step = 10; // 10픽셀마다 샘플링

        for (int y = 0; y < height; y += step) {
            for (int x = 0; x < width; x += step) {
                int idx = (y * width + x) * bytes_per_pixel;
                if (bytes_per_pixel >= 3) {
        
                    uint8_t brightness = (uint8_t)(
                        0.299f * color_data[idx] + 
                        0.587f * color_data[idx + 1] + 
                        0.114f * color_data[idx + 2]
                    );
                    brightness_sum += brightness;
                    sample_count++;
                }
            }
        }

        if (sample_count > 0) {
            analysis.avg_brightness = brightness_sum / sample_count;
        }

        return analysis;
    }

    DepthAnalysis _depth(const RealsenseBufferData& data) {
        DepthAnalysis analysis;
        
        if (!data.has_depth_) return analysis;

        auto depth_frame = data.depth_frame_.as<rs2::depth_frame>();
        const uint16_t* depth_data = (const uint16_t*)depth_frame.get_data();
        
        int width = depth_frame.get_width();
        int height = depth_frame.get_height();
        int total_pixels = width * height;

        // 중앙 픽셀 깊이값
        int center_x = width / 2;
        int center_y = height / 2;
        analysis.center_depth = depth_data[center_y * width + center_x];

        // 깊이 통계 계산
        uint32_t sum_depth = 0;
        int valid_pixels = 0;

        for (int i = 0; i < total_pixels; i++) {
            uint16_t depth = depth_data[i];
            
            if (depth > 0) {  // 유효한 깊이값
                valid_pixels++;
                sum_depth += depth;
                
                if (depth < analysis.min_depth) analysis.min_depth = depth;
                if (depth > analysis.max_depth) analysis.max_depth = depth;
            }
        }

        if (valid_pixels > 0) {
            analysis.valid_pixel_ratio = (float)valid_pixels / total_pixels * 100.0f;
            analysis.avg_depth = sum_depth / valid_pixels;
        } else {
            analysis.min_depth = 0;
        }

        return analysis;
    }
};

thread_local rs2::colorizer RealsenseBroker::colorizer_;