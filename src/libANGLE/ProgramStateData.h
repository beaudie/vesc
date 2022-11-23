//
// Created by yuxinhu on 11/28/22.
//

#ifndef LIBANGLE_PROGRAMSTATEDATA_H_
#define LIBANGLE_PROGRAMSTATEDATA_H_

#include <GLSLANG/ShaderVars.h>
#include <libANGLE/Uniform.h>
#include <libANGLE/renderer/renderer_utils.h>
#include <string>

namespace gl
{

struct ProgramStateData
{
    ProgramStateData();
    ~ProgramStateData();

    std::string mLabel;

    sh::WorkGroupSize mComputeShaderLocalSize;

    uint32_t mLocationsUsedForXfbExtension;
    std::vector<std::string> mTransformFeedbackVaryingNames;

    std::vector<VariableLocation> mUniformLocations;
    std::vector<BufferVariable> mBufferVariables;

    bool mBinaryRetrieveableHint;
    bool mSeparable;
    rx::SpecConstUsageBits mSpecConstUsageBits;

    // ANGLE_multiview.
    int mNumViews;

    // GL_ANGLE_multi_draw
    int mDrawIDLocation;

    // GL_ANGLE_base_vertex_base_instance_shader_builtin
    int mBaseVertexLocation;
    int mBaseInstanceLocation;
    // Cached value of base vertex and base instance
    // need to reset them to zero if using non base vertex or base instance draw calls.
    GLint mCachedBaseVertex;
    GLuint mCachedBaseInstance;
};
}  // namespace gl

#endif  // LIBANGLE_PROGRAMSTATEDATA_H_
