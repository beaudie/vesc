//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_HASH_MAP_H_
#define COMMON_HASH_MAP_H_

#if defined(ANGLE_USE_ABSEIL)
#    include "absl/container/flat_hash_map.h"
#else
#    include <functional>
#    include <unordered_map>
#endif  // defined(ANGLE_USE_ABSEIL)

namespace angle
{

#if defined(ANGLE_USE_ABSEIL)
template <typename Key,
          typename T,
          class Hash = absl::container_internal::hash_default_hash<Key>,
          class Eq   = absl::container_internal::hash_default_eq<Key>>
using HashMap = absl::flat_hash_map<Key, T, Hash, Eq>;

// Absl has generic lookup unconditionally
#    define ANGLE_HAS_HASH_MAP_GENERIC_LOOKUP 1
#else
template <typename Key,
          typename T,
          class Hash     = std::hash<Key>,
          class KeyEqual = std::equal_to<Key>>
using HashMap = std::unordered_map<Key, T, Hash, KeyEqual>;
#    if __cpp_lib_generic_unordered_lookup >= 201811L
#        define ANGLE_HAS_HASH_MAP_GENERIC_LOOKUP 1
#    else
#        define ANGLE_HAS_HASH_MAP_GENERIC_LOOKUP 0
#    endif
#endif  // defined(ANGLE_USE_ABSEIL)

}  // namespace angle

#endif  // COMMON_HASH_MAP_H_
