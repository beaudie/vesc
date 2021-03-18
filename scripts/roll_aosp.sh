#!/bin/bash

#  Copyright The ANGLE Project Authors. All rights reserved.
#  Use of this source code is governed by a BSD-style license that can be
#  found in the LICENSE file.
#
# Generates a roll CL within the ANGLE repository of AOSP.

# exit when any command fails
set -e

# Change the working directory to the ANGLE root directory
cd "${0%/*}/.."

# Check out depot_tools locally and add it to the path
DEPOT_TOOLS_DIR=_depot_tools
rm -rf ${DEPOT_TOOLS_DIR}
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git ${DEPOT_TOOLS_DIR}
export PATH=`pwd`/${DEPOT_TOOLS_DIR}:$PATH

GN_OUTPUT_DIRECTORY=out/Android

deps=(
    "third_party/abseil-cpp"
    "third_party/jsoncpp"
    "third_party/jsoncpp/source"
    "third_party/vulkan-deps/glslang/src"
    "third_party/vulkan-deps/spirv-headers/src"
    "third_party/vulkan-deps/spirv-tools/src"
    "third_party/vulkan-deps/vulkan-headers/src"
    "third_party/vulkan_memory_allocator"
    "third_party/zlib"
)

# Only add the parts of NDK and vulkan-deps that are required by ANGLE. The entire dep is too large.
delete_only_deps=(
    "third_party/android_ndk"
    "third_party/vulkan-deps"
)

add_only_deps=(
    "third_party/android_ndk/sources/android/cpufeatures"
)

# Delete dep directories so that gclient can check them out
for dep in ${deps[@]} ${delete_only_deps[@]}; do
    rm -rf $dep
done

# Sync all of ANGLE's deps so that 'gn gen' works
python scripts/bootstrap.py
gclient sync --reset --force --ignore_locks --delete_unversioned_trees --break_repo_locks

./generate_android_bp.sh ${DEPOT_TOOLS_DIR} ${GN_OUTPUT_DIRECTORY}

rm -rf ${GN_OUTPUT_DIRECTORY}
git add Android.bp

# Delete the .git files in each dep so that it can be added to this repo. Some deps like jsoncpp
# have multiple layers of deps so delete everything before adding them.
for dep in ${deps[@]} ${delete_only_deps[@]}; do
   rm -rf $dep/.git
done

extra_removal_files=(
   # Some third_party deps have OWNERS files which contains users that have not logged into
   # the Android gerrit. Repo cannot upload with these files present.
   "third_party/abseil-cpp/OWNERS"
   "third_party/jsoncpp/OWNERS"
   "third_party/vulkan_memory_allocator/OWNERS"
   "third_party/zlib/OWNERS"
   "third_party/zlib/google/OWNERS"
   "third_party/zlib/contrib/tests/OWNERS"
   "third_party/zlib/contrib/bench/OWNERS"
   "third_party/zlib/contrib/tests/fuzzers/OWNERS"
)

for removal_file in ${extra_removal_files[@]}; do
   rm -f $removal_file
done

for dep in ${deps[@]} ${add_only_deps[@]}; do
   git add -f $dep
done

# Done with depot_tools
rm -rf $DEPOT_TOOLS_DIR
