#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <Syncorder/error/exception.h>

// device
#include <Syncorder/device/camera_device.cpp>
#include <Syncorder/device/tobii_device.cpp>
#include <Syncorder/device/realsense_device.cpp>

// callback
#include <Syncorder/callback/camera_callback.cpp>


int main() {
    /**
     * Camera Device Test
    */
    auto camera_device = std::make_unique<CameraDevice>(0);
    auto camera_callback = Microsoft::WRL::Make<CameraCallback>();

    camera_device->pre_setup(camera_callback->getIUnknown());
    camera_device->setup();

    camera_callback->pre_setup(camera_device->getReader());
    camera_callback->setup();

    camera_device->warmup();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::cout << "Camera test completed\n";

    /**
     * Tobii Device Test
    */

    

    std::cout << "All tests completed successfully\n";
    return 0;
}