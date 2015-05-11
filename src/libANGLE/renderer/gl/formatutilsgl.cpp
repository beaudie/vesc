//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// formatutilsgl.cpp: Queries for GL image formats and their translations to native
// GL formats.

#include "libANGLE/renderer/gl/formatutilsgl.h"

#include <map>

#include "libANGLE/renderer/gl/renderergl_utils.h"

namespace rx
{

namespace nativegl
{

SupportRequirement::SupportRequirement()
    : majorVersion(std::numeric_limits<GLuint>::max()),
      minorVersion(std::numeric_limits<GLuint>::max()),
      extensions()
{
}


InternalFormatRequirements::InternalFormatRequirements()
    : texture(),
      filter(),
      renderbuffer(),
      framebufferAttachment()
{
}

InternalFormat::InternalFormat()
    : glSupport(),
      glesSupport()
{
}

static inline SupportRequirement VersionOrExts(GLuint major, GLuint minor, const std::string &ext)
{
    SupportRequirement requirement;
    requirement.majorVersion = major;
    requirement.minorVersion = minor;
    requirement.extensions = TokenizeExtensionsString(ext.c_str());
    return requirement;
}

static inline SupportRequirement VersionAndExts(GLuint major, GLuint minor, const std::string &ext)
{
    // TODO
    SupportRequirement requirement;
    requirement.majorVersion = major;
    requirement.minorVersion = minor;
    requirement.extensions = TokenizeExtensionsString(ext.c_str());
    return requirement;
}

static inline SupportRequirement VersionOnly(GLuint major, GLuint minor)
{
    SupportRequirement requirement;
    requirement.majorVersion = major;
    requirement.minorVersion = minor;
    return requirement;
}

static inline SupportRequirement ExtsOnly(const std::string &ext)
{
    SupportRequirement requirement;
    requirement.extensions = TokenizeExtensionsString(ext.c_str());
    return requirement;
}

static inline SupportRequirement Always()
{
    SupportRequirement requirement;
    requirement.majorVersion = 0;
    requirement.minorVersion = 0;
    return requirement;
}

static inline SupportRequirement Never()
{
    SupportRequirement requirement;
    requirement.majorVersion = std::numeric_limits<GLuint>::max();
    requirement.minorVersion = std::numeric_limits<GLuint>::max();
    return requirement;
}

/*
// Supporst both texture and filter with the same check
static inline InternalFormatRequirements TextureFilter(const SupportRequirement& texture)
{
    InternalFormatRequirements requirements;
    requirements.texture = texture;
    requirements.filter = texture;
    return requirements;
}

static inline InternalFormatRequirements TextureRender(const SupportRequirement& support)
{
    InternalFormatRequirements requirements;
    requirements.texture = support;
    requirements.renderbuffer = support;
    requirements.framebufferAttachment = support;
    return requirements;
}

static inline InternalFormatRequirements TextureFilterRender(const SupportRequirement& texture, const SupportRequirement& render)
{
    InternalFormatRequirements requirements;
    requirements.texture = texture;
    requirements.filter = texture;
    requirements.renderbuffer = render;
    requirements.framebufferAttachment = render;
    return requirements;
}

static inline InternalFormatRequirements Everything(const SupportRequirement& everything)
{
    InternalFormatRequirements requirements;
    requirements.texture = everything;
    requirements.filter = everything;
    requirements.renderbuffer = everything;
    requirements.framebufferAttachment = everything;
    return requirements;
}
*/

typedef std::pair<GLenum, InternalFormat> InternalFormatInfoPair;
typedef std::map<GLenum, InternalFormat> InternalFormatInfoMap;

// A helper function to insert data into the format map with fewer characters.
static inline void InsertFormatMapping(InternalFormatInfoMap *map, GLenum internalFormat,
    const SupportRequirement &desktopTexture, const SupportRequirement &desktopFilter, const SupportRequirement &desktopRender,
    const SupportRequirement &esTexture, const SupportRequirement &esFilter, const SupportRequirement &esRender)
{
    InternalFormat formatInfo;
    formatInfo.glSupport.texture = desktopTexture;
    formatInfo.glSupport.filter = desktopFilter;
    formatInfo.glSupport.renderbuffer = desktopRender;
    formatInfo.glSupport.framebufferAttachment = desktopRender;
    formatInfo.glesSupport.texture = esTexture;
    formatInfo.glesSupport.filter = esTexture;
    formatInfo.glesSupport.renderbuffer = esFilter;
    formatInfo.glesSupport.framebufferAttachment = esRender;
    map->insert(std::make_pair(internalFormat, formatInfo));
}

static InternalFormatInfoMap BuildInternalFormatInfoMap()
{
    InternalFormatInfoMap map;
    /*
    // Floating point formats
    //                       | Internal format     | Texture support     | Render support     | Filter support      |
    InsertFormatMapping(&map, GL_R16F,              UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RG16F,             UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGB16F,            UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGBA16F,           UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_R32F,              UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RG32F,             UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGB32F,            UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGBA32F,           UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    */

    InsertFormatMapping(&map, GL_R8,                VersionOrExts(3, 0, "GL_ARB_texture_rg"), Always(), VersionOrExts(3, 0, "GL_ARB_texture_rg"),    VersionOrExts(3, 0, "GL_EXT_texture_rg"), Always(), VersionOrExts(3, 0, "GL_EXT_texture_rg"));
    InsertFormatMapping(&map, GL_R8_SNORM,          VersionOnly(3, 1), Always(), Never(),                                                            VersionOnly(3, 0), Always(), Never());
    InsertFormatMapping(&map, GL_RG8,               VersionOrExts(3, 0, "GL_ARB_texture_rg"), Always(), VersionOrExts(3, 0, "GL_ARB_texture_rg"),    VersionOrExts(3, 0, "GL_EXT_texture_rg"), Always(), VersionOrExts(3, 0, "GL_ARB_texture_rg"));
    InsertFormatMapping(&map, GL_RG8_SNORM,         VersionOnly(3, 1), Always(), Never(),                                                             VersionOnly(3, 0), Always(), Never());
    InsertFormatMapping(&map, GL_RGB8,              Always(), Always(), Always(),                                                                      VersionOrExts(3, 0, "GL_OES_rgb8_rgba8"), Always(), Always());
    InsertFormatMapping(&map, GL_RG8_SNORM,         VersionOnly(3, 1), Always(), Never(),                                                            VersionOnly(3, 0), Always(), Never());
    InsertFormatMapping(&map, GL_RGB565,            Always(), Always(), Always(),                                                                      Always(), Always(), Always());
    InsertFormatMapping(&map, GL_RGBA4,             Always(), Always(), Always(),                                                                      Always(), Always(), Always());
    InsertFormatMapping(&map, GL_RGB5_A1,           Always(), Always(), Always(),                                                                      Always(), Always(), Always());
    InsertFormatMapping(&map, GL_RGBA8,             Always(), Always(), Always(),                                                                      VersionOrExts(3, 0, "GL_OES_rgb8_rgba8"), Always(), VersionOrExts(3, 0, "GL_OES_rgb8_rgba8"));
    InsertFormatMapping(&map, GL_RGBA8_SNORM,       VersionOnly(3, 1), Always(), Never(),                                                                      VersionOnly(3, 0), Always(), Never());
    InsertFormatMapping(&map, GL_RGB10_A2,          Always(), Always(), Always(),                                                                               VersionOnly(3, 0), Always(), VersionOnly(3, 0));
    InsertFormatMapping(&map, GL_RGB10_A2UI,        VersionOrExts(3, 3, "GL_ARB_texture_rgb10_a2ui"), Never(), Never(),                                     VersionOnly(3, 0), Never(), Never());
    InsertFormatMapping(&map, GL_SRGB8,             VersionOrExts(2, 1, "GL_EXT_texture_sRGB"), Always(), VersionOrExts(2, 1, "GL_EXT_texture_sRGB"),     VersionOrExts(3, 0, "GL_EXT_texture_sRGB"), Always(), VersionOrExts(3, 0, "GL_EXT_texture_sRGB"));
    InsertFormatMapping(&map, GL_SRGB8_ALPHA8,      VersionOrExts(2, 1, "GL_EXT_texture_sRGB"), Always(), VersionOrExts(2, 1, "GL_EXT_texture_sRGB"),     VersionOrExts(3, 0, "GL_EXT_texture_sRGB"), Always(), VersionOrExts(3, 0, "GL_EXT_texture_sRGB"));
    InsertFormatMapping(&map, GL_R8I,               VersionOrExts(3, 0, "GL_ARB_texture_rg"), Never(), VersionOrExts(3, 0, "GL_ARB_texture_rg"),          VersionOnly(3, 0), Never(), VersionOnly(3, 0));
    InsertFormatMapping(&map, GL_R8UI,              VersionOrExts(3, 0, "GL_ARB_texture_rg"), Never(), VersionOrExts(3, 0, "GL_ARB_texture_rg"),          VersionOnly(3, 0), Never(), VersionOnly(3, 0));
    InsertFormatMapping(&map, GL_R16I,              VersionOrExts(3, 0, "GL_ARB_texture_rg"), Never(), VersionOrExts(3, 0, "GL_ARB_texture_rg"),          VersionOnly(3, 0), Never(), VersionOnly(3, 0));
    InsertFormatMapping(&map, GL_R16UI,             VersionOrExts(3, 0, "GL_ARB_texture_rg"), Never(), VersionOrExts(3, 0, "GL_ARB_texture_rg"),          VersionOnly(3, 0), Never(), VersionOnly(3, 0));
    InsertFormatMapping(&map, GL_R32I,              VersionOrExts(3, 0, "GL_ARB_texture_rg"), Never(), VersionOrExts(3, 0, "GL_ARB_texture_rg"),          VersionOnly(3, 0), Never(), VersionOnly(3, 0));
    InsertFormatMapping(&map, GL_R32UI,             VersionOrExts(3, 0, "GL_ARB_texture_rg"), Never(), VersionOrExts(3, 0, "GL_ARB_texture_rg"),          VersionOnly(3, 0), Never(), VersionOnly(3, 0));
    InsertFormatMapping(&map, GL_RG8I,              VersionOrExts(3, 0, "GL_ARB_texture_rg"), Never(), VersionOrExts(3, 0, "GL_ARB_texture_rg"),          VersionOnly(3, 0), Never(), VersionOnly(3, 0));
    InsertFormatMapping(&map, GL_RG8UI,             VersionOrExts(3, 0, "GL_ARB_texture_rg"), Never(), VersionOrExts(3, 0, "GL_ARB_texture_rg"),          VersionOnly(3, 0), Never(), VersionOnly(3, 0));
    InsertFormatMapping(&map, GL_RG16I,             VersionOrExts(3, 0, "GL_ARB_texture_rg"), Never(), VersionOrExts(3, 0, "GL_ARB_texture_rg"),          VersionOnly(3, 0), Never(), VersionOnly(3, 0));
    InsertFormatMapping(&map, GL_RG16UI,            VersionOrExts(3, 0, "GL_ARB_texture_rg"), Never(), VersionOrExts(3, 0, "GL_ARB_texture_rg"),          VersionOnly(3, 0), Never(), VersionOnly(3, 0));
    InsertFormatMapping(&map, GL_RG32I,             VersionOrExts(3, 0, "GL_ARB_texture_rg"), Never(), VersionOrExts(3, 0, "GL_ARB_texture_rg"),          VersionOnly(3, 0), Never(), VersionOnly(3, 0));
    InsertFormatMapping(&map, GL_RG32UI,            VersionOrExts(3, 0, "GL_ARB_texture_rg"), Never(), VersionOrExts(3, 0, "GL_ARB_texture_rg"),          VersionOnly(3, 0), Never(), VersionOnly(3, 0));
    InsertFormatMapping(&map, GL_RGB8I,             VersionOrExts(3, 0, "GL_EXT_texture_integer"), Never(), Never(),                                      VersionOnly(3, 0), Never(), Never());
    InsertFormatMapping(&map, GL_RGB8UI,            VersionOrExts(3, 0, "GL_EXT_texture_integer"), Never(), Never(),                                      VersionOnly(3, 0), Never(), Never());
    InsertFormatMapping(&map, GL_RGB16I,            VersionOrExts(3, 0, "GL_EXT_texture_integer"), Never(), Never(),                                      VersionOnly(3, 0), Never(), Never());
    InsertFormatMapping(&map, GL_RGB16UI,           VersionOrExts(3, 0, "GL_EXT_texture_integer"), Never(), Never(),                                      VersionOnly(3, 0), Never(), Never());
    InsertFormatMapping(&map, GL_RGB32I,            VersionOrExts(3, 0, "GL_EXT_texture_integer"), Never(), Never(),                                      VersionOnly(3, 0), Never(), Never());
    InsertFormatMapping(&map, GL_RGB32UI,           VersionOrExts(3, 0, "GL_EXT_texture_integer"), Never(), Never(),                                      VersionOnly(3, 0), Never(), Never());
    InsertFormatMapping(&map, GL_RGBA8I,            VersionOrExts(3, 0, "GL_EXT_texture_integer"), Never(), VersionOrExts(3, 0, "GL_EXT_texture_integer"),          VersionOnly(3, 0), Never(), VersionOnly(3, 0));
    InsertFormatMapping(&map, GL_RGBA8UI,           VersionOrExts(3, 0, "GL_EXT_texture_integer"), Never(), VersionOrExts(3, 0, "GL_EXT_texture_integer"),          VersionOnly(3, 0), Never(), VersionOnly(3, 0));
    InsertFormatMapping(&map, GL_RGBA16I,           VersionOrExts(3, 0, "GL_EXT_texture_integer"), Never(), VersionOrExts(3, 0, "GL_EXT_texture_integer"),          VersionOnly(3, 0), Never(), VersionOnly(3, 0));
    InsertFormatMapping(&map, GL_RGBA16UI,          VersionOrExts(3, 0, "GL_EXT_texture_integer"), Never(), VersionOrExts(3, 0, "GL_EXT_texture_integer"),          VersionOnly(3, 0), Never(), VersionOnly(3, 0));
    InsertFormatMapping(&map, GL_RGBA32I,           VersionOrExts(3, 0, "GL_EXT_texture_integer"), Never(), VersionOrExts(3, 0, "GL_EXT_texture_integer"),          VersionOnly(3, 0), Never(), VersionOnly(3, 0));
    InsertFormatMapping(&map, GL_RGBA32UI,          VersionOrExts(3, 0, "GL_EXT_texture_integer"), Never(), VersionOrExts(3, 0, "GL_EXT_texture_integer"),          VersionOnly(3, 0), Never(), VersionOnly(3, 0));

    InsertFormatMapping(&map, GL_BGRA8_EXT,         Never(), Never(), Never(),                                    ExtsOnly("textureFormatBGRA8888"), Always(), ExtsOnly("textureFormatBGRA8888"));

    // Floating point formats
    InsertFormatMapping(&map, GL_R11F_G11F_B10F,    VersionOrExts(3, 0, "GL_EXT_packed_float"), Always(), VersionOrExts(3, 0, "GL_EXT_packed_float ARB_color_buffer_float"),                                    VersionOnly(3, 0), Always(), VersionAndExts(3, 0, "GL_EXT_color_buffer_float"));
    InsertFormatMapping(&map, GL_RGB9_E5,           VersionOrExts(3, 0, "GL_EXT_texture_shared_exponent"), Always(), VersionOrExts(3, 0, "GL_EXT_texture_shared_exponent ARB_color_buffer_float"),              VersionOnly(3, 0), Always(), VersionAndExts(3, 0, "GL_EXT_color_buffer_float"));

    InsertFormatMapping(&map, GL_R16F, VersionOrExts(3, 0, "GL_ARB_texture_rg ARB_texture_float"), Always(), VersionOrExts(3, 0, "GL_ARB_texture_rg ARB_texture_float ARB_color_buffer_float"), VersionOrExts(3, 0, "GL_OES_texture_half_float GL_EXT_texture_rg"), ExtsOnly("GL_OES_texture_half_float_linear"), VersionOrExts(3, 0, "GL_OES_texture_half_float GL_EXT_texture_rg"));

    /*
    // From ES 3.0.1 spec, table 3.12
    InsertFormatMapping(&map, GL_NONE,              NeverSupported,       NeverSupported,       NeverSupported);

    //                       | Internal format     | Texture support     | Render support     | Filter support      |
    InsertFormatMapping(&map, GL_R8,                UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_R8_SNORM,          UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RG8,               UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RG8_SNORM,         UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGB8,              AlwaysSupported,      AlwaysSupported,      AlwaysSupported     );
    InsertFormatMapping(&map, GL_RGB8_SNORM,        UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGB565,            AlwaysSupported,      AlwaysSupported,      AlwaysSupported     );
    InsertFormatMapping(&map, GL_RGBA4,             AlwaysSupported,      AlwaysSupported,      AlwaysSupported     );
    InsertFormatMapping(&map, GL_RGB5_A1,           UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGBA8,             AlwaysSupported,      AlwaysSupported,      AlwaysSupported     );
    InsertFormatMapping(&map, GL_RGBA8_SNORM,       UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGB10_A2,          UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGB10_A2UI,        UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_SRGB8,             UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_SRGB8_ALPHA8,      UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_R11F_G11F_B10F,    UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGB9_E5,           UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_R8I,               UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_R8UI,              UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_R16I,              UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_R16UI,             UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_R32I,              UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_R32UI,             UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RG8I,              UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RG8UI,             UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RG16I,             UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RG16UI,            UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RG32I,             UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RG32UI,            UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGB8I,             UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGB8UI,            UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGB16I,            UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGB16UI,           UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGB32I,            UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGB32UI,           UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGBA8I,            UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGBA8UI,           UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGBA16I,           UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGBA16UI,          UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGBA32I,           UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGBA32UI,          UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);

    InsertFormatMapping(&map, GL_BGRA8_EXT,         UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);

    // Floating point formats
    //                       | Internal format     | Texture support     | Render support     | Filter support      |
    InsertFormatMapping(&map, GL_R16F,              UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RG16F,             UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGB16F,            UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGBA16F,           UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_R32F,              UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RG32F,             UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGB32F,            UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGBA32F,           UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);

    // Depth stencil formats
    //                       | Internal format         | Texture support     | Render support     | Filter support      |
    InsertFormatMapping(&map, GL_DEPTH_COMPONENT16,     AlwaysSupported,      AlwaysSupported,      NeverSupported      );
    InsertFormatMapping(&map, GL_DEPTH_COMPONENT24,     UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_DEPTH_COMPONENT32F,    UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_DEPTH_COMPONENT32_OES, UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_DEPTH24_STENCIL8,      UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_DEPTH32F_STENCIL8,     UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_STENCIL_INDEX8,        AlwaysSupported,      AlwaysSupported,      NeverSupported      );

    // Luminance alpha formats
    //                       | Internal format          | Texture support     | Render support     | Filter support      |
    InsertFormatMapping(&map, GL_ALPHA8_EXT,             UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_LUMINANCE8_EXT,         UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_ALPHA32F_EXT,           UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_LUMINANCE32F_EXT,       UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_ALPHA16F_EXT,           UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_LUMINANCE16F_EXT,       UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_LUMINANCE8_ALPHA8_EXT,  UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_LUMINANCE_ALPHA32F_EXT, UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_LUMINANCE_ALPHA16F_EXT, UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);

    // Unsized formats
    //                       | Internal format   | Texture support     | Render support     | Filter support      |
    InsertFormatMapping(&map, GL_ALPHA,           UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_LUMINANCE,       UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_LUMINANCE_ALPHA, UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RED,             UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RG,              UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGB,             UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGBA,            UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RED_INTEGER,     UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RG_INTEGER,      UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGB_INTEGER,     UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_RGBA_INTEGER,    UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_BGRA_EXT,        UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_DEPTH_COMPONENT, UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_DEPTH_STENCIL,   UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_SRGB_EXT,        UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_SRGB_ALPHA_EXT,  UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);

    // Compressed formats, From ES 3.0.1 spec, table 3.16
    //                       | Internal format                             | Texture support     | Render support     | Filter support      |
    InsertFormatMapping(&map, GL_COMPRESSED_R11_EAC,                        UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_COMPRESSED_SIGNED_R11_EAC,                 UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_COMPRESSED_RG11_EAC,                       UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_COMPRESSED_SIGNED_RG11_EAC,                UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_COMPRESSED_RGB8_ETC2,                      UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_COMPRESSED_SRGB8_ETC2,                     UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,  UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_COMPRESSED_RGBA8_ETC2_EAC,                 UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC,          UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);

    // From GL_EXT_texture_compression_dxt1
    //                       | Internal format                   | Texture support     | Render support     | Filter support      |
    InsertFormatMapping(&map, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,    UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    InsertFormatMapping(&map, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,   UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);

    // From GL_ANGLE_texture_compression_dxt3
    InsertFormatMapping(&map, GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE, UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);

    // From GL_ANGLE_texture_compression_dxt5
    InsertFormatMapping(&map, GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE, UnimplementedSupport, UnimplementedSupport, UnimplementedSupport);
    */
    return map;
}

static const InternalFormatInfoMap &GetInternalFormatMap()
{
    static const InternalFormatInfoMap formatMap = BuildInternalFormatInfoMap();
    return formatMap;
}

const InternalFormat &GetInternalFormatInfo(GLenum internalFormat)
{
    const InternalFormatInfoMap &formatMap = GetInternalFormatMap();
    InternalFormatInfoMap::const_iterator iter = formatMap.find(internalFormat);
    if (iter != formatMap.end())
    {
        return iter->second;
    }
    else
    {
        static const InternalFormat defaultInternalFormat;
        return defaultInternalFormat;
    }
}

}

}
