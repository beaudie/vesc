//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// InputLayoutCache.cpp: Defines InputLayoutCache, a class that builds and caches
// D3D11 input layouts.

#include "libANGLE/renderer/d3d/d3d11/InputLayoutCache.h"
#include "libANGLE/renderer/d3d/d3d11/VertexBuffer11.h"
#include "libANGLE/renderer/d3d/d3d11/IndexBuffer11.h"
#include "libANGLE/renderer/d3d/d3d11/Buffer11.h"
#include "libANGLE/renderer/d3d/d3d11/ShaderExecutable11.h"
#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"
#include "libANGLE/renderer/d3d/ProgramD3D.h"
#include "libANGLE/renderer/d3d/VertexDataManager.h"
#include "libANGLE/renderer/d3d/IndexDataManager.h"
#include "libANGLE/renderer/d3d/IndexBuffer.h"
#include "libANGLE/Program.h"
#include "libANGLE/VertexAttribute.h"

#include "third_party/murmurhash/MurmurHash3.h"

namespace rx
{

static void GetInputLayout(const TranslatedAttribute translatedAttributes[gl::MAX_VERTEX_ATTRIBS],
                           gl::VertexFormat inputLayout[gl::MAX_VERTEX_ATTRIBS])
{
    for (unsigned int attributeIndex = 0; attributeIndex < gl::MAX_VERTEX_ATTRIBS; attributeIndex++)
    {
        const TranslatedAttribute &translatedAttribute = translatedAttributes[attributeIndex];

        if (translatedAttributes[attributeIndex].active)
        {
            inputLayout[attributeIndex] = gl::VertexFormat(*translatedAttribute.attribute,
                                                           translatedAttribute.currentValueType);
        }
    }
}

const unsigned int InputLayoutCache::kMaxInputLayouts = 1024;

InputLayoutCache::InputLayoutCache() : mInputLayoutMap(kMaxInputLayouts, hashInputLayout, compareInputLayouts)
{
    mCounter = 0;
    mDevice = NULL;
    mDeviceContext = NULL;
    mCurrentIL = NULL;
    for (unsigned int i = 0; i < gl::MAX_VERTEX_ATTRIBS; i++)
    {
        mCurrentBuffers[i] = NULL;
        mCurrentVertexStrides[i] = -1;
        mCurrentVertexOffsets[i] = -1;
    }
    mPointSpriteVertexBuffer = NULL;
    mPointSpriteIndexBuffer = NULL;
    mIndexedPointSpriteEnableBuffer = NULL;
}

InputLayoutCache::~InputLayoutCache()
{
    clear();
}

void InputLayoutCache::initialize(ID3D11Device *device, ID3D11DeviceContext *context)
{
    clear();
    mDevice = device;
    mDeviceContext = context;
    mFeatureLevel = device->GetFeatureLevel();
}

void InputLayoutCache::clear()
{
    for (InputLayoutMap::iterator i = mInputLayoutMap.begin(); i != mInputLayoutMap.end(); i++)
    {
        SafeRelease(i->second.inputLayout);
    }
    mInputLayoutMap.clear();
    SafeRelease(mPointSpriteVertexBuffer);
    SafeRelease(mPointSpriteIndexBuffer);
    SafeRelease(mIndexedPointSpriteEnableBuffer);
    markDirty();
}

void InputLayoutCache::markDirty()
{
    mCurrentIL = NULL;
    for (unsigned int i = 0; i < gl::MAX_VERTEX_ATTRIBS; i++)
    {
        mCurrentBuffers[i] = NULL;
        mCurrentVertexStrides[i] = -1;
        mCurrentVertexOffsets[i] = -1;
    }
}

void InputLayoutCache::configureIndexedPointSpriteEnableBuffer(std::vector<float> &enableBuffer, TranslatedIndexData *indexInfo)
{
    // Reset the temporary enable buffer to ensure that all pointsprites are configured to not
    // be rendered.  Only pointsprites indicated in the caller's supplied index buffer will be 
    // marked as enabled.
    std::fill(enableBuffer.begin(), enableBuffer.end(), 1.0f);

    // The caller's specified index buffer is read and translated into the point mapping
    // buffer by setting a value to 0 (meaning the point is to be rendered) or
    // or 1 (meaning the point is not to be rendered).
    // 0 is chosed to represent enabled because it simplifies the shader implementation
    // by eliminating the need for a branch (if statement) for determining if the point should be
    // rendered or not.  Branches in shaders are expensive and non performant.

    // First determine format and size of the caller supplied index buffer.  This is needed for
    // accessing and reading index values for mapping.
    DXGI_FORMAT bufferFormat = (indexInfo->indexType == GL_UNSIGNED_INT) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
    unsigned int elementSizeBytes = 0;
    switch (bufferFormat)
    {
    case DXGI_FORMAT_R32_UINT: elementSizeBytes = sizeof(GLuint); break;
    case DXGI_FORMAT_R16_UINT: elementSizeBytes = sizeof(GLushort); break;
    default: UNREACHABLE(); elementSizeBytes = 0;
    }

    // Obtain a pointer to the raw index values.  This buffer will be accessed in a read-only way.
    // Caller suppliked index data can be accessed either directly in the storage itself, or from
    // a translated index buffer. Both of these options must be checked to obtain the correct buffer.
    // If the storage is NULL, then it is assumed that the index buffer contains the indices needed.
    void* indices = nullptr;
    unsigned int indicesBufferSize = 0;

    if (indexInfo->storage)
    {
        Buffer11 *storage = Buffer11::makeBuffer11(indexInfo->storage);
        indicesBufferSize = storage->getSize();
        const uint8_t* data = nullptr;
        storage->getData(&data);
        indices = (void*)data;
    }
    else
    {
        // If the index buffer was used, then ensure that it is mapped before continuing.
        // The index buffer will be unmapped below.
        indicesBufferSize = indexInfo->indexBuffer->getBufferSize();
        indexInfo->indexBuffer->mapBuffer(0, indicesBufferSize, &indices);
    }

    // Iterate over the indices list and update the temporary buffer that represents the
    // enable/disable points map.
    // Note that the total number of indices is calculated using the start offset configured
    // in the index information.  This ensures that we start reading indices from the correct
    // position.
    unsigned int totalIndices = (indicesBufferSize - indexInfo->startOffset) / elementSizeBytes;
    ASSERT(totalIndices <= enableBuffer.size());
    uint8_t* ptr = (uint8_t*)indices;
    // The indices pointer must also be offset to start at the read position.
    ptr += indexInfo->startOffset;
    for (unsigned int i = 0; i < totalIndices; i++)
    {
        unsigned int idx = 0;
        switch (bufferFormat)
        {
        case DXGI_FORMAT_R32_UINT: idx = ((GLuint*)ptr)[i]; break;
        case DXGI_FORMAT_R16_UINT: idx = ((GLushort*)ptr)[i]; break;
        default: UNREACHABLE();
        }

        // Only enable points read from the the indices data
        ASSERT(idx < enableBuffer.size());
        enableBuffer[idx] = 0.0f;
    }

    // If the index buffer was used, then ensure that it is unmapped before continuing.
    if (indexInfo->indexBuffer)
    {
        indexInfo->indexBuffer->unmapBuffer();
    }
}

gl::Error InputLayoutCache::applyVertexBuffers(TranslatedAttribute attributes[gl::MAX_VERTEX_ATTRIBS],
                                               GLenum mode, gl::Program *program, TranslatedIndexData *indexInfo,
                                               bool appliedIndexBufferChanged)
{
    ProgramD3D *programD3D = GetImplAs<ProgramD3D>(program);

    int sortedSemanticIndices[gl::MAX_VERTEX_ATTRIBS];
    programD3D->sortAttributesByLayout(attributes, sortedSemanticIndices);
    bool programUsesInstancedPointSprites = programD3D->usesPointSize() && programD3D->usesInstancedPointSpriteEmulation();
    bool instancedPointSpritesActive = programUsesInstancedPointSprites && (mode == GL_POINTS);
    bool indexedPointSpritesActive = instancedPointSpritesActive && (indexInfo != nullptr);
    int indexedPointSpriteCount = (indexInfo != nullptr) ? (indexInfo->indexRange.end + 1) : 0;

    if (!mDevice || !mDeviceContext)
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Internal input layout cache is not initialized.");
    }

