//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramD3D.h: Defines the rx::ProgramD3D class which implements rx::ProgramImpl.

#ifndef LIBGLESV2_RENDERER_PROGRAMD3D_H_
#define LIBGLESV2_RENDERER_PROGRAMD3D_H_

#include "libGLESv2/renderer/ProgramImpl.h"

#include <string>
#include <vector>

namespace gl
{
struct LinkedUniform;
struct VariableLocation;
struct VertexFormat;
}

namespace rx
{

class UniformStorage;

class ProgramD3D : public ProgramImpl
{
  public:
    ProgramD3D(rx::Renderer *renderer);
    virtual ~ProgramD3D();

    static ProgramD3D *makeProgramD3D(ProgramImpl *impl);
    static const ProgramD3D *makeProgramD3D(const ProgramImpl *impl);

    const std::vector<rx::PixelShaderOutputVariable> &getPixelShaderKey() { return mPixelShaderKey; }
    int getShaderVersion() const { return mShaderVersion; }
    GLenum getTransformFeedbackBufferMode() const { return mTransformFeedbackBufferMode; }
    std::vector<gl::LinkedVarying> &getTransformFeedbackLinkedVaryings() { return mTransformFeedbackLinkedVaryings; }
    sh::Attribute *getShaderAttributes() { return mShaderAttributes; }
    const std::vector<gl::LinkedUniform*> &getUniforms() const { return mUniforms; }
    const std::vector<gl::VariableLocation> &getUniformIndices() const { return mUniformIndex; }
    const std::vector<gl::UniformBlock*> &getUniformBlocks() const { return mUniformBlocks; }
    gl::LinkedUniform *getUniformByLocation(GLint location) const;
    GLuint getUniformBlockIndex(std::string name) const;

    GLint getSamplerMapping(gl::SamplerType type, unsigned int samplerIndex, const gl::Caps &caps) const;
    GLenum getSamplerTextureType(gl::SamplerType type, unsigned int samplerIndex) const;
    GLint getUsedSamplerRange(gl::SamplerType type) const;
    void updateSamplerMapping();
    bool validateSamplers(gl::InfoLog *infoLog, const gl::Caps &caps);

    bool usesPointSize() const { return mUsesPointSize; }
    bool usesPointSpriteEmulation() const;
    bool usesGeometryShader() const;

    GLenum getBinaryFormat() { return GL_PROGRAM_BINARY_ANGLE; }
    bool load(gl::InfoLog &infoLog, gl::BinaryInputStream *stream);
    bool save(gl::BinaryOutputStream *stream);

    ShaderExecutable *getPixelExecutableForFramebuffer(const gl::Framebuffer *fbo);
    ShaderExecutable *getPixelExecutableForOutputLayout(const std::vector<GLenum> &outputLayout);
    ShaderExecutable *getVertexExecutableForInputLayout(const gl::VertexFormat inputLayout[gl::MAX_VERTEX_ATTRIBS]);
    ShaderExecutable *getGeometryExecutable() const { return mGeometryExecutable; }

    bool compileProgramExecutables(gl::InfoLog &infoLog, gl::Shader *fragmentShader, gl::Shader *vertexShader,
                                   int registers);

    bool link(gl::InfoLog &infoLog, gl::Shader *fragmentShader, gl::Shader *vertexShader,
              const std::vector<std::string> &transformFeedbackVaryings, GLenum transformFeedbackBufferMode,
              int *registers, std::vector<gl::LinkedVarying> *linkedVaryings,
              std::map<int, gl::VariableLocation> *outputVariables, const gl::Caps &caps);

    void getInputLayoutSignature(const gl::VertexFormat inputLayout[], GLenum signature[]) const;

    void initializeUniformStorage(const std::vector<gl::LinkedUniform*> &uniforms);
    gl::Error applyUniforms(const std::vector<gl::LinkedUniform*> &uniforms);
    gl::Error applyUniformBuffers(const std::vector<gl::Buffer*> boundBuffers, const gl::Caps &caps);
    bool assignUniformBlockRegister(gl::InfoLog &infoLog, gl::UniformBlock *uniformBlock, GLenum shader,
                                    unsigned int registerIndex, const gl::Caps &caps);
    void dirtyAllUniforms();

    void setUniform1fv(GLint location, GLsizei count, const GLfloat *v);
    void setUniform2fv(GLint location, GLsizei count, const GLfloat *v);
    void setUniform3fv(GLint location, GLsizei count, const GLfloat *v);
    void setUniform4fv(GLint location, GLsizei count, const GLfloat *v);
    void setUniform1iv(GLint location, GLsizei count, const GLint *v);
    void setUniform2iv(GLint location, GLsizei count, const GLint *v);
    void setUniform3iv(GLint location, GLsizei count, const GLint *v);
    void setUniform4iv(GLint location, GLsizei count, const GLint *v);
    void setUniform1uiv(GLint location, GLsizei count, const GLuint *v);
    void setUniform2uiv(GLint location, GLsizei count, const GLuint *v);
    void setUniform3uiv(GLint location, GLsizei count, const GLuint *v);
    void setUniform4uiv(GLint location, GLsizei count, const GLuint *v);
    void setUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void setUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void setUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void setUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void setUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void setUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void setUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void setUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void setUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

    void getUniformfv(GLint location, GLfloat *params);
    void getUniformiv(GLint location, GLint *params);
    void getUniformuiv(GLint location, GLuint *params);

