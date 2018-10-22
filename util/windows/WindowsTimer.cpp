//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WindowsTimer.cpp: Implementation of a high precision timer class on Windows

#include "windows/WindowsTimer.h"

namespace
{
LONGLONG getFrequency()
{
    static LARGE_INTEGER frequency = {};

    if (frequency.QuadPart == 0)
    {
        QueryPerformanceFrequency(&frequency);
    }

    return frequency.QuadPart;
}
}  // anonymous namespace

WindowsTimer::WindowsTimer() : mRunning(false), mStartTime(0), mStopTime(0)
{
}

void WindowsTimer::start()
{
    LARGE_INTEGER curTime;
    QueryPerformanceCounter(&curTime);
    mStartTime = curTime.QuadPart;

    mRunning = true;
}

void WindowsTimer::stop()
{
    LARGE_INTEGER curTime;
    QueryPerformanceCounter(&curTime);
    mStopTime = curTime.QuadPart;

    mRunning = false;
}

double WindowsTimer::getElapsedTime() const
{
    LONGLONG endTime;
    if (mRunning)
    {
        LARGE_INTEGER curTime;
        QueryPerformanceCounter(&curTime);
        endTime = curTime.QuadPart;
    }
    else
    {
        endTime = mStopTime;
    }

    return static_cast<double>(endTime - mStartTime) / getFrequency();
}

double WindowsTimer::getAbsoluteTime() const
{
    LARGE_INTEGER curTime;
    QueryPerformanceCounter(&curTime);

    return static_cast<double>(curTime) / getFrequency();
}

Timer *CreateTimer()
{
    return new WindowsTimer();
}
