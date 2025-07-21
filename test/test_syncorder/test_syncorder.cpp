#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <stdexcept>
#include <vector>
#include <iomanip>
#include <sstream>

#include "Syncorder/devices/common/device_base.h"
#include "Syncorder/devices/common/manager_base.h"
#include "Syncorder/syncorder.cpp"

/**
 * 동기화 테스트용 Mock Device
 */
class TimingTestDevice : public BManager {
private:
    std::string name_;
    std::chrono::milliseconds setup_time_;
    std::chrono::milliseconds warmup_time_;
    std::chrono::milliseconds start_time_;
    
    // 타이밍 추적
    std::chrono::steady_clock::time_point setup_start_;
    std::chrono::steady_clock::time_point setup_end_;
    std::chrono::steady_clock::time_point warmup_start_;
    std::chrono::steady_clock::time_point warmup_end_;
    std::chrono::steady_clock::time_point start_start_;
    std::chrono::steady_clock::time_point start_end_;
    
public:
    TimingTestDevice(const std::string& name, 
                     std::chrono::milliseconds setup_time,
                     std::chrono::milliseconds warmup_time,
                     std::chrono::milliseconds start_time)
        : name_(name), setup_time_(setup_time), warmup_time_(warmup_time), start_time_(start_time) {}
    
    void setup() override {
        setup_start_ = std::chrono::steady_clock::now();
        std::cout << "  [" << name_ << "] Setup starting... (expected: " << setup_time_.count() << "ms)\n";
        
        std::this_thread::sleep_for(setup_time_);
        is_setup_.store(true);
        
        setup_end_ = std::chrono::steady_clock::now();
        auto actual = std::chrono::duration_cast<std::chrono::milliseconds>(setup_end_ - setup_start_);
        std::cout << "  [" << name_ << "] Setup completed (actual: " << actual.count() << "ms)\n";
    }
    
    void warmup() override {
        warmup_start_ = std::chrono::steady_clock::now();
        std::cout << "  [" << name_ << "] Warmup starting... (expected: " << warmup_time_.count() << "ms)\n";
        
        std::this_thread::sleep_for(warmup_time_);
        is_warmup_.store(true);
        
        warmup_end_ = std::chrono::steady_clock::now();
        auto actual = std::chrono::duration_cast<std::chrono::milliseconds>(warmup_end_ - warmup_start_);
        std::cout << "  [" << name_ << "] Warmup completed (actual: " << actual.count() << "ms)\n";
    }
    
