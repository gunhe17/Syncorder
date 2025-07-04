#pragma once

#include <windows.h>
#include <string>
#include <functional>
#include <chrono>

// Tobii Research SDK
#include "tobii_research.h"
#include "tobii_research_eyetracker.h"
#include "tobii_research_calibration.h"
#include "tobii_research_streams.h"

#pragma comment(lib, "tobii_research.lib")

#include <Syncorder/error/exception.h>
#include <Syncorder/device/base.h>