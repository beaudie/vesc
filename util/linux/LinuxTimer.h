//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef SAMPLE_UTIL_LINUX_TIMER_H
#define SAMPLE_UTIL_LINUX_TIMER_H

#include <time.h>

#include "Timer.h"

class LinuxTimer : public Timer
{
  public:
    LinuxTimer();

    void start();
    void stop();
    double getElapsedTime() const ;

  private:
    bool mRunning;
    struct timespec mStartTime;
    struct timespec mStopTime;
};

#endif // SAMPLE_UTIL_LINUX_TIMER_H
