//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Copyright 2022 Rive
//

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
constexpr static int MAX_LOCAL_STORAGE_BYTES                 = 16;
constexpr static int MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE = 1;

// ES 3.1 unfortunately requires most image formats to be either readonly or writeonly. (My
// understanding is that this limitation exists due to atomics; a read-modify-write operation is
// still valid as long as the appropriate barriers and "volatile" qualifiers and are in place.) To
// work around this limitation, we create two volatile views of the same image -- one for reading
// and one for writing.
//
// We mark the image variables as "coherent" so the compiler knows different shader invocations can
// modify the same pixel (with a barrier in between).
//
// We mark the image variables as "volatile" since we are referencing the same memory with separate
// readonly and writeonly views.
//
// We bind the readonly view at i*2, and the writeonly at i*2+1 (where "i" is the local storage
// plane index). For now, the client code is responsible to know this and manage the specific
// binding points and names.
constexpr static const char kLocalStorageGLSLDefines[] = R"(
#define DECLARE_LOCAL_STORAGE(FORMAT, BINDING_R, NAME_R, BINDING_W, NAME_W) \
    layout(FORMAT, binding=BINDING_R) volatile coherent readonly highp uniform image2D NAME_R; \
    layout(FORMAT, binding=BINDING_W) volatile coherent writeonly highp uniform image2D NAME_W
#define DECLARE_LOCAL_STORAGE_I(FORMAT, BINDING_R, NAME_R, BINDING_W, NAME_W) \
    layout(FORMAT, binding=BINDING_R) volatile coherent readonly highp uniform iimage2D NAME_R; \
    layout(FORMAT, binding=BINDING_W) volatile coherent writeonly highp uniform iimage2D NAME_W
#define DECLARE_LOCAL_STORAGE_UI(FORMAT, BINDING_R, NAME_R, BINDING_W, NAME_W) \
    layout(FORMAT, binding=BINDING_R) volatile coherent readonly highp uniform uimage2D NAME_R; \
    layout(FORMAT, binding=BINDING_W) volatile coherent writeonly highp uniform uimage2D NAME_W
#define pixelLocalLoad(NAME_R) imageLoad(NAME_R, ivec2(floor(gl_FragCoord.xy)))
#define pixelLocalStore(NAME_W, VALUE) imageStore(NAME_W, ivec2(floor(gl_FragCoord.xy)), VALUE)
)";

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
    void framebufferPixelLocalClearValuefv(GLuint unit, const float *value);
    void framebufferPixelLocalClearValueiv(GLuint unit, const GLint *value);
    void framebufferPixelLocalClearValueuiv(GLuint unit, const GLuint *value);
    void beginPixelLocalStorage(GLsizei n, const GLenum *loadOps);
    void pixelLocalStorageBarrier();
    void endPixelLocalStorage();

  private:
    class BackingTexture
    {
      public:
        BackingTexture()                                  = default;
        BackingTexture(const BackingTexture &)            = delete;
        BackingTexture &operator=(const BackingTexture &) = delete;

        void reset(GLuint tex, GLsizei width, GLsizei height, GLuint internalformat)
        {
            if (mID && mMemoryless)
            {
                glDeleteTextures(1, &mID);
            }
            mMemoryless = !tex;
            if (mMemoryless)
            {
                GLint textureBinding2D;
                glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureBinding2D);
                glGenTextures(1, &mID);
                glBindTexture(GL_TEXTURE_2D, mID);
                glTexStorage2D(GL_TEXTURE_2D, 1, internalformat, width, height);
                glBindTexture(GL_TEXTURE_2D, textureBinding2D);
            }
            else
            {
                mID = tex;
            }
            mInternalformat = internalformat;
        }

        ~BackingTexture()
        {
            if (mID && mMemoryless)
            {
                glDeleteTextures(1, &mID);
            }
        }

        GLuint id() const { return mID; }
        GLenum internalformat() const { return mInternalformat; }

      private:
        bool mMemoryless;
        GLuint mID = 0;
        GLenum mInternalformat;
    };

    struct LocalStoragePlane
    {
        BackingTexture backingTexture;
        GLfloat clearValuef[4]{};
        GLint clearValuei[4]{};
        GLuint clearValueui[4]{};
    };

    std::array<LocalStoragePlane, MAX_LOCAL_STORAGE_PLANES> &boundLocalStoragePlanes()
    {
        GLint drawFBO;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFBO);
        assert(drawFBO != 0);  // GL_INVALID_OPERATION!
        return mLocalStoragePlanes[drawFBO];
    }

    std::map<GLuint, std::array<LocalStoragePlane, MAX_LOCAL_STORAGE_PLANES>> mLocalStoragePlanes;
    int mNumEnabledLocalStoragePlanes = 0;
};

// Expose the draft extension assuming an in-scope PixelLocalStoragePrototype object named "pls".
#define glFramebufferPixelLocalStorageANGLE pls.framebufferPixelLocalStorage
#define glFramebufferPixelLocalClearValuefvANGLE pls.framebufferPixelLocalClearValuefv
#define glFramebufferPixelLocalClearValueivANGLE pls.framebufferPixelLocalClearValueiv
#define glFramebufferPixelLocalClearValueuivANGLE pls.framebufferPixelLocalClearValueuiv
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
    assert(level == 0);                                    // NOT IMPLEMENTED!
    assert(layer == 0);                                    // NOT IMPLEMENTED!
    assert(width > 0 && height > 0);                       // NOT IMPLEMENTED!
    boundLocalStoragePlanes()[unit].backingTexture.reset(backingtexture, width, height,
                                                         internalformat);
}

void PixelLocalStoragePrototype::framebufferPixelLocalClearValuefv(GLuint unit,
                                                                   const GLfloat *value)
{
    assert(0 <= unit && unit < MAX_LOCAL_STORAGE_PLANES);  // GL_INVALID_VALUE!
    memcpy(boundLocalStoragePlanes()[unit].clearValuef, value, sizeof(GLfloat) * 4);
}

void PixelLocalStoragePrototype::framebufferPixelLocalClearValueiv(GLuint unit, const GLint *value)
{
    assert(0 <= unit && unit < MAX_LOCAL_STORAGE_PLANES);  // GL_INVALID_VALUE!
    memcpy(boundLocalStoragePlanes()[unit].clearValuei, value, sizeof(GLint) * 4);
}

void PixelLocalStoragePrototype::framebufferPixelLocalClearValueuiv(GLuint unit,
                                                                    const GLuint *value)
{
    assert(0 <= unit && unit < MAX_LOCAL_STORAGE_PLANES);  // GL_INVALID_VALUE!
    memcpy(boundLocalStoragePlanes()[unit].clearValueui, value, sizeof(GLuint) * 4);
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
    }

    ~AutoRestoreClearState()
    {
        glDrawBuffers(MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE, mDrawBuffers);
        if (mScissorTestEnabled)
        {
            glEnable(GL_SCISSOR_TEST);
        }
    }

    bool scissorTestEnabled() const { return mScissorTestEnabled; }

  private:
    GLenum mDrawBuffers[MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE];
    GLint mScissorTestEnabled;
};