    InputLayoutKey ilKey = { 0 };

    static const char* semanticName = "TEXCOORD";

    unsigned int firstIndexedElement = gl::MAX_VERTEX_ATTRIBS;
    unsigned int firstInstancedElement = gl::MAX_VERTEX_ATTRIBS;
    unsigned int nextAvailableInputSlot = 0;

    for (unsigned int i = 0; i < gl::MAX_VERTEX_ATTRIBS; i++)
    {
        if (attributes[i].active)
        {
            D3D11_INPUT_CLASSIFICATION inputClass = attributes[i].divisor > 0 ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
            // If rendering points and instanced pointsprite emulation is being used, the inputClass is required to be configured as per instance data
            inputClass = instancedPointSpritesActive ? D3D11_INPUT_PER_INSTANCE_DATA : inputClass;

            gl::VertexFormat vertexFormat(*attributes[i].attribute, attributes[i].currentValueType);
            const d3d11::VertexFormat &vertexFormatInfo = d3d11::GetVertexFormatInfo(vertexFormat, mFeatureLevel);

            // Record the type of the associated vertex shader vector in our key
            // This will prevent mismatched vertex shaders from using the same input layout
            GLint attributeSize;
            program->getActiveAttribute(ilKey.elementCount, 0, NULL, &attributeSize, &ilKey.elements[ilKey.elementCount].glslElementType, NULL);

            ilKey.elements[ilKey.elementCount].desc.SemanticName = semanticName;
            ilKey.elements[ilKey.elementCount].desc.SemanticIndex = sortedSemanticIndices[i];
            ilKey.elements[ilKey.elementCount].desc.Format = vertexFormatInfo.nativeFormat;
            ilKey.elements[ilKey.elementCount].desc.InputSlot = i;
            ilKey.elements[ilKey.elementCount].desc.AlignedByteOffset = 0;
            ilKey.elements[ilKey.elementCount].desc.InputSlotClass = inputClass;
            ilKey.elements[ilKey.elementCount].desc.InstanceDataStepRate = instancedPointSpritesActive ? 1 : attributes[i].divisor;

            if (inputClass == D3D11_INPUT_PER_VERTEX_DATA && firstIndexedElement == gl::MAX_VERTEX_ATTRIBS)
            {
                firstIndexedElement = ilKey.elementCount;
            }
            else if (inputClass == D3D11_INPUT_PER_INSTANCE_DATA && firstInstancedElement == gl::MAX_VERTEX_ATTRIBS)
            {
                firstInstancedElement = ilKey.elementCount;
            }

            ilKey.elementCount++;
            nextAvailableInputSlot = i + 1;
        }
    }

