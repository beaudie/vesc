
@@#if __METAL_VERSION__ >= 240

#include "common.h"

//
// This shader contains a "pullVertexAsUInt4" function to fetch a vertex attribute and return a 4
// components vector. The returned vector is bit casted to uint4 for easier reuse. Unlike Metal
// vertex fetching API based on MTLVertexDescriptor, the vertex pulling function here can fetch data
// at unaligned offset.
//
// At runtime, when a pipeline is being compiled, we can generate a code to call a specialized version
// of this precompiled function to fetch the vertex attribute. The specialization depends on offset,
// stride, format, etc constants. For example, let's say we have vertex shader with 2 attributes:
//  - attribute1
//  - attribute2.
// When a pipline is compiled with vertex descriptor:
//  - attribute1 bound to offset=2, stride=3.
//  - attribute2 bound to offset=3, stride=4.
// We can generate two stitchable shader functions:
//  - fetchAttribute1: will call specialized "pullVertexAsUInt4" with constants (offset=2,
//  stride=3).
//  - fetchAttribute2: will call specialized "pullVertexAsUInt4" with constants (offset=3,
//  stride=4).
//
// The benefit of this is that it allows the vertex fetching code to be precompiled instead of being
// generated together with the users' shaders like transform feedback code.
//

using namespace metal;
using namespace rx::mtl_shader;

constant uint kVertexPullingType [[function_constant(2100)]];
constant uint kVertexPullingComponentCount [[function_constant(2101)]];
constant uint kVertexPullingOffset [[function_constant(2102)]];
constant uint kVertexPullingStride [[function_constant(2103)]];
constant uint kVertexPullingDivisor [[function_constant(2104)]];
constant bool kVertexPullingUseDefaultAttribs [[function_constant(2105)]];
constant bool kVertexPullingOffsetIsAligned [[function_constant(2106)]];
constant uint kVertexPullingConvertMode [[function_constant(2107)]];

constant bool kVertexPullingNormalize = kVertexPullingConvertMode == kVertexConvertModeNormalize;
constant bool kVertexPullingConvertToFloat = kVertexPullingConvertMode == kVertexConvertModeToFloat;

template <typename Short>
static inline Short fetchShortFromBytes(const device uchar *input, uint offset)
{
    if (kVertexPullingOffsetIsAligned) {
        auto shortPtr = reinterpret_cast<const device Short*>(input + offset);
        return shortPtr[0];
    }
    auto packedPtr = reinterpret_cast<const device packed_uchar2*>(input + offset);
    packed_uchar2 data = packedPtr[0];
    return as_type<Short>(data);
}

template <typename Int>
static inline Int fetchIntFromBytes(const device uchar *input, uint offset)
{
    if (kVertexPullingOffsetIsAligned) {
        auto intPtr = reinterpret_cast<const device Int*>(input + offset);
        return intPtr[0];
    }
    auto packedPtr = reinterpret_cast<const device packed_uchar4*>(input + offset);
    packed_uchar4 data = packedPtr[0];
    return as_type<Int>(data);
}

template <typename T1, typename T2>
static inline T1 fixedToFloat(T2 fixed)
{
    constexpr float divisor = 1.0 / static_cast<float>(1 << 16);
    return static_cast<T1>(fixed) * divisor;
}

// Copy from src/libANGLE/renderer/copyvertex.inc.h
template <bool isSigned, bool normalized>
static inline float packedXYZ101010ToFloat(uint32_t data)
{
    const uint32_t rgbSignMask  = 0x200;       // 1 set at the 9 bit
    const uint32_t negativeMask = 0xFFFFFC00;  // All bits from 10 to 31 set to 1

    float finalValue = static_cast<float>(data);
    if (isSigned)
    {
        if (data & rgbSignMask)
        {
            int negativeNumber = data | negativeMask;
            finalValue         = static_cast<float>(negativeNumber);
        }

        if (normalized)
        {
            const int32_t maxValue = 0x1FF;       // 1 set in bits 0 through 8
            const int32_t minValue = 0xFFFFFE01;  // Inverse of maxValue

            // A 10-bit two's complement number has the possibility of being minValue - 1 but
            // OpenGL's normalization rules dictate that it should be clamped to minValue in
            // this case.
            if (finalValue < minValue)
            {
                finalValue = minValue;
            }

            const int32_t halfRange = (maxValue - minValue) >> 1;
            finalValue              = ((finalValue - minValue) / halfRange) - 1.0f;
        }
    }
    else
    {
        if (normalized)
        {
            const uint32_t maxValue = 0x3FF;  // 1 set in bits 0 through 9
            finalValue /= static_cast<float>(maxValue);
        }
    }

    return finalValue;
}

