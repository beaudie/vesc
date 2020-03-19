//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CovglTests.cpp:
//   GLES1 conformance covgl tests.
//   Function prototypes taken from tproto.h and turned into gtest tests using a macro.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(GL_OES_VERSION_1_1) && defined(GL_VERSION_ES_CM_1_1) && defined(GL_VERSION_ES_CL_1_1)
#    if GL_VERSION_ES_CM_1_1 || GL_VERSION_ES_CL_1_1
#        define GL_OES_VERSION_1_1
#    endif
#endif

// ES 1.0
extern void StateReset(void);
extern GLboolean tkCheckExtension(const char *name);

extern void CallActiveTexture(void);
extern void CallAlphaFunc(void);
extern void CallBlendFunc(void);
extern void CallBindDeleteTexture(void);
extern void CallClear(void);
extern void CallClearColor(void);
extern void CallClearDepth(void);
extern void CallClearStencil(void);
extern void CallClientActiveTexture(void);
extern void CallColor(void);
extern void CallColorMask(void);
extern void CallColorPointer(void);
extern void CallCompressedTexImage2D(void);
extern void CallCompressedTexSubImage2D(void);
extern void CallCopyTexImage2D(void);
extern void CallCopyTexSubImage2D(void);
extern void CallCullFace(void);
extern void CallDepthFunc(void);
extern void CallDepthMask(void);
extern void CallDepthRange(void);
extern void CallDrawArrays(void);
extern void CallDrawElements(void);

#ifdef GL_OES_draw_texture
extern void CallDrawTex(void);
#endif /* GL_OES_draw_texture */

extern void CallEdgeFlag(void);
extern void CallEnableDisable(void);
extern void CallEnableDisableClientState(void);
extern void CallFinish(void);
extern void CallFlush(void);
extern void CallFog(void);
extern void CallFrontFace(void);
extern void CallFrustum(void);
extern void CallGenTextures(void);
extern void CallGet(void);
extern void CallGetError(void);
extern void CallGetString(void);
#ifdef GL_OES_VERSION_1_1
extern void CallGetTexEnv(void);
extern void CallGetLight(void);
extern void CallGetMaterial(void);
extern void CallGetClipPlane(void);
extern void CallGetPointer(void);
#endif /* GL_OES_VERSION_1_1 */

#ifdef GL_OES_VERSION_1_1
extern void CallGetBufferParameter(void);
extern void CallGetTexParameter(void);
#endif /* GL_OES_VERSION_1_1 */

extern void CallHint(void);
extern void CallLight(void);
extern void CallLightModel(void);
extern void CallLineWidth(void);
extern void CallLoadIdentity(void);
extern void CallLoadMatrix(void);
extern void CallLogicOp(void);
extern void CallMaterial(void);
extern void CallMatrixMode(void);
extern void CallMultiTexCoord(void);
extern void CallMultMatrix(void);
extern void CallNormal(void);
extern void CallNormalPointer(void);
extern void CallOrtho(void);
extern void CallPixelStore(void);
#ifdef GL_OES_VERSION_1_1
extern void CallPointParameter(void);
#endif /* GL_OES_VERSION_1_1 */
extern void CallPointSize(void);
extern void CallPolygonOffset(void);
extern void CallPopMatrix(void);
extern void CallPushMatrix(void);
extern void CallReadPixels(void);
extern void CallRotate(void);
extern void CallSampleCoverage(void);
extern void CallScale(void);
extern void CallScissor(void);
extern void CallShadeModel(void);
extern void CallStencilFunc(void);
extern void CallStencilMask(void);
extern void CallStencilOp(void);
#ifdef GL_OES_VERSION_1_1
extern void CallIsTexture(void);
extern void CallIsEnabled(void);
#endif /* GL_OES_VERSION_1_1 */
extern void CallTexCoord(void);
extern void CallTexCoordPointer(void);
extern void CallTexEnv(void);
extern void CallTexImage2D(void);
extern void CallTexParameter(void);
extern void CallTexSubImage2D(void);
extern void CallTranslate(void);
extern void CallVertexPointer(void);
extern void CallViewport(void);