void PixelLocalStoragePrototype::beginPixelLocalStorage(GLsizei n, const GLenum *loadOps)
{
    assert(1 <= n && n <= MAX_LOCAL_STORAGE_PLANES);  // GL_INVALID_VALUE!
    assert(mNumEnabledLocalStoragePlanes == 0);       // GL_INVALID_OPERATION!

    const auto &localStoragePlanes = boundLocalStoragePlanes();

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

    GLenum attachmentsToClear[MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE + MAX_LOCAL_STORAGE_PLANES];
    memset(attachmentsToClear, 0, sizeof(attachmentsToClear));
    bool needsClear = false;

    for (int i = 0; i < n; ++i)
    {
        GLuint tex            = 0;
        GLenum internalformat = GL_RGBA8;
        if (loadOps[i] != GL_DISABLED_ANGLE)
        {
            tex            = localStoragePlanes[i].backingTexture.id();
            internalformat = localStoragePlanes[i].backingTexture.internalformat();
            assert(tex);  // GL_INVALID_FRAMMEBUFFER_OPERATION!
        }
        // Attach all local storage backing textures to the framebuffer, which we will disable via
        // glDrawBuffers during local storage draws. This allows us to easily clear local storage in
        // a way that hints to the driver that this is the beginning of a render pass
        GLenum attachmentPoint = GL_COLOR_ATTACHMENT0 + MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE + i;
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentPoint, GL_TEXTURE_2D, tex, 0);
        if (loadOps[i] == GL_ZERO || loadOps[i] == GL_REPLACE)
        {
            // If the GL is bound to a draw framebuffer object, the ith buffer listed in bufs must
            // be GL_COLOR_ATTACHMENTi or GL_NONE.
            attachmentsToClear[MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE + i] = attachmentPoint;
            needsClear                                                      = true;
        }
        // Bind all local storage planes to image textures for shader access. Tell the driver both
        // bindings are GL_READ_WRITE. This seems more descriptive of our intent, even though one
        // will be used exclusively for reading and the other for writing. (e.g., if the texture
        // gets bound separately as GL_READ_ONLY and GL_WRITE_ONLY, does that mean _can't_ read _or_
        // write??)
        glBindImageTexture(i * 2, tex, 0, GL_FALSE, 0, GL_READ_WRITE, internalformat);
        glBindImageTexture(i * 2 + 1, tex, 0, GL_FALSE, 0, GL_READ_WRITE, internalformat);
    }
    if (needsClear)
    {
        AutoRestoreClearState arcs;
        glDrawBuffers(MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE + n, attachmentsToClear);
        if (arcs.scissorTestEnabled())
        {
            glDisable(GL_SCISSOR_TEST);
        }
        for (int i = 0; i < n; ++i)
        {
            if (loadOps[i] != GL_ZERO && loadOps[i] != GL_REPLACE)
            {
                continue;
            }
            constexpr static GLfloat zerof[4]{};
            constexpr static GLint zeroi[4]{};
            constexpr static GLuint zeroui[4]{};
            switch (localStoragePlanes[i].backingTexture.internalformat())
            {
                case GL_RGBA8:
                case GL_R32F:
                case GL_RGBA16F:
                case GL_RGBA32F:
                    glClearBufferfv(
                        GL_COLOR, MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE + i,
                        loadOps[i] == GL_ZERO ? zerof : localStoragePlanes[i].clearValuef);
                    break;
                case GL_RGBA8I:
                case GL_RGBA16I:
                    glClearBufferiv(
                        GL_COLOR, MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE + i,
                        loadOps[i] == GL_ZERO ? zeroi : localStoragePlanes[i].clearValuei);
                    break;
                case GL_RGBA8UI:
                case GL_R32UI:
                case GL_RGBA16UI:
                case GL_RGBA32UI:
                    glClearBufferuiv(
                        GL_COLOR, MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE + i,
                        loadOps[i] == GL_ZERO ? zeroui : localStoragePlanes[i].clearValueui);
                    break;
                default:
                    assert(false);  // invalid internalformat
            }
        }
    }
    mNumEnabledLocalStoragePlanes = n;

    // TODO: we probably don't need *ALL* barrier bits.
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void PixelLocalStoragePrototype::pixelLocalStorageBarrier()
{
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void PixelLocalStoragePrototype::endPixelLocalStorage()
{
    assert(mNumEnabledLocalStoragePlanes > 0);  // GL_INVALID_OPERATION!

    // TODO: we probably don't need *ALL* barrier bits.
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // Detach pixel-local backing textures from the framebuffer.
    // TODO: This might be slightly more optimal if we skip the ones that were GL_DISABLED_ANGLE,
    // since they're already 0.
    for (int i = 0; i < mNumEnabledLocalStoragePlanes; ++i)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0 + MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE + i,
                               GL_TEXTURE_2D, 0, 0);
    }

    mNumEnabledLocalStoragePlanes = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr static int W = 128, H = 128;
constexpr static std::array<float, 4> FULLSCREEN = {0, 0, W, H};

template <typename U>
struct ReadPixelsArgs
{};
template <>
struct ReadPixelsArgs<uint8_t>
{
    constexpr static GLenum format = GL_RGBA;
    constexpr static GLenum type   = GL_UNSIGNED_BYTE;
};
template <>
struct ReadPixelsArgs<float>
{
    constexpr static GLenum format = GL_RGBA;
    constexpr static GLenum type   = GL_FLOAT;
};
template <>
struct ReadPixelsArgs<int32_t>
{
    constexpr static GLenum format = GL_RGBA_INTEGER;
    constexpr static GLenum type   = GL_INT;
};
template <>
struct ReadPixelsArgs<uint32_t>
{
    constexpr static GLenum format = GL_RGBA_INTEGER;
    constexpr static GLenum type   = GL_UNSIGNED_INT;
};

template <typename T>
void ExpectFramebufferPixels(int x1, int y1, int x2, int y2, std::array<T, 4> expectedColor)
{
    int h = y2 - y1, w = x2 - x1;
    ASSERT(w >= 0);
    ASSERT(h >= 0);
    std::vector<std::array<T, 4>> pixels(h * w);
    glReadPixels(x1, y1, w, h, ReadPixelsArgs<T>::format, ReadPixelsArgs<T>::type, pixels.data());
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            auto pixel = pixels[y * w + x];
            if (pixel != expectedColor)
            {
                printf("FAILED at pixel location [%i, %i]\n", x1 + x, y1 + y);
            }
            ASSERT_EQ(pixel, expectedColor);
        }
    }
}

template <typename T>
void ExpectFramebufferPixels(std::array<T, 4> expectedColor)
{
    ExpectFramebufferPixels(0, 0, W, H, expectedColor);
}

class PixelLocalTexture : public GLTexture
{
  public:
    PixelLocalTexture(GLenum internalformat) : PixelLocalTexture(internalformat, W, H) {}
    PixelLocalTexture(GLenum internalformat, int w, int h)
    {
        glBindTexture(GL_TEXTURE_2D, *this);
        glTexStorage2D(GL_TEXTURE_2D, 1, internalformat, w, h);
    }
};

