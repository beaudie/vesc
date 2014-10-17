//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef SAMPLE_UTIL_FONT_H
#define SAMPLE_UTIL_FONT_H

#include "stb_truetype.h"

#include "geometry_utils.h"

#include <GLES2/gl2.h>

#include <cstdint>
#include <vector>
#include <map>
#include <string>

struct Glyph
{
    Rect bounds;

    GLuint texture;
    Rect texcoords;
};

class Font
{
  public:
    ~Font();

    bool loadData(const std::vector<uint8_t> &fontData);

    void prepareText(const std::string &text, size_t fontSize, Vector2 *outSize,
                     std::vector<Glyph> *outGlyphs);

  private:
    std::vector<uint8_t> mFontData;
    stbtt_fontinfo mFont;

    struct GlyphPage
    {
        GLuint texture;
        RectanglePacker packer;

        GlyphPage(GLuint texture, const Vector2 &size);
    };
    std::vector<GlyphPage> mPages;

    typedef uint32_t CodePoint;
    typedef std::pair<CodePoint, size_t> CodePointSizePair;

    struct LoadedGlyph
    {
        GLuint texture;
        Rect texcoords;

        int leftSideBearing;
        int advance;

        Vector2 origin;
        Rect bounds;
    };

    typedef std::map<CodePointSizePair, LoadedGlyph> GlyphMap;
    GlyphMap mGlyphs;

    const LoadedGlyph &getGlyph(CodePoint codePoint, size_t size);
    static CodePoint decodeCodepoint(const char* data, size_t *curIdx);
};

class FontRenderer
{
public:
    FontRenderer();
    ~FontRenderer();

    bool initialize();

    void renderText(const std::vector<Glyph> &glyphs, const Vector2 &position, const Vector4 &color);

private:
    size_t mVertexBufferSize;
    GLuint mVertexBuffer;

    GLuint mProgram;

    GLint mPositionLoc;
    GLint mTexCoordLoc;

    GLuint mTransformUniform;
    GLuint mTextureUniform;
    GLuint mColorUniform;
};

bool LoadTTFFontFromFile(const std::string &path, Font *font);

#endif // SAMPLE_UTIL_FONT_H
