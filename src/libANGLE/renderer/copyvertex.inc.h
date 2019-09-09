//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// copyvertex.inc.h: Implementation of vertex buffer copying and conversion functions

namespace rx
{

template <typename T,
          size_t inputComponentCount,
          size_t outputComponentCount,
          uint32_t alphaDefaultValueBits>
inline void CopyNativeVertexData(const uint8_t *input, size_t stride, size_t count, uint8_t *output)
{
    const size_t attribSize = sizeof(T) * inputComponentCount;

    if (attribSize == stride && inputComponentCount == outputComponentCount)
    {
        memcpy(output, input, count * attribSize);
        return;
    }

    if (inputComponentCount == outputComponentCount)
    {
        for (size_t i = 0; i < count; i++)
        {
            const T *offsetInput = reinterpret_cast<const T *>(input + (i * stride));
            T *offsetOutput      = reinterpret_cast<T *>(output) + i * outputComponentCount;

            memcpy(offsetOutput, offsetInput, attribSize);
        }
        return;
    }

    const T defaultAlphaValue                = gl::bitCast<T>(alphaDefaultValueBits);
    const size_t lastNonAlphaOutputComponent = std::min<size_t>(outputComponentCount, 3);

    for (size_t i = 0; i < count; i++)
    {
        const T *offsetInput = reinterpret_cast<const T *>(input + (i * stride));
        T *offsetOutput      = reinterpret_cast<T *>(output) + i * outputComponentCount;

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
inline void Copy8SintTo16SintVertexData(const uint8_t *input,
                                        size_t stride,
                                        size_t count,
                                        uint8_t *output)
{
    const size_t lastNonAlphaOutputComponent = std::min<size_t>(outputComponentCount, 3);

    for (size_t i = 0; i < count; i++)
    {
        const GLbyte *offsetInput = reinterpret_cast<const GLbyte *>(input + i * stride);
        GLshort *offsetOutput     = reinterpret_cast<GLshort *>(output) + i * outputComponentCount;

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
inline void Copy8SnormTo16SnormVertexData(const uint8_t *input,
                                          size_t stride,
                                          size_t count,
                                          uint8_t *output)
{
    for (size_t i = 0; i < count; i++)
    {
        const GLbyte *offsetInput = reinterpret_cast<const GLbyte *>(input + i * stride);
        GLshort *offsetOutput     = reinterpret_cast<GLshort *>(output) + i * outputComponentCount;

        for (size_t j = 0; j < inputComponentCount; j++)
        {
            // The original GLbyte value ranges from -128 to +127 (INT8_MAX).
            // When converted to GLshort, the value must be scaled to between -32768 and +32767
            // (INT16_MAX).
            if (offsetInput[j] > 0)
            {
                offsetOutput[j] =
                    offsetInput[j] << 8 | offsetInput[j] << 1 | ((offsetInput[j] & 0x40) >> 6);
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
inline void Copy32FixedTo32FVertexData(const uint8_t *input,
                                       size_t stride,
                                       size_t count,
                                       uint8_t *output)
{
    static const float divisor = 1.0f / (1 << 16);

    for (size_t i = 0; i < count; i++)
    {
        const uint8_t *offsetInput = input + i * stride;
        float *offsetOutput        = reinterpret_cast<float *>(output) + i * outputComponentCount;

        // GLfixed access must be 4-byte aligned on arm32, input and stride sometimes are not
        if (reinterpret_cast<uintptr_t>(offsetInput) % sizeof(GLfixed) == 0)
        {
            for (size_t j = 0; j < inputComponentCount; j++)
            {
                offsetOutput[j] =
                    static_cast<float>(reinterpret_cast<const GLfixed *>(offsetInput)[j]) * divisor;
            }
        }
        else
        {
            for (size_t j = 0; j < inputComponentCount; j++)
            {
                GLfixed alignedInput;
                memcpy(&alignedInput, offsetInput + j * sizeof(GLfixed), sizeof(GLfixed));
                offsetOutput[j] = static_cast<float>(alignedInput) * divisor;
            }
        }

        // 4-component output formats would need special padding in the alpha channel.
        static_assert(!(inputComponentCount < 4 && outputComponentCount == 4),
                      "An inputComponentCount less than 4 and an outputComponentCount equal to 4 "
                      "is not supported.");

        for (size_t j = inputComponentCount; j < outputComponentCount; j++)
        {
            offsetOutput[j] = 0.0f;
        }
    }
}

namespace priv
{

// snorm -> float conversion function as per GLES2.0 spec
template <typename T>
inline void CopyNormToFloatScale(const T *input, GLfloat *output, T max)
{
    if (std::is_signed<T>::value)
    {
        *output = static_cast<GLfloat>(2 * (*input) + 1) / static_cast<GLfloat>(2 * max + 1);
    }
    else
    {
        *output = static_cast<GLfloat>(*input) / static_cast<GLfloat>(max);
    }
}

// snorm -> float conversion function as per GLES3.0 spec
template <typename T>
inline void CopyNormToFloatClamp(const T *input, GLfloat *output, T max)
{
    static const GLfloat min = static_cast<GLfloat>(-1.0f);
    *output                  = static_cast<GLfloat>(*input) / static_cast<GLfloat>(max);

    // GLES normalization rules dictate that it should be clamped to -1.0f in this case.
    if (*output < min)
    {
        *output = min;
    }
}

template <bool isSigned, bool normalized, bool toFloat, bool isGLES20>
inline void CopyPackedRGB(uint32_t data, uint8_t *output)
{
    const uint32_t rgbSignMask  = 0x200;       // 1 set at the 9 bit
    const uint32_t negativeMask = 0xFFFFFC00;  // All bits from 10 to 31 set to 1

    if (toFloat)
    {
        GLfloat *floatOutput = reinterpret_cast<GLfloat *>(output);
        if (isSigned)
        {
            int32_t finalValue = static_cast<int32_t>(data);
            if (finalValue & rgbSignMask)
            {
                finalValue |= negativeMask;
            }

            if (normalized)
            {
                const int32_t maxValue = 0x1FF;  // 1 set in bits 0 through 8
                if (isGLES20)
                {
                    CopyNormToFloatScale<int32_t>(&finalValue, floatOutput, maxValue);
                }
                else
                {
                    CopyNormToFloatClamp<int32_t>(&finalValue, floatOutput, maxValue);
                }
            }
            else
            {
                *floatOutput = static_cast<GLfloat>(finalValue);
            }
        }
        else
        {
            if (normalized)
            {
                const uint32_t maxValue = 0x3FF;  // 1 set in bits 0 through 9
                if (isGLES20)
                {
                    CopyNormToFloatScale<uint32_t>(&data, floatOutput, maxValue);
                }
                else
                {
                    CopyNormToFloatClamp<uint32_t>(&data, floatOutput, maxValue);
                }
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
            GLshort *intOutput = reinterpret_cast<GLshort *>(output);

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
            GLushort *uintOutput = reinterpret_cast<GLushort *>(output);
            *uintOutput          = static_cast<GLushort>(data);
        }
    }
}

template <bool isSigned, bool normalized, bool toFloat, bool isGLES20>
inline void CopyPackedAlpha(uint32_t data, uint8_t *output)
{
    if (toFloat)
    {
        GLfloat *floatOutput = reinterpret_cast<GLfloat *>(output);
        if (isSigned)
        {
            if (normalized)
            {
                int32_t inputValue = data;

                constexpr uint32_t signMask     = 0x2;         // 1 set at the 1th bit
                constexpr uint32_t negativeMask = 0xFFFFFFFC;  // All bits from 2 to 31 set to 1

                if (inputValue & signMask)
                {
                    inputValue |= negativeMask;
                }

                constexpr int32_t maxValue = 0x1;
                if (isGLES20)
                {
                    CopyNormToFloatScale<int32_t>(reinterpret_cast<int32_t *>(&inputValue),
                                                  floatOutput, maxValue);
                }
                else
                {
                    CopyNormToFloatClamp<int32_t>(reinterpret_cast<int32_t *>(&inputValue),
                                                  floatOutput, maxValue);
                }
            }
            else
            {
                switch (data)
                {
                    case 0x0:
                        *floatOutput = 0.0f;
                        break;
                    case 0x1:
                        *floatOutput = 1.0f;
                        break;
                    case 0x2:
                        *floatOutput = -2.0f;
                        break;
                    case 0x3:
                        *floatOutput = -1.0f;
                        break;
                    default:
                        UNREACHABLE();
                }
            }
        }
        else
        {
            if (normalized)
            {
                constexpr uint32_t maxValue = 0x3;
                if (isGLES20)
                {
                    CopyNormToFloatScale<uint32_t>(&data, floatOutput, maxValue);
                }
                else
                {
                    CopyNormToFloatClamp<uint32_t>(&data, floatOutput, maxValue);
                }
            }
            else
            {
                switch (data)
                {
                    case 0x0:
                        *floatOutput = 0.0f;
                        break;
                    case 0x1:
                        *floatOutput = 1.0f;
                        break;
                    case 0x2:
                        *floatOutput = 2.0f;
                        break;
                    case 0x3:
                        *floatOutput = 3.0f;
                        break;
                    default:
                        UNREACHABLE();
                }
            }
        }
    }
    else
    {
        if (isSigned)
        {
            GLshort *intOutput = reinterpret_cast<GLshort *>(output);
            switch (data)
            {
                case 0x0:
                    *intOutput = 0;
                    break;
                case 0x1:
                    *intOutput = 1;
                    break;
                case 0x2:
                    *intOutput = -2;
                    break;
                case 0x3:
                    *intOutput = -1;
                    break;
                default:
                    UNREACHABLE();
            }
        }
        else
        {
            GLushort *uintOutput = reinterpret_cast<GLushort *>(output);
            switch (data)
            {
                case 0x0:
                    *uintOutput = 0;
                    break;
                case 0x1:
                    *uintOutput = 1;
                    break;
                case 0x2:
                    *uintOutput = 2;
                    break;
                case 0x3:
                    *uintOutput = 3;
                    break;
                default:
                    UNREACHABLE();
            }
        }
    }
}

}  // namespace priv

template <typename T, size_t inputComponentCount, size_t outputComponentCount, bool normalized>
inline void CopyTo32FVertexData(const uint8_t *input, size_t stride, size_t count, uint8_t *output)
{
    typedef std::numeric_limits<T> NL;

    for (size_t i = 0; i < count; i++)
    {
        const T *offsetInput = reinterpret_cast<const T *>(input + (stride * i));
        float *offsetOutput  = reinterpret_cast<float *>(output) + i * outputComponentCount;

        for (size_t j = 0; j < inputComponentCount; j++)
        {
            if (normalized)
            {
                priv::CopyNormToFloatClamp<T>(offsetInput, offsetOutput, NL::max());
            }
            else
            {
                offsetOutput[j] = static_cast<float>(offsetInput[j]);
            }
        }

        // This would require special padding.
        static_assert(!(inputComponentCount < 4 && outputComponentCount == 4),
                      "An inputComponentCount less than 4 and an outputComponentCount equal to 4 "
                      "is not supported.");

        for (size_t j = inputComponentCount; j < outputComponentCount; j++)
        {
            offsetOutput[j] = 0.0f;
        }
    }
}

template <bool isSigned, bool normalized, bool toFloat>
inline void CopyXYZ10W2ToXYZW32FVertexData(const uint8_t *input,
                                           size_t stride,
                                           size_t count,
                                           uint8_t *output)
{
    const size_t outputComponentSize = toFloat ? 4 : 2;
    const size_t componentCount      = 4;

    const uint32_t rgbMask  = 0x3FF;  // 1 set in bits 0 through 9
    const size_t redShift   = 0;      // red is bits 0 through 9
    const size_t greenShift = 10;     // green is bits 10 through 19
    const size_t blueShift  = 20;     // blue is bits 20 through 29

    const uint32_t alphaMask = 0x3;  // 1 set in bits 0 and 1
    const size_t alphaShift  = 30;   // Alpha is the 30 and 31 bits

    for (size_t i = 0; i < count; i++)
    {
        GLuint packedValue    = *reinterpret_cast<const GLuint *>(input + (i * stride));
        uint8_t *offsetOutput = output + (i * outputComponentSize * componentCount);

        priv::CopyPackedRGB<isSigned, normalized, toFloat, false>(
            (packedValue >> redShift) & rgbMask, offsetOutput + (0 * outputComponentSize));
        priv::CopyPackedRGB<isSigned, normalized, toFloat, false>(
            (packedValue >> greenShift) & rgbMask, offsetOutput + (1 * outputComponentSize));
        priv::CopyPackedRGB<isSigned, normalized, toFloat, false>(
            (packedValue >> blueShift) & rgbMask, offsetOutput + (2 * outputComponentSize));
        priv::CopyPackedAlpha<isSigned, normalized, toFloat, false>(
            (packedValue >> alphaShift) & alphaMask, offsetOutput + (3 * outputComponentSize));
    }
}

template <typename T, size_t inputComponentCount, size_t outputComponentCount, bool normalized>
inline void CopyTo32FVertexDataForGLES20(const uint8_t *input,
                                         size_t stride,
                                         size_t count,
                                         uint8_t *output)
{
    typedef std::numeric_limits<T> NL;

    for (size_t i = 0; i < count; i++)
    {
        const T *offsetInput = reinterpret_cast<const T *>(input + (stride * i));
        float *offsetOutput  = reinterpret_cast<float *>(output) + i * outputComponentCount;

        for (size_t j = 0; j < inputComponentCount; j++)
        {
            if (normalized)
            {
                priv::CopyNormToFloatScale<T>(offsetInput, offsetOutput, NL::max());
            }
            else
            {
                offsetOutput[j] = static_cast<float>(offsetInput[j]);
            }
        }

        // This would require special padding.
        static_assert(!(inputComponentCount < 4 && outputComponentCount == 4),
                      "An inputComponentCount less than 4 and an outputComponentCount equal to 4 "
                      "is not supported.");

        for (size_t j = inputComponentCount; j < outputComponentCount; j++)
        {
            offsetOutput[j] = 0.0f;
        }
    }
}

template <bool isSigned, bool normalized, bool toFloat>
inline void CopyXYZ10W2ToXYZW32FVertexDataForGLES20(const uint8_t *input,
                                                    size_t stride,
                                                    size_t count,
                                                    uint8_t *output)
{
    const size_t outputComponentSize = toFloat ? 4 : 2;
    const size_t componentCount      = 4;

    const uint32_t rgbMask  = 0x3FF;  // 1 set in bits 0 through 9
    const size_t redShift   = 0;      // red is bits 0 through 9
    const size_t greenShift = 10;     // green is bits 10 through 19
    const size_t blueShift  = 20;     // blue is bits 20 through 29

    const uint32_t alphaMask = 0x3;  // 1 set in bits 0 and 1
    const size_t alphaShift  = 30;   // Alpha is the 30 and 31 bits

    for (size_t i = 0; i < count; i++)
    {
        GLuint packedValue    = *reinterpret_cast<const GLuint *>(input + (i * stride));
        uint8_t *offsetOutput = output + (i * outputComponentSize * componentCount);

        priv::CopyPackedRGB<isSigned, normalized, toFloat, true>(
            (packedValue >> redShift) & rgbMask, offsetOutput + (0 * outputComponentSize));
        priv::CopyPackedRGB<isSigned, normalized, toFloat, true>(
            (packedValue >> greenShift) & rgbMask, offsetOutput + (1 * outputComponentSize));
        priv::CopyPackedRGB<isSigned, normalized, toFloat, true>(
            (packedValue >> blueShift) & rgbMask, offsetOutput + (2 * outputComponentSize));
        priv::CopyPackedAlpha<isSigned, normalized, toFloat, true>(
            (packedValue >> alphaShift) & alphaMask, offsetOutput + (3 * outputComponentSize));
    }
}

}  // namespace rx
