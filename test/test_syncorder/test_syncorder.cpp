#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <stdexcept>

#include "Syncorder/devices/common/device_base.h"
#include "Syncorder/devices/common/manager_base.h"
#include "Syncorder/syncorder.cpp"

/**
 * Mock Device Implementations
 */

class MockCameraDevice : public BManager {
private:
    std::string name_;
    bool simulate_failure_;
    std::chrono::milliseconds delay_;
    
public:
    MockCameraDevice(const std::string& name, bool simulate_failure = false, 
                     std::chrono::milliseconds delay = std::chrono::milliseconds(100))
        : name_(name), simulate_failure_(simulate_failure), delay_(delay) {}
    
    void setup() override {
        std::this_thread::sleep_for(delay_);
        if (simulate_failure_ && name_.find("fail") != std::string::npos) {
            throw std::runtime_error("Camera setup failed");
        }
        is_setup_.store(true);
    }
    
    void warmup() override {
        if (!is_setup_.load()) {
            throw std::runtime_error("Setup not completed");
        }
        std::this_thread::sleep_for(delay_);
        is_warmup_.store(true);
    }
    
    void start() override {
        if (!is_warmup_.load()) {
            throw std::runtime_error("Warmup not completed");
        }
        std::this_thread::sleep_for(delay_);
        is_running_.store(true);
    }
    
    void stop() override {
        is_running_.store(false);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    void cleanup() override {
        is_setup_.store(false);
        is_warmup_.store(false);
        is_running_.store(false);
    }
    
    std::string __name__() const override { return name_; }
};

class MockTobiiDevice : public BManager {
private:
    std::string name_;
    bool simulate_slow_;
    
public:
    MockTobiiDevice(const std::string& name, bool simulate_slow = false)
        : name_(name), simulate_slow_(simulate_slow) {}
    
    void setup() override {
        auto delay = simulate_slow_ ? std::chrono::milliseconds(500) : std::chrono::milliseconds(150);
        std::this_thread::sleep_for(delay);
        is_setup_.store(true);
    }
    
    void warmup() override {
        if (!is_setup_.load()) {
            throw std::runtime_error("Setup not completed");
        }
        auto delay = simulate_slow_ ? std::chrono::milliseconds(800) : std::chrono::milliseconds(200);
        std::this_thread::sleep_for(delay);
        is_warmup_.store(true);
    }
    
    void start() override {
        if (!is_warmup_.load()) {
            throw std::runtime_error("Warmup not completed");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        is_running_.store(true);
    }
    
    void stop() override {
        is_running_.store(false);
    }
    
    void cleanup() override {
        is_setup_.store(false);
        is_warmup_.store(false);
        is_running_.store(false);
    }
    
    std::string __name__() const override { return name_; }
};

class MockRealsenseDevice : public BManager {
private:
    std::string name_;
    bool simulate_hang_;
    
public:
    MockRealsenseDevice(const std::string& name, bool simulate_hang = false)
        : name_(name), simulate_hang_(simulate_hang) {}
    
    void setup() override {
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
        is_setup_.store(true);
    }
    
    void warmup() override {
        if (!is_setup_.load()) {
            throw std::runtime_error("Setup not completed");
        }
        if (simulate_hang_) {
            // 무한 대기 시뮬레이션 (timeout 테스트용)
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(180));
        is_warmup_.store(true);
    }
    
    void start() override {
        if (!is_warmup_.load()) {
            throw std::runtime_error("Warmup not completed");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(90));
        is_running_.store(true);
    }
    
    void stop() override {
        is_running_.store(false);
    }
    
    void cleanup() override {
        is_setup_.store(false);
        is_warmup_.store(false);
        is_running_.store(false);
    }
    
