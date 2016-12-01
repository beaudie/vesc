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

namespace
{

template <typename ResourceType>
GLuint AllocateEmptyObject(HandleAllocator *handleAllocator, ResourceMap<ResourceType> *objectMap)
{
    GLuint handle        = handleAllocator->allocate();
    (*objectMap)[handle] = nullptr;
    return handle;
}

template <typename ResourceType, typename CreationFunction>
GLuint InsertObject(HandleAllocator *handleAllocator,
                    ResourceMap<ResourceType> *objectMap,
                    const CreationFunction &func)
{
    GLuint handle        = handleAllocator->allocate();
    (*objectMap)[handle] = func(handle);
    return handle;
}

template <typename ResourceType, typename CreationFunction>
ResourceType *CheckObjectAllocation(HandleAllocator *handleAllocator,
                                    ResourceMap<ResourceType> *objectMap,
                                    GLuint handle,
                                    const CreationFunction &func)
{
    ResourceType *object = nullptr;
    if (handle != 0)
    {
        auto objectMapIter   = objectMap->find(handle);
        bool handleAllocated = (objectMapIter != objectMap->end());
        if (handleAllocated && objectMapIter->second != nullptr)
        {
            object = objectMapIter->second;
        }
        else
        {
            object = func(handle);
            if (handleAllocated)
            {
                objectMapIter->second = object;
            }
            else
            {
                handleAllocator->reserve(handle);
                (*objectMap)[handle] = object;
            }
        }
    }
    return object;
}

template <typename ResourceType, typename DeletionFunction>
void DeleteObject(HandleAllocator *handleAllocator,
                  ResourceMap<ResourceType> *objectMap,
                  GLuint handle,
                  const DeletionFunction &deletionFunc)
{
    auto objectIter = objectMap->find(handle);
    if (objectIter == objectMap->end())
    {
        return;
    }

    if (objectIter->second != nullptr)
    {
        deletionFunc(objectIter->second);
    }

    handleAllocator->release(objectIter->first);
    objectMap->erase(objectIter);
}

template <typename ResourceType>
void DeleteObject(HandleAllocator *handleAllocator,
                  ResourceMap<ResourceType> *objectMap,
                  GLuint handle)
{
    return DeleteObject(handleAllocator, objectMap, handle,
                        [](ResourceType *object) { object->release(); });
}

template <typename ResourceType>
ResourceType *GetObject(const ResourceMap<ResourceType> &objectMap, GLuint handle)
{
    auto iter = objectMap.find(handle);
    return iter != objectMap.end() ? iter->second : nullptr;
}

}  // anonymous namespace

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
TypedResourceManager<ResourceType, HandleAllocatorType>::~TypedResourceManager()
{
    while (!mObjectMap.empty())
    {
        DeleteObject(&this->mHandleAllocator, &mObjectMap, mObjectMap.begin()->first);
    }
}

// Instantiations of ResourceManager
template class ResourceManagerBase<HandleAllocator>;
template class ResourceManagerBase<HandleRangeAllocator>;
template class TypedResourceManager<Buffer, HandleAllocator>;
template class TypedResourceManager<Texture, HandleAllocator>;
template class TypedResourceManager<Renderbuffer, HandleAllocator>;
template class TypedResourceManager<Sampler, HandleAllocator>;
template class TypedResourceManager<FenceSync, HandleAllocator>;

GLuint BufferManager::createBuffer()
{
    return AllocateEmptyObject(&mHandleAllocator, &mObjectMap);
}

void BufferManager::deleteBuffer(GLuint buffer)
{
    DeleteObject(&mHandleAllocator, &mObjectMap, buffer);
}

Buffer *BufferManager::getBuffer(GLuint handle) const
{
    return GetObject(mObjectMap, handle);
}

Buffer *BufferManager::checkBufferAllocation(rx::GLImplFactory *factory, GLuint handle)
{
    struct bufferAllocator
    {
        bufferAllocator(rx::GLImplFactory *factory) : factory(factory) {}

        rx::GLImplFactory *factory;

        Buffer *operator()(GLuint handle) const
        {
            Buffer *buffer = new Buffer(factory, handle);
            buffer->addRef();
            return buffer;
        }
    };

    return CheckObjectAllocation(&mHandleAllocator, &mObjectMap, handle, bufferAllocator(factory));
}

bool BufferManager::isBufferGenerated(GLuint buffer) const
{
    return buffer == 0 || mObjectMap.find(buffer) != mObjectMap.end();
}

