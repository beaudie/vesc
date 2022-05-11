//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Copyright 2022 Rive
//

#include <sstream>
#include <string>
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

////////////////////////////////////////////////////////////////////////////////////////////////////
// GL_ANGLE_shader_noncoherent_pixel_local_storage prototype.
//
// NOTE: the intention is for this to eventually move into ANGLE.

#define GL_DISABLED_ANGLE 0xbaadbeef

constexpr static int MAX_LOCAL_STORAGE_PLANES                = 3;
constexpr static int MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE = 1;

// ES 3.1 unfortunately requires most image formats to be either readonly or writeonly. (My
// understanding is that this limitation exists due to atomics; a read-modify-write operation is
// still valid as long as the appropriate barriers and "volatile" qualifiers and are in place.) To
// work around this limitation, we create two volatile views of the same image -- one for reading
// and one for writing.
//
// We mark the image variables as "volatile" since we are referencing the same memory with separate
// readonly and writeonly views. (Variables declared as volatile are automatically treated as
// "coherent" as well.)
//
// We bind the readonly view at i*2, and the writeonly at i*2+1 (where "i" is the local storage
// plane index).
struct pixelLocal
{
    pixelLocal(int unit, const char *format, const char *name)
        : unit(unit), format(format), name(name)
    {}
    int unit;
    const char *format;
    const char *name;
};

std::ostream &operator<<(std::ostream &os, const pixelLocal &decl)
{
    os << "layout(binding=" << (decl.unit + 1) << ") highp uniform sampler2D " << decl.name
       << "_R;\n";
    os << "layout(binding=" << decl.unit << ", " << decl.format
       << ") coherent volatile writeonly highp uniform image2D " << decl.name << "_W;\n";
    return os;
}

// The compiler doesn't appear to support macro concatenation (e.g., NAME ## _R). For now, the
// client code is responsible to know there are two image variables and append _R for pixelLocalLoad
// and _W for pixelLocalStore.
constexpr static const char kLocalStorageGLSLDefines[] = R"(
#define pixelLocalLoad(NAME_R) \
    (memoryBarrier(), texelFetch(NAME_R, ivec2(floor(gl_FragCoord.xy)), 0))
#define pixelLocalStore(NAME_W, VALUE) { \
    memoryBarrier(); \
    imageStore(NAME_W, ivec2(floor(gl_FragCoord.xy)), VALUE); \
    memoryBarrier(); \
}
layout(early_fragment_tests) in;)";

class PixelLocalStoragePrototype
{
  public:
    void framebufferPixelLocalStorage(GLuint unit,
                                      GLuint backingtexture,
                                      GLint level,
                                      GLint layer,
                                      GLint width,
                                      GLint height,
                                      GLenum internalformat);
    void beginPixelLocalStorage(GLsizei n, const GLenum *loadOps);
    void pixelLocalStorageBarrier();
    void endPixelLocalStorage();

  private:
    struct LocalStoragePlane
    {
        GLuint tex;
        GLsizei width;
        GLsizei height;
        GLenum internalformat;
    };
    std::array<LocalStoragePlane, MAX_LOCAL_STORAGE_PLANES> &boundLocalStoragePlanes()
    {
        GLint drawFBO;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFBO);
        assert(drawFBO != 0);  // GL_INVALID_OPERATION!
        return mLocalStoragePlanes[drawFBO];
    }
    std::map<GLuint, std::array<LocalStoragePlane, MAX_LOCAL_STORAGE_PLANES>> mLocalStoragePlanes;
    int mNumLocalStoragePlanesEnabled = false;
    std::vector<int> mEnabledLocalStoragePlanes;
    GLint mFramebufferPreviousDefaultWidth  = 0;
    GLint mFramebufferPreviousDefaultHeight = 0;
};

// Expose the draft extension assuming an in-scope PixelLocalStoragePrototype object named "pls".
#define glFramebufferPixelLocalStorageANGLE pls.framebufferPixelLocalStorage
#define glBeginPixelLocalStorageANGLE pls.beginPixelLocalStorage
#define glPixelLocalStorageBarrierANGLE pls.pixelLocalStorageBarrier
#define glEndPixelLocalStorageANGLE pls.endPixelLocalStorage