template <typename T>
struct Array
{
    Array(const std::initializer_list<T> &l) : mVec(l) {}
    operator const T *() const { return mVec.data(); }
    std::vector<T> mVec;
};
template <typename T>
static Array<T> MakeArray(const std::initializer_list<T> &l)
{
    return Array<T>(l);
}
static Array<GLenum> GLenumArray(const std::initializer_list<GLenum> &l)
{
    return Array<GLenum>(l);
}

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
            // Our image shaders fail to compile in D3D11.
            return false;
        }

#ifdef _DEBUG
        if (isVulkanRenderer())
        {
            // We hit internal validation errors on Vulkan. Maybe due to the fact that the local
            // storage backing textures are also bound to color attachments, even though they are
            // turned off via glDrawBuffers?
            return false;
        }
#endif
        return true;
    }

    bool supportsCoherentPixelLocalStorage()
    {
        return false;  // ES 3.1 shader images can't be coherent.
    }

    void useProgram(const char fsMain[])
    {
        compileProgram(mScratchProgram, fsMain);
        useProgram(mScratchProgram);
    }

    void compileProgram(GLProgram &program, const char fsMain[])
    {
        program.makeRaster(
            R"(#version 310 es
            precision highp float;

            uniform float W, H;
            in vec4 inltrb;
            in vec4 incolor;
            in vec4 inaux1;
            in vec4 inaux2;
            out vec4 color;
            out vec4 aux1;
            out vec4 aux2;

            void main() {
                color = incolor;
                aux1 = inaux1;
                aux2 = inaux2;
                gl_Position.x = ((gl_VertexID & 1) == 0 ? inltrb.x : inltrb.z) * 2.0/W - 1.0;
                gl_Position.y = ((gl_VertexID & 2) == 0 ? inltrb.y : inltrb.w) * 2.0/H - 1.0;
                gl_Position.zw = vec2(0, 1);
            })",

            std::string(R"(#version 310 es
            precision highp float;
            in vec4 color;
            in vec4 aux1;
            in vec4 aux2;)")
                .append(kLocalStorageGLSLDefines)
                .append(fsMain)
                .c_str());
    }

    void useProgram(const GLProgram &program)
    {
        ASSERT_TRUE(program.valid());

        if (mLTRBLocation >= 0)
        {
            glDisableVertexAttribArray(mLTRBLocation);
        }
        if (mRGBALocation >= 0)
        {
            glDisableVertexAttribArray(mRGBALocation);
        }
        if (mAux1Location >= 0)
        {
            glDisableVertexAttribArray(mAux1Location);
        }
        if (mAux2Location >= 0)
        {
            glDisableVertexAttribArray(mAux2Location);
        }

        glUseProgram(program);

        glUniform1f(glGetUniformLocation(program, "W"), W);
        glUniform1f(glGetUniformLocation(program, "H"), H);

        mLTRBLocation = glGetAttribLocation(program, "inltrb");
        glEnableVertexAttribArray(mLTRBLocation);
        glVertexAttribDivisor(mLTRBLocation, 1);

        mRGBALocation = glGetAttribLocation(program, "incolor");
        glEnableVertexAttribArray(mRGBALocation);
        glVertexAttribDivisor(mRGBALocation, 1);

        mAux1Location = glGetAttribLocation(program, "inaux1");
        glEnableVertexAttribArray(mAux1Location);
        glVertexAttribDivisor(mAux1Location, 1);

        mAux2Location = glGetAttribLocation(program, "inaux2");
        glEnableVertexAttribArray(mAux2Location);
        glVertexAttribDivisor(mAux2Location, 1);
    }

    struct Box
    {
        using float4 = std::array<float, 4>;
        Box(float4 inltrb) : ltrb(inltrb), color{}, aux1{}, aux2{} {}
        Box(float4 inltrb, float4 incolor) : ltrb(inltrb), color(incolor), aux1{}, aux2{} {}
        Box(float4 inltrb, float4 incolor, float4 inaux1)
            : ltrb(inltrb), color(incolor), aux1(inaux1), aux2{}
        {}
        Box(float4 inltrb, float4 incolor, float4 inaux1, float4 inaux2)
            : ltrb(inltrb), color(incolor), aux1(inaux1), aux2(inaux2)
        {}
        float4 ltrb;
        float4 color;
        float4 aux1;
        float4 aux2;
    };

    enum class UseBarriers : bool
    {
        No = false,
        IfNoncoherent
    };

    void drawBoxes(PixelLocalStoragePrototype &pls,
                   std::vector<Box> boxes,
                   UseBarriers useBarriers = UseBarriers::IfNoncoherent)
    {
        if (!supportsCoherentPixelLocalStorage() && useBarriers == UseBarriers::IfNoncoherent)
        {
            for (const auto &box : boxes)
            {
                glVertexAttribPointer(mLTRBLocation, 4, GL_FLOAT, GL_FALSE, sizeof(Box),
                                      box.ltrb.data());
                glVertexAttribPointer(mRGBALocation, 4, GL_FLOAT, GL_FALSE, sizeof(Box),
                                      box.color.data());
                glVertexAttribPointer(mAux1Location, 4, GL_FLOAT, GL_FALSE, sizeof(Box),
                                      box.aux1.data());
                glVertexAttribPointer(mAux2Location, 4, GL_FLOAT, GL_FALSE, sizeof(Box),
                                      box.aux2.data());
                glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 1);
                glPixelLocalStorageBarrierANGLE();
            }
        }
        else
        {
            glVertexAttribPointer(mLTRBLocation, 4, GL_FLOAT, GL_FALSE, sizeof(Box),
                                  boxes[0].ltrb.data());
            glVertexAttribPointer(mRGBALocation, 4, GL_FLOAT, GL_FALSE, sizeof(Box),
                                  boxes[0].color.data());
            glVertexAttribPointer(mAux1Location, 4, GL_FLOAT, GL_FALSE, sizeof(Box),
                                  boxes[0].aux1.data());
            glVertexAttribPointer(mAux2Location, 4, GL_FLOAT, GL_FALSE, sizeof(Box),
                                  boxes[0].aux2.data());
            glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, boxes.size());
        }
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

    GLProgram mScratchProgram;
    GLint mLTRBLocation = -1;
    GLint mRGBALocation = -1;
    GLint mAux1Location = -1;
    GLint mAux2Location = -1;

    GLuint mScratchFBO = 0;
};

