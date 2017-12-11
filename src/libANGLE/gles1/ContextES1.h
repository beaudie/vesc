//
// Copyright (c) 2002-2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ContextES1.h: Defines context state and API for GLES1 contexts. Relies on
// GLES3+-specific implementation of Context underneath. It is constructed
// with the same interface as other Contexts.

#ifndef LIBANGLE_CONTEXTES1_H_
#define LIBANGLE_CONTEXTES1_H_

#include "libANGLE/Context.h"

#include <GLES/gl.h>
#include <GLES/glext.h>

#include <unordered_map>
#include <vector>

namespace gl {

class ContextES1 : public Context {
  public:
    ContextES1(rx::EGLImplFactory *implFactory,
               const egl::Config *config,
               const Context *shareContext,
               TextureManager *shareTextures,
               MemoryProgramCache *memoryProgramCache,
               const egl::AttributeMap &attribs,
               const egl::DisplayExtensions &displayExtensions);

    // API
    virtual void getIntegerv(GLenum pname, GLint *params) override;
    virtual void getBooleanv(GLenum pname, GLboolean *params) override;
    virtual void getFloatv(GLenum pname, GLfloat *params) override;
    void getFixedv(GLenum pname, GLfixed *params);

    virtual void enable(GLenum cap) override;
    virtual void disable(GLenum cap) override;

    void shadeModel(GLenum mode);
    GLenum getShadeModel() const { return mShadeModel; }

    void matrixMode(GLenum mode);
    void loadIdentity();
    void loadMatrixf(const GLfloat* m);
    void pushMatrix();
    void popMatrix();
    void multMatrixf(const GLfloat* m);

    void orthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
    void frustumf(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);

    void texEnvf(GLenum target, GLenum pname, GLfloat param);
    void texEnvfv(GLenum target, GLenum pname, const GLfloat* params);
    void texEnvi(GLenum target, GLenum pname, GLint param);
    void texEnviv(GLenum target, GLenum pname, const GLint* params);
    void getTexEnvfv(GLenum env, GLenum pname, GLfloat* params);
    void getTexEnviv(GLenum env, GLenum pname, GLint* params);

    void texGenf(GLenum coord, GLenum pname, GLfloat param);
    void texGenfv(GLenum coord, GLenum pname, const GLfloat* params);
    void texGeni(GLenum coord, GLenum pname, GLint param);
    void texGeniv(GLenum coord, GLenum pname, const GLint* params);
    void getTexGeniv(GLenum coord, GLenum pname, GLint* params);
    void getTexGenfv(GLenum coord, GLenum pname, GLfloat* params);

    void materialf(GLenum face, GLenum pname, GLfloat param);
    void materialfv(GLenum face, GLenum pname, const GLfloat* params);
    void getMaterialfv(GLenum face, GLenum pname, GLfloat* params);

    void lightModelf(GLenum pname, GLfloat param);
    void lightModelfv(GLenum pname, const GLfloat* params);
    void lightf(GLenum light, GLenum pname, GLfloat param);
    void lightfv(GLenum light, GLenum pname, const GLfloat* params);
    void getLightfv(GLenum light, GLenum pname, GLfloat* params);

    void multiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
    void normal3f(GLfloat nx, GLfloat ny, GLfloat nz);

    void fogf(GLenum pname, GLfloat param);
    void fogfv(GLenum pname, const GLfloat* params);

    void enableClientState(GLenum clientState);
    void disableClientState(GLenum clientState);

    void drawTexOES(float x, float y, float z, float width, float height);

    void rotatef(float angle, float x, float y, float z);
    void scalef(float x, float y, float z);
    void translatef(float x, float y, float z);

    void color4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    void color4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);

    void drawArrays(GLenum mode, GLint first, GLsizei count);
    void drawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);

    void clientActiveTexture(GLenum texture);

protected:
    union GLVal {
        GLfloat floatVal[4];
        GLint intVal[4];
        GLubyte ubyteVal[4];
        GLenum enumVal[4];
    };

    struct GLValTyped {
        GLenum type;
        GLVal val;
    };

    using Mat4 = std::vector<float>;
    using TexEnv = std::unordered_map<GLenum, GLValTyped>;
    using TexUnitEnvs = std::vector<TexEnv>;
    using TexGens = std::vector<TexEnv>;
    using MatrixStack = std::vector<Mat4>;

    struct Material {
        GLfloat ambient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
        GLfloat diffuse[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
        GLfloat specular[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat emissive[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat specularExponent = 0.0f;
    };

    struct LightModel {
        GLfloat color[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
        bool twoSided = false;
    };

    struct Light {
        GLfloat ambient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat diffuse[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat specular[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat position[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
        GLfloat direction[3] = { 0.0f, 0.0f, -1.0f };
        GLfloat spotlightExponent = 0.0f;
        GLfloat spotlightCutoffAngle = 180.0f;
        GLfloat attenuationConst = 1.0f;
        GLfloat attenuationLinear = 0.0f;
        GLfloat attenuationQuadratic = 0.0f;
    };

    struct Fog {
        GLenum mode = GL_EXP;
        float density = 1.0f;
        float start = 0.0f;
        float end = 1.0f;
        GLfloat color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    };

    static constexpr int kMaxTextureUnits = 4;
    static constexpr int kMaxLights = 8;

    GLenum mShadeModel = GL_SMOOTH;
    GLenum mCurrMatrixMode = GL_PROJECTION;

    GLValTyped mColor;
    GLValTyped mNormal;
    GLVal mMultiTexCoord[kMaxTextureUnits] = {};

    TexUnitEnvs mTexUnitEnvs;
    TexGens mTexGens;

    MatrixStack mProjMatrices;
    MatrixStack mModelviewMatrices;
    std::vector<MatrixStack> mTextureMatrices;
    Mat4& currMatrix();
    MatrixStack& currMatrixStack();
    void restoreMatrixStack(const MatrixStack& matrices);

    Material mMaterial;
    LightModel mLightModel;
    Light mLights[kMaxLights] = {};
    Fog mFog = {};
    
};

} // namespace gl

#endif
