#ifndef LIBANGLE_INTERNALENUMS_H_
#define LIBANGLE_INTERNALENUMS_H_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <array>

namespace gl
{

enum class TextureTarget {
    e2D = 0,
    e2DMultisample = 1,
    e2DArray = 2,
    Rectangle = 3,
    External = 4,
    CubeMap = 5,
    e3D = 6,
};
constexpr int kCountTextureTarget = 7;

static bool FromGLenum(GLenum external, TextureTarget *internal) {
    switch(external) {
        case GL_TEXTURE_2D:
            *internal = TextureTarget::e2D;
            return true;
        case GL_TEXTURE_CUBE_MAP:
            *internal = TextureTarget::CubeMap;
            return true;
        case GL_TEXTURE_RECTANGLE_ANGLE:
            *internal = TextureTarget::Rectangle;
            return true;
        case GL_TEXTURE_3D:
            *internal = TextureTarget::e3D;
            return true;
        case GL_TEXTURE_2D_ARRAY:
            *internal = TextureTarget::e2DArray;
            return true;
        case GL_TEXTURE_2D_MULTISAMPLE:
            *internal = TextureTarget::e2DMultisample;
            return true;
        case GL_TEXTURE_EXTERNAL_OES:
            *internal = TextureTarget::External;
            return true;
        default:
            return false;
    }
}

static GLenum ToGLenum(TextureTarget internal) {
    switch(internal) {
        case TextureTarget::e2D:
            return GL_TEXTURE_2D;
        case TextureTarget::CubeMap:
            return GL_TEXTURE_CUBE_MAP;
        case TextureTarget::Rectangle:
            return GL_TEXTURE_RECTANGLE_ANGLE;
        case TextureTarget::e3D:
            return GL_TEXTURE_3D;
        case TextureTarget::e2DArray:
            return GL_TEXTURE_2D_ARRAY;
        case TextureTarget::e2DMultisample:
            return GL_TEXTURE_2D_MULTISAMPLE;
        case TextureTarget::External:
            return GL_TEXTURE_EXTERNAL_OES;
    }
}

template<typename T, typename Enum, size_t Max>
struct EnumArray : public std::array<T, Max> {
    T& operator[](Enum n) {return std::array<T, Max>::operator[] (static_cast<int>(n));}
    const T& operator[](Enum n) const {return std::array<T, Max>::operator[] (static_cast<int>(n));}
    T& operator[](int n) {return std::array<T, Max>::operator[] (n);}
    const T& operator[](int n) const {return std::array<T, Max>::operator[] (n);}
    const T& at(Enum n) const {return std::array<T, Max>::iat(static_cast<int>(n));}
    T& at(Enum n) {return std::array<T, Max>::at(static_cast<int>(n));}
};

} // namespace gl

#endif
