//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// renderer_utils:
//   Helper methods pertaining to most or all back-ends.
//

#ifndef LIBANGLE_RENDERER_RENDERER_UTILS_H_
#define LIBANGLE_RENDERER_RENDERER_UTILS_H_

#include <cstdint>

#include <limits>
#include <map>

#include "common/angleutils.h"
#include "libANGLE/angletypes.h"

namespace angle
{
struct Format;
}  // namespace angle

namespace gl
{
struct FormatType;
struct InternalFormat;
}  // namespace gl

namespace egl
{
class AttributeMap;
}  // namespace egl

namespace rx
{

class ResourceSerial
{
  public:
    constexpr ResourceSerial() : mValue(kDirty) {}
    explicit constexpr ResourceSerial(uintptr_t value) : mValue(value) {}
    constexpr bool operator==(ResourceSerial other) const { return mValue == other.mValue; }
    constexpr bool operator!=(ResourceSerial other) const { return mValue != other.mValue; }

    void dirty() { mValue = kDirty; }
    void clear() { mValue = kEmpty; }

    constexpr bool valid() const { return mValue != kEmpty && mValue != kDirty; }
    constexpr bool empty() const { return mValue == kEmpty; }

  private:
    constexpr static uintptr_t kDirty = std::numeric_limits<uintptr_t>::max();
    constexpr static uintptr_t kEmpty = 0;

    uintptr_t mValue;
};

class SerialFactory;

class Serial final
{
  public:
    constexpr Serial() : mValue(kInvalid) {}
    constexpr Serial(const Serial &other) = default;
    Serial &operator=(const Serial &other) = default;

    constexpr bool operator==(const Serial &other) const
    {
        return mValue != kInvalid && mValue == other.mValue;
    }
    constexpr bool operator==(uint32_t value) const
    {
        return mValue != kInvalid && mValue == static_cast<uint64_t>(value);
    }
    constexpr bool operator!=(const Serial &other) const
    {
        return mValue == kInvalid || mValue != other.mValue;
    }
    constexpr bool operator>(const Serial &other) const { return mValue > other.mValue; }
    constexpr bool operator>=(const Serial &other) const { return mValue >= other.mValue; }
    constexpr bool operator<(const Serial &other) const { return mValue < other.mValue; }
    constexpr bool operator<=(const Serial &other) const { return mValue <= other.mValue; }

    constexpr bool operator<(uint32_t value) const { return mValue < static_cast<uint64_t>(value); }

    // Useful for serialization.
    constexpr uint64_t getValue() const { return mValue; }

  private:
    friend class SerialFactory;
    constexpr explicit Serial(uint64_t value) : mValue(value) {}
    uint64_t mValue;
    static constexpr uint64_t kInvalid = 0;
};

class SerialFactory final : angle::NonCopyable
{
  public:
    SerialFactory() : mSerial(1) {}

    Serial generate()
    {
        ASSERT(mSerial != std::numeric_limits<uint64_t>::max());
        return Serial(mSerial++);
    }

  private:
    uint64_t mSerial;
};

using MipGenerationFunction = void (*)(size_t sourceWidth,
                                       size_t sourceHeight,
                                       size_t sourceDepth,
                                       const uint8_t *sourceData,
                                       size_t sourceRowPitch,
                                       size_t sourceDepthPitch,
                                       uint8_t *destData,
                                       size_t destRowPitch,
                                       size_t destDepthPitch);

typedef void (*ColorReadFunction)(const uint8_t *source, uint8_t *dest);
typedef void (*ColorWriteFunction)(const uint8_t *source, uint8_t *dest);
typedef void (*ColorCopyFunction)(const uint8_t *source, uint8_t *dest);

class FastCopyFunctionMap
{
  public:
    struct Entry
    {
        GLenum format;
        GLenum type;
        ColorCopyFunction func;
    };

    constexpr FastCopyFunctionMap() : FastCopyFunctionMap(nullptr, 0) {}

    constexpr FastCopyFunctionMap(const Entry *data, size_t size) : mSize(size), mData(data) {}

    bool has(const gl::FormatType &formatType) const;
    ColorCopyFunction get(const gl::FormatType &formatType) const;