#ifdef GL_OES_VERSION_1_1
extern void CallBindDeleteBuffer(void);
extern void CallBufferData(void);
extern void CallBufferSubData(void);
extern void CallGenBuffers(void);
extern void CallIsBuffer(void);
extern void CallPointSizePointerOES(void);
extern void CallClipPlane(void);
#endif /* GL_OES_VERSION_1_1 */

#ifdef GL_OES_matrix_palette
extern void CallCurrentPaletteMatrixOES(void);
extern void CallLoadPaletteFromModelViewMatrixOES(void);
extern void CallMatrixIndexPointerOES(void);
extern void CallWeightPointerOES(void);
#endif /* GL_OES_matrix_palette */

#ifdef GL_OES_query_matrix
extern void CallQueryMatrix(void);
#endif

void FailAndDie(void)
{
    GTEST_FAIL();
}

void ProbeEnum(void)
{
    ASSERT_GL_NO_ERROR();
}

void ZeroBuf(GLenum type, long size, void *buf)
{
    size_t itemSize = 0;

    switch (type)
    {
        case GL_UNSIGNED_BYTE:
            itemSize = sizeof(unsigned char);
            break;
        case GL_BYTE:
            itemSize = sizeof(char);
            break;
        case GL_UNSIGNED_SHORT:
        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_5_5_5_1:
            itemSize = sizeof(unsigned short);
            break;
        case GL_SHORT:
            itemSize = sizeof(short);
            break;
#ifdef COMMON
        case GL_FLOAT:
            itemSize = sizeof(float);
            break;
#endif
        default:
            GTEST_FAIL();
    }
    memset(buf, 0, itemSize * size);
}

#ifdef __cplusplus
}

#endif

namespace angle
{
class GLES1CovglTest : public ANGLETest
{
  protected:
    GLES1CovglTest()
    {
        setWindowWidth(48);
        setWindowHeight(48);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setConfigStencilBits(8);
    }

