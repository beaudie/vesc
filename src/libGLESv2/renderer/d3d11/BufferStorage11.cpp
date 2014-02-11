#include "precompiled.h"
//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BufferStorage11.cpp Defines the BufferStorage11 class.

#include "libGLESv2/renderer/d3d11/BufferStorage11.h"
#include "libGLESv2/main.h"
#include "libGLESv2/renderer/d3d11/Renderer11.h"
#include "libGLESv2/renderer/d3d11/formatutils11.h"
#include "libGLESv2/renderer/d3d11/renderer11_utils.h"

namespace rx
{

BufferStorage11::BufferStorage11(Renderer11 *renderer)
    : mRenderer(renderer),
      mResolvedDataRevision(0),
      mReadUsageCount(0),
      mWriteUsageCount(0),
      mSize(0)
{
}

BufferStorage11 *BufferStorage11::makeBufferStorage11(BufferStorage *bufferStorage)
{
    ASSERT(HAS_DYNAMIC_TYPE(BufferStorage11*, bufferStorage));
    return static_cast<BufferStorage11*>(bufferStorage);
}

void *BufferStorage11::getData()
{
    std::shared_ptr<DirectBufferStorage11> stagingBuffer = getStorage(BUFFER_USAGE_STAGING).lock();
    if (stagingBuffer->getDataRevision() > mResolvedDataRevision)
    {
        if (stagingBuffer->getSize() > mResolvedData.size())
        {
            mResolvedData.resize(stagingBuffer->getSize());
        }

        ID3D11DeviceContext *context = mRenderer->getDeviceContext();

        std::shared_ptr<ID3D11Buffer> d3dBuffer = stagingBuffer->getD3DBuffer().lock();

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        HRESULT result = context->Map(d3dBuffer.get(), 0, D3D11_MAP_READ, 0, &mappedResource);
        if (FAILED(result))
        {
            return gl::error(GL_OUT_OF_MEMORY, (void*)NULL);
        }

        memcpy(mResolvedData.data(), mappedResource.pData, stagingBuffer->getSize());

        context->Unmap(d3dBuffer.get(), 0);

        mResolvedDataRevision = stagingBuffer->getDataRevision();
    }

    return mResolvedData.data();
}

void BufferStorage11::setData(const void* data, unsigned int size, unsigned int offset)
{
    std::shared_ptr<DirectBufferStorage11> stagingBuffer = getStorage(BUFFER_USAGE_STAGING).lock();

    // Explicitly resize the staging buffer, preserving data if the new data will not
    // completely fill the buffer
    size_t requiredSize = size + offset;
    if (stagingBuffer->getSize() < requiredSize)
    {
        bool preserveData = (offset > 0);
        stagingBuffer->resize(requiredSize, preserveData);
    }

    if (data)
    {
        ID3D11DeviceContext *context = mRenderer->getDeviceContext();

        std::shared_ptr<ID3D11Buffer> d3dBuffer = stagingBuffer->getD3DBuffer().lock();

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        HRESULT result = context->Map(d3dBuffer.get(), 0, D3D11_MAP_WRITE, 0, &mappedResource);
        if (FAILED(result))
        {
            return gl::error(GL_OUT_OF_MEMORY);
        }

        unsigned char *offsetBufferPointer = reinterpret_cast<unsigned char *>(mappedResource.pData) + offset;
        memcpy(offsetBufferPointer, data, size);

        context->Unmap(d3dBuffer.get(), 0);
    }

    stagingBuffer->setDataRevision(stagingBuffer->getDataRevision() + 1);
    mWriteUsageCount = 0;
    mSize = std::max(mSize, requiredSize);
}

void BufferStorage11::copyData(BufferStorage* sourceStorage, unsigned int size,
                               unsigned int sourceOffset, unsigned int destOffset)
{
    BufferStorage11* sourceStorage11 = makeBufferStorage11(sourceStorage);
    if (sourceStorage11)
    {
        std::shared_ptr<DirectBufferStorage11> dest = getLatestStorage().lock();
        if (!dest)
        {
            dest = getStorage(BUFFER_USAGE_STAGING).lock();
        }

        std::shared_ptr<DirectBufferStorage11> source = sourceStorage11->getLatestStorage().lock();
        if (source && dest)
        {
            dest->copyFromStorage(source, sourceOffset, size, destOffset);
            dest->setDataRevision(dest->getDataRevision() + 1);
        }

        mSize = std::max(mSize, destOffset + size);
    }
}

void BufferStorage11::clear()
{
    mSize = 0;
    mResolvedDataRevision = 0;
}

unsigned int BufferStorage11::getSize() const
{
    return mSize;
}

bool BufferStorage11::supportsDirectBinding() const
{
    return true;
}

void BufferStorage11::markBufferUsage()
{
    mReadUsageCount++;
    mWriteUsageCount++;

    const unsigned int usageLimit = 5;

    if (mReadUsageCount > usageLimit && mResolvedData.size() > 0)
    {
        mResolvedData.resize(0);
        mResolvedDataRevision = 0;
    }
}

std::weak_ptr<ID3D11Buffer> BufferStorage11::getBuffer(BufferUsage usage)
{
    markBufferUsage();
    return getStorage(usage).lock()->getD3DBuffer();
}

std::weak_ptr<ID3D11ShaderResourceView> BufferStorage11::getSRV(DXGI_FORMAT srvFormat)
{
    std::shared_ptr<DirectBufferStorage11> storage = getStorage(BUFFER_USAGE_PIXEL_UNPACK).lock();
    std::shared_ptr<ID3D11Buffer> buffer = storage->getD3DBuffer().lock();

    auto bufferSRVIt = mBufferResourceViews.find(srvFormat);

    if (bufferSRVIt != mBufferResourceViews.end())
    {
        if (bufferSRVIt->second.first == buffer)
        {
            return bufferSRVIt->second.second;
        }
    }

    ID3D11Device *device = mRenderer->getDevice();
    ID3D11ShaderResourceView *bufferSRV = NULL;

    D3D11_SHADER_RESOURCE_VIEW_DESC bufferSRVDesc;
    bufferSRVDesc.Buffer.ElementOffset = 0;
    bufferSRVDesc.Buffer.ElementWidth = mSize / d3d11::GetFormatPixelBytes(srvFormat);
    bufferSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    bufferSRVDesc.Format = srvFormat;

    HRESULT result = device->CreateShaderResourceView(buffer.get(), &bufferSRVDesc, &bufferSRV);
    ASSERT(SUCCEEDED(result));

    std::shared_ptr<ID3D11ShaderResourceView> srvPtr = d3d11::MakeSharedCOM(bufferSRV);
    mBufferResourceViews[srvFormat] = BufferSRVPair(buffer, srvPtr);

    return srvPtr;
}

std::weak_ptr<DirectBufferStorage11> BufferStorage11::getStorage(BufferUsage usage)
{
    std::shared_ptr<DirectBufferStorage11> directBuffer;
    auto directBufferIt = mDirectBuffers.find(usage);
    if (directBufferIt != mDirectBuffers.end())
    {
        directBuffer = directBufferIt->second;
    }

    if (!directBuffer)
    {
        // buffer is not allocated, create it
        directBuffer = std::make_shared<DirectBufferStorage11>(mRenderer, usage);
        mDirectBuffers.insert(std::make_pair(usage, directBuffer));
    }

    std::shared_ptr<DirectBufferStorage11> latestBuffer = getLatestStorage().lock();
    if (latestBuffer && latestBuffer->getDataRevision() > directBuffer->getDataRevision())
    {
        // if copyFromStorage returns true, the D3D buffer has been recreated
        // and we should update our serial
        if (directBuffer->copyFromStorage(latestBuffer, 0, latestBuffer->getSize(), 0))
        {
            updateSerial();
        }
        directBuffer->setDataRevision(latestBuffer->getDataRevision());
    }

    return directBuffer;
}

std::weak_ptr<DirectBufferStorage11> BufferStorage11::getLatestStorage() const
{
    // Even though we iterate over all the direct buffers, it is expected that only
    // 1 or 2 will be present.
    std::shared_ptr<DirectBufferStorage11> latestStorage;
    DataRevision latestRevision = 0;
    for (auto it = mDirectBuffers.begin(); it != mDirectBuffers.end(); it++)
    {
        std::shared_ptr<DirectBufferStorage11> storage = it->second;
        if (storage->getDataRevision() > latestRevision)
        {
            latestStorage = storage;
            latestRevision = storage->getDataRevision();
        }
    }

    return latestStorage;
}

DirectBufferStorage11::DirectBufferStorage11(Renderer11 *renderer, BufferUsage usage)
    : mRenderer(renderer),
      mUsage(usage),
      mRevision(0),
      mDirectBuffer(NULL),
      mBufferSize(0)
{
}

BufferUsage DirectBufferStorage11::getUsage() const
{
    return mUsage;
}

// Returns true if it recreates the direct buffer
bool DirectBufferStorage11::copyFromStorage(std::weak_ptr<DirectBufferStorage11> source, size_t sourceOffset, size_t size, size_t destOffset)
{
    ID3D11DeviceContext *context = mRenderer->getDeviceContext();
    std::shared_ptr<DirectBufferStorage11> lockedSource = source.lock();

    size_t requiredSize = sourceOffset + size;
    bool createBuffer = !mDirectBuffer || mBufferSize < requiredSize;

    // (Re)initialize D3D buffer if needed
    if (createBuffer)
    {
        bool preserveData = (destOffset > 0);
        resize(lockedSource->getSize(), preserveData);
    }

    D3D11_BOX srcBox;
    srcBox.left = sourceOffset;
    srcBox.right = sourceOffset + size;
    srcBox.top = 0;
    srcBox.bottom = 1;
    srcBox.front = 0;
    srcBox.back = 1;

    context->CopySubresourceRegion(mDirectBuffer.get(), 0, destOffset, 0, 0, lockedSource->mDirectBuffer.get(), 0, &srcBox);

    return createBuffer;
}

void DirectBufferStorage11::resize(size_t size, bool preserveData)
{
    ID3D11Device *device = mRenderer->getDevice();
    ID3D11DeviceContext *context = mRenderer->getDeviceContext();

    D3D11_BUFFER_DESC bufferDesc;
    fillBufferDesc(&bufferDesc, mRenderer, mUsage, size);

    ID3D11Buffer *newBuffer;
    HRESULT result = device->CreateBuffer(&bufferDesc, NULL, &newBuffer);

    if (FAILED(result))
    {
        return gl::error(GL_OUT_OF_MEMORY);
    }

    if (mDirectBuffer && preserveData)
    {
        D3D11_BOX srcBox;
        srcBox.left = 0;
        srcBox.right = size;
        srcBox.top = 0;
        srcBox.bottom = 1;
        srcBox.front = 0;
        srcBox.back = 1;

        context->CopySubresourceRegion(newBuffer, 0, 0, 0, 0, mDirectBuffer.get(), 0, &srcBox);
    }

    // No longer need the old buffer
    mDirectBuffer = d3d11::MakeSharedCOM(newBuffer);

    mBufferSize = bufferDesc.ByteWidth;
}

void DirectBufferStorage11::fillBufferDesc(D3D11_BUFFER_DESC* bufferDesc, Renderer *renderer, BufferUsage usage, unsigned int bufferSize)
{
    bufferDesc->ByteWidth = bufferSize;
    bufferDesc->MiscFlags = 0;
    bufferDesc->StructureByteStride = 0;

    switch (usage)
    {
      case BUFFER_USAGE_STAGING:
        bufferDesc->Usage = D3D11_USAGE_STAGING;
        bufferDesc->BindFlags = 0;
        bufferDesc->CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
        break;

      case BUFFER_USAGE_VERTEX:
        bufferDesc->Usage = D3D11_USAGE_DEFAULT;
        bufferDesc->BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc->CPUAccessFlags = 0;
        break;

      case BUFFER_USAGE_INDEX:
        bufferDesc->Usage = D3D11_USAGE_DEFAULT;
        bufferDesc->BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDesc->CPUAccessFlags = 0;
        break;

      case BUFFER_USAGE_PIXEL_UNPACK:
        bufferDesc->Usage = D3D11_USAGE_DEFAULT;
        bufferDesc->BindFlags = D3D11_BIND_SHADER_RESOURCE;
        bufferDesc->CPUAccessFlags = 0;
        break;

      case BUFFER_USAGE_UNIFORM:
        bufferDesc->Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc->BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc->CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        // Constant buffers must be of a limited size, and aligned to 16 byte boundaries
        // For our purposes we ignore any buffer data past the maximum constant buffer size
        bufferDesc->ByteWidth = roundUp(bufferDesc->ByteWidth, 16u);
        bufferDesc->ByteWidth = std::min(bufferDesc->ByteWidth, renderer->getMaxUniformBufferSize());
        break;

    default:
        UNREACHABLE();
    }
}

}
