//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// UniformLinker.h: implements link-time checks for default block uniforms, and generates uniform
// locations. Populates data structures related to uniforms so that they can be stored in program
// state.

#ifndef LIBANGLE_UNIFORMLINKER_H_
#define LIBANGLE_UNIFORMLINKER_H_

#include "libANGLE/Program.h"
#include "libANGLE/Uniform.h"

namespace gl
{

class UniformLinker
{
  public:
    UniformLinker(const ProgramState &state);

    bool link(InfoLog &infoLog, const Caps &caps, const Program::Bindings &uniformLocationBindings);

    void getResults(std::vector<LinkedUniform> *uniforms,
                    std::vector<VariableLocation> *uniformLocations);

  private:
    struct LimitCounts
    {
        LimitCounts() : vector(0), sampler(0), atomicCounter(0) {}
        LimitCounts(const LimitCounts &other) = default;
        LimitCounts &operator=(const LimitCounts &other) = default;

        LimitCounts &operator+=(const LimitCounts &other)
        {
            vector += other.vector;
            sampler += other.sampler;
            atomicCounter += other.atomicCounter;
            return *this;
        }

        unsigned int vector;
        unsigned int sampler;
        unsigned int atomicCounter;
    };

    bool validateVertexAndFragmentUniforms(InfoLog &infoLog) const;

    static bool linkValidateUniforms(InfoLog &infoLog,
                                     const std::string &uniformName,
                                     const sh::Uniform &vertexUniform,
                                     const sh::Uniform &fragmentUniform);

    bool flattenUniformsAndCheckCapsForShader(const Shader &shader,
                                              GLuint maxUniformComponents,
                                              GLuint maxTextureImageUnits,
                                              GLuint maxAtomicCounters,
                                              const std::string &componentsErrorMessage,
                                              const std::string &samplerErrorMessage,
                                              const std::string &atomicCounterErrorMessage,
                                              std::vector<LinkedUniform> &samplerUniforms,
                                              InfoLog &infoLog);
    bool flattenUniformsAndCheckCaps(const Caps &caps, InfoLog &infoLog);
    bool checkMaxCombinedAtomicCounters(const Caps &caps, InfoLog &infoLog);

    LimitCounts flattenUniform(const sh::Uniform &uniform,
                               std::vector<LinkedUniform> *samplerUniforms);

    // markStaticUse is given as a separate parameter because it is tracked here at struct
    // granularity.
    LimitCounts flattenUniformImpl(const sh::ShaderVariable &uniform,
                                   const std::string &fullName,
                                   std::vector<LinkedUniform> *samplerUniforms,
                                   bool markStaticUse,
                                   int binding,
                                   int offset,
                                   int *location);

    bool indexUniforms(InfoLog &infoLog, const Program::Bindings &uniformLocationBindings);
    bool gatherUniformLocationsAndCheckConflicts(InfoLog &infoLog,
                                                 const Program::Bindings &uniformLocationBindings,
                                                 std::set<GLuint> *reservedLocations,
                                                 std::set<GLuint> *ignoredLocations,
                                                 int *maxUniformLocation);
    void pruneUnusedUniforms();

    const ProgramState &mState;
    std::vector<LinkedUniform> mUniforms;
    std::vector<VariableLocation> mUniformLocations;
};

}  // namespace gl

#endif  // LIBANGLE_UNIFORMLINKER_H_