    std::string __name__() const override { return name_; }
};

/**
 * Test Functions
 */

void testBasicFlow() {
    std::cout << "\n=== Test 1: Basic Flow ===\n";
    
    SyncController controller;
    controller.setTimeout(std::chrono::milliseconds(2000));
    
    // Add devices
    controller.addDevice(std::make_unique<MockCameraDevice>("Camera-0"));
    controller.addDevice(std::make_unique<MockCameraDevice>("Camera-1"));
    controller.addDevice(std::make_unique<MockTobiiDevice>("Tobii-0"));
    controller.addDevice(std::make_unique<MockRealsenseDevice>("Realsense-0"));
    
    // Execute stages
    bool success = true;
    success &= controller.executeSetup();
    success &= controller.executeWarmup();
    success &= controller.executeStart();
    
    if (success) {
        std::cout << "\n[TEST] Recording simulation (1 second)...\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        controller.executeStop();
        controller.executeCleanup();
        
        std::cout << "[TEST] Basic flow completed successfully\n";
    } else {
        std::cout << "[TEST] Basic flow failed\n";
    }
}

void testFailureHandling() {
    std::cout << "\n=== Test 2: Failure Handling ===\n";
    
    SyncController controller;
    controller.setTimeout(std::chrono::milliseconds(1000));
    
    // Add devices with one failure
    controller.addDevice(std::make_unique<MockCameraDevice>("Camera-0"));
    controller.addDevice(std::make_unique<MockCameraDevice>("Camera-fail", true)); // 실패 시뮬레이션
    controller.addDevice(std::make_unique<MockTobiiDevice>("Tobii-0"));
    
    bool success = controller.executeSetup();
    
    if (!success) {
        std::cout << "[TEST] Failure handling working correctly\n";
        controller.executeCleanup();
    } else {
        std::cout << "[TEST] Failure handling test failed - should have failed\n";
    }
}

void testTimeoutHandling() {
    std::cout << "\n=== Test 3: Timeout Handling ===\n";
    
    SyncController controller;
    controller.setTimeout(std::chrono::milliseconds(500)); // 짧은 timeout
    
    // Add slow/hanging devices
    controller.addDevice(std::make_unique<MockCameraDevice>("Camera-0"));
    controller.addDevice(std::make_unique<MockRealsenseDevice>("Realsense-hang", true)); // hang 시뮬레이션
    
    bool success = controller.executeSetup();
    if (success) {
        success = controller.executeWarmup(); // 이것이 timeout되어야 함
    }
    
    if (!success) {
        std::cout << "[TEST] Timeout handling working correctly\n";
        controller.executeCleanup();
    } else {
        std::cout << "[TEST] Timeout handling test failed - should have timed out\n";
    }
}

void testAbortMechanism() {
    std::cout << "\n=== Test 4: Abort Mechanism ===\n";
    
    SyncController controller;
    controller.setTimeout(std::chrono::milliseconds(3000));
    
    controller.addDevice(std::make_unique<MockCameraDevice>("Camera-0"));
    controller.addDevice(std::make_unique<MockTobiiDevice>("Tobii-0", true)); // slow device
    
    // Start in separate thread and abort
    bool test_completed = false;
    std::thread test_thread([&controller, &test_completed]() {
        bool success = controller.executeSetup();
        if (success && !controller.isAborted()) {
            controller.executeWarmup();
        }
        test_completed = true;
    });
    
    // Abort after short delay
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    controller.abort();
    
    test_thread.join();
    
    if (controller.isAborted() && test_completed) {
        std::cout << "[TEST] Abort mechanism working correctly\n";
    } else {
        std::cout << "[TEST] Abort mechanism test failed\n";
    }
    
    controller.executeCleanup();
}

void testEmptyDeviceList() {
    std::cout << "\n=== Test 5: Empty Device List ===\n";
    
    SyncController controller;
    
    bool success = controller.executeSetup();
    
    if (!success && controller.getDeviceCount() == 0) {
        std::cout << "[TEST] Empty device list handling working correctly\n";
    } else {
        std::cout << "[TEST] Empty device list test failed - should have failed\n";
    }
}

void testLargeDeviceCount() {
    std::cout << "\n=== Test 6: Large Device Count ===\n";
    
    SyncController controller;
    controller.setTimeout(std::chrono::milliseconds(5000));
    
    // Add many devices
    for (int i = 0; i < 8; ++i) {
        controller.addDevice(std::make_unique<MockCameraDevice>("Camera-" + std::to_string(i)));
    }
    for (int i = 0; i < 2; ++i) {
        controller.addDevice(std::make_unique<MockTobiiDevice>("Tobii-" + std::to_string(i)));
    }
    
    auto start_time = std::chrono::steady_clock::now();
    
    bool success = true;
    success &= controller.executeSetup();
    success &= controller.executeWarmup();
    success &= controller.executeStart();
    
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    if (success) {
        std::cout << "[TEST] Large device count (" << controller.getDeviceCount() 
                  << " devices) completed in " << duration.count() << "ms\n";
        controller.executeStop();
        controller.executeCleanup();
    } else {
        std::cout << "[TEST] Large device count test failed\n";
    }
}

void testStageSequencing() {
    std::cout << "\n=== Test 7: Stage Sequencing ===\n";
    
    SyncController controller;
    controller.setTimeout(std::chrono::milliseconds(2000));
    
    controller.addDevice(std::make_unique<MockCameraDevice>("Camera-seq"));
    controller.addDevice(std::make_unique<MockTobiiDevice>("Tobii-seq"));
    
    // Test proper sequencing
    bool setup_ok = controller.executeSetup();
    bool warmup_ok = controller.executeWarmup();
    bool start_ok = controller.executeStart();
    
    if (setup_ok && warmup_ok && start_ok) {
        std::cout << "[TEST] Stage sequencing working correctly\n";
        controller.executeStop();
        controller.executeCleanup();
    } else {
        std::cout << "[TEST] Stage sequencing failed\n";
    }
}

/**
 * Main Test Runner
 */
int main() {
    std::cout << "=== SyncController Test Suite ===\n";
    std::cout << "Testing synchronized multi-device recording system\n";
    
    try {
        testBasicFlow();
        testFailureHandling();
        testTimeoutHandling();
        testAbortMechanism();
        testEmptyDeviceList();
        testLargeDeviceCount();
        testStageSequencing();
        
        std::cout << "\n=== All Tests Completed ===\n";
        std::cout << "SyncController test suite finished!\n";
        
    } catch (const std::exception& e) {
        std::cout << "\n[FATAL ERROR] Test suite failed: " << e.what() << "\n";
        return -1;
    }
    
    return 0;
}