//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Buffer11.h: Defines the rx::Buffer11 class which implements rx::BufferImpl via rx::BufferD3D.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_BUFFER11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_BUFFER11_H_

#include <map>

#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/d3d/BufferD3D.h"

namespace rx
{
class Renderer11;

enum BufferUsage
{
    BUFFER_USAGE_STAGING,
    BUFFER_USAGE_VERTEX_OR_TRANSFORM_FEEDBACK,
    BUFFER_USAGE_INDEX,
    BUFFER_USAGE_PIXEL_UNPACK,
    BUFFER_USAGE_PIXEL_PACK,
    BUFFER_USAGE_UNIFORM,
    BUFFER_USAGE_SYSTEM_MEMORY,
};

struct PackPixelsParams
{
    PackPixelsParams();
    PackPixelsParams(const gl::Rectangle &area, GLenum format, GLenum type, GLuint outputPitch,
                     const gl::PixelPackState &pack, ptrdiff_t offset);

    gl::Rectangle area;
    GLenum format;
    GLenum type;
    GLuint outputPitch;
    gl::Buffer *packBuffer;
    gl::PixelPackState pack;
    ptrdiff_t offset;
};

typedef size_t DataRevision;

class Buffer11 : public BufferD3D
{
  public:
    Buffer11(Renderer11 *renderer);
    virtual ~Buffer11();

    static Buffer11 *makeBuffer11(BufferImpl *buffer);

    ID3D11Buffer *getBuffer(BufferUsage usage);
    ID3D11ShaderResourceView *getSRV(DXGI_FORMAT srvFormat);
    bool isMapped() const { return mMappedStorage != NULL; }
    gl::Error packPixels(ID3D11Texture2D *srcTexure, UINT srcSubresource, const PackPixelsParams &params);

    // BufferD3D implementation
    virtual size_t getSize() const { return mSize; }
    virtual bool supportsDirectBinding() const;
    RendererD3D *getRenderer() override;

    // BufferImpl implementation
    virtual gl::Error setData(size_t size, const uint8_t *data, GLenum usage);
    gl::Error getData(const uint8_t **outData) override;
    virtual gl::Error setSubData(size_t offset, size_t size, const uint8_t *data);
    virtual gl::Error copySubData(const gl::Buffer *source, size_t sourceOffset, size_t destOffset, size_t size);
    virtual gl::Error map(GLbitfield access, GLvoid **mapPtr);
    virtual gl::Error mapRange(size_t offset, size_t length, GLbitfield access, GLvoid **mapPtr);
    virtual gl::Error unmap(GLboolean *result);
    virtual void markTransformFeedbackUsage();

  private:
    DISALLOW_COPY_AND_ASSIGN(Buffer11);

    class BufferStorage;
    class NativeStorage;
    class PackStorage;
    class SystemMemoryStorage;

    Renderer11 *mRenderer;
    size_t mSize;

    BufferStorage *mMappedStorage;

    std::map<BufferUsage, BufferStorage*> mBufferStorages;

    typedef std::pair<ID3D11Buffer *, ID3D11ShaderResourceView *> BufferSRVPair;
    std::map<DXGI_FORMAT, BufferSRVPair> mBufferResourceViews;

    unsigned int mReadUsageCount;
    bool mHasSystemMemoryStorage;

    void markBufferUsage();
    NativeStorage *getStagingStorage();
    PackStorage *getPackStorage();
    gl::Error getSystemMemoryStorage(SystemMemoryStorage **storageOut);

    BufferStorage *getBufferStorage(BufferUsage usage);
    BufferStorage *getLatestBufferStorage() const;
};

}

#endif // LIBANGLE_RENDERER_D3D_D3D11_BUFFER11_H_