// Verify that values from separate draw calls persist in pixel local storage, for all supported
// formats. Also verify that clear-to-zero works on every supported format.
TEST_P(PixelLocalStorageTest, AllFormats)
{
    if (!supportsPixelLocalStorage())
    {
        return;
    }

    {
        PixelLocalStoragePrototype pls;

        useProgram(R"(
        DECLARE_LOCAL_STORAGE(rgba8, 0, plane1_R, 1, plane1_W);
        DECLARE_LOCAL_STORAGE_I(rgba8i, 2, plane2_R, 3, plane2_W);
        DECLARE_LOCAL_STORAGE_UI(rgba8ui, 4, plane3_R, 5, plane3_W);
        void main() {
            pixelLocalStore(plane1_W, color + pixelLocalLoad(plane1_R));
            pixelLocalStore(plane2_W, ivec4(aux1) + pixelLocalLoad(plane2_R));
            pixelLocalStore(plane3_W, uvec4(aux2) + pixelLocalLoad(plane3_R));
        })");

        PixelLocalTexture tex1(GL_RGBA8);
        PixelLocalTexture tex2(GL_RGBA8I);
        PixelLocalTexture tex3(GL_RGBA8UI);

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferPixelLocalStorageANGLE(0, tex1, 0, 0, W, H, GL_RGBA8);
        glFramebufferPixelLocalStorageANGLE(1, tex2, 0, 0, W, H, GL_RGBA8I);
        glFramebufferPixelLocalStorageANGLE(2, tex3, 0, 0, W, H, GL_RGBA8UI);
        glViewport(0, 0, W, H);
        glDrawBuffers(0, nullptr);

        glBeginPixelLocalStorageANGLE(3, GLenumArray({GL_ZERO, GL_ZERO, GL_ZERO}));

        // Accumulate R, G, B, A in 4 separate passes.
        drawBoxes(pls, {{FULLSCREEN, {1, 0, 0, 0}, {-5, 0, 0, 0}, {1, 0, 0, 0}},
                        {FULLSCREEN, {0, 1, 0, 0}, {0, -100, 0, 0}, {0, 50, 0, 0}},
                        {FULLSCREEN, {0, 0, 1, 0}, {0, 0, -70, 0}, {0, 0, 100, 0}},
                        {FULLSCREEN, {0, 0, 0, 1}, {0, 0, 0, 22}, {0, 0, 0, 255}}});

        glEndPixelLocalStorageANGLE();

        attachTextureToScratchFBO(tex1);
        ExpectFramebufferPixels<uint8_t>({255, 255, 255, 255});

        attachTextureToScratchFBO(tex2);
        ExpectFramebufferPixels<int32_t>({-5, -100, -70, 22});

        attachTextureToScratchFBO(tex3);
        ExpectFramebufferPixels<uint32_t>({1, 50, 100, 255});

        ASSERT_GL_NO_ERROR();
    }

    {
        PixelLocalStoragePrototype pls;

        useProgram(R"(
        DECLARE_LOCAL_STORAGE(r32f, 0, plane1_R, 1, plane1_W);
        DECLARE_LOCAL_STORAGE_UI(r32ui, 2, plane2_R, 3, plane2_W);
        void main() {
            pixelLocalStore(plane1_W, color + pixelLocalLoad(plane1_R));
            pixelLocalStore(plane2_W, uvec4(aux1) + pixelLocalLoad(plane2_R));
        })");

        PixelLocalTexture tex1(GL_R32F);
        PixelLocalTexture tex2(GL_R32UI);

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferPixelLocalStorageANGLE(0, tex1, 0, 0, W, H, GL_R32F);
        glFramebufferPixelLocalStorageANGLE(1, tex2, 0, 0, W, H, GL_R32UI);
        glViewport(0, 0, W, H);
        glDrawBuffers(0, nullptr);

        glBeginPixelLocalStorageANGLE(2, GLenumArray({GL_ZERO, GL_ZERO}));

        // Accumulate R in 4 separate passes.
        drawBoxes(pls, {{FULLSCREEN, {-1.5, 0, 0, 0}, {5, 0, 0, 0}},
                        {FULLSCREEN, {-10.25, 0, 0, 0}, {60, 0, 0, 0}},
                        {FULLSCREEN, {-100, 0, 0, 0}, {700, 0, 0, 0}},
                        {FULLSCREEN, {.25, 0, 0, 0}, {8000, 0, 0, 22}}});

        glEndPixelLocalStorageANGLE();

        attachTextureToScratchFBO(tex1);
        ExpectFramebufferPixels<float>({-111.5, 0, 0, 1});

        attachTextureToScratchFBO(tex2);
        ExpectFramebufferPixels<uint32_t>({8765, 0, 0, 1});

        ASSERT_GL_NO_ERROR();
    }

    {
        PixelLocalStoragePrototype pls;

        useProgram(R"(
        DECLARE_LOCAL_STORAGE(rgba16f, 0, plane1_R, 1, plane1_W);
        DECLARE_LOCAL_STORAGE_I(rgba16i, 2, plane2_R, 3, plane2_W);
        DECLARE_LOCAL_STORAGE_UI(rgba16ui, 4, plane3_R, 5, plane3_W);
        void main() {
            pixelLocalStore(plane1_W, color + pixelLocalLoad(plane1_R));
            pixelLocalStore(plane2_W, ivec4(aux1) + pixelLocalLoad(plane2_R));
            pixelLocalStore(plane3_W, uvec4(aux2) + pixelLocalLoad(plane3_R));
        })");

        PixelLocalTexture tex1(GL_RGBA16F);
        PixelLocalTexture tex2(GL_RGBA16I);
        PixelLocalTexture tex3(GL_RGBA16UI);

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferPixelLocalStorageANGLE(0, tex1, 0, 0, W, H, GL_RGBA16F);
        glFramebufferPixelLocalStorageANGLE(1, tex2, 0, 0, W, H, GL_RGBA16I);
        glFramebufferPixelLocalStorageANGLE(2, tex3, 0, 0, W, H, GL_RGBA16UI);
        glViewport(0, 0, W, H);
        glDrawBuffers(0, nullptr);

        glBeginPixelLocalStorageANGLE(3, GLenumArray({GL_ZERO, GL_ZERO, GL_ZERO}));

        // Accumulate R, G, B, A in 4 separate passes.
        drawBoxes(pls, {{FULLSCREEN, {-100.5, 0, 0, 0}, {-500, 0, 0, 0}, {1, 0, 0, 0}},
                        {FULLSCREEN, {0, 1024, 0, 0}, {0, -10000, 0, 0}, {0, 500, 0, 0}},
                        {FULLSCREEN, {0, 0, -4096, 0}, {0, 0, -7000, 0}, {0, 0, 10000, 0}},
                        {FULLSCREEN, {0, 0, 0, 16384}, {0, 0, 0, 2200}, {0, 0, 0, 65535}}});

        glEndPixelLocalStorageANGLE();

        attachTextureToScratchFBO(tex1);
        ExpectFramebufferPixels<float>({-100.5, 1024, -4096, 16384});

        attachTextureToScratchFBO(tex2);
        ExpectFramebufferPixels<int32_t>({-500, -10000, -7000, 2200});

        attachTextureToScratchFBO(tex3);
        ExpectFramebufferPixels<uint32_t>({1, 500, 10000, 65535});

        ASSERT_GL_NO_ERROR();
    }

    {
        PixelLocalStoragePrototype pls;

        useProgram(R"(
        DECLARE_LOCAL_STORAGE(rgba32f, 0, plane1_R, 1, plane1_W);
        DECLARE_LOCAL_STORAGE_UI(rgba32ui, 2, plane2_R, 3, plane2_W);
        void main() {
            pixelLocalStore(plane1_W, color + pixelLocalLoad(plane1_R));
            pixelLocalStore(plane2_W, uvec4(aux1) + pixelLocalLoad(plane2_R));
        })");

        PixelLocalTexture tex1(GL_RGBA32F);
        PixelLocalTexture tex2(GL_RGBA32UI);

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferPixelLocalStorageANGLE(0, tex1, 0, 0, W, H, GL_RGBA32F);
        glFramebufferPixelLocalStorageANGLE(1, tex2, 0, 0, W, H, GL_RGBA32UI);
        glViewport(0, 0, W, H);
        glDrawBuffers(0, nullptr);

        glBeginPixelLocalStorageANGLE(2, GLenumArray({GL_ZERO, GL_ZERO}));

        // Accumulate R, G, B, A in 4 separate passes.
        drawBoxes(pls, {{FULLSCREEN, {-100.5, 0, 0, 0}, {1, 0, 0, 0}},
                        {FULLSCREEN, {0, 1024, 0, 0}, {0, 500, 0, 0}},
                        {FULLSCREEN, {0, 0, -4096, 0}, {0, 0, 10000, 0}},
                        {FULLSCREEN, {0, 0, 0, 16384}, {0, 0, 0, 65535}}});

        glEndPixelLocalStorageANGLE();

        attachTextureToScratchFBO(tex1);
        ExpectFramebufferPixels<float>({-100.5, 1024, -4096, 16384});

        attachTextureToScratchFBO(tex2);
        ExpectFramebufferPixels<uint32_t>({1, 500, 10000, 65535});

        ASSERT_GL_NO_ERROR();
    }
}

