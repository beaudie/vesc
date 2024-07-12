//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_HASH_SET_H_
#define COMMON_HASH_SET_H_

#if defined(ANGLE_USE_ABSEIL)
#    include "absl/container/flat_hash_set.h"
#else
#    include <functional>
#    include <unordered_set>
#endif  // defined(ANGLE_USE_ABSEIL)

namespace angle
{

#if defined(ANGLE_USE_ABSEIL)
template <typename Key,
          class Hash = absl::container_internal::hash_default_hash<Key>,
          class Eq   = absl::container_internal::hash_default_eq<Key>>
using HashSet = absl::flat_hash_set<Key, Hash, Eq>;
#else
template <typename Key, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>>
using HashSet = std::unordered_set<Key, Hash, KeyEqual>;
#endif  // defined(ANGLE_USE_ABSEIL)

}  // namespace angle

#endif  // COMMON_HASH_SET_H_