void PixelLocalStoragePrototype::framebufferPixelLocalStorage(GLuint unit,
                                                              GLuint backingtexture,
                                                              GLint level,
                                                              GLint layer,
                                                              GLsizei width,
                                                              GLsizei height,
                                                              GLenum internalformat)
{
    assert(0 <= unit && unit < MAX_LOCAL_STORAGE_PLANES);  // GL_INVALID_VALUE!
    assert(backingtexture != 0);                           // NOT IMPLEMENTED!
    assert(level == 0);                                    // NOT IMPLEMENTED!
    assert(layer == 0);                                    // NOT IMPLEMENTED!
    assert(width > 0 && height > 0);                       // NOT IMPLEMENTED!
    boundLocalStoragePlanes()[unit] = {backingtexture, width, height, internalformat};
}

// This RAII class saves and restores the necessary GL state for us to clear local storage backing
// textures.
class AutoRestoreClearState
{
  public:
    AutoRestoreClearState()
    {
        GLint MAX_COLOR_ATTACHMENTS;
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &MAX_COLOR_ATTACHMENTS);

        GLint MAX_DRAW_BUFFERS;
        glGetIntegerv(GL_MAX_DRAW_BUFFERS, &MAX_DRAW_BUFFERS);

        for (int i = 0; i < MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE; ++i)
        {
            GLint drawBuffer;
            glGetIntegerv(GL_DRAW_BUFFER0 + i, &drawBuffer);
            // glDrawBuffers must not reference an attachment at or beyond
            // MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE.
            if (GL_COLOR_ATTACHMENT0 <= drawBuffer &&
                drawBuffer < GL_COLOR_ATTACHMENT0 + MAX_COLOR_ATTACHMENTS)
            {
                // GL_INVALID_OPERATION!
                assert(drawBuffer < GL_COLOR_ATTACHMENT0 + MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE);
            }
            mDrawBuffers[i] = drawBuffer;
        }
        // glDrawBuffers must be GL_NONE at or beyond MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE.
        for (int i = MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE; i < MAX_DRAW_BUFFERS; ++i)
        {
            GLint drawBuffer;
            glGetIntegerv(GL_DRAW_BUFFER0 + i, &drawBuffer);
            assert(drawBuffer == GL_NONE);  // GL_INVALID_OPERATION!
        }

        glGetIntegerv(GL_SCISSOR_TEST, &mScissorTestEnabled);

        glGetFloatv(GL_COLOR_CLEAR_VALUE, mClearColor);
    }

    ~AutoRestoreClearState()
    {
        glDrawBuffers(MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE, mDrawBuffers);
        if (mScissorTestEnabled)
        {
            glEnable(GL_SCISSOR_TEST);
        }
        glClearColor(mClearColor[0], mClearColor[1], mClearColor[2], mClearColor[3]);
    }

    bool scissorTestEnabled() const { return mScissorTestEnabled; }

  private:
    GLenum mDrawBuffers[MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE];
    GLint mScissorTestEnabled;
    float mClearColor[4];
};