// Check that results are only nondeterministic within predictable constraints, and that no data is
// random or leaked from other contexts when we forget to insert a barrier.
TEST_P(PixelLocalStorageTest, ForgetBarrier)
{
    if (!supportsPixelLocalStorage())
    {
        return;
    }

    PixelLocalStoragePrototype pls;

    useProgram(R"(
    DECLARE_LOCAL_STORAGE_UI(r32ui, 0, framebuffer_R, 1, framebuffer_W);
    void main() {
        uvec4 dst = pixelLocalLoad(framebuffer_R);
        pixelLocalStore(framebuffer_W, uvec4(color) + dst * 2u);
    })");

    // Draw r=100, one pixel at a time, in random order.
    constexpr static int NUM_PIXELS = H * W;
    std::vector<Box> boxesA_100;
    int pixelIdx = 0;
    for (int i = 0; i < NUM_PIXELS; ++i)
    {
        int iy  = pixelIdx / W;
        float y = iy;
        int ix  = pixelIdx % W;
        float x = ix;
        pixelIdx =
            (pixelIdx + 69484171) % NUM_PIXELS;  // Prime numbers guarantee we hit every pixel once.
        boxesA_100.push_back(Box{{x, y, x + 1, y + 1}, {100, 0, 0, 0}});
    }

    // Draw r=1, one pixel at a time, in random order.
    std::vector<Box> boxesB_1;
    for (int i = 0; i < NUM_PIXELS; ++i)
    {
        int iy  = pixelIdx / W;
        float y = iy;
        int ix  = pixelIdx % W;
        float x = ix;
        pixelIdx =
            (pixelIdx + 97422697) % NUM_PIXELS;  // Prime numbers guarantee we hit every pixel once.
        boxesB_1.push_back(Box{{x, y, x + 1, y + 1}, {1, 0, 0, 0}});
    }

    PixelLocalTexture tex(GL_R32UI);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferPixelLocalStorageANGLE(0, tex, 0, 0, W, H, GL_R32UI);
    glViewport(0, 0, W, H);
    glDrawBuffers(0, nullptr);

    // First make sure it works properly with a barrier.
    glBeginPixelLocalStorageANGLE(1, GLenumArray({GL_ZERO}));
    drawBoxes(pls, boxesA_100, UseBarriers::No);
    glPixelLocalStorageBarrierANGLE();
    drawBoxes(pls, boxesB_1, UseBarriers::No);
    glEndPixelLocalStorageANGLE();

    attachTextureToScratchFBO(tex);
    ExpectFramebufferPixels<uint32_t>({201, 0, 0, 1});

    ASSERT_GL_NO_ERROR();

    // Now forget to insert the barrier and ensure our nondeterminism still falls within predictable
    // constraints.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBeginPixelLocalStorageANGLE(1, GLenumArray({GL_ZERO}));
    drawBoxes(pls, boxesA_100, UseBarriers::No);
    // OOPS! We forgot to insert a barrier!
    drawBoxes(pls, boxesB_1, UseBarriers::No);
    glEndPixelLocalStorageANGLE();

    uint32_t pixels[H * W * 4];
    attachTextureToScratchFBO(tex);
    glReadPixels(0, 0, W, H, GL_RGBA_INTEGER, GL_UNSIGNED_INT, pixels);
    for (int r = 0; r < NUM_PIXELS; r += 4)
    {
        // When two fragments, A and B, touch a pixel, there are 6 possible orderings of operations:
        //
        //   * Read A, Write A, Read B, Write B
        //   * Read B, Write B, Read A, Write A
        //   * Read A, Read B, Write A, Write B
        //   * Read A, Read B, Write B, Write A
        //   * Read B, Read A, Write B, Write A
        //   * Read B, Read A, Write A, Write B
        //
        // Which (assumimg the read and/or write operations themselves are atomic), is equivalent to
        // 1 of 4 potential effects:
        bool isAcceptableValue = pixels[r] == 201 ||  // A, then B  (1 + 100 * 2 == 201)
                                 pixels[r] == 102 ||  // B, then A  (100 + 1 * 2 == 102)
                                 pixels[r] == 100 ||  // A only     (100         == 100)
                                 pixels[r] == 1;      // B only     (1           ==   1)
        ASSERT_TRUE(isAcceptableValue);
        if (!isAcceptableValue)
        {
            break;
        }
    }

    ASSERT_GL_NO_ERROR();
}

// Check that stores and loads in a single shader invocation are coherent.
TEST_P(PixelLocalStorageTest, CoherentStoreLoad)
{
    if (!supportsPixelLocalStorage())
    {
        return;
    }

    PixelLocalStoragePrototype pls;

    // Bind to the last local storage plane, just to verify GL_DISABLED_ANGLE.
    std::vector<GLenum> loadOps(MAX_LOCAL_STORAGE_PLANES, GL_DISABLED_ANGLE);
    loadOps.back() = GL_ZERO;
    int idxR       = (MAX_LOCAL_STORAGE_PLANES - 1) * 2;
    int idxW       = idxR + 1;

    std::stringstream fs;
    fs << "DECLARE_LOCAL_STORAGE_UI(rgba32ui, " << idxR << ", fibonacci_R, " << idxW
       << ", fibonacci_W);\n";
    fs << R"(
    void main() {
        pixelLocalStore(fibonacci_W, uvec4(1, 0, 0, 0));  // fib(1, 0, 0, 0)
        for (int i = 0; i < 3; ++i) {
            uvec4 fib0 = pixelLocalLoad(fibonacci_R);
            uvec4 fib1;
            fib1.w = fib0.x + fib0.y;
            fib1.z = fib1.w + fib0.x;
            fib1.y = fib1.z + fib1.w;
            fib1.x = fib1.y + fib1.z;  // fib(i*4 + (5, 4, 3, 2))
            pixelLocalStore(fibonacci_W, fib1);
        }
        // fib is at indices (13, 12, 11, 10)
    })";
    useProgram(fs.str().c_str());

    PixelLocalTexture tex(GL_RGBA32UI);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferPixelLocalStorageANGLE(MAX_LOCAL_STORAGE_PLANES - 1, tex, 0, 0, W, H, GL_RGBA32UI);
    glViewport(0, 0, W, H);
    glDrawBuffers(0, nullptr);

    glBeginPixelLocalStorageANGLE(MAX_LOCAL_STORAGE_PLANES, loadOps.data());

    drawBoxes(pls, {{FULLSCREEN}});

    glEndPixelLocalStorageANGLE();

    attachTextureToScratchFBO(tex);
    ExpectFramebufferPixels<uint32_t>({233, 144, 89, 55});  // fib(13, 12, 11, 10)

    ASSERT_GL_NO_ERROR();
}

