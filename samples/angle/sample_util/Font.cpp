//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "Font.h"
#include "Matrix.h"
#include "shader_utils.h"

#include <fstream>
#include <algorithm>
#include <assert.h>

Font::~Font()
{
    for (size_t i = 0; i < mPages.size(); i++)
    {
        glDeleteTextures(1, &mPages[i].texture);
    }
}

Font::GlyphPage::GlyphPage(GLuint texture, const Vector2 &size)
    : texture(texture), packer(size)
{
}

bool Font::loadData(const std::vector<uint8_t> &fontData)
{
    mFontData = fontData;
    int result = stbtt_InitFont(&mFont, mFontData.data(), 0);
    if (!result)
    {
        mFontData.clear();
    }
    return (result != 0);
}

void Font::prepareText(const std::string &text, size_t fontSize, Vector2 *outSize, std::vector<Glyph> *outGlyphs)
{
    // loadData must be called first
    assert(!mFontData.empty());

    const LoadedGlyph &spaceGlyph = getGlyph(' ', fontSize);

    int ascend;
    int descend;
    int lineGap;
    stbtt_GetFontVMetrics(&mFont, &ascend, &descend, &lineGap);

    float scale = stbtt_ScaleForMappingEmToPixels(&mFont, fontSize);

    Vector2 position(0, -(ascend * scale));
    float maxWidth = 0;

    outGlyphs->reserve(text.length());

    CodePoint prevCodePoint = 0;

    size_t i = 0;
    while (i < text.length())
    {
        CodePoint curCodePoint = decodeCodepoint(text, &i);

        switch (curCodePoint)
        {
          case ' ' : position.x += spaceGlyph.advance * scale;                               continue;
          case '\t': position.x += spaceGlyph.advance * 4 * scale;                           continue;
          case '\n': position.y -= (ascend - descend + lineGap) * scale;     position.x = 0; continue;
          case '\v': position.y -= (ascend - descend + lineGap) * 4 * scale; position.x = 0; continue;
        }

        // Normal character
        const LoadedGlyph &glyph = getGlyph(curCodePoint, fontSize);

        Glyph outGlyph;
        outGlyph.bounds = Rect(position.x + glyph.bounds.x, position.y + glyph.bounds.y,
                               glyph.bounds.w, glyph.bounds.h);
        outGlyph.texture = glyph.texture;
        outGlyph.texcoords = glyph.texcoords;

        outGlyphs->push_back(outGlyph);

        int kern = stbtt_GetCodepointKernAdvance(&mFont, prevCodePoint, curCodePoint);

        position.x += (glyph.advance - glyph.leftSideBearing + kern) * scale;
        maxWidth = std::max(position.x, maxWidth);

        prevCodePoint = curCodePoint;
    }

    if (outSize)
    {
        outSize->x = maxWidth;
        outSize->y = (-position.y) - (descend * scale);
    }
}

const Font::LoadedGlyph &Font::getGlyph(CodePoint codePoint, size_t size)
{
    CodePointSizePair pair(codePoint, size);
    if (mGlyphs.find(pair) == mGlyphs.end())
    {
        GLint boundTexture = 0;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTexture);

        float scale = stbtt_ScaleForMappingEmToPixels(&mFont, size);

        int xOffset, yOffset, width, height;
        unsigned char* bitMap = stbtt_GetCodepointBitmap(&mFont, scale, scale, codePoint, &width, &height, &xOffset, &yOffset);

        const Vector2 textureSize(512, 512);

        // Find a place to put the glyph in the glyph pages
        bool packed = false;
        GLuint packTexture = 0;
        Vector2 packPosition;

        for (size_t i = 0; i < mPages.size(); i++)
        {
            // Pad the pack position with one pixel so that filtering will
            // not pick up adjacent glyphs

            if (mPages[i].packer.insert(Vector2(width + 1, height + 1), &packPosition))
            {
                packTexture = mPages[i].texture;
                packed = true;
                break;
            }
        }

        if (!packed)
        {
            // Allocate a new glyph page
            GLuint newTexture;
            glGenTextures(1, &newTexture);
            glBindTexture(GL_TEXTURE_2D, newTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, textureSize.x, textureSize.y, 0, GL_LUMINANCE,
                         GL_UNSIGNED_BYTE, NULL);

            // Set the filtering mode
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            GlyphPage newPage(newTexture, textureSize);

            packTexture = newTexture;
            bool packResult = newPage.packer.insert(Vector2(width + 1, height + 1), &packPosition);
            assert(packResult);

            mPages.push_back(newPage);
        }

        glBindTexture(GL_TEXTURE_2D, packTexture);

        // Use tightly packed data
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexSubImage2D(GL_TEXTURE_2D, 0, packPosition.x, packPosition.y, width, height,
                        GL_LUMINANCE, GL_UNSIGNED_BYTE, bitMap);

        LoadedGlyph loadedglyph;
        loadedglyph.texture = packTexture;
        loadedglyph.texcoords = Rect(packPosition.x / textureSize.x, (packPosition.y + height) / textureSize.y,
                                     width / textureSize.x,          -height / textureSize.y);
        stbtt_GetCodepointHMetrics(&mFont, codePoint, &loadedglyph.advance, &loadedglyph.leftSideBearing);
        loadedglyph.origin = Vector2(xOffset, yOffset);
        loadedglyph.bounds = Rect(xOffset, -(height + yOffset), width, height);

        mGlyphs[CodePointSizePair(codePoint, size)] = loadedglyph;

        glBindTexture(GL_TEXTURE_2D, boundTexture);
    }

    return mGlyphs[pair];
}

