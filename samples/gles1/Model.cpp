//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "SampleApplication.h"
#include "obj_utils.h"
#include "system_utils.h"
#include "texture_utils.h"

#include <sstream>

#include <GLES/gl.h>
#include <GLES/glext.h>

class ModelSample : public SampleApplication
{
  public:
    ModelSample(EGLint displayType)
        : SampleApplication("ModelSample", 1280, 720,
                            1 /* gles1 */, 0, displayType)
    {
    }

    virtual bool initialize()
    {
        std::stringstream modelName;
        modelName << angle::GetExecutableDirectory() << "/colored_cube.obj";

        ObjModel parsedModel(modelName.str());
        mVertexAttrs = parsedModel.vertexData;
        mIndices = parsedModel.indexData;

        std::stringstream diffuseTextureName;
        diffuseTextureName << angle::GetExecutableDirectory() << "/colored_cube_diffuse.png";

        // Load the texture
        mTexture = CreateTextureFromPng(diffuseTextureName.str().c_str(),
                                        &mTextureWidth,
                                        &mTextureHeight);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0);
        glClientActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTexture);

        glClearColor(0.1f, 0.2f, 0.4f, 0.5f);

        glGenBuffers(1, &mIndexBuffer);
        glGenBuffers(1, &mVertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * 4, mIndices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, mVertexAttrs.size() * sizeof(ObjModel::VertexAttributes), mVertexAttrs.data(), GL_STATIC_DRAW);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glVertexPointer(3, GL_FLOAT, 8 * sizeof(GLfloat), 0);
        glNormalPointer(GL_FLOAT, 8 * sizeof(GLfloat), (void*)(uintptr_t)(3 * sizeof(GLfloat)));
        glTexCoordPointer(2, GL_FLOAT, 8 * sizeof(GLfloat), (void*)(uintptr_t)(6 * sizeof(GLfloat)));

        glMatrixMode(GL_PROJECTION);
        glFrustumf(-1.7778f, 1.7778f, -1.0f, 1.0f, 1.0f, 10.0f);

        mTime = 0.0f;
        return true;
    }

    virtual void destroy()
    {
        glDeleteTextures(1, &mTexture);
        glDeleteBuffers(1, &mIndexBuffer);
        glDeleteBuffers(1, &mVertexBuffer);
    }

    virtual void draw()
    {
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0.00f, 0.00f, -5.0f);
        glRotatef(mTime, 0.00f, 1.00f, 0.00f);
        glRotatef(90.0f, 1.00f, 0.00f, 0.00f);

        glDrawElements(GL_TRIANGLES, mIndices.size(), GL_UNSIGNED_INT, 0);

        mTime += 0.03f;
    }

  private:
    GLuint mTexture;
    GLuint mTextureWidth;
    GLuint mTextureHeight;
    GLuint mIndexBuffer;
    GLuint mVertexBuffer;
    std::vector<ObjModel::VertexAttributes> mVertexAttrs;
    std::vector<unsigned int> mIndices;
    float mTime;
};

int main(int argc, char **argv)
{
    EGLint displayType = EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE;

    if (argc > 1)
    {
        displayType = GetDisplayTypeFromArg(argv[1]);
    }

    ModelSample app(displayType);
    return app.run();
}