// Check loading and storing from memoryless local storage planes.
TEST_P(PixelLocalStorageTest, MemorylessStorage)
{
    if (!supportsPixelLocalStorage())
    {
        return;
    }

    PixelLocalStoragePrototype pls;

    // Bind the texture, but don't call glTexStorage until after creating the memoryless plane.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    // Create a memoryless plane.
    glFramebufferPixelLocalStorageANGLE(1, 0, 0, 0, W, H, GL_RGBA8);
    // Define the persistent texture now, after attaching the memoryless pixel local storage. This
    // verifies that the GL_TEXTURE_2D binding doesn't get disturbed by local storage.
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, W, H);
    glFramebufferPixelLocalStorageANGLE(0, tex, 0, 0, W, H, GL_RGBA8);
    glViewport(0, 0, W, H);
    glDrawBuffers(0, nullptr);

    // Clear color won't affect a GL_ZERO load op.
    glFramebufferPixelLocalClearValuefvANGLE(1, MakeArray({0.f, 0.f, 0.f, 1.f}));

    glBeginPixelLocalStorageANGLE(2, GLenumArray({GL_ZERO, GL_ZERO}));

    // Draw into memoryless storage.
    GLProgram writeMemless;
    compileProgram(writeMemless, R"(
    DECLARE_LOCAL_STORAGE(rgba8, 2, memoryless_R, 3, memoryless_W);
    void main() {
        pixelLocalStore(memoryless_W, color + pixelLocalLoad(memoryless_R));
    })");
    useProgram(writeMemless);

    drawBoxes(pls, {{{0, 20, W, H}, {1, 0, 0, 0}},
                    {{0, 40, W, H}, {0, 1, 0, 0}},
                    {{0, 60, W, H}, {0, 0, 1, 0}}});

    // Transfer to a texture.
    GLProgram transferMemless;
    compileProgram(transferMemless, R"(
    DECLARE_LOCAL_STORAGE(rgba8, 0, framebuffer_R, 1, framebuffer_W);
    DECLARE_LOCAL_STORAGE(rgba8, 2, memoryless_R, 3, memoryless_W);
    void main() {
        pixelLocalStore(framebuffer_W, vec4(1) - pixelLocalLoad(memoryless_R));
    })");
    useProgram(transferMemless);

    drawBoxes(pls, {{FULLSCREEN}});

    glEndPixelLocalStorageANGLE();

    attachTextureToScratchFBO(tex);
    ExpectFramebufferPixels<uint8_t>(0, 60, W, H, {0, 0, 0, 255});
    ExpectFramebufferPixels<uint8_t>(0, 40, W, 60, {0, 0, 255, 255});
    ExpectFramebufferPixels<uint8_t>(0, 20, W, 40, {0, 255, 255, 255});
    ExpectFramebufferPixels<uint8_t>(0, 0, W, 20, {255, 255, 255, 255});

    // Repeat, this time with a clear color in memoryless storage.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBeginPixelLocalStorageANGLE(2, GLenumArray({GL_ZERO, GL_REPLACE}));
    useProgram(writeMemless);
    drawBoxes(pls, {{{0, 20, W, H}, {1, 0, 0, 0}},
                    {{0, 40, W, H}, {0, 1, 0, 0}},
                    {{0, 60, W, H}, {0, 0, 1, 0}}});
    useProgram(transferMemless);
    drawBoxes(pls, {{FULLSCREEN}});
    glEndPixelLocalStorageANGLE();

    attachTextureToScratchFBO(tex);
    ExpectFramebufferPixels<uint8_t>(0, 60, W, H, {0, 0, 0, 0});
    ExpectFramebufferPixels<uint8_t>(0, 40, W, 60, {0, 0, 255, 0});
    ExpectFramebufferPixels<uint8_t>(0, 20, W, 40, {0, 255, 255, 0});
    ExpectFramebufferPixels<uint8_t>(0, 0, W, 20, {255, 255, 255, 0});

    // Ensure the GL_TEXTURE_2D binding still hasn't been disturbed by local storage.
    GLint textureBinding2D;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureBinding2D);
    ASSERT_EQ((GLuint)textureBinding2D, tex);

    ASSERT_GL_NO_ERROR();
}