template <bool isSigned, bool normalized>
inline float packedW2ToFloat(uint32_t data)
{
    float finalValue = 0;
    if (isSigned)
    {
        if (normalized)
        {
            switch (data)
            {
                case 0x0:
                    finalValue = 0.0f;
                    break;
                case 0x1:
                    finalValue = 1.0f;
                    break;
                case 0x2:
                    finalValue = -1.0f;
                    break;
                case 0x3:
                    finalValue = -1.0f;
                    break;
            }
        }
        else
        {
            switch (data)
            {
                case 0x0:
                    finalValue = 0.0f;
                    break;
                case 0x1:
                    finalValue = 1.0f;
                    break;
                case 0x2:
                    finalValue = -2.0f;
                    break;
                case 0x3:
                    finalValue = -1.0f;
                    break;
            }
        }
    }
    else
    {
        if (normalized)
        {
            finalValue = data / 3.0f;
        }
        else
        {
            finalValue = static_cast<float>(data);
        }
    }

    return finalValue;
}

template <bool isSigned, bool normalized>
static inline float4 packedXYZW1010102ToFloat(uint packedValue)
{
    const uint32_t rgbMask  = 0x3FF;  // 1 set in bits 0 through 9
    const size_t redShift   = 0;      // red is bits 0 through 9
    const size_t greenShift = 10;     // green is bits 10 through 19
    const size_t blueShift  = 20;     // blue is bits 20 through 29

    const uint32_t alphaMask = 0x3;  // 1 set in bits 0 and 1
    const size_t alphaShift  = 30;   // Alpha is the 30 and 31 bits

    float4 re;
    re.x = packedXYZ101010ToFloat<isSigned, normalized>((packedValue >> redShift) & rgbMask);
    re.y = packedXYZ101010ToFloat<isSigned, normalized>((packedValue >> greenShift) & rgbMask);
    re.z = packedXYZ101010ToFloat<isSigned, normalized>((packedValue >> blueShift) & rgbMask);
    re.w = packedW2ToFloat<isSigned, normalized>((packedValue >> alphaShift) & alphaMask);
    return re;
}

template <unsigned int inputBitCount, typename T1, typename T2>
static inline T1 normalizedToFloat(T2 input)
{
    static_assert(inputBitCount <= (sizeof(T2) * 8),
                  "T2 must have more bits than or same bits as inputBitCount.");

    if (inputBitCount < 32)
    {
        const float inverseMax = 1.0f / (static_cast<uint>(0x1 << inputBitCount) - 1);
        return max(static_cast<T1>(input) * inverseMax, T1(-1.0));
    }
    else
    {
        constexpr float inverseMax32 = 1.0f / 0xffffffff;
        return max(static_cast<T1>(input) * inverseMax32, T1(-1.0));
    }
}

// uchar
static inline uchar4 fetchUByte(const device uchar *input,
                                int offset,
                                int stride,
                                int index,
                                uint components,
                                uchar defaultAlpha = 1)
{
    uchar4 re = uchar4(0, 0, 0, defaultAlpha);
    for (uint i = 0; i < components; ++i)
    {
        re[i] = input[offset + stride * index + i];
    }
    return re;
}

// uchar normalized
static inline float4 fetchUByteNorm(const device uchar *input,
                                    int offset,
                                    int stride,
                                    int index,
                                    uint components)
{
    uchar4 re = fetchUByte(input, offset, stride, index, components, 255);
    return normalizedToFloat<8, float4>(re);
}

// char
static inline char4 fetchByte(const device uchar *input,
                              int offset,
                              int stride,
                              int index,
                              uint components)
{
    return as_type<char4>(fetchUByte(input, offset, stride, index, components));
}

