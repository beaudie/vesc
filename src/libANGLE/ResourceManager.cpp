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
#include "libANGLE/Path.h"
#include "libANGLE/Program.h"
#include "libANGLE/Renderbuffer.h"
#include "libANGLE/Sampler.h"
#include "libANGLE/Shader.h"
#include "libANGLE/Texture.h"
#include "libANGLE/renderer/GLImplFactory.h"

namespace gl
{

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

    gl::Buffer *BufferManager::getBuffer(GLuint handle) const
    {
        return getObject(handle);
    }

    gl::Buffer *BufferManager::checkBufferAllocation(rx::GLImplFactory *factory, GLuint handle)
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
                                       const gl::Limitations &rendererLimitations,
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

    gl::Shader *ShaderManager::getShader(GLuint handle) const
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

    gl::Program *ProgramManager::getProgram(GLuint handle) const
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

    gl::Texture *TextureManager::getTexture(GLuint handle) const
    {
        ASSERT(getObject(0) == nullptr);
        return getObject(handle);
    }

    gl::Texture *TextureManager::checkTextureAllocation(rx::GLImplFactory *factory,
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

    gl::Renderbuffer *RenderbufferManager::getRenderbuffer(GLuint handle)
    {
        return getObject(handle);
    }

    gl::Renderbuffer *RenderbufferManager::checkRenderbufferAllocation(rx::GLImplFactory *factory,
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

    gl::Sampler *SamplerManager::getSampler(GLuint handle)
    {
        return getObject(handle);
    }

    gl::Sampler *SamplerManager::checkSamplerAllocation(rx::GLImplFactory *factory, GLuint handle)
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

    gl::FenceSync *FenceSyncManager::getFenceSync(GLuint handle)
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

    gl::ErrorOrResult<GLuint> PathManager::createPaths(rx::GLImplFactory *factory, GLsizei range)
    {
        // Allocate client side handles.
        const GLuint client = mHandleAllocator.allocateRange(static_cast<GLuint>(range));
        if (client == HandleRangeAllocator::kInvalidHandle)
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to allocate path handle range.");

        const auto &paths = factory->createPaths(range);
        if (paths.empty())
        {
            mHandleAllocator.releaseRange(client, range);
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to allocate path objects.");
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

    gl::Path *PathManager::getPath(GLuint handle) const
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

}  // namespace gl
