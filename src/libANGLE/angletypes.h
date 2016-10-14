//
// Copyright (c) 2012-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// angletypes.h : Defines a variety of structures and enum types that are used throughout libGLESv2

#ifndef LIBANGLE_ANGLETYPES_H_
#define LIBANGLE_ANGLETYPES_H_

#include "libANGLE/Constants.h"
#include "libANGLE/RefCountObject.h"

#include <stdint.h>

#include <bitset>
#include <unordered_map>

namespace gl
{
class Buffer;
class State;
class Program;
struct VertexAttribute;
struct VertexAttribCurrentValueData;

enum PrimitiveType
{
    PRIMITIVE_POINTS,
    PRIMITIVE_LINES,
    PRIMITIVE_LINE_STRIP,
    PRIMITIVE_LINE_LOOP,
    PRIMITIVE_TRIANGLES,
    PRIMITIVE_TRIANGLE_STRIP,
    PRIMITIVE_TRIANGLE_FAN,
    PRIMITIVE_TYPE_MAX,
};

PrimitiveType GetPrimitiveType(GLenum drawMode);

enum SamplerType
{
    SAMPLER_PIXEL,
    SAMPLER_VERTEX
};

struct Rectangle
{
    Rectangle() : x(0), y(0), width(0), height(0) {}
    Rectangle(int x_in, int y_in, int width_in, int height_in)
        : x(x_in), y(y_in), width(width_in), height(height_in)
    {
    }

    int x0() const { return x; }
    int y0() const { return y; }
    int x1() const { return x + width; }
    int y1() const { return y + height; }

    int x;
    int y;
    int width;
    int height;
};

bool operator==(const Rectangle &a, const Rectangle &b);
bool operator!=(const Rectangle &a, const Rectangle &b);

bool ClipRectangle(const Rectangle &source, const Rectangle &clip, Rectangle *intersection);

struct Offset
{
    int x;
    int y;
    int z;

    Offset() : x(0), y(0), z(0) { }
    Offset(int x_in, int y_in, int z_in) : x(x_in), y(y_in), z(z_in) { }
};

struct Extents
{
    int width;
    int height;
    int depth;

    Extents() : width(0), height(0), depth(0) { }
    Extents(int width_, int height_, int depth_) : width(width_), height(height_), depth(depth_) { }

    Extents(const Extents &other) = default;
    Extents &operator=(const Extents &other) = default;

    bool empty() const { return (width * height * depth) == 0; }
};

bool operator==(const Extents &lhs, const Extents &rhs);
bool operator!=(const Extents &lhs, const Extents &rhs);

struct Box
{
    int x;
    int y;
    int z;
    int width;
    int height;
    int depth;

    Box() : x(0), y(0), z(0), width(0), height(0), depth(0) { }
    Box(int x_in, int y_in, int z_in, int width_in, int height_in, int depth_in) : x(x_in), y(y_in), z(z_in), width(width_in), height(height_in), depth(depth_in) { }
    Box(const Offset &offset, const Extents &size) : x(offset.x), y(offset.y), z(offset.z), width(size.width), height(size.height), depth(size.depth) { }
    bool operator==(const Box &other) const;
    bool operator!=(const Box &other) const;
};


struct RasterizerState
{
    bool cullFace;
    GLenum cullMode;
    GLenum frontFace;

    bool polygonOffsetFill;
    GLfloat polygonOffsetFactor;
    GLfloat polygonOffsetUnits;

    bool pointDrawMode;
    bool multiSample;

    bool rasterizerDiscard;
};

struct BlendState
{
    bool blend;
    GLenum sourceBlendRGB;
    GLenum destBlendRGB;
    GLenum sourceBlendAlpha;
    GLenum destBlendAlpha;
    GLenum blendEquationRGB;
    GLenum blendEquationAlpha;

    bool colorMaskRed;
    bool colorMaskGreen;
    bool colorMaskBlue;
    bool colorMaskAlpha;

    bool sampleAlphaToCoverage;

    bool dither;
};

struct DepthStencilState
{
    bool depthTest;
    GLenum depthFunc;
    bool depthMask;

    bool stencilTest;
    GLenum stencilFunc;
    GLuint stencilMask;
    GLenum stencilFail;
    GLenum stencilPassDepthFail;
    GLenum stencilPassDepthPass;
    GLuint stencilWritemask;
    GLenum stencilBackFunc;
    GLuint stencilBackMask;
    GLenum stencilBackFail;
    GLenum stencilBackPassDepthFail;
    GLenum stencilBackPassDepthPass;
    GLuint stencilBackWritemask;
};

// State from Table 6.10 (state per sampler object)
struct SamplerState
{
    SamplerState();
    static SamplerState CreateDefaultForTarget(GLenum target);