// char normalized
static inline float4 fetchByteNorm(const device uchar *input,
                                   int offset,
                                   int stride,
                                   int index,
                                   uint components)
{
    char4 re = as_type<char4>(fetchUByte(input, offset, stride, index, components, 127));
    return normalizedToFloat<7, float4>(re);
}

// ushort
static inline ushort4 fetchUShort(const device uchar *input,
                                  int offset,
                                  int stride,
                                  int index,
                                  uint components,
                                  ushort defaultAlpha = 1)
{
    ushort4 re = ushort4(0, 0, 0, defaultAlpha);
    for (uint i = 0; i < components; ++i)
    {
        re[i] = fetchShortFromBytes<ushort>(input, offset + stride * index + i * 2);
    }
    return re;
}

// ushort normalized
static inline float4 fetchUShortNorm(const device uchar *input,
                                     int offset,
                                     int stride,
                                     int index,
                                     uint components)
{
    ushort4 re = fetchUShort(input, offset, stride, index, components, 0xffff);
    return normalizedToFloat<16, float4>(re);
}

// short
static inline short4 fetchShort(const device uchar *input,
                                int offset,
                                int stride,
                                int index,
                                uint components)
{
    return as_type<short4>(fetchUShort(input, offset, stride, index, components));
}

// short normalized
static inline float4 fetchShortNorm(const device uchar *input,
                                    int offset,
                                    int stride,
                                    int index,
                                    uint components)
{
    short4 re = as_type<short4>(fetchUShort(input, offset, stride, index, components, 0x7fff));
    return normalizedToFloat<15, float4>(re);
}

// uint
static inline uint4 fetchUInt(const device uchar *input,
                              int offset,
                              int stride,
                              int index,
                              uint components,
                              uint defaultAlpha = 1)
{
    uint4 re = uint4(0, 0, 0, defaultAlpha);
    for (uint i = 0; i < components; ++i)
    {
        re[i] = fetchIntFromBytes<uint>(input, offset + stride * index + i * 4);
    }
    return re;
}

// uint normalized
static inline float4 fetchUIntNorm(const device uchar *input,
                                   int offset,
                                   int stride,
                                   int index,
                                   uint components)
{
    uint4 re = fetchUInt(input, offset, stride, index, components, 0xffffffff);
    return normalizedToFloat<32, float4>(re);
}

// int
static inline int4 fetchInt(const device uchar *input,
                            int offset,
                            int stride,
                            int index,
                            uint components)
{
    return as_type<int4>(fetchUInt(input, offset, stride, index, components));
}

// int normalized
static inline float4 fetchIntNorm(const device uchar *input,
                                  int offset,
                                  int stride,
                                  int index,
                                  uint components)
{
    int4 re = as_type<int4>(fetchUInt(input, offset, stride, index, components, 0x7fffffff));
    return normalizedToFloat<31, float4>(re);
}

// half float
static inline half4 fetchHalf(const device uchar *input,
                              int offset,
                              int stride,
                              int index,
                              uint components)
{
    constexpr half defaultAlpha = 1.0;
    return as_type<half4>(
        fetchUShort(input, offset, stride, index, components, as_type<ushort>(defaultAlpha)));
}

// float
static inline float4 fetchFloat(const device uchar *input,
                                int offset,
                                int stride,
                                int index,
                                uint components)
{
    constexpr float defaultAlpha = 1.0;
    return as_type<float4>(
        fetchUInt(input, offset, stride, index, components, as_type<uint>(defaultAlpha)));
}

// fixed
static inline float4 fetchFixed(const device uchar *input,
                                int offset,
                                int stride,
                                int index,
                                uint components)
{
    float4 re = float4(0, 0, 0, 1);
    float4 convertedWithPadding =
        fixedToFloat<float4>(fetchInt(input, offset, stride, index, components));
    for (uint i = 0; i < components; ++i)
    {
        re[i] = convertedWithPadding[i];
    }
    return re;
}

