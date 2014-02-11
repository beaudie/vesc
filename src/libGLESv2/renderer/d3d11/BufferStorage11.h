//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BufferStorage11.h Defines the BufferStorage11 class.

#ifndef LIBGLESV2_RENDERER_BUFFERSTORAGE11_H_
#define LIBGLESV2_RENDERER_BUFFERSTORAGE11_H_

#include "libGLESv2/renderer/BufferStorage.h"

namespace rx
{
class Renderer;
class Renderer11;
class DirectBufferStorage11;

enum BufferUsage
{
    BUFFER_USAGE_STAGING = 0,
    BUFFER_USAGE_VERTEX = 1,
    BUFFER_USAGE_INDEX = 2,
    BUFFER_USAGE_PIXEL_UNPACK = 3,
    BUFFER_USAGE_UNIFORM = 4,
};

typedef size_t DataRevision;

class BufferStorage11 : public BufferStorage
{
  public:
    explicit BufferStorage11(Renderer11 *renderer);

    static BufferStorage11 *makeBufferStorage11(BufferStorage *bufferStorage);

    virtual void *getData();
    virtual void setData(const void* data, unsigned int size, unsigned int offset);
    virtual void copyData(BufferStorage* sourceStorage, unsigned int size,
                          unsigned int sourceOffset, unsigned int destOffset);
    virtual void clear();
    virtual unsigned int getSize() const;
    virtual bool supportsDirectBinding() const;

    std::weak_ptr<ID3D11Buffer> getBuffer(BufferUsage usage);
    std::weak_ptr<ID3D11ShaderResourceView> getSRV(DXGI_FORMAT srvFormat);

  private:
    Renderer11 *mRenderer;

    std::map<BufferUsage, std::shared_ptr<DirectBufferStorage11> > mDirectBuffers;

    typedef std::pair<std::shared_ptr<ID3D11Buffer>, std::shared_ptr<ID3D11ShaderResourceView> > BufferSRVPair;
    std::map<DXGI_FORMAT, BufferSRVPair> mBufferResourceViews;

    std::vector<unsigned char> mResolvedData;
    DataRevision mResolvedDataRevision;

    unsigned int mReadUsageCount;
    unsigned int mWriteUsageCount;

    size_t mSize;

    void markBufferUsage();

    std::weak_ptr<DirectBufferStorage11> getStorage(BufferUsage usage);
    std::weak_ptr<DirectBufferStorage11> getLatestStorage() const;
};

// Each instance of BufferStorageD3DBuffer11 is specialized for a class of D3D binding points
// - vertex/transform feedback buffers
// - index buffers
// - pixel unpack buffers
// - uniform buffers
class DirectBufferStorage11
{
  public:
    DirectBufferStorage11(Renderer11 *renderer, BufferUsage usage);

    BufferUsage getUsage() const;
    std::weak_ptr<ID3D11Buffer> getD3DBuffer() const { return mDirectBuffer; }
    size_t getSize() const {return mBufferSize; }

    bool copyFromStorage(std::weak_ptr<DirectBufferStorage11> source, size_t sourceOffset, size_t size, size_t destOffset);
    void resize(size_t size, bool preserveData);

    DataRevision getDataRevision() const { return mRevision; }
    void setDataRevision(DataRevision rev) { mRevision = rev; }

  private:
    Renderer11 *mRenderer;
    const BufferUsage mUsage;
    DataRevision mRevision;

    std::shared_ptr<ID3D11Buffer> mDirectBuffer;
    size_t mBufferSize;

    static void fillBufferDesc(D3D11_BUFFER_DESC* bufferDesc, Renderer *renderer, BufferUsage usage, unsigned int bufferSize);
};

}

#endif // LIBGLESV2_RENDERER_BUFFERSTORAGE11_H_
