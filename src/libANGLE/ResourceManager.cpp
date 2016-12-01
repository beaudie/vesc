//
// Copyright (c) 2002-2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ResourceManager.cpp: Implements the the ResourceManager classes, which handle allocation and
// lifetime of GL objects.

#include "libANGLE/ResourceManager.h"

#include "libANGLE/Buffer.h"
#include "libANGLE/Fence.h"
#include "libANGLE/Path.h"
#include "libANGLE/Program.h"
#include "libANGLE/Renderbuffer.h"
#include "libANGLE/Sampler.h"
#include "libANGLE/Shader.h"
#include "libANGLE/Texture.h"
#include "libANGLE/renderer/GLImplFactory.h"

namespace gl
{

template <typename ResourceType, typename HandleAllocatorType>
ResourceManager<ResourceType, HandleAllocatorType>::ResourceManager() : mRefCount(1)
{
}

template <typename ResourceType, typename HandleAllocatorType>
void ResourceManager<ResourceType, HandleAllocatorType>::addRef()
{
    mRefCount++;
}

template <typename ResourceType, typename HandleAllocatorType>
void ResourceManager<ResourceType, HandleAllocatorType>::release()
{
    if (--mRefCount == 0)
    {
        delete this;
    }
}

template <typename ResourceType, typename HandleAllocatorType>
ResourceManager<ResourceType, HandleAllocatorType>::~ResourceManager()
{
}

template <typename ResourceType, typename HandleAllocatorType>
GLuint ResourceManager<ResourceType, HandleAllocatorType>::allocateEmptyObject()
{
    GLuint handle      = mHandleAllocator.allocate();
    mObjectMap[handle] = nullptr;
    return handle;
}

template <typename ResourceType, typename HandleAllocatorType>
template <typename CreationFunction>
GLuint ResourceManager<ResourceType, HandleAllocatorType>::insertObject(CreationFunction func)
{
    GLuint handle      = mHandleAllocator.allocate();
    mObjectMap[handle] = func(handle);
    return handle;
}

template <typename ResourceType, typename HandleAllocatorType>
template <typename CreationFunction>
ResourceType *ResourceManager<ResourceType, HandleAllocatorType>::checkObjectAllocation(
    GLuint handle,
    CreationFunction func)
{
    if (handle == 0)
    {
        return nullptr;
    }

    auto objectMapIter   = mObjectMap.find(handle);
    bool handleAllocated = (objectMapIter != mObjectMap.end());

    if (handleAllocated && objectMapIter->second != nullptr)
    {
        return objectMapIter->second;
    }

    ResourceType *object = func();
    if (handleAllocated)
    {
        objectMapIter->second = object;
    }
    else
    {
        mHandleAllocator.reserve(handle);
        mObjectMap[handle] = object;
    }

    return object;
}

template <typename ResourceType, typename HandleAllocatorType>
template <typename DeletionFunction>
void ResourceManager<ResourceType, HandleAllocatorType>::deleteObject(GLuint handle,
                                                                      DeletionFunction deleteFunc)
{
    auto objectIter = mObjectMap.find(handle);
    if (objectIter == mObjectMap.end())
    {
        return;
    }

    if (objectIter->second == nullptr || deleteFunc(objectIter->second))
    {
        mHandleAllocator.release(objectIter->first);
        mObjectMap.erase(objectIter);
    }
}

template <typename ResourceType, typename HandleAllocatorType>
ResourceType *ResourceManager<ResourceType, HandleAllocatorType>::getObject(GLuint handle) const
{
    auto iter = mObjectMap.find(handle);
    return iter != mObjectMap.end() ? iter->second : nullptr;
}

// Instantiations of ResourceManager
template class ResourceManager<Buffer, HandleAllocator>;
template class ResourceManager<Shader, HandleAllocator>;
template class ResourceManager<Program, HandleAllocator>;
template class ResourceManager<Texture, HandleAllocator>;
template class ResourceManager<Renderbuffer, HandleAllocator>;
template class ResourceManager<Sampler, HandleAllocator>;
template class ResourceManager<FenceSync, HandleAllocator>;
template class ResourceManager<Path, HandleRangeAllocator>;

GLuint BufferManager::createBuffer()
{
    return allocateEmptyObject();
}

void BufferManager::deleteBuffer(GLuint buffer)
{
    deleteObject(buffer, [](Buffer *buffer) {
        buffer->release();
        return true;
    });
}

Buffer *BufferManager::getBuffer(GLuint handle) const
{
    return getObject(handle);
}

Buffer *BufferManager::checkBufferAllocation(rx::GLImplFactory *factory, GLuint handle)
{
    return checkObjectAllocation(handle, [&]() {
        Buffer *buffer = new Buffer(factory, handle);
        buffer->addRef();
        return buffer;
    });
}

bool BufferManager::isBufferGenerated(GLuint buffer) const
{
    return buffer == 0 || mObjectMap.find(buffer) != mObjectMap.end();
}

BufferManager::~BufferManager()
{
    while (!mObjectMap.empty())
    {
        deleteBuffer(mObjectMap.begin()->first);
    }
}

GLuint ShaderManager::createShader(rx::GLImplFactory *factory,
                                   const Limitations &rendererLimitations,
                                   GLenum type)
{
    ASSERT(type == GL_VERTEX_SHADER || type == GL_FRAGMENT_SHADER || type == GL_COMPUTE_SHADER);
    return insertObject([&](GLuint handle) {
        return new Shader(this, factory, rendererLimitations, type, handle);
    });
}

void ShaderManager::deleteShader(GLuint shader)
{
    deleteObject(shader, [](Shader *shader) {
        if (shader->getRefCount() == 0)
        {
            delete shader;
            return true;
        }
        else
        {
            shader->flagForDeletion();
            return false;
        }
    });
}

Shader *ShaderManager::getShader(GLuint handle) const
{
    return getObject(handle);
}

ShaderManager::~ShaderManager()
{
    while (!mObjectMap.empty())
    {
        deleteShader(mObjectMap.begin()->first);
    }
}

GLuint ProgramManager::createProgram(rx::GLImplFactory *factory)
{
    return insertObject([&](GLuint handle) { return new Program(factory, this, handle); });
}

void ProgramManager::deleteProgram(GLuint program)
{
    deleteObject(program, [](Program *program) {
        if (program->getRefCount() == 0)
        {
            delete program;
            return true;
        }
        else
        {
            program->flagForDeletion();
            return false;
        }
    });
}

Program *ProgramManager::getProgram(GLuint handle) const
{
    return getObject(handle);
}

ProgramManager::~ProgramManager()
{
    while (!mObjectMap.empty())
    {
        deleteProgram(mObjectMap.begin()->first);
    }
}

GLuint TextureManager::createTexture()
{
    return allocateEmptyObject();
}

void TextureManager::deleteTexture(GLuint texture)
{
    deleteObject(texture, [](Texture *texture) {
        texture->release();
        return true;
    });
}

Texture *TextureManager::getTexture(GLuint handle) const
{
    ASSERT(getObject(0) == nullptr);
    return getObject(handle);
}

Texture *TextureManager::checkTextureAllocation(rx::GLImplFactory *factory,
                                                GLuint handle,
                                                GLenum type)
{
    return checkObjectAllocation(handle, [&]() {
        Texture *texture = new Texture(factory, handle, type);
        texture->addRef();
        return texture;
    });
}

bool TextureManager::isTextureGenerated(GLuint texture) const
{
    return texture == 0 || mObjectMap.find(texture) != mObjectMap.end();
}

TextureManager::~TextureManager()
{
    while (!mObjectMap.empty())
    {
        deleteTexture(mObjectMap.begin()->first);
    }
}

GLuint RenderbufferManager::createRenderbuffer()
{
    return allocateEmptyObject();
}

void RenderbufferManager::deleteRenderbuffer(GLuint renderbuffer)
{
    deleteObject(renderbuffer, [](Renderbuffer *renderbuffer) {
        renderbuffer->release();
        return true;
    });
}

Renderbuffer *RenderbufferManager::getRenderbuffer(GLuint handle)
{
    return getObject(handle);
}

Renderbuffer *RenderbufferManager::checkRenderbufferAllocation(rx::GLImplFactory *factory,
                                                               GLuint handle)
{
    return checkObjectAllocation(handle, [&]() {
        Renderbuffer *renderbuffer = new Renderbuffer(factory->createRenderbuffer(), handle);
        renderbuffer->addRef();
        return renderbuffer;
    });
}

bool RenderbufferManager::isRenderbufferGenerated(GLuint renderbuffer) const
{
    return renderbuffer == 0 || mObjectMap.find(renderbuffer) != mObjectMap.end();
}

RenderbufferManager::~RenderbufferManager()
{
    while (!mObjectMap.empty())
    {
        deleteRenderbuffer(mObjectMap.begin()->first);
    }
}

GLuint SamplerManager::createSampler()
{
    return allocateEmptyObject();
}

void SamplerManager::deleteSampler(GLuint sampler)
{
    deleteObject(sampler, [](Sampler *sampler) {
        sampler->release();
        return true;
    });
}

Sampler *SamplerManager::getSampler(GLuint handle)
{
    return getObject(handle);
}

Sampler *SamplerManager::checkSamplerAllocation(rx::GLImplFactory *factory, GLuint handle)
{
    return checkObjectAllocation(handle, [&]() {
        Sampler *sampler = new Sampler(factory, handle);
        sampler->addRef();
        return sampler;
    });
}

bool SamplerManager::isSampler(GLuint sampler)
{
    return mObjectMap.find(sampler) != mObjectMap.end();
}

SamplerManager::~SamplerManager()
{
    while (!mObjectMap.empty())
    {
        deleteSampler(mObjectMap.begin()->first);
    }
}

GLuint FenceSyncManager::createFenceSync(rx::GLImplFactory *factory)
{
    return insertObject([&](GLuint handle) {
        FenceSync *fenceSync = new FenceSync(factory->createFenceSync(), handle);
        fenceSync->addRef();
        return fenceSync;
    });
}

void FenceSyncManager::deleteFenceSync(GLuint fenceSync)
{
    deleteObject(fenceSync, [](FenceSync *fenceSync) {
        fenceSync->release();
        return true;
    });
}

FenceSync *FenceSyncManager::getFenceSync(GLuint handle)
{
    return getObject(handle);
}

FenceSyncManager::~FenceSyncManager()
{
    while (!mObjectMap.empty())
    {
        deleteFenceSync(mObjectMap.begin()->first);
    }
}

ErrorOrResult<GLuint> PathManager::createPaths(rx::GLImplFactory *factory, GLsizei range)
{
    // Allocate client side handles.
    const GLuint client = mHandleAllocator.allocateRange(static_cast<GLuint>(range));
    if (client == HandleRangeAllocator::kInvalidHandle)
        return Error(GL_OUT_OF_MEMORY, "Failed to allocate path handle range.");

    const auto &paths = factory->createPaths(range);
    if (paths.empty())
    {
        mHandleAllocator.releaseRange(client, range);
        return Error(GL_OUT_OF_MEMORY, "Failed to allocate path objects.");
    }

    auto hint = mObjectMap.begin();

    for (GLsizei i = 0; i < range; ++i)
    {
        const auto impl = paths[static_cast<unsigned>(i)];
        const auto id   = client + i;
        hint            = mObjectMap.insert(hint, std::make_pair(id, new Path(impl)));
    }
    return client;
}

void PathManager::deletePaths(GLuint first, GLsizei range)
{
    for (GLsizei i = 0; i < range; ++i)
    {
        const auto id = first + i;
        const auto it = mObjectMap.find(id);
        if (it == mObjectMap.end())
            continue;
        Path *p = it->second;
        delete p;
        mObjectMap.erase(it);
    }
    mHandleAllocator.releaseRange(first, static_cast<GLuint>(range));
}

Path *PathManager::getPath(GLuint handle) const
{
    return getObject(handle);
}

bool PathManager::hasPath(GLuint handle) const
{
    return mHandleAllocator.isUsed(handle);
}

PathManager::~PathManager()
{
    for (auto path : mObjectMap)
    {
        SafeDelete(path.second);
    }
}

GLuint FramebufferManager::createFramebuffer()
{
    return allocateEmptyObject();
}

void FramebufferManager::deleteFramebuffer(GLuint framebuffer)
{
    deleteObject(framebuffer, [](Framebuffer *framebuffer) {
        ASSERT(framebuffer->id() != 0);
        delete framebuffer;
        return true;
    });
}

Framebuffer *FramebufferManager::checkFramebufferAllocation(rx::GLImplFactory *factory, const Caps& caps, GLuint handle)
{
    return checkObjectAllocation(handle, [&]() {
        return new Framebuffer(caps, factory, handle);
    });
}

Framebuffer *FramebufferManager::getFramebuffer(GLuint handle) const
{
    return getObject(handle);
}

void FramebufferManager::setDefaultFramebuffer(Framebuffer* framebuffer)
{
    ASSERT(framebuffer == nullptr || framebuffer->id() == 0);
    mObjectMap[0] = framebuffer;
}

FramebufferManager::~FramebufferManager()
{
    for (auto framebuffer : mObjectMap)
    {
        // Default framebuffer are owned by their respective Surface
        if (framebuffer.second != nullptr && framebuffer.second->id() != 0)
        {
            SafeDelete(framebuffer.second);
        }
    }
}

}  // namespace gl
