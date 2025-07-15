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
#include <Syncorder/callback/tobii_callback.cpp>
#include <Syncorder/callback/realsense_callback.cpp>

// buffer
#include <Syncorder/buffer/camera_buffer.cpp>
#include <Syncorder/buffer/tobii_buffer.cpp>
#include <Syncorder/buffer/realsense_buffer.cpp>

// broker
#include <Syncorder/broker/camera_broker.cpp>
#include <Syncorder/broker/tobii_broker.cpp>
#include <Syncorder/broker/realsense_broker.cpp>


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