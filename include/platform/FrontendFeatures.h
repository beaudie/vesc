//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FrontendFeatures.h: Features/workarounds for driver bugs and other behaviors seen
// on all platforms.

#ifndef ANGLE_PLATFORM_FRONTENDFEATURES_H_
#define ANGLE_PLATFORM_FRONTENDFEATURES_H_

#include "platform/Feature.h"

namespace angle
{

struct FrontendFeatures : angle::FeatureSetBase
{
    FrontendFeatures();
    ~FrontendFeatures();

    // Force the context to be lost (via KHR_robustness) if a GL_OUT_OF_MEMORY error occurs. The
    // driver may be in an inconsistent state if this happens, and some users of ANGLE rely on this
    // notification to prevent further execution.
    angle::Feature loseContextOnOutOfMemory = {
        "lose_context_on_out_of_memory", angle::FeatureCategory::FrontendWorkarounds,
        "Some users rely on a lost context notification if a GL_OUT_OF_MEMORY "
        "error occurs",
        &members};

    // Program binaries don't contain transform feedback varyings on Qualcomm GPUs.
    // Work around this by disabling the program cache for programs with transform feedback.
    angle::Feature disableProgramCachingForTransformFeedback = {
        "disable_program_caching_for_transform_feedback",
        angle::FeatureCategory::FrontendWorkarounds,
        "On some GPUs, program binaries don't contain transform feedback varyings", &members};

    // On Windows Intel OpenGL drivers TexImage sometimes seems to interact with the Framebuffer.
    // Flaky crashes can occur unless we sync the Framebuffer bindings. The workaround is to add
    // Framebuffer binding dirty bits to TexImage updates. See http://anglebug.com/2906
    angle::Feature syncFramebufferBindingsOnTexImage = {
        "sync_framebuffer_bindings_on_tex_image", angle::FeatureCategory::FrontendWorkarounds,
        "On some drivers TexImage sometimes seems to interact "
        "with the Framebuffer",
        &members};

    angle::Feature scalarizeVecAndMatConstructorArgs = {
        "scalarize_vec_and_mat_constructor_args", angle::FeatureCategory::FrontendWorkarounds,
        "Always rewrite vec/mat constructors to be consistent", &members,
        "http://crbug.com/398694"};

    // Allow disabling of GL_EXT_texture_filter_anisotropic through a runtime feature for
    // performance comparisons.
    angle::Feature disableAnisotropicFiltering = {
        "disable_anisotropic_filtering", angle::FeatureCategory::FrontendWorkarounds,
        "Disable support for anisotropic filtering", &members};

    // Force a level-of-detail offset to all texture sampling. Arbitrary offsets up to 15 can be
    // encoded using a combination of force_texture_lod_offset_* features. Using these features
    // makes ANGLE non-conformant.
    std::array<angle::Feature, 4> forceTextureLODOffset = {
        angle::Feature{"force_texture_lod_offset_1", angle::FeatureCategory::FrontendWorkarounds,
                       "Increase the minimum texture level-of-detail by 1 when sampling.",
                       &members},
        angle::Feature{"force_texture_lod_offset_2", angle::FeatureCategory::FrontendWorkarounds,
                       "Increase the minimum texture level-of-detail by 2 when sampling.",
                       &members},
        angle::Feature{"force_texture_lod_offset_4", angle::FeatureCategory::FrontendWorkarounds,
                       "Increase the minimum texture level-of-detail by 4 when sampling.",
                       &members},
        angle::Feature{"force_texture_lod_offset_8", angle::FeatureCategory::FrontendWorkarounds,
                       "Increase the minimum texture level-of-detail by 8 when sampling.",
                       &members},
    };
};

inline FrontendFeatures::FrontendFeatures()  = default;
inline FrontendFeatures::~FrontendFeatures() = default;

}  // namespace angle

#endif  // ANGLE_PLATFORM_FRONTENDFEATURES_H_
