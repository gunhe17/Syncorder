#pragma once

#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <vector>

// local
#include <Syncorder/gonfig/gonfig.h>
#include <Syncorder/error/exception.h>
#include <Syncorder/devices/common/broker_base.h>
#include <Syncorder/devices/realsense/model.h>


/**
 * @class Broker
 */

class RealsenseBroker : public TBBroker<RealsenseBufferData> {
private:
    std::ofstream csv_;
    std::string output_;

    std::vector<std::string> raw_files_;

public:
    RealsenseBroker() {
        output_ = gonfig.output_directory + "realsense/";

        std::filesystem::create_directories(output_);
        std::filesystem::create_directories(output_ + "color/");
        std::filesystem::create_directories(output_ + "depth/");

        csv_.open(output_ + "realsense_data.csv");
        csv_ 
            << "system_time,"
            << "device_timestamp,"
            << "frame_number,"
            << "has_depth,"
            << "has_color,"
            << "color_file,"
            << "depth_file\n";
    }
    ~RealsenseBroker() {
        csv_.close();
    }

public:
    void cleanup() {
        _convert();
    }
protected:
    void _process(const RealsenseBufferData& data) override {
        _write(data);
        _capture(data);
        std::cout << "[RealsenseBroker] Processing timestamp\n";
    }

private:
    void _write(const RealsenseBufferData& data) {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(data.sys_time_.time_since_epoch()).count();

        std::string color_filename = data.has_color_ ? ("color/frame_" + std::to_string(data.frame_number_) + ".png") : "";
        std::string depth_filename = data.has_depth_ ? ("depth/frame_" + std::to_string(data.frame_number_) + ".png") : "";
       
        csv_ 
            << ms << ","
            << data.device_timestamp_ << ","
            << data.frame_number_ << ","
            << (data.has_depth_ ? 1 : 0) << ","
            << (data.has_color_ ? 1 : 0) << ","
            << color_filename << ","
            << depth_filename << "\n";
    }

    void _capture(const RealsenseBufferData& data) {
        std::string frame_id = std::to_string(data.frame_number_);

        // Color 저장
        if (data.has_color_) {
            auto color = data.color_frame_.as<rs2::video_frame>();
            std::string color_file = output_ + "color/frame_" + frame_id + ".raw";
            std::ofstream file(color_file, std::ios::binary);
            file.write((char*)color.get_data(), color.get_data_size());
            
            // 해상도와 포맷 정보를 함께 저장
            std::string file_info = color_file + ":" + 
                                std::to_string(color.get_width()) + "x" + std::to_string(color.get_height()) + 
                                ":rgb24";
            raw_files_.push_back(file_info);
            
            std::cout << "[RealsenseBroker] Saved: " << file_info << std::endl;  // 디버그용
        }

        // Depth 저장  
        if (data.has_depth_) {
            auto depth = data.depth_frame_.as<rs2::video_frame>();
            std::string depth_file = output_ + "depth/frame_" + frame_id + ".raw";
            std::ofstream file(depth_file, std::ios::binary);
            file.write((char*)depth.get_data(), depth.get_data_size());
            
            std::string file_info = depth_file + ":" + 
                                std::to_string(depth.get_width()) + "x" + std::to_string(depth.get_height()) + 
                                ":gray16le";
            raw_files_.push_back(file_info);
            
            std::cout << "[RealsenseBroker] Saved: " << file_info << std::endl;  // 디버그용
        }
    }

    void _convert() {
        std::cout << "[RealsenseBroker] Converting raw files to images...\n";
        
        for (const auto& entry : std::filesystem::recursive_directory_iterator(output_)) {
            if (entry.path().extension() == ".raw") {
                std::string raw_file = entry.path().string();
                std::string img_file = raw_file;
                img_file.replace(img_file.find(".raw"), 4, ".jpg");
                
                std::string cmd;
                if (raw_file.find("color") != std::string::npos) {
                    cmd = "ffmpeg -y -f rawvideo -pixel_format rgb24 -video_size 1280x720 -i \"" + raw_file + "\" -update 1 \"" + img_file + "\"";
                } else if (raw_file.find("depth") != std::string::npos) {
                    cmd = "ffmpeg -y -f rawvideo -pixel_format gray16le -video_size 848x480 -i \"" + raw_file + "\" -update 1 \"" + img_file + "\"";
                } else {
                    continue;
                }
                
                std::cout << "[RealsenseBroker] Converting: " << entry.path().filename().string() << std::endl;
                int result = system(cmd.c_str());
                
                if (result == 0) {
                    std::cout << "[RealsenseBroker] ✓ Success" << std::endl;
                } else {
                    std::cout << "[RealsenseBroker] ✗ Failed: " << result << std::endl;
                }
            }
        }
    }
};