// Check that it works to render with the maximum supported data payload:
//
//   GL_MAX_LOCAL_STORAGE_PLANES_ANGLE
//   GL_MAX_LOCAL_STORAGE_BYTES_ANGLE
//   GL_MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE_ANGLE
//
TEST_P(PixelLocalStorageTest, MaxCapacity)
{
    if (!supportsPixelLocalStorage())
    {
        return;
    }

    PixelLocalStoragePrototype pls;

    // Try to use up MAX_LOCAL_STORAGE_BYTES of data.
    int numRegisters = MAX_LOCAL_STORAGE_BYTES / 4;
    ASSERT_TRUE(numRegisters >=
                MAX_LOCAL_STORAGE_PLANES);  // Otherwise MAX_LOCAL_STORAGE_PLANES is impossible.
    int numExtraRegisters = numRegisters - MAX_LOCAL_STORAGE_PLANES;
    int num32s            = std::min(numExtraRegisters / 3, MAX_LOCAL_STORAGE_PLANES);
    int num16s = std::min(numExtraRegisters - num32s * 3, MAX_LOCAL_STORAGE_PLANES - num32s);
    int num8s  = MAX_LOCAL_STORAGE_PLANES - num32s - num16s;
    ASSERT(num8s >= 0);
    ASSERT(num32s * 4 + num16s * 2 + num8s <= numRegisters);

    std::stringstream fs;
    for (int i = 0; i < MAX_LOCAL_STORAGE_PLANES; ++i)
    {
        const char *format = i < num32s              ? "rgba32ui"
                             : i < (num32s + num16s) ? "rgba16ui"
                                                     : "rgba8ui";
        fs << "DECLARE_LOCAL_STORAGE_UI(" << format << ", " << (i * 2) << ", " << char('a' + i)
           << "_R, " << (i * 2 + 1) << ", " << char('a' + i) << "_W);\n";
    }
    for (int i = 0; i < MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE; ++i)
    {
        fs << "out uvec4 out" << i << ";\n";
    }
    fs << "void main() {\n";
    for (int i = 0; i < MAX_LOCAL_STORAGE_PLANES; ++i)
    {
        fs << "pixelLocalStore(" << char('a' + i) << "_W, uvec4(color) - uvec4(" << i << "));\n";
    }
    for (int i = 0; i < MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE; ++i)
    {
        fs << "out" << i << " = uvec4(aux1) + uvec4(" << i << ");\n";
    }
    fs << "}";
    useProgram(fs.str().c_str());

    glViewport(0, 0, W, H);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    std::vector<PixelLocalTexture> localTexs;
    for (int i = 0; i < MAX_LOCAL_STORAGE_PLANES; ++i)
    {
        GLenum internalformat = i < num32s              ? GL_RGBA32UI
                                : i < (num32s + num16s) ? GL_RGBA16UI
                                                        : GL_RGBA8UI;
        localTexs.emplace_back(internalformat);
        glFramebufferPixelLocalStorageANGLE(i, localTexs[i], 0, 0, W, H, internalformat);
    }
    std::vector<PixelLocalTexture> renderTexs;
    std::vector<GLenum> drawBuffers;
    for (int i = 0; i < MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE; ++i)
    {
        renderTexs.emplace_back(GL_RGBA32UI);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D,
                               renderTexs[i], 0);
        drawBuffers.emplace_back(GL_COLOR_ATTACHMENT0 + i);
    }
    glDrawBuffers(drawBuffers.size(), drawBuffers.data());

    glBeginPixelLocalStorageANGLE(MAX_LOCAL_STORAGE_PLANES,
                                  std::vector<GLenum>(MAX_LOCAL_STORAGE_PLANES, GL_ZERO).data());
    drawBoxes(pls, {{FULLSCREEN, {255, 254, 253, 252}, {0, 1, 2, 3}}});
    glEndPixelLocalStorageANGLE();

    for (int i = 0; i < MAX_LOCAL_STORAGE_PLANES; ++i)
    {
        attachTextureToScratchFBO(localTexs[i]);
        ExpectFramebufferPixels<uint32_t>({255u - i, 254u - i, 253u - i, 252u - i});
    }
    for (int i = 0; i < MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE; ++i)
    {
        attachTextureToScratchFBO(renderTexs[i]);
        ExpectFramebufferPixels<uint32_t>({0u + i, 1u + i, 2u + i, 3u + i});
    }

    ASSERT_GL_NO_ERROR();
}

// Check proper functioning of glFramebufferPixelLocalClearValue{fi ui}vANGLE.
TEST_P(PixelLocalStorageTest, ClearValue)
{
    if (!supportsPixelLocalStorage())
    {
        return;
    }

    PixelLocalStoragePrototype pls;

    // Scissor and clear color should not affect clear loads.
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, 1, 1);
    glClearColor(.1f, .2f, .3f, .4f);

    PixelLocalTexture texf(GL_RGBA8);
    PixelLocalTexture texi(GL_RGBA16I);
    PixelLocalTexture texui(GL_RGBA16UI);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferPixelLocalStorageANGLE(0, texf, 0, 0, W, H, GL_RGBA8);
    glFramebufferPixelLocalStorageANGLE(1, texi, 0, 0, W, H, GL_RGBA16I);
    glFramebufferPixelLocalStorageANGLE(2, texui, 0, 0, W, H, GL_RGBA16UI);
    auto clearLoads = GLenumArray({GL_REPLACE, GL_REPLACE, GL_REPLACE});

    // Clear values are initially zero.
    glBeginPixelLocalStorageANGLE(3, clearLoads);
    glEndPixelLocalStorageANGLE();
    attachTextureToScratchFBO(texf);
    ExpectFramebufferPixels<uint8_t>({0, 0, 0, 0});
    attachTextureToScratchFBO(texi);
    ExpectFramebufferPixels<int32_t>({0, 0, 0, 0});
    attachTextureToScratchFBO(texui);
    ExpectFramebufferPixels<uint32_t>({0, 0, 0, 0});

    // Test custom clear values.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferPixelLocalClearValuefvANGLE(0, MakeArray({1.f, 0.f, 0.f, 0.f}));
    glFramebufferPixelLocalClearValueivANGLE(1, MakeArray({1, 2, 3, 4}));
    glFramebufferPixelLocalClearValueuivANGLE(2, MakeArray({5u, 6u, 7u, 8u}));
    glBeginPixelLocalStorageANGLE(3, clearLoads);
    glEndPixelLocalStorageANGLE();
    attachTextureToScratchFBO(texf);
    ExpectFramebufferPixels<uint8_t>({255, 0, 0, 0});
    attachTextureToScratchFBO(texi);
    ExpectFramebufferPixels<int32_t>({1, 2, 3, 4});
    attachTextureToScratchFBO(texui);
    ExpectFramebufferPixels<uint32_t>({5u, 6u, 7u, 8u});

    // Different clear value types are separate state values.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferPixelLocalStorageANGLE(1, texf, 0, 0, W, H, GL_RGBA8);
    glFramebufferPixelLocalStorageANGLE(2, texi, 0, 0, W, H, GL_RGBA16I);
    glFramebufferPixelLocalStorageANGLE(0, texui, 0, 0, W, H, GL_RGBA16UI);
    glBeginPixelLocalStorageANGLE(3, clearLoads);
    glEndPixelLocalStorageANGLE();
    attachTextureToScratchFBO(texf);
    ExpectFramebufferPixels<uint8_t>({0, 0, 0, 0});
    attachTextureToScratchFBO(texi);
    ExpectFramebufferPixels<int32_t>({0, 0, 0, 0});
    attachTextureToScratchFBO(texui);
    ExpectFramebufferPixels<uint32_t>({0, 0, 0, 0});

    // Set new custom clear values.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferPixelLocalClearValuefvANGLE(1, MakeArray({0.f, 1.f, 0.f, 0.f}));
    glFramebufferPixelLocalClearValueivANGLE(2, MakeArray({100, 200, 300, 400}));
    glFramebufferPixelLocalClearValueuivANGLE(0, MakeArray({500u, 600u, 700u, 800u}));
    glBeginPixelLocalStorageANGLE(3, clearLoads);
    glEndPixelLocalStorageANGLE();
    attachTextureToScratchFBO(texf);
    ExpectFramebufferPixels<uint8_t>({0, 255, 0, 0});
    attachTextureToScratchFBO(texi);
    ExpectFramebufferPixels<int32_t>({100, 200, 300, 400});
    attachTextureToScratchFBO(texui);
    ExpectFramebufferPixels<uint32_t>({500u, 600u, 700u, 800u});

    // Different clear value types are separate state values (final rotation).
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferPixelLocalStorageANGLE(2, texf, 0, 0, W, H, GL_RGBA8);
    glFramebufferPixelLocalStorageANGLE(0, texi, 0, 0, W, H, GL_RGBA16I);
    glFramebufferPixelLocalStorageANGLE(1, texui, 0, 0, W, H, GL_RGBA16UI);
    glBeginPixelLocalStorageANGLE(3, clearLoads);
    glEndPixelLocalStorageANGLE();
    attachTextureToScratchFBO(texf);
    ExpectFramebufferPixels<uint8_t>({0, 0, 0, 0});
    attachTextureToScratchFBO(texi);
    ExpectFramebufferPixels<int32_t>({0, 0, 0, 0});
    attachTextureToScratchFBO(texui);
    ExpectFramebufferPixels<uint32_t>({0, 0, 0, 0});

    // Set new custom clear values (final rotation).
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferPixelLocalClearValuefvANGLE(2, MakeArray({0.f, 0.f, 1.f, 0.f}));
    glFramebufferPixelLocalClearValueivANGLE(0, MakeArray({1000, 2000, 3000, 4000}));
    glFramebufferPixelLocalClearValueuivANGLE(1, MakeArray({5000u, 6000u, 7000u, 8000u}));
    glBeginPixelLocalStorageANGLE(3, clearLoads);
    glEndPixelLocalStorageANGLE();
    attachTextureToScratchFBO(texf);
    ExpectFramebufferPixels<uint8_t>({0, 0, 255, 0});
    attachTextureToScratchFBO(texi);
    ExpectFramebufferPixels<int32_t>({1000, 2000, 3000, 4000});
    attachTextureToScratchFBO(texui);
    ExpectFramebufferPixels<uint32_t>({5000u, 6000u, 7000u, 8000u});

    // GL_ZERO shouldn't be affected by the clear color state.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBeginPixelLocalStorageANGLE(3, GLenumArray({GL_ZERO, GL_ZERO, GL_ZERO}));
    glEndPixelLocalStorageANGLE();
    attachTextureToScratchFBO(texf);
    ExpectFramebufferPixels<uint8_t>({0, 0, 0, 0});
    attachTextureToScratchFBO(texi);
    ExpectFramebufferPixels<int32_t>({0, 0, 0, 0});
    attachTextureToScratchFBO(texui);
    ExpectFramebufferPixels<uint32_t>({0, 0, 0, 0});
}

