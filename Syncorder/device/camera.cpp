#pragma once

#include <windows.h>
#include <wrl/client.h>
#include <wrl/implements.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfobjects.h>
#include <mfreadwrite.h>
#include <mferror.h>

#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "ole32.lib")

using namespace Microsoft::WRL;

#include <Syncorder/error/exception.cpp>
#include <Syncorder/error/exception.h>
#include <Syncorder/device/base.cpp>


/**
 * @class () Callback
 */

// Empty callback (비워진 콜백)s
class EmptyCallback : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IMFSourceReaderCallback> {
public:
    HRESULT STDMETHODCALLTYPE OnReadSample(HRESULT, DWORD, DWORD, LONGLONG, IMFSample*) override {
        // 비워진 콜백 - 아무 작업 안함
        return S_OK;
    }
    
    HRESULT STDMETHODCALLTYPE OnEvent(DWORD, IMFMediaEvent*) override { return S_OK; }
    HRESULT STDMETHODCALLTYPE OnFlush(DWORD) override { return S_OK; }
};


/**
 * @class Device
 */
class CameraDevice : public BDevice {
private:
   bool mf_initialized_;
   ComPtr<IMFActivate> device_;
   ComPtr<EmptyCallback> callback_;
   ComPtr<IMFSourceReader> reader_;

public:
    CameraDevice(int device_id)
    : 
        BDevice(device_id), 
        mf_initialized_(false) 
    {}
    
    ~CameraDevice() {
        cleanup();
    }

public:  
    bool _init() override {
        _initializeMediaFoundation();

        return true;
    }
    
    bool _setup() override {
        device_ = _createDevice(device_id_);
        reader_ = _createSourceReader(device_, callback_);

        return true;
    }
    
    bool _warmup() override {
        // TODO: warm-up 로직 정의 필요

        return true;
    }
    
    bool _start() override {
        _runSourceReader();

        return true;
    }
    
    bool _stop() override {
        return true;
    }
    
    bool _cleanup() override {
        // reverse
        reader_.Reset();
        device_.Reset();
        callback_.Reset();
        _shutdownMediaFoundation();

        return true;
    }

private:
    void _initializeMediaFoundation() {
        HRESULT hr = MFStartup(MF_VERSION);
        if (FAILED(hr)) throw MediaFoundationError("MFStartup failed");
        
        mf_initialized_ = true;
    }
    
    void _shutdownMediaFoundation() {
        if (mf_initialized_) {
            MFShutdown();
            mf_initialized_ = false;
        }
    }
    
    ComPtr<IMFActivate> _createDevice(int index) {
        HRESULT hr = S_OK;
        
        ComPtr<IMFAttributes> attributes;
        hr = MFCreateAttributes(&attributes, 1);
        if (FAILED(hr)) throw MediaFoundationError("Device attributes creation failed");
        
        hr = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
        if (FAILED(hr)) throw MediaFoundationError("Device attributes setup failed");
        
        IMFActivate** devices_raw = nullptr;
        UINT32 count = 0;
        hr = MFEnumDeviceSources(attributes.Get(), &devices_raw, &count);
        if (FAILED(hr)) throw MediaFoundationError("Device enumeration failed");
        
        if (index >= static_cast<int>(count)) {
            CoTaskMemFree(devices_raw);
            throw MediaFoundationError("Device index out of range");
        }
        
        ComPtr<IMFActivate> device(devices_raw[index]);
        CoTaskMemFree(devices_raw);
        
        return device;
    }
    
    ComPtr<IMFSourceReader> _createSourceReader(ComPtr<IMFActivate> device, ComPtr<IMFSourceReaderCallback> callback) {
        HRESULT hr = S_OK;
        
        ComPtr<IMFMediaSource> source;
        hr = device->ActivateObject(IID_PPV_ARGS(&source));
        if (FAILED(hr)) throw MediaFoundationError("Device activation failed");
        
        ComPtr<IMFAttributes> attributes;
        hr = MFCreateAttributes(&attributes, 1);
        if (FAILED(hr)) throw MediaFoundationError("Reader attributes creation failed");
        
        hr = attributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, callback.Get());
        if (FAILED(hr)) throw MediaFoundationError("Callback setup failed");
        
        ComPtr<IMFSourceReader> reader;
        hr = MFCreateSourceReaderFromMediaSource(source.Get(), attributes.Get(), &reader);
        if (FAILED(hr)) throw MediaFoundationError("SourceReader creation failed");
        
        return reader;
    }

    void _runSourceReader() {
        return;
    }
};