// packed XYZW1010102
template <bool isSigned, bool normalized>
static inline float4 fetchPackedXYZW1010102(const device uchar *input,
                                            int offset,
                                            int stride,
                                            int index)
{
    uint packedValue = fetchIntFromBytes<uint>(input, offset + stride * index);
    return packedXYZW1010102ToFloat<isSigned, normalized>(packedValue);
}

template <bool isSigned, bool normalized>
static inline float4 fetchPackedXYZW1010102(const device uchar *input,
                                            int offset,
                                            int stride,
                                            int index,
                                            int components /* ignored */)
{
    return fetchPackedXYZW1010102<isSigned, normalized>(input, offset, stride, index);
}

#define fetchXYZW1010102Int fetchPackedXYZW1010102<true, false>
#define fetchXYZW1010102IntNorm fetchPackedXYZW1010102<true, true>

#define fetchXYZW1010102UInt fetchPackedXYZW1010102<false, false>
#define fetchXYZW1010102UIntNorm fetchPackedXYZW1010102<false, true>

// Bit cast to uint4
static inline uint4 asUInt4(uint4 src)
{
    return src;
}
static inline uint4 asUInt4(float4 src)
{
    return as_type<uint4>(src);
}
static inline uint4 asUInt4(int4 src)
{
    return as_type<uint4>(src);
}

// Promote to float4 then bit cast to uint4
static inline uint4 asUInt4(half4 src)
{
    return asUInt4(static_cast<float4>(src));
}

// Promote to uint4
static inline uint4 asUInt4(uchar4 src)
{
    return static_cast<uint4>(src);
}
static inline uint4 asUInt4(ushort4 src)
{
    return static_cast<uint4>(src);
}

// Promote to int4 then bit cast to uint4
static inline uint4 asUInt4(char4 src)
{
    return asUInt4(static_cast<int4>(src));
}
static inline uint4 asUInt4(short4 src)
{
    return asUInt4(static_cast<int4>(src));
}

// TYPE = Float, Fixed, etc
// COMP_VAR = number of components, can be expression, variable, number, etc.
// INPUT = vertex buffer.
// INDEX = vertex index.
#define FETCH_UNALIGNED(TYPE, COMP_VAR, INPUT, INDEX) \
    fetch##TYPE(INPUT, kVertexPullingOffset, kVertexPullingStride, INDEX, COMP_VAR)

#define FETCH_UNALIGNED_NORM(TYPE, COMP_VAR, INPUT, INDEX) \
    fetch##TYPE##Norm(INPUT, kVertexPullingOffset, kVertexPullingStride, INDEX, COMP_VAR)

// Fetch source integer vertex attribute from INPUT buffer and assign to DEST as uint4 (preserve
// bits if needed).
#define FETCH_UNALIGNED_INT(TYPE, COMP_VAR, INPUT, INDEX, DEST)                             \
    if (kVertexPullingNormalize)                                                            \
    {                                                                                       \
        DEST = asUInt4(FETCH_UNALIGNED_NORM(TYPE, COMP_VAR, INPUT, INDEX));                 \
    }                                                                                       \
    else if (kVertexPullingConvertToFloat)                                                  \
    {                                                                                       \
        DEST = asUInt4(static_cast<float4>(FETCH_UNALIGNED(TYPE, COMP_VAR, INPUT, INDEX))); \
    }                                                                                       \
    else                                                                                    \
    {                                                                                       \
        DEST = asUInt4(FETCH_UNALIGNED(TYPE, COMP_VAR, INPUT, INDEX));                      \
    }

// Fetch source float/fixed/half vertex attribute from INPUT buffer, assign to DEST as uint4
// (preserve bits if needed).
#define FETCH_UNALIGNED_FLOAT(TYPE, COMP_VAR, INPUT, INDEX, DEST) \
    DEST = asUInt4(static_cast<float4>(FETCH_UNALIGNED(TYPE, COMP_VAR, INPUT, INDEX)));

#define FETCH_CASE(TYPE, COMP_VAR, UNALIGNED_INPUT, INDEX, FETCH_UNALIGNED_PROC, DEST) \
    case kVertexType##TYPE:                                                            \
        FETCH_UNALIGNED_PROC(TYPE, COMP_VAR, UNALIGNED_INPUT, INDEX, DEST);            \
        break;

