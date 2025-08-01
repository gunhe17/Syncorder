﻿#pragma once

#include <chrono>
#include <any>
#include <string>
#include <atomic>
#include <iostream>

// installed
#include "tobii_research.h"
#include "tobii_research_eyetracker.h"
#include "tobii_research_streams.h"

#pragma comment(lib, "tobii_research.lib")

// local
#include <Syncorder/gonfig/gonfig.h>
#include <Syncorder/error/exception.h>
#include <Syncorder/devices/common/device_base.h>


/**
 * @class TobiiDevice
 */

class TobiiDevice : public BDevice {
private:
    TobiiResearchEyeTracker* device_;
    
    void* callback_;
    void* gaze_;

    TobiiResearchDisplayArea display_area_;

public:
    TobiiDevice(int device_id = 0) 
    : 
        BDevice(device_id)
    {}
    
    ~TobiiDevice() {
        cleanup();
    }

public:
    bool pre_setup(void* callback, void* gaze) {
        callback_ = callback;
        gaze_ = gaze;

        return true;
    }
    
    bool _setup() override {
        device_ = _createDevice();

        _setFrequency();

        _loadDisplayArea();
        _loadCalibration();
        
        return true;
    }
    
    bool _warmup() override {
        _readSource();
        
        return true;
    }
    
    bool _start() override {
        return true;
    }
    
    bool _stop() override {
        return true;
    }
    
    bool _cleanup() override {
        return true;
    }

    // get
    TobiiResearchEyeTracker* getDevice() {
        return device_;
    }

private:
    TobiiResearchEyeTracker* _createDevice() {
        TobiiResearchEyeTrackers* devices;
        TobiiResearchStatus status;

        status = tobii_research_find_all_eyetrackers(&devices);

        if (status != TOBII_RESEARCH_STATUS_OK || devices->count == 0 || !devices) {
            throw TobiiDeviceError("No eye trackers found");
        }

        if (device_id_ >= static_cast<int>(devices->count)) {
            tobii_research_free_eyetrackers(devices);
            throw TobiiDeviceError("Device index " + std::to_string(device_id_) + " out of range (0-" + std::to_string(devices->count-1) + ")");
        }

        TobiiResearchEyeTracker* device = devices->eyetrackers[device_id_];
        tobii_research_free_eyetrackers(devices);

        return device;
    }

    void _setFrequency() {
        TobiiResearchStatus status;

        status = tobii_research_set_gaze_output_frequency(device_, 60.0f);
        if (status != TOBII_RESEARCH_STATUS_OK) {
            throw TobiiDeviceError("Failed to set frequency");
        }
    }
    
    void _loadDisplayArea() {
        TobiiResearchStatus status;
        
        status = tobii_research_get_display_area(device_, &display_area_);
        if (status != TOBII_RESEARCH_STATUS_OK) {
            throw TobiiDeviceError("Failed to get display area");
        }
        
        status = tobii_research_set_display_area(device_, &display_area_);
        if (status != TOBII_RESEARCH_STATUS_OK) {
            throw TobiiDeviceError("Failed to set display area");
        }
    }
    
    void _loadCalibration() {
        // TODO: Config에서 경로 읽기
        FILE* f = fopen(gonfig.calibration_path.c_str(), "rb");
        if (!f) return;

        fseek(f, 0, SEEK_END);
        size_t size = ftell(f);
        rewind(f);
        
        if (size == 0) { 
            fclose(f); 
            std::cout << "Warning: Calibration file is empty, skipping\n";
            return;
        }

        void* buffer = malloc(size);
        if (fread(buffer, 1, size, f) != size) {
            free(buffer);
            fclose(f);
            throw TobiiDeviceError("Failed to read calibration file");
        }

        fclose(f);

        TobiiResearchCalibrationData data = { buffer, size };
        TobiiResearchStatus status = tobii_research_apply_calibration_data(device_, &data);
        free(buffer);

        if (status != TOBII_RESEARCH_STATUS_OK) {
            throw TobiiDeviceError("Failed to apply calibration data");
        }
    }

    void _readSource() {
        if (!callback_) {
            throw TobiiDeviceError("Callback not set before warmup");
        }
        
        if (!gaze_) {
            throw TobiiDeviceError("Gaze callback not set before warmup");
        }
        
        auto func = reinterpret_cast<void(*)(TobiiResearchGazeData*, void*)>(gaze_);
        
        TobiiResearchStatus status = tobii_research_subscribe_to_gaze_data(device_, func, callback_);
        if (status != TOBII_RESEARCH_STATUS_OK) {
            throw TobiiDeviceError("Failed to subscribe to gaze data. Status: " + std::to_string(status));
        }
    }
};