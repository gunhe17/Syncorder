#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <thread>

// local
#include <Syncorder/error/exception.h>
#include <Syncorder/syncorder.cpp>


int main() {
    try {
        std::cout << "=== Syncorder Multi-Device Recording ===\n\n";
        
        Syncorder syncorder;
        
        // Configuration
        syncorder.setTimeout(std::chrono::milliseconds(3000));
        
        // Device registration
        syncorder.addCamera(2);
        syncorder.addTobii(0);
        syncorder.addRealsense(0);
        
    
        // Start synchronization
        if (syncorder.start()) {
            // Simulate recording time
            std::this_thread::sleep_for(std::chrono::seconds(3));
            
            // Optional: stop early
            // syncorder.stop();
            
            // Wait for completion
            syncorder.waitForCompletion();
            
            if (syncorder.getState() == Syncorder::State::COMPLETED) {
            } else {
                std::cout << "\nRecording failed\n";
                return -1;
            }
            
        } else {
            std::cout << "\nFailed to start synchronization\n";
            return -1;
        }
        
    } catch (const std::exception& e) {
        std::cout << "[ERROR] " << e.what() << "\n";
        return -1;
    }
    
    return 0;
}