  private:
    size_t mSize;
    const Entry *mData;
};

struct PackPixelsParams
{
    PackPixelsParams();
    PackPixelsParams(const gl::Rectangle &area,
                     GLenum format,
                     GLenum type,
                     GLuint outputPitch,
                     const gl::PixelPackState &pack,
                     gl::Buffer *packBufferIn,
                     ptrdiff_t offset);

    gl::Rectangle area;
    GLenum format;
    GLenum type;
    GLuint outputPitch;
    gl::Buffer *packBuffer;
    gl::PixelPackState pack;
    ptrdiff_t offset;
};

void PackPixels(const PackPixelsParams &params,
                const angle::Format &sourceFormat,
                int inputPitch,
                const uint8_t *source,
                uint8_t *destination);

ColorWriteFunction GetColorWriteFunction(const gl::FormatType &formatType);
ColorCopyFunction GetFastCopyFunction(const FastCopyFunctionMap &fastCopyFunctions,
                                      const gl::FormatType &formatType);

using InitializeTextureDataFunction = void (*)(size_t width,
                                               size_t height,
                                               size_t depth,
                                               uint8_t *output,
                                               size_t outputRowPitch,
                                               size_t outputDepthPitch);

using LoadImageFunction = void (*)(size_t width,
                                   size_t height,
                                   size_t depth,
                                   const uint8_t *input,
                                   size_t inputRowPitch,
                                   size_t inputDepthPitch,
                                   uint8_t *output,
                                   size_t outputRowPitch,
                                   size_t outputDepthPitch);

struct LoadImageFunctionInfo
{
    LoadImageFunctionInfo() : loadFunction(nullptr), requiresConversion(false) {}
    LoadImageFunctionInfo(LoadImageFunction loadFunction, bool requiresConversion)
        : loadFunction(loadFunction), requiresConversion(requiresConversion)
    {
    }

    LoadImageFunction loadFunction;
    bool requiresConversion;
};

using LoadFunctionMap = LoadImageFunctionInfo (*)(GLenum);

bool ShouldUseDebugLayers(const egl::AttributeMap &attribs);

void CopyImageCHROMIUM(const uint8_t *sourceData,
                       size_t sourceRowPitch,
                       size_t sourcePixelBytes,
                       ColorReadFunction readFunction,
                       uint8_t *destData,
                       size_t destRowPitch,
                       size_t destPixelBytes,
                       ColorWriteFunction colorWriteFunction,
                       GLenum destUnsizedFormat,
                       GLenum destComponentType,
                       size_t width,
                       size_t height,
                       bool unpackFlipY,
                       bool unpackPremultiplyAlpha,
                       bool unpackUnmultiplyAlpha);

// Incomplete textures are 1x1 textures filled with black, used when samplers are incomplete.
// This helper class encapsulates handling incomplete textures. Because the GL back-end
// can take advantage of the driver's incomplete textures, and because clearing multisample
// textures is so difficult, we can keep an instance of this class in the back-end instead
// of moving the logic to the Context front-end.

// This interface allows us to call-back to init a multisample texture.
class MultisampleTextureInitializer
{
  public:
    virtual ~MultisampleTextureInitializer() {}
    virtual gl::Error initializeMultisampleTextureToBlack(const gl::Context *context,
                                                          gl::Texture *glTexture) = 0;
};

class IncompleteTextureSet final : angle::NonCopyable
{
  public:
    IncompleteTextureSet();
    ~IncompleteTextureSet();

    void onDestroy(const gl::Context *context);

    gl::Error getIncompleteTexture(const gl::Context *context,
                                   gl::TextureType type,
                                   MultisampleTextureInitializer *multisampleInitializer,
                                   gl::Texture **textureOut);