#define FETCH_CASE_PACKED(TYPE, UNALIGNED_INPUT, INDEX, DEST)      \
    case kVertexType##TYPE:                                        \
        FETCH_UNALIGNED_INT(TYPE, 0, UNALIGNED_INPUT, INDEX, DEST) \
        break;

#define FETCH_CASE_INT(TYPE, COMP_VAR, UNALIGNED_INPUT, INDEX, DEST) \
    FETCH_CASE(TYPE, COMP_VAR, UNALIGNED_INPUT, INDEX, FETCH_UNALIGNED_INT, DEST)

#define FETCH_CASE_FLOAT(TYPE, COMP_VAR, UNALIGNED_INPUT, INDEX, DEST) \
    FETCH_CASE(TYPE, COMP_VAR, UNALIGNED_INPUT, INDEX, FETCH_UNALIGNED_FLOAT, DEST)

#define FETCH_CODE(UNALIGNED_INPUT, DEST)                                                          \
    {                                                                                              \
        uint vertexFinalIndex;                                                                     \
        if (kVertexPullingDivisor == 0)                                                            \
        {                                                                                          \
            vertexFinalIndex = gl_VertexIndex;                                                     \
        }                                                                                          \
        else                                                                                       \
        {                                                                                          \
            vertexFinalIndex = gl_BaseInstance + gl_InstanceIndex / kVertexPullingDivisor;         \
        }                                                                                          \
        switch (kVertexPullingType)                                                                \
        {                                                                                          \
            FETCH_CASE_INT(Byte, kVertexPullingComponentCount, UNALIGNED_INPUT, vertexFinalIndex,  \
                           DEST)                                                                   \
            FETCH_CASE_INT(UByte, kVertexPullingComponentCount, UNALIGNED_INPUT, vertexFinalIndex, \
                           DEST)                                                                   \
            FETCH_CASE_INT(Short, kVertexPullingComponentCount, UNALIGNED_INPUT, vertexFinalIndex, \
                           DEST)                                                                   \
            FETCH_CASE_INT(UShort, kVertexPullingComponentCount, UNALIGNED_INPUT,                  \
                           vertexFinalIndex, DEST)                                                 \
            FETCH_CASE_INT(Int, kVertexPullingComponentCount, UNALIGNED_INPUT, vertexFinalIndex,   \
                           DEST)                                                                   \
            FETCH_CASE_INT(UInt, kVertexPullingComponentCount, UNALIGNED_INPUT, vertexFinalIndex,  \
                           DEST)                                                                   \
                                                                                                   \
            FETCH_CASE_PACKED(XYZW1010102Int, UNALIGNED_INPUT, vertexFinalIndex, DEST)             \
            FETCH_CASE_PACKED(XYZW1010102UInt, UNALIGNED_INPUT, vertexFinalIndex, DEST)            \
                                                                                                   \
            FETCH_CASE_FLOAT(Float, kVertexPullingComponentCount, UNALIGNED_INPUT,                 \
                             vertexFinalIndex, DEST)                                               \
            FETCH_CASE_FLOAT(Half, kVertexPullingComponentCount, UNALIGNED_INPUT,                  \
                             vertexFinalIndex, DEST)                                               \
            FETCH_CASE_FLOAT(Fixed, kVertexPullingComponentCount, UNALIGNED_INPUT,                 \
                             vertexFinalIndex, DEST)                                               \
                                                                                                   \
            break;                                                                                 \
        }                                                                                          \
    }

[[stitchable]] uint4 ANGLE_pullVertexAsUInt4(const device uchar *inputUnaligned,
                                             const device uchar *defaultAttribs,
                                             int gl_VertexIndex,
                                             int gl_InstanceIndex,
                                             int gl_BaseInstance)
{
    uint4 re;
    // TODO(abc): optimize if offset is aligned based on kVertexPullingOffsetIsAligned.
    if (kVertexPullingUseDefaultAttribs)
    {
        FETCH_CODE(defaultAttribs, re);
    }
    else
    {
        FETCH_CODE(inputUnaligned, re);
    }

    return re;
}

@@#endif  // __METAL_VERSION__ >= 230
