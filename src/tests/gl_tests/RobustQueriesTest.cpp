//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RobustQueriesTest.cpp : Tests of the GL_ANGLE_robust_queries extension.

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"

namespace angle
{
    class RobustQueriestTest : public ANGLETest
    {
    protected:
        RobustQueriestTest()
        {
            setWindowWidth(128);
            setWindowHeight(128);
            setConfigRedBits(8);
            setConfigGreenBits(8);
            setConfigBlueBits(8);
            setConfigAlphaBits(8);
        }

        void SetUp() override
        {
            ANGLETest::SetUp();

            glGetBooleanvRobustANGLE = reinterpret_cast<PFNGLGETBOOLEANVROBUSTANGLE>(eglGetProcAddress("glGetBooleanvRobustANGLE"));
            glGetBufferParameterivRobustANGLE = reinterpret_cast<PFNGLGETBUFFERPARAMETERIVROBUSTANGLE>(eglGetProcAddress("glGetBufferParameterivRobustANGLE"));
            glGetFloatvRobustANGLE = reinterpret_cast<PFNGLGETFLOATVROBUSTANGLE>(eglGetProcAddress("glGetFloatvRobustANGLE"));
            glGetFramebufferAttachmentParameterivRobustANGLE = reinterpret_cast<PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVROBUSTANGLE>(eglGetProcAddress("glGetFramebufferAttachmentParameterivRobustANGLE"));
            glGetIntegervRobustANGLE = reinterpret_cast<PFNGLGETINTEGERVROBUSTANGLE>(eglGetProcAddress("glGetIntegervRobustANGLE"));
            glGetProgramivRobustANGLE = reinterpret_cast<PFNGLGETPROGRAMIVROBUSTANGLE>(eglGetProcAddress("glGetProgramivRobustANGLE"));
            glGetRenderbufferParameterivRobustANGLE = reinterpret_cast<PFNGLGETRENDERBUFFERPARAMETERIVROBUSTANGLE>(eglGetProcAddress("glGetRenderbufferParameterivRobustANGLE"));
            glGetShaderivRobustANGLE = reinterpret_cast<PFNGLGETSHADERIVROBUSTANGLE>(eglGetProcAddress("glGetShaderivRobustANGLE"));
            glGetTexParameterfvRobustANGLE = reinterpret_cast<PFNGLGETTEXPARAMETERFVROBUSTANGLE>(eglGetProcAddress("glGetTexParameterfvRobustANGLE"));
            glGetTexParameterivRobustANGLE = reinterpret_cast<PFNGLGETTEXPARAMETERIVROBUSTANGLE>(eglGetProcAddress("glGetTexParameterivRobustANGLE"));
            glGetUniformfvRobustANGLE = reinterpret_cast<PFNGLGETUNIFORMFVROBUSTANGLE>(eglGetProcAddress("glGetUniformfvRobustANGLE"));
            glGetUniformivRobustANGLE = reinterpret_cast<PFNGLGETUNIFORMIVROBUSTANGLE>(eglGetProcAddress("glGetUniformivRobustANGLE"));
            glGetVertexAttribfvRobustANGLE = reinterpret_cast<PFNGLGETVERTEXATTRIBFVROBUSTANGLE>(eglGetProcAddress("glGetVertexAttribfvRobustANGLE"));
            glGetVertexAttribivRobustANGLE = reinterpret_cast<PFNGLGETVERTEXATTRIBIVROBUSTANGLE>(eglGetProcAddress("glGetVertexAttribivRobustANGLE"));
            glGetVertexAttribPointervRobustANGLE = reinterpret_cast<PFNGLGETVERTEXATTRIBPOINTERVROBUSTANGLE>(eglGetProcAddress("glGetVertexAttribPointervRobustANGLE"));
            glReadPixelsRobustANGLE = reinterpret_cast<PFNGLREADPIXELSROBUSTANGLE>(eglGetProcAddress("glReadPixelsRobustANGLE"));
            glTexImage2DRobustANGLE = reinterpret_cast<PFNGLTEXIMAGE2DROBUSTANGLE>(eglGetProcAddress("glTexImage2DRobustANGLE"));
            glTexParameterfvRobustANGLE = reinterpret_cast<PFNGLTEXPARAMETERFVROBUSTANGLE>(eglGetProcAddress("glTexParameterfvRobustANGLE"));
            glTexParameterivRobustANGLE = reinterpret_cast<PFNGLTEXPARAMETERIVROBUSTANGLE>(eglGetProcAddress("glTexParameterivRobustANGLE"));
            glTexSubImage2DRobustANGLE = reinterpret_cast<PFNGLTEXSUBIMAGE2DROBUSTANGLE>(eglGetProcAddress("glTexSubImage2DRobustANGLE"));
            glTexImage3DRobustANGLE = reinterpret_cast<PFNGLTEXIMAGE3DROBUSTANGLE>(eglGetProcAddress("glTexImage3DRobustANGLE"));
            glTexSubImage3DRobustANGLE = reinterpret_cast<PFNGLTEXSUBIMAGE3DROBUSTANGLE>(eglGetProcAddress("glTexSubImage3DRobustANGLE"));
            glGetQueryivRobustANGLE = reinterpret_cast<PFNGLGETQUERYIVROBUSTANGLE>(eglGetProcAddress("glGetQueryivRobustANGLE"));
            glGetQueryObjectuivRobustANGLE = reinterpret_cast<PFNGLGETQUERYOBJECTUIVROBUSTANGLE>(eglGetProcAddress("glGetQueryObjectuivRobustANGLE"));
            glGetBufferPointervRobustANGLE = reinterpret_cast<PFNGLGETBUFFERPOINTERVROBUSTANGLE>(eglGetProcAddress("glGetBufferPointervRobustANGLE"));
            glGetIntegeri_vRobustANGLE = reinterpret_cast<PFNGLGETINTEGERI_VROBUSTANGLE>(eglGetProcAddress("glGetIntegeri_vRobustANGLE"));
            glGetVertexAttribIivRobustANGLE = reinterpret_cast<PFNGLGETVERTEXATTRIBIIVROBUSTANGLE>(eglGetProcAddress("glGetVertexAttribIivRobustANGLE"));
            glGetVertexAttribIuivRobustANGLE = reinterpret_cast<PFNGLGETVERTEXATTRIBIUIVROBUSTANGLE>(eglGetProcAddress("glGetVertexAttribIuivRobustANGLE"));
            glGetUniformuivRobustANGLE = reinterpret_cast<PFNGLGETUNIFORMUIVROBUSTANGLE>(eglGetProcAddress("glGetUniformuivRobustANGLE"));
            glGetActiveUniformBlockivRobustANGLE = reinterpret_cast<PFNGLGETACTIVEUNIFORMBLOCKIVROBUSTANGLE>(eglGetProcAddress("glGetActiveUniformBlockivRobustANGLE"));
            glGetInteger64vRobustANGLE = reinterpret_cast<PFNGLGETINTEGER64VROBUSTANGLE>(eglGetProcAddress("glGetInteger64vRobustANGLE"));
            glGetInteger64i_vRobustANGLE = reinterpret_cast<PFNGLGETINTEGER64I_VROBUSTANGLE>(eglGetProcAddress("glGetInteger64i_vRobustANGLE"));
            glGetBufferParameteri64vRobustANGLE = reinterpret_cast<PFNGLGETBUFFERPARAMETERI64VROBUSTANGLE>(eglGetProcAddress("glGetBufferParameteri64vRobustANGLE"));
            glSamplerParameterivRobustANGLE = reinterpret_cast<PFNGLSAMPLERPARAMETERIVROBUSTANGLE>(eglGetProcAddress("glSamplerParameterivRobustANGLE"));
            glSamplerParameterfvRobustANGLE = reinterpret_cast<PFNGLSAMPLERPARAMETERFVROBUSTANGLE>(eglGetProcAddress("glSamplerParameterfvRobustANGLE"));
            glGetSamplerParameterivRobustANGLE = reinterpret_cast<PFNGLGETSAMPLERPARAMETERIVROBUSTANGLE>(eglGetProcAddress("glGetSamplerParameterivRobustANGLE"));
            glGetSamplerParameterfvRobustANGLE = reinterpret_cast<PFNGLGETSAMPLERPARAMETERFVROBUSTANGLE>(eglGetProcAddress("glGetSamplerParameterfvRobustANGLE"));
            glGetFramebufferParameterivRobustANGLE = reinterpret_cast<PFNGLGETFRAMEBUFFERPARAMETERIVROBUSTANGLE>(eglGetProcAddress("glGetFramebufferParameterivRobustANGLE"));
            glGetProgramInterfaceivRobustANGLE = reinterpret_cast<PFNGLGETPROGRAMINTERFACEIVROBUSTANGLE>(eglGetProcAddress("glGetProgramInterfaceivRobustANGLE"));
            glGetBooleani_vRobustANGLE = reinterpret_cast<PFNGLGETBOOLEANI_VROBUSTANGLE>(eglGetProcAddress("glGetBooleani_vRobustANGLE"));
            glGetMultisamplefvRobustANGLE = reinterpret_cast<PFNGLGETMULTISAMPLEFVROBUSTANGLE>(eglGetProcAddress("glGetMultisamplefvRobustANGLE"));
            glGetTexLevelParameterivRobustANGLE = reinterpret_cast<PFNGLGETTEXLEVELPARAMETERIVROBUSTANGLE>(eglGetProcAddress("glGetTexLevelParameterivRobustANGLE"));
            glGetTexLevelParameterfvRobustANGLE = reinterpret_cast<PFNGLGETTEXLEVELPARAMETERFVROBUSTANGLE>(eglGetProcAddress("glGetTexLevelParameterfvRobustANGLE"));
            glGetPointervRobustANGLERobustANGLE = reinterpret_cast<PFNGLGETPOINTERVROBUSTANGLEROBUSTANGLE>(eglGetProcAddress("glGetPointervRobustANGLERobustANGLE"));
            glReadnPixelsRobustANGLE = reinterpret_cast<PFNGLREADNPIXELSROBUSTANGLE>(eglGetProcAddress("glReadnPixelsRobustANGLE"));
            glGetnUniformfvRobustANGLE = reinterpret_cast<PFNGLGETNUNIFORMFVROBUSTANGLE>(eglGetProcAddress("glGetnUniformfvRobustANGLE"));
            glGetnUniformivRobustANGLE = reinterpret_cast<PFNGLGETNUNIFORMIVROBUSTANGLE>(eglGetProcAddress("glGetnUniformivRobustANGLE"));
            glGetnUniformuivRobustANGLE = reinterpret_cast<PFNGLGETNUNIFORMUIVROBUSTANGLE>(eglGetProcAddress("glGetnUniformuivRobustANGLE"));
            glTexParameterIivRobustANGLE = reinterpret_cast<PFNGLTEXPARAMETERIIVROBUSTANGLE>(eglGetProcAddress("glTexParameterIivRobustANGLE"));
            glTexParameterIuivRobustANGLE = reinterpret_cast<PFNGLTEXPARAMETERIUIVROBUSTANGLE>(eglGetProcAddress("glTexParameterIuivRobustANGLE"));
            glGetTexParameterIivRobustANGLE = reinterpret_cast<PFNGLGETTEXPARAMETERIIVROBUSTANGLE>(eglGetProcAddress("glGetTexParameterIivRobustANGLE"));
            glGetTexParameterIuivRobustANGLE = reinterpret_cast<PFNGLGETTEXPARAMETERIUIVROBUSTANGLE>(eglGetProcAddress("glGetTexParameterIuivRobustANGLE"));
            glSamplerParameterIivRobustANGLE = reinterpret_cast<PFNGLSAMPLERPARAMETERIIVROBUSTANGLE>(eglGetProcAddress("glSamplerParameterIivRobustANGLE"));
            glSamplerParameterIuivRobustANGLE = reinterpret_cast<PFNGLSAMPLERPARAMETERIUIVROBUSTANGLE>(eglGetProcAddress("glSamplerParameterIuivRobustANGLE"));
            glGetSamplerParameterIivRobustANGLE = reinterpret_cast<PFNGLGETSAMPLERPARAMETERIIVROBUSTANGLE>(eglGetProcAddress("glGetSamplerParameterIivRobustANGLE"));
            glGetSamplerParameterIuivRobustANGLE = reinterpret_cast<PFNGLGETSAMPLERPARAMETERIUIVROBUSTANGLE>(eglGetProcAddress("glGetSamplerParameterIuivRobustANGLE"));
            glGetQueryObjectivRobustANGLE = reinterpret_cast<PFNGLGETQUERYOBJECTIVROBUSTANGLE>(eglGetProcAddress("glGetQueryObjectivRobustANGLE"));
            glGetQueryObjecti64vRobustANGLE = reinterpret_cast<PFNGLGETQUERYOBJECTI64VROBUSTANGLE>(eglGetProcAddress("glGetQueryObjecti64vRobustANGLE"));
            glGetQueryObjectui64vRobustANGLE = reinterpret_cast<PFNGLGETQUERYOBJECTUI64VROBUSTANGLE>(eglGetProcAddress("glGetQueryObjectui64vRobustANGLE"));
        }