  private:
    gl::TextureMap mIncompleteTextures;
};

//#include "common/mathutil.h"

typedef void (*VertexCopyFunction)(const uint8_t *input, size_t stride, size_t count, uint8_t *output);

template <typename T, size_t inputComponentCount, size_t outputComponentCount, uint32_t alphaDefaultValueBits>
inline void CopyNativeVertexData(const uint8_t *input, size_t stride, size_t count, uint8_t *output)
{
    const size_t attribSize = sizeof(T)* inputComponentCount;

    if (attribSize == stride && inputComponentCount == outputComponentCount)
    {
        memcpy(output, input, count * attribSize);
        return;
    }

    if (inputComponentCount == outputComponentCount)
    {
        for (size_t i = 0; i < count; i++)
        {
            const T *offsetInput = reinterpret_cast<const T*>(input + (i * stride));
            T *offsetOutput = reinterpret_cast<T*>(output) + i * outputComponentCount;

            memcpy(offsetOutput, offsetInput, attribSize);
        }
        return;
    }

    const T defaultAlphaValue = gl::bitCast<T>(alphaDefaultValueBits);
    const size_t lastNonAlphaOutputComponent = std::min<size_t>(outputComponentCount, 3);

    for (size_t i = 0; i < count; i++)
    {
        const T *offsetInput = reinterpret_cast<const T*>(input + (i * stride));
        T *offsetOutput = reinterpret_cast<T*>(output) + i * outputComponentCount;

        memcpy(offsetOutput, offsetInput, attribSize);

        if (inputComponentCount < lastNonAlphaOutputComponent)
        {
            // Set the remaining G/B channels to 0.
            size_t numComponents = (lastNonAlphaOutputComponent - inputComponentCount);
            memset(&offsetOutput[inputComponentCount], 0, numComponents * sizeof(T));
        }

        if (inputComponentCount < outputComponentCount && outputComponentCount == 4)
        {
            // Set the remaining alpha channel to the defaultAlphaValue.
            offsetOutput[3] = defaultAlphaValue;
        }
    }
}

template <size_t inputComponentCount, size_t outputComponentCount>
inline void Copy8SintTo16SintVertexData(const uint8_t *input, size_t stride, size_t count, uint8_t *output)
{
    const size_t lastNonAlphaOutputComponent = std::min<size_t>(outputComponentCount, 3);

    for (size_t i = 0; i < count; i++)
    {
        const GLbyte *offsetInput = reinterpret_cast<const GLbyte*>(input + i * stride);
        GLshort *offsetOutput = reinterpret_cast<GLshort*>(output)+i * outputComponentCount;

        for (size_t j = 0; j < inputComponentCount; j++)
        {
            offsetOutput[j] = static_cast<GLshort>(offsetInput[j]);
        }

        for (size_t j = inputComponentCount; j < lastNonAlphaOutputComponent; j++)
        {
            // Set remaining G/B channels to 0.
            offsetOutput[j] = 0;
        }

        if (inputComponentCount < outputComponentCount && outputComponentCount == 4)
        {
            // On integer formats, we must set the Alpha channel to 1 if it's unused.
            offsetOutput[3] = 1;
        }
    }
}

template <size_t inputComponentCount, size_t outputComponentCount>
inline void Copy8SnormTo16SnormVertexData(const uint8_t *input, size_t stride, size_t count, uint8_t *output)
{
    for (size_t i = 0; i < count; i++)
    {
        const GLbyte *offsetInput = reinterpret_cast<const GLbyte*>(input + i * stride);
        GLshort *offsetOutput = reinterpret_cast<GLshort*>(output) + i * outputComponentCount;

        for (size_t j = 0; j < inputComponentCount; j++)
        {
            // The original GLbyte value ranges from -128 to +127 (INT8_MAX).
            // When converted to GLshort, the value must be scaled to between -32768 and +32767 (INT16_MAX).
            if (offsetInput[j] > 0)
            {
                offsetOutput[j] = offsetInput[j] << 8 | offsetInput[j] << 1 | ((offsetInput[j] & 0x40) >> 6);
            }
            else
            {
                offsetOutput[j] = offsetInput[j] << 8;
            }
        }

        for (size_t j = inputComponentCount; j < std::min<size_t>(outputComponentCount, 3); j++)
        {
            // Set remaining G/B channels to 0.
            offsetOutput[j] = 0;
        }

        if (inputComponentCount < outputComponentCount && outputComponentCount == 4)
        {
            // On normalized formats, we must set the Alpha channel to the max value if it's unused.
            offsetOutput[3] = INT16_MAX;
        }
    }
}

template <size_t inputComponentCount, size_t outputComponentCount>
inline void Copy32FixedTo32FVertexData(const uint8_t *input, size_t stride, size_t count, uint8_t *output)
{
    static const float divisor = 1.0f / (1 << 16);

    for (size_t i = 0; i < count; i++)
    {
        const GLfixed* offsetInput = reinterpret_cast<const GLfixed*>(input + (stride * i));
        float* offsetOutput = reinterpret_cast<float*>(output) + i * outputComponentCount;

        for (size_t j = 0; j < inputComponentCount; j++)
        {
            offsetOutput[j] = static_cast<float>(offsetInput[j]) * divisor;
        }

        // 4-component output formats would need special padding in the alpha channel.
        static_assert(!(inputComponentCount < 4 && outputComponentCount == 4),
                      "An inputComponentCount less than 4 and an outputComponentCount equal to 4 is not supported.");

        for (size_t j = inputComponentCount; j < outputComponentCount; j++)
        {
            offsetOutput[j] = 0.0f;
        }
    }
}

template <typename T, size_t inputComponentCount, size_t outputComponentCount, bool normalized>
inline void CopyTo32FVertexData(const uint8_t *input, size_t stride, size_t count, uint8_t *output)
{
    typedef std::numeric_limits<T> NL;

    for (size_t i = 0; i < count; i++)
    {
        const T *offsetInput = reinterpret_cast<const T*>(input + (stride * i));
        float *offsetOutput = reinterpret_cast<float*>(output) + i * outputComponentCount;

        for (size_t j = 0; j < inputComponentCount; j++)
        {
            if (normalized)
            {
                if (NL::is_signed)
                {
                    const float divisor = 1.0f / (2 * static_cast<float>(NL::max()) + 1);
                    offsetOutput[j] = (2 * static_cast<float>(offsetInput[j]) + 1) * divisor;
                }
                else
                {
                    offsetOutput[j] =  static_cast<float>(offsetInput[j]) / NL::max();
                }
            }
            else
            {
                offsetOutput[j] =  static_cast<float>(offsetInput[j]);
            }
        }

        // This would require special padding.
        static_assert(!(inputComponentCount < 4 && outputComponentCount == 4),
                      "An inputComponentCount less than 4 and an outputComponentCount equal to 4 is not supported.");

        for (size_t j = inputComponentCount; j < outputComponentCount; j++)
        {
            offsetOutput[j] = 0.0f;
        }
    }
}

namespace priv
{

template <bool isSigned, bool normalized, bool toFloat>
static inline void CopyPackedRGB(uint32_t data, uint8_t *output)
{
    const uint32_t rgbSignMask = 0x200;       // 1 set at the 9 bit
    const uint32_t negativeMask = 0xFFFFFC00; // All bits from 10 to 31 set to 1

    if (toFloat)
    {
        GLfloat *floatOutput = reinterpret_cast<GLfloat*>(output);
        if (isSigned)
        {
            GLfloat finalValue = 0;
            if (data & rgbSignMask)
            {
                int negativeNumber = data | negativeMask;
                finalValue = static_cast<GLfloat>(negativeNumber);
            }
            else
            {
                finalValue = static_cast<GLfloat>(data);
            }

            if (normalized)
            {
                const int32_t maxValue = 0x1FF;      // 1 set in bits 0 through 8
                const int32_t minValue = 0xFFFFFE01; // Inverse of maxValue

                // A 10-bit two's complement number has the possibility of being minValue - 1 but
                // OpenGL's normalization rules dictate that it should be clamped to minValue in this
                // case.
                if (finalValue < minValue)
                {
                    finalValue = minValue;
                }

                const int32_t halfRange = (maxValue - minValue) >> 1;
                *floatOutput = ((finalValue - minValue) / halfRange) - 1.0f;
            }
            else
            {
                *floatOutput = finalValue;
            }
        }
        else
        {
            if (normalized)
            {
                const uint32_t maxValue = 0x3FF; // 1 set in bits 0 through 9
                *floatOutput = static_cast<GLfloat>(data) / static_cast<GLfloat>(maxValue);
            }
            else
            {
                *floatOutput = static_cast<GLfloat>(data);
            }
        }
    }
    else
    {
        if (isSigned)
        {
            GLshort *intOutput = reinterpret_cast<GLshort*>(output);

            if (data & rgbSignMask)
            {
                *intOutput = static_cast<GLshort>(data | negativeMask);
            }
            else
            {
                *intOutput = static_cast<GLshort>(data);
            }
        }
        else
        {
            GLushort *uintOutput = reinterpret_cast<GLushort*>(output);
            *uintOutput = static_cast<GLushort>(data);
        }
    }
}

template <bool isSigned, bool normalized, bool toFloat>
inline void CopyPackedAlpha(uint32_t data, uint8_t *output)
{
    if (toFloat)
    {
        GLfloat *floatOutput = reinterpret_cast<GLfloat*>(output);
        if (isSigned)
        {
            if (normalized)
            {
                switch (data)
                {
                  case 0x0: *floatOutput =  0.0f; break;
                  case 0x1: *floatOutput =  1.0f; break;
                  case 0x2: *floatOutput = -1.0f; break;
                  case 0x3: *floatOutput = -1.0f; break;
                  default: UNREACHABLE();
                }
            }
            else
            {
                switch (data)
                {
                  case 0x0: *floatOutput =  0.0f; break;
                  case 0x1: *floatOutput =  1.0f; break;
                  case 0x2: *floatOutput = -2.0f; break;
                  case 0x3: *floatOutput = -1.0f; break;
                  default: UNREACHABLE();
                }
            }
        }
        else
        {
            if (normalized)
            {
                switch (data)
                {
                  case 0x0: *floatOutput = 0.0f / 3.0f; break;
                  case 0x1: *floatOutput = 1.0f / 3.0f; break;
                  case 0x2: *floatOutput = 2.0f / 3.0f; break;
                  case 0x3: *floatOutput = 3.0f / 3.0f; break;
                  default: UNREACHABLE();
                }
            }
            else
            {
                switch (data)
                {
                  case 0x0: *floatOutput = 0.0f; break;
                  case 0x1: *floatOutput = 1.0f; break;
                  case 0x2: *floatOutput = 2.0f; break;
                  case 0x3: *floatOutput = 3.0f; break;
                  default: UNREACHABLE();
                }
            }
        }
    }
    else
    {
        if (isSigned)
        {
            GLshort *intOutput = reinterpret_cast<GLshort*>(output);
            switch (data)
            {
              case 0x0: *intOutput =  0; break;
              case 0x1: *intOutput =  1; break;
              case 0x2: *intOutput = -2; break;
              case 0x3: *intOutput = -1; break;
              default: UNREACHABLE();
            }
        }
        else
        {
            GLushort *uintOutput = reinterpret_cast<GLushort*>(output);
            switch (data)
            {
              case 0x0: *uintOutput = 0; break;
              case 0x1: *uintOutput = 1; break;
              case 0x2: *uintOutput = 2; break;
              case 0x3: *uintOutput = 3; break;
              default: UNREACHABLE();
            }
        }
    }
}

}

template <bool isSigned, bool normalized, bool toFloat>
inline void CopyXYZ10W2ToXYZW32FVertexData(const uint8_t *input, size_t stride, size_t count, uint8_t *output)
{
    const size_t outputComponentSize = toFloat ? 4 : 2;
    const size_t componentCount = 4;

    const uint32_t rgbMask = 0x3FF; // 1 set in bits 0 through 9
    const size_t redShift = 0;    // red is bits 0 through 9
    const size_t greenShift = 10; // green is bits 10 through 19
    const size_t blueShift = 20;  // blue is bits 20 through 29

    const uint32_t alphaMask = 0x3; // 1 set in bits 0 and 1
    const size_t alphaShift = 30; // Alpha is the 30 and 31 bits

    for (size_t i = 0; i < count; i++)
    {
        GLuint packedValue = *reinterpret_cast<const GLuint*>(input + (i * stride));
        uint8_t *offsetOutput = output + (i * outputComponentSize * componentCount);

        priv::CopyPackedRGB<isSigned, normalized, toFloat>(  (packedValue >> redShift)   & rgbMask,   offsetOutput + (0 * outputComponentSize));
        priv::CopyPackedRGB<isSigned, normalized, toFloat>(  (packedValue >> greenShift) & rgbMask,   offsetOutput + (1 * outputComponentSize));
        priv::CopyPackedRGB<isSigned, normalized, toFloat>(  (packedValue >> blueShift)  & rgbMask,   offsetOutput + (2 * outputComponentSize));
        priv::CopyPackedAlpha<isSigned, normalized, toFloat>((packedValue >> alphaShift) & alphaMask, offsetOutput + (3 * outputComponentSize));
    }
}

}

#endif  // LIBANGLE_RENDERER_RENDERER_UTILS_H_
