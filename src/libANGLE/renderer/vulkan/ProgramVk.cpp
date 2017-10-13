//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramVk.cpp:
//    Implements the class methods for ProgramVk.
//

#include "libANGLE/renderer/vulkan/ProgramVk.h"

#include "common/debug.h"
#include "libANGLE/Context.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/GlslangWrapper.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"

namespace rx
{

namespace
{

gl::Error InitDefaultUniformBlock(const gl::Context *context,
                                  VkDevice device,
                                  gl::Shader *shader,
                                  vk::Buffer *bufferOut,
                                  vk::DeviceMemory *bufferMemoryOut,
                                  sh::BlockLayoutMap *blockLayoutMapOut,
                                  size_t *requiredSizeOut)
{
    const auto &uniforms = shader->getUniforms(context);

    if (uniforms.empty())
    {
        *requiredSizeOut = 0;
        return gl::NoError();
    }

    sh::Std140BlockEncoder blockEncoder;
    sh::GetUniformBlockInfo(uniforms, "", &blockEncoder, false, blockLayoutMapOut);

    size_t blockSize = blockEncoder.getBlockSize();

    // TODO(jmadill): I think we still need a valid block for the pipeline even if zero sized.
    if (blockSize == 0)
    {
        *requiredSizeOut = 0;
        return gl::NoError();
    }

    VkBufferCreateInfo uniformBufferInfo;
    uniformBufferInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    uniformBufferInfo.pNext                 = nullptr;
    uniformBufferInfo.flags                 = 0;
    uniformBufferInfo.size                  = blockSize;
    uniformBufferInfo.usage                 = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    uniformBufferInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    uniformBufferInfo.queueFamilyIndexCount = 0;
    uniformBufferInfo.pQueueFamilyIndices   = nullptr;

    ANGLE_TRY(bufferOut->init(device, uniformBufferInfo));

    ANGLE_TRY(AllocateBufferMemory(GetImplAs<ContextVk>(context), blockSize, bufferOut,
                                   bufferMemoryOut, requiredSizeOut));

    return gl::NoError();
}

template <typename T>
void UpdateDefaultUniformBlock(GLsizei count,
                               const T *v,
                               const sh::BlockMemberInfo &layoutInfo,
                               angle::MemoryBuffer *uniformData)
{
    // Assume an offset of -1 means the block is unused.
    if (layoutInfo.offset == -1)
    {
        return;
    }

    if (layoutInfo.arrayStride == sizeof(T))
    {
        uint8_t *writePtr = uniformData->data() + layoutInfo.offset;
        memcpy(writePtr, v, sizeof(T) * count);
    }
    else
    {
        UNIMPLEMENTED();
    }
}

vk::Error SyncDefaultUniformBlock(VkDevice device,
                                  vk::DeviceMemory *bufferMemory,
                                  const angle::MemoryBuffer &bufferData)
{
    ASSERT(bufferMemory->valid() && !bufferData.empty());
    uint8_t *mapPointer = nullptr;
    ANGLE_TRY(bufferMemory->map(device, 0, bufferData.size(), 0, &mapPointer));
    memcpy(mapPointer, bufferData.data(), bufferData.size());
    bufferMemory->unmap(device);
}

}  // anonymous namespace

ProgramVk::ProgramVk(const gl::ProgramState &state)
    : ProgramImpl(state), mVertexUniformsDirty(false), mFragmentUniformsDirty(false)
{
}

ProgramVk::~ProgramVk()
{
}

void ProgramVk::destroy(const gl::Context *contextImpl)
{
    VkDevice device = GetImplAs<ContextVk>(contextImpl)->getDevice();

    mDefaultVertexUniformsMemory.destroy(device);
    mDefaultFragmentUniformsMemory.destroy(device);
    mDefaultVertexUniformsBuffer.destroy(device);
    mDefaultFragmentUniformsBuffer.destroy(device);

    for (auto &descriptorSetLayout : mDescriptorSetLayouts)
    {
        descriptorSetLayout.destroy(device);
    }

    mLinkedFragmentModule.destroy(device);
    mLinkedVertexModule.destroy(device);
    mPipelineLayout.destroy(device);
}

gl::LinkResult ProgramVk::load(const gl::Context *contextImpl,
                               gl::InfoLog &infoLog,
                               gl::BinaryInputStream *stream)
{
    UNIMPLEMENTED();
    return gl::InternalError();
}

void ProgramVk::save(const gl::Context *context, gl::BinaryOutputStream *stream)
{
    UNIMPLEMENTED();
}

void ProgramVk::setBinaryRetrievableHint(bool retrievable)
{
    UNIMPLEMENTED();
}

void ProgramVk::setSeparable(bool separable)
{
    UNIMPLEMENTED();
}

gl::LinkResult ProgramVk::link(const gl::Context *glContext,
                               const gl::VaryingPacking &packing,
                               gl::InfoLog &infoLog)
{
    ContextVk *context             = GetImplAs<ContextVk>(glContext);
    RendererVk *renderer           = context->getRenderer();
    GlslangWrapper *glslangWrapper = renderer->getGlslangWrapper();

    std::vector<uint32_t> vertexCode;
    std::vector<uint32_t> fragmentCode;
    bool linkSuccess = false;
    ANGLE_TRY_RESULT(glslangWrapper->linkProgram(glContext, mState, &vertexCode, &fragmentCode),
                     linkSuccess);
    if (!linkSuccess)
    {
        return false;
    }

    vk::ShaderModule vertexModule;
    vk::ShaderModule fragmentModule;
    VkDevice device = renderer->getDevice();

    {
        VkShaderModuleCreateInfo vertexShaderInfo;
        vertexShaderInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        vertexShaderInfo.pNext    = nullptr;
        vertexShaderInfo.flags    = 0;
        vertexShaderInfo.codeSize = vertexCode.size() * sizeof(uint32_t);
        vertexShaderInfo.pCode    = vertexCode.data();
        ANGLE_TRY(vertexModule.init(device, vertexShaderInfo));
    }

    {
        VkShaderModuleCreateInfo fragmentShaderInfo;
        fragmentShaderInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        fragmentShaderInfo.pNext    = nullptr;
        fragmentShaderInfo.flags    = 0;
        fragmentShaderInfo.codeSize = fragmentCode.size() * sizeof(uint32_t);
        fragmentShaderInfo.pCode    = fragmentCode.data();

        ANGLE_TRY(fragmentModule.init(device, fragmentShaderInfo));
    }

    mLinkedVertexModule.retain(device, std::move(vertexModule));
    mLinkedFragmentModule.retain(device, std::move(fragmentModule));

    // Process vertex and fragment uniforms into std140 packing.
    sh::BlockLayoutMap vertexLayoutMap;
    size_t requiredVertexUniformsBufferSize = 0;
    gl::Shader *vertexShader                = mState.getAttachedVertexShader();
    ANGLE_TRY(InitDefaultUniformBlock(glContext, device, vertexShader,
                                      &mDefaultVertexUniformsBuffer, &mDefaultVertexUniformsMemory,
                                      &vertexLayoutMap, &requiredVertexUniformsBufferSize));

    sh::BlockLayoutMap fragmentLayoutMap;
    size_t requiredFragmentUniformsBufferSize = 0;
    gl::Shader *fragmentShader                = mState.getAttachedFragmentShader();
    ANGLE_TRY(InitDefaultUniformBlock(
        glContext, device, fragmentShader, &mDefaultFragmentUniformsBuffer,
        &mDefaultFragmentUniformsMemory, &fragmentLayoutMap, &requiredFragmentUniformsBufferSize));

    // Init the default block layout info.
    const auto &locations = mState.getUniformLocations();
    const auto &uniforms  = mState.getUniforms();
    for (size_t locationIndex = 0; locationIndex < locations.size(); ++locationIndex)
    {
        DefaultBlockUniformInfo blockInfo;

        const auto &location = locations[locationIndex];
        if (location.used() && !location.ignored)
        {
            const auto &uniform           = uniforms[location.index];
            const std::string uniformName = uniform.name + ArrayIndexString(location.arrayIndices);

            auto vertexIt = vertexLayoutMap.find(uniformName);
            if (vertexIt != vertexLayoutMap.end())
            {
                blockInfo.vertexInfo = vertexIt->second;
            }

            auto fragmentIt = fragmentLayoutMap.find(uniformName);
            if (fragmentIt != fragmentLayoutMap.end())
            {
                blockInfo.fragmentInfo = fragmentIt->second;
            }
        }

        mDefaultUniformLayout.push_back(blockInfo);
    }

    if (requiredVertexUniformsBufferSize > 0)
    {
        if (!mVertexUniformData.resize(requiredVertexUniformsBufferSize))
        {
            return gl::OutOfMemory() << "Memory allocation failure.";
        }
        mVertexUniformData.fill(0);
        mVertexUniformsDirty = true;
    }

    if (requiredFragmentUniformsBufferSize > 0)
    {
        if (!mFragmentUniformData.resize(requiredFragmentUniformsBufferSize))
        {
            return gl::OutOfMemory() << "Memory allocation failure.";
        }
        mFragmentUniformData.fill(0);
        mFragmentUniformsDirty = true;
    }

    return true;
}

GLboolean ProgramVk::validate(const gl::Caps &caps, gl::InfoLog *infoLog)
{
    UNIMPLEMENTED();
    return GLboolean();
}

template <typename T>
void ProgramVk::setUniformImpl(GLint location, GLsizei count, const T *v, GLenum entryPointType)
{
    const DefaultBlockUniformInfo &layoutInfo = mDefaultUniformLayout[location];
    const gl::VariableLocation &locationInfo  = mState.getUniformLocations()[location];
    const gl::LinkedUniform &linkedUniform    = mState.getUniforms()[locationInfo.index];

    if (linkedUniform.type == entryPointType)
    {
        UpdateDefaultUniformBlock(count, v, layoutInfo.vertexInfo, &mVertexUniformData);
        UpdateDefaultUniformBlock(count, v, layoutInfo.fragmentInfo, &mFragmentUniformData);
    }
    else
    {
        ASSERT(linkedUniform.type == gl::VariableBoolVectorType(entryPointType));
        UNIMPLEMENTED();
    }
}

void ProgramVk::setUniform1fv(GLint location, GLsizei count, const GLfloat *v)
{
    setUniformImpl(location, count, v, GL_FLOAT);
}

void ProgramVk::setUniform2fv(GLint location, GLsizei count, const GLfloat *v)
{
    setUniformImpl(location, count, v, GL_FLOAT_VEC2);
}

void ProgramVk::setUniform3fv(GLint location, GLsizei count, const GLfloat *v)
{
    setUniformImpl(location, count, v, GL_FLOAT_VEC3);
}

void ProgramVk::setUniform4fv(GLint location, GLsizei count, const GLfloat *v)
{
    setUniformImpl(location, count, v, GL_FLOAT_VEC4);
}

void ProgramVk::setUniform1iv(GLint location, GLsizei count, const GLint *v)
{
    UNIMPLEMENTED();
}

void ProgramVk::setUniform2iv(GLint location, GLsizei count, const GLint *v)
{
    UNIMPLEMENTED();
}

void ProgramVk::setUniform3iv(GLint location, GLsizei count, const GLint *v)
{
    UNIMPLEMENTED();
}

void ProgramVk::setUniform4iv(GLint location, GLsizei count, const GLint *v)
{
    UNIMPLEMENTED();
}

void ProgramVk::setUniform1uiv(GLint location, GLsizei count, const GLuint *v)
{
    UNIMPLEMENTED();
}

void ProgramVk::setUniform2uiv(GLint location, GLsizei count, const GLuint *v)
{
    UNIMPLEMENTED();
}

void ProgramVk::setUniform3uiv(GLint location, GLsizei count, const GLuint *v)
{
    UNIMPLEMENTED();
}

void ProgramVk::setUniform4uiv(GLint location, GLsizei count, const GLuint *v)
{
    UNIMPLEMENTED();
}

void ProgramVk::setUniformMatrix2fv(GLint location,
                                    GLsizei count,
                                    GLboolean transpose,
                                    const GLfloat *value)
{
    UNIMPLEMENTED();
}

void ProgramVk::setUniformMatrix3fv(GLint location,
                                    GLsizei count,
                                    GLboolean transpose,
                                    const GLfloat *value)
{
    UNIMPLEMENTED();
}

void ProgramVk::setUniformMatrix4fv(GLint location,
                                    GLsizei count,
                                    GLboolean transpose,
                                    const GLfloat *value)
{
    UNIMPLEMENTED();
}

void ProgramVk::setUniformMatrix2x3fv(GLint location,
                                      GLsizei count,
                                      GLboolean transpose,
                                      const GLfloat *value)
{
    UNIMPLEMENTED();
}

void ProgramVk::setUniformMatrix3x2fv(GLint location,
                                      GLsizei count,
                                      GLboolean transpose,
                                      const GLfloat *value)
{
    UNIMPLEMENTED();
}

void ProgramVk::setUniformMatrix2x4fv(GLint location,
                                      GLsizei count,
                                      GLboolean transpose,
                                      const GLfloat *value)
{
    UNIMPLEMENTED();
}

void ProgramVk::setUniformMatrix4x2fv(GLint location,
                                      GLsizei count,
                                      GLboolean transpose,
                                      const GLfloat *value)
{
    UNIMPLEMENTED();
}

void ProgramVk::setUniformMatrix3x4fv(GLint location,
                                      GLsizei count,
                                      GLboolean transpose,
                                      const GLfloat *value)
{
    UNIMPLEMENTED();
}

void ProgramVk::setUniformMatrix4x3fv(GLint location,
                                      GLsizei count,
                                      GLboolean transpose,
                                      const GLfloat *value)
{
    UNIMPLEMENTED();
}

void ProgramVk::setUniformBlockBinding(GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
    UNIMPLEMENTED();
}

bool ProgramVk::getUniformBlockSize(const std::string &blockName,
                                    const std::string &blockMappedName,
                                    size_t *sizeOut) const
{
    UNIMPLEMENTED();
    return bool();
}

bool ProgramVk::getUniformBlockMemberInfo(const std::string &memberUniformName,
                                          const std::string &memberUniformMappedName,
                                          sh::BlockMemberInfo *memberInfoOut) const
{
    UNIMPLEMENTED();
    return bool();
}

void ProgramVk::setPathFragmentInputGen(const std::string &inputName,
                                        GLenum genMode,
                                        GLint components,
                                        const GLfloat *coeffs)
{
    UNIMPLEMENTED();
}

const vk::ShaderModule &ProgramVk::getLinkedVertexModule() const
{
    ASSERT(mLinkedVertexModule.getHandle() != VK_NULL_HANDLE);
    return mLinkedVertexModule;
}

const vk::ShaderModule &ProgramVk::getLinkedFragmentModule() const
{
    ASSERT(mLinkedFragmentModule.getHandle() != VK_NULL_HANDLE);
    return mLinkedFragmentModule;
}

gl::ErrorOrResult<vk::PipelineLayout *> ProgramVk::getPipelineLayout(VkDevice device)
{
    if (mPipelineLayout.valid())
    {
        return &mPipelineLayout;
    }

    // Create two descriptor set layouts: one for default uniform info, and one for textures.
    VkDescriptorSetLayoutBinding uniformBindings[2];
    uniformBindings[0].binding            = 0;
    uniformBindings[0].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformBindings[0].descriptorCount    = 1;
    uniformBindings[0].stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
    uniformBindings[0].pImmutableSamplers = nullptr;

    uniformBindings[1].binding            = 1;
    uniformBindings[1].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformBindings[1].descriptorCount    = 1;
    uniformBindings[1].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
    uniformBindings[1].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo uniformInfo;
    uniformInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    uniformInfo.pNext        = nullptr;
    uniformInfo.flags        = 0;
    uniformInfo.bindingCount = 2;
    uniformInfo.pBindings    = uniformBindings;

    vk::DescriptorSetLayout uniformLayout;
    ANGLE_TRY(uniformLayout.init(device, uniformInfo));
    mDescriptorSetLayouts.push_back(std::move(uniformLayout));

    VkDescriptorSetLayoutBinding textureBindings[2];
    textureBindings[0].binding            = 0;
    textureBindings[0].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureBindings[0].descriptorCount    = 16;
    textureBindings[0].stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
    textureBindings[0].pImmutableSamplers = nullptr;

    textureBindings[1].binding            = 17;
    textureBindings[1].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureBindings[1].descriptorCount    = 16;
    textureBindings[1].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
    textureBindings[1].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo textureInfo;
    textureInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    textureInfo.pNext        = nullptr;
    textureInfo.flags        = 0;
    textureInfo.bindingCount = 2;
    textureInfo.pBindings    = textureBindings;

    vk::DescriptorSetLayout textureLayout;
    ANGLE_TRY(textureLayout.init(device, textureInfo));
    mDescriptorSetLayouts.push_back(std::move(textureLayout));

    VkPipelineLayoutCreateInfo createInfo;
    createInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.pNext                  = nullptr;
    createInfo.flags                  = 0;
    createInfo.setLayoutCount         = static_cast<uint32_t>(mDescriptorSetLayouts.size());
    createInfo.pSetLayouts            = mDescriptorSetLayouts[0].ptr();
    createInfo.pushConstantRangeCount = 0;
    createInfo.pPushConstantRanges    = nullptr;

    vk::PipelineLayout pipelineLayout;
    ANGLE_TRY(pipelineLayout.init(device, createInfo));
    mPipelineLayout.retain(device, std::move(pipelineLayout));

    return &mPipelineLayout;
}

void ProgramVk::getUniformfv(const gl::Context *context, GLint location, GLfloat *params) const
{
    UNIMPLEMENTED();
}

void ProgramVk::getUniformiv(const gl::Context *context, GLint location, GLint *params) const
{
    UNIMPLEMENTED();
}

void ProgramVk::getUniformuiv(const gl::Context *context, GLint location, GLuint *params) const
{
    UNIMPLEMENTED();
}

vk::Error ProgramVk::updateUniforms(ContextVk *contextVk)
{
    VkDevice device = contextVk->getDevice();

    // Update buffer memory by immediate mapping. This immediate update only works once.
    // TODO(jmadill): Handle inserting updates into the command stream, or use dynamic buffers.
    if (mVertexUniformsDirty)
    {
        ANGLE_TRY(
            SyncDefaultUniformBlock(device, &mDefaultVertexUniformsMemory, mVertexUniformData));
        mVertexUniformsDirty = false;
    }

    if (mFragmentUniformsDirty)
    {
        ANGLE_TRY(
            SyncDefaultUniformBlock(device, &mDefaultFragmentUniformsMemory, mFragmentUniformData));
        mFragmentUniformsDirty = false;
    }
}

}  // namespace rx
