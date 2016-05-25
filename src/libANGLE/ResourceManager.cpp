//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ResourceManager.cpp: Implements the gl::ResourceManager class, which tracks and
// retrieves objects which may be shared by multiple Contexts.

#include "libANGLE/ResourceManager.h"

#include "libANGLE/Buffer.h"
#include "libANGLE/Fence.h"
#include "libANGLE/Program.h"
#include "libANGLE/Renderbuffer.h"
#include "libANGLE/Sampler.h"
#include "libANGLE/Shader.h"
#include "libANGLE/Texture.h"
#include "libANGLE/renderer/GLImplFactory.h"
#include "libANGLE/renderer/PathRenderingImpl.h"

namespace gl
{
ResourceManager::ResourceManager() : mRefCount(1)
{
}

ResourceManager::~ResourceManager()
{
    while (!mBufferMap.empty())
    {
        deleteBuffer(mBufferMap.begin()->first);
    }

    while (!mProgramMap.empty())
    {
        deleteProgram(mProgramMap.begin()->first);
    }

    while (!mShaderMap.empty())
    {
        deleteShader(mShaderMap.begin()->first);
    }

    while (!mRenderbufferMap.empty())
    {
        deleteRenderbuffer(mRenderbufferMap.begin()->first);
    }

    while (!mTextureMap.empty())
    {
        deleteTexture(mTextureMap.begin()->first);
    }

    while (!mSamplerMap.empty())
    {
        deleteSampler(mSamplerMap.begin()->first);
    }

    while (!mFenceSyncMap.empty())
    {
        deleteFenceSync(mFenceSyncMap.begin()->first);
    }
}

void ResourceManager::addRef()
{
    mRefCount++;
}

void ResourceManager::release()
{
    if (--mRefCount == 0)
    {
        delete this;
    }
}

// Returns an unused buffer name
GLuint ResourceManager::createBuffer()
{
    GLuint handle = mBufferHandleAllocator.allocate();

    mBufferMap[handle] = nullptr;

    return handle;
}

// Returns an unused shader/program name
GLuint ResourceManager::createShader(rx::GLImplFactory *factory,
                                     const gl::Limitations &rendererLimitations,
                                     GLenum type)
{
    GLuint handle = mProgramShaderHandleAllocator.allocate();

    if (type == GL_VERTEX_SHADER || type == GL_FRAGMENT_SHADER)
    {
        mShaderMap[handle] = new Shader(this, factory, rendererLimitations, type, handle);
    }
    else UNREACHABLE();

    return handle;
}

// Returns an unused program/shader name
GLuint ResourceManager::createProgram(rx::GLImplFactory *factory)
{
    GLuint handle = mProgramShaderHandleAllocator.allocate();

    mProgramMap[handle] = new Program(factory, this, handle);

    return handle;
}

// Returns an unused texture name
GLuint ResourceManager::createTexture()
{
    GLuint handle = mTextureHandleAllocator.allocate();

    mTextureMap[handle] = nullptr;

    return handle;
}

// Returns an unused renderbuffer name
GLuint ResourceManager::createRenderbuffer()
{
    GLuint handle = mRenderbufferHandleAllocator.allocate();

    mRenderbufferMap[handle] = nullptr;

    return handle;
}

// Returns an unused sampler name
GLuint ResourceManager::createSampler()
{
    GLuint handle = mSamplerHandleAllocator.allocate();

    mSamplerMap[handle] = nullptr;

    return handle;
}

// Returns the next unused fence name, and allocates the fence
GLuint ResourceManager::createFenceSync(rx::GLImplFactory *factory)
{
    GLuint handle = mFenceSyncHandleAllocator.allocate();

    FenceSync *fenceSync = new FenceSync(factory->createFenceSync(), handle);
    fenceSync->addRef();
    mFenceSyncMap[handle] = fenceSync;

    return handle;
}

GLuint ResourceManager::createPaths(rx::GLImplFactory* factory, GLsizei range)
{
    // Path allocation happens in (potentially) large ranges.
    // For example the classic SVG tiger (http://www.amplesdk.com/examples/markup/svg/tiger/)
    // has over 200 path objects which are all allocated in one call to the driver.
    // What we do here is that we allocate the handles from our own "handle space" that
    // we call the "client" using a PathAllocator.
    // Then we also ask the driver to perform the same allocation which gives us another range
    // of handles in a space we call "service". Then we create a mapping
    // between these two allocated ranges using the PathMapper.
    // Finally we expose the handles in the client space to the client
    // code calling into us.
    // The path ranges are all identified by their starting handle
    // and then cover the incremental range from that handle onwards.

    // Allocate client side handles.
    const GLuint client = mPathHandleAllocator.allocateRange(static_cast<GLuint>(range));
    if (client == PathAllocator::kInvalidPath)
        return 0;

    // Allocate service (driver side) handles.
    GLuint service = 0;
    Error err = factory->getPathRenderer()->createPaths(range, &service);
    if (err.isError())
    {
        mPathHandleAllocator.releaseRange(client, static_cast<GLuint>(range));
        return 0;
    }

    // setup the mapping.
    mPathMap.createMapping(client, client + range - 1, service);

    return client;
}

void ResourceManager::deleteBuffer(GLuint buffer)
{
    auto bufferObject = mBufferMap.find(buffer);

    if (bufferObject != mBufferMap.end())
    {
        mBufferHandleAllocator.release(bufferObject->first);
        if (bufferObject->second) bufferObject->second->release();
        mBufferMap.erase(bufferObject);
    }
}

void ResourceManager::deleteShader(GLuint shader)
{
    auto shaderObject = mShaderMap.find(shader);

    if (shaderObject != mShaderMap.end())
    {
        if (shaderObject->second->getRefCount() == 0)
        {
            mProgramShaderHandleAllocator.release(shaderObject->first);
            delete shaderObject->second;
            mShaderMap.erase(shaderObject);
        }
        else
        {
            shaderObject->second->flagForDeletion();
        }
    }
}

void ResourceManager::deleteProgram(GLuint program)
{
    auto programObject = mProgramMap.find(program);

    if (programObject != mProgramMap.end())
    {
        if (programObject->second->getRefCount() == 0)
        {
            mProgramShaderHandleAllocator.release(programObject->first);
            delete programObject->second;
            mProgramMap.erase(programObject);
        }
        else
        {
            programObject->second->flagForDeletion();
        }
    }
}

void ResourceManager::deleteTexture(GLuint texture)
{
    auto textureObject = mTextureMap.find(texture);

    if (textureObject != mTextureMap.end())
    {
        mTextureHandleAllocator.release(textureObject->first);
        if (textureObject->second) textureObject->second->release();
        mTextureMap.erase(textureObject);
    }
}

void ResourceManager::deleteRenderbuffer(GLuint renderbuffer)
{
    auto renderbufferObject = mRenderbufferMap.find(renderbuffer);

    if (renderbufferObject != mRenderbufferMap.end())
    {
        mRenderbufferHandleAllocator.release(renderbufferObject->first);
        if (renderbufferObject->second) renderbufferObject->second->release();
        mRenderbufferMap.erase(renderbufferObject);
    }
}

void ResourceManager::deleteSampler(GLuint sampler)
{
    auto samplerObject = mSamplerMap.find(sampler);

    if (samplerObject != mSamplerMap.end())
    {
        mSamplerHandleAllocator.release(samplerObject->first);
        if (samplerObject->second) samplerObject->second->release();
        mSamplerMap.erase(samplerObject);
    }
}

void ResourceManager::deleteFenceSync(GLuint fenceSync)
{
    auto fenceObjectIt = mFenceSyncMap.find(fenceSync);

    if (fenceObjectIt != mFenceSyncMap.end())
    {
        mFenceSyncHandleAllocator.release(fenceObjectIt->first);
        if (fenceObjectIt->second) fenceObjectIt->second->release();
        mFenceSyncMap.erase(fenceObjectIt);
    }
}

void ResourceManager::deletePaths(rx::GLImplFactory* factory, GLuint first, GLsizei range)
{
    std::vector<PathMapper::RemovedRange> graveyard;
    mPathMap.removeMapping(first, first + range - 1, &graveyard);

    for (const auto& carcass : graveyard)
    {
        GLuint first = carcass.mFirstServiceHandle;
        GLuint range = carcass.mRange;
        while (range > 0)
        {
            GLsizei irange;
            if (range >= static_cast<GLuint>(std::numeric_limits<GLsizei>::max()))
                irange = std::numeric_limits<GLsizei>::max();
            else irange = static_cast<GLsizei>(range);

            factory->getPathRenderer()->deletePaths(first, range);

            range -= irange;
            first += irange;
        }
    }

    mPathHandleAllocator.releaseRange(first, static_cast<GLuint>(range));
}


Buffer *ResourceManager::getBuffer(unsigned int handle)
{
    auto buffer = mBufferMap.find(handle);

    if (buffer == mBufferMap.end())
    {
        return nullptr;
    }
    else
    {
        return buffer->second;
    }
}

Shader *ResourceManager::getShader(unsigned int handle)
{
    auto shader = mShaderMap.find(handle);

    if (shader == mShaderMap.end())
    {
        return nullptr;
    }
    else
    {
        return shader->second;
    }
}

Texture *ResourceManager::getTexture(unsigned int handle)
{
    if (handle == 0)
        return nullptr;

    auto texture = mTextureMap.find(handle);

    if (texture == mTextureMap.end())
    {
        return nullptr;
    }
    else
    {
        return texture->second;
    }
}

Program *ResourceManager::getProgram(unsigned int handle) const
{
    auto program = mProgramMap.find(handle);

    if (program == mProgramMap.end())
    {
        return nullptr;
    }
    else
    {
        return program->second;
    }
}

Renderbuffer *ResourceManager::getRenderbuffer(unsigned int handle)
{
    auto renderbuffer = mRenderbufferMap.find(handle);

    if (renderbuffer == mRenderbufferMap.end())
    {
        return nullptr;
    }
    else
    {
        return renderbuffer->second;
    }
}

Sampler *ResourceManager::getSampler(unsigned int handle)
{
    auto sampler = mSamplerMap.find(handle);

    if (sampler == mSamplerMap.end())
    {
        return nullptr;
    }
    else
    {
        return sampler->second;
    }
}

FenceSync *ResourceManager::getFenceSync(unsigned int handle)
{
    auto fenceObjectIt = mFenceSyncMap.find(handle);

    if (fenceObjectIt == mFenceSyncMap.end())
    {
        return nullptr;
    }
    else
    {
        return fenceObjectIt->second;
    }
}

GLuint ResourceManager::getPath(GLuint path) const
{
    GLuint service = 0;
    mPathMap.getPath(path, &service);
    return service;
}

bool ResourceManager::isPath(rx::GLImplFactory* factory, GLuint path) const
{
    if (!mPathHandleAllocator.isUsed(path))
        return false;

    GLuint service = 0;
    mPathMap.getPath(path, &service);

    return factory->getPathRenderer()->isPath(service);
}

bool ResourceManager::hasPath(GLuint handle) const
{
    return mPathHandleAllocator.isUsed(handle);
}

Error ResourceManager::setPathCommands(rx::GLImplFactory *factory, GLuint path, GLsizei numCommands,
                                      const GLubyte *commands,
                                      GLsizei numCoords,
                                      GLenum coordType,
                                      const void *coords)
{
    GLuint service = 0;
    mPathMap.getPath(path, &service);

    return factory->getPathRenderer()->setCommands(service, numCommands,
        commands, numCoords, coordType, coords);
}

void ResourceManager::setPathParameter(rx::GLImplFactory *factory, GLuint path, GLenum pname, GLfloat value)
{
    GLuint service = 0;
    mPathMap.getPath(path, &service);

    factory->getPathRenderer()->setPathParameter(service, pname, value);
}

void ResourceManager::getPathParameter(rx::GLImplFactory *factory, GLuint path, GLenum pname, GLfloat *value) const
{
    GLuint service = 0;
    mPathMap.getPath(path, &service);

    factory->getPathRenderer()->getPathParameter(service, pname, value);
}


void ResourceManager::setRenderbuffer(GLuint handle, Renderbuffer *buffer)
{
    mRenderbufferMap[handle] = buffer;
}

Buffer *ResourceManager::checkBufferAllocation(rx::GLImplFactory *factory, GLuint handle)
{
    if (handle == 0)
    {
        return nullptr;
    }

    auto bufferMapIt     = mBufferMap.find(handle);
    bool handleAllocated = (bufferMapIt != mBufferMap.end());

    if (handleAllocated && bufferMapIt->second != nullptr)
    {
        return bufferMapIt->second;
    }

    Buffer *buffer = new Buffer(factory->createBuffer(), handle);
    buffer->addRef();

    if (handleAllocated)
    {
        bufferMapIt->second = buffer;
    }
    else
    {
        mBufferHandleAllocator.reserve(handle);
        mBufferMap[handle] = buffer;
    }

    return buffer;
}

Texture *ResourceManager::checkTextureAllocation(rx::GLImplFactory *factory,
                                                 GLuint handle,
                                                 GLenum type)
{
    if (handle == 0)
    {
        return nullptr;
    }

    auto textureMapIt    = mTextureMap.find(handle);
    bool handleAllocated = (textureMapIt != mTextureMap.end());

    if (handleAllocated && textureMapIt->second != nullptr)
    {
        return textureMapIt->second;
    }

    Texture *texture = new Texture(factory, handle, type);
    texture->addRef();

    if (handleAllocated)
    {
        textureMapIt->second = texture;
    }
    else
    {
        mTextureHandleAllocator.reserve(handle);
        mTextureMap[handle] = texture;
    }

    return texture;
}

Renderbuffer *ResourceManager::checkRenderbufferAllocation(rx::GLImplFactory *factory,
                                                           GLuint handle)
{
    if (handle == 0)
    {
        return nullptr;
    }

    auto renderbufferMapIt = mRenderbufferMap.find(handle);
    bool handleAllocated   = (renderbufferMapIt != mRenderbufferMap.end());

    if (handleAllocated && renderbufferMapIt->second != nullptr)
    {
        return renderbufferMapIt->second;
    }

    Renderbuffer *renderbuffer = new Renderbuffer(factory->createRenderbuffer(), handle);
    renderbuffer->addRef();

    if (handleAllocated)
    {
        renderbufferMapIt->second = renderbuffer;
    }
    else
    {
        mRenderbufferHandleAllocator.reserve(handle);
        mRenderbufferMap[handle] = renderbuffer;
    }

    return renderbuffer;
}

Sampler *ResourceManager::checkSamplerAllocation(rx::GLImplFactory *factory, GLuint samplerHandle)
{
    // Samplers cannot be created via Bind
    if (samplerHandle == 0)
    {
        return nullptr;
    }

    Sampler *sampler = getSampler(samplerHandle);

    if (!sampler)
    {
        sampler                    = new Sampler(factory, samplerHandle);
        mSamplerMap[samplerHandle] = sampler;
        sampler->addRef();
    }

    return sampler;
}

bool ResourceManager::isSampler(GLuint sampler)
{
    return mSamplerMap.find(sampler) != mSamplerMap.end();
}

}  // namespace gl
