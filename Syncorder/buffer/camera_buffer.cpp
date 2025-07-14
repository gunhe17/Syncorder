#pragma once

#include <chrono>
#include <array>
#include <atomic>
#include <optional>
#include <iostream>

// installed
#include <windows.h>
#include <wrl/client.h>
#include <mfobjects.h>

using namespace Microsoft::WRL;

// local
#include <Syncorder/error/exception.h>
#include <Syncorder/buffer/common/base.h>


/**
 * @struct Data
 */

struct CameraBufferData {
    // sample
    ComPtr<IMFSample> sample_;
    
    // time
    std::chrono::system_clock::time_point sys_time_;
    LONGLONG mf_ts_;
    
    // metadata
    DWORD stream_index_;
    DWORD flags_;

public:
    CameraBufferData() {}
    CameraBufferData(
        ComPtr<IMFSample> sample,
        std::chrono::system_clock::time_point sys_time,
        LONGLONG mf_ts,
        DWORD stream_index = 0,
        DWORD flags = 0
    ) {
        sample_ = std::move(sample);
        sys_time_ = sys_time;
        mf_ts_ = mf_ts;
        stream_index_ = stream_index;
        flags_ = flags;
    }
};


/**
 * @class Buffer
 */

constexpr std::size_t CAMERA_RING_BUFFER_SIZE = 1024;

class CameraBuffer : public BBuffer<CameraBufferData, CAMERA_RING_BUFFER_SIZE> {
protected:
    void onOverflow() noexcept override { std::cout << "[CameraBuffer Warning] Buffer overflow\n"; }
};