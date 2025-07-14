#pragma once

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
#include <Syncorder/buffer/camera_buffer.cpp>
#include <Syncorder/error/exception.h>


/**
 * @class Callback
 */

class CameraCallback
: 
    public RuntimeClass<RuntimeClassFlags<ClassicCom>, IMFSourceReaderCallback> 
{
private:
    ComPtr<IMFSourceReader> reader_;
    void* buffer_;

public:
    void pre_setup(ComPtr<IMFSourceReader> reader, void* buffer) {
        reader_ = reader;
        buffer_ = buffer;
    }

    void setup() {}

    // get
    IUnknown* getIUnknown() {
        return static_cast<IUnknown*>(this);
    }

public:
    HRESULT STDMETHODCALLTYPE OnReadSample(HRESULT hr, DWORD, DWORD, LONGLONG timestamp, IMFSample* sample) override {
        if (buffer_ && sample) {
            auto* cam_buffer = static_cast<CameraBuffer*>(buffer_);
            CameraBufferData data = _map(sample, timestamp);
            cam_buffer->enqueue(std::move(data));
        }
        
        reader_->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, nullptr, nullptr, nullptr, nullptr);
        std::cout << "Cam: callback run\n";
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE OnEvent(DWORD, IMFMediaEvent*) override { return S_OK; }
    HRESULT STDMETHODCALLTYPE OnFlush(DWORD) override { return S_OK; }

private: 
    CameraBufferData _map(IMFSample* sample, LONGLONG ts) {
        return CameraBufferData(
            ComPtr<IMFSample>(sample),
            std::chrono::system_clock::now(),
            ts
        );
    }
};