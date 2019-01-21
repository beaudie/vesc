//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FuchsiaTimer.h:
//    Definition of a high precision timer class on Fuchsia.
//

#ifndef UTIL_FUCHSIA_FUCHSIA_TIMER_H
#define UTIL_FUCHSIA_FUCHSIA_TIMER_H

#include <stdint.h>
#include <time.h>

#include "util/Timer.h"

class ANGLE_UTIL_EXPORT FuchsiaTimer : public Timer
{
  public:
    FuchsiaTimer();

    void start() override;
    void stop() override;
    double getElapsedTime() const override;

    double getAbsoluteTime() override;

  private:
    bool mRunning;
    uint64_t mStartTimeNs;
    uint64_t mStopTimeNs;
};

#endif  // UTIL_FUCHSIA_FUCHSIA_TIMER_H
