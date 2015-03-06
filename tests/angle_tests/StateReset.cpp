/*-------------------------------------------------------------------------
 * drawElements Quality Program OpenGL ES Utilities
 * ------------------------------------------------
 *
 * Copyright 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *//*!
 * \file
 * \brief OpenGL State Reset.
 *//*--------------------------------------------------------------------*/

#include "ANGLETest.h"

void ANGLETest::resetState()
{
    GLint clientVersion = getClientVersion();

    // Reset error state.
    EXPECT_GL_NO_ERROR();

    // Vertex attrib array state.
    {
        int numVertexAttribArrays = 0;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numVertexAttribArrays);

        glBindBuffer    (GL_ARRAY_BUFFER,           0);
        glBindBuffer    (GL_ELEMENT_ARRAY_BUFFER,   0);

        if (clientVersion >= 3)
        {
            glBindVertexArray  (0);
            glDisable          (GL_PRIMITIVE_RESTART_FIXED_INDEX);
        }

        //if (contextSupports(type, ApiType::es(3,1)))
        //    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

        for (int ndx = 0; ndx < numVertexAttribArrays; ndx++)
        {
            glDisableVertexAttribArray (ndx);
            glVertexAttribPointer      (ndx, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

            if (clientVersion >= 3)
                glVertexAttribDivisor(ndx, 0);

            if (extensionEnabled("GL_ANGLE_instanced_arrays"))
            {
                auto vertexAttribDivisorANGLE = (PFNGLVERTEXATTRIBDIVISORANGLEPROC)eglGetProcAddress("glVertexAttribDivisorANGLE");
                vertexAttribDivisorANGLE(ndx, 0);
            }
        }

        EXPECT_GL_NO_ERROR();
    }

    // Transformation state.
    {
        GLint viewportWidth = mEGLWindow->getWidth();
        GLint viewportHeight = mEGLWindow->getHeight();
        glViewport     (0, 0, viewportWidth, viewportHeight);
        glDepthRangef  (0.0f, 1.0f);

        if (clientVersion >= 3)
            glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

        EXPECT_GL_NO_ERROR();
    }

    // Rasterization state
    {
        glLineWidth    (1.0f);
        glDisable      (GL_CULL_FACE);
        glCullFace     (GL_BACK);
        glFrontFace    (GL_CCW);
        glPolygonOffset(0.0f, 0.0f);
        glDisable      (GL_POLYGON_OFFSET_FILL);

        if (clientVersion >= 3)
            glDisable(GL_RASTERIZER_DISCARD);

        EXPECT_GL_NO_ERROR();
    }

    // Multisampling state
    {
        glDisable          (GL_SAMPLE_ALPHA_TO_COVERAGE);
        glDisable          (GL_SAMPLE_COVERAGE);
        glSampleCoverage   (1.0f, GL_FALSE);

        //if (contextSupports(type, ApiType::es(3,1)))
        //{
        //    int numSampleMaskWords = 0;
        //    glGetIntegerv(GL_MAX_SAMPLE_MASK_WORDS, &numSampleMaskWords);

        //    glDisable(GL_SAMPLE_MASK);

        //    for (int ndx = 0; ndx < numSampleMaskWords; ndx++)
        //        glSampleMaski(ndx, ~0u);
        //}

        EXPECT_GL_NO_ERROR();
    }

    // Texture state.
    // \todo [2013-04-08 pyry] Reset all levels?
    {
        //const float borderColor[]   = { 0.0f, 0.0f, 0.0f, 0.0f };
        int         numTexUnits     = 0;
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &numTexUnits);

        for (int ndx = 0; ndx < numTexUnits; ndx++)
        {
            glActiveTexture(GL_TEXTURE0 + ndx);

            // Reset 2D texture.
            glBindTexture(GL_TEXTURE_2D, 0);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,      GL_NEAREST_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,      GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,          GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,          GL_REPEAT);

            if (clientVersion >= 3)
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R,       GL_RED);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G,       GL_GREEN);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B,       GL_BLUE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A,       GL_ALPHA);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD,         -1000.0f);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD,         1000.0f);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL,      0);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,       1000);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,    GL_NONE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC,    GL_LEQUAL);
            }

            //if (contextSupports(type, ApiType::es(3,1)))
            //    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);

            //if (ctxInfo->isExtensionSupported("GL_EXT_texture_border_clamp"))
            //    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &borderColor[0]);

            // Reset cube map texture.
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,    GL_NEAREST_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER,    GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,        GL_REPEAT);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,        GL_REPEAT);

            if (clientVersion >= 3)
            {
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_R,     GL_RED);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_G,     GL_GREEN);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_B,     GL_BLUE);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_A,     GL_ALPHA);
                glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_LOD,       -1000.0f);
                glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LOD,       1000.0f);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL,    0);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL,     1000);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE,  GL_NONE);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC,  GL_LEQUAL);
            }

            //if (contextSupports(type, ApiType::es(3,1)))
            //    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);

            //if (ctxInfo->isExtensionSupported("GL_EXT_texture_border_clamp"))
            //    glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, &borderColor[0]);

            if (clientVersion >= 3)
            {
                // Reset 2D array texture.
                glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
                glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 0, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER,    GL_NEAREST_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER,    GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S,        GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T,        GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_R,     GL_RED);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_G,     GL_GREEN);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_B,     GL_BLUE);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_A,     GL_ALPHA);
                glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_LOD,       -1000.0f);
                glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LOD,       1000.0f);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL,    0);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL,     1000);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE,  GL_NONE);
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC,  GL_LEQUAL);

                //if (ctxInfo->isExtensionSupported("GL_EXT_texture_border_clamp"))
                //    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, &borderColor[0]);
            }

            //if (contextSupports(type, ApiType::es(3,1)))
            //    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);

            if (clientVersion >= 3)
            {
                // Reset 3D texture.
                glBindTexture(GL_TEXTURE_3D, 0);
                glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 0, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER,      GL_NEAREST_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER,      GL_LINEAR);
                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S,          GL_REPEAT);
                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T,          GL_REPEAT);
                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R,          GL_REPEAT);
                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_SWIZZLE_R,       GL_RED);
                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_SWIZZLE_G,       GL_GREEN);
                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_SWIZZLE_B,       GL_BLUE);
                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_SWIZZLE_A,       GL_ALPHA);
                glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_LOD,         -1000.0f);
                glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAX_LOD,         1000.0f);
                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL,      0);
                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL,       1000);
                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_COMPARE_MODE,    GL_NONE);
                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_COMPARE_FUNC,    GL_LEQUAL);

                //if (ctxInfo->isExtensionSupported("GL_EXT_texture_border_clamp"))
                //    glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, &borderColor[0]);
            }

            //if (contextSupports(type, ApiType::es(3,1)))
            //    glTexParameteri(GL_TEXTURE_3D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);

            //if (contextSupports(type, ApiType::es(3,1)))
            //{
            //    // Reset multisample textures.
            //    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
            //    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_SWIZZLE_R,   GL_RED);
            //    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_SWIZZLE_G,   GL_GREEN);
            //    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_SWIZZLE_B,   GL_BLUE);
            //    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_SWIZZLE_A,   GL_ALPHA);
            //    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_BASE_LEVEL,  0);
            //    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL,   1000);
            //}

            //if (ctxInfo->isExtensionSupported("GL_OES_texture_storage_multisample_2d_array"))
            //{
            //    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, 0);
            //    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, GL_TEXTURE_SWIZZLE_R,     GL_RED);
            //    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, GL_TEXTURE_SWIZZLE_G,     GL_GREEN);
            //    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, GL_TEXTURE_SWIZZLE_B,     GL_BLUE);
            //    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, GL_TEXTURE_SWIZZLE_A,     GL_ALPHA);
            //    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, GL_TEXTURE_BASE_LEVEL,    0);
            //    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, GL_TEXTURE_MAX_LEVEL,     1000);
            //}

            //if (ctxInfo->isExtensionSupported("GL_EXT_texture_cube_map_array"))
            //{
            //    // Reset cube array texture.
            //    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);
            //    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER,      GL_NEAREST_MIPMAP_LINEAR);
            //    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER,      GL_LINEAR);
            //    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S,          GL_REPEAT);
            //    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T,          GL_REPEAT);
            //    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_SWIZZLE_R,       GL_RED);
            //    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_SWIZZLE_G,       GL_GREEN);
            //    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_SWIZZLE_B,       GL_BLUE);
            //    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_SWIZZLE_A,       GL_ALPHA);
            //    glTexParameterf(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_LOD,         -1000.0f);
            //    glTexParameterf(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAX_LOD,         1000.0f);
            //    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_BASE_LEVEL,      0);
            //    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAX_LEVEL,       1000);
            //    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_COMPARE_MODE,    GL_NONE);
            //    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_COMPARE_FUNC,    GL_LEQUAL);

            //    //if (ctxInfo->isExtensionSupported("GL_EXT_texture_border_clamp"))
            //    //    glTexParameterfv(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_BORDER_COLOR, &borderColor[0]);
            //}
        }

        glActiveTexture(GL_TEXTURE0);

        if (clientVersion >= 3)
        {
            for (int ndx = 0; ndx < numTexUnits; ndx++)
                glBindSampler(ndx, 0);
        }

        EXPECT_GL_NO_ERROR();
    }

    // Pixel operations.
    {
        GLint scissorWidth = mEGLWindow->getWidth();
        GLint scissorHeight = mEGLWindow->getHeight();

        glDisable      (GL_SCISSOR_TEST);
        glScissor      (0, 0, scissorWidth, scissorHeight);

        glDisable      (GL_STENCIL_TEST);
        glStencilFunc  (GL_ALWAYS, 0, ~0u);
        glStencilOp    (GL_KEEP, GL_KEEP, GL_KEEP);

        glDisable      (GL_DEPTH_TEST);
        glDepthFunc    (GL_LESS);

        glDisable      (GL_BLEND);
        glBlendFunc    (GL_ONE, GL_ZERO);
        glBlendEquation(GL_FUNC_ADD);
        glBlendColor   (0.0f, 0.0f, 0.0f, 0.0f);

        glEnable       (GL_DITHER);

        EXPECT_GL_NO_ERROR();
    }

    // Framebuffer control.
    {
        glColorMask        (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask        (GL_TRUE);
        glStencilMask      (~0u);

        glClearColor       (0.0f, 0.0f, 0.0f, 0.0f);
        glClearDepthf      (1.0f);
        glClearStencil     (0);

        EXPECT_GL_NO_ERROR();
    }

    // Framebuffer state.
    {
        // \note Actually spec explictly says 0 but on some platforms (iOS) no default framebuffer exists.
        const GLenum drawBuffer      = GL_BACK;
        const GLenum readBuffer      = GL_BACK;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        if (clientVersion >= 3)
        {
            glDrawBuffers  (1, &drawBuffer);
            glReadBuffer   (readBuffer);
        }

        EXPECT_GL_NO_ERROR();
    }

    // Renderbuffer state.
    {
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        EXPECT_GL_NO_ERROR();
    }

    // Pixel transfer state.
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT,     4);
        glPixelStorei(GL_PACK_ALIGNMENT,       4);

        if (clientVersion >= 3)
        {
            glPixelStorei(GL_UNPACK_IMAGE_HEIGHT,  0);
            glPixelStorei(GL_UNPACK_SKIP_IMAGES,   0);
            glPixelStorei(GL_UNPACK_ROW_LENGTH,    0);
            glPixelStorei(GL_UNPACK_SKIP_ROWS,     0);
            glPixelStorei(GL_UNPACK_SKIP_PIXELS,   0);

            glPixelStorei(GL_PACK_ROW_LENGTH,      0);
            glPixelStorei(GL_PACK_SKIP_ROWS,       0);
            glPixelStorei(GL_PACK_SKIP_PIXELS,     0);

            glBindBuffer(GL_PIXEL_PACK_BUFFER,     0);
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER,   0);
        }

        EXPECT_GL_NO_ERROR();
    }

    // Program object state.
    {
        glUseProgram(0);

        if (clientVersion >= 3)
        {
            int maxUniformBufferBindings = 0;
            glGetIntegerv  (GL_MAX_UNIFORM_BUFFER_BINDINGS, &maxUniformBufferBindings);
            glBindBuffer   (GL_UNIFORM_BUFFER, 0);

            for (int ndx = 0; ndx < maxUniformBufferBindings; ndx++)
                glBindBufferBase(GL_UNIFORM_BUFFER, ndx, 0);
        }

        //if (contextSupports(type, ApiType::es(3,1)))
        //{
        //    glBindProgramPipeline(0);

        //    {
        //        int maxAtomicCounterBufferBindings = 0;
        //        glGetIntegerv  (GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS, &maxAtomicCounterBufferBindings);
        //        glBindBuffer   (GL_ATOMIC_COUNTER_BUFFER, 0);

        //        for (int ndx = 0; ndx < maxAtomicCounterBufferBindings; ndx++)
        //            glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, ndx, 0);
        //    }

        //    {
        //        int maxShaderStorageBufferBindings = 0;
        //        glGetIntegerv  (GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &maxShaderStorageBufferBindings);
        //        glBindBuffer   (GL_SHADER_STORAGE_BUFFER, 0);

        //        for (int ndx = 0; ndx < maxShaderStorageBufferBindings; ndx++)
        //            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ndx, 0);
        //    }
        //}

        EXPECT_GL_NO_ERROR();
    }

    // Vertex shader state.
    {
        int numVertexAttribArrays = 0;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numVertexAttribArrays);

        for (int ndx = 0; ndx < numVertexAttribArrays; ndx++)
            glVertexAttrib4f(ndx, 0.0f, 0.0f, 0.0f, 1.0f);

        EXPECT_GL_NO_ERROR();
    }

    // Transform feedback state.
    if (clientVersion >= 3)
    {
        int numTransformFeedbackSeparateAttribs = 0;
        GLboolean transformFeedbackActive = 0;
        glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS,  &numTransformFeedbackSeparateAttribs);
        glGetBooleanv(GL_TRANSFORM_FEEDBACK_ACTIVE,                &transformFeedbackActive);

        if (transformFeedbackActive)
            glEndTransformFeedback();

        glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);

        for (int ndx = 0; ndx < numTransformFeedbackSeparateAttribs; ndx++)
            glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, ndx, 0);

        EXPECT_GL_NO_ERROR();
    }

    // Asynchronous query state.
    if (clientVersion >= 3)
    {
        static const GLenum targets[] = { GL_ANY_SAMPLES_PASSED, GL_ANY_SAMPLES_PASSED_CONSERVATIVE, GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN };

        for (int i = 0; i < ArraySize(targets); i++)
        {
            int queryActive = 0;
            glGetQueryiv(targets[i], GL_CURRENT_QUERY, &queryActive);

            if (queryActive != 0)
                glEndQuery(targets[i]);
        }

        EXPECT_GL_NO_ERROR();
    }

    // Hints.
    {
        glHint(GL_GENERATE_MIPMAP_HINT, GL_DONT_CARE);

        if (clientVersion >= 3)
            glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_DONT_CARE);

        EXPECT_GL_NO_ERROR();
    }

    // Compute.
    //if (contextSupports(type, ApiType::es(3,1)))
    //{
    //    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, 0);
    //    EXPECT_GL_NO_ERROR();
    //}

    // Buffer copy state.
    if (clientVersion >= 3)
    {
        glBindBuffer(GL_COPY_READ_BUFFER,  0);
        glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

        EXPECT_GL_NO_ERROR();
    }

    // Images.
    //if (contextSupports(type, ApiType::es(3,1)))
    //{
    //    int numImageUnits = 0;
    //    glGetIntegerv(GL_MAX_IMAGE_UNITS, &numImageUnits);

    //    for (int ndx = 0; ndx < numImageUnits; ndx++)
    //        glBindImageTexture(ndx, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);

    //    EXPECT_GL_NO_ERROR();
    //}

    // Sample shading state.
    //if (contextSupports(type, ApiType::es(3,1)) && ctxInfo->isExtensionSupported("GL_OES_sample_shading"))
    //{
    //    glMinSampleShading(0.0f);
    //    glDisable(GL_SAMPLE_SHADING);

    //    EXPECT_GL_NO_ERROR();
    //}

    // Debug state
    //if (ctxInfo->isExtensionSupported("GL_KHR_debug"))
    //{
    //    const bool entrypointsPresent = glDebugMessageControl  != nullptr  &&
    //                                    glDebugMessageCallback != nullptr  &&
    //                                    glPopDebugGroup        != nullptr;

    //    // some drivers advertise GL_KHR_debug but give out null pointers. Silently ignore.
    //    if (entrypointsPresent)
    //    {
    //        int stackDepth = 0;
    //        glGetIntegerv(GL_DEBUG_GROUP_STACK_DEPTH, &stackDepth);
    //        for (int ndx = 1; ndx < stackDepth; ++ndx)
    //            glPopDebugGroup();

    //        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
    //        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, false);
    //        glDebugMessageCallback(nullptr, nullptr);

    //        if (type.getFlags() & glu::CONTEXT_DEBUG)
    //            glEnable(GL_DEBUG_OUTPUT);
    //        else
    //            glDisable(GL_DEBUG_OUTPUT);
    //        glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    //        EXPECT_GL_NO_ERROR();
    //    }
    //}

    // Primitive bounding box state.
    //if (ctxInfo->isExtensionSupported("GL_EXT_primitive_bounding_box"))
    //{
    //    glPrimitiveBoundingBoxEXT(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    //    EXPECT_GL_NO_ERROR();
    //}

    // Tessellation state
    //if (ctxInfo->isExtensionSupported("GL_EXT_tessellation_shader"))
    //{
    //    glPatchParameteri(GL_PATCH_VERTICES, 3);
    //    EXPECT_GL_NO_ERROR();
    //}

    // Advanced coherent blending
    //if (ctxInfo->isExtensionSupported("GL_KHR_blend_equation_advanced_coherent"))
    //{
    //    glEnable(GL_BLEND_ADVANCED_COHERENT_KHR);
    //    EXPECT_GL_NO_ERROR();
    //}

    // Texture buffer
    //if (ctxInfo->isExtensionSupported("GL_EXT_texture_buffer"))
    //{
    //    glBindTexture(GL_TEXTURE_BUFFER, 0);
    //    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    //    EXPECT_GL_NO_ERROR();
    //}
}