Font::CodePoint Font::decodeCodepoint(const std::string &data, size_t *curIdx)
{
    uint8_t lead = data[(*curIdx)++];
    if(lead < 192)
    {
        return lead;
    }

    size_t trailSize = 0;
    if(lead < 224)
    {
        trailSize = 1;
    }
    else if(lead < 240)
    {
        trailSize = 2;
    }
    else
    {
        trailSize = 3;
    }

    CodePoint c = lead & ((1 << (6 - trailSize)) - 1);
    switch(trailSize)
    {
      case 3: c = (c << 6) | ( static_cast<uint8_t>(data[(*curIdx)++]) & 0x3F);
      case 2: c = (c << 6) | ( static_cast<uint8_t>(data[(*curIdx)++]) & 0x3F);
      case 1: c = (c << 6) | ( static_cast<uint8_t>(data[(*curIdx)++]) & 0x3F);
    }

    return c;
}

bool LoadTTFFontFromFile(const std::string &path, Font *font)
{
    std::ifstream is(path, std::ifstream::binary);
    if (!is)
    {
        return false;
    }

    is.seekg(0, is.end);
    int length = is.tellg();
    is.seekg(0, is.beg);

    std::vector<uint8_t> fileData(length);
    is.read(reinterpret_cast<char*>(fileData.data()), length);

    return font->loadData(fileData);
}

FontRenderer::FontRenderer()
    : mVertexBuffer(0),
      mProgram(0),
      mColorUniform(0)
{
}

FontRenderer::~FontRenderer()
{
    glDeleteBuffers(1, &mVertexBuffer);
    glDeleteProgram(mProgram);
}

bool FontRenderer::initialize()
{
    // Get the currently bound array buffer so it can be re-applied
    GLint prevArrayBuffer = 0;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prevArrayBuffer);

    // Generate the vertex buffer to hold mVertexBufferSize floats
    mVertexBufferSize = 2048;
    glGenBuffers(1, &mVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, mVertexBufferSize, NULL, GL_DYNAMIC_DRAW);

    // Re-apply the old vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, prevArrayBuffer);

    const std::string vs = SHADER_SOURCE
    (
        precision mediump float;\n

        attribute vec4 a_position;\n
        attribute vec2 a_texCoord;\n

        varying vec2 v_texCoord;\n

        uniform mat4 u_transform;\n

        void main()\n
        {\n
            gl_Position = u_transform * a_position;\n
            gl_Position.z = 0.5; \n
            v_texCoord = a_texCoord;\n
        }\n
    );

    const std::string fs = SHADER_SOURCE
    (
        precision mediump float;\n

        varying vec2 v_texCoord;\n

        uniform vec4 u_color;\n
        uniform sampler2D s_texture;\n

        void main()\n
        {\n
            gl_FragColor = texture2D(s_texture, v_texCoord) * u_color;\n
        }\n
    );

    mProgram = CompileProgram(vs, fs);
    if (!mProgram)
    {
        return false;
    }

    mPositionLoc = glGetAttribLocation(mProgram, "a_position");
    mTexCoordLoc = glGetAttribLocation(mProgram, "a_texCoord");

    mTransformUniform = glGetUniformLocation(mProgram, "u_transform");
    mTextureUniform = glGetUniformLocation(mProgram, "s_texture");
    mColorUniform = glGetUniformLocation(mProgram, "u_color");

    return true;
}

