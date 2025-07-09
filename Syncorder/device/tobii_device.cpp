#pragma once

#include <chrono>

// installed
#include "tobii_research.h"
#include "tobii_research_eyetracker.h"
#include "tobii_research_streams.h"

#pragma comment(lib, "tobii_research.lib")

// local
#include <Syncorder/error/exception.h>
#include <Syncorder/device/base.h>


/**
 * @struct
 */
struct TobiiGazeData {
    std::chrono::system_clock::time_point timestamp_;
    int64_t device_timestamp_;
    
    struct EyeData {
        double x, y;
        bool validity;
        double pupil_diameter;
    } left_eye_, right_eye_;
    
    struct Eye3DData {
        double x, y, z;
        bool validity;
    } left_eye_3d_, right_eye_3d_;
};


/**
 * @class Callback
 */
class TobiiCallback {
public:
    TobiiCallback() {}
    ~TobiiCallback() {}
    
    void setGazeHandler(void* handler) { gaze_handler_ = handler; }
    void setEyeImageHandler(void* handler) { eye_image_handler_ = handler; }
    
private:
    void* gaze_handler_ = nullptr;
    void* eye_image_handler_ = nullptr;
};


/**
 * @class Device
 */

class TobiiDevice : public BDevice {
private:
    TobiiResearchEyeTracker* device_;
    TobiiCallback* callback_;
    
    // info
    std::string address_;
    std::string serial_number_;
    std::string device_name_;
    std::string model_;
    
    TobiiResearchDisplayArea display_area_;
    
    // calibration
    bool calibration_loaded_;
    std::string calibration_file_path_;

public:
    TobiiDevice()
    : 
        BDevice(0)
    {}
    
    ~TobiiDevice() {
        cleanup();
    }

public:    
    bool _setup() override {
        device_ = _createDevice();
        callback_ = _createCallback();

        _loadDisplayArea();
        _loadCalibration();

        return true;
    }
    
    bool _warmup() override {
        return true;
    }
    
    bool _start() override {
        _startStreaming();

        return true;
    }
    
    bool _stop() override {
        _stopStreaming();

        return true;
    }
    
    bool _cleanup() override {
        // reverse
        device_ = nullptr;
        if (callback_) {
            delete callback_;
            callback_ = nullptr;
        }
        return true;
    }

private:    
    TobiiResearchEyeTracker* _createDevice() {
        TobiiResearchEyeTrackers* devices;
        TobiiResearchStatus status;

        status = tobii_research_find_all_eyetrackers(&devices);

        if (
            status != TOBII_RESEARCH_STATUS_OK ||
            devices->count == 0 ||
            !devices
        )
            throw ArducamDeviceError("No eye trackers found");

        TobiiResearchEyeTracker* device = devices->eyetrackers[0];
        tobii_research_free_eyetrackers(devices);

        // info
        char* s = nullptr;

        tobii_research_get_address(device, &s);
        address_ = s ? s : "";
        tobii_research_free_string(s);

        tobii_research_get_serial_number(device, &s);
        serial_number_ = s ? s : "";
        tobii_research_free_string(s);

        tobii_research_get_device_name(device, &s);
        device_name_ = s ? s : "";
        tobii_research_free_string(s);

        tobii_research_get_model(device, &s);
        model_ = s ? s : "";
        tobii_research_free_string(s);

        return device;
    }

    TobiiCallback* _createCallback() {
        return new TobiiCallback();
    }

    void _loadDisplayArea() {
        TobiiResearchStatus status;
        
        status = tobii_research_get_display_area(device_, &display_area_);
        if (status != TOBII_RESEARCH_STATUS_OK) throw ArducamDeviceError("Failed to get display area");
        
        status = tobii_research_set_display_area(device_, &display_area_);
        if (status != TOBII_RESEARCH_STATUS_OK) throw ArducamDeviceError("Failed to set display area");
    }
    
    void _loadCalibration() {
        // TODO: GONFIG
        FILE* f = fopen("bin/calibration.bin", "rb");
        if (!f) throw ArducamDeviceError("Failed to open calibration file");

        fseek(f, 0, SEEK_END);
        size_t size = ftell(f);
        rewind(f);
        if (size == 0) { fclose(f); throw ArducamDeviceError("Calibration file is empty"); }

        void* buffer = malloc(size);
        if (fread(buffer, 1, size, f) != size) {
            free(buffer);
            fclose(f);
            throw ArducamDeviceError("Failed to read calibration file");
        }

        fclose(f);

        TobiiResearchCalibrationData data = { buffer, size };
        TobiiResearchStatus status = tobii_research_apply_calibration_data(device_, &data);
        free(buffer);

        if (status != TOBII_RESEARCH_STATUS_OK)
            throw ArducamDeviceError("Failed to apply calibration data");

        calibration_loaded_ = true;
    }
    
    void _startStreaming() {
        // Start streaming logic (추후 구현)
    }
    
    void _stopStreaming() {
        // Stop streaming logic (추후 구현)
    }
};