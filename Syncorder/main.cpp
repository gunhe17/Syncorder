#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <Syncorder/error/exception.h>

#include <Syncorder/devices/camera/manager.cpp>
#include <Syncorder/devices/realsense/manager.cpp>
#include <Syncorder/devices/tobii/manager.cpp>


int main() {
    CameraManager camera_manager = CameraManager(2);

    camera_manager.setup();
    camera_manager.warmup();
    camera_manager.run();

    RealsenseManager realsense_manager = RealsenseManager(0);

    realsense_manager.setup();
    realsense_manager.warmup();
    realsense_manager.run();

    TobiiManager tobii_manager = TobiiManager(0);

    tobii_manager.setup();
    tobii_manager.warmup();
    tobii_manager.run();
    
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "All tests completed successfully\n";
    return 0;
}