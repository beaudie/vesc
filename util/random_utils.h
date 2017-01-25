//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// random_utils:
//   Helper functions for random number generation.
//

#ifndef UTIL_RANDOM_UTILS_H
#define UTIL_RANDOM_UTILS_H

// TODO(jmadill): Rework this if Chromium decides to ban <random>
#include <random>

#include <export.h>

namespace angle
{

class ANGLE_EXPORT RNG
{
  public:
    // Seed from clock
    RNG();
    // Seed from fixed number.
    RNG(unsigned int seed);
    ~RNG();

    void reseed(unsigned int newSeed);

    int randomInt();
    int randomIntBetween(int min, int max);
    unsigned int randomUInt();
    float randomFloat();
    float randomFloatBetween(float min, float max);
    float randomNegativeOneToOne();
    std::vector<uint8_t> randomUByteVector(size_t size);

  private:
    std::default_random_engine mGenerator;
};

inline std::vector<uint8_t> RandomVector(size_t size)
{
    RNG rng;
    return rng.randomUByteVector(size);
}

}  // namespace angle

#endif // UTIL_RANDOM_UTILS_H
