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

template <typename HandleAllocatorType>
ResourceManagerBase<HandleAllocatorType>::ResourceManagerBase() : mRefCount(1)
{
}

template <typename HandleAllocatorType>
void ResourceManagerBase<HandleAllocatorType>::addRef()
{
    mRefCount++;
}

template <typename HandleAllocatorType>
void ResourceManagerBase<HandleAllocatorType>::release()
{
    if (--mRefCount == 0)
    {
        delete this;
    }
}

template <typename ResourceType, typename HandleAllocatorType>
ResourceManager<ResourceType, HandleAllocatorType>::~ResourceManager()
{
    while (!mObjectMap.empty())
    {
        deleteObject(mObjectMap.begin()->first);
    }
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
void ResourceManager<ResourceType, HandleAllocatorType>::deleteObject(GLuint handle)
{
    auto objectIter = mObjectMap.find(handle);
    if (objectIter == mObjectMap.end())
    {
        return;
    }

    if (objectIter->second != nullptr)
    {
        objectIter->second->release();
    }

    mHandleAllocator.release(objectIter->first);
    mObjectMap.erase(objectIter);
}

template <typename ResourceType, typename HandleAllocatorType>
ResourceType *ResourceManager<ResourceType, HandleAllocatorType>::getObject(GLuint handle) const
{
    auto iter = mObjectMap.find(handle);
    return iter != mObjectMap.end() ? iter->second : nullptr;
}

// Instantiations of ResourceManager
template class ResourceManagerBase<HandleAllocator>;
template class ResourceManagerBase<HandleRangeAllocator>;
template class ResourceManager<Buffer, HandleAllocator>;
template class ResourceManager<Texture, HandleAllocator>;
template class ResourceManager<Renderbuffer, HandleAllocator>;
template class ResourceManager<Sampler, HandleAllocator>;
template class ResourceManager<FenceSync, HandleAllocator>;

GLuint BufferManager::createBuffer()
{
    return allocateEmptyObject();
}

void BufferManager::deleteBuffer(GLuint buffer)
{
    deleteObject(buffer);
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

ShaderProgramManager::~ShaderProgramManager()
{
    while (!mShaders.empty())
    {
        deleteShader(mShaders.begin()->first);
    }
    while (!mPrograms.empty())
    {
        deleteProgram(mPrograms.begin()->first);
    }
}

GLuint ShaderProgramManager::createShader(rx::GLImplFactory *factory,
                                          const gl::Limitations &rendererLimitations,
                                          GLenum type)
{
    ASSERT(type == GL_VERTEX_SHADER || type == GL_FRAGMENT_SHADER || type == GL_COMPUTE_SHADER);
    GLuint handle    = mHandleAllocator.allocate();
    mShaders[handle] = new Shader(this, factory, rendererLimitations, type, handle);
    return handle;
}

void ShaderProgramManager::deleteShader(GLuint shader)
{
    deleteObject(&mShaders, shader);
}

Shader *ShaderProgramManager::getShader(GLuint handle) const
{
    return getObject(mShaders, handle);
}

GLuint ShaderProgramManager::createProgram(rx::GLImplFactory *factory)
{
    GLuint handle     = mHandleAllocator.allocate();
    mPrograms[handle] = new Program(factory, this, handle);
    return handle;
}

void ShaderProgramManager::deleteProgram(GLuint program)
{
    deleteObject(&mPrograms, program);
}

Program *ShaderProgramManager::getProgram(GLuint handle) const
{
    return getObject(mPrograms, handle);
}

template <typename ObjectType>
ObjectType *ShaderProgramManager::getObject(const ResourceMap<ObjectType> &objectMap, GLuint handle)
{
    auto iter = objectMap.find(handle);
    return iter != objectMap.end() ? iter->second : nullptr;
}

template <typename ObjectType>
void ShaderProgramManager::deleteObject(ResourceMap<ObjectType> *objectMap, GLuint id)
{
    auto iter = objectMap->find(id);
    if (iter == objectMap->end())
    {
        return;
    }

    auto object = iter->second;
    if (object->getRefCount() == 0)
    {
        mHandleAllocator.release(id);
        SafeDelete(object);
        objectMap->erase(iter);
    }
    else
    {
        object->flagForDeletion();
    }
}

GLuint TextureManager::createTexture()
{
    return allocateEmptyObject();
}

void TextureManager::deleteTexture(GLuint texture)
{
    deleteObject(texture);
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

GLuint RenderbufferManager::createRenderbuffer()
{
    return allocateEmptyObject();
}

void RenderbufferManager::deleteRenderbuffer(GLuint renderbuffer)
{
    deleteObject(renderbuffer);
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

GLuint SamplerManager::createSampler()
{
    return allocateEmptyObject();
}

void SamplerManager::deleteSampler(GLuint sampler)
{
    deleteObject(sampler);
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
    deleteObject(fenceSync);
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

    auto hint = mPaths.begin();

    for (GLsizei i = 0; i < range; ++i)
    {
        const auto impl = paths[static_cast<unsigned>(i)];
        const auto id   = client + i;
        hint            = mPaths.insert(hint, std::make_pair(id, new Path(impl)));
    }
    return client;
}

void PathManager::deletePaths(GLuint first, GLsizei range)
{
    for (GLsizei i = 0; i < range; ++i)
    {
        const auto id = first + i;
        const auto it = mPaths.find(id);
        if (it == mPaths.end())
            continue;
        Path *p = it->second;
        delete p;
        mPaths.erase(it);
    }
    mHandleAllocator.releaseRange(first, static_cast<GLuint>(range));
}

Path *PathManager::getPath(GLuint handle) const
{
    auto iter = mPaths.find(handle);
    return iter != mPaths.end() ? iter->second : nullptr;
}

bool PathManager::hasPath(GLuint handle) const
{
    return mHandleAllocator.isUsed(handle);
}

PathManager::~PathManager()
{
    for (auto path : mPaths)
    {
        SafeDelete(path.second);
    }
}

}  // namespace gl