    GLenum minFilter;
    GLenum magFilter;

    GLenum wrapS;
    GLenum wrapT;
    GLenum wrapR;

    // From EXT_texture_filter_anisotropic
    float maxAnisotropy;

    GLfloat minLod;
    GLfloat maxLod;

    GLenum compareMode;
    GLenum compareFunc;
};

bool operator==(const SamplerState &a, const SamplerState &b);
bool operator!=(const SamplerState &a, const SamplerState &b);

struct PixelStoreStateBase
{
    BindingPointer<Buffer> pixelBuffer;
    GLint alignment   = 4;
    GLint rowLength   = 0;
    GLint skipRows    = 0;
    GLint skipPixels  = 0;
    GLint imageHeight = 0;
    GLint skipImages  = 0;
};

struct PixelUnpackState : PixelStoreStateBase
{
    PixelUnpackState() {}

    PixelUnpackState(GLint alignmentIn, GLint rowLengthIn)
    {
        alignment = alignmentIn;
        rowLength = rowLengthIn;
    }
};

struct PixelPackState : PixelStoreStateBase
{
    PixelPackState() {}

    PixelPackState(GLint alignmentIn, bool reverseRowOrderIn)
        : reverseRowOrder(reverseRowOrderIn)
    {
        alignment = alignmentIn;
    }

    bool reverseRowOrder = false;
};

// Used in Program and VertexArray.
typedef std::bitset<MAX_VERTEX_ATTRIBS> AttributesMask;

// Use in Program
typedef std::bitset<IMPLEMENTATION_MAX_COMBINED_SHADER_UNIFORM_BUFFERS> UniformBlockBindingMask;

// A map of GL objects indexed by object ID. The specific map implementation may change.
// Client code should treat it as a std::map.
template <class ResourceT>
using ResourceMap = std::unordered_map<GLuint, ResourceT *>;
}

