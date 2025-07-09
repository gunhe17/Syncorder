// ardu.cpp 수정
#pragma once

#include <windows.h>

#include <Syncorder/error/exception.h>
#include <Syncorder/device/base.h>


class ArduDevice : public BDevice {
private:
    HMODULE dll_handle_;
    void* device_list_;
    int device_count_;
    
public:
    ArduDevice(int device_id = 0)
    :
        BDevice(device_id),
        dll_handle_(nullptr),
        device_list_(nullptr),
        device_count_(0)
    {}
    
    ~ArduDevice() {
        cleanup();
    }

public:
    bool _setup() override {
        std::cout << "[ArduDevice] Setup started\n";
        
        try {
            SetDllDirectoryA("C:\\Users\\user\\Workspace\\evk_sdk\\bin");
            
            _loadDLL();
            _findDevices();
            
            std::cout << "[ArduDevice] Setup completed successfully\n";
            return true;
            
        } catch (const std::exception& e) {
            std::cout << "[ArduDevice] Setup failed: " << e.what() << "\n";
            throw ArducamDeviceError(e.what());
        }
    }
    
    bool _warmup() override {
        std::cout << "[ArduDevice] Warmup completed\n";
        return true;
    }
    
    bool _start() override {
        std::cout << "[ArduDevice] Start completed\n";
        return true;
    }
    
    bool _stop() override {
        std::cout << "[ArduDevice] Stop completed\n";
        return true;
    }
    
    bool _cleanup() override {
        std::cout << "[ArduDevice] Cleanup started\n";
        
        if (dll_handle_) {
            FreeLibrary(dll_handle_);
            dll_handle_ = nullptr;
        }
        
        device_list_ = nullptr;
        device_count_ = 0;
        
        std::cout << "[ArduDevice] Cleanup completed\n";
        return true;
    }

private:
    void _loadDLL() {
        std::cout << "[ArduDevice] Loading DLL...\n";
        
        dll_handle_ = LoadLibraryA("arducam_evk_cpp_sdk.dll");
        if (!dll_handle_) {
            throw ArducamDeviceError("Failed to load Arducam DLL");
        }
        
        std::cout << "[ArduDevice] DLL loaded successfully\n";
    }
    
    void _findDevices() {
        std::cout << "[ArduDevice] Finding devices using C API...\n";
        
        typedef int (*ArducamListDeviceFunc)(void**);
        
        FARPROC func = GetProcAddress(dll_handle_, "ArducamListDevice");
        if (!func) {
            throw ArducamDeviceError("ArducamListDevice function not found");
        }
        
        int result = ((ArducamListDeviceFunc)func)(&device_list_);
        std::cout << "ArducamListDevice returned: " << result << "\n";

        if (result == 0) {
            std::cout << "No devices found - possible causes:\n";
            std::cout << "1. Camera not connected\n";
            std::cout << "2. Driver not installed\n";
            std::cout << "3. USB permission issue\n";
        } else if (result > 0) {
            std::cout << "Found " << result << " device(s)\n";
        } else {
            std::cout << "Error occurred (negative return value)\n";
        }

        std::cout << "_findDevices end";
    }
};