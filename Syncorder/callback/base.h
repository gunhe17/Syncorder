#pragma once

#include <Syncorder/error/exception.h>

/**
 * @class BCallback - Base Callback Interface (Header-only)
 */

class BCallback {
public:
    BCallback() {}
    virtual ~BCallback() {}

    // for device callback
    virtual IMFSourceReaderCallback* getComCallback() {
        return nullptr;
    }
};