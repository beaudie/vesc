//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// IndexDataManager.h: Defines the IndexDataManager, a class that
// runs the Buffer translation process for index buffers.

#ifndef LIBANGLE_INDEXDATAMANAGER_H_
#define LIBANGLE_INDEXDATAMANAGER_H_

#include <GLES2/gl2.h>

#include "common/angleutils.h"
#include "common/mathutil.h"
#include "libANGLE/Error.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"

namespace
{
enum
{
    INITIAL_INDEX_BUFFER_SIZE = 4096 * sizeof(GLuint)
};
}

namespace gl
{
class Buffer;
}

namespace rx
{
class IndexBufferInterface;
class StaticIndexBufferInterface;
class StreamingIndexBufferInterface;
class IndexBuffer;
class BufferD3D;
class RendererD3D;

struct SourceIndexData
{
    BufferD3D *srcBuffer;
    const void *srcIndices;
    unsigned int srcCount;
    GLenum srcIndexType;
    bool srcIndicesChanged;
};

struct TranslatedIndexData
{
    unsigned int startIndex;
    unsigned int startOffset;  // In bytes

    IndexBuffer *indexBuffer;
    BufferD3D *storage;
    GLenum indexType;
    unsigned int serial;

    SourceIndexData srcIndexData;
};

class IndexDataManager : angle::NonCopyable
{
  public:
    explicit IndexDataManager(BufferFactoryD3D *factory);
    virtual ~IndexDataManager();

    void deinitialize();

    static bool IsStreamingIndexData(const gl::Context *context,
                                     GLenum srcType,
                                     RendererClass rendererClass);
    gl::Error prepareIndexData(const gl::Context *context,
                               GLenum srcType,
                               GLenum dstType,
                               GLsizei count,
                               gl::Buffer *glBuffer,
                               const void *indices,
                               TranslatedIndexData *translated);

  private:
    gl::Error streamIndexData(const void *data,
                              unsigned int count,
                              GLenum srcType,
                              GLenum dstType,
                              bool usePrimitiveRestartFixedIndex,
                              TranslatedIndexData *translated);
    gl::Error getStreamingIndexBuffer(GLenum destinationIndexType,
                                      IndexBufferInterface **outBuffer);

    using StreamingBuffer = std::unique_ptr<StreamingIndexBufferInterface>;

    BufferFactoryD3D *const mFactory;
    std::unique_ptr<StreamingIndexBufferInterface> mStreamingBufferShort;
    std::unique_ptr<StreamingIndexBufferInterface> mStreamingBufferInt;
};

GLenum GetIndexTranslationDestType(GLenum srcType,
                                   const gl::HasIndexRange &lazyIndexRange,
                                   bool usePrimitiveRestartWorkaround);

bool UsePrimitiveRestartWorkaround(bool primitiveRestartFixedIndexEnabled,
                                   GLenum type,
                                   RendererClass rendererClass);

enum class IndexStorageType
{
    // Dynamic indexes are re-streamed every frame. They come from a client data pointer or
    // from buffers that are updated frequently.
    Dynamic,

    // Static indexes are translated from the original storage once, and re-used multiple times.
    Static,

    // Direct indexes are never transated and are used directly from the source buffer. They are
    // the fastest available path.
    Direct,

    // Not a real storage type.
    Invalid,
};

IndexStorageType ClassifyIndexStorage(const gl::State &glState,
                                      const gl::Buffer *elementArrayBuffer,
                                      GLenum elementType,
                                      GLenum destElementType,
                                      const void *indices,
                                      bool *needsTranslation);

}  // namespace rx

#endif  // LIBANGLE_INDEXDATAMANAGER_H_
