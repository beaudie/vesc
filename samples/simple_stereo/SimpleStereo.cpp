//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "SampleApplication.h"
#include "Matrix.h"
#include "common/vector_utils.h"
#include "shader_utils.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <array>
#include <string.h>

const float CUBE_HALF_SIZE = 0.03f;
const float CAMERA_DISTANCE = 0.2f;
const float CAMERA_INCLINATION_ANGLE_DEGREES = 45.0f;
const float ROTATION_SPEED_DEGREES_PER_SECOND = 45.0f;

void computeLookAtMatrix(Matrix4 &m, const angle::Vector3 &eye, const angle::Vector3 &center,
  const angle::Vector3 &up);

  class SimpleStereoSample : public SampleApplication
{
public:
    SimpleStereoSample(int argc, char **argv)
      : SampleApplication("SimpleStereo", argc, argv, 3, 0, 1280, 720, true),
    mCameraAngleDegrees(0.0f)
  {
  }

  virtual bool initialize()
  {
    GLint numBuffers = 0;
    glGetIntegerv(GL_MAX_MULTIVIEW_BUFFERS_EXT, &numBuffers);
    mMultiviewBuffers = numBuffers == 2 ? true : false;

    const std::string vs =
      R"(attribute vec4 a_vertexPosition;
            attribute vec4 a_vertexNormal;
            attribute vec3 a_vertexColor;
            uniform mat4 u_viewMatrix;
            uniform mat4 u_projectionMatrix;
            varying vec4 v_vertexColor;
            void main()
            {
                v_vertexColor = vec4(a_vertexColor, 1.0);
                gl_Position = u_projectionMatrix * u_viewMatrix * a_vertexPosition;
            })";

    const std::string fs =
      R"(precision mediump float;
            varying vec4 v_vertexColor;
            void main()
            {
                gl_FragColor = v_vertexColor;
            })";

    mProgram = CompileProgram(vs, fs);
    if (!mProgram)
    {
      return false;
    }

    mVertexPositionAttribLoc = glGetAttribLocation(mProgram, "a_vertexPosition");
    mVertexNormalAttribLoc = glGetAttribLocation(mProgram, "a_vertexNormal");
    mVertexColorAttribLoc = glGetAttribLocation(mProgram, "a_vertexColor");

    mViewMatrixUniformLoc = glGetUniformLocation(mProgram, "u_viewMatrix");
    mProjectionMatrixUniformLoc = glGetUniformLocation(mProgram, "u_projectionMatrix");

    initializeVertexData();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepthf(1.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    return true;
  }

  virtual void destroy()
  {
    glDeleteProgram(mProgram);
  }

  virtual void step(float dt, double /* totalTime */)
  {
    mCameraAngleDegrees += ROTATION_SPEED_DEGREES_PER_SECOND * dt;
    mCameraAngleDegrees = fmodf(mCameraAngleDegrees, 360.0f);

    const float cameraInclinationAngleRadians = CAMERA_INCLINATION_ANGLE_DEGREES *
      static_cast<float>(M_PI) / 180.0f;

    const float cameraAngleRadians = mCameraAngleDegrees * static_cast<float>(M_PI) / 180.0f;

    const angle::Vector3 eyePosition(
      CAMERA_DISTANCE * sin(cameraInclinationAngleRadians) * sin(cameraAngleRadians),
      CAMERA_DISTANCE * cos(cameraInclinationAngleRadians),
      CAMERA_DISTANCE * sin(cameraInclinationAngleRadians) * cos(cameraAngleRadians));

    const angle::Vector3 centerPosition(0.0f, 0.0f, 0.0f);

    const angle::Vector3 upDirection(0.0f, 1.0f, 0.0f);

    computeLookAtMatrix(mViewMatrix, eyePosition, centerPosition, upDirection);

    mEyeViewMatrices[0] =
      mViewMatrix.translate(angle::Vector3(-0.01f, 0.0f, 0.0f)) * mViewMatrix;
    mEyeViewMatrices[1] =
      mViewMatrix.translate(angle::Vector3(0.01f, 0.0f, 0.0f)) * mViewMatrix;
  }

  virtual void draw()
  {
    // Set the viewport
    glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

    // Set the projection matrix
    mProjectionMatrix = Matrix4::perspective(60.0f,
      static_cast<float>(getWindow()->getWidth()) /
      getWindow()->getHeight(),
      0.01f,
      100.0f);

    // Use the program object
    glUseProgram(mProgram);

    // Load the vertex position data
    glVertexAttribPointer(mVertexPositionAttribLoc,
      3,
      GL_FLOAT,
      GL_FALSE,
      9 * sizeof(GLfloat),
      mVertexAttribData.data());
    glEnableVertexAttribArray(mVertexPositionAttribLoc);

    // Load the vertex normal data
    glVertexAttribPointer(mVertexNormalAttribLoc,
      3,
      GL_FLOAT,
      GL_FALSE,
      9 * sizeof(GLfloat),
      mVertexAttribData.data() + 3);
    glEnableVertexAttribArray(mVertexNormalAttribLoc);

    // Load the vertex color data
    glVertexAttribPointer(mVertexColorAttribLoc,
      3,
      GL_FLOAT,
      GL_FALSE,
      9 * sizeof(GLfloat),
      mVertexAttribData.data() + 6);
    glEnableVertexAttribArray(mVertexColorAttribLoc);

    // Set the projection and view matrices
    glUniformMatrix4fv(mProjectionMatrixUniformLoc, 1, GL_FALSE, mProjectionMatrix.data);
    glUniformMatrix4fv(mViewMatrixUniformLoc, 1, GL_FALSE, mViewMatrix.data);

    drawEye(0);
    if (mMultiviewBuffers)
    {
        drawEye(1);
    }
  }