void FontRenderer::renderText(const std::vector<Glyph> &glyphs, const Vector2 &position, const Vector4 &color)
{
    // Early out so we can assume glyphs has at least one element
    if (glyphs.empty())
    {
        return;
    }

    // Grab the viewport size to generate the orthographic projection matrix
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    GLint textureUnit = 0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &textureUnit);

    Matrix4 transform = Matrix4::ortho(0.0f, viewport[2], 0.0f, viewport[3], 0.0f, 1.0f);

    // Save the previously applied array buffer, texture and program
    GLint prevArrayBuffer = 0;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prevArrayBuffer);

    GLint prevTexture = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevTexture);

    GLint prevProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prevProgram);

    GLboolean blendEnabled = glIsEnabled(GL_BLEND);

    GLint sourceBlend = 0, destBlend = 0;
    glGetIntegerv(GL_BLEND_SRC_RGB, &sourceBlend);
    glGetIntegerv(GL_BLEND_DST_RGB, &destBlend);

    // TODO: Save vertex attribute state or use VAOs

    // Apply the uniforms
    glUseProgram(mProgram);
    glUniformMatrix4fv(mTransformUniform, 1, GL_FALSE, transform.data);
    glUniform4fv(mColorUniform, 1, color.data);

    glBindTexture(GL_TEXTURE_2D, glyphs[0].texture);
    glUniform1i(mTextureUniform, textureUnit - GL_TEXTURE0);

    // Apply the vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);

    // Apply the vertex attributes
    const size_t bytePerVertex = sizeof(Vector2) * 2;
    const size_t bytesPerQuad = bytePerVertex * 6;

    glVertexAttribPointer(mPositionLoc, 2, GL_FLOAT, GL_FALSE, bytePerVertex, NULL);
    glEnableVertexAttribArray(mPositionLoc);

    glVertexAttribPointer(mTexCoordLoc, 2, GL_FLOAT, GL_FALSE, bytePerVertex, reinterpret_cast<void*>(sizeof(Vector2)));
    glEnableVertexAttribArray(mTexCoordLoc);

    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    // Generate the geometry
    GLuint curTexture = glyphs[0].texture;
    size_t bufferPos = 0;
    for (size_t i = 0; i < glyphs.size(); i++)
    {
        // If the buffer is out of space, draw the quads and reset the position
        if (bufferPos + bytesPerQuad > mVertexBufferSize)
        {
            size_t quadCount = bufferPos / bytesPerQuad;
            glDrawArrays(GL_TRIANGLES, 0, quadCount * 6);

            bufferPos = 0;
        }

        const Rect &bounds = glyphs[i].bounds;
        const Rect &texcoords = glyphs[i].texcoords;
        Vector2 vertices[12] =
        {
            position + Vector2(bounds.x, bounds.y),
            Vector2(texcoords.x, texcoords.y),

            position + Vector2(bounds.x, bounds.y + bounds.h),
            Vector2(texcoords.x, texcoords.y + texcoords.h),

            position + Vector2(bounds.x + bounds.w, bounds.y),
            Vector2(texcoords.x + texcoords.w, texcoords.y),

            position + Vector2(bounds.x, bounds.y + bounds.h),
            Vector2(texcoords.x, texcoords.y + texcoords.h),

            position + Vector2(bounds.x + bounds.w, bounds.y + bounds.h),
            Vector2(texcoords.x + texcoords.w, texcoords.y + texcoords.h),

            position + Vector2(bounds.x + bounds.w, bounds.y),
            Vector2(texcoords.x + texcoords.w, texcoords.y),
        };
        assert(sizeof(vertices) == bytesPerQuad);

        glBufferSubData(GL_ARRAY_BUFFER, bufferPos, bytesPerQuad, vertices);
        bufferPos += bytesPerQuad;

        // If the texture has changed, draw the quads in the buffer and swap the texture
        if (curTexture != glyphs[i].texture)
        {
            if (bufferPos > 0)
            {
                size_t quadCount = bufferPos / bytesPerQuad;
                glDrawArrays(GL_TRIANGLES, 0, quadCount * 6);

                bufferPos = 0;
            }

            glBindTexture(GL_TEXTURE_2D, glyphs[i].texture);
            curTexture = glyphs[i].texture;
        }
    }

    // Draw the remaining quads in the buffer
    if (bufferPos > 0)
    {
        size_t quadCount = bufferPos / bytesPerQuad;
        glDrawArrays(GL_TRIANGLES, 0, quadCount * 6);
    }

    // Re-apply the previous state
    glUseProgram(prevProgram);
    glBindTexture(GL_TEXTURE_2D, prevTexture);
    glBindBuffer(GL_ARRAY_BUFFER, prevArrayBuffer);

    if (blendEnabled)
    {
        glEnable(GL_BLEND);
    }
    else
    {
        glDisable(GL_BLEND);
    }
    glBlendFunc(sourceBlend, destBlend);
}