    // Instanced PointSprite emulation requires additional entries in the
    // inputlayout to support the vertices that make up the pointsprite quad.
    // We do this even if mode != GL_POINTS, since the shader signature has these inputs, and the input layout must match the shader
    if (programUsesInstancedPointSprites)
    {
        ilKey.elements[ilKey.elementCount].desc.SemanticName = "SPRITEPOSITION";
        ilKey.elements[ilKey.elementCount].desc.SemanticIndex = 0;
        ilKey.elements[ilKey.elementCount].desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
        ilKey.elements[ilKey.elementCount].desc.InputSlot = nextAvailableInputSlot;
        ilKey.elements[ilKey.elementCount].desc.AlignedByteOffset = 0;
        ilKey.elements[ilKey.elementCount].desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        ilKey.elements[ilKey.elementCount].desc.InstanceDataStepRate = 0;

        // The new elements are D3D11_INPUT_PER_VERTEX_DATA data so the indexed element
        // tracking must be applied.  This ensures that the instancing specific
        // buffer swapping logic continues to work.
        if (firstIndexedElement == gl::MAX_VERTEX_ATTRIBS)
        {
            firstIndexedElement = ilKey.elementCount;
        }

        ilKey.elementCount++;

        ilKey.elements[ilKey.elementCount].desc.SemanticName = "SPRITETEXCOORD";
        ilKey.elements[ilKey.elementCount].desc.SemanticIndex = 0;
        ilKey.elements[ilKey.elementCount].desc.Format = DXGI_FORMAT_R32G32_FLOAT;
        ilKey.elements[ilKey.elementCount].desc.InputSlot = nextAvailableInputSlot;
        ilKey.elements[ilKey.elementCount].desc.AlignedByteOffset = sizeof(float) * 3;
        ilKey.elements[ilKey.elementCount].desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        ilKey.elements[ilKey.elementCount].desc.InstanceDataStepRate = 0;

        ilKey.elementCount++;
        nextAvailableInputSlot++;

        // Add index mapping buffer.  This buffer will be a per instance value if indexed points
        // are being rendered, othewise it will be treated as a per vertex data.  The shader 
        // will read this value to determine the point needs to be rendered or not.
        // 0 = render point, 1 = do not render (render off screen)
        ilKey.elements[ilKey.elementCount].desc.SemanticName = "SPRITEENABLEMAP";
        ilKey.elements[ilKey.elementCount].desc.SemanticIndex = 0;
        ilKey.elements[ilKey.elementCount].desc.Format = DXGI_FORMAT_R32_FLOAT;
        ilKey.elements[ilKey.elementCount].desc.InputSlot = nextAvailableInputSlot;
        ilKey.elements[ilKey.elementCount].desc.AlignedByteOffset = 0;
        ilKey.elements[ilKey.elementCount].desc.InputSlotClass = indexedPointSpritesActive ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
        ilKey.elements[ilKey.elementCount].desc.InstanceDataStepRate = 0;

        if (ilKey.elements[ilKey.elementCount].desc.InputSlotClass == D3D11_INPUT_PER_INSTANCE_DATA && firstInstancedElement == gl::MAX_VERTEX_ATTRIBS)
        {
            firstInstancedElement = ilKey.elementCount;
        }

        ilKey.elementCount++;
    }