    void testSetUp() override
    {
        StateReset();
        glViewport(0, 0, 48, 48);
        glScissor(0, 0, 48, 48);
    }
};

#define RUN_TEST_CALL(func) \
    func();                 \
    ASSERT_GL_NO_ERROR();   \
    StateReset();

TEST_P(GLES1CovglTest, Get)
{
    RUN_TEST_CALL(CallGet);
}

TEST_P(GLES1CovglTest, GetError)
{
    RUN_TEST_CALL(CallGetError);
}
TEST_P(GLES1CovglTest, GetString)
{
    RUN_TEST_CALL(CallGetString);
}

#ifdef GL_OES_VERSION_1_1
TEST_P(GLES1CovglTest, GetTexEnv)
{
    RUN_TEST_CALL(CallGetTexEnv);
}
TEST_P(GLES1CovglTest, GetLight)
{
    RUN_TEST_CALL(CallGetLight);
}
TEST_P(GLES1CovglTest, GetMaterial)
{
    RUN_TEST_CALL(CallGetMaterial);
}
TEST_P(GLES1CovglTest, GetClipPlane)
{
    RUN_TEST_CALL(CallGetClipPlane);
}
TEST_P(GLES1CovglTest, GetPointer)
{
    RUN_TEST_CALL(CallGetPointer);
}
TEST_P(GLES1CovglTest, GetTexParameter)
{
    RUN_TEST_CALL(CallGetTexParameter);
}
TEST_P(GLES1CovglTest, GetBufferParameter)
{
    RUN_TEST_CALL(CallGetBufferParameter);
}
#endif /* GL_OES_VERSION_1_1 */

TEST_P(GLES1CovglTest, EnableDisable)
{
    RUN_TEST_CALL(CallEnableDisable);
}

TEST_P(GLES1CovglTest, Hint)
{
    RUN_TEST_CALL(CallHint);
}

TEST_P(GLES1CovglTest, Viewport)
{
    RUN_TEST_CALL(CallViewport);
}
TEST_P(GLES1CovglTest, Ortho)
{
    RUN_TEST_CALL(CallOrtho);
}
TEST_P(GLES1CovglTest, Frustum)
{
    RUN_TEST_CALL(CallFrustum);
}
TEST_P(GLES1CovglTest, Scissor)
{
    RUN_TEST_CALL(CallScissor);
}

TEST_P(GLES1CovglTest, LoadIdentity)
{
    RUN_TEST_CALL(CallLoadIdentity);
}
TEST_P(GLES1CovglTest, MatrixMode)
{
    RUN_TEST_CALL(CallMatrixMode);
}
TEST_P(GLES1CovglTest, PushMatrix)
{
    RUN_TEST_CALL(CallPushMatrix);
}
TEST_P(GLES1CovglTest, LoadMatrix)
{
    RUN_TEST_CALL(CallLoadMatrix);
}
TEST_P(GLES1CovglTest, MultMatrix)
{
    RUN_TEST_CALL(CallMultMatrix);
}
TEST_P(GLES1CovglTest, Rotate)
{
    RUN_TEST_CALL(CallRotate);
}
TEST_P(GLES1CovglTest, Scale)
{
    RUN_TEST_CALL(CallScale);
}
TEST_P(GLES1CovglTest, Translate)
{
    RUN_TEST_CALL(CallTranslate);
}
TEST_P(GLES1CovglTest, PopMatrix)
{
    RUN_TEST_CALL(CallPopMatrix);
}

TEST_P(GLES1CovglTest, Clear)
{
    RUN_TEST_CALL(CallClear);
}
TEST_P(GLES1CovglTest, ClearColor)
{
    RUN_TEST_CALL(CallClearColor);
}
TEST_P(GLES1CovglTest, ClearDepth)
{
    RUN_TEST_CALL(CallClearDepth);
}
TEST_P(GLES1CovglTest, ClearStencil)
{
    RUN_TEST_CALL(CallClearStencil);
}

TEST_P(GLES1CovglTest, ColorMask)
{
    RUN_TEST_CALL(CallColorMask);
}
TEST_P(GLES1CovglTest, Color)
{
    RUN_TEST_CALL(CallColor);
}

TEST_P(GLES1CovglTest, Normal)
{
    RUN_TEST_CALL(CallNormal);
}

TEST_P(GLES1CovglTest, AlphaFunc)
{
    RUN_TEST_CALL(CallAlphaFunc);
}
TEST_P(GLES1CovglTest, BlendFunc)
{
    RUN_TEST_CALL(CallBlendFunc);
}
TEST_P(GLES1CovglTest, DepthFunc)
{
    RUN_TEST_CALL(CallDepthFunc);
}
TEST_P(GLES1CovglTest, DepthMask)
{
    RUN_TEST_CALL(CallDepthMask);
}
TEST_P(GLES1CovglTest, DepthRange)
{
    RUN_TEST_CALL(CallDepthRange);
}
TEST_P(GLES1CovglTest, LogicOp)
{
    RUN_TEST_CALL(CallLogicOp);
}
TEST_P(GLES1CovglTest, StencilFunc)
{
    RUN_TEST_CALL(CallStencilFunc);
}
TEST_P(GLES1CovglTest, StencilMask)
{
    RUN_TEST_CALL(CallStencilMask);
}
TEST_P(GLES1CovglTest, StencilOp)
{
    RUN_TEST_CALL(CallStencilOp);
}

TEST_P(GLES1CovglTest, PixelStore)
{
    RUN_TEST_CALL(CallPixelStore);
}
TEST_P(GLES1CovglTest, ReadPixels)
{
    RUN_TEST_CALL(CallReadPixels);
}

TEST_P(GLES1CovglTest, Fog)
{
    RUN_TEST_CALL(CallFog);
}
TEST_P(GLES1CovglTest, LightModel)
{
    RUN_TEST_CALL(CallLightModel);
}
TEST_P(GLES1CovglTest, Light)
{
    RUN_TEST_CALL(CallLight);
}
TEST_P(GLES1CovglTest, Material)
{
    RUN_TEST_CALL(CallMaterial);
}

#ifdef GL_OES_VERSION_1_1
TEST_P(GLES1CovglTest, IsTexture)
{
    RUN_TEST_CALL(CallIsTexture);
}
TEST_P(GLES1CovglTest, IsEnabled)
{
    RUN_TEST_CALL(CallIsEnabled);
}
#endif /* GL_OES_VERSION_1_1 */

TEST_P(GLES1CovglTest, TexEnv)
{
    RUN_TEST_CALL(CallTexEnv);
}
TEST_P(GLES1CovglTest, TexParameter)
{
    RUN_TEST_CALL(CallTexParameter);
}
TEST_P(GLES1CovglTest, TexImage2D)
{
    RUN_TEST_CALL(CallTexImage2D);
}
TEST_P(GLES1CovglTest, TexSubImage2D)
{
    RUN_TEST_CALL(CallTexSubImage2D);
}
TEST_P(GLES1CovglTest, GenTextures)
{
    RUN_TEST_CALL(CallGenTextures);
}
TEST_P(GLES1CovglTest, BindDeleteTexture)
{
    RUN_TEST_CALL(CallBindDeleteTexture);
}
TEST_P(GLES1CovglTest, CopyTexImage2D)
{
    RUN_TEST_CALL(CallCopyTexImage2D);
}
TEST_P(GLES1CovglTest, CopyTexSubImage2D)
{
    RUN_TEST_CALL(CallCopyTexSubImage2D);
}
TEST_P(GLES1CovglTest, CompressedTexImage2D)
{
    RUN_TEST_CALL(CallCompressedTexImage2D);
}
TEST_P(GLES1CovglTest, CompressedTexSubImage2D)
{
    RUN_TEST_CALL(CallCompressedTexSubImage2D);
}

#ifdef GL_OES_VERSION_1_1
TEST_P(GLES1CovglTest, BindDeleteBuffer)
{
    RUN_TEST_CALL(CallBindDeleteBuffer);
}
TEST_P(GLES1CovglTest, IsBuffer)
{
    RUN_TEST_CALL(CallIsBuffer);
}
TEST_P(GLES1CovglTest, BufferData)
{
    RUN_TEST_CALL(CallBufferData);
}
TEST_P(GLES1CovglTest, BufferSubData)
{
    RUN_TEST_CALL(CallBufferSubData);
}
TEST_P(GLES1CovglTest, GenBuffers)
{
    RUN_TEST_CALL(CallGenBuffers);
}
#endif /* GL_OES_VERSION_1_1 */

TEST_P(GLES1CovglTest, ShadeModel)
{
    RUN_TEST_CALL(CallShadeModel);
}
TEST_P(GLES1CovglTest, PointSize)
{
    RUN_TEST_CALL(CallPointSize);
}
TEST_P(GLES1CovglTest, LineWidth)
{
    RUN_TEST_CALL(CallLineWidth);
}
TEST_P(GLES1CovglTest, CullFace)
{
    RUN_TEST_CALL(CallCullFace);
}
TEST_P(GLES1CovglTest, FrontFace)
{
    RUN_TEST_CALL(CallFrontFace);
}
TEST_P(GLES1CovglTest, PolygonOffset)
{
    RUN_TEST_CALL(CallPolygonOffset);
}

#ifdef GL_OES_VERSION_1_1
TEST_P(GLES1CovglTest, PointParameter)
{
    RUN_TEST_CALL(CallPointParameter);
}
#endif /* GL_OES_VERSION_1_1 */

TEST_P(GLES1CovglTest, Flush)
{
    RUN_TEST_CALL(CallFlush);
}
TEST_P(GLES1CovglTest, Finish)
{
    RUN_TEST_CALL(CallFinish);
}

TEST_P(GLES1CovglTest, ColorPointer)
{
    RUN_TEST_CALL(CallColorPointer);
}
TEST_P(GLES1CovglTest, DrawArrays)
{
    RUN_TEST_CALL(CallDrawArrays);
}
TEST_P(GLES1CovglTest, DrawElements)
{
    RUN_TEST_CALL(CallDrawElements);
}
#ifdef GL_OES_draw_texture
TEST_P(GLES1CovglTest, DrawTex)
{
    ANGLE_SKIP_TEST_IF(!tkCheckExtension("GL_OES_draw_texture"));

    RUN_TEST_CALL(CallDrawTex);
}
#endif /*GL_OES_draw_texture */
TEST_P(GLES1CovglTest, NormalPointer)
{
    RUN_TEST_CALL(CallNormalPointer);
}
TEST_P(GLES1CovglTest, TexCoordPointer)
{
    RUN_TEST_CALL(CallTexCoordPointer);
}
TEST_P(GLES1CovglTest, VertexPointer)
{
    RUN_TEST_CALL(CallVertexPointer);
}
#ifdef GL_OES_VERSION_1_1
TEST_P(GLES1CovglTest, PointSizePointerOES)
{
    RUN_TEST_CALL(CallPointSizePointerOES);
}
#endif
TEST_P(GLES1CovglTest, EnableDisableClientState)
{
    RUN_TEST_CALL(CallEnableDisableClientState);
}

TEST_P(GLES1CovglTest, ActiveTexture)
{
    RUN_TEST_CALL(CallActiveTexture);
}
TEST_P(GLES1CovglTest, ClientActiveTexture)
{
    RUN_TEST_CALL(CallClientActiveTexture);
}
TEST_P(GLES1CovglTest, MultiTexCoord)
{
    RUN_TEST_CALL(CallMultiTexCoord);
}

TEST_P(GLES1CovglTest, SampleCoverage)
{
    RUN_TEST_CALL(CallSampleCoverage);
}

#ifdef GL_OES_query_matrix
TEST_P(GLES1CovglTest, QueryMatrix)
{
    ANGLE_SKIP_TEST_IF(!tkCheckExtension("GL_OES_query_matrix"));
    RUN_TEST_CALL(CallQueryMatrix);
}
#endif

#ifdef GL_OES_matrix_palette
TEST_P(GLES1CovglTest, CurrentPaletteMatrixOES)
{
    ANGLE_SKIP_TEST_IF(!tkCheckExtension("GL_OES_matrix_palette"));
    RUN_TEST_CALL(CallCurrentPaletteMatrixOES);
}
TEST_P(GLES1CovglTest, LoadPaletteFromModelViewMatrixOES)
{
    ANGLE_SKIP_TEST_IF(!tkCheckExtension("GL_OES_matrix_palette"));
    RUN_TEST_CALL(CallLoadPaletteFromModelViewMatrixOES);
}
TEST_P(GLES1CovglTest, MatrixIndexPointerOES)
{
    ANGLE_SKIP_TEST_IF(!tkCheckExtension("GL_OES_matrix_palette"));
    RUN_TEST_CALL(CallMatrixIndexPointerOES);
}
TEST_P(GLES1CovglTest, WeightPointerOES)
{
    ANGLE_SKIP_TEST_IF(!tkCheckExtension("GL_OES_matrix_palette"));
    RUN_TEST_CALL(CallWeightPointerOES);
}
#endif

#ifdef GL_OES_VERSION_1_1
TEST_P(GLES1CovglTest, ClipPlane)
{
    RUN_TEST_CALL(CallClipPlane);
}
#endif

ANGLE_INSTANTIATE_TEST(GLES1CovglTest, ES1_OPENGL(), ES1_VULKAN());
}  // namespace angle
