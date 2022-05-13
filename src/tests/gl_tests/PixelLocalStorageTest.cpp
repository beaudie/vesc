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
    void beginPixelLocalStorage(GLsizei n, const GLenum *loadOps);
    void pixelLocalStorageBarrier();
    void endPixelLocalStorage();

  private:
    struct LocalStoragePlane
    {
        GLenum internalformat;
        GLuint tex;
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
    boundLocalStoragePlanes()[unit] = {internalformat, backingtexture};
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
    assert(mNumEnabledLocalStoragePlanes == 0);       // GL_INVALID_OPERATION!

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

    const auto &localStoragePlanes = boundLocalStoragePlanes();
    for (int i = 0; i < n; ++i)
    {
        GLenum internalformat = GL_RGBA8;
        GLuint tex            = 0;
        if (loadOps[i] != GL_DISABLED_ANGLE)
        {
            internalformat = localStoragePlanes[i].internalformat;
            tex            = localStoragePlanes[i].tex;
            assert(tex);  // GL_INVALID_FRAMMEBUFFER_OPERATION!
        }
        // Attach all local storage backing textures to the framebuffer, which we will disable via
        // glDrawBuffers during local storage draws. This allows us to easily clear local storage in
        // a way that hints to the driver that this is the beginning of a render pass
        GLenum attachmentPoint = GL_COLOR_ATTACHMENT0 + MAX_FRAGMENT_OUTPUTS_WITH_LOCAL_STORAGE + i;
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentPoint, GL_TEXTURE_2D, tex, 0);
        if (loadOps[i] == GL_ZERO)
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
        // TODO: We should use glClearBuffer here.
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
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

        mProgram.makeRaster(
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

        ASSERT_TRUE(mProgram.valid());

        glUseProgram(mProgram);

        glUniform1f(glGetUniformLocation(mProgram, "W"), W);
        glUniform1f(glGetUniformLocation(mProgram, "H"), H);

        mLTRBLocation = glGetAttribLocation(mProgram, "inltrb");
        glEnableVertexAttribArray(mLTRBLocation);
        glVertexAttribDivisor(mLTRBLocation, 1);

        mRGBALocation = glGetAttribLocation(mProgram, "incolor");
        glEnableVertexAttribArray(mRGBALocation);
        glVertexAttribDivisor(mRGBALocation, 1);

        mAux1Location = glGetAttribLocation(mProgram, "inaux1");
        glEnableVertexAttribArray(mAux1Location);
        glVertexAttribDivisor(mAux1Location, 1);

        mAux2Location = glGetAttribLocation(mProgram, "inaux2");
        glEnableVertexAttribArray(mAux2Location);
        glVertexAttribDivisor(mAux2Location, 1);
    }

    struct Box
    {
        using float4 = std::array<float, 4>;
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

    GLProgram mProgram;
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

        glBeginPixelLocalStorageANGLE(3, std::vector<GLenum>{GL_ZERO, GL_ZERO, GL_ZERO}.data());

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

        glBeginPixelLocalStorageANGLE(2, std::vector<GLenum>{GL_ZERO, GL_ZERO}.data());

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

        glBeginPixelLocalStorageANGLE(3, std::vector<GLenum>{GL_ZERO, GL_ZERO, GL_ZERO}.data());

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

        glBeginPixelLocalStorageANGLE(2, std::vector<GLenum>{GL_ZERO, GL_ZERO}.data());

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
    glBeginPixelLocalStorageANGLE(1, std::vector<GLenum>{GL_ZERO}.data());
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
    glBeginPixelLocalStorageANGLE(1, std::vector<GLenum>{GL_ZERO}.data());
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

ANGLE_INSTANTIATE_TEST_ES31(PixelLocalStorageTest);
