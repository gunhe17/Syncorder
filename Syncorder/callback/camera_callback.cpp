#pragma once


/**
 * @class Callback
 */

class CameraCallback
: 
    public BCallback, 
    public RuntimeClass<RuntimeClassFlags<ClassicCom>, IMFSourceReaderCallback> 
{
public:
    ComPtr<IMFSourceReader> reader_;

    HRESULT STDMETHODCALLTYPE OnReadSample(HRESULT, DWORD, DWORD, LONGLONG, IMFSample*) override {
        reader_->ReadSample(
            MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, nullptr, nullptr, nullptr, nullptr
        );

        std::cout << "callback run\n";
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE OnEvent(DWORD, IMFMediaEvent*) override { return S_OK; }
    HRESULT STDMETHODCALLTYPE OnFlush(DWORD) override { return S_OK; }

    void setup(ComPtr<IMFSourceReader> reader) {
        _setReader(reader);
    }

    // for BCallback
    IMFSourceReaderCallback* getComCallback() override {
        return this;
    }

private:
    void _setReader(ComPtr<IMFSourceReader> reader) {
        reader_ = reader;
    }
};