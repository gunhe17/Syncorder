#pragma once

#include <vector>
#include <thread>
#include <memory>
#include <atomic>
#include <iostream>
#include <chrono>
#include <future>
#include <functional>
#include <string>

#include <Syncorder/devices/common/manager_base.h>

/**
 * SyncController Implementation
 */
class SyncController {
private:
    std::vector<std::unique_ptr<BManager>> devices_;
    std::atomic<bool> abort_flag_{false};
    std::chrono::milliseconds default_timeout_{5000};
    
public:
    void addDevice(std::unique_ptr<BManager> device) {
        if (!device) {
            std::cout << "[SyncController] Warning: null device ignored\n";
            return;
        }
        
        std::cout << "[SyncController] Added device: " << device->__name__() << "\n";
        devices_.push_back(std::move(device));
    }
    
    bool executeSetup() {
        std::cout << "[SyncController] Starting setup phase...\n";
        return executeStage("setup", [](BManager& device) {
            device.setup();
            return device.__is_setup__();
        });
    }
    
    bool executeWarmup() {
        std::cout << "[SyncController] Starting warmup phase...\n";
        return executeStage("warmup", [](BManager& device) {
            device.warmup();
            return device.__is_warmup__();
        });
    }
    
    bool executeStart() {
        std::cout << "[SyncController] Starting devices...\n";
        return executeStage("start", [](BManager& device) {
            device.start();
            return device.__is_running__();
        });
    }
    
    void executeStop() {
        std::cout << "[SyncController] Stopping all devices...\n";
        std::vector<std::future<void>> futures;
        
        for (auto& device : devices_) {
            futures.push_back(
                std::async(std::launch::async, [&device]() {
                    try {
                        device->stop();
                        std::cout << "[" << device->__name__() << "] Stopped\n";
                    } catch (const std::exception& e) {
                        std::cout << "[" << device->__name__() << "] Stop error: " << e.what() << "\n";
                    }
                })
            );
        }
        
        waitForAllFutures(futures, default_timeout_);
    }
    
    void executeCleanup() {
        std::cout << "[SyncController] Cleaning up...\n";
        for (auto& device : devices_) {
            try {
                device->cleanup();
                std::cout << "[" << device->__name__() << "] Cleaned up\n";
            } catch (const std::exception& e) {
                std::cout << "[" << device->__name__() << "] Cleanup error: " << e.what() << "\n";
            }
        }
    }
    
    void abort() {
        std::cout << "[SyncController] Abort requested\n";
        abort_flag_.store(true);
        executeStop();
    }
    
    void setTimeout(std::chrono::milliseconds timeout) {
        default_timeout_ = timeout;
        std::cout << "[SyncController] Timeout set to " << timeout.count() << "ms\n";
    }
    
    size_t getDeviceCount() const {
        return devices_.size();
    }
    
    bool isAborted() const {
        return abort_flag_.load();
    }

private:
    template<typename StageFunc>
    bool executeStage(const std::string& stage_name, StageFunc func) {
        if (abort_flag_.load()) {
            std::cout << "[SyncController] Aborted during " << stage_name << "\n";
            return false;
        }
        
        if (devices_.empty()) {
            std::cout << "[SyncController] No devices registered\n";
            return false;
        }
        
        std::vector<std::future<bool>> futures;
        
        for (auto& device : devices_) {
            futures.push_back(
                std::async(std::launch::async, [&device, &func, &stage_name]() {
                    try {
                        std::cout << "[" << device->__name__() << "] " << stage_name << " started\n";
                        bool success = func(*device);
                        std::cout << "[" << device->__name__() << "] " << stage_name 
                                 << (success ? " completed" : " failed") << "\n";
                        return success;
                    } catch (const std::exception& e) {
                        std::cout << "[" << device->__name__() << "] " << stage_name 
                                 << " error: " << e.what() << "\n";
                        return false;
                    }
                })
            );
        }
        
        bool all_success = waitForAllFutures(futures, default_timeout_);
        
        if (!all_success) {
            std::cout << "[SyncController] " << stage_name << " phase failed\n";
            abort_flag_.store(true);
        } else {
            std::cout << "[SyncController] " << stage_name << " phase completed successfully\n";
        }
        
        return all_success;
    }
    
    template<typename T>
    bool waitForAllFutures(std::vector<std::future<T>>& futures, 
                          std::chrono::milliseconds timeout) {
        auto deadline = std::chrono::steady_clock::now() + timeout;
        bool all_success = true;
        
        for (auto& future : futures) {
            auto remaining = deadline - std::chrono::steady_clock::now();
            if (remaining <= std::chrono::milliseconds(0)) {
                std::cout << "[SyncController] Timeout waiting for completion\n";
                return false;
            }
            
            if (future.wait_for(remaining) == std::future_status::timeout) {
                std::cout << "[SyncController] Device timeout\n";
                return false;
            }
            
            if constexpr (std::is_same_v<T, bool>) {
                try {
                    if (!future.get()) {
                        all_success = false;
                    }
                } catch (const std::exception& e) {
                    std::cout << "[SyncController] Future exception: " << e.what() << "\n";
                    all_success = false;
                }
            } else {
                try {
                    future.get();
                } catch (const std::exception& e) {
                    std::cout << "[SyncController] Future exception: " << e.what() << "\n";
                }
            }
        }
        
        return all_success;
    }
};