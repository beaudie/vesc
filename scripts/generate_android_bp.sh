#!/bin/bash

#  Copyright The ANGLE Project Authors. All rights reserved.
#  Use of this source code is governed by a BSD-style license that can be
#  found in the LICENSE file.
#
# Generates the Android.bp file in cwd.
# Usage - ./scripts/generate_android_bp.sh ${DEPOT_TOOLS} ${GN_OUTPUT_DIRECTORY}

# exit when any command fails
set -e

# Error out if depot_tools path or output directory was not specified
if [[ $1 == "" || $2 == "" ]]; then
	echo "Usage - ./scripts/generate_android_bp.sh DEPOT_TOOLS_PATH GN_OUTPUT_DIRECTORY"
	exit 1
fi

# Error out if path provided for depot_tools or output directory was invalid
if [[ ! -d $1 || ! -d $2 ]]; then
	echo "Need to provide a valid depot tools path and output directory"
	exit 1
fi

# Change the working directory to the ANGLE root directory
cd "${0%/*}/.."

# Check out depot_tools locally and add it to the path
DEPOT_TOOLS_DIR=$1
export PATH=`pwd`/${DEPOT_TOOLS_DIR}:$PATH

GN_OUTPUT_DIRECTORY=$2

abis=(
    "arm"
    "arm64"
    "x86"
    "x64"
)

rm -rf ${GN_OUTPUT_DIRECTORY}
for abi in ${abis[@]}; do
    # generate gn build files and convert them to blueprints
    gn_args=(
        "target_os = \"android\""
        "is_component_build = false"
        "is_debug = false"

        # Build for 64-bit CPUs
        "target_cpu = \"$abi\""

        # Target ndk API 26 to make sure ANGLE can use the Vulkan backend on Android
        "android32_ndk_api_level = 26"
        "android64_ndk_api_level = 26"

        # Disable all backends except Vulkan
        "angle_enable_vulkan = true"
        "angle_enable_gl = true" # TODO(geofflang): Disable GL once Andrid no longer requires it. anglebug.com/4444
        "angle_enable_d3d9 = false"
        "angle_enable_d3d11 = false"
        "angle_enable_null = false"
        "angle_enable_metal = false"

        # SwiftShader is loaded as the system Vulkan driver on Android, not compiled by ANGLE
        "angle_enable_swiftshader = false"

        # Disable all shader translator targets except desktop GL (for Vulkan)
        "angle_enable_essl = true" # TODO(geofflang): Disable ESSL once Andrid no longer requires it. anglebug.com/4444
        "angle_enable_glsl = true" # TODO(geofflang): Disable ESSL once Andrid no longer requires it. anglebug.com/4444
        "angle_enable_hlsl = false"

        "angle_enable_commit_id = false"

        # Disable histogram/protobuf support
        "angle_has_histograms = false"

        # Disable _LIBCPP_ABI_UNSTABLE, since it breaks std::string
        "libcxx_abi_unstable = false"
    )

    gn gen ${GN_OUTPUT_DIRECTORY} --args="${gn_args[*]}"
    gn desc ${GN_OUTPUT_DIRECTORY} --format=json "*" > ${GN_OUTPUT_DIRECTORY}/desc.$abi.json
done

python scripts/generate_android_bp.py \
    ${GN_OUTPUT_DIRECTORY}/desc.arm.json \
    ${GN_OUTPUT_DIRECTORY}/desc.arm64.json \
    ${GN_OUTPUT_DIRECTORY}/desc.x86.json \
    ${GN_OUTPUT_DIRECTORY}/desc.x64.json > Android.bp
