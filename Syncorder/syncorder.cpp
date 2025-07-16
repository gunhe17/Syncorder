#pragma once

#include <vector>
#include <thread>
#include <memory>
#include <array>
#include <atomic>
#include <iostream>
#include <chrono>
#include <mutex>
#include <condition_variable>

// local
#include <Syncorder/devices/common/manager_base.h>
#include <Syncorder/devices/camera/manager.cpp>
#include <Syncorder/devices/realsense/manager.cpp>
#include <Syncorder/devices/tobii/manager.cpp>


/**
 * @class Barrier
 */

class SyncBarrier {
private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::size_t count_;
    std::size_t waiting_;
    std::size_t generation_;

public:
    explicit SyncBarrier(std::size_t count) 
        : count_(count), waiting_(0), generation_(0) {}
    
    bool arrive_and_wait(std::chrono::milliseconds timeout = std::chrono::milliseconds(5000)) {
        std::unique_lock<std::mutex> lock(mutex_);
        std::size_t current_generation = generation_;
        ++waiting_;
        
        if (waiting_ == count_) {
            waiting_ = 0;
            ++generation_;
            cv_.notify_all();
            return true;
        } else {
            return cv_.wait_for(lock, timeout, [this, current_generation] {
                return generation_ != current_generation;
            });
        }
    }
};


/**
 * Syncorder
 */

 class Syncorder {
public:
    enum class State {
        IDLE,
        RUNNING,
        COMPLETED,
        FAILED
    };

private:
    enum Stage { SETUP = 0, WARMUP = 1, RUN = 2, NUM_STAGES = 3 };
    
    // Core data
    std::vector<std::unique_ptr<BManager>> managers_;
    std::array<std::unique_ptr<SyncBarrier>, NUM_STAGES> barriers_;
    
    // State management
    std::atomic<State> state_{State::IDLE};
    std::atomic<bool> stop_requested_{false};
    
    // Configuration
    std::chrono::milliseconds default_timeout_{5000};

public:
    // Device registration
    void addCamera(int device_id) {
        addManager(std::make_unique<CameraManager>(device_id));
    }
    
    void addTobii(int device_id) {
        addManager(std::make_unique<TobiiManager>(device_id));
    }
    
    void addRealsense(int device_id) {
        addManager(std::make_unique<RealsenseManager>(device_id));
    }
    
    void addManager(std::unique_ptr<BManager> manager) {
        if (state_.load() != State::IDLE) {
            std::cout << "[ERROR] Cannot add devices while running\n";
            return;
        }
        
        std::cout << "[Syncorder] Added " << manager->__name__() << "\n";
        managers_.push_back(std::move(manager));
    }

    bool start() {
        if (state_.load() != State::IDLE) {
            std::cout << "[ERROR] Already running or completed\n";
            return false;
        }
        
        if (managers_.empty()) {
            std::cout << "[ERROR] No devices registered\n";
            _setState(State::FAILED);
            return false;
        }
        
        _setState(State::RUNNING);
        stop_requested_.store(false);
        
        return _initialize() && _runSynchronized();
    }

    void stop() {
        stop_requested_.store(true);
        std::cout << "[Syncorder] Stop requested\n";
    }
    
    void waitForCompletion() {
        while (state_.load() == State::RUNNING) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    // State query
    State getState() const { return state_.load(); }
    size_t getDeviceCount() const { return managers_.size(); }
    
    // Configuration
    void setTimeout(std::chrono::milliseconds timeout) { default_timeout_ = timeout; }

private:
    bool _initialize() {
        std::size_t participant_count = managers_.size() + 1;
        
        try {
            for (int i = 0; i < NUM_STAGES; ++i) {
                barriers_[i] = std::make_unique<SyncBarrier>(participant_count);
            }
            
            std::cout << "[Syncorder] Initialized " << managers_.size() << " devices\n";
            return true;
            
        } catch (const std::exception& e) {
            std::cout << "[ERROR] Initialization failed: " << e.what() << "\n";
            _setState(State::FAILED);
            return false;
        }
    }
    
    bool _runSynchronized() {
        std::vector<std::thread> device_threads;
        std::atomic<bool> all_success{true};
        
        try {
            // Start device threads
            for (auto& manager : managers_) {
                device_threads.emplace_back([this, &manager, &all_success]() {
                    if (!_deviceFlow(*manager)) {
                        all_success.store(false);
                    }
                });
            }
            
            // Run coordinator in current thread
            bool coordinator_success = _coordinatorFlow();
            if (!coordinator_success) {
                all_success.store(false);
            }
            
            // Wait for all threads
            for (auto& thread : device_threads) {
                if (thread.joinable()) {
                    thread.join();
                }
            }
            
            // Set final state
            bool success = all_success.load() && coordinator_success;
            _setState(success ? State::COMPLETED : State::FAILED);
            
            std::cout << "[Syncorder] " << (success ? "SUCCESS" : "FAILED") << "\n";
            
            return success;
            
        } catch (const std::exception& e) {
            std::cout << "[Syncorder] Runtime error: " << e.what() << "\n";
            _setState(State::FAILED);
            _cleanup();
            return false;
        }
    }
    
    bool _deviceFlow(BManager& manager) {
        try {
            // Setup phase
            if (!barriers_[SETUP]->arrive_and_wait(default_timeout_)) {
                std::cout << "[ERROR] " << manager.__name__() << " setup timeout\n";
                return false;
            }
            
            if (stop_requested_.load()) return false;
            manager.setup();
            
            // Warmup phase
            if (!barriers_[WARMUP]->arrive_and_wait(default_timeout_)) {
                std::cout << "[ERROR] " << manager.__name__() << " warmup timeout\n";
                return false;
            }
            
            if (stop_requested_.load()) return false;
            manager.warmup();
            
            // Run phase
            if (!barriers_[RUN]->arrive_and_wait(default_timeout_)) {
                std::cout << "[ERROR] " << manager.__name__() << " run timeout\n";
                return false;
            }
            
            if (stop_requested_.load()) return false;
            manager.run();
            
            return true;
            
        } catch (const std::exception& e) {
            std::cout << "[ERROR] " << manager.__name__() << ": " << e.what() << "\n";
            return false;
        }
    }
    
    bool _coordinatorFlow() {
        try {
            // Setup coordination
            if (!barriers_[SETUP]->arrive_and_wait(default_timeout_)) {
                std::cout << "[ERROR] Setup phase timeout\n";
                return false;
            }
            if (stop_requested_.load()) return false;
            std::cout << "[Syncorder] Setup completed\n";
            
            // Warmup coordination
            if (!barriers_[WARMUP]->arrive_and_wait(default_timeout_)) {
                std::cout << "[ERROR] Warmup phase timeout\n";
                return false;
            }
            if (stop_requested_.load()) return false;
            std::cout << "[Syncorder] Warmup completed\n";

            std::this_thread::sleep_for(std::chrono::milliseconds(3000));
            
            // Run coordination
            if (!barriers_[RUN]->arrive_and_wait(default_timeout_)) {
                std::cout << "[ERROR] Run phase timeout\n";
                return false;
            }
            if (stop_requested_.load()) return false;
            std::cout << "[Syncorder] Recording started\n";
            
            return true;
            
        } catch (const std::exception& e) {
            std::cout << "[ERROR] Coordinator: " << e.what() << "\n";
            return false;
        }
    }
    
    void _setState(State new_state) {
        state_.store(new_state);
    }
    
    void _cleanup() {
        // Reset barriers
        for (auto& barrier : barriers_) {
            barrier.reset();
        }
    }
};