        void TearDown() override { ANGLETest::TearDown(); }

        PFNGLGETBOOLEANVROBUSTANGLE glGetBooleanvRobustANGLE = nullptr;
        PFNGLGETBUFFERPARAMETERIVROBUSTANGLE glGetBufferParameterivRobustANGLE = nullptr;
        PFNGLGETFLOATVROBUSTANGLE glGetFloatvRobustANGLE = nullptr;
        PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVROBUSTANGLE glGetFramebufferAttachmentParameterivRobustANGLE = nullptr;
        PFNGLGETINTEGERVROBUSTANGLE glGetIntegervRobustANGLE = nullptr;
        PFNGLGETPROGRAMIVROBUSTANGLE glGetProgramivRobustANGLE = nullptr;
        PFNGLGETRENDERBUFFERPARAMETERIVROBUSTANGLE glGetRenderbufferParameterivRobustANGLE = nullptr;
        PFNGLGETSHADERIVROBUSTANGLE glGetShaderivRobustANGLE = nullptr;
        PFNGLGETTEXPARAMETERFVROBUSTANGLE glGetTexParameterfvRobustANGLE = nullptr;
        PFNGLGETTEXPARAMETERIVROBUSTANGLE glGetTexParameterivRobustANGLE = nullptr;
        PFNGLGETUNIFORMFVROBUSTANGLE glGetUniformfvRobustANGLE = nullptr;
        PFNGLGETUNIFORMIVROBUSTANGLE glGetUniformivRobustANGLE = nullptr;
        PFNGLGETVERTEXATTRIBFVROBUSTANGLE glGetVertexAttribfvRobustANGLE = nullptr;
        PFNGLGETVERTEXATTRIBIVROBUSTANGLE glGetVertexAttribivRobustANGLE = nullptr;
        PFNGLGETVERTEXATTRIBPOINTERVROBUSTANGLE glGetVertexAttribPointervRobustANGLE = nullptr;
        PFNGLREADPIXELSROBUSTANGLE glReadPixelsRobustANGLE = nullptr;
        PFNGLTEXIMAGE2DROBUSTANGLE glTexImage2DRobustANGLE = nullptr;
        PFNGLTEXPARAMETERFVROBUSTANGLE glTexParameterfvRobustANGLE = nullptr;
        PFNGLTEXPARAMETERIVROBUSTANGLE glTexParameterivRobustANGLE = nullptr;
        PFNGLTEXSUBIMAGE2DROBUSTANGLE glTexSubImage2DRobustANGLE = nullptr;
        PFNGLTEXIMAGE3DROBUSTANGLE glTexImage3DRobustANGLE = nullptr;
        PFNGLTEXSUBIMAGE3DROBUSTANGLE glTexSubImage3DRobustANGLE = nullptr;
        PFNGLGETQUERYIVROBUSTANGLE glGetQueryivRobustANGLE = nullptr;
        PFNGLGETQUERYOBJECTUIVROBUSTANGLE glGetQueryObjectuivRobustANGLE = nullptr;
        PFNGLGETBUFFERPOINTERVROBUSTANGLE glGetBufferPointervRobustANGLE = nullptr;
        PFNGLGETINTEGERI_VROBUSTANGLE glGetIntegeri_vRobustANGLE = nullptr;
        PFNGLGETVERTEXATTRIBIIVROBUSTANGLE glGetVertexAttribIivRobustANGLE = nullptr;
        PFNGLGETVERTEXATTRIBIUIVROBUSTANGLE glGetVertexAttribIuivRobustANGLE = nullptr;
        PFNGLGETUNIFORMUIVROBUSTANGLE glGetUniformuivRobustANGLE = nullptr;
        PFNGLGETACTIVEUNIFORMBLOCKIVROBUSTANGLE glGetActiveUniformBlockivRobustANGLE = nullptr;
        PFNGLGETINTEGER64VROBUSTANGLE glGetInteger64vRobustANGLE = nullptr;
        PFNGLGETINTEGER64I_VROBUSTANGLE glGetInteger64i_vRobustANGLE = nullptr;
        PFNGLGETBUFFERPARAMETERI64VROBUSTANGLE glGetBufferParameteri64vRobustANGLE = nullptr;
        PFNGLSAMPLERPARAMETERIVROBUSTANGLE glSamplerParameterivRobustANGLE = nullptr;
        PFNGLSAMPLERPARAMETERFVROBUSTANGLE glSamplerParameterfvRobustANGLE = nullptr;
        PFNGLGETSAMPLERPARAMETERIVROBUSTANGLE glGetSamplerParameterivRobustANGLE = nullptr;
        PFNGLGETSAMPLERPARAMETERFVROBUSTANGLE glGetSamplerParameterfvRobustANGLE = nullptr;
        PFNGLGETFRAMEBUFFERPARAMETERIVROBUSTANGLE glGetFramebufferParameterivRobustANGLE = nullptr;
        PFNGLGETPROGRAMINTERFACEIVROBUSTANGLE glGetProgramInterfaceivRobustANGLE = nullptr;
        PFNGLGETBOOLEANI_VROBUSTANGLE glGetBooleani_vRobustANGLE = nullptr;
        PFNGLGETMULTISAMPLEFVROBUSTANGLE glGetMultisamplefvRobustANGLE = nullptr;
        PFNGLGETTEXLEVELPARAMETERIVROBUSTANGLE glGetTexLevelParameterivRobustANGLE = nullptr;
        PFNGLGETTEXLEVELPARAMETERFVROBUSTANGLE glGetTexLevelParameterfvRobustANGLE = nullptr;
        PFNGLGETPOINTERVROBUSTANGLEROBUSTANGLE glGetPointervRobustANGLERobustANGLE = nullptr;
        PFNGLREADNPIXELSROBUSTANGLE glReadnPixelsRobustANGLE = nullptr;
        PFNGLGETNUNIFORMFVROBUSTANGLE glGetnUniformfvRobustANGLE = nullptr;
        PFNGLGETNUNIFORMIVROBUSTANGLE glGetnUniformivRobustANGLE = nullptr;
        PFNGLGETNUNIFORMUIVROBUSTANGLE glGetnUniformuivRobustANGLE = nullptr;
        PFNGLTEXPARAMETERIIVROBUSTANGLE glTexParameterIivRobustANGLE = nullptr;
        PFNGLTEXPARAMETERIUIVROBUSTANGLE glTexParameterIuivRobustANGLE = nullptr;
        PFNGLGETTEXPARAMETERIIVROBUSTANGLE glGetTexParameterIivRobustANGLE = nullptr;
        PFNGLGETTEXPARAMETERIUIVROBUSTANGLE glGetTexParameterIuivRobustANGLE = nullptr;
        PFNGLSAMPLERPARAMETERIIVROBUSTANGLE glSamplerParameterIivRobustANGLE = nullptr;
        PFNGLSAMPLERPARAMETERIUIVROBUSTANGLE glSamplerParameterIuivRobustANGLE = nullptr;
        PFNGLGETSAMPLERPARAMETERIIVROBUSTANGLE glGetSamplerParameterIivRobustANGLE = nullptr;
        PFNGLGETSAMPLERPARAMETERIUIVROBUSTANGLE glGetSamplerParameterIuivRobustANGLE = nullptr;
        PFNGLGETQUERYOBJECTIVROBUSTANGLE glGetQueryObjectivRobustANGLE = nullptr;
        PFNGLGETQUERYOBJECTI64VROBUSTANGLE glGetQueryObjecti64vRobustANGLE = nullptr;
        PFNGLGETQUERYOBJECTUI64VROBUSTANGLE glGetQueryObjectui64vRobustANGLE = nullptr;
    };

