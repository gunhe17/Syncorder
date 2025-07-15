#pragma once

#include <chrono>
#include <array>
#include <atomic>
#include <optional>
#include <iostream>

// installed
#include "tobii_research.h"

// local
#include <Syncorder/error/exception.h>
#include <Syncorder/buffer/common/base.h>
#include <Syncorder/model/tobii_model.h>


/**
 * @class Buffer
 */

constexpr std::size_t TOBII_RING_BUFFER_SIZE = 2048;

class TobiiBuffer : public BBuffer<TobiiBufferData, TOBII_RING_BUFFER_SIZE> {
protected:
    void onOverflow() noexcept override { std::cout << "[TobiiBuffer Warning] Buffer overflow\n"; }
};