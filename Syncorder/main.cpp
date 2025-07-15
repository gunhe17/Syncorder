#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <Syncorder/error/exception.h>

// device
#include <Syncorder/devices/camera/device.cpp>
#include <Syncorder/devices/tobii/device.cpp>
#include <Syncorder/devices/realsense/device.cpp>

// callback
#include <Syncorder/devices/camera/callback.cpp>
#include <Syncorder/devices/tobii/callback.cpp>
#include <Syncorder/devices/realsense/callback.cpp>

// buffer
#include <Syncorder/devices/camera/buffer.cpp>
#include <Syncorder/devices/tobii/buffer.cpp>
#include <Syncorder/devices/realsense/buffer.cpp>

// broker
#include <Syncorder/devices/camera/broker.cpp>
#include <Syncorder/devices/tobii/broker.cpp>
#include <Syncorder/devices/realsense/broker.cpp>


int main() {
    
    /**
     * Camera Device Test
    */
    auto camera_device = std::make_unique<CameraDevice>(2);
    auto camera_callback = Microsoft::WRL::Make<CameraCallback>();
    auto camera_buffer = std::make_unique<CameraBuffer>();
    auto camera_broker = std::make_unique<CameraBroker>();

    // setup
    camera_device->pre_setup(camera_callback->getIUnknown());
    camera_device->setup();

    camera_callback->setup(camera_device->getReader(), static_cast<void*>(camera_buffer.get()));

    camera_broker->setup(camera_buffer.get(), reinterpret_cast<void*>(&CameraBuffer::dequeue));

    // warmup
    camera_device->warmup();

    camera_broker->run();
    
    std::cout << "Camera test completed\n";


    /**
     * Tobii Device Test
    */
    auto tobii_device = std::make_unique<TobiiDevice>(0);
    auto tobii_callback = std::make_unique<TobiiCallback>();
    auto tobii_buffer = std::make_unique<TobiiBuffer>();
    auto tobii_broker = std::make_unique<TobiiBroker>();

    // setup
    tobii_device->pre_setup(tobii_callback.get(), reinterpret_cast<void*>(&TobiiCallback::onGaze));
    tobii_device->setup();

    tobii_callback->setup(static_cast<void*>(tobii_buffer.get()));

    tobii_broker->setup(tobii_buffer.get(), reinterpret_cast<void*>(&TobiiBuffer::dequeue));

    // warmup
    tobii_device->warmup();

    tobii_broker->run();

    std::cout << "Tobii test completed\n";


    /**
     * RealSense Device Test
    */
    auto realsense_device = std::make_unique<RealsenseDevice>(0);
    auto realsense_callback = std::make_unique<RealsenseCallback>();
    auto realsense_buffer = std::make_unique<RealsenseBuffer>();
    auto realsense_broker = std::make_unique<RealsenseBroker>();

    // setup
    realsense_device->pre_setup(reinterpret_cast<void*>(&RealsenseCallback::onFrameset));
    realsense_device->setup();

    realsense_callback->setup(static_cast<void*>(realsense_buffer.get()));

    realsense_broker->setup(realsense_buffer.get(), reinterpret_cast<void*>(&RealsenseBuffer::dequeue));

    // warmup
    realsense_device->warmup();

    realsense_broker->run();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::cout << "RealSense test completed\n";


    std::cout << "All tests completed successfully\n";
    return 0;
}