    // On 9_3, we must ensure that slot 0 contains non-instanced data.
    // If slot 0 currently contains instanced data then we swap it with a non-instanced element.
    // Note that instancing is only available on 9_3 via ANGLE_instanced_arrays, since 9_3 doesn't support OpenGL ES 3.0.
    // As per the spec for ANGLE_instanced_arrays, not all attributes can be instanced simultaneously, so a non-instanced element must exist.
    ASSERT(!(mFeatureLevel <= D3D_FEATURE_LEVEL_9_3 && firstIndexedElement == gl::MAX_VERTEX_ATTRIBS));
    bool moveFirstIndexedIntoSlotZero = mFeatureLevel <= D3D_FEATURE_LEVEL_9_3 && firstInstancedElement == 0 && firstIndexedElement != gl::MAX_VERTEX_ATTRIBS;

    if (moveFirstIndexedIntoSlotZero)
    {
        ilKey.elements[firstInstancedElement].desc.InputSlot = ilKey.elements[firstIndexedElement].desc.InputSlot;
        ilKey.elements[firstIndexedElement].desc.InputSlot = 0;

        // Instanced PointSprite emulation uses multiple layout entries across a single vertex buffer.
        // If an index swap is performed, we need to ensure that all elements get the proper InputSlot.
        if (programUsesInstancedPointSprites)
        {
            ilKey.elements[firstIndexedElement + 1].desc.InputSlot = 0;
        }
    }

    ID3D11InputLayout *inputLayout = NULL;

