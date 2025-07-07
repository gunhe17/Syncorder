#pragma once

#include <algorithm>

// installed
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

// local
#include <Syncorder/error/exception.h>
#include <Syncorder/device/base.h>


/**
 * @class Callback
 */
class EmptyCallback : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IMFSourceReaderCallback> {
public:
    HRESULT STDMETHODCALLTYPE OnReadSample(HRESULT, DWORD, DWORD, LONGLONG, IMFSample*) override {
        std::cout << "something";
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
   ComPtr<IMFActivate> device_;
   ComPtr<EmptyCallback> callback_;
   ComPtr<IMFSourceReader> reader_;

public:
    CameraDevice(int device_id)
    : 
        BDevice(device_id)
    {}
    
    ~CameraDevice() {
        cleanup();
    }

public:      
    bool _setup() override {
        _startMF();

        device_ = _createDevice(device_id_);
        callback_ = _createCallback();
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
        
        _endMF();

        return true;
    }

private:
    void _startMF() {
        HRESULT hr = MFStartup(MF_VERSION);
        if (FAILED(hr)) throw MediaFoundationError("MFStartup failed");
    }
    
    void _endMF() {
        MFShutdown();
    }
    
    ComPtr<IMFActivate> _createDevice(int index) {
        HRESULT hr = S_OK;

        ComPtr<IMFAttributes> attributes;

        IMFActivate** devices_raw = nullptr;
        UINT32 device_count = 0;

        hr = MFCreateAttributes(&attributes, 1);
        if (FAILED(hr)) throw MediaFoundationError("Device attributes creation failed");

        hr = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
        if (FAILED(hr)) throw MediaFoundationError("Device attributes setup failed");

        // every device
        hr = MFEnumDeviceSources(attributes.Get(), &devices_raw, &device_count);
        if (FAILED(hr)) throw MediaFoundationError("Device enumeration failed");

        IMFActivate* target_device = devices_raw[index];
        ComPtr<IMFActivate> selected_device = target_device;

        // validation
        BOOL match = FALSE;
        bool is_valid = std::any_of(
            devices_raw, devices_raw + device_count,
            [&](IMFActivate* dev) {
                match = FALSE;
                return SUCCEEDED(target_device->Compare(dev, MF_ATTRIBUTES_MATCH_INTERSECTION, &match)) && match;
            }
        );

        CoTaskMemFree(devices_raw);

        if (!is_valid) throw MediaFoundationError("Target device not matched in enumeration");

        return selected_device;
    }


    ComPtr<EmptyCallback> _createCallback() {
        return Microsoft::WRL::Make<EmptyCallback>();
    }
    
    ComPtr<IMFSourceReader> _createSourceReader(ComPtr<IMFActivate> device, ComPtr<IMFSourceReaderCallback> callback) {
        HRESULT hr = S_OK;
        
        ComPtr<IMFMediaSource> source;
        ComPtr<IMFAttributes> attributes;
        ComPtr<IMFSourceReader> reader;
        ComPtr<IMFMediaType> type;

        hr = device->ActivateObject(IID_PPV_ARGS(&source));
        if (FAILED(hr)) throw MediaFoundationError("Device activation failed");
        
        hr = MFCreateAttributes(&attributes, 1);
        if (FAILED(hr)) throw MediaFoundationError("Reader attributes creation failed");
        
        hr = attributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, callback.Get());
        if (FAILED(hr)) throw MediaFoundationError("Callback setup failed");
        
        hr = MFCreateSourceReaderFromMediaSource(source.Get(), attributes.Get(), &reader);
        if (FAILED(hr)) throw MediaFoundationError("SourceReader creation failed");

        hr = MFCreateMediaType(&type);
        if (FAILED(hr)) throw MediaFoundationError("Type creation failed");

        type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_MJPG); //TODO: Config
        type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
        type->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);

        MFSetAttributeSize(type.Get(), MF_MT_FRAME_SIZE, 1280, 720); //TODO: Config
        MFSetAttributeRatio(type.Get(), MF_MT_FRAME_RATE, 30, 1); //TODO: Config

        hr = reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, type.Get());
        if (FAILED(hr)) {
            std::cout << "Type setting failed. HRESULT = 0x" << std::hex << hr;
            throw MediaFoundationError("Type setting failed\n");
        }

        return reader;
    }

    void _runSourceReader() {
        reader_->ReadSample(
            MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, nullptr, nullptr, nullptr, nullptr
        );
    }
};