#pragma once

#include <iostream>
#include <memory>
#include <string>

#include <Syncorder/error/exception.h>
#include <Syncorder/device/camera.cpp>
#include <Syncorder/device/tobii.cpp>


int main() {
    // Camera Device Test
    std::unique_ptr<BDevice> camera_device = std::make_unique<CameraDevice>(0);

    if (!camera_device->init()) {
        std::cout << "Camera init() failed\n";
        return 1;
    }

    if (!camera_device->setup()) {
        std::cout << "Camera setup() failed\n";
        return 1;
    }

    if (!camera_device->warmup()) {
        std::cout << "Camera warmup() failed\n";
        return 1;
    }

    if (!camera_device->start()) {
        std::cout << "Camera start() failed\n";
        return 1;
    }

    if (!camera_device->stop()) {
        std::cout << "Camera stop() failed\n";
        return 1;
    }

    if (!camera_device->cleanup()) {
        std::cout << "Camera cleanup() failed\n";
        return 1;
    }

    std::cout << "Camera test completed\n";

    std::cout << "All tests completed successfully\n";
    return 0;
}