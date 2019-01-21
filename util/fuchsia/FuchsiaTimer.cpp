//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FuchsiaTimer.cpp:
//    Implementation of a high precision timer class on Fuchsia
//

#include "util/fuchsia/FuchsiaTimer.h"
#include <iostream>

FuchsiaTimer::FuchsiaTimer() : mRunning(false) {}

namespace
{
uint64_t getCurrentTimeNs()
{
    struct timespec currentTime;
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    return currentTime.tv_sec * 1'000'000'000llu + currentTime.tv_nsec;
}
}  // anonymous namespace

void FuchsiaTimer::start()
{
    mStartTimeNs = getCurrentTimeNs();
    mRunning     = true;
}

void FuchsiaTimer::stop()
{
    mStopTimeNs = getCurrentTimeNs();
    mRunning    = false;
}

double FuchsiaTimer::getElapsedTime() const
{
    uint64_t endTimeNs;
    if (mRunning)
    {
        endTimeNs = getCurrentTimeNs();
    }
    else
    {
        endTimeNs = mStopTimeNs;
    }

    return (endTimeNs - mStartTimeNs) * 1e-9;
}

double FuchsiaTimer::getAbsoluteTime()
{
    return getCurrentTimeNs() * 1e-9;
}

Timer *CreateTimer()
{
    return new FuchsiaTimer();
}
