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
#include <Syncorder/error/exception.h>
#include <Syncorder/callback/base.h>


/**
 * @class Callback
 */

class CameraCallback
: 
    public BCallback, 
    public RuntimeClass<RuntimeClassFlags<ClassicCom>, IMFSourceReaderCallback> 
{
private:
    ComPtr<IMFSourceReader> reader_;

public:
    void pre_setup(ComPtr<IMFSourceReader> reader) {
        reader_ = reader;
    }

    void setup() {
        
    }

    // get
    IUnknown* getIUnknown() {
        return static_cast<IUnknown*>(this);
    }

public:
    HRESULT STDMETHODCALLTYPE OnReadSample(HRESULT, DWORD, DWORD, LONGLONG, IMFSample*) override {
        reader_->ReadSample(
            MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, nullptr, nullptr, nullptr, nullptr
        );

        std::cout << "callback run\n";
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE OnEvent(DWORD, IMFMediaEvent*) override { return S_OK; }
    HRESULT STDMETHODCALLTYPE OnFlush(DWORD) override { return S_OK; }
};