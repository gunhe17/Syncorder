#pragma once

#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <stdexcept>
#include <vector>
#include <iomanip>
#include <sstream>

// local
#include <Syncorder/error/exception.h>
#include <Syncorder/syncorder.cpp>

#include <Syncorder/devices/camera/device.cpp>
#include <Syncorder/devices/tobii/device.cpp>
#include <Syncorder/devices/realsense/device.cpp>

#include <Syncorder/devices/camera/manager.cpp>
#include <Syncorder/devices/tobii/manager.cpp>
#include <Syncorder/devices/realsense/manager.cpp>


int main() {
    try {
        std::cout << "=== Syncorder Multi-Device Recording ===\n\n";
        
        Syncorder syncorder;
        syncorder.setTimeout(std::chrono::milliseconds(5000)); //TODO: setup()으로 개선?

        syncorder.addDevice(std::make_unique<CameraManager>(2));
        syncorder.addDevice(std::make_unique<TobiiManager>(0));
        syncorder.addDevice(std::make_unique<RealsenseManager>(0));
        
        std::cout << "[Main] " << syncorder.getDeviceCount() << " devices registered\n\n";
        
        // 동기화된 초기화
        std::cout << "[Main] === SETUP PHASE ===\n";
        if (!syncorder.executeSetup()) {
            std::cout << "[ERROR] Setup failed\n";
            syncorder.executeCleanup();
            return -1;
        }
        
        // 동기화된 준비
        std::cout << "\n[Main] === WARMUP PHASE ===\n";
        if (!syncorder.executeWarmup()) {
            std::cout << "[ERROR] Warmup failed\n";
            syncorder.executeCleanup();
            return -1;
        }
        
        // 동기화된 녹화 시작
        std::cout << "\n[Main] === START PHASE ===\n";
        if (!syncorder.executeStart()) {
            std::cout << "[ERROR] Start failed\n";
            syncorder.executeStop();
            syncorder.executeCleanup();
            return -1;
        }
        
        // 녹화 진행
        std::cout << "\n[Main] === RECORDING ===\n";
        std::cout << "[Main] Recording for 10 seconds...\n";
        std::this_thread::sleep_for(std::chrono::seconds(10));
        
        // 동기화된 종료
        std::cout << "\n[Main] === SHUTDOWN ===\n";
        syncorder.executeStop();
        syncorder.executeCleanup();
        
        std::cout << "\n[Main] === COMPLETED SUCCESSFULLY ===\n";
            
    } catch (const std::exception& e) {
        std::cout << "[FATAL ERROR] " << e.what() << "\n";
        return -1;
    }
    
    return 0;
}