ShaderProgramManager::~ShaderProgramManager()
{
    while (!mPrograms.empty())
    {
        deleteProgram(mPrograms.begin()->first);
    }
    while (!mShaders.empty())
    {
        deleteShader(mShaders.begin()->first);
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
    return GetObject(mShaders, handle);
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
    return GetObject(mPrograms, handle);
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
    return AllocateEmptyObject(&mHandleAllocator, &mObjectMap);
}

void TextureManager::deleteTexture(GLuint texture)
{
    DeleteObject(&mHandleAllocator, &mObjectMap, texture);
}

Texture *TextureManager::getTexture(GLuint handle) const
{
    ASSERT(GetObject(mObjectMap, 0) == nullptr);
    return GetObject(mObjectMap, handle);
}

Texture *TextureManager::checkTextureAllocation(rx::GLImplFactory *factory,
                                                GLuint handle,
                                                GLenum type)
{
    struct textureAllocator
    {
        textureAllocator(rx::GLImplFactory *factory, GLenum type) : factory(factory), type(type) {}

        rx::GLImplFactory *factory;
        GLenum type;

        Texture *operator()(GLuint handle) const
        {
            Texture *texture = new Texture(factory, handle, type);
            texture->addRef();
            return texture;
        }
    };

    return CheckObjectAllocation(&mHandleAllocator, &mObjectMap, handle,
                                 textureAllocator(factory, type));
}

bool TextureManager::isTextureGenerated(GLuint texture) const
{
    return texture == 0 || mObjectMap.find(texture) != mObjectMap.end();
}

GLuint RenderbufferManager::createRenderbuffer()
{
    return AllocateEmptyObject(&mHandleAllocator, &mObjectMap);
}

void RenderbufferManager::deleteRenderbuffer(GLuint renderbuffer)
{
    DeleteObject(&mHandleAllocator, &mObjectMap, renderbuffer);
}

Renderbuffer *RenderbufferManager::getRenderbuffer(GLuint handle)
{
    return GetObject(mObjectMap, handle);
}

Renderbuffer *RenderbufferManager::checkRenderbufferAllocation(rx::GLImplFactory *factory,
                                                               GLuint handle)
{
    struct renderbufferAllocator
    {
        renderbufferAllocator(rx::GLImplFactory *factory) : factory(factory) {}

        rx::GLImplFactory *factory;

        Renderbuffer *operator()(GLuint handle) const
        {
            Renderbuffer *renderbuffer = new Renderbuffer(factory->createRenderbuffer(), handle);
            renderbuffer->addRef();
            return renderbuffer;
        }
    };

    return CheckObjectAllocation(&mHandleAllocator, &mObjectMap, handle,
                                 renderbufferAllocator(factory));
}

bool RenderbufferManager::isRenderbufferGenerated(GLuint renderbuffer) const
{
    return renderbuffer == 0 || mObjectMap.find(renderbuffer) != mObjectMap.end();
}

GLuint SamplerManager::createSampler()
{
    return AllocateEmptyObject(&mHandleAllocator, &mObjectMap);
}

void SamplerManager::deleteSampler(GLuint sampler)
{
    DeleteObject(&mHandleAllocator, &mObjectMap, sampler);
}

Sampler *SamplerManager::getSampler(GLuint handle)
{
    return GetObject(mObjectMap, handle);
}

Sampler *SamplerManager::checkSamplerAllocation(rx::GLImplFactory *factory, GLuint handle)
{
    struct samplerAllocator
    {
        samplerAllocator(rx::GLImplFactory *factory) : factory(factory) {}

        rx::GLImplFactory *factory;

        Sampler *operator()(GLuint handle) const
        {
            Sampler *sampler = new Sampler(factory, handle);
            sampler->addRef();
            return sampler;
        }
    };

    return CheckObjectAllocation(&mHandleAllocator, &mObjectMap, handle, samplerAllocator(factory));
}

bool SamplerManager::isSampler(GLuint sampler)
{
    return mObjectMap.find(sampler) != mObjectMap.end();
}

GLuint FenceSyncManager::createFenceSync(rx::GLImplFactory *factory)
{
    struct fenceSyncAllocator
    {
        fenceSyncAllocator(rx::GLImplFactory *factory) : factory(factory) {}

        rx::GLImplFactory *factory;

        FenceSync *operator()(GLuint handle) const
        {
            FenceSync *fenceSync = new FenceSync(factory->createFenceSync(), handle);
            fenceSync->addRef();
            return fenceSync;
        }
    };

    return InsertObject(&mHandleAllocator, &mObjectMap, fenceSyncAllocator(factory));
}

void FenceSyncManager::deleteFenceSync(GLuint fenceSync)
{
    DeleteObject(&mHandleAllocator, &mObjectMap, fenceSync);
}

FenceSync *FenceSyncManager::getFenceSync(GLuint handle)
{
    return GetObject(mObjectMap, handle);
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