// Check proper support of GL_ZERO, GL_KEEP, GL_REPLACE, and GL_DISABLED_ANGLE loadOps. Also verify
// that it works do draw with GL_MAX_LOCAL_STORAGE_PLANES_ANGLE planes.
TEST_P(PixelLocalStorageTest, LoadOps)
{
    if (!supportsPixelLocalStorage())
    {
        return;
    }

    PixelLocalStoragePrototype pls;

    std::stringstream fs;
    for (int i = 0; i < MAX_LOCAL_STORAGE_PLANES; ++i)
    {
        fs << "DECLARE_LOCAL_STORAGE(rgba8, " << (i * 2) << ", " << char('a' + i) << "_R, "
           << (i * 2 + 1) << ", " << char('a' + i) << "_W);\n";
    }
    fs << "void main() {\n";
    for (int i = 0; i < MAX_LOCAL_STORAGE_PLANES; ++i)
    {
        fs << "pixelLocalStore(" << char('a' + i) << "_W, color + pixelLocalLoad(" << char('a' + i)
           << "_R));\n";
    }
    fs << "}";
    useProgram(fs.str().c_str());

    glClearColor(1, 0, 0, 1);

    std::vector<PixelLocalTexture> texs;
    for (int i = 0; i < MAX_LOCAL_STORAGE_PLANES; ++i)
    {
        texs.emplace_back(GL_RGBA8);
        attachTextureToScratchFBO(texs[i]);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    // Turn on scissor to try and confuse the local storage clear step.
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, 20, H);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    std::vector<GLenum> loadOps(MAX_LOCAL_STORAGE_PLANES);
    for (int i = 0; i < MAX_LOCAL_STORAGE_PLANES; ++i)
    {
        glFramebufferPixelLocalClearValuefvANGLE(i, MakeArray({0.f, 0.f, 0.f, 1.f}));
        glFramebufferPixelLocalStorageANGLE(i, texs[i], 0, 0, W, H, GL_RGBA8);
        loadOps[i] = (i & 1) ? GL_REPLACE : GL_KEEP;
    }
    glViewport(0, 0, W, H);
    glDrawBuffers(0, nullptr);

    glBeginPixelLocalStorageANGLE(MAX_LOCAL_STORAGE_PLANES, loadOps.data());
    drawBoxes(pls, {{{FULLSCREEN}, {0, 1, 0, 0}}});
    glEndPixelLocalStorageANGLE();

    for (int i = 0; i < MAX_LOCAL_STORAGE_PLANES; ++i)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texs[i], 0);
        // Check that the draw buffers didn't get disturbed by local storage -- GL_COLOR_ATTACHMENT0
        // is currently off, so glClear has no effect. This also verifies that local storage planes
        // didn't get left attached to the framebuffer somewhere with draw buffers on.
        glClear(GL_COLOR_BUFFER_BIT);
        ExpectFramebufferPixels<uint8_t>(
            0, 0, 20, H,
            loadOps[i] == GL_REPLACE ? std::array<uint8_t, 4>{0, 255, 0, 255}
                                     : /*GL_KEEP*/ std::array<uint8_t, 4>{255, 255, 0, 255});

        // Check that the scissor didn't get disturbed when we cleared local storage.
        ExpectFramebufferPixels<uint8_t>(20, 0, W, H,
                                         loadOps[i] == GL_REPLACE
                                             ? std::array<uint8_t, 4>{0, 0, 0, 255}
                                             : std::array<uint8_t, 4>{255, 0, 0, 255});
    }

    // Now test GL_DISABLED_ANGLE and GL_ZERO.
    for (int i = 0; i < MAX_LOCAL_STORAGE_PLANES; ++i)
    {
        loadOps[i] = loadOps[i] == GL_REPLACE ? GL_ZERO : GL_DISABLED_ANGLE;
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glBeginPixelLocalStorageANGLE(MAX_LOCAL_STORAGE_PLANES, loadOps.data());
    // No draw.
    glEndPixelLocalStorageANGLE();

    for (int i = 0; i < MAX_LOCAL_STORAGE_PLANES; ++i)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texs[i], 0);
        if (loadOps[i] == GL_ZERO)
        {
            ExpectFramebufferPixels<uint8_t>({0, 0, 0, 0});
        }
        else
        {
            ExpectFramebufferPixels<uint8_t>(0, 0, 20, H, {255, 255, 0, 255});
            ExpectFramebufferPixels<uint8_t>(20, 0, W, H, {255, 0, 0, 255});
        }
    }

    // Now turn GL_COLOR_ATTACHMENT0 back on and check that the clear color and scissor didn't get
    // disturbed by local storage.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texs[1], 0);
    glDrawBuffers(1, std::array<GLenum, 1>{GL_COLOR_ATTACHMENT0}.data());
    glClear(GL_COLOR_BUFFER_BIT);
    ExpectFramebufferPixels<uint8_t>(0, 0, 20, H, {255, 0, 0, 255});
    ExpectFramebufferPixels<uint8_t>(20, 0, W, H, {0, 0, 0, 0});

    ASSERT_GL_NO_ERROR();
}

ANGLE_INSTANTIATE_TEST_ES31(PixelLocalStorageTest);