    // Verify that all extension entry points are available
    TEST_P(RobustQueriestTest, EntryPoints)
    {
        if (extensionEnabled("GL_ANGLE_webgl_compatibility"))
        {
            EXPECT_NE(nullptr, eglGetProcAddress("glEnableExtensionANGLE"));
        }
    }

    // WebGL 1 allows GL_DEPTH_STENCIL_ATTACHMENT as a valid binding point.  Make sure it is usable,
    // even in ES2 contexts.
    TEST_P(WebGLCompatibilityTest, DepthStencilBindingPoint)
    {
        GLRenderbuffer renderbuffer;
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer.get());
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 32, 32);

        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.get());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
            renderbuffer.get());

        EXPECT_GL_NO_ERROR();
    }

    // Test that attempting to enable an extension that doesn't exist generates GL_INVALID_OPERATION
    TEST_P(WebGLCompatibilityTest, EnableExtensionValidation)
    {
        EXPECT_EQ(GL_FALSE, glEnableExtensionANGLE("invalid_extension_string"));
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    }

    // Test enabling the GL_OES_element_index_uint extension
    TEST_P(WebGLCompatibilityTest, EnableExtensionUintIndices)
    {
        if (getClientMajorVersion() != 2)
        {
            // This test only works on ES2 where uint indices are not available by default
            return;
        }

        EXPECT_FALSE(extensionEnabled("GL_OES_element_index_uint"));

        GLBuffer indexBuffer;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.get());

        GLuint data[] = { 0, 1, 2, 1, 3, 2 };
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

        ANGLE_GL_PROGRAM(program, "void main() { gl_Position = vec4(0, 0, 0, 1); }",
            "void main() { gl_FragColor = vec4(0, 1, 0, 1); }")
            glUseProgram(program.get());

        glDrawElements(GL_TRIANGLES, 2, GL_UNSIGNED_INT, nullptr);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);

        if (glEnableExtensionANGLE("GL_OES_element_index_uint"))
        {
            EXPECT_GL_NO_ERROR();
            EXPECT_TRUE(extensionEnabled("GL_OES_element_index_uint"));

            glDrawElements(GL_TRIANGLES, 2, GL_UNSIGNED_INT, nullptr);
            EXPECT_GL_NO_ERROR();
        }
    }

    // Use this to select which configurations (e.g. which renderer, which GLES major version) these
    // tests should be run against.
    ANGLE_INSTANTIATE_TEST(WebGLCompatibilityTest,
        ES2_D3D9(),
        ES2_D3D11(),
        ES3_D3D11(),
        ES2_D3D11_FL9_3(),
        ES2_OPENGL(),
        ES3_OPENGL(),
        ES2_OPENGLES(),
        ES3_OPENGLES());

}  // namespace
