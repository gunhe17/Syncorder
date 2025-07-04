#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <thread>

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


/**
 * @class (Base) Callback
 */

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
 * @class (Base) Device
 */
class BDevice {
protected:
    int device_id_;

public:
    BDevice(int device_id) 
    :   device_id_(device_id)
    {}

    virtual ~BDevice() {}

    virtual bool init() final { EXCEPTION(return _init();) }

    virtual bool setup() final { EXCEPTION(return _setup();) }

    virtual bool warmup() final { EXCEPTION(return _warmup();) }

    virtual bool start() final { EXCEPTION(return _start();) }

    virtual bool stop() final { EXCEPTION(return _stop();) }

    virtual bool cleanup() final { EXCEPTION(return _cleanup();) }

protected:
    virtual bool _init() = 0;
    virtual bool _setup() = 0;
    virtual bool _warmup() = 0;
    virtual bool _start() = 0;
    virtual bool _stop() = 0;
    virtual bool _cleanup() = 0;
};