    InputLayoutMap::iterator keyIter = mInputLayoutMap.find(ilKey);
    if (keyIter != mInputLayoutMap.end())
    {
        inputLayout = keyIter->second.inputLayout;
        keyIter->second.lastUsedTime = mCounter++;
    }
    else
    {
        gl::VertexFormat shaderInputLayout[gl::MAX_VERTEX_ATTRIBS];
        GetInputLayout(attributes, shaderInputLayout);

        ShaderExecutableD3D *shader = NULL;
        gl::Error error = programD3D->getVertexExecutableForInputLayout(shaderInputLayout, &shader, nullptr);
        if (error.isError())
        {
            return error;
        }

        ShaderExecutableD3D *shader11 = ShaderExecutable11::makeShaderExecutable11(shader);

        D3D11_INPUT_ELEMENT_DESC descs[gl::MAX_VERTEX_ATTRIBS];
        for (unsigned int j = 0; j < ilKey.elementCount; ++j)
        {
            descs[j] = ilKey.elements[j].desc;
        }

        HRESULT result = mDevice->CreateInputLayout(descs, ilKey.elementCount, shader11->getFunction(), shader11->getLength(), &inputLayout);
        if (FAILED(result))
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal input layout, HRESULT: 0x%08x", result);
        }

        if (mInputLayoutMap.size() >= kMaxInputLayouts)
        {
            TRACE("Overflowed the limit of %u input layouts, removing the least recently used "
                  "to make room.", kMaxInputLayouts);

            InputLayoutMap::iterator leastRecentlyUsed = mInputLayoutMap.begin();
            for (InputLayoutMap::iterator i = mInputLayoutMap.begin(); i != mInputLayoutMap.end(); i++)
            {
                if (i->second.lastUsedTime < leastRecentlyUsed->second.lastUsedTime)
                {
                    leastRecentlyUsed = i;
                }
            }
            SafeRelease(leastRecentlyUsed->second.inputLayout);
            mInputLayoutMap.erase(leastRecentlyUsed);
        }

        InputLayoutCounterPair inputCounterPair;
        inputCounterPair.inputLayout = inputLayout;
        inputCounterPair.lastUsedTime = mCounter++;

