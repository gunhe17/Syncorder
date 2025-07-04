#pragma once

#include <iostream>
#include <memory>
#include <string>

#include "Syncorder/error/exception.h"
#include <Syncorder/error/exception.cpp>
#include <Syncorder/device/camera.cpp>


/**
 * @class main
 */

int main() {
    std::unique_ptr<BDevice> device = std::make_unique<CameraDevice>(0);

    if (!device->init()) {
        std::cout << "init() failed\n";
        return 1;
    }

    if (!device->setup()) {
        std::cout << "setup() failed\n";
        return 1;
    }

    if (!device->warmup()) {
        std::cout << "warmup() failed\n";
        return 1;
    }

    if (!device->start()) {
        std::cout << "start() failed\n";
        return 1;
    }

    if (!device->stop()) {
        std::cout << "stop() failed\n";
        return 1;
    }

    if (!device->cleanup()) {;
        std::cout << "Device stopped and cleaned up\n";
        return 1;
    }

    std::cout << "clear\n";
    return 0;
};