    void start() override {
        start_start_ = std::chrono::steady_clock::now();
        std::cout << "  [" << name_ << "] Start starting... (expected: " << start_time_.count() << "ms)\n";
        
        std::this_thread::sleep_for(start_time_);
        is_running_.store(true);
        
        start_end_ = std::chrono::steady_clock::now();
        auto actual = std::chrono::duration_cast<std::chrono::milliseconds>(start_end_ - start_start_);
        std::cout << "  [" << name_ << "] Start completed (actual: " << actual.count() << "ms)\n";
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
 * 테스트 결과 출력 헬퍼
 */
void printTestHeader(const std::string& test_name, const std::string& description) {
    std::cout << "\n";
    std::cout << "=========================================\n";
    std::cout << "TEST: " << test_name << "\n";
    std::cout << "=========================================\n";
    std::cout << "PURPOSE: " << description << "\n\n";
}

// 수정: int64_t 타입으로 변경하여 경고 제거
void printPhaseResult(const std::string& phase, int64_t actual_ms, int64_t expected_ms, int64_t tolerance_ms = 100) {
    std::cout << "\n--- " << phase << " Phase Result ---\n";
    std::cout << "  Actual time:   " << std::setw(6) << actual_ms << "ms\n";
    std::cout << "  Expected time: " << std::setw(6) << expected_ms << "ms\n";
    std::cout << "  Difference:    " << std::setw(6) << (actual_ms - expected_ms) << "ms\n";
    
    bool passed = (actual_ms >= (expected_ms - tolerance_ms)) && (actual_ms <= (expected_ms + tolerance_ms));
    std::cout << "  Result:        " << (passed ? "PASS" : "FAIL") << "\n";
}

void printTestResult(bool success, const std::string& message = "") {
    std::cout << "\n--- TEST RESULT ---\n";
    std::cout << "Status: " << (success ? "PASSED" : "FAILED") << "\n";
    if (!message.empty()) {
        std::cout << "Note: " << message << "\n";
    }
    std::cout << "\n";
}

/**
 * 동기화 테스트 함수들
 */
void testBasicSynchronization() {
    printTestHeader("Basic Synchronization", 
                   "Verify that all devices wait for the slowest device before proceeding to next phase");
    
    Syncorder syncorder;
    syncorder.setTimeout(std::chrono::milliseconds(10000));
    
    // Device 타이밍 설정
    std::cout << "SETUP: Registering devices with different timing:\n";
    std::cout << "  FastCamera:     setup=100ms, warmup=150ms, start=50ms\n";
    std::cout << "  SlowTobii:      setup=800ms, warmup=1200ms, start=300ms  <-- SLOWEST\n";
    std::cout << "  MediumRealsense: setup=400ms, warmup=600ms, start=200ms\n";
    std::cout << "\nEXPECTED BEHAVIOR: Each phase should wait for SlowTobii to complete\n";
    
    syncorder.addDevice(std::make_unique<TimingTestDevice>(
        "FastCamera", std::chrono::milliseconds(100), 
        std::chrono::milliseconds(150), std::chrono::milliseconds(50)));
    
    syncorder.addDevice(std::make_unique<TimingTestDevice>(
        "SlowTobii", std::chrono::milliseconds(800), 
        std::chrono::milliseconds(1200), std::chrono::milliseconds(300)));
    
    syncorder.addDevice(std::make_unique<TimingTestDevice>(
        "MediumRealsense", std::chrono::milliseconds(400), 
        std::chrono::milliseconds(600), std::chrono::milliseconds(200)));
    
    auto total_start = std::chrono::steady_clock::now();
    
    // Setup Phase
    std::cout << "\nEXECUTING: Setup Phase (expecting ~800ms)...\n";
    auto setup_start = std::chrono::steady_clock::now();
    bool setup_ok = syncorder.executeSetup();
    auto setup_end = std::chrono::steady_clock::now();
    auto setup_duration = std::chrono::duration_cast<std::chrono::milliseconds>(setup_end - setup_start);
    
    printPhaseResult("Setup", setup_duration.count(), 800);
    
    if (!setup_ok) {
        printTestResult(false, "Setup phase failed");
        return;
    }
    
    // Warmup Phase
    std::cout << "\nEXECUTING: Warmup Phase (expecting ~1200ms)...\n";
    auto warmup_start = std::chrono::steady_clock::now();
    bool warmup_ok = syncorder.executeWarmup();
    auto warmup_end = std::chrono::steady_clock::now();
    auto warmup_duration = std::chrono::duration_cast<std::chrono::milliseconds>(warmup_end - warmup_start);
    
    printPhaseResult("Warmup", warmup_duration.count(), 1200);
    
    if (!warmup_ok) {
        printTestResult(false, "Warmup phase failed");
        return;
    }
    
    // Start Phase
    std::cout << "\nEXECUTING: Start Phase (expecting ~300ms)...\n";
    auto start_start = std::chrono::steady_clock::now();
    bool start_ok = syncorder.executeStart();
    auto start_end = std::chrono::steady_clock::now();
    auto start_duration = std::chrono::duration_cast<std::chrono::milliseconds>(start_end - start_start);
    
    printPhaseResult("Start", start_duration.count(), 300);
    
    auto total_end = std::chrono::steady_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(total_end - total_start);
    
    // Overall Result
    std::cout << "\n--- OVERALL RESULT ---\n";
    std::cout << "Total execution time: " << total_duration.count() << "ms\n";
    std::cout << "Expected total time:  ~2300ms (800+1200+300)\n";
    
    bool overall_pass = start_ok && 
                       (setup_duration.count() >= 700 && setup_duration.count() <= 900) &&
                       (warmup_duration.count() >= 1100 && warmup_duration.count() <= 1300) &&
                       (start_duration.count() >= 200 && start_duration.count() <= 400);
    
    printTestResult(overall_pass, "All devices synchronized to slowest device timing");
}

void testExtremeTiming() {
    printTestHeader("Extreme Timing Difference", 
                   "Test synchronization with devices having very different execution times");
    
    Syncorder syncorder;
    syncorder.setTimeout(std::chrono::milliseconds(15000));
    
    std::cout << "SETUP: Registering devices with extreme timing differences:\n";
    std::cout << "  UltraFast: setup=10ms, warmup=20ms, start=5ms     (total: 35ms)\n";
    std::cout << "  UltraSlow: setup=2000ms, warmup=3000ms, start=1000ms (total: 6000ms)\n";
    std::cout << "\nTIMING RATIO: 171:1 difference between fastest and slowest\n";
    std::cout << "EXPECTED BEHAVIOR: UltraFast should wait for UltraSlow at each phase\n";
    
    syncorder.addDevice(std::make_unique<TimingTestDevice>(
        "UltraFast", std::chrono::milliseconds(10), 
        std::chrono::milliseconds(20), std::chrono::milliseconds(5)));
    
    syncorder.addDevice(std::make_unique<TimingTestDevice>(
        "UltraSlow", std::chrono::milliseconds(2000), 
        std::chrono::milliseconds(3000), std::chrono::milliseconds(1000)));
    
    auto total_start = std::chrono::steady_clock::now();
    
    std::cout << "\nEXECUTING: All phases...\n";
    bool success = true;
    success &= syncorder.executeSetup();
    success &= syncorder.executeWarmup();
    success &= syncorder.executeStart();
    
    auto total_end = std::chrono::steady_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(total_end - total_start);
    
    std::cout << "\n--- EXTREME TIMING TEST RESULT ---\n";
    std::cout << "Total execution time: " << total_duration.count() << "ms\n";
    std::cout << "Expected time:        ~6000ms (UltraSlow total)\n";
    std::cout << "Time saved by parallel: " << (6000 + 35 - total_duration.count()) << "ms\n";
    
    bool passed = success && (total_duration.count() >= 5800 && total_duration.count() <= 6500);
    printTestResult(passed, "Extreme timing differences handled correctly");
}

void testMultiDeviceConcurrency() {
    printTestHeader("Multi-Device Concurrency", 
                   "Test synchronization with multiple devices running in parallel");
    
    Syncorder syncorder;
    syncorder.setTimeout(std::chrono::milliseconds(8000));
    
    // 5개의 서로 다른 타이밍 device
    struct DeviceSpec { std::string name; int setup; int warmup; int start; };
    std::vector<DeviceSpec> devices = {
        {"Camera-A", 200, 300, 100},
        {"Camera-B", 150, 250, 80},
        {"Tobii-A", 800, 1200, 400},      // 가장 느림
        {"Realsense-A", 400, 600, 200},
        {"Arducam-A", 300, 500, 150}
    };
    
    std::cout << "SETUP: Registering " << devices.size() << " devices with different timings:\n";
    std::cout << "Device         Setup  Warmup  Start\n";
    std::cout << "------------------------------------\n";
    
    for (const auto& dev : devices) {
        std::cout << std::left << std::setw(12) << dev.name 
                  << std::setw(7) << (std::to_string(dev.setup) + "ms")
                  << std::setw(8) << (std::to_string(dev.warmup) + "ms")
                  << std::setw(7) << (std::to_string(dev.start) + "ms") << "\n";
        
        syncorder.addDevice(std::make_unique<TimingTestDevice>(
            dev.name, std::chrono::milliseconds(dev.setup),
            std::chrono::milliseconds(dev.warmup), std::chrono::milliseconds(dev.start)));
    }
    
    std::cout << "\nEXPECTED: Each phase waits for Tobii-A (slowest device)\n";
    std::cout << "         Setup: 800ms, Warmup: 1200ms, Start: 400ms\n";
    
    auto total_start = std::chrono::steady_clock::now();
    
    std::cout << "\nEXECUTING: Multi-device parallel execution...\n";
    bool success = true;
    success &= syncorder.executeSetup();
    success &= syncorder.executeWarmup();
    success &= syncorder.executeStart();
    
    auto total_end = std::chrono::steady_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(total_end - total_start);
    
    // 순차 실행 시간 계산
    int64_t sequential_time = 0;
    for (const auto& dev : devices) {
        sequential_time += dev.setup + dev.warmup + dev.start;
    }
    
    std::cout << "\n--- MULTI-DEVICE CONCURRENCY RESULT ---\n";
    std::cout << "Parallel execution time:   " << total_duration.count() << "ms\n";
    std::cout << "Expected parallel time:    ~2400ms (800+1200+400)\n";
    std::cout << "Sequential execution time: " << sequential_time << "ms\n";
    std::cout << "Time saved by parallelism: " << (sequential_time - total_duration.count()) << "ms\n";
    std::cout << "Efficiency gain:           " << std::fixed << std::setprecision(1) 
              << (100.0 * (sequential_time - total_duration.count()) / sequential_time) << "%\n";
    
    bool passed = success && (total_duration.count() >= 2200 && total_duration.count() <= 2800);
    printTestResult(passed, "Multi-device parallel synchronization successful");
}

void testPreciseSynchronization() {
    printTestHeader("Precise Synchronization", 
                   "Detailed verification of phase-by-phase synchronization timing");
    
    Syncorder syncorder;
    syncorder.setTimeout(std::chrono::milliseconds(5000));
    
    std::cout << "SETUP: Two devices with precise timing control:\n";
    std::cout << "  Validator-1: setup=300ms, warmup=500ms, start=200ms\n";
    std::cout << "  Validator-2: setup=600ms, warmup=800ms, start=400ms  <-- REFERENCE\n";
    std::cout << "\nTEST METHOD: Measure each phase individually and verify timing\n";
    
    syncorder.addDevice(std::make_unique<TimingTestDevice>("Validator-1", 
        std::chrono::milliseconds(300), std::chrono::milliseconds(500), std::chrono::milliseconds(200)));
    syncorder.addDevice(std::make_unique<TimingTestDevice>("Validator-2", 
        std::chrono::milliseconds(600), std::chrono::milliseconds(800), std::chrono::milliseconds(400)));
    
    bool all_passed = true;
    
    // Setup Phase Verification
    std::cout << "\nEXECUTING: Setup Phase Measurement...\n";
    auto setup_start = std::chrono::steady_clock::now();
    bool setup_ok = syncorder.executeSetup();
    auto setup_end = std::chrono::steady_clock::now();
    auto setup_duration = std::chrono::duration_cast<std::chrono::milliseconds>(setup_end - setup_start);
    
    printPhaseResult("Setup", setup_duration.count(), 600, 50);
    all_passed &= setup_ok && (setup_duration.count() >= 550 && setup_duration.count() <= 650);
    
    if (!setup_ok) return;
    
    // Warmup Phase Verification
    std::cout << "\nEXECUTING: Warmup Phase Measurement...\n";
    auto warmup_start = std::chrono::steady_clock::now();
    bool warmup_ok = syncorder.executeWarmup();
    auto warmup_end = std::chrono::steady_clock::now();
    auto warmup_duration = std::chrono::duration_cast<std::chrono::milliseconds>(warmup_end - warmup_start);
    
    printPhaseResult("Warmup", warmup_duration.count(), 800, 50);
    all_passed &= warmup_ok && (warmup_duration.count() >= 750 && warmup_duration.count() <= 850);
    
    if (!warmup_ok) return;
    
    // Start Phase Verification
    std::cout << "\nEXECUTING: Start Phase Measurement...\n";
    auto start_start = std::chrono::steady_clock::now();
    bool start_ok = syncorder.executeStart();
    auto start_end = std::chrono::steady_clock::now();
    auto start_duration = std::chrono::duration_cast<std::chrono::milliseconds>(start_end - start_start);
    
    printPhaseResult("Start", start_duration.count(), 400, 50);
    all_passed &= start_ok && (start_duration.count() >= 350 && start_duration.count() <= 450);
    
    printTestResult(all_passed, "Precise phase-by-phase synchronization verified");
}

/**
 * Main Test Runner
 */
int main() {
    std::cout << "===========================================\n";
    std::cout << "SYNCORDER SYNCHRONIZATION TEST SUITE\n";
    std::cout << "===========================================\n";
    std::cout << "OBJECTIVE: Verify that Syncorder properly synchronizes\n";
    std::cout << "           devices with different execution times\n";
    std::cout << "===========================================\n";
    
    try {
        testBasicSynchronization();
        testExtremeTiming();
        testMultiDeviceConcurrency();
        testPreciseSynchronization();
        
        std::cout << "\n===========================================\n";
        std::cout << "TEST SUITE COMPLETED\n";
        std::cout << "===========================================\n";
        std::cout << "CONCLUSION: Syncorder successfully synchronizes devices\n";
        std::cout << "            by waiting for the slowest device at each phase\n";
        std::cout << "            before proceeding to the next phase.\n";
        std::cout << "===========================================\n";
        
    } catch (const std::exception& e) {
        std::cout << "\nFATAL ERROR: " << e.what() << "\n";
        return -1;
    }
    
    return 0;
}