private:

  void initializeVertexData()
  {
    // Positions and normals are interleaved.
    const float vertexAttribData[] = {
      // Side 1
      -CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE,
      0.0f, 0.0f, -1.0f,
      1.0f, 0.0f, 0.0f,
      -CUBE_HALF_SIZE,  CUBE_HALF_SIZE, -CUBE_HALF_SIZE,
      0.0f, 0.0f, -1.0f,
      1.0f, 0.0f, 0.0f,
      CUBE_HALF_SIZE,  CUBE_HALF_SIZE, -CUBE_HALF_SIZE,
      0.0f, 0.0f, -1.0f,
      1.0f, 0.0f, 0.0f,
      CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE,
      0.0f, 0.0f, -1.0f,
      1.0f, 0.0f, 0.0f,

      // Side 2
      -CUBE_HALF_SIZE,  CUBE_HALF_SIZE, CUBE_HALF_SIZE,
      0.0f, 0.0f, 1.0f,
      0.0f, 1.0f, 0.0f,
      -CUBE_HALF_SIZE, -CUBE_HALF_SIZE, CUBE_HALF_SIZE,
      0.0f, 0.0f, 1.0f,
      0.0f, 1.0f, 0.0f,
      CUBE_HALF_SIZE, -CUBE_HALF_SIZE, CUBE_HALF_SIZE,
      0.0f, 0.0f, 1.0f,
      0.0f, 1.0f, 0.0f,
      CUBE_HALF_SIZE,  CUBE_HALF_SIZE, CUBE_HALF_SIZE,
      0.0f, 0.0f, 1.0f,
      0.0f, 1.0f, 0.0f,

      // Side 3
      -CUBE_HALF_SIZE, CUBE_HALF_SIZE, -CUBE_HALF_SIZE,
      0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 1.0f,
      -CUBE_HALF_SIZE, CUBE_HALF_SIZE,  CUBE_HALF_SIZE,
      0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 1.0f,
      CUBE_HALF_SIZE, CUBE_HALF_SIZE,  CUBE_HALF_SIZE,
      0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 1.0f,
      CUBE_HALF_SIZE, CUBE_HALF_SIZE, -CUBE_HALF_SIZE,
      0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 1.0f,

      // Side 4
      -CUBE_HALF_SIZE, -CUBE_HALF_SIZE,  CUBE_HALF_SIZE,
      0.0f, -1.0f, 0.0f,
      0.0f, 1.0f, 1.0f,
      -CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE,
      0.0f, -1.0f, 0.0f,
      0.0f, 1.0f, 1.0f,
      CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE,
      0.0f, -1.0f, 0.0f,
      0.0f, 1.0f, 1.0f,
      CUBE_HALF_SIZE, -CUBE_HALF_SIZE,  CUBE_HALF_SIZE,
      0.0f, -1.0f, 0.0f,
      0.0f, 1.0f, 1.0f,

      // Side 5
      CUBE_HALF_SIZE,  CUBE_HALF_SIZE, -CUBE_HALF_SIZE,
      1.0f, 0.0f, 0.0f,
      1.0f, 0.0f, 1.0f,
      CUBE_HALF_SIZE,  CUBE_HALF_SIZE,  CUBE_HALF_SIZE,
      1.0f, 0.0f, 0.0f,
      1.0f, 0.0f, 1.0f,
      CUBE_HALF_SIZE, -CUBE_HALF_SIZE,  CUBE_HALF_SIZE,
      1.0f, 0.0f, 0.0f,
      1.0f, 0.0f, 1.0f,
      CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE,
      1.0f, 0.0f, 0.0f,
      1.0f, 0.0f, 1.0f,

      // Side 6
      -CUBE_HALF_SIZE,  CUBE_HALF_SIZE, -CUBE_HALF_SIZE,
      -1.0f, 0.0f, 0.0f,
      1.0f, 1.0f, 0.0f,
      -CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE,
      -1.0f, 0.0f, 0.0f,
      1.0f, 1.0f, 0.0f,
      -CUBE_HALF_SIZE, -CUBE_HALF_SIZE,  CUBE_HALF_SIZE,
      -1.0f, 0.0f, 0.0f,
      1.0f, 1.0f, 0.0f,
      -CUBE_HALF_SIZE,  CUBE_HALF_SIZE,  CUBE_HALF_SIZE,
      -1.0f, 0.0f, 0.0f,
      1.0f, 1.0f, 0.0f,
    };

    const GLushort vertexIndices[] = {
      // Side 1
      0, 1, 2,
      0, 2, 3,

      // Side 2
      5, 6, 4,
      6, 7, 4,

      // Side 3
      8, 10, 9,
      8, 11, 10,

      // Side 4
      12, 14, 13,
      12, 15, 14,

      // Side 5
      16, 17, 18,
      16, 18, 19,

      // Side 6
      20, 21, 22,
      20, 22, 23,
    };

    const int numVertexAttribDataFloats = sizeof(vertexAttribData) / sizeof(float);

    mVertexAttribData.clear();
    mVertexAttribData.reserve(numVertexAttribDataFloats);
    for (int i = 0; i < numVertexAttribDataFloats; ++i)
    {
      mVertexAttribData.push_back(vertexAttribData[i]);
    }

    const int numVertexIndices = sizeof(vertexIndices) / sizeof(GLushort);

    mVertexIndices.clear();
    mVertexIndices.reserve(numVertexIndices);
    for (int i = 0; i < numVertexIndices; ++i)
    {
      mVertexIndices.push_back(vertexIndices[i]);
    }
  }

  void drawEye(int eyeIndex)
  {
    if (mMultiviewBuffers)
    {
      GLenum locations[] = { GL_MULTIVIEW_EXT };
      GLint indices[] = { eyeIndex };

      glDrawBuffersIndexedEXT(1, locations, indices);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUniformMatrix4fv(mViewMatrixUniformLoc, 1, GL_FALSE, mEyeViewMatrices[eyeIndex].data);

    glDrawElements(GL_TRIANGLES,
      static_cast<GLsizei>(mVertexIndices.size()),
      GL_UNSIGNED_SHORT,
      mVertexIndices.data());
  }

  GLuint mProgram;

  GLint mVertexPositionAttribLoc;
  GLint mVertexNormalAttribLoc;
  GLint mVertexColorAttribLoc;

  GLint mViewMatrixUniformLoc;
  GLint mProjectionMatrixUniformLoc;

  std::vector<float> mVertexAttribData;
  std::vector<GLushort> mVertexIndices;

  Matrix4 mViewMatrix;
  Matrix4 mEyeViewMatrices[2];
  Matrix4 mProjectionMatrix;

  float mCameraAngleDegrees;

  bool mMultiviewBuffers;
};

int main(int argc, char **argv)
{
  SimpleStereoSample app(argc, argv);
  return app.run();
}

void computeLookAtMatrix(Matrix4 &m, const angle::Vector3 &eye, const angle::Vector3 &center,
  const angle::Vector3 &up)
{
  auto zAxis = (center - eye).normalized();
  auto xAxis = zAxis.cross(up.normalized()).normalized();
  auto yAxis = xAxis.cross(zAxis).normalized();

  m.data[0] = xAxis.x();
  m.data[1] = yAxis.x();
  m.data[2] = -zAxis.x();
  m.data[3] = 0.0;
  m.data[4] = xAxis.y();
  m.data[5] = yAxis.y();
  m.data[6] = -zAxis.y();
  m.data[7] = 0.0;
  m.data[8] = xAxis.z();
  m.data[9] = yAxis.z();
  m.data[10] = -zAxis.z();
  m.data[11] = 0.0;
  m.data[12] = -(xAxis.dot(eye));
  m.data[13] = -(yAxis.dot(eye));
  m.data[14] = zAxis.dot(eye);
  m.data[15] = 1.0;
}