void PixelLocalStoragePrototype::beginPixelLocalStorage(GLsizei n, const GLenum *loadOps)
{
    assert(1 <= n && n <= MAX_LOCAL_STORAGE_PLANES);  // GL_INVALID_VALUE!
    assert(!mNumLocalStoragePlanesEnabled);           // GL_INVALID_OPERATION!
    assert(mEnabledLocalStoragePlanes.empty());

    mNumLocalStoragePlanesEnabled = n;

    const auto &planes = boundLocalStoragePlanes();

    // A framebuffer must have no attachments at or beyond MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE.
    GLint MAX_COLOR_ATTACHMENTS;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &MAX_COLOR_ATTACHMENTS);
    for (int i = MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE; i < MAX_COLOR_ATTACHMENTS; ++i)
    {
        GLint type;
        glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                                              GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
        assert(type == GL_NONE);  // GL_INVALID_OPERATION!
    }

    int framebufferWidth  = 0;
    int framebufferHeight = 0;
    bool needsClear       = false;
    GLenum attachmentsToClear[MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE + MAX_LOCAL_STORAGE_PLANES];
    memset(attachmentsToClear, 0, sizeof(attachmentsToClear));

    GLint activeTexture;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);

    for (int i = 0; i < n; ++i)
    {
        GLuint tex            = 0;
        GLenum internalformat = GL_RGBA8;
        if (loadOps[i] != GL_DISABLED_ANGLE)
        {
            assert(planes[i].tex);  // GL_INVALID_FRAMMEBUFFER_OPERATION!
            tex            = planes[i].tex;
            internalformat = planes[i].internalformat;

            // GL_INVALID_FRAMMEBUFFER_OPERATION!
            assert(!framebufferWidth || framebufferWidth == planes[i].width);
            assert(!framebufferHeight || framebufferHeight == planes[i].height);
            framebufferWidth  = planes[i].width;
            framebufferHeight = planes[i].height;

            mEnabledLocalStoragePlanes.push_back(i);
        }
        if (loadOps[i] == GL_ZERO)
        {
            // Attach all textures that need clearing to the framebuffer.
            GLenum attachmentPoint =
                GL_COLOR_ATTACHMENT0 + MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE + i;
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentPoint, GL_TEXTURE_2D, tex, 0);
            // If the GL is bound to a draw framebuffer object, the ith buffer listed in bufs must
            // be GL_COLOR_ATTACHMENTi or GL_NONE.
            needsClear                                                      = true;
            attachmentsToClear[MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE + i] = attachmentPoint;
        }
        // Bind all local storage planes to image textures for shader access. Tell the driver both
        // bindings are GL_READ_WRITE. This seems more descriptive of our intent, even though one
        // will be used exclusively for reading and the other for writing. (e.g., if the texture
        // gets bound separately as GL_READ_ONLY and GL_WRITE_ONLY, does that mean _can't_ read _or_
        // write??)
        glActiveTexture(GL_TEXTURE0 + i + 1);
        glBindTexture(GL_TEXTURE_2D, tex);
        glBindImageTexture(i, tex, 0, GL_FALSE, 0, GL_READ_WRITE, internalformat);
    }
    if (needsClear)
    {
        {
            AutoRestoreClearState arcs;
            glDrawBuffers(MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE + n, attachmentsToClear);
            if (arcs.scissorTestEnabled())
            {
                glDisable(GL_SCISSOR_TEST);
            }
            // TODO: We should use glClearBuffer here.
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        // Detach the textures that needed clearing.
        for (int i = 0; i < n; ++i)
        {
            if (loadOps[i] == GL_ZERO)
            {
                glFramebufferTexture2D(
                    GL_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0 + MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE + i,
                    GL_TEXTURE_2D, 0, 0);
            }
        }
    }

    glActiveTexture(activeTexture);

    glGetFramebufferParameteriv(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH,
                                &mFramebufferPreviousDefaultWidth);
    glGetFramebufferParameteriv(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT,
                                &mFramebufferPreviousDefaultHeight);
    glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, framebufferWidth);
    glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, framebufferHeight);

    // Do *ALL* barriers since we don't know what the client will do with memory after this.
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void PixelLocalStoragePrototype::pixelLocalStorageBarrier()
{
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void PixelLocalStoragePrototype::endPixelLocalStorage()
{
    assert(mNumLocalStoragePlanesEnabled);  // GL_INVALID_OPERATION!

    // Do *ALL* barriers since we don't know what the client will do with memory after this.
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // Other backends already detached these.
    if (IsOpenGL())
    {
        for (int i = 0; i < mNumLocalStoragePlanesEnabled; ++i)
        {
            glFramebufferTexture2D(
                GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE + i,
                GL_TEXTURE_2D, 0, 0);
        }
    }

    // Restore framebuffer default dimensions.
    glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH,
                            mFramebufferPreviousDefaultWidth);
    glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT,
                            mFramebufferPreviousDefaultHeight);

    GLint activeTexture;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);

    // Unbind local storage image textures.
    for (int i : mEnabledLocalStoragePlanes)
    {
        glActiveTexture(GL_TEXTURE0 + i + 1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindImageTexture(i, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
    }
    mEnabledLocalStoragePlanes.clear();

    glActiveTexture(activeTexture);

    mNumLocalStoragePlanesEnabled = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr static int W = 128, H = 128;

class PixelLocalStorageTest : public ANGLETest
{
  protected:
    PixelLocalStorageTest()
    {
        setWindowWidth(1);
        setWindowHeight(1);
    }

    ~PixelLocalStorageTest()
    {
        if (mScratchFBO)
        {
            glDeleteFramebuffers(1, &mScratchFBO);
        }
    }

    bool supportsPixelLocalStorage()
    {
        ASSERT(getClientMajorVersion() == 3);
        ASSERT(getClientMinorVersion() == 1);

        if (isD3D11Renderer())
        {
            // D3D11 can't support aliased readonly/writeonly shader images. It will need a custom
            // local storage implementation that uses RWTexture2D, or ideally,
            // RasterizerOrderedTexture2D. https://anglebug.com/3032
            return false;
        }

        return true;
    }

    void useProgram(std::initializer_list<pixelLocal> pixelLocalDecls, const char fsMain[])
    {
        const char *vs = R"(#version 310 es
        precision highp float;

        uniform float W, H;
        uniform vec4 rect;

        void main() {
            gl_Position.x = ((gl_VertexID & 1) == 0 ? rect.x : rect.z) * 2.0/W - 1.0;
            gl_Position.y = ((gl_VertexID & 2) == 0 ? rect.y : rect.w) * 2.0/H - 1.0;
            gl_Position.zw = vec2(0, 1);
        })";

        std::ostringstream fs;
        fs << R"(#version 310 es
        precision mediump float;
        uniform vec4 color;)";
        fs << kLocalStorageGLSLDefines;
        for (const auto &decl : pixelLocalDecls)
        {
            fs << decl;
        }
        fs << fsMain;

        mProgram.makeRaster(vs, fs.str().c_str());

        ASSERT_TRUE(mProgram.valid());

        glUseProgram(mProgram);

        glUniform1f(glGetUniformLocation(mProgram, "W"), W);
        glUniform1f(glGetUniformLocation(mProgram, "H"), H);

        mRectUniform = glGetUniformLocation(mProgram, "rect");
        ASSERT_TRUE(mRectUniform >= 0);

        mColorUniform = glGetUniformLocation(mProgram, "color");
        ASSERT_TRUE(mColorUniform >= 0);
    }

    void attachTextureToScratchFBO(GLuint tex)
    {
        if (!mScratchFBO)
        {
            glGenFramebuffers(1, &mScratchFBO);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, mScratchFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    }

    GLProgram mProgram;
    GLint mRectUniform  = -1;
    GLint mColorUniform = -1;

    GLuint mScratchFBO = 0;
};

// Verify that R,G,B values from separate draw calls persist in pixel local storage.
TEST_P(PixelLocalStorageTest, RGB)
{
    if (!supportsPixelLocalStorage())
    {
        return;
    }

    PixelLocalStoragePrototype pls;

    // Accumulate R, G, B in 3 separate passes.
    useProgram({pixelLocal(0, "rgba8", "framebuffer")}, R"(
    void main() {
        vec4 dst = pixelLocalLoad(framebuffer_R);
        pixelLocalStore(framebuffer_W, color + dst);
    })");

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, W, H);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferPixelLocalStorageANGLE(0, tex, 0, 0, W, H, GL_RGBA8);
    glViewport(0, 0, W, H);
    glDrawBuffers(0, nullptr);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBeginPixelLocalStorageANGLE(1, std::vector<GLenum>{GL_ZERO}.data());

    // Draw fullscreen rects.
    glUniform4f(mRectUniform, 0, 0, W, H);

    glUniform4f(mColorUniform, 1, 0, 0, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glPixelLocalStorageBarrierANGLE();

    glUniform4f(mColorUniform, 0, 1, 0, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glPixelLocalStorageBarrierANGLE();

    glUniform4f(mColorUniform, 0, 0, 1, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glPixelLocalStorageBarrierANGLE();

    glEndPixelLocalStorageANGLE();

    attachTextureToScratchFBO(tex);
    EXPECT_PIXEL_RECT_EQ(0, 0, W, H, GLColor(255, 255, 255, 0));

    ASSERT_GL_NO_ERROR();
}

ANGLE_INSTANTIATE_TEST_ES31(PixelLocalStorageTest);