namespace rx
{
enum VendorID : uint32_t
{
    VENDOR_ID_UNKNOWN  = 0x0,
    VENDOR_ID_AMD      = 0x1002,
    VENDOR_ID_INTEL    = 0x8086,
    VENDOR_ID_NVIDIA   = 0x10DE,
    // This is Qualcomm PCI Vendor ID.
    // Android doesn't have a PCI bus, but all we need is a unique id.
    VENDOR_ID_QUALCOMM = 0x5143,
};

// Referenced from https://cgit.freedesktop.org/vaapi/intel-driver/tree/src/i965_pciids.h
namespace Intel_DeviceIDs
{
const uint32_t Haswell[] = {
    0x0402, 0x0412, 0x0422, 0x0406, 0x0416, 0x0426, 0x040A, 0x041A, 0x042A, 0x040B, 0x041B, 0x042B,
    0x040E, 0x041E, 0x042E, 0x0C02, 0x0C12, 0x0C22, 0x0C06, 0x0C16, 0x0C26, 0x0C0A, 0x0C1A, 0x0C2A,
    0x0C0B, 0x0C1B, 0x0C2B, 0x0C0E, 0x0C1E, 0x0C2E, 0x0A02, 0x0A12, 0x0A22, 0x0A06, 0x0A16, 0x0A26,
    0x0A0A, 0x0A1A, 0x0A2A, 0x0A0B, 0x0A1B, 0x0A2B, 0x0A0E, 0x0A1E, 0x0A2E, 0x0D02, 0x0D12, 0x0D22,
    0x0D06, 0x0D16, 0x0D26, 0x0D0A, 0x0D1A, 0x0D2A, 0x0D0B, 0x0D1B, 0x0D2B, 0x0D0E, 0x0D1E, 0x0D2E};

const uint32_t Broadwell[] = {0x1602, 0x1606, 0x160A, 0x160B, 0x160D, 0x160E,
                              0x1612, 0x1616, 0x161A, 0x161B, 0x161D, 0x161E,
                              0x1622, 0x1626, 0x162A, 0x162B, 0x162D, 0x162E};

const uint32_t Skylake[] = {0x1902, 0x1906, 0x190A, 0x190B, 0x190E, 0x1912, 0x1913, 0x1915, 0x1916,
                            0x1917, 0x191A, 0x191B, 0x191D, 0x191E, 0x1921, 0x1923, 0x1926, 0x1927,
                            0x192A, 0x192B, 0x192D, 0x1932, 0x193A, 0x193B, 0x193D};

const uint32_t Kabylake[] = {0x5916, 0x5913, 0x5906, 0x5926, 0x5921, 0x5915, 0x590E,
                             0x591E, 0x5912, 0x5917, 0x5902, 0x591B, 0x593B, 0x590B,
                             0x591A, 0x590A, 0x591D, 0x5908, 0x5923, 0x5927};

inline bool isKabylake(uint32_t DeviceId)
{
    ULONGLONG size = sizeof(Kabylake) / sizeof(uint32_t);
    for (ULONGLONG i = 0; i < size; i++)
    {
        if (DeviceId == Kabylake[i])
            return true;
    }
    return false;
}

inline bool isSkylake(uint32_t DeviceId)
{
    ULONGLONG size = sizeof(Skylake) / sizeof(uint32_t);
    for (ULONGLONG i = 0; i < size; i++)
    {
        if (DeviceId == Skylake[i])
            return true;
    }
    return false;
}

inline bool isBroadwell(uint32_t DeviceId)
{
    ULONGLONG size = sizeof(Broadwell) / sizeof(uint32_t);
    for (ULONGLONG i = 0; i < size; i++)
    {
        if (DeviceId == Broadwell[i])
            return true;
    }
    return false;
}

inline bool isHaswell(uint32_t DeviceId)
{
    ULONGLONG size = sizeof(Haswell) / sizeof(uint32_t);
    for (ULONGLONG i = 0; i < size; i++)
    {
        if (DeviceId == Haswell[i])
            return true;
    }
    return false;
}
}  // namespace Intel_DeviceIDs

// A macro that determines whether an object has a given runtime type.
#if defined(__clang__)
#if __has_feature(cxx_rtti)
#define ANGLE_HAS_DYNAMIC_CAST 1
#endif
#elif !defined(NDEBUG) && (!defined(_MSC_VER) || defined(_CPPRTTI)) && (!defined(__GNUC__) || __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 3) || defined(__GXX_RTTI))
#define ANGLE_HAS_DYNAMIC_CAST 1
#endif

#ifdef ANGLE_HAS_DYNAMIC_CAST
#define ANGLE_HAS_DYNAMIC_TYPE(type, obj) (dynamic_cast<type >(obj) != nullptr)
#undef ANGLE_HAS_DYNAMIC_CAST
#else
#define ANGLE_HAS_DYNAMIC_TYPE(type, obj) (obj != nullptr)
#endif

// Downcast a base implementation object (EG TextureImpl to TextureD3D)
template <typename DestT, typename SrcT>
inline DestT *GetAs(SrcT *src)
{
    ASSERT(ANGLE_HAS_DYNAMIC_TYPE(DestT*, src));
    return static_cast<DestT*>(src);
}

template <typename DestT, typename SrcT>
inline const DestT *GetAs(const SrcT *src)
{
    ASSERT(ANGLE_HAS_DYNAMIC_TYPE(const DestT*, src));
    return static_cast<const DestT*>(src);
}

#undef ANGLE_HAS_DYNAMIC_TYPE

// Downcast a GL object to an Impl (EG gl::Texture to rx::TextureD3D)
template <typename DestT, typename SrcT>
inline DestT *GetImplAs(SrcT *src)
{
    return GetAs<DestT>(src->getImplementation());
}

}

#include "angletypes.inl"

namespace angle
{
// Zero-based for better array indexing
enum FramebufferBinding
{
    FramebufferBindingRead = 0,
    FramebufferBindingDraw,
    FramebufferBindingSingletonMax,
    FramebufferBindingBoth = FramebufferBindingSingletonMax,
    FramebufferBindingMax,
    FramebufferBindingUnknown = FramebufferBindingMax,
};

inline FramebufferBinding EnumToFramebufferBinding(GLenum enumValue)
{
    switch (enumValue)
    {
        case GL_READ_FRAMEBUFFER:
            return FramebufferBindingRead;
        case GL_DRAW_FRAMEBUFFER:
            return FramebufferBindingDraw;
        case GL_FRAMEBUFFER:
            return FramebufferBindingBoth;
        default:
            UNREACHABLE();
            return FramebufferBindingUnknown;
    }
}

inline GLenum FramebufferBindingToEnum(FramebufferBinding binding)
{
    switch (binding)
    {
        case FramebufferBindingRead:
            return GL_READ_FRAMEBUFFER;
        case FramebufferBindingDraw:
            return GL_DRAW_FRAMEBUFFER;
        case FramebufferBindingBoth:
            return GL_FRAMEBUFFER;
        default:
            UNREACHABLE();
            return GL_NONE;
    }
}
}

#endif // LIBANGLE_ANGLETYPES_H_