    const UniformStorage &getVertexUniformStorage() const { return *mVertexUniformStorage; }
    const UniformStorage &getFragmentUniformStorage() const { return *mFragmentUniformStorage; }

    bool linkUniforms(gl::InfoLog &infoLog, const gl::Shader &vertexShader, const gl::Shader &fragmentShader,
                      const gl::Caps &caps);
    bool linkUniformBlocks(gl::InfoLog &infoLog, const gl::Shader &vertexShader, const gl::Shader &fragmentShader,
                           const gl::Caps &caps);

    gl::LinkedUniform *getUniformByName(const std::string &name) const;

    void reset();

  private:
    DISALLOW_COPY_AND_ASSIGN(ProgramD3D);

    class VertexExecutable
    {
      public:
        VertexExecutable(const gl::VertexFormat inputLayout[gl::MAX_VERTEX_ATTRIBS],
                         const GLenum signature[gl::MAX_VERTEX_ATTRIBS],
                         rx::ShaderExecutable *shaderExecutable);
        ~VertexExecutable();

        bool matchesSignature(const GLenum convertedLayout[gl::MAX_VERTEX_ATTRIBS]) const;

        const gl::VertexFormat *inputs() const { return mInputs; }
        const GLenum *signature() const { return mSignature; }
        rx::ShaderExecutable *shaderExecutable() const { return mShaderExecutable; }

      private:
        gl::VertexFormat mInputs[gl::MAX_VERTEX_ATTRIBS];
        GLenum mSignature[gl::MAX_VERTEX_ATTRIBS];
        rx::ShaderExecutable *mShaderExecutable;
    };

    class PixelExecutable
    {
      public:
        PixelExecutable(const std::vector<GLenum> &outputSignature, rx::ShaderExecutable *shaderExecutable);
        ~PixelExecutable();

        bool matchesSignature(const std::vector<GLenum> &signature) const { return mOutputSignature == signature; }

        const std::vector<GLenum> &outputSignature() const { return mOutputSignature; }
        rx::ShaderExecutable *shaderExecutable() const { return mShaderExecutable; }

      private:
        std::vector<GLenum> mOutputSignature;
        rx::ShaderExecutable *mShaderExecutable;
    };
    
    struct Sampler
    {
        Sampler();

        bool active;
        GLint logicalTextureUnit;
        GLenum textureType;
    };

    void defineUniformBase(GLenum shader, const sh::Uniform &uniform, unsigned int uniformRegister);
    void defineUniform(GLenum shader, const sh::ShaderVariable &uniform, const std::string &fullName,
                       sh::HLSLBlockEncoder *encoder);
    bool indexSamplerUniform(const gl::LinkedUniform &uniform, gl::InfoLog &infoLog, const gl::Caps &caps);
    bool indexUniforms(gl::InfoLog &infoLog, const gl::Caps &caps);
    static bool assignSamplers(unsigned int startSamplerIndex, GLenum samplerType, unsigned int samplerCount,
                               std::vector<Sampler> &outSamplers, GLuint *outUsedRange);
    bool areMatchingInterfaceBlocks(gl::InfoLog &infoLog, const sh::InterfaceBlock &vertexInterfaceBlock,
                                    const sh::InterfaceBlock &fragmentInterfaceBlock);

    template <typename T>
    void setUniform(GLint location, GLsizei count, const T* v, GLenum targetUniformType);

    template <int cols, int rows>
    void setUniformMatrixfv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value, GLenum targetUniformType);

    template <typename T>
    void getUniformv(GLint location, T *params, GLenum uniformType);

    template <typename VarT>
    void defineUniformBlockMembers(const std::vector<VarT> &fields, const std::string &prefix, int blockIndex,
                                   sh::BlockLayoutEncoder *encoder, std::vector<unsigned int> *blockUniformIndexes,
                                   bool inRowMajorLayout);
    bool defineUniformBlock(gl::InfoLog &infoLog, const gl::Shader &shader, const sh::InterfaceBlock &interfaceBlock, const gl::Caps &caps);

    Renderer *mRenderer;
    DynamicHLSL *mDynamicHLSL;

    std::vector<VertexExecutable *> mVertexExecutables;
    std::vector<PixelExecutable *> mPixelExecutables;
    rx::ShaderExecutable *mGeometryExecutable;

    std::string mVertexHLSL;
    rx::D3DWorkaroundType mVertexWorkarounds;

    std::string mPixelHLSL;
    rx::D3DWorkaroundType mPixelWorkarounds;
    bool mUsesFragDepth;
    std::vector<rx::PixelShaderOutputVariable> mPixelShaderKey;

    bool mUsesPointSize;

    UniformStorage *mVertexUniformStorage;
    UniformStorage *mFragmentUniformStorage;

    GLenum mTransformFeedbackBufferMode;
    std::vector<gl::LinkedVarying> mTransformFeedbackLinkedVaryings;

    sh::Attribute mShaderAttributes[gl::MAX_VERTEX_ATTRIBS];

    std::vector<gl::LinkedUniform*> mUniforms;
    std::vector<gl::VariableLocation> mUniformIndex;
    std::vector<gl::UniformBlock*> mUniformBlocks;

    std::vector<Sampler> mSamplersPS;
    std::vector<Sampler> mSamplersVS;
    GLuint mUsedVertexSamplerRange;
    GLuint mUsedPixelSamplerRange;
    bool mDirtySamplerMapping;

    int mShaderVersion;
};

}

#endif // LIBGLESV2_RENDERER_PROGRAMD3D_H_