        mInputLayoutMap.insert(std::make_pair(ilKey, inputCounterPair));
    }

    if (inputLayout != mCurrentIL)
    {
        mDeviceContext->IASetInputLayout(inputLayout);
        mCurrentIL = inputLayout;
    }

    bool dirtyBuffers = false;
    size_t minDiff = gl::MAX_VERTEX_ATTRIBS;
    size_t maxDiff = 0;
    unsigned int nextAvailableIndex = 0;

    for (unsigned int i = 0; i < gl::MAX_VERTEX_ATTRIBS; i++)
    {
        ID3D11Buffer *buffer = NULL;

        if (attributes[i].active)
        {
            VertexBuffer11 *vertexBuffer = VertexBuffer11::makeVertexBuffer11(attributes[i].vertexBuffer);
            Buffer11 *bufferStorage = attributes[i].storage ? Buffer11::makeBuffer11(attributes[i].storage) : NULL;

            buffer = bufferStorage ? bufferStorage->getBuffer(BUFFER_USAGE_VERTEX_OR_TRANSFORM_FEEDBACK)
                                   : vertexBuffer->getBuffer();
        }

        UINT vertexStride = attributes[i].stride;
        UINT vertexOffset = attributes[i].offset;

        if (buffer != mCurrentBuffers[i] || vertexStride != mCurrentVertexStrides[i] ||
            vertexOffset != mCurrentVertexOffsets[i])
        {
            dirtyBuffers = true;
            minDiff = std::min(minDiff, static_cast<size_t>(i));
            maxDiff = std::max(maxDiff, static_cast<size_t>(i));

            mCurrentBuffers[i] = buffer;
            mCurrentVertexStrides[i] = vertexStride;
            mCurrentVertexOffsets[i] = indexedPointSpritesActive ? 0 : vertexOffset;

            // If a non null ID3D11Buffer is being assigned to mCurrentBuffers,
            // then the next available index needs to be tracked to ensure 
            // that any instanced pointsprite emulation buffers will be properly packed.
            if (buffer)
            {
                nextAvailableIndex = i + 1;
            }
        }
    }

    // Instanced PointSprite emulation requires two additional ID3D11Buffers.
    // A vertex buffer needs to be created and added to the list of current buffers, 
    // strides and offsets collections.  This buffer contains the vertices for a single
    // PointSprite quad.
    // An index buffer also needs to be created and applied because rendering instanced
    // data on D3D11 FL9_3 requires DrawIndexedInstanced() to be used.
    if (instancedPointSpritesActive)
    {
        HRESULT result = S_OK;
        const UINT pointSpriteVertexStride = sizeof(float) * 5;

        if (!mPointSpriteVertexBuffer)
        {
            static const float pointSpriteVertices[] =
            {
                // Position        // TexCoord
               -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
               -1.0f,  1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
               -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
            };

            D3D11_SUBRESOURCE_DATA vertexBufferData = { pointSpriteVertices, 0, 0 };
            D3D11_BUFFER_DESC vertexBufferDesc;
            vertexBufferDesc.ByteWidth = sizeof(pointSpriteVertices);
            vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
            vertexBufferDesc.CPUAccessFlags = 0;
            vertexBufferDesc.MiscFlags = 0;
            vertexBufferDesc.StructureByteStride = 0;

            result = mDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &mPointSpriteVertexBuffer);
            if (FAILED(result))
            {
                return gl::Error(GL_OUT_OF_MEMORY, "Failed to create instanced pointsprite emulation vertex buffer, HRESULT: 0x%08x", result);
            }
        }

        mCurrentBuffers[nextAvailableIndex] = mPointSpriteVertexBuffer;
        mCurrentVertexStrides[nextAvailableIndex] = pointSpriteVertexStride;
        mCurrentVertexOffsets[nextAvailableIndex] = 0;
        nextAvailableIndex++;

        if (!mPointSpriteIndexBuffer)
        {
            // Create an index buffer and set it for pointsprite rendering
            static const unsigned short pointSpriteIndices[] =
            {
                0, 1, 2, 3, 4, 5,
            };

            D3D11_SUBRESOURCE_DATA indexBufferData = { pointSpriteIndices, 0, 0 };
            D3D11_BUFFER_DESC indexBufferDesc;
            indexBufferDesc.ByteWidth = sizeof(pointSpriteIndices);
            indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
            indexBufferDesc.CPUAccessFlags = 0;
            indexBufferDesc.MiscFlags = 0;
            indexBufferDesc.StructureByteStride = 0;

            result = mDevice->CreateBuffer(&indexBufferDesc, &indexBufferData, &mPointSpriteIndexBuffer);
            if (FAILED(result))
            {
                SafeRelease(mPointSpriteVertexBuffer);
                return gl::Error(GL_OUT_OF_MEMORY, "Failed to create instanced pointsprite emulation index buffer, HRESULT: 0x%08x", result);
            }
        }

        // The index buffer is applied here because Instanced PointSprite emulation uses
        // this buffer to render an instanced indexed quad.
        // If indexed point points are being rendered, this index buffer will replace the
        // current index buffer set previously in applyIndexBuffer(). The caller's specified
        // index buffer will still be referred to in the enable/disable point sprite
        // logic below.
        // Setting this buffer here ensures that the rendering path will contain the correct index buffer
        // for rendering an instanced point sprite.
        mDeviceContext->IASetIndexBuffer(mPointSpriteIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

        // If an index buffer was applied and has changed the sprite enable mapping buffer
        // must also be invalidated.  This ensures that a new enable buffer is created that matches
        // the currently applied index buffer.
        if (appliedIndexBufferChanged)
        {
            SafeRelease(mIndexedPointSpriteEnableBuffer);
        }

        // Indexed PointSprite emulation leverages the existing Instanced PointSprite rendering path with an
        // additional ID3D11Buffer that contains on/off values enabling/disabling points to be rendered.
        // The enable/disable state is configured by reading the caller's supplied index buffer.
        //
        // This code path is shared between glDrawArrays( ) and glDrawElements( ).
        //
        // The glDrawArrays path uses the point mapping buffers as a per-vertex
        // buffer.  All values in the mapping buffer are set to an enabled state.
        //
        // The glDrawElements path uses the point enable buffers as a per instance buffer.
        // The caller's specified index buffer is read and translated into the point mapping
        // buffer by setting a value to 0 (meaning the point is to be rendered) or
        // or 1 (meaning the point is not to be rendered).
        // 0 is chosed to represent enabled because it simplifies the shader implementation
        // by eliminating the need for a branch (if statement) for determining if the point should be
        // rendered or not.  Branches in shaders are expensive and non performant.
        // Create a enable buffer if one is not currently available to use.
        if (!mIndexedPointSpriteEnableBuffer)
        {
            // glDrawArrays is a more common rendering path for rendering points. For this case,
            // the mapping buffer will contain 6 entries that indicated that all points
            // of the instanced quad should be rendered.
            std::vector<float> indexedPointSpriteEnable(6, 0.0f);

            // If indexed points are being rendered (via glDrawElements), then the enable buffer must be updated
            // to reflect the contents of the supplied index buffer.
            if (indexedPointSpritesActive)
            {
                // Resize the indexed pointsprite enable buffer to be the total number of point instances possible to render.
                // All points must be considered because of reuse of the instanced pointsprite rendering path.
                // The shader will cull points that should not be rendered using this enabled buffer.
                indexedPointSpriteEnable.resize(indexedPointSpriteCount);
                configureIndexedPointSpriteEnableBuffer(indexedPointSpriteEnable, indexInfo);
            }

            D3D11_SUBRESOURCE_DATA indexBufferData = { indexedPointSpriteEnable.data(), 0, 0 };
            D3D11_BUFFER_DESC indexBufferDesc;
            indexBufferDesc.ByteWidth = indexedPointSpriteEnable.size() * sizeof(float);
            indexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
            indexBufferDesc.CPUAccessFlags = 0;
            indexBufferDesc.MiscFlags = 0;
            indexBufferDesc.StructureByteStride = 0;

            result = mDevice->CreateBuffer(&indexBufferDesc, &indexBufferData, &mIndexedPointSpriteEnableBuffer);
            if (FAILED(result))
            {
                SafeRelease(mPointSpriteIndexBuffer);
                SafeRelease(mPointSpriteVertexBuffer);
                return gl::Error(GL_OUT_OF_MEMORY, "Failed to create indexed pointsprite emulation enable buffer, HRESULT: 0x%08x", result);
            }
        }

        mCurrentBuffers[nextAvailableIndex] = mIndexedPointSpriteEnableBuffer;
        mCurrentVertexStrides[nextAvailableIndex] = sizeof(float);
        mCurrentVertexOffsets[nextAvailableIndex] = 0;
    }

    nextAvailableIndex++;

    if (moveFirstIndexedIntoSlotZero)
    {
        // In this case, we swapped the slots of the first instanced element and the first indexed element, to ensure
        // that the first slot contains non-instanced data (required by Feature Level 9_3).
        // We must also swap the corresponding buffers sent to IASetVertexBuffers so that the correct data is sent to each slot.
        std::swap(mCurrentBuffers[firstIndexedElement], mCurrentBuffers[firstInstancedElement]);
        std::swap(mCurrentVertexStrides[firstIndexedElement], mCurrentVertexStrides[firstInstancedElement]);
        std::swap(mCurrentVertexOffsets[firstIndexedElement], mCurrentVertexOffsets[firstInstancedElement]);
    }

    if (dirtyBuffers)
    {
        ASSERT(minDiff <= maxDiff && maxDiff < gl::MAX_VERTEX_ATTRIBS);
        mDeviceContext->IASetVertexBuffers(minDiff, maxDiff - minDiff + 1, mCurrentBuffers + minDiff,
                                           mCurrentVertexStrides + minDiff, mCurrentVertexOffsets + minDiff);
    }

    return gl::Error(GL_NO_ERROR);
}

std::size_t InputLayoutCache::hashInputLayout(const InputLayoutKey &inputLayout)
{
    static const unsigned int seed = 0xDEADBEEF;

    std::size_t hash = 0;
    MurmurHash3_x86_32(inputLayout.begin(), inputLayout.end() - inputLayout.begin(), seed, &hash);
    return hash;
}

bool InputLayoutCache::compareInputLayouts(const InputLayoutKey &a, const InputLayoutKey &b)
{
    if (a.elementCount != b.elementCount)
    {
        return false;
    }

    return std::equal(a.begin(), a.end(), b.begin());
}

}
