#pragma once

#include <chrono>
#include <array>
#include <atomic>
#include <optional>
#include <iostream>
#include <memory>

// installed
#include <librealsense2/rs.hpp>

// local
#include <Syncorder/error/exception.h>
#include <Syncorder/buffer/common/base.h>
#include <Syncorder/model/realsense_model.h>


/**
 * @class Buffer
 */

constexpr std::size_t REALSENSE_RING_BUFFER_SIZE = 1024;

class RealsenseBuffer : public BBuffer<RealsenseBufferData, REALSENSE_RING_BUFFER_SIZE> {
protected:
    void onOverflow() noexcept override { std::cout << "[RealsenseBuffer Warning] Buffer overflow\n"; }
};