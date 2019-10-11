//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DynamicVK.h: Interface for link and run-time GL generation for Vulkan
//

#ifndef LIBANGLE_RENDERER_VULKAN_DYNAMICVK_H_
#define LIBANGLE_RENDERER_VULKAN_DYNAMICVK_H_

#include <vector>

#include <GLSLANG/ShaderVars.h>

#include "angle_gl.h"
#include "common/angleutils.h"
#include "libANGLE/angletypes.h"

namespace gl
{
class VaryingPacking;
}  // namespace gl

namespace rx
{

class DynamicVk : angle::NonCopyable
{
  public:
    DynamicVk();
    ~DynamicVk();

    void clear()
    {
        mGeometryShader.clear();
        mVaryingStructList.clear();
    }
    void generateGeometryShaderPreamble(const gl::VaryingPacking &varyingPacking,
                                        bool shaderTessellationAndGeometryPointSize);
    const std::string generateGeometryShader(const gl::PrimitiveMode &drawMode,
                                             bool enableLineRasterEmulation,
                                             size_t inputSize,
                                             gl::ProvokingVertexConvention provokingVertex);

  private:
    // order of field declaration, string of field declaration (type, name)
    using StructFieldMap = std::map<int, std::pair<std::string, std::string>>;
    struct StructType
    {
        StructType() : location(-1) {}
        std::string name;
        int location;
        sh::InterpolationType interpolation;
        StructFieldMap fields;
    };
    // struct declartion name, its reconstructed type
    using VaryingStructMap = std::unordered_map<std::string, StructType>;
    // reconstruct structs from VaryingPacking
    void generateStructListVk(const gl::VaryingPacking &varyingPacking);
    void generateVaryingDeclarationStructVk(std::ostringstream &glslStream, bool isOut) const;
    void generateVaryingDeclarationVaribleVk(std::ostringstream &glslStream,
                                             const gl::VaryingPacking &varyingPacking,
                                             bool isOut) const;
    static void generateStructreDeclarationVk(std::ostringstream &glslStream,
                                              const StructType structType,
                                              const std::string &postString);
    bool findPointSize(const gl::VaryingPacking &varyingPacking);

    std::string mGeometryShader;
    VaryingStructMap mVaryingStructList;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_DYNAMICVK_H_
