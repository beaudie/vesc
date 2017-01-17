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
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/GlslangWrapper.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "libANGLE/renderer/vulkan/TextureVk.h"
#include "libANGLE/Uniform.h"

namespace rx
{

ProgramVk::ProgramVk(ContextImpl *context, const gl::ProgramState &state)
    : ProgramImpl(state), mDescPool(VK_NULL_HANDLE)
{
    mContext = GetAs<ContextVk>(context);
}

ProgramVk::~ProgramVk()
{
    auto device = mContext->getDevice();

    // TODO(Jie): Wrap these raw Vulkan objects
    if (mDescSets.size() > 0)
        vkFreeDescriptorSets(device, mDescPool, mDescSets.size(), mDescSets.data());
    for (auto layout : mDescSetLayouts)
        vkDestroyDescriptorSetLayout(device, layout, nullptr);
    if (mDescPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(device, mDescPool, nullptr);
}

LinkResult ProgramVk::load(const ContextImpl *contextImpl,
                           gl::InfoLog &infoLog,
                           gl::BinaryInputStream *stream)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error ProgramVk::save(gl::BinaryOutputStream *stream)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

void ProgramVk::setBinaryRetrievableHint(bool retrievable)
{
    UNIMPLEMENTED();
}

LinkResult ProgramVk::link(ContextImpl *contextImpl, gl::InfoLog &infoLog)
{
    ContextVk *context             = GetAs<ContextVk>(contextImpl);
    RendererVk *renderer           = context->getRenderer();
    GlslangWrapper *glslangWrapper = renderer->getGlslangWrapper();

    const std::string &vertexSource   = mState.getAttachedVertexShader()->getTranslatedSource();
    const std::string &fragmentSource = mState.getAttachedFragmentShader()->getTranslatedSource();

    std::vector<uint32_t> vertexCode;
    std::vector<uint32_t> fragmentCode;
    bool linkSuccess = false;
    ANGLE_TRY_RESULT(
        glslangWrapper->linkProgram(vertexSource, fragmentSource, &vertexCode, &fragmentCode),
        linkSuccess);
    if (!linkSuccess)
    {
        return false;
    }

    vk::ShaderModule vertexModule(renderer->getDevice());
    vk::ShaderModule fragmentModule(renderer->getDevice());

    {
        VkShaderModuleCreateInfo vertexShaderInfo;
        vertexShaderInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        vertexShaderInfo.pNext    = nullptr;
        vertexShaderInfo.flags    = 0;
        vertexShaderInfo.codeSize = vertexCode.size() * sizeof(uint32_t);
        vertexShaderInfo.pCode    = vertexCode.data();
        ANGLE_TRY(static_cast<gl::Error>(vertexModule.init(vertexShaderInfo)));
    }

    {
        VkShaderModuleCreateInfo fragmentShaderInfo;
        fragmentShaderInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        fragmentShaderInfo.pNext    = nullptr;
        fragmentShaderInfo.flags    = 0;
        fragmentShaderInfo.codeSize = fragmentCode.size() * sizeof(uint32_t);
        fragmentShaderInfo.pCode    = fragmentCode.data();

        ANGLE_TRY(static_cast<gl::Error>(fragmentModule.init(fragmentShaderInfo)));
    }

    mLinkedVertexModule   = std::move(vertexModule);
    mLinkedFragmentModule = std::move(fragmentModule);

    // (Re)initialize the uniform value for all samplers.
    mLinkedSamplers.clear();
    for (unsigned int i = 0; i < mState.getUniforms().size(); i++)
    {
        if (mState.getUniforms()[i].isSampler())
        {
            mLinkedSamplers.push_back(LinkedSampler(i));
        }
    }
    mDescSetLayouts.clear();

    return true;
}

GLboolean ProgramVk::validate(const gl::Caps &caps, gl::InfoLog *infoLog)
{
    UNIMPLEMENTED();
    return GLboolean();
}

void ProgramVk::setUniform1fv(GLint location, GLsizei count, const GLfloat *v)
{
    UNIMPLEMENTED();
}

void ProgramVk::setUniform2fv(GLint location, GLsizei count, const GLfloat *v)
{
    UNIMPLEMENTED();
}

void ProgramVk::setUniform3fv(GLint location, GLsizei count, const GLfloat *v)
{
    UNIMPLEMENTED();
}

void ProgramVk::setUniform4fv(GLint location, GLsizei count, const GLfloat *v)
{
    UNIMPLEMENTED();
}

void ProgramVk::setUniform1iv(GLint location, GLsizei count, const GLint *v)
{
    auto index   = mState.getUniformLocations()[location].index;
    auto uniform = mState.getUniforms()[index];
    if (uniform.isSampler())
    {
        for (size_t i = 0; i < mLinkedSamplers.size(); i++)
        {
            if (mLinkedSamplers[i].index == index)
                mLinkedSamplers[i].elements.assign(&v[0], &v[count]);
        }
    }
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

bool ProgramVk::getUniformBlockSize(const std::string &blockName, size_t *sizeOut) const
{
    UNIMPLEMENTED();
    return bool();
}

bool ProgramVk::getUniformBlockMemberInfo(const std::string &memberUniformName,
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

gl::ErrorOrResult<vk::PipelineLayout *> ProgramVk::getPipelineLayout()
{
    auto device = mContext->getDevice();
    std::vector<std::vector<VkDescriptorSetLayoutBinding>> layoutBindings;

    for (unsigned int i = 0; i < mState.getUniforms().size(); i++)
    {
        auto uniform = mState.getUniforms()[i];
        if (uniform.isSampler())
        {
            uint32_t set     = getSamplerSet(uniform);
            uint32_t binding = getSamplerBinding(uniform);
            if (layoutBindings.size() <= set)
                layoutBindings.resize(set + 1);

            VkDescriptorSetLayoutBinding bindingDesc;

            bindingDesc.binding            = binding;
            bindingDesc.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            bindingDesc.descriptorCount    = uniform.elementCount();
            bindingDesc.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
            bindingDesc.pImmutableSamplers = nullptr;

            layoutBindings[set].push_back(bindingDesc);

            for (auto sampler : mLinkedSamplers)
            {
                if (sampler.index == i)
                {
                    sampler.set     = set;
                    sampler.binding = binding;
                }
            }
        }
    }
    if (mDescSetLayouts.empty())
    {
        for (uint32_t set = 0; set < layoutBindings.size(); set++)
        {
            auto bindings = layoutBindings[set];
            if (!bindings.empty())
            {
                VkDescriptorSetLayoutCreateInfo info;
                VkDescriptorSetLayout layout;

                info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                info.flags        = 0u;
                info.pNext        = nullptr;
                info.bindingCount = bindings.size();
                info.pBindings    = bindings.data();

                ANGLE_VK_TRY(vkCreateDescriptorSetLayout(device, &info, nullptr, &layout));
                mDescSetLayouts.push_back(layout);
            }
        }

        vk::PipelineLayout newLayout(device);

        VkPipelineLayoutCreateInfo createInfo;
        createInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        createInfo.pNext                  = nullptr;
        createInfo.flags                  = 0;
        createInfo.setLayoutCount         = mDescSetLayouts.size();
        createInfo.pSetLayouts            = mDescSetLayouts.data();
        createInfo.pushConstantRangeCount = 0;
        createInfo.pPushConstantRanges    = nullptr;

        ANGLE_TRY(static_cast<gl::Error>(newLayout.init(createInfo)));

        mPipelineLayout = std::move(newLayout);
    }

    return &mPipelineLayout;
}

gl::ErrorOrResult<std::vector<VkDescriptorSet> *> ProgramVk::getDescriptorSets(
    const gl::State &state)
{
    auto device = mContext->getDevice();
    if (mDescSets.size() > 0)
    {
        ANGLE_VK_TRY(vkFreeDescriptorSets(device, mDescPool, mDescSets.size(), mDescSets.data()));
    }
    mDescSets.clear();
    if (mDescSetLayouts.size() == 0)
        return nullptr;
    mDescSets.resize(mDescSetLayouts.size());

    if (mDescPool == VK_NULL_HANDLE)
    {
        VkDescriptorPoolSize typeCount;
        typeCount.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        typeCount.descriptorCount = mDescSetLayouts.size();

        VkDescriptorPoolCreateInfo poolInfo;
        poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.pNext         = nullptr;
        poolInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets       = 128;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes    = &typeCount;

        ANGLE_VK_TRY(vkCreateDescriptorPool(device, &poolInfo, nullptr, &mDescPool));
    }

    VkDescriptorSetAllocateInfo descSetAllocInfo;
    descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descSetAllocInfo.pNext = nullptr, descSetAllocInfo.descriptorPool = mDescPool;
    descSetAllocInfo.descriptorSetCount = mDescSetLayouts.size();
    descSetAllocInfo.pSetLayouts        = mDescSetLayouts.data();
    ANGLE_VK_TRY(vkAllocateDescriptorSets(device, &descSetAllocInfo, mDescSets.data()));

    for (auto sampler : mLinkedSamplers)
    {
        for (uint32_t element = 0; element < sampler.elements.size(); element++)
        {
            // TODO(Jie): Unit samplers may supersede texture samplers.

            // TODO(Jie): Map samper to textuer target
            ASSERT(mState.getUniforms()[sampler.index].type == GL_SAMPLER_2D);
            auto texture   = state.getSamplerTexture(sampler.elements[element], GL_TEXTURE_2D);
            auto textureVk = GetAs<TextureVk>(texture->getImplementation());
            VkDescriptorImageInfo *imageInfo;
            ANGLE_TRY_RESULT(textureVk->getDescriptorImageInfo(), imageInfo);

            VkWriteDescriptorSet write;
            write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet          = mDescSets[sampler.set];
            write.dstBinding      = sampler.binding;
            write.dstArrayElement = element;
            write.descriptorCount = 1;
            write.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write.pImageInfo      = imageInfo;
            vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
        }
    }
    return &mDescSets;
}

// TODO(Jie): Generate set and binding info in compiler.
uint32_t ProgramVk::getSamplerSet(const gl::LinkedUniform &uniform)
{
    return 0u;
}

uint32_t ProgramVk::getSamplerBinding(const gl::LinkedUniform &uniform)
{
    return 0u;
}